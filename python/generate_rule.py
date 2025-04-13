# rule_generator_v2.py
import argparse
import json
import os
import random
import time

from dataclasses import dataclass
from typing import Any, Literal

import tqdm

ROOT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


StateType = Literal["dead", "live", "dying"]


@dataclass
class Rule:
    table: list[list[str]]  # [current_state][next_state] -> CSV / "A" / "C" / "-"
    live_states: set[int]   # bold (neighbour‑counting) states
    params: dict[str, Any]

    def to_json(self, filename: str) -> None:
        """Convert the rule to JSON format and save it to a file.

        Args:
            filename: Path to save the JSON file
        """
        # Create a JSON-serializable dictionary
        data = {
            "table": self.table,
            "live_states": list(self.live_states),
            "params": self.params,
        }

        # Write to file
        os.makedirs(os.path.dirname(os.path.abspath(filename)), exist_ok=True)
        with open(filename, 'w') as f:
            json.dump(data, f, indent=2)


def clamp(x: int, lo: int, hi: int) -> int:
    return max(lo, min(x, hi))


def get_state_type(state: int, live_states: set[int]) -> StateType:
    """Determine the type of a state: dead, live, or dying."""
    if state == 0:
        return "dead"
    elif state in live_states:
        return "live"
    else:
        return "dying"


def generate_rule(
        n_dims: int,
        n_states: int,
        beta_live: float,
        beta_sparse: dict[StateType, float],
        beta_want: dict[StateType, float],
        beta_unused: dict[StateType, float],
        rng: random.Random,
) -> Rule:
    """Produce a random `n_states`‑state `n_dims`‑D Moore‑neighbour rule.

    Args:
        n_dims: number of spatial dimensions
        n_states: total number of states (>= 3, state 0 is always dead)
        beta_live: bias param for live states (>1 for bias toward fewer live states)
        beta_sparse: bias param for non-empty targets by state type (>1 for bias toward fewer targets)
        beta_want: bias param for neighbor counts by state type (>1 for bias toward fewer counts)
        beta_unused: bias param for selecting smaller `unused` values by state type (>1 for stronger bias)
        rng: optional random.Random for determinism

    Returns:
        rule (Rule): A randomly-generated rule
    """
    if n_states < 3:
        raise ValueError("Need at least three states")

    max_nbrs = int(3**n_dims - 1)

    params = {
        'beta_live': beta_live,
        'beta_sparse': beta_sparse,
        'beta_want': beta_want,
        'beta_unused': beta_unused,
    }

    # ---------- live‑state set -------------------------------------------------
    # Scale beta distribution to get number of live states between 1 and n_states-1
    want_live = clamp(1 + int(rng.betavariate(1, beta_live) * (n_states - 1)), 1, n_states - 1)
    live_pool = list(range(1, n_states))
    rng.shuffle(live_pool)
    live_states = set(live_pool[:want_live])

    # ---------- build the rule table ------------------------------------------
    table = [["-"] * n_states for _ in range(n_states)]

    # Special case: death to death is always 'C'
    table[0][0] = "C"

    for cur in range(n_states):
        state_type = get_state_type(cur, live_states)

        # Skip row 0, col 0 as it's already set to 'C'
        if cur == 0:
            # For death state (row 0), start columns from 1
            start_col = 1
        else:
            start_col = 0

        # Create target columns pool (excluding already handled cells)
        targets_pool = list(range(start_col, n_states))

        # Scale beta distribution to get number of non-empty targets
        max_non_empty = len(targets_pool)
        non_empty = clamp(1 + int(rng.betavariate(1, beta_sparse[state_type]) * (max_non_empty - 1)), 1, max_non_empty)

        rng.shuffle(targets_pool)
        targets = targets_pool[:non_empty]

        # For death state (row 0), unused should exclude 0 to avoid 0 in transition rules
        if cur == 0:
            unused = list(range(1, max_nbrs + 1))  # 1 … 26 for death state
        else:
            unused = list(range(max_nbrs + 1))     # 0 … 26 for other states

        # Determine how many elements to assign to each column (except last)
        if cur == 0:
            columns_to_fill = targets[:]
        else:
            columns_to_fill = targets[:-1]
        column_counts = {}

        unused_left = len(unused)
        active_columns = []

        for tgt in columns_to_fill:
            if unused_left > 0:
                # Scale beta distribution to get number of elements for this column
                want = clamp(1 + int(rng.betavariate(1, beta_want[state_type]) * unused_left), 1, unused_left)
                column_counts[tgt] = want
                unused_left -= want
                active_columns.append(tgt)

        # Allocate unused values to columns in round-robin fashion with bias
        column_contents = {tgt: [] for tgt in active_columns}

        while active_columns and unused:
            for tgt in list(active_columns):  # Use a copy for safe removal
                if not unused:
                    break

                # Use beta distribution to bias toward smaller indices
                # Beta(1, beta_unused) gives values in [0,1] biased toward 0 as beta_unused increases
                index = int(rng.betavariate(1, beta_unused[state_type]) * len(unused))
                value = unused.pop(index)

                column_contents[tgt].append(value)

                # Remove this column if it has reached its allocation
                if len(column_contents[tgt]) >= column_counts[tgt]:
                    active_columns.remove(tgt)

        # Fill the table with the selected values for each column
        for tgt, values in column_contents.items():
            table[cur][tgt] = ",".join(map(str, sorted(values)))

        # Handle the last column: "C" if anything is left, else "‑"
        # For row 0, a "C" has already been placed
        if cur > 0:
            if unused:
                table[cur][targets[-1]] = "C"
            else:
                table[cur][targets[-1]] = "-"

    return Rule(table, live_states, params)


def parse_dict_param(param_str: str) -> dict[StateType, float]:
    """Parse a comma-separated string of key:value pairs into a dictionary."""
    result = {}
    pairs = param_str.split(',')
    for pair in pairs:
        key, value = pair.split(':')
        key = key.strip()
        if key not in ["dead", "live", "dying"]:
            raise ValueError(f"Invalid state type: {key}. Must be 'dead', 'live', or 'dying'")
        result[key] = float(value.strip())
    return result


if __name__ == "__main__":
    # Set up command line argument parser
    parser = argparse.ArgumentParser(description="Generate a random cellular automaton rule")

    parser.add_argument("--n_dims", type=int, default=3,
                        help="Number of spatial dimensions (default: 3)")
    parser.add_argument("--n_states", type=int, default=5,
                        help="Number of states (>= 3, state 0 is always dead) (default: 5)")
    parser.add_argument("--beta_live", type=float, default=2.0,
                        help="Bias parameter for live states; higher values mean fewer live states (default: 2.0)")
    parser.add_argument("--beta_sparse", type=str, default="dead:4,live:2,dying:1.5",
                        help="Bias parameters for non-empty targets by state type (default: dead:1.2,live:1.5,dying:2.0)")
    parser.add_argument("--beta_want", type=str, default="dead:6.0,live:5.0,dying:2.5",
                        help="Bias parameters for neighbor counts by state type (default: dead:2.0,live:3.0,dying:1.5)")
    parser.add_argument("--beta_unused", type=str, default="dead:4.0,live:2.0,dying:1.2",
                        help="Bias parameters for selecting smaller values by state type (default: dead:3.0,live:2.0,dying:1.5)")
    parser.add_argument("--seed", type=int, default=None,
                        help="Random seed for reproducibility (default: None, uses system time)")
    parser.add_argument("--n_rules", type=int, default=1,
                        help="Number of rules to generate (default: 1)")
    parser.add_argument("--beta_noise", type=float, default=0,
                        help="Gaussian noise std to apply multiplicatively to beta values")
    args = parser.parse_args()

    # Parse dictionary parameters
    original_beta_sparse = parse_dict_param(args.beta_sparse)
    original_beta_want = parse_dict_param(args.beta_want)
    original_beta_unused = parse_dict_param(args.beta_unused)
    originals = [original_beta_sparse, original_beta_want, original_beta_unused]

    for n in tqdm.tqdm(range(args.n_rules), total=args.n_rules):
        if args.seed is None:
            seed = args.seed
        else:
            seed = args.seed + n

        rng = random.Random(args.seed)

        today = time.strftime('%Y-%m-%d')
        rules_dir = f'{ROOT_DIR}/output/rules/{today}'
        os.makedirs(rules_dir, exist_ok=True)
        n_rules = len([f for f in os.listdir(rules_dir) if f.endswith('.json')])
        save_file = f'{rules_dir}/{n_rules:03}.json'

        if args.beta_noise == 0:
            beta_live = args.beta_live
            beta_sparse = original_beta_sparse
            beta_want = original_beta_want
            beta_unused = original_beta_unused
        else:
            beta_live = max(1, args.beta_live * random.gauss(mu=1, sigma=args.beta_noise))
            beta_sparse = {}
            beta_want = {}
            beta_unused = {}
            for final, original in zip([beta_sparse, beta_want, beta_unused], originals):
                for k, v in original.items():
                    final[k] = max(1, v * random.gauss(mu=1, sigma=args.beta_noise))

        # Generate the rule
        rule = generate_rule(
            n_dims=args.n_dims,
            n_states=args.n_states,
            beta_live=beta_live,
            beta_sparse=beta_sparse,
            beta_want=beta_want,
            beta_unused=beta_unused,
            rng=rng,
        )

        # Save to JSON file
        rule.to_json(save_file)
