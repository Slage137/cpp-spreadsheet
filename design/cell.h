#pragma once

#include "common.h"
#include "formula.h"

#include <optional>

enum class CellType {
    Empty,
    Text,
    Formula,
    Error [[maybe_unused]] = -1
};

class Cell : public CellInterface {
    class Impl;

    std::unique_ptr<Impl> impl;
    SheetInterface* sheet{};

    std::set<CellInterface*> referenced_cells;  /// Зависимые ячейки

    mutable std::optional<Value> value; /// Кэш значения
public:
    Cell();
    ~Cell() override;

    void Set(std::string text);
    void Clear();

    [[nodiscard]] Value GetValue() const override;
    [[nodiscard]] std::string GetText() const override;

private:

    class Impl {
    public:
        virtual ~Impl() = default;
        [[nodiscard]] virtual CellType GetType() const = 0;
        [[nodiscard]] virtual std::string GetText() const = 0;
        [[nodiscard]] virtual Value GetValue() const = 0;
        [[nodiscard]] virtual std::vector<Position> GetReferencedCells() const = 0;
    };

    /// Классы-наследники Impl

    class EmptyImpl : public Impl {
    public:
        [[nodiscard]] CellType GetType() const override;
        [[nodiscard]] std::string GetText() const override;
        [[nodiscard]] Value GetValue() const override;
    };

    class TextImpl : public Impl {
        std::string text_;
    public:
        explicit TextImpl(std::string text);
        [[nodiscard]] CellType GetType() const override;
        [[nodiscard]] std::string GetText() const override;
        [[nodiscard]] Value GetValue() const override;
    };

    class FormulaImpl : public Impl {
        std::unique_ptr<FormulaInterface> formula;
    public:
        explicit FormulaImpl(std::string formula_text);
        [[nodiscard]] CellType GetType() const override;
        [[nodiscard]] std::string GetText() const override;
        [[nodiscard]] Value GetValue() const override;
        [[nodiscard]] std::vector<Position> GetReferencedCells() const override;
    };
};