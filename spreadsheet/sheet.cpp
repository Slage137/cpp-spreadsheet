#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

void Sheet::SetCell(Position pos, std::string text) {
    ThrowIfInvalidPosition(pos);

    auto newRowSize = std::max(pos.row + 1, static_cast<int>(cells_.size()));
    cells_.resize(newRowSize);
    auto newColSize = std::max(pos.col + 1, static_cast<int>(cells_[pos.row].size()));
    cells_[pos.row].resize(newColSize);

    auto& cell = cells_[pos.row][pos.col];

    if (!cell) {
        cell = std::make_unique<Cell>(*this);
    }
    cell->Set(std::move(text));
}

CellInterface* Sheet::GetCell(Position pos) {
    ThrowIfInvalidPosition(pos);

    if (pos.row < static_cast<int>(cells_.size()) &&
        pos.col < static_cast<int>(cells_[pos.row].size())) {
        auto& cell = cells_[pos.row][pos.col];
        if (!cell->GetText().empty()) {
            return cell.get();
        }
    }

    return nullptr;
}

const CellInterface* Sheet::GetCell(Position pos) const {
    ThrowIfInvalidPosition(pos);

    if (pos.row < static_cast<int>(cells_.size()) &&
        pos.col < static_cast<int>(cells_[pos.row].size())) {
        auto& cell = cells_[pos.row][pos.col];
        if (!cell->GetText().empty()) {
            return cell.get();
        }
    }

    return nullptr;
}

Cell* Sheet::GetCellNotInterface(Position pos) {
    ThrowIfInvalidPosition(pos);

    if (pos.row < static_cast<int>(cells_.size()) && pos.col < static_cast<int>(cells_[pos.row].size())) {
        return cells_[pos.row][pos.col].get();
    }

    return nullptr;
}

const Cell* Sheet::GetCellNotInterface(Position pos) const {
    ThrowIfInvalidPosition(pos);

    if (pos.row < static_cast<int>(cells_.size()) && pos.col < static_cast<int>(cells_[pos.row].size())) {
        return cells_[pos.row][pos.col].get();
    }

    return nullptr;
}

void Sheet::ClearCell(Position pos) {
    ThrowIfInvalidPosition(pos);

    const auto row = static_cast<size_t>(pos.row);
    const auto col = static_cast<size_t>(pos.col);

    if (row < cells_.size() && col < cells_[row].size()) {
        auto& cell = cells_[row][col];
        if (cell) {
            cell->Clear();
            if (!cell->IsReferenced()) {
                cell.reset();
            }
        }
    }
}

Size Sheet::GetPrintableSize() const {
    Size size;

    for (int row = 0; row < static_cast<int>(cells_.size()); ++row) {
        const auto& rowCells = cells_[row];
        for (int col = static_cast<int>(rowCells.size() - 1); col >= 0; --col) {
            auto& cell = rowCells[col];
            if (cell && !cell->GetText().empty()) {
                size.rows = std::max(size.rows, row + 1);
                size.cols = std::max(size.cols, col + 1);
                break;
            }
        }
    }

    return size;
}

void Sheet::PrintValues(std::ostream& output) const {
    Size printableSize = GetPrintableSize();

    for (int row = 0; row < printableSize.rows; ++row) {
        for (int col = 0; col < printableSize.cols; ++col) {
            if (col > 0) {
                output << '\t';
            }

            if (col < static_cast<int>(cells_[row].size())) {
                auto& cell = cells_[row][col];
                if (cell) {
                    std::visit([&output] (const auto& obj) {
                        output << obj;
                    }, cell->GetValue());
                }
            }
        }

        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    Size printableSize = GetPrintableSize();

    for (int row = 0; row < printableSize.rows; ++row) {
        for (int col = 0; col < printableSize.cols; ++col) {
            if (col > 0) {
                output << '\t';
            }

            if (col < static_cast<int>(cells_[row].size())) {
                auto& cell = cells_[row][col];
                if (cell) {
                    output << cell->GetText();
                }
            }
        }

        output << '\n';
    }
}

void Sheet::ThrowIfInvalidPosition(Position pos) const {
    if (!pos.IsValid())
        throw InvalidPositionException("invalid position" + pos.ToString());
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
