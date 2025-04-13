import numpy as np
import json


class CAValueFunction:
    """
    Value function for cellular automata rules based on population dynamics

    Computes a single score based on growth rate, periodicity, and complexity
    of the CA's population time series
    """

    def __init__(self,
                 transient_ratio=0.3,
                 growth_weight=0.4,
                 periodicity_weight=0.4,
                 complexity_weight=0.2,
                 ideal_growth_exp=1.0):
        """
        Initialize the value function with customizable weights

        Parameters:
        -----------
        transient_ratio : float
            Fraction of the time series to discard as initial transient (0.0 - 1.0)
        growth_weight : float
            Weight for growth rate loss term
        periodicity_weight : float
            Weight for periodicity loss term
        complexity_weight : float
            Weight for complexity loss term
        ideal_growth_exp : float
            Ideal growth exponent (0=constant, 1=linear, 2=quadratic)
        """
        self.transient_ratio = transient_ratio
        self.growth_weight = growth_weight
        self.periodicity_weight = periodicity_weight
        self.complexity_weight = complexity_weight
        self.ideal_growth_exp = ideal_growth_exp

    def evaluate(self, simulation_dict):
        """
        Evaluate a CA rule based on its simulation results

        Parameters:
        -----------
        simulation_dict : str
            Simulation results dict with 'populationRecord' and 'endStatus'

        Returns:
        --------
        float
            Value score for the rule (higher is better)
        dict
            Detailed breakdown of the score components
        """
        # Check for early termination conditions
        end_status = simulation_dict.get("endStatus", None)
        if end_status in ["explosion", "extinction", "flatline"]:
            return 0.0, {
                "reason": f"Early termination: {end_status}",
                "growth_loss": 1.0,
                "periodicity_loss": 1.0,
                "complexity_loss": 1.0,
                "total_loss": 1.0,
                "value": 0.0
            }

        # Extract population time series
        population_series = self._extract_population(simulation_dict)

        if population_series.size < 10:
            # Not enough data points for meaningful analysis
            return 0.0, {"reason": "Insufficient data points", "value": 0.0}

        # Remove initial transient behavior
        cutoff = int(population_series.size * self.transient_ratio)
        steady_population = population_series[cutoff:]

        if len(steady_population) < 5:
            # Not enough steady-state data for analysis
            return 0.0, {"reason": "Insufficient steady-state data", "value": 0.0}

        # Calculate individual loss terms
        growth_loss = self._growth_rate_loss(steady_population)
        periodicity_loss = self._periodicity_loss(steady_population)
        complexity_loss = self._complexity_loss(steady_population)

        # Compute weighted total loss
        total_loss = (
                self.growth_weight * growth_loss +
                self.periodicity_weight * periodicity_loss +
                self.complexity_weight * complexity_loss
        )

        # Convert loss to value (higher is better)
        v = 1.0 - total_loss

        # Ensure value is in [0, 1] range
        v = np.clip(v, 0, 1)

        # Return value and detailed breakdown
        return v, {
            "growth_loss": growth_loss,
            "periodicity_loss": periodicity_loss,
            "complexity_loss": complexity_loss,
            "total_loss": total_loss,
            "value": v
        }

    @staticmethod
    def _extract_population(simulation_json):
        """
        Extract population time series from simulation JSON

        Parameters:
        -----------
        simulation_json : dict
            Simulation results dictionary

        Returns:
        --------
        ndarray
            Array of population counts over time
        """
        population_record = simulation_json.get("populationRecord", {})

        # Convert string keys to integers and sort by time step
        time_steps = sorted([int(k) for k in population_record.keys()])

        # Extract population counts
        population = []
        for t in time_steps:
            t_str = str(t)
            if t_str in population_record:
                num_active = population_record[t_str].get("numActiveCubes", 0)
                population.append(num_active)

        return np.array(population)

    def _growth_rate_loss(self, population):
        """
        Calculate loss for growth rate

        Fits power law and computes distance from ideal growth

        Parameters:
        -----------
        population : ndarray
            Population time series

        Returns:
        --------
        float
            Growth rate loss term (0.0 - 1.0)
        """
        # Create time array
        time = np.arange(1, len(population) + 1)

        # Ensure all values are positive for log fitting
        pop_safe = np.maximum(population, 1e-10)

        try:
            # Fit power law using log-log linear regression
            log_time = np.log(time)
            log_pop = np.log(pop_safe)

            # Calculate slope (exponent b)
            slope = np.polyfit(log_time, log_pop, 1)

            # Convert to power law parameters
            exponent = slope

        except (RuntimeError, ValueError):
            # Fallback if curve fitting fails
            exponent = 0

        # Calculate distance from ideal growth exponent
        growth_error = abs(exponent - self.ideal_growth_exp)

        # Convert to loss (0 to 1 scale)
        max_error = 3.0  # Assuming difference of 3 in exponent is maximum error
        loss = np.minimum(growth_error / max_error, 1.0)

        return loss

    @staticmethod
    def _periodicity_loss(population):
        """
        Calculate loss for periodicity

        Uses P/E-inspired metric with log transformation

        Parameters:
        -----------
        population : ndarray
            Population time series

        Returns:
        --------
        float
            Periodicity loss term (0.0 - 1.0)
        """
        # Normalize the signal to zero mean
        normalized = population - np.mean(population)

        if np.linalg.norm(normalized) < 1e-10:
            # Flat signal, maximum periodicity
            return 1.0

        # Calculate FFT
        fft = np.abs(np.fft.rfft(normalized))

        # Find peak amplitude (excluding DC component)
        if len(fft) > 1:
            peak_amplitude = np.max(fft[1:])
        else:
            peak_amplitude = 0

        # Calculate signal norm
        signal_norm = np.linalg.norm(normalized)

        # Compute P/E ratio with log transformation
        if signal_norm > 0:
            periodicity_score = np.log1p(peak_amplitude / signal_norm)
        else:
            periodicity_score = 0

        # Cap the score at a reasonable maximum
        max_score = np.log1p(50)
        periodicity_score = np.minimum(periodicity_score, max_score)

        # Convert to loss (0 to 1 scale, where higher periodicity = higher loss)
        loss = periodicity_score / max_score

        return loss

    @staticmethod
    def _complexity_loss(population):
        """
        Calculate loss for complexity using total variation

        Lower values indicate more complex behavior

        Parameters:
        -----------
        population : ndarray
            Population time series

        Returns:
        --------
        float
            Complexity loss term (0.0 - 1.0)
        """
        # Normalize the signal
        normalized = population / (np.max(population) if np.max(population) > 0 else 1)

        # Calculate total variation
        tv = np.sum(np.abs(np.diff(normalized)))

        # Scale based on signal length
        scaled_tv = tv / (len(normalized) - 1)

        # Convert to complexity score (higher TV = higher complexity)
        # We cap the maximum TV at 1.0 (average absolute difference of 1.0)
        complexity_score = np.minimum(scaled_tv, 1.0)

        # Convert to loss (less complex = higher loss)
        loss = 1.0 - complexity_score

        return loss


def evaluate_ca_rule(simulation_json, **kwargs):
    """
    Convenience function to evaluate a CA rule

    Parameters:
    -----------
    simulation_json : dict or str
        Path to simulation results JSON
    **kwargs : dict
        Parameters to pass to CAValueFunction

    Returns:
    --------
    float
        Value score for the rule
    dict
        Detailed breakdown of the score components
    """
    with open(simulation_json, 'r') as fd:
        simulation_dict = json.load(fd)

    # Create value function with provided parameters
    value_function = CAValueFunction(**kwargs)

    # Evaluate rule
    return value_function.evaluate(simulation_dict)


if __name__ == "__main__":
    import sys
    example_json = sys.argv[1]

    # Evaluate with default parameters
    value, details = evaluate_ca_rule(example_json)

    print(f"Rule value: {value:.4f}")
    print("Breakdown:")
    for key, val in details.items():
        print(f"  {key}: {val}")
