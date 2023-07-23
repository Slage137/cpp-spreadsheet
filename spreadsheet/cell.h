#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <unordered_set>
#include <stack>
#include <set>

class Sheet;

enum class CellType
{
    EMPTY,    // default type on cell creation
    TEXT,
    FORMULA
};

class Cell : public CellInterface {
    class Impl;
public:
    explicit Cell(Sheet& sheet);
    ~Cell() override = default;

    void Set(std::string text);
    void Clear();

    [[nodiscard]] Value GetValue() const override;
    [[nodiscard]] std::string GetText() const override;
    [[nodiscard]] std::vector<Position> GetReferencedCells() const override;
    [[nodiscard]] bool IsReferenced() const;

private:
    /// Поля класса
    std::unique_ptr<Impl> impl_;
    Sheet& sheet_;

    std::set<Cell*> dependent_cells_;
    std::set<Cell*> referenced_cells_;

    /// Вспомогательные методы
    std::unique_ptr<Impl> CreateImplFromText(std::string text);
    bool HasCircularDependency(Impl* temp_impl);
    void UpdateDependencies(std::unique_ptr<Impl> new_impl);

    /// Вспомогательные классы
    class Impl {
    public:
        [[maybe_unused]] [[nodiscard]] virtual CellType GetType() const = 0;
        [[nodiscard]] virtual Value GetValue() const = 0;
        [[nodiscard]] virtual std::string GetText() const = 0;
        [[nodiscard]] virtual std::vector<Position> GetReferencedCells() const = 0;

        virtual void InvalidateCache() = 0;

        virtual ~Impl() = default;
    };

    class EmptyImpl : public Impl {
    public:
        [[nodiscard]] CellType GetType() const override { return CellType::EMPTY; }
        [[nodiscard]] Value GetValue() const override;
        [[nodiscard]] std::string GetText() const override;
        [[nodiscard]] std::vector<Position> GetReferencedCells() const override { return {}; }
        void InvalidateCache() override {}    /// поддержка интерфейса
    };

    class TextImpl : public Impl {
    public:
        explicit TextImpl(std::string text);
        [[nodiscard]] CellType GetType() const override { return CellType::TEXT; }
        [[nodiscard]] Value GetValue() const override;
        [[nodiscard]] std::string GetText() const override;
        [[nodiscard]] std::vector<Position> GetReferencedCells() const override { return {}; }
        void InvalidateCache() override {}    /// поддержка интерфейса
    private:
        std::string text_;
    };

    class FormulaImpl : public Impl {
    public:

        explicit FormulaImpl(std::string text, SheetInterface& sheet);
        [[nodiscard]] CellType GetType() const override { return CellType::FORMULA; }
        Value GetValue() const override;
        std::string GetText() const override;
        std::vector<Position> GetReferencedCells() const override;

        void InvalidateCache() override;

    private:
        mutable std::optional<FormulaInterface::Value> cache_;
        std::unique_ptr<FormulaInterface> formula_ptr_;
        SheetInterface& sheet_;
    };
};