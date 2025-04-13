import random


def generate_rule(
        n_states: int,
        live_bias: float,
        small_count_bias: float,
        heuristic_max: float,
        dying_score: float = 0.1) -> list:
    """Generate a generalized CA rule with one dead state and an arbitrary number of dead or dying states.

    Args:
        n_states (int): Total number of states in the rule.
        p_first (float): Probability that a non-null first character is generated in every place.
        p_next (float): Probability that follow-on characters are generated.
        live_bias (float): The rules have at least one live state, then this is the probability that each subsequent
            remaining state is live as well. The rest are dying.
        heuristic_max (float): For a given rule set, define a heuristic H(rule) such that if rule[i] is the row in the
            rule table for state[i], H(rule) = sum_{i in [1, n_states]} sum_{j in [1, n_states]} sum_{c in rule[i][j]} 1/c^2 * S(j), where
            S(j) = 0 if state j is dead, 0.1 if state j is dying, and 1 if state j is alive.

            Examples:
                Game of life: 1/3^2 + (1/2^2 + 1/3^2) * 1 = 17/36
                Brians' brain: 1/3^2 + (1/2^2 + 1/3^2) * 1 + H(C) * 0.1
                That one rule set: 1/4^2 + (1/6^2 + 1/7^2 + 1/8^2 + 1/9^2) + H(A) * 0.1 + (1/7^2 + 1/8^2) + (1/3^2 + 1/4^2 + 1/5^2 + 1/6^2) * 0.1 + H(C) * 0.1
        dying_score (float):


    Returns:

    """