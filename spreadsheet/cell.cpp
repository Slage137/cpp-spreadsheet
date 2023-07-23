#include "cell.h"
#include "sheet.h"

#include <iostream>
#include <string>


Cell::Cell(Sheet& sheet) : impl_(std::make_unique<EmptyImpl>()),
                           sheet_(sheet)
                           {}

void Cell::Set(std::string text) {
    std::unique_ptr<Impl> temp_impl = CreateImplFromText(std::move(text));

    if (HasCircularDependency(temp_impl.get())) {
        throw CircularDependencyException("circular dependency detected");
    }

    UpdateDependencies(std::move(temp_impl));
}

std::unique_ptr<Cell::Impl> Cell::CreateImplFromText(std::string text) {
    if (text.empty()) {
        return std::make_unique<EmptyImpl>();
    } else if (text.at(0) == FORMULA_SIGN && text.size() >= 2 ) {
        return std::make_unique<FormulaImpl>(text.substr(1), sheet_);
    } else {
        return std::make_unique<TextImpl>(std::move(text));
    }
}

bool Cell::HasCircularDependency(Impl* temp_impl) {
    // Коллекция для хранения ссылок на ячейки, которые используются в формуле текущей ячейки
    std::set<const Cell*> ref_collection;

    // Коллекция для хранения ячеек, которые нужно проверить на наличие циклической зависимости
    std::vector<const Cell*> to_enter_collection;

    // Заполняем ref_collection ссылками на ячейки, которые используются в формуле текущей ячейки
    for (const auto& position : temp_impl->GetReferencedCells()) {
        ref_collection.insert(sheet_.GetCellNotInterface(position));
    }

    // Начинаем проверку на циклические зависимости с текущей ячейки
    to_enter_collection.push_back(this);

    while (!to_enter_collection.empty()) {
        // Берем текущую ячейку для проверки
        const Cell* ongoing = to_enter_collection.back();
        to_enter_collection.pop_back();

        // Проверяем, если текущая ячейка уже есть в ref_collection, то это означает циклическую зависимость
        if (ref_collection.find(ongoing) != ref_collection.end()) {
            return true; // Circular dependency detected
        }

        // Иначе, добавляем в to_enter_collection ячейки, от которых зависит текущая ячейка
        for (const Cell* dependent : ongoing->dependent_cells_) {
            // Но добавляем только если их нет еще в ref_collection, чтобы избежать повторной проверки
            if (ref_collection.find(dependent) == ref_collection.end()) {
                to_enter_collection.push_back(dependent);
            }
        }
    }
    // Если циклических зависимостей не обнаружено, возвращаем false
    return false; // No circular dependency
}

void Cell::UpdateDependencies(std::unique_ptr<Impl> new_impl) {
    // Шаг 1: Обновляем зависимости в ячейках, которые ссылались на текущую ячейку
    for (Cell* referenced : referenced_cells_) {
        referenced->dependent_cells_.erase(this); // Удаляем текущую ячейку из dependent_cells_ ячеек, которые на нее ссылались
    }
    referenced_cells_.clear(); // Очищаем текущий список зависимых ячеек

    // Шаг 2: Обновляем список зависимых ячеек текущей ячейки на основе новой формулы
    for (const auto& position : new_impl->GetReferencedCells()) {
        Cell* referenced = sheet_.GetCellNotInterface(position);

        // Если ячейки с такими координатами нет, то создаем пустую ячейку
        if (!referenced) {
            sheet_.SetCell(position, "");
            referenced = sheet_.GetCellNotInterface(position);
        }

        // Проверяем, чтобы текущая ячейка не добавлялась в свой же список зависимых ячеек
        if (referenced != this) {
            referenced_cells_.emplace(referenced); // Добавляем найденную ячейку в список зависимых
            referenced->dependent_cells_.emplace(this); // Добавляем текущую ячейку в список ячеек, которые на нее ссылаются
        }
    }

    // Шаг 3: Обновляем формулу текущей ячейки на новую формулу
    impl_ = std::move(new_impl);
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {
    return impl_->GetValue();
}

std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}

bool Cell::IsReferenced() const {
    return !dependent_cells_.empty();
}

std::vector<Position> Cell::Impl::GetReferencedCells() const {
    return {};
}

Cell::Value Cell::EmptyImpl::GetValue() const {
    return {};
}
std::string Cell::EmptyImpl::GetText() const {
    return {};
}

Cell::TextImpl::TextImpl(std::string text) : text_(std::move(text)) {}

Cell::Value Cell::TextImpl::GetValue() const {
    if (text_.empty()) {
        throw FormulaException("it is empty impl, not text");

    } else if (text_.at(0) == ESCAPE_SIGN) {
        return text_.substr(1);

    } else {
        return text_;
    }
}

std::string Cell::TextImpl::GetText() const {
    return text_;
}

Cell::FormulaImpl::FormulaImpl(std::string text, SheetInterface& sheet) :
    formula_ptr_(ParseFormula(std::move(text))),
    sheet_(sheet) {}

Cell::Value Cell::FormulaImpl::GetValue() const {
    if (!cache_) {
        cache_ = formula_ptr_->Evaluate(sheet_);
    }

    return std::visit([](auto& val){
        return Value(val);
        }, *cache_);
}

std::string Cell::FormulaImpl::GetText() const {
    return FORMULA_SIGN + formula_ptr_->GetExpression();
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const {
    return formula_ptr_->GetReferencedCells();
}

void Cell::FormulaImpl::InvalidateCache() {
    cache_.reset();
}