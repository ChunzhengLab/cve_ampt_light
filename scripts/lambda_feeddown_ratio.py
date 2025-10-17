import ROOT
import argparse
import sys
from array import array
import csv

MAX_CENTRALITY = 55.0

palette_colors = [
    ROOT.kGreen+2,
    ROOT.kMagenta+1,
    ROOT.kOrange+7,
    ROOT.kBlue+1,
    ROOT.kRed+1,
    ROOT.kCyan+2,
    ROOT.kViolet+1,
    ROOT.kGray+2
]

marker_styles = [
    ROOT.kFullCircle,       # 20
    ROOT.kFullSquare,       # 21
    ROOT.kFullTriangleUp,   # 22
    ROOT.kFullTriangleDown, # 23
    ROOT.kOpenCircle,       # 24
    ROOT.kOpenSquare,       # 25
    ROOT.kOpenTriangleUp,   # 26
    ROOT.kOpenDiamond       # 27
]

# Define key entries we want to plot the ratio for
lambda_lambda_entry = {
    "name": "lambda_lambda",
    "label": "#Lambda #minus #Lambda",
    "color": palette_colors[3],
    "marker": marker_styles[3]
}

p_from_lambda_entry = {
    "name": "pFromLambda_vs_Lambda",
    "label": "p from #Lambda #minus #Lambda",
    "color": palette_colors[1],
    "marker": marker_styles[1]
}

pi_from_lambda_entry = {
    "name": "piFromLambda_vs_Lambda",
    "label": "#pi from #Lambda #minus #Lambda",
    "color": palette_colors[2],
    "marker": marker_styles[2]
}

def profile_to_graph(profile, color=1, marker=20, name=None):
    if not profile:
        print(f"[Warning] TProfile '{name}' not found — skipping this entry.")
        return None
    graph = ROOT.TGraphErrors()
    graph.SetName(name or profile.GetName())
    graph.SetLineColor(color)
    graph.SetMarkerColor(color)
    graph.SetMarkerStyle(marker)
    for i in range(1, profile.GetNbinsX() + 1):
        x = profile.GetBinCenter(i) * 10
        y = profile.GetBinContent(i)
        ey = profile.GetBinError(i)
        graph.SetPoint(graph.GetN(), x, y)
        graph.SetPointError(graph.GetN() - 1, 0, ey)
    return graph

def create_ratio_graph(num_graph, denom_graph, name, color, marker, offset=0.0):
    ratio_graph = ROOT.TGraphErrors()
    ratio_graph.SetName(name)
    ratio_graph.SetLineColor(color)
    ratio_graph.SetMarkerColor(color)
    ratio_graph.SetMarkerStyle(marker)

    for i in range(num_graph.GetN()):
        x_arr, y_arr = array('d', [0.]), array('d', [0.])
        num_graph.GetPoint(i, x_arr, y_arr)
        x, y_num = x_arr[0], y_arr[0]

        if x > MAX_CENTRALITY:
            continue

        # Get denominator value at this x
        denom_y = denom_graph.Eval(x)

        # Skip if denominator is zero
        if denom_y == 0:
            continue

        # Calculate ratio
        ratio = y_num / denom_y

        # Calculate error propagation
        num_err = num_graph.GetErrorY(i)
        denom_err = denom_graph.GetErrorY(i)
        rel_err = ((num_err/y_num)**2 + (denom_err/denom_y)**2)**0.5
        total_err = ratio * rel_err

        # Add point to graph
        ratio_graph.SetPoint(ratio_graph.GetN(), x + offset, ratio)
        ratio_graph.SetPointError(ratio_graph.GetN() - 1, 0, total_err)

        print(f"Centrality {x:.1f}%, Ratio {name} = {ratio:.3f} ± {total_err:.3f}")

    # Perform pol0 fit
    fit_func = ROOT.TF1(f"{name}_fit", "pol0", 0, MAX_CENTRALITY)
    fit_func.SetLineColor(color)
    fit_func.SetLineStyle(2)  # Dashed line

    # Do the fit
    fit_result = ratio_graph.Fit(fit_func, "QS")  # Quiet and Save result

    # Get fit parameters
    constant = fit_func.GetParameter(0)
    constant_err = fit_func.GetParError(0)

    # Print fit result
    print(f"\n*** Fit result for {name}: {constant:.4f} ± {constant_err:.4f} ***\n")

    return ratio_graph, fit_func

def save_graph_to_csv(graph, filename, label=""):
    """Save TGraphErrors data to CSV file"""
    with open(filename, 'w', newline='') as csvfile:
        writer = csv.writer(csvfile)
        # Write header
        writer.writerow(['label', 'centrality_percent', 'value', 'error'])
        
        # Write data points
        for i in range(graph.GetN()):
            x_arr, y_arr = array('d', [0.]), array('d', [0.])
            graph.GetPoint(i, x_arr, y_arr)
            x, y = x_arr[0], y_arr[0]
            error = graph.GetErrorY(i)
            writer.writerow([label, x, y, error])
    
    print(f"Saved data to {filename}")

def save_ratio_to_csv(graph, fit_func, filename, label=""):
    """Save ratio graph and fit result to CSV file"""
    with open(filename, 'w', newline='') as csvfile:
        writer = csv.writer(csvfile)
        # Write header
        writer.writerow(['label', 'centrality_percent', 'ratio', 'ratio_error', 'fit_constant', 'fit_error'])
        
        # Get fit parameters
        fit_constant = fit_func.GetParameter(0)
        fit_error = fit_func.GetParError(0)
        
        # Write data points
        for i in range(graph.GetN()):
            x_arr, y_arr = array('d', [0.]), array('d', [0.])
            graph.GetPoint(i, x_arr, y_arr)
            x, y = x_arr[0], y_arr[0]
            error = graph.GetErrorY(i)
            writer.writerow([label, x, y, error, fit_constant, fit_error])
    
    print(f"Saved ratio data to {filename}")

def draw_ratio_plot(ratio_data, labels, title, filename):
    c = ROOT.TCanvas("c_ratio", title, 800, 600)
    c.SetGrid()

    # Create frame with proper range
    frame = c.DrawFrame(0, 0, 60.0, 2.0, ";Centrality (%);Ratio to #Lambda #minus #Lambda")
    frame.GetYaxis().SetMaxDigits(3)

    # Create legend
    leg = ROOT.TLegend(0.15, 0.65, 0.6, 0.85)
    leg.SetTextSize(0.035)
    leg.SetBorderSize(0)
    leg.SetFillStyle(0)

    # Draw graphs and fits
    for i, (graph, fit_func) in enumerate(ratio_data):
        graph.Draw("P SAME")
        fit_func.Draw("SAME")
        leg.AddEntry(graph, labels[i], "lp")
        leg.AddEntry(fit_func, "Constant fit", "l")

    leg.Draw()

    # Add ratio=1 reference line
    line = ROOT.TLine(0, 1.0, 60.0, 1.0)
    line.SetLineStyle(2)
    line.SetLineColor(ROOT.kGray+1)
    line.Draw()

    # Save to file
    c.SaveAs(filename)
    return c

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("input", help="Input ROOT file")
    parser.add_argument("--out", default="./lambda_feeddown_ratio.pdf")
    parser.add_argument("--gamma", action="store_true", help="Use Gamma correlations (default: Delta)")
    args = parser.parse_args()

    f = ROOT.TFile.Open(args.input)
    if not f or f.IsZombie():
        print(f"Error: Cannot open file: {args.input}", file=sys.stderr)
        sys.exit(1)

    # Determine which correlation type to use
    corr_type = "Gamma" if args.gamma else "Delta"
    print(f"Using {corr_type} correlations")

    # Get the base Lambda-Lambda correlation
    lambda_lambda_name = f"{corr_type}_{lambda_lambda_entry['name']}"
    lambda_lambda_profile = f.Get(lambda_lambda_name)
    if not lambda_lambda_profile:
        print(f"Error: Cannot find {lambda_lambda_name} in the input file", file=sys.stderr)
        sys.exit(1)

    # Create graph for Lambda-Lambda correlation
    lambda_lambda_graph = profile_to_graph(
        lambda_lambda_profile,
        color=lambda_lambda_entry["color"],
        marker=lambda_lambda_entry["marker"],
        name=lambda_lambda_name
    )

    # Get p from Lambda correlation
    p_from_lambda_name = f"{corr_type}_{p_from_lambda_entry['name']}"
    p_from_lambda_profile = f.Get(p_from_lambda_name)
    if not p_from_lambda_profile:
        print(f"Error: Cannot find {p_from_lambda_name} in the input file", file=sys.stderr)
        sys.exit(1)

    # Create graph for p from Lambda correlation
    p_from_lambda_graph = profile_to_graph(
        p_from_lambda_profile,
        color=p_from_lambda_entry["color"],
        marker=p_from_lambda_entry["marker"],
        name=p_from_lambda_name
    )

    # Get pi from Lambda correlation
    pi_from_lambda_name = f"{corr_type}_{pi_from_lambda_entry['name']}"
    pi_from_lambda_profile = f.Get(pi_from_lambda_name)
    if not pi_from_lambda_profile:
        print(f"Error: Cannot find {pi_from_lambda_name} in the input file", file=sys.stderr)
        sys.exit(1)

    # Create graph for pi from Lambda correlation
    pi_from_lambda_graph = profile_to_graph(
        pi_from_lambda_profile,
        color=pi_from_lambda_entry["color"],
        marker=pi_from_lambda_entry["marker"],
        name=pi_from_lambda_name
    )

    # Create ratio graphs with fits
    p_lambda_ratio, p_lambda_fit = create_ratio_graph(
        p_from_lambda_graph,
        lambda_lambda_graph,
        "p_from_lambda_ratio",
        p_from_lambda_entry["color"],
        p_from_lambda_entry["marker"],
        offset=-0.5
    )

    pi_lambda_ratio, pi_lambda_fit = create_ratio_graph(
        pi_from_lambda_graph,
        lambda_lambda_graph,
        "pi_from_lambda_ratio",
        pi_from_lambda_entry["color"],
        pi_from_lambda_entry["marker"],
        offset=0.5
    )

    # Determine output file name
    output_file = f"./{corr_type.lower()}_lambda_feeddown_ratio.pdf"
    if args.out:
        output_file = args.out
    
    # Save original correlation data to CSV files
    csv_prefix = output_file.replace('.pdf', '') if args.out else f"{corr_type.lower()}_lambda_feeddown"
    
    # Save Lambda-Lambda correlation
    save_graph_to_csv(
        lambda_lambda_graph,
        f"{csv_prefix}_lambda_lambda.csv",
        f"{corr_type}_Lambda-Lambda"
    )
    
    # Save p from Lambda correlation
    save_graph_to_csv(
        p_from_lambda_graph,
        f"{csv_prefix}_p_from_lambda.csv",
        f"{corr_type}_p_from_Lambda-Lambda"
    )
    
    # Save pi from Lambda correlation
    save_graph_to_csv(
        pi_from_lambda_graph,
        f"{csv_prefix}_pi_from_lambda.csv",
        f"{corr_type}_pi_from_Lambda-Lambda"
    )
    
    # Save ratio data to CSV files
    save_ratio_to_csv(
        p_lambda_ratio,
        p_lambda_fit,
        f"{csv_prefix}_p_lambda_ratio.csv",
        "p_from_Lambda/Lambda-Lambda"
    )
    
    save_ratio_to_csv(
        pi_lambda_ratio,
        pi_lambda_fit,
        f"{csv_prefix}_pi_lambda_ratio.csv",
        "pi_from_Lambda/Lambda-Lambda"
    )
    
    # Draw the ratio plot
    ratio_data = [(p_lambda_ratio, p_lambda_fit), (pi_lambda_ratio, pi_lambda_fit)]
    ratio_labels = [
        "p from #Lambda - #Lambda / #Lambda - #Lambda",
        "#pi from #Lambda - #Lambda / #Lambda - #Lambda"
    ]

    draw_ratio_plot(
        ratio_data,
        ratio_labels,
        f"{corr_type} Correlation Ratios to #Lambda#minus#Lambda",
        output_file
    )

    print(f"Output saved to {output_file}")

if __name__ == "__main__":
    ROOT.gROOT.SetBatch(True)
    main()
