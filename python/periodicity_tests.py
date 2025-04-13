import numpy as np
import matplotlib.pyplot as plt
from scipy import signal
import seaborn as sns

# Set plot style
sns.set(style="whitegrid")
plt.rcParams.update({'font.size': 12, 'figure.figsize': (14, 10)})


def generate_near_periodic_signal(length=1000, base_period=50, noise_level=0.1, num_components=3, seed=None):
    """
    Generate a nearly-periodic signal as a sum of sinusoids plus noise

    Parameters:
    -----------
    length : int
        Length of the signal
    base_period : int
        Base period for the main sinusoid
    noise_level : float
        Standard deviation of the Gaussian noise
    num_components : int
        Number of sinusoidal components to add
    seed : int
        Random seed for reproducibility

    Returns:
    --------
    signal : ndarray
        The generated signal
    """
    if seed is not None:
        np.random.seed(seed)

    t = np.arange(length)
    sig = np.zeros(length)

    # Add multiple sinusoidal components with decreasing amplitudes
    for i in range(num_components):
        period = base_period / (i + 1)  # Harmonics
        amplitude = 1.0 / (i + 1)       # Decreasing amplitude
        phase = np.random.uniform(0, 2*np.pi)
        sig += amplitude * np.sin(2 * np.pi * t / period + phase)

    # Normalize before adding noise
    sig = sig / np.max(np.abs(sig))

    # Add Gaussian noise
    noise = np.random.normal(0, noise_level, length)

    return sig + noise


def generate_periodic_random_walk(length=1000, base_period=50, drift_amplitude=0.1,
                                  num_components=3, walk_noise=0.02, seed=None):
    """
    Generate a random walk with periodically changing drift using a trig polynomial

    Parameters:
    -----------
    length : int
        Length of the signal
    base_period : int
        Base period for the main sinusoidal component of drift
    drift_amplitude : float
        Maximum amplitude of the periodic drift
    num_components : int
        Number of sinusoidal components to add in the drift function
    walk_noise : float
        Standard deviation of the random walk noise
    seed : int
        Random seed for reproducibility

    Returns:
    --------
    signal : ndarray
        The generated signal
    """
    if seed is not None:
        np.random.seed(seed)

    t = np.arange(length)

    # Create periodic drift function as a trig polynomial
    drift = np.zeros(length)
    for i in range(num_components):
        period = base_period / (i + 1)  # Harmonics
        amplitude = drift_amplitude / (i + 1)  # Decreasing amplitude
        phase = np.random.uniform(0, 2*np.pi)
        drift += amplitude * np.sin(2 * np.pi * t / period + phase)

    # Generate random walk increments
    increments = drift + np.random.normal(0, walk_noise, length)

    # Cumulative sum to get the random walk
    sig = np.cumsum(increments)

    # De-trend to keep bounded
    sig = sig - np.linspace(sig[0], sig[-1], length)

    # Normalize
    sig = sig / np.max(np.abs(sig))

    return sig


def analyze_periodicity(sig, title, max_period=200):
    """
    Analyze and plot periodicity metrics for the signal
    """
    # Create a figure with multiple subplots
    fig, axes = plt.subplots(3, 1, figsize=(14, 12))

    # Plot the signal itself
    axes[0].plot(sig, lw=1)
    axes[0].set_title(f'{title} - Time Series')
    axes[0].set_xlabel('Time')
    axes[0].set_ylabel('Value')

    # Compute and plot autocorrelation
    autocorr = compute_autocorrelation(sig, max_period)
    axes[1].plot(autocorr, lw=1)
    axes[1].set_title(f'{title} - Autocorrelation')
    axes[1].set_xlabel('Lag')
    axes[1].set_ylabel('Correlation')

    # Compute and plot FFT
    freqs, spectrum, dominant_period = compute_fft(sig)
    axes[2].plot(1/freqs[1:], spectrum[1:], lw=1)  # Skip the DC component
    axes[2].set_title(f'{title} - FFT Spectrum (Dominant Period: {dominant_period:.1f})')
    axes[2].set_xlabel('Period')
    axes[2].set_ylabel('Magnitude')
    axes[2].set_xlim(0, max_period)

    plt.tight_layout()
    return fig


def compute_autocorrelation(sig, max_lag=None):
    """Compute autocorrelation of the signal"""
    if max_lag is None:
        max_lag = len(sig) // 2

    # Normalize the signal
    sig = (sig - np.mean(sig)) / np.std(sig)

    # Compute autocorrelation
    autocorr = np.correlate(sig, sig, mode='full')
    autocorr = autocorr[autocorr.size//2:]  # Keep only positive lags
    autocorr = autocorr[:max_lag]  # Limit to max_lag

    # Normalize autocorrelation
    autocorr /= autocorr[0]

    return autocorr


def compute_fft(sig):
    """Compute FFT and find dominant period"""
    # Apply FFT
    fft = np.abs(np.fft.rfft(sig))
    freqs = np.fft.rfftfreq(len(sig))

    # Find dominant frequency (ignoring DC component)
    dominant_idx = np.argmax(fft[1:]) + 1
    dominant_freq = freqs[dominant_idx]
    dominant_period = 1 / dominant_freq if dominant_freq > 0 else 0

    return freqs, fft, dominant_period


def compare_periodicity_methods(signal1, signal2, title1, title2):
    """Compare multiple signals using different periodicity detection methods"""
    # Common parameters
    window_size = min(200, len(signal1) // 10)
    max_period = 200

    # Create autocorrelation plots
    fig, axes = plt.subplots(2, 2, figsize=(16, 12))

    # Standard autocorrelation for both signals
    autocorr1 = compute_autocorrelation(signal1, max_period)
    autocorr2 = compute_autocorrelation(signal2, max_period)

    axes[0, 0].plot(autocorr1, label=title1)
    axes[0, 0].plot(autocorr2, label=title2)
    axes[0, 0].set_title('Autocorrelation Comparison')
    axes[0, 0].set_xlabel('Lag')
    axes[0, 0].set_ylabel('Correlation')
    axes[0, 0].legend()

    # FFT for both signals (plotted by period rather than frequency)
    freqs1, spectrum1, period1 = compute_fft(signal1)
    freqs2, spectrum2, period2 = compute_fft(signal2)

    axes[0, 1].plot(1/freqs1[1:], spectrum1[1:], label=f"{title1} (Period: {period1:.1f})")
    axes[0, 1].plot(1/freqs2[1:], spectrum2[1:], label=f"{title2} (Period: {period2:.1f})")
    axes[0, 1].set_title('FFT Spectrum Comparison')
    axes[0, 1].set_xlabel('Period')
    axes[0, 1].set_ylabel('Magnitude')
    axes[0, 1].set_xlim(0, max_period)
    axes[0, 1].legend()

    # Advanced method: Welch's method (better for noisy signals)
    freqs1_welch, psd1 = signal.welch(signal1, nperseg=window_size)
    freqs2_welch, psd2 = signal.welch(signal2, nperseg=window_size)

    # Convert frequency to period for plotting
    with np.errstate(divide='ignore'):
        periods1_welch = 1 / freqs1_welch
        periods2_welch = 1 / freqs2_welch

    # Skip the DC component
    axes[1, 0].plot(periods1_welch[1:], psd1[1:], label=title1)
    axes[1, 0].plot(periods2_welch[1:], psd2[1:], label=title2)
    axes[1, 0].set_title("Welch's Method (Power Spectral Density)")
    axes[1, 0].set_xlabel('Period')
    axes[1, 0].set_ylabel('Power/Frequency')
    axes[1, 0].set_xlim(0, max_period)
    axes[1, 0].legend()

    # Wavelet analysis for time-frequency domain
    # Just show the original signals for comparison
    axes[1, 1].plot(signal1, alpha=0.7, label=title1)
    axes[1, 1].plot(signal2, alpha=0.7, label=title2)
    axes[1, 1].set_title('Original Signals')
    axes[1, 1].set_xlabel('Time')
    axes[1, 1].set_ylabel('Value')
    axes[1, 1].legend()

    plt.tight_layout()
    return fig


def enhanced_fft_analysis(sig, title, max_harmonics=5):
    """
    Perform enhanced FFT analysis to detect harmonic structure
    and distinguish complex from periodic signals
    """
    # Apply FFT
    fft = np.abs(np.fft.rfft(sig))
    freqs = np.fft.rfftfreq(len(sig))

    # Find dominant frequencies (peaks in the spectrum)
    # Skip first bin (DC component)
    peak_indices, _ = signal.find_peaks(fft[1:], height=0.1*np.max(fft[1:]))
    peak_indices = peak_indices + 1  # Adjust for skipped DC

    # Sort peaks by magnitude
    sorted_peaks = sorted(peak_indices, key=lambda i: fft[i], reverse=True)
    top_peaks = sorted_peaks[:min(max_harmonics, len(sorted_peaks))]

    # Calculate relationships between peak frequencies
    peak_freqs = freqs[top_peaks]
    peak_periods = 1.0 / peak_freqs

    # Compute ratio of max peak amplitude to total signal energy
    peak_energy_ratio = fft[top_peaks[0]] / fft[0]

    # Create figure
    fig, axes = plt.subplots(2, 1, figsize=(14, 10))

    # Plot signal
    axes[0].plot(sig, lw=1)
    axes[0].set_title(f'{title} - Time Series')
    axes[0].set_xlabel('Time')
    axes[0].set_ylabel('Value')

    # Plot FFT with identified peaks
    axes[1].plot(1/freqs[1:], fft[1:], lw=1)
    for i, peak_idx in enumerate(top_peaks):
        period = 1.0 / freqs[peak_idx]
        axes[1].axvline(x=period, color='r', linestyle='--', alpha=0.5)
        axes[1].text(period, fft[peak_idx], f'P{i+1}={period:.1f}',
                     verticalalignment='bottom', horizontalalignment='center')

    axes[1].set_title(f'{title} - FFT Spectrum with Top {len(top_peaks)} Peaks (P/E {peak_energy_ratio:.2f})')
    axes[1].set_xlabel('Period')
    axes[1].set_ylabel('Magnitude')
    axes[1].set_xlim(0, len(fft[1:]))  # max(200, 2*peak_periods[0]) if len(peak_periods) > 0 else 200)

    # Analyze harmonicity
    if len(peak_freqs) >= 2:
        # Check if frequencies are multiples of the fundamental
        fundamental = peak_freqs[0]
        ratios = peak_freqs / fundamental

        # Calculate how well these match integer ratios
        expected_ratios = np.round(ratios)
        ratio_errors = np.abs(ratios - expected_ratios)

        # A signal with strong harmonic structure will have small ratio errors
        is_harmonic = np.mean(ratio_errors[1:]) < 0.1

        harmonic_text = "Strong harmonic structure detected" if is_harmonic else \
            "Non-harmonic frequency components detected"

        fig.suptitle(f"{title}\n{harmonic_text}", fontsize=16)

        # Print ratios for inspection
        ratio_info = "\n".join([f"Peak {i+1}/{1}: {r:.3f} (Error: {e:.3f})"
                                for i, (r, e) in enumerate(zip(ratios[1:], ratio_errors[1:]))])
        props = dict(boxstyle='round', facecolor='wheat', alpha=0.2)
        axes[1].text(0.05, 0.95, ratio_info, transform=axes[1].transAxes,
                     fontsize=10, verticalalignment='top', bbox=props)

    plt.tight_layout()
    return fig


# Main function to run tests
def run_periodicity_tests():
    """Generate and analyze different types of signals"""
    # Set random seed for reproducibility
    seed = 42

    # Generate signals
    length = 1000

    # 1. Nearly periodic signal (trig polynomial + noise)
    near_periodic = generate_near_periodic_signal(
        length=length,
        base_period=50,
        noise_level=0.01,
        num_components=2,
        seed=seed
    )

    # 2. Random walk with periodic drift
    random_walk = generate_periodic_random_walk(
        length=length,
        base_period=50,
        drift_amplitude=0.1,
        num_components=2,
        walk_noise=0.01,
        seed=seed
    )

    # Analyze individual signals
    # fig1 = analyze_periodicity(near_periodic, "Nearly Periodic Signal")
    # fig2 = analyze_periodicity(random_walk, "Random Walk with Periodic Drift")
    #
    # # Compare signals with multiple methods
    # fig3 = compare_periodicity_methods(
    #     near_periodic,
    #     random_walk,
    #     "Nearly Periodic",
    #     "Random Walk"
    # )

    # Enhanced FFT analysis for each signal
    fig4 = enhanced_fft_analysis(near_periodic, "Nearly Periodic Signal")
    fig5 = enhanced_fft_analysis(random_walk, "Random Walk with Periodic Drift")

    # Create variants with higher complexity
    complex_periodic = generate_near_periodic_signal(
        length=length,
        base_period=500,
        noise_level=0.1,  # More noise
        num_components=10,  # More frequency components
        seed=seed
    )

    complex_walk = generate_periodic_random_walk(
        length=length,
        base_period=100,
        drift_amplitude=0.01,  # Subtle periodicity
        num_components=7,      # More components
        walk_noise=0.02,        # More random
        seed=seed
    )

    # Compare simple vs complex variants
    # fig6 = compare_periodicity_methods(
    #     near_periodic,
    #     complex_periodic,
    #     "Simple Periodic",
    #     "Complex Periodic"
    # )

    # fig7 = compare_periodicity_methods(
    #     random_walk,
    #     complex_walk,
    #     "Simple Random Walk",
    #     "Complex Random Walk"
    # )

    # Enhanced FFT analysis for complex signals
    fig8 = enhanced_fft_analysis(complex_periodic, "Complex Periodic Signal")
    fig9 = enhanced_fft_analysis(complex_walk, "Complex Random Walk")

    # Display all figures
    # for i, fig in enumerate([fig1, fig2, fig3, fig4, fig5, fig6, fig7, fig8, fig9]):
    for i, fig in enumerate([fig4, fig5, fig8, fig9]):
        plt.figure(fig.number)
        plt.tight_layout()
        plt.savefig(f"periodicity_analysis_{i+1}.png", dpi=150, bbox_inches='tight')

    plt.show()


if __name__ == "__main__":
    run_periodicity_tests()
