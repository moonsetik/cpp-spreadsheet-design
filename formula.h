#pragma once

#include "common.h"

#include "FormulaAST.h"

#include <memory>
#include <variant>

class FormulaInterface {
public:
    using Value = std::variant<double, FormulaError>;

    virtual ~FormulaInterface() = default;

    virtual Value Evaluate() const = 0;

    virtual std::string GetExpression() const = 0;
};

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression);