#include "formula.h"
#include "FormulaAST.h"
#include <sstream>

std::ostream& operator<<(std::ostream& output, const FormulaError& fe) {
    return output << "#ARITHM!";
}

namespace {

class Formula : public FormulaInterface {
public:
    explicit Formula(std::string expression)
        : ast_(ParseFormulaAST(expression)) {}

    Value Evaluate(const SheetInterface& sheet) const override {
        try {
            return ast_.Execute(sheet);
        } catch (const FormulaError& e) {
            return e;
        }
    }

    std::string GetExpression() const override {
        std::ostringstream out;
        ast_.PrintFormula(out);
        return out.str();
    }

    std::vector<Position> GetReferencedCells() const override {
        return ast_.GetReferencedCells();
    }

private:
    FormulaAST ast_;
};

}

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}

FormulaInterface::Value FormulaInterface::Evaluate() const {
    auto sheet = CreateSheet();
    return Evaluate(*sheet);
}