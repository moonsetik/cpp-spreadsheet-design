#include "cell.h"

#include <memory>
#include <string>
#include <variant>

using namespace std;

class Cell::Impl {
public:
    virtual ~Impl() = default;
    virtual Value GetValue() const = 0;
    virtual string GetText() const = 0;
};

class Cell::EmptyImpl : public Cell::Impl {
public:
    Value GetValue() const override {
        return string{};
    }
    string GetText() const override {
        return {};
    }
};

class Cell::TextImpl : public Cell::Impl {
public:
    explicit TextImpl(string text) : raw_text_(std::move(text)) {
        if (!raw_text_.empty() && raw_text_[0] == ESCAPE_SIGN) {
            if (raw_text_.size() == 1) {
                display_text_ = "";
            } else {
                display_text_ = raw_text_.substr(1);
            }
        } else {
            display_text_ = raw_text_;
        }
    }

    Value GetValue() const override {
        return display_text_;
    }

    string GetText() const override {
        return raw_text_;
    }

private:
    string raw_text_;
    string display_text_;
};

class Cell::FormulaImpl : public Cell::Impl {
public:
    explicit FormulaImpl(string text) : raw_text_(std::move(text)) {
        string expression = raw_text_.substr(1);
        formula_ = ParseFormula(std::move(expression));
    }

    Value GetValue() const override {
        auto result = formula_->Evaluate();
        if (std::holds_alternative<double>(result)) {
            return std::get<double>(result);
        } else {
            return std::get<FormulaError>(result);
        }
    }

    string GetText() const override {
        return "=" + formula_->GetExpression();
    }

private:
    string raw_text_;
    unique_ptr<FormulaInterface> formula_;
};

Cell::Cell() : impl_(make_unique<EmptyImpl>()) {}

Cell::~Cell() = default;

void Cell::Set(std::string text) {
    if (text.empty()) {
        impl_ = make_unique<EmptyImpl>();
    } else if (text[0] == ESCAPE_SIGN) {
        impl_ = make_unique<TextImpl>(std::move(text));
    } else if (text[0] == FORMULA_SIGN) {
        if (text.size() == 1) {
            impl_ = make_unique<TextImpl>(std::move(text));
        } else {
            impl_ = make_unique<FormulaImpl>(std::move(text));
        }
    } else {
        impl_ = make_unique<TextImpl>(std::move(text));
    }
}

void Cell::Clear() {
    impl_ = make_unique<EmptyImpl>();
}

CellInterface::Value Cell::GetValue() const {
    return impl_->GetValue();
}

string Cell::GetText() const {
    return impl_->GetText();
}