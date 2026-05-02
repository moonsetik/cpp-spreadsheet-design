#pragma once

#include "common.h"
#include "formula.h"
#include <memory>
#include <optional>
#include <vector>

class Sheet;

class Cell : public CellInterface {
public:
    Cell();
    ~Cell();

    void Set(std::string text) override;
    void Clear();

    void SetSheet(Sheet* sheet);
    void SetPosition(Position pos);

    void InvalidateCache();

    Value GetValue() const override;
    std::string GetText() const override;

    std::vector<Position> GetReferencedCells() const;

private:
    class Impl;
    class EmptyImpl;
    class TextImpl;
    class FormulaImpl;

    std::unique_ptr<Impl> impl_;
    Sheet* sheet_ = nullptr;
    Position pos_ = Position::NONE;
    mutable std::optional<Value> cached_value_;
    mutable bool dirty_ = true;
    std::vector<Position> outgoing_deps_;

    void UpdateDependencies();
    void ClearDependencies();
};