import json
import os

import matplotlib.pyplot as plt


def visualize_cube_stats(json_file: str, metrics_to_plot: list[str],
                         title: str = None,
                         save_plot: bool = False,
                         output_file: str = None) -> None:
    """Parse a JSON file containing cube state data and visualize specified metrics over time.

    Args:
        json_file: Path to the JSON file containing the cube state data.
        metrics_to_plot: List of metrics to plot. Valid options are 'numActiveCubes',
            'numLiveCubes', 'numDyingCubes'.
        title: Custom title for the plot. If None, a default title will be generated.
        save_plot: Whether to save the plot to a file. Defaults to False.
        output_file: Path where to save the plot. If None and save_plot is True,
            the plot will be saved next to the input JSON with a .png extension.

    Returns:
        None

    Raises:
        ValueError: If an invalid metric is provided or no population record is found.
        Exception: If there's an error loading the JSON file.
    """
    # Validate input metrics
    valid_metrics = ['numActiveCubes', 'numLiveCubes', 'numDyingCubes', 'numNonDeadCubes']
    for metric in metrics_to_plot:
        if metric not in valid_metrics:
            raise ValueError(f"Invalid metric: {metric}. Valid options are {valid_metrics}")

    # Load the JSON data
    try:
        with open(json_file, 'r') as f:
            data = json.load(f)
    except Exception as e:
        raise Exception(f"Error loading JSON file: {e}")

    # Extract rule string for the title
    rule_string = data.get('ruleString', 'Unknown Rule')

    # Parse the population record
    population_record = data.get('populationRecord', {})
    if not population_record:
        raise ValueError("No population record found in the JSON file")

    # Convert string keys to integers and sort by time step
    time_steps = sorted([int(ts) for ts in population_record.keys()])

    # Initialize data containers for each metric
    metrics_data = {metric: [] for metric in metrics_to_plot}

    # Extract data for each time step
    for ts in time_steps:
        ts_str = str(ts)
        for metric in metrics_to_plot:
            # Skip if the metric doesn't exist for this time step
            if metric in population_record[ts_str]:
                metrics_data[metric].append(population_record[ts_str][metric])
            else:
                metrics_data[metric].append(None)

    # Create the plot
    plt.figure(figsize=(12, 7))

    # Plot each metric
    for metric in metrics_to_plot:
        plt.plot(time_steps, metrics_data[metric], label=metric, linewidth=2, marker='o', markersize=4)

    # Add labels and title
    plt.xlabel('Time Step', fontsize=12)
    plt.ylabel('Count', fontsize=12)

    if title:
        plt.title(title, fontsize=14)
    else:
        plt.title(f'Cube States Over Time - Rule: {rule_string}', fontsize=14)

    plt.grid(True, linestyle='--', alpha=0.7)
    plt.legend(fontsize=12)

    # Add some padding to the x-axis
    plt.xlim(min(time_steps) - 1, max(time_steps) + 1)

    # Ensure y-axis starts at 0
    plt.ylim(bottom=0)

    # Tight layout for better spacing
    plt.tight_layout()

    # Save the plot if requested
    if save_plot:
        if not output_file:
            # Default output file is next to the input JSON
            base_name = os.path.splitext(json_file)[0]
            output_file = f"{base_name}_visualization.png"

        # Create directory if it doesn't exist
        os.makedirs(os.path.dirname(os.path.abspath(output_file)), exist_ok=True)
        plt.savefig(output_file, dpi=300, bbox_inches='tight')
        print(f"Plot saved to {output_file}")

    # Show the plot
    plt.show()


# Example usage
if __name__ == "__main__":
    import sys
    data_file = sys.argv[1]

    visualize_cube_stats(
        sys.argv[1],
        ["numActiveCubes", "numLiveCubes", "numDyingCubes", "numNonDeadCubes"],
        save_plot=False,
    )
