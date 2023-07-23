#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, const FormulaError& fe) {
    output << fe.ToString();
    return output;
}

namespace {
    class Formula : public FormulaInterface {
    public:

        explicit Formula(const std::string &expression) try :
            ast_ (ParseFormulaAST(expression)) {}
            catch (...) {
                throw FormulaException("Error when parsing: " + expression);
            }

        [[nodiscard]] Value Evaluate(const SheetInterface& sheet) const override {
            /// Лямбда-функция для обработки текстовых ячеек
            auto ProcessTextCell = [](const std::string& text) {
                if (text.empty()) return 0.0;
                try {
                    return std::stod(text);
                }
                catch (...) {
                    throw FormulaError(FormulaError::Category::Value);
                }
            };

            /// Лямбда-функция для вычисления значения ячейки
            auto eval = [&sheet, &ProcessTextCell](const Position pos)->double {
                if (!pos.IsValid())
                    throw FormulaError(FormulaError::Category::Ref);

                const auto* cell = sheet.GetCell(pos);
                if (cell) {
                    if (std::holds_alternative<double>(cell->GetValue())) {
                        return std::get<double>(cell->GetValue());
                    }
                    else if (std::holds_alternative<std::string>(cell->GetValue())) {
                        auto str_value = std::get<std::string>(cell->GetValue());
                        return ProcessTextCell(str_value);
                    }
                    else {
                        throw FormulaError(std::get<FormulaError>(cell->GetValue()));
                    }
                } else {
                    return 0.0;
                }
            };

            try {
                return ast_.Execute(eval);
            }
            catch (const FormulaError& fe) {
                return fe;
            }
        }


        [[nodiscard]] std::string GetExpression() const override {
            std::ostringstream out;
            ast_.PrintFormula(out);

            return out.str();
        }

        [[nodiscard]] std::vector<Position> GetReferencedCells() const override {
            std::vector<Position> cells;
            for (const auto& cell : ast_.GetCells()) {
                if (!cell.IsValid())
                    continue;

                cells.push_back(cell);
            }
            return cells;
        }

    private:
        FormulaAST ast_;
    };

}//end namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}