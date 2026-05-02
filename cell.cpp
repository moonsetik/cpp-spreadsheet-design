#include "cell.h"
#include "sheet.h"
#include "formula.h"
#include <sstream>

class Cell::Impl {
public:
    virtual ~Impl() = default;
    virtual Value GetValue(const SheetInterface& sheet) const = 0;
    virtual std::string GetText() const = 0;
    virtual std::vector<Position> GetReferencedCells() const { return {}; }
};

class Cell::EmptyImpl : public Cell::Impl {
public:
    Value GetValue(const SheetInterface&) const override { return std::string{}; }
    std::string GetText() const override { return {}; }
};

class Cell::TextImpl : public Cell::Impl {
public:
    explicit TextImpl(std::string text) : raw_text_(std::move(text)) {
        if (!raw_text_.empty() && raw_text_[0] == ESCAPE_SIGN) {
            display_text_ = (raw_text_.size() == 1) ? "" : raw_text_.substr(1);
        } else {
            display_text_ = raw_text_;
        }
    }
    Value GetValue(const SheetInterface&) const override { return display_text_; }
    std::string GetText() const override { return raw_text_; }
private:
    std::string raw_text_;
    std::string display_text_;
};

class Cell::FormulaImpl : public Cell::Impl {
public:
    explicit FormulaImpl(std::string text) : raw_text_(std::move(text)) {
        std::string expression = raw_text_.substr(1);
        formula_ = ParseFormula(std::move(expression));
    }
    Value GetValue(const SheetInterface& sheet) const override {
        auto result = formula_->Evaluate(sheet);
        if (std::holds_alternative<double>(result)) return std::get<double>(result);
        return std::get<FormulaError>(result);
    }
    std::string GetText() const override { return "=" + formula_->GetExpression(); }
    std::vector<Position> GetReferencedCells() const override { return formula_->GetReferencedCells(); }
private:
    std::string raw_text_;
    std::unique_ptr<FormulaInterface> formula_;
};

Cell::Cell() : impl_(std::make_unique<EmptyImpl>()) {}
Cell::~Cell() = default;

void Cell::Set(std::string text) {
    ClearDependencies();
    if (text.empty()) {
        impl_ = std::make_unique<EmptyImpl>();
    } else if (text[0] == ESCAPE_SIGN) {
        impl_ = std::make_unique<TextImpl>(std::move(text));
    } else if (text[0] == FORMULA_SIGN) {
        if (text.size() == 1) {
            impl_ = std::make_unique<TextImpl>(std::move(text));
        } else {
            auto temp_formula = ParseFormula(text.substr(1));
            auto refs = temp_formula->GetReferencedCells();
            if (sheet_ && sheet_->HasCycleAfterAdding(pos_, refs)) {
                throw CircularDependencyException("Circular dependency detected");
            }
            impl_ = std::make_unique<FormulaImpl>(std::move(text));
            UpdateDependencies();
        }
    } else {
        impl_ = std::make_unique<TextImpl>(std::move(text));
    }
    dirty_ = true;
    if (sheet_) {
        sheet_->InvalidateCell(pos_);
    }
}

void Cell::Clear() {
    ClearDependencies();
    impl_ = std::make_unique<EmptyImpl>();
    dirty_ = true;
    if (sheet_) {
        sheet_->InvalidateCell(pos_);
    }
}

void Cell::SetSheet(Sheet* sheet) {
    sheet_ = sheet;
}

void Cell::SetPosition(Position pos) {
    pos_ = pos;
}

void Cell::InvalidateCache() {
    dirty_ = true;
}

CellInterface::Value Cell::GetValue() const {
    if (dirty_) {
        if (!sheet_) return FormulaError("NO_SHEET");
        cached_value_ = impl_->GetValue(*sheet_);
        dirty_ = false;
    }
    return *cached_value_;
}

std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}

void Cell::UpdateDependencies() {
    if (!sheet_ || pos_ == Position::NONE) return;
    auto refs = impl_->GetReferencedCells();
    outgoing_deps_ = refs;
    for (const auto& dep_pos : refs) {
        sheet_->AddDependency(pos_, dep_pos);
    }
}

void Cell::ClearDependencies() {
    if (!sheet_) return;
    for (const auto& dep_pos : outgoing_deps_) {
        sheet_->RemoveDependency(pos_, dep_pos);
    }
    outgoing_deps_.clear();
}