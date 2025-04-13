//
// Created by matt on 4/12/25.
//

#ifndef GOL3D_RULE_H
#define GOL3D_RULE_H
#pragma once

#include <algorithm>
#include <numeric>
#include <random>
#include <set>
#include <string>
#include <vector>

    struct Rule {
        std::vector<std::vector<std::string>> table; // [current][next]
        std::set<int> liveStates;                    // ⊆ {1 … N‑1}
    };

Rule generateRule(
        int n_dims,
        int n_states,
        double L_live,
        double L_sparse,
        std::mt19937& rng);

Rule parseRuleFromJson(const std::string& filePath);

#endif //GOL3D_RULE_H
