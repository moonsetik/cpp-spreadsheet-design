#include "sheet.h"
#include "formula.h"
#include <algorithm>
#include <functional>
#include <sstream>
#include <variant>

Sheet::~Sheet() = default;

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) throw InvalidPositionException("Invalid position");

    if (text.empty()) {
        ClearCell(pos);
        return;
    }

    auto& cell_ptr = cells_[pos];
    if (!cell_ptr) {
        cell_ptr = std::make_unique<Cell>();
    }
    cell_ptr->SetSheet(this);
    cell_ptr->SetPosition(pos);
    cell_ptr->Set(std::move(text));
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (!pos.IsValid()) throw InvalidPositionException("Invalid position");
    auto it = cells_.find(pos);
    return it != cells_.end() ? it->second.get() : nullptr;
}

CellInterface* Sheet::GetCell(Position pos) {
    if (!pos.IsValid()) throw InvalidPositionException("Invalid position");
    auto it = cells_.find(pos);
    return it != cells_.end() ? it->second.get() : nullptr;
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) throw InvalidPositionException("Invalid position");
    auto it = cells_.find(pos);
    if (it != cells_.end()) {
        it->second->Clear();
        ClearIncomingDependencies(pos);
        cells_.erase(it);
    }
}

Size Sheet::GetPrintableSize() const {
    int max_row = -1, max_col = -1;
    for (const auto& [pos, cell] : cells_) {
        if (cell) {
            max_row = std::max(max_row, pos.row);
            max_col = std::max(max_col, pos.col);
        }
    }
    return {max_row + 1, max_col + 1};
}

void Sheet::PrintValues(std::ostream& output) const {
    Size size = GetPrintableSize();
    for (int r = 0; r < size.rows; ++r) {
        for (int c = 0; c < size.cols; ++c) {
            if (c > 0) output << '\t';
            Position pos{r, c};
            auto it = cells_.find(pos);
            if (it != cells_.end() && it->second) {
                auto val = it->second->GetValue();
                std::visit([&output](const auto& v) { output << v; }, val);
            }
        }
        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    Size size = GetPrintableSize();
    for (int r = 0; r < size.rows; ++r) {
        for (int c = 0; c < size.cols; ++c) {
            if (c > 0) output << '\t';
            Position pos{r, c};
            auto it = cells_.find(pos);
            if (it != cells_.end() && it->second) output << it->second->GetText();
        }
        output << '\n';
    }
}

void Sheet::AddDependency(Position dependent, Position dependency) {
    dependencies_[dependent].push_back(dependency);
    dependents_[dependency].push_back(dependent);
}

void Sheet::RemoveDependency(Position dependent, Position dependency) {
    auto dep_it = dependencies_.find(dependent);
    if (dep_it != dependencies_.end()) {
        auto& vec = dep_it->second;
        vec.erase(std::remove(vec.begin(), vec.end(), dependency), vec.end());
        if (vec.empty()) {
            dependencies_.erase(dep_it);
        }
    }
    auto dep_it2 = dependents_.find(dependency);
    if (dep_it2 != dependents_.end()) {
        auto& vec = dep_it2->second;
        vec.erase(std::remove(vec.begin(), vec.end(), dependent), vec.end());
        if (vec.empty()) {
            dependents_.erase(dep_it2);
        }
    }
}

void Sheet::InvalidateCell(Position pos) {
    std::unordered_set<Position, PositionHasher> visited;
    InvalidateCellDFS(pos, visited);
}

void Sheet::InvalidateCellDFS(Position pos, std::unordered_set<Position, PositionHasher>& visited) {
    if (visited.count(pos)) return;
    visited.insert(pos);

    auto cell_it = cells_.find(pos);
    if (cell_it != cells_.end() && cell_it->second) {
        cell_it->second->InvalidateCache();
    }

    auto dep_it = dependents_.find(pos);
    if (dep_it != dependents_.end()) {
        for (const auto& dependent : dep_it->second) {
            InvalidateCellDFS(dependent, visited);
        }
    }
}

void Sheet::ClearIncomingDependencies(Position pos) {
    auto dep_it = dependents_.find(pos);
    if (dep_it != dependents_.end()) {
        for (const auto& dependent : dep_it->second) {
            auto it = dependencies_.find(dependent);
            if (it != dependencies_.end()) {
                auto& vec = it->second;
                vec.erase(std::remove(vec.begin(), vec.end(), pos), vec.end());
                if (vec.empty()) {
                    dependencies_.erase(it);
                }
            }
        }
        dependents_.erase(dep_it);
    }
}

bool Sheet::HasCycleAfterAdding(Position target, const std::vector<Position>& refs) const {
    for (const auto& ref : refs) {
        std::unordered_set<Position, PositionHasher> visited;
        std::function<bool(Position)> dfs = [&](Position cur) -> bool {
            if (cur == target) return true;
            if (visited.count(cur)) return false;
            visited.insert(cur);
            auto it = dependencies_.find(cur);
            if (it != dependencies_.end()) {
                for (const auto& dep : it->second) {
                    if (dfs(dep)) return true;
                }
            }
            return false;
        };
        if (dfs(ref)) return true;
    }
    return false;
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}