#pragma once

#include "common.h"
#include <memory>
#include <variant>
#include <vector>

class SheetInterface;

class FormulaInterface {
public:
    using Value = std::variant<double, FormulaError>;

    virtual ~FormulaInterface() = default;

    virtual Value Evaluate(const SheetInterface& sheet) const = 0;

    Value Evaluate() const;

    virtual std::string GetExpression() const = 0;

    virtual std::vector<Position> GetReferencedCells() const { return {}; }
};

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression);