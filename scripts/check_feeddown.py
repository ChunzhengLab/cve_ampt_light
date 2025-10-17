import ROOT
import argparse
import sys
from array import array

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

# Marker styles: first 4 solid, next 4 hollow
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

# Complete style table: name, label, left and right styles
style_table = [
    {"name": "p_lambda",
     "label": "p #minus #Lambda",
     "left": {"color": palette_colors[0], "marker": marker_styles[0], "draw": "3"},
     "right": {"color": palette_colors[0], "marker": marker_styles[0], "offset": 0.0, "draw": "P"}},
    {"name": "pFromLambda_vs_Lambda",
     "label": "p from #Lambda #minus #Lambda",
     "left": {"color": palette_colors[1], "marker": marker_styles[1], "draw": "3"},
     "right": {"color": palette_colors[1], "marker": marker_styles[1], "offset": 1.0, "draw": "P"}},
    {"name": "piFromLambda_vs_Lambda",
     "label": "#pi from #Lambda #minus #Lambda",
     "left": {"color": palette_colors[2], "marker": marker_styles[2], "draw": "3"},
     "right": {"color": palette_colors[2], "marker": marker_styles[2], "offset": 2.0, "draw": "P"}},
    {"name": "lambda_lambda",
     "label": "#Lambda #minus #Lambda",
     "left": {"color": palette_colors[3], "marker": marker_styles[3], "draw": "3"},
     "right": {"color": palette_colors[3], "marker": marker_styles[3], "offset": 3.0, "draw": "P"}},
    {"name": "pbar_lambda",
     "label": "#bar{p} #minus #bar{#Lambda}",
     "left": {"color": palette_colors[4], "marker": marker_styles[4], "draw": "3"},
     "right": {"color": palette_colors[4], "marker": marker_styles[4], "offset": 0.0, "draw": "P"}},
    {"name": "pFromLambdaBar_vs_Lambda",
     "label": "#bar{p} from #bar{#Lambda} #minus #bar{#Lambda}",
     "left": {"color": palette_colors[5], "marker": marker_styles[5], "draw": "3"},
     "right": {"color": palette_colors[5], "marker": marker_styles[5], "offset": -1.0, "draw": "P"}},
    {"name": "piFromLambdaBar_vs_Lambda",
     "label": "#pi+ from #bar{#Lambda} #minus #bar{#Lambda}",
     "left": {"color": palette_colors[6], "marker": marker_styles[6], "draw": "3"},
     "right": {"color": palette_colors[6], "marker": marker_styles[6], "offset": -2.0, "draw": "P"}},
    {"name": "lambda_lambdabar",
     "label": "#Lambda #minus #bar{#Lambda}",
     "left": {"color": palette_colors[7], "marker": marker_styles[7], "draw": "3"},
     "right": {"color": palette_colors[7], "marker": marker_styles[7], "offset": -3.0, "draw": "P"}},
]

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

def draw_dual_panel(name, delta_graphs, gamma_graphs, title, labels,
                    alice_delta_ss=None, alice_delta_os=None,
                    alice_gamma_ss=None, alice_gamma_os=None,
                    alice_lh_ss=None, alice_lh_os=None,
                    alice_glh_ss=None, alice_glh_os=None,
                    labels_ratio=None, colors_ratio=None):
    print(f"[Debug] draw_dual_panel called with {len(delta_graphs)} delta_graphs and {len(gamma_graphs)} gamma_graphs")
    c = ROOT.TCanvas(name, title, 1200, 500)
    # Create two manual pads for Delta and Gamma
    pad1 = ROOT.TPad("pad1", "Delta pad", 0.0, 0.0, 0.5, 1.0)
    pad2 = ROOT.TPad("pad2", "Gamma pad", 0.5, 0.0, 1.0, 1.0)
    pad1.Draw()
    pad2.Draw()
    pad1.SetGrid()
    pad1.cd()
    frame1 = pad1.DrawFrame(0, -0.013, 60.0, 0.013, ";Centrality (%);#delta")
    frame1.GetYaxis().SetMaxDigits(3)
    pad2.SetGrid()
    pad2.cd()
    frame2 = pad2.DrawFrame(0, 0, 60.0, 2, ";Centrality (%);ratio")
    frame2.GetYaxis().SetMaxDigits(3)

    pad1.cd()
    leg1 = ROOT.TLegend(0.15, 0.6, 0.6, 0.88)
    leg1.SetTextSize(0.03)
    leg1.SetBorderSize(0); leg1.SetFillStyle(0)
    leg1.SetNColumns(2)
    # Draw lines and points for each Delta graph
    for idx, g in enumerate(delta_graphs):
        g.Draw("3 SAME")
        leg1.AddEntry(g, labels[idx], "f")
    leg1.Draw()
    # prevent Python GC from collecting the legend on the pad
    pad1._leg1 = leg1

    pad2.cd()
    leg2 = ROOT.TLegend(0.15, 0.15, 0.6, 0.4)
    leg2.SetTextSize(0.03)
    leg2.SetBorderSize(0); leg2.SetFillStyle(0)
    # Draw lines and points for each Gamma/ratio graph
    for idx, g in enumerate(gamma_graphs):
        g.Draw("P SAME")
        if labels_ratio is not None:
            leg2.AddEntry(g, labels_ratio[idx], "lp")
        else:
            leg2.AddEntry(g, labels[idx], "lp")
    leg2.Draw()
    # prevent Python GC from collecting the legend on the pad
    pad2._leg2 = leg2

    c.Update()
    return c

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("input", help="Input ROOT file")
    parser.add_argument("--out", default="feeddown.pdf")
    args = parser.parse_args()

    f = ROOT.TFile.Open(args.input)
    if not f or f.IsZombie():
        print(f"Error: Cannot open file: {args.input}", file=sys.stderr)
        sys.exit(1)

    # Use style_table entries directly
    delta_graphs = []
    gamma_graphs = []
    labels = []
    for entry in style_table:
        name = entry["name"]
        dname = f"Delta_{name}"
        gname = f"Gamma_{name}"
        d = f.Get(dname); g = f.Get(gname)
        if not d:
            print(f"[Error] Missing {dname}")
        if not g:
            print(f"[Error] Missing {gname}")
        if not d or not g:
            continue
        # create graphs
        dg = profile_to_graph(d, name=dname)
        gg = profile_to_graph(g, name=gname)
        # apply centralized styles
        col = entry["left"]["color"]
        mstyle = entry["left"]["marker"]
        dg.SetLineColor(col); dg.SetMarkerColor(col); dg.SetMarkerStyle(mstyle); dg.SetFillColorAlpha(col, 0.3)
        col_r = entry["right"]["color"]
        mstyle_r = entry["right"]["marker"]
        gg.SetLineColor(col_r); gg.SetMarkerColor(col_r); gg.SetMarkerStyle(mstyle_r)
        gg.SetFillColorAlpha(col_r, 0.3)
        # append to lists
        delta_graphs.append(dg); gamma_graphs.append(gg)
        labels.append(entry["label"])

    # Build ratio entries directly from style_table, excluding denominators
    delta_ratio_entries = [entry for entry in style_table if entry["name"] not in ("p_lambda","pbar_lambda")]

    # Delta ratio: build in same order as left panel
    delta_ratio_graphs = []
    delta_ratio_labels = []
    delta_ratio_colors = []
    base_bar = next(g for entry,g in zip(style_table, delta_graphs) if entry["name"] == "pbar_lambda")
    base_norm = next(g for entry,g in zip(style_table, delta_graphs) if entry["name"] == "p_lambda")
    for idx, entry in enumerate(delta_ratio_entries):
        name = entry["name"]
        num_graph = next(g for entry,g in zip(style_table, delta_graphs) if entry["name"] == name)
        base = base_bar if name in ["pFromLambdaBar_vs_Lambda", "piFromLambdaBar_vs_Lambda", "lambda_lambdabar"] else base_norm
        rg = ROOT.TGraphErrors()
        # apply style for ratio graph
        col_r = entry["right"]["color"]
        mstyle_r = entry["right"]["marker"]
        rg.SetLineColor(col_r)
        rg.SetMarkerColor(col_r)
        rg.SetMarkerStyle(mstyle_r)
        rg.SetName(name + "_ratio")
        for i in range(num_graph.GetN()):
            x_arr, y_arr = array('d', [0.]), array('d', [0.])
            num_graph.GetPoint(i, x_arr, y_arr)
            x, y = x_arr[0], y_arr[0]
            if x > MAX_CENTRALITY: continue
            dnum = num_graph.GetErrorY(i)
            base_y = base.Eval(x)
            ratio = y/base_y if base_y else 0
            dbase = base.GetErrorY(i)
            err = ratio * ((dnum/y)**2 + (dbase/base_y)**2)**0.5 if y and base_y else 0
            offset = entry["right"]["offset"]
            rg.SetPoint(i, x + offset, ratio)
            rg.SetPointError(i, 0, err)
            print(f"Centrality {x:.1f}%, Delta ratio {name} = {ratio:.3f} ± {err:.3f}")
        delta_ratio_graphs.append(rg)
        delta_ratio_labels.append(entry["label"])
        delta_ratio_colors.append(entry["left"]["color"])

    # Build ratio entries directly from style_table, excluding denominators
    gamma_ratio_entries = [entry for entry in style_table if entry["name"] not in ("p_lambda","pbar_lambda")]

    # Gamma ratio: build in same order as left panel
    gamma_ratio_graphs = []
    gamma_ratio_labels = []
    gamma_ratio_colors = []
    base_bar_g = next(g for entry,g in zip(style_table, gamma_graphs) if entry["name"] == "pbar_lambda")
    base_norm_g = next(g for entry,g in zip(style_table, gamma_graphs) if entry["name"] == "p_lambda")
    for idx, entry in enumerate(gamma_ratio_entries):
        name = entry["name"]
        num_graph = next(g for entry,g in zip(style_table, gamma_graphs) if entry["name"] == name)
        base = base_bar_g if name in ["pFromLambdaBar_vs_Lambda", "piFromLambdaBar_vs_Lambda", "lambda_lambdabar"] else base_norm_g
        rg = ROOT.TGraphErrors()
        # apply style for ratio graph
        col_r = entry["right"]["color"]
        mstyle_r = entry["right"]["marker"]
        rg.SetLineColor(col_r)
        rg.SetMarkerColor(col_r)
        rg.SetMarkerStyle(mstyle_r)
        rg.SetName(name + "_ratio")
        for i in range(num_graph.GetN()):
            x_arr, y_arr = array('d', [0.]), array('d', [0.])
            num_graph.GetPoint(i, x_arr, y_arr)
            x, y = x_arr[0], y_arr[0]
            if x > MAX_CENTRALITY: continue
            dnum = num_graph.GetErrorY(i)
            base_y = base.Eval(x)
            ratio = y/base_y if base_y else 0
            dbase = base.GetErrorY(i)
            err = ratio * ((dnum/y)**2 + (dbase/base_y)**2)**0.5 if y and base_y else 0
            offset = entry["right"]["offset"]
            rg.SetPoint(i, x + offset, ratio)
            rg.SetPointError(i, 0, err)
            print(f"Centrality {x:.1f}%, Gamma ratio {name} = {ratio:.3f} ± {err:.3f}")
        gamma_ratio_graphs.append(rg)
        gamma_ratio_labels.append(entry["label"])
        gamma_ratio_colors.append(entry["left"]["color"])

    # First panel: Delta vs Ratio
    c_delta = draw_dual_panel("panel_delta", delta_graphs, delta_ratio_graphs, "#delta and ratio", labels[:len(delta_graphs)], labels_ratio=delta_ratio_labels, colors_ratio=delta_ratio_colors)
    c_delta.Print("delta_and_ratio.pdf")

    # Second panel: Gamma vs Ratio
    c_gamma = draw_dual_panel("panel_gamma", gamma_graphs, gamma_ratio_graphs, "#gamma and ratio", labels[:len(gamma_graphs)], labels_ratio=gamma_ratio_labels, colors_ratio=gamma_ratio_colors)
    c_gamma.Print("gamma_and_ratio.pdf")

if __name__ == "__main__":
    ROOT.gROOT.SetBatch(True)
    main()