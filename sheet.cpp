#include "sheet.h"

#include "common.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <variant>

using namespace std;

Sheet::~Sheet() = default;

void Sheet::SetCell(Position pos, string text) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("invalid position");
    }
    if (text.empty()) {
        ClearCell(pos);
        return;
    }
    auto cell = make_unique<Cell>();
    cell->Set(move(text));
    cells_[pos] = move(cell);
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("invalid position");
    }
    auto it = cells_.find(pos);
    return it != cells_.end() ? it->second.get() : nullptr;
}

CellInterface* Sheet::GetCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("invalid position");
    }
    auto it = cells_.find(pos);
    return it != cells_.end() ? it->second.get() : nullptr;
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("invalid position");
    }
    cells_.erase(pos);
}

Size Sheet::GetPrintableSize() const {
    int max_row = 0;
    int max_col = 0;
    for (const auto& [pos, cell] : cells_) {
        if (cell) {
            max_row = max(max_row, pos.row + 1);
            max_col = max(max_col, pos.col + 1);
        }
    }
    return {max_row, max_col};
}

void Sheet::PrintValues(ostream& output) const {
    Size size = GetPrintableSize();
    for (int r = 0; r < size.rows; ++r) {
        for (int c = 0; c < size.cols; ++c) {
            if (c > 0) {
                output << '\t';
            }
            Position pos{r, c};
            const CellInterface* cell = GetCell(pos);
            if (cell) {
                auto value = cell->GetValue();
                visit([&output](const auto& v) { output << v; }, value);
            }
        }
        output << '\n';
    }
}

void Sheet::PrintTexts(ostream& output) const {
    Size size = GetPrintableSize();
    for (int r = 0; r < size.rows; ++r) {
        for (int c = 0; c < size.cols; ++c) {
            if (c > 0) {
                output << '\t';
            }
            Position pos{r, c};
            const CellInterface* cell = GetCell(pos);
            if (cell) {
                output << cell->GetText();
            }
        }
        output << '\n';
    }
}

unique_ptr<SheetInterface> CreateSheet() {
    return make_unique<Sheet>();
}