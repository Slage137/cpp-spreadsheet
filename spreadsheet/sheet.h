#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <vector>


class Sheet : public SheetInterface {
public:
    ~Sheet() override = default;

    void SetCell(Position pos, std::string text) override;

    CellInterface* GetCell(Position pos) override;
    [[nodiscard]] const CellInterface* GetCell(Position pos) const override;
    Cell* GetCellNotInterface(Position pos);
    [[nodiscard]] const Cell* GetCellNotInterface(Position pos) const;

    void ClearCell(Position pos) override;

    [[nodiscard]] Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

private:
    std::vector<std::vector<std::unique_ptr<Cell>>> cells_;

    void ThrowIfInvalidPosition(Position pos) const;
};