#!/usr/bin/env python3
"""
Bootstrap analysis for Lambda feeddown ratio calculation.
This script properly accounts for correlation between numerator and denominator
by resampling events and recalculating the ratio for each bootstrap sample.
"""

import ROOT
import numpy as np
import argparse
import sys
from array import array

def load_event_data(root_file, centrality_bin=None):
    """
    Load event-by-event correlation data from ROOT file.

    Args:
        root_file: ROOT TFile object
        centrality_bin: If specified, filter events by centrality (0-9)

    Returns:
        Dictionary with arrays of event data
    """
    tree = root_file.Get("EventData")
    if not tree:
        print("Error: EventData tree not found in ROOT file", file=sys.stderr)
        return None

    # Read all events into numpy arrays
    n_events = tree.GetEntries()

    centrality = []
    delta_lambda_lambda = []
    delta_pFromLambda = []
    delta_piFromLambda = []
    npairs_lambda_lambda = []
    npairs_pFromLambda = []
    npairs_piFromLambda = []

    for i in range(n_events):
        tree.GetEntry(i)
        cent = tree.centrality

        # Filter by centrality if requested
        if centrality_bin is not None and cent != centrality_bin:
            continue

        # Only include events with valid data
        if tree.npairs_lambda_lambda > 0:
            centrality.append(cent)
            delta_lambda_lambda.append(tree.delta_lambda_lambda)
            delta_pFromLambda.append(tree.delta_pFromLambda)
            delta_piFromLambda.append(tree.delta_piFromLambda)
            npairs_lambda_lambda.append(tree.npairs_lambda_lambda)
            npairs_pFromLambda.append(tree.npairs_pFromLambda)
            npairs_piFromLambda.append(tree.npairs_piFromLambda)

    return {
        'centrality': np.array(centrality),
        'delta_lambda_lambda': np.array(delta_lambda_lambda),
        'delta_pFromLambda': np.array(delta_pFromLambda),
        'delta_piFromLambda': np.array(delta_piFromLambda),
        'npairs_lambda_lambda': np.array(npairs_lambda_lambda),
        'npairs_pFromLambda': np.array(npairs_pFromLambda),
        'npairs_piFromLambda': np.array(npairs_piFromLambda),
    }

def bootstrap_ratio(data, n_bootstrap=1000, correlation_type='pFromLambda'):
    """
    Calculate ratio and uncertainty using bootstrap resampling.

    Args:
        data: Dictionary of event data from load_event_data
        n_bootstrap: Number of bootstrap samples
        correlation_type: 'pFromLambda' or 'piFromLambda'

    Returns:
        tuple: (ratio, ratio_error, bootstrap_distribution)
    """
    n_events = len(data['centrality'])

    if n_events == 0:
        return 0, 0, np.array([])

    # Get the numerator and denominator arrays
    numerator = data[f'delta_{correlation_type}']
    denominator = data['delta_lambda_lambda']
    npairs_num = data[f'npairs_{correlation_type}']
    npairs_denom = data['npairs_lambda_lambda']

    # Calculate original ratio (weighted average)
    # Use npairs as weights for more accurate averaging
    orig_num = np.sum(numerator * npairs_num) / np.sum(npairs_num)
    orig_denom = np.sum(denominator * npairs_denom) / np.sum(npairs_denom)
    orig_ratio = orig_num / orig_denom if orig_denom != 0 else 0

    # Bootstrap resampling
    bootstrap_ratios = np.zeros(n_bootstrap)

    rng = np.random.RandomState(42)  # For reproducibility

    for b in range(n_bootstrap):
        # Resample events with replacement
        indices = rng.choice(n_events, size=n_events, replace=True)

        # Calculate ratio for this bootstrap sample
        boot_numerator = numerator[indices]
        boot_denominator = denominator[indices]
        boot_npairs_num = npairs_num[indices]
        boot_npairs_denom = npairs_denom[indices]

        # Weighted average for this bootstrap sample
        boot_num = np.sum(boot_numerator * boot_npairs_num) / np.sum(boot_npairs_num)
        boot_denom = np.sum(boot_denominator * boot_npairs_denom) / np.sum(boot_npairs_denom)

        bootstrap_ratios[b] = boot_num / boot_denom if boot_denom != 0 else 0

    # Calculate uncertainty as standard deviation of bootstrap distribution
    ratio_error = np.std(bootstrap_ratios, ddof=1)

    return orig_ratio, ratio_error, bootstrap_ratios

def analyze_centrality_bin(data, cent_bin, n_bootstrap=1000):
    """
    Analyze a specific centrality bin.

    Returns:
        Dictionary with results for both p and pi from Lambda
    """
    # Filter data for this centrality bin
    mask = data['centrality'] == cent_bin
    bin_data = {key: val[mask] for key, val in data.items()}

    n_events = np.sum(mask)

    if n_events < 10:  # Need minimum number of events
        print(f"Warning: Only {n_events} events in centrality bin {cent_bin}, skipping")
        return None

    print(f"\nCentrality bin {cent_bin} ({cent_bin*10}-{(cent_bin+1)*10}%): {n_events} events")

    # Calculate ratios with bootstrap errors
    p_ratio, p_error, p_dist = bootstrap_ratio(bin_data, n_bootstrap, 'pFromLambda')
    pi_ratio, pi_error, pi_dist = bootstrap_ratio(bin_data, n_bootstrap, 'piFromLambda')

    print(f"  p from Lambda / Lambda-Lambda: {p_ratio:.6f} ± {p_error:.6f}")
    print(f"  π from Lambda / Lambda-Lambda: {pi_ratio:.6f} ± {pi_error:.6f}")

    return {
        'centrality': cent_bin * 10 + 5,  # Center of bin
        'p_ratio': p_ratio,
        'p_error': p_error,
        'p_distribution': p_dist,
        'pi_ratio': pi_ratio,
        'pi_error': pi_error,
        'pi_distribution': pi_dist,
        'n_events': n_events
    }

def create_graphs(results):
    """
    Create TGraphErrors from bootstrap results.

    Returns:
        tuple: (p_graph, pi_graph)
    """
    p_graph = ROOT.TGraphErrors()
    pi_graph = ROOT.TGraphErrors()

    p_graph.SetName("p_from_lambda_ratio_bootstrap")
    pi_graph.SetName("pi_from_lambda_ratio_bootstrap")

    for res in results:
        if res is None:
            continue

        cent = res['centrality']

        # p from Lambda ratio
        n = p_graph.GetN()
        p_graph.SetPoint(n, cent, res['p_ratio'])
        p_graph.SetPointError(n, 0, res['p_error'])

        # pi from Lambda ratio
        n = pi_graph.GetN()
        pi_graph.SetPoint(n, cent, res['pi_ratio'])
        pi_graph.SetPointError(n, 0, res['pi_error'])

    return p_graph, pi_graph

def main():
    parser = argparse.ArgumentParser(
        description='Bootstrap analysis for Lambda feeddown ratios')
    parser.add_argument('input', help='Input ROOT file (output from analysis_cve)')
    parser.add_argument('--output', default='bootstrap_ratios.root',
                       help='Output ROOT file')
    parser.add_argument('--n-bootstrap', type=int, default=1000,
                       help='Number of bootstrap samples (default: 1000)')
    parser.add_argument('--max-centrality', type=int, default=5,
                       help='Maximum centrality bin to analyze (0-9, default: 5 = 0-50%%)')
    args = parser.parse_args()

    # Open input file
    f = ROOT.TFile.Open(args.input)
    if not f or f.IsZombie():
        print(f"Error: Cannot open file: {args.input}", file=sys.stderr)
        sys.exit(1)

    # Load all event data
    print("Loading event-by-event data...")
    data = load_event_data(f)

    if data is None or len(data['centrality']) == 0:
        print("Error: No valid event data found", file=sys.stderr)
        sys.exit(1)

    print(f"Loaded {len(data['centrality'])} events")

    # Analyze each centrality bin
    results = []
    for cent_bin in range(args.max_centrality + 1):
        result = analyze_centrality_bin(data, cent_bin, args.n_bootstrap)
        results.append(result)

    # Create output graphs
    p_graph, pi_graph = create_graphs(results)

    # Save to output file
    fout = ROOT.TFile(args.output, "RECREATE")
    p_graph.Write()
    pi_graph.Write()

    # Also save bootstrap distributions as histograms for verification
    for i, res in enumerate(results):
        if res is None:
            continue

        cent = int(res['centrality'])

        h_p = ROOT.TH1D(f"h_bootstrap_p_cent{cent}",
                       f"Bootstrap dist. p ratio, cent {cent}%",
                       50, res['p_ratio'] - 5*res['p_error'],
                       res['p_ratio'] + 5*res['p_error'])
        for val in res['p_distribution']:
            h_p.Fill(val)
        h_p.Write()

        h_pi = ROOT.TH1D(f"h_bootstrap_pi_cent{cent}",
                        f"Bootstrap dist. π ratio, cent {cent}%",
                        50, res['pi_ratio'] - 5*res['pi_error'],
                        res['pi_ratio'] + 5*res['pi_error'])
        for val in res['pi_distribution']:
            h_pi.Fill(val)
        h_pi.Write()

    fout.Close()
    f.Close()

    print(f"\nBootstrap results saved to {args.output}")
    print(f"\nSummary:")
    print("Centrality  |  p/Λ-Λ ratio      |  π/Λ-Λ ratio")
    print("-" * 55)
    for res in results:
        if res is None:
            continue
        print(f"{res['centrality']:5.0f}%     | {res['p_ratio']:7.5f}±{res['p_error']:.5f} | "
              f"{res['pi_ratio']:7.5f}±{res['pi_error']:.5f}")

if __name__ == "__main__":
    main()
