#pragma once

#include "cell.h"
#include "common.h"

#include <functional>

class Sheet : public SheetInterface {
public:
    ~Sheet() override;

    void SetCell(Position pos, std::string text) override;

    [[nodiscard]] const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    [[nodiscard]] Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

    void ThrowIfInvalidPosition(Position pos) const;

private:
    std::vector<std::vector<std::unique_ptr<Cell>>> cells_;
};