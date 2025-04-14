"""Rank a collection of rules by their value.

"""
import json
import os

from compute_rule_value import evaluate_ca_rule


def rank_rules_by_value(
        result_files: list[str],
        max_value: float | None = None) -> dict[str, float]:
    """Rank a collection of rules by the result value calculated from a simulation.

    This function talks about "value" but really uses the losses (smaller = better),
    since values are just the negatives of losses.

    Args:
        result_files: List of result files to process.
        max_value: If supplied, keep only rules with value <= this.

    Returns:
        rule_map (dict[str, float]): Map from rule files to values.

    """
    rule_value_pairs = []
    for file in result_files:
        value, _ = evaluate_ca_rule(file)
        value = -value  # Convert back to a loss for simplicity

        if max_value is not None and value <= max_value:
            # Making some major file system assumptions. Basically,
            # only works if you're using the default settings for
            # rule and result generation
            rule_file = file.replace('results', 'rules')
            rule_value_pairs.append((rule_file, value))
    rule_value_pairs = sorted(
        rule_value_pairs,
        key=lambda p: p[1])

    rule_map = {p[0]: p[1] for p in rule_value_pairs}
    return rule_map


if __name__ == '__main__':
    import sys
    result_dir = sys.argv[1]
    save_file = sys.argv[2]
    max_value = None
    if len(sys.argv) > 3:
        max_value = float(sys.argv[3])
    n_print = 0
    if len(sys.argv) > 4:
        n_print = int(sys.argv[4])

    result_files = [
        os.path.join(result_dir, f)
        for f in os.listdir(result_dir)
        if f.endswith('.json')
    ]

    rule_map = rank_rules_by_value(result_files, max_value)

    save_dir = os.path.dirname(save_file)
    os.makedirs(save_dir, exist_ok=True)
    with open(save_file, 'w') as fd:
        json.dump(rule_map, fd)

    if n_print > 0:
        kvs = list(rule_map.items())[:n_print]
        for kv in kvs:
            print(f'{kv[0]}: {kv[1]}')
