//
// Created by matt on 4/12/25.
//
#include "Rule.h"

#include <fstream>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

Rule generateRule(
        const int n_dims,
        const int n_states,
        const double L_live,
        const double L_sparse,
        std::mt19937& rng)
{
    const int maxNbrs = (int)std::round(pow(3, n_dims) - 1);

    if (n_states < 3) throw std::invalid_argument("Need at least 3 states");

    std::exponential_distribution<double> expLive(L_live);
    std::exponential_distribution<double> expSparse(L_sparse);
    std::uniform_int_distribution<int>    uniNbr(0, maxNbrs);
    std::uniform_int_distribution<int>    coin(0, 1);

    /* -------- live‑state set ------------------------------------------ */
    int wantLive = std::clamp(1 + static_cast<int>(expLive(rng)),
                              1, n_states - 1);
    std::vector<int> pool(n_states - 1);
    std::iota(pool.begin(), pool.end(), 1);
    std::shuffle(pool.begin(), pool.end(), rng);
    std::set<int> liveStates(pool.begin(), pool.begin() + wantLive);

    /* -------- helper lambdas ------------------------------------------ */
    auto pickCounts = [&](int howMany,
                          std::vector<int>& unused) -> std::vector<int>
    {
        std::shuffle(unused.begin(), unused.end(), rng);
        std::vector<int> out(unused.begin(),
                             unused.begin() + howMany);
        unused.erase(unused.begin(), unused.begin() + howMany);
        return out;
    };

    auto toCSV = [](const std::vector<int>& v) -> std::string
    {
        std::string s;
        for (size_t i = 0; i < v.size(); ++i) {
            s += std::to_string(v[i]);
            if (i + 1 < v.size()) s += ',';
        }
        return s;
    };

    /* -------- build the rule table ------------------------------------ */
    std::vector<std::vector<std::string>> table(n_states,
                                                std::vector<std::string>(n_states, "-"));

    for (int cur = 0; cur < n_states; ++cur) {
        /* how many columns in this row will be non‑empty? */
        int nonEmpty = std::clamp(1 + static_cast<int>(expSparse(rng)),
                                  1, n_states);
        std::vector<int> targets(n_states);
        std::iota(targets.begin(), targets.end(), 0);
        std::shuffle(targets.begin(), targets.end(), rng);
        targets.resize(nonEmpty);

        std::vector<int> unused(maxNbrs + 1);
        std::iota(unused.begin(), unused.end(), 0);

        /* special case: exactly one non‑empty ⇒ just use “A” */
        if (nonEmpty == 1) {
            table[cur][targets[0]] = "A";
            continue;
        }

        /* otherwise, reserve last chosen column for “C” (50 % chance) or
           for the explicit remainder set                                              */
        for (size_t k = 0; k + 1 < targets.size(); ++k) {
            int want = std::clamp(1 + uniNbr(rng) % (maxNbrs / 2),
                                  1, static_cast<int>(unused.size()) - 1);
            table[cur][targets[k]] = toCSV(pickCounts(want, unused));
        }

        int last = targets.back();
        if (!unused.empty() && coin(rng)) {
            table[cur][last] = "C";        // complement of the previous union
        } else {
            table[cur][last] = unused.empty() ? "-" : toCSV(unused);
            unused.clear();
        }
    }

    return {std::move(table), std::move(liveStates)};
}


/**
 * Parse a JSON rule file and return a Rule struct
 *
 * @param filePath Path to the JSON rule file
 * @return Rule struct containing the parsed rule
 * @throws std::runtime_error if file cannot be opened or JSON is invalid
 */
Rule parseRuleFromJson(const std::string& filePath) {
    // Result rule
    Rule rule;

    try {
        // Open the file
        std::ifstream file(filePath);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open file: " + filePath);
        }

        // Parse JSON
        json jsonData;
        file >> jsonData;

        // Parse the table
        if (jsonData.contains("table") && jsonData["table"].is_array()) {
            auto& tableJson = jsonData["table"];
            rule.table.resize(tableJson.size());

            for (size_t i = 0; i < tableJson.size(); ++i) {
                auto& row = tableJson[i];
                if (!row.is_array()) {
                    throw std::runtime_error("Table row is not an array at index " + std::to_string(i));
                }

                rule.table[i].resize(row.size());
                for (size_t j = 0; j < row.size(); ++j) {
                    if (!row[j].is_string()) {
                        throw std::runtime_error("Table element is not a string at [" +
                                                 std::to_string(i) + "][" + std::to_string(j) + "]");
                    }
                    rule.table[i][j] = row[j];
                }
            }
        } else {
            throw std::runtime_error("JSON is missing 'table' array");
        }

        // Parse live states
        if (jsonData.contains("live_states") && jsonData["live_states"].is_array()) {
            auto& liveStatesJson = jsonData["live_states"];
            for (const auto& state : liveStatesJson) {
                if (!state.is_number_integer()) {
                    throw std::runtime_error("Live state is not an integer");
                }
                rule.liveStates.insert(state.get<int>());
            }
        } else {
            throw std::runtime_error("JSON is missing 'live_states' array");
        }

    } catch (const json::parse_error& e) {
        throw std::runtime_error("JSON parse error: " + std::string(e.what()));
    } catch (const std::exception& e) {
        throw std::runtime_error("Error parsing rule: " + std::string(e.what()));
    }

    return rule;
}