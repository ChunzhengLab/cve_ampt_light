import ROOT
import argparse
import sys
import csv
import math
import ctypes
from typing import List, Tuple

# Complete style table for combinations
style_table = [
    {"a": "p",      "b": "lambda", "label": "p-#Lambda",
     "left": {"color": ROOT.kBlue+1, "marker": ROOT.kFullCircle, "draw": "E3"},
     "right": {"color": ROOT.kBlue+1, "marker": ROOT.kFullCircle, "draw": "PE"}},
    {"a": "pbar",   "b": "lambda", "label": "#bar{p}-#Lambda",
     "left": {"color": ROOT.kRed+1,  "marker": ROOT.kFullSquare, "draw": "E3"},
     "right": {"color": ROOT.kRed+1,  "marker": ROOT.kFullSquare, "draw": "PE"}},
    {"a": "lambda", "b": "pi+",     "label": "#Lambda-#pi^{+}",
     "left": {"color": ROOT.kGreen+2,"marker": ROOT.kFullTriangleUp, "draw": "E3"},
     "right": {"color": ROOT.kGreen+2,"marker": ROOT.kFullTriangleUp, "draw": "PE"}},
    {"a": "lambda", "b": "pi-",     "label": "#Lambda-#pi^{-}",
     "left": {"color": ROOT.kMagenta+1,"marker": ROOT.kFullTriangleDown, "draw": "E3"},
     "right": {"color": ROOT.kMagenta+1,"marker": ROOT.kFullTriangleDown, "draw": "PE"}},
]

def read_alice_preliminary(csv_path):
    # Returns cent, dss_list, dssErr_list, dos_list, dosErr_list, gss_list, gssErr_list, gos_list, gosErr_list,
    #           lh_ss_list, lh_ssErr_list, lh_os_list, lh_osErr_list, glh_ss_list, glh_ssErr_list, glh_os_list, glh_osErr_list
    cent, dss_list, dssErr_list, dos_list, dosErr_list, gss_list, gssErr_list, gos_list, gosErr_list, lh_ss_list, lh_ssErr_list, lh_os_list, lh_osErr_list, glh_ss_list, glh_ssErr_list, glh_os_list, glh_osErr_list = [], [], [], [], [], [], [], [], [], [], [], [], [], [], [], [], []
    with open(csv_path, newline='') as csvfile:
        reader = csv.DictReader(csvfile)
        for row in reader:
            c = float(row["Centrality (%)"])
            dss      = float(row["Delta_SS"]);   dos      = float(row["Delta_OS"])
            dss_stat = float(row["Delta_SS_StatErr"]); dos_stat = float(row["Delta_OS_StatErr"])
            dss_sys  = float(row["Delta_SS_SystErr"]);    dos_sys  = float(row["Delta_OS_SystErr"])
            gss      = float(row["Gamma_SS"]);   gos      = float(row["Gamma_OS"])
            gss_stat = float(row["Gamma_SS_StatErr"]); gos_stat = float(row["Gamma_OS_StatErr"])
            gss_sys  = float(row["Gamma_SS_SystErr"]);    gos_sys  = float(row["Gamma_OS_SystErr"])
            err_dss = math.hypot(dss_stat, dss_sys)
            err_dos = math.hypot(dos_stat, dos_sys)
            err_gss = math.hypot(gss_stat, gss_sys)
            err_gos = math.hypot(gos_stat, gos_sys)
            # same-sign (SS)
            dss_list.append(dss); dssErr_list.append(err_dss)
            gss_list.append(gss); gssErr_list.append(err_gss)
            # opposite-sign (OS)
            dos_list.append(dos); dosErr_list.append(err_dos)
            gos_list.append(gos); gosErr_list.append(err_gos)
            cent.append(c)
            # LH (pi-Lambda) same-sign and opposite-sign
            lh_os  = float(row["Delta_LH_OS"]); lh_os_stat = float(row["Delta_LH_OS_StatErr"]); lh_os_syst = float(row["Delta_LH_OS_SystErr"])
            lh_ss  = float(row["Delta_LH_SS"]); lh_ss_stat = float(row["Delta_LH_SS_StatErr"]); lh_ss_syst = float(row["Delta_LH_SS_SystErr"])
            err_lh_os = math.hypot(lh_os_stat, lh_os_syst)
            err_lh_ss = math.hypot(lh_ss_stat, lh_ss_syst)
            lh_os_list.append(lh_os); lh_osErr_list.append(err_lh_os)
            lh_ss_list.append(lh_ss); lh_ssErr_list.append(err_lh_ss)
            # Gamma LH
            glh_os = float(row["Gamma_LH_OS"]); glh_os_stat = float(row["Gamma_LH_OS_StatErr"]); glh_os_syst = float(row["Gamma_LH_OS_SystErr"])
            glh_ss = float(row["Gamma_LH_SS"]); glh_ss_stat = float(row["Gamma_LH_SS_StatErr"]); glh_ss_syst = float(row["Gamma_LH_SS_SystErr"])
            err_glh_os = math.hypot(glh_os_stat, glh_os_syst)
            err_glh_ss = math.hypot(glh_ss_stat, glh_ss_syst)
            glh_os_list.append(glh_os); glh_osErr_list.append(err_glh_os)
            glh_ss_list.append(glh_ss); glh_ssErr_list.append(err_glh_ss)
    return (cent, dss_list, dssErr_list, dos_list, dosErr_list, gss_list, gssErr_list, gos_list, gosErr_list,
            lh_ss_list, lh_ssErr_list, lh_os_list, lh_osErr_list, glh_ss_list, glh_ssErr_list, glh_os_list, glh_osErr_list)

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
                    delta_frame_range=(0, -0.013, 70.0, 0.013),
                    gamma_frame_range=(0, -0.004, 70.0, 0.001),
                    is_diff=False):
    c = ROOT.TCanvas(name, title, 1200, 500)
    c.Divide(2, 1)
    pad1 = c.cd(1)
    pad1.SetGrid()
    frame1 = pad1.DrawFrame(*delta_frame_range, ";Centrality (%);#delta")
    frame1.GetYaxis().SetMaxDigits(3)
    pad2 = c.cd(2)
    pad2.SetGrid()
    frame2 = pad2.DrawFrame(*gamma_frame_range, ";Centrality (%);#gamma")
    frame2.GetYaxis().SetMaxDigits(3)

    pad1.cd()
    leg1 = ROOT.TLegend(0.15, 0.6, 0.65, 0.88)
    leg1.SetTextSize(0.03)
    leg1.SetBorderSize(0); leg1.SetFillStyle(0)
    leg1.SetNColumns(2);
    # AMPT band(s)
    for g in delta_graphs:
        g.SetFillColorAlpha(g.GetLineColor(), 0.3)
        g.SetLineColor(0); g.SetMarkerSize(0)
        g.Draw("E3 SAME")
    # add one legend entry per AMPT band
    for idx, g in enumerate(delta_graphs):
        leg1.AddEntry(g, labels[idx], "f")
    if alice_delta_ss:
        alice_delta_ss.Draw("PE SAME")
        leg1.AddEntry(alice_delta_ss, "ALICE #Lambda-p SS", "pe")
    if alice_delta_os:
        alice_delta_os.Draw("PE SAME")
        leg1.AddEntry(alice_delta_os, "ALICE #Lambda-p OS", "pe")
    if alice_lh_ss:
        alice_lh_ss.Draw("PE SAME")
        leg1.AddEntry(alice_lh_ss, "ALICE #Lambda-h SS", "pe")
    if alice_lh_os:
        alice_lh_os.Draw("PE SAME")
        leg1.AddEntry(alice_lh_os, "ALICE #Lambda-h OS", "pe")
    leg1.Draw()
    # prevent Python GC from collecting the legend
    c._leg1 = leg1

    pad2.cd()
    if is_diff:
        leg2 = ROOT.TLegend(0.15, 0.6, 0.65, 0.88)
    else:
        leg2 = ROOT.TLegend(0.15, 0.15, 0.65, 0.45)
    leg2.SetTextSize(0.03)
    leg2.SetBorderSize(0); leg2.SetFillStyle(0)
    leg2.SetNColumns(2);
    # AMPT band(s)
    for g in gamma_graphs:
        g.SetFillColorAlpha(g.GetLineColor(), 0.3)
        g.SetLineColor(0); g.SetMarkerSize(0)
        g.Draw("E3 SAME")
    # add one legend entry per AMPT band
    for idx, g in enumerate(gamma_graphs):
        leg2.AddEntry(g, labels[idx], "f")
    if alice_gamma_ss:
        alice_gamma_ss.Draw("PE SAME")
        leg2.AddEntry(alice_gamma_ss, "ALICE #Lambda-p SS", "pe")
    if alice_gamma_os:
        alice_gamma_os.Draw("PE SAME")
        leg2.AddEntry(alice_gamma_os, "ALICE #Lambda-p OS", "pe")
    if alice_glh_ss:
        alice_glh_ss.Draw("PE SAME")
        leg2.AddEntry(alice_glh_ss, "ALICE #Lambda-h SS", "pe")
    if alice_glh_os:
        alice_glh_os.Draw("PE SAME")
        leg2.AddEntry(alice_glh_os, "ALICE #Lambda-h OS", "pe")
    leg2.Draw()
    # prevent Python GC from collecting the legend
    c._leg2 = leg2

    c.Update()
    return c

def subtract_graphs(g1, g2, name, color, marker):
    n = min(g1.GetN(), g2.GetN())
    g = ROOT.TGraphErrors(n)
    g.SetName(name)
    g.SetLineColor(color)
    g.SetFillColorAlpha(color, 0.3)
    g.SetMarkerColor(color)
    g.SetMarkerStyle(marker)
    for i in range(n):
        x1 = ctypes.c_double(0.0)
        y1 = ctypes.c_double(0.0)
        x2 = ctypes.c_double(0.0)
        y2 = ctypes.c_double(0.0)
        g1.GetPoint(i, x1, y1)
        g2.GetPoint(i, x2, y2)
        x1 = x1.value; y1 = y1.value
        x2 = x2.value; y2 = y2.value
        if abs(x1 - x2) > 1e-3:
            print(f"[Warning] x mismatch at {i}: {x1} vs {x2}")
        y = y1 - y2
        ey = math.hypot(g1.GetErrorY(i), g2.GetErrorY(i))
        g.SetPoint(i, x1, y)
        g.SetPointError(i, 0, ey)
    # g.SetMarkerSize(1.2)
    # g.SetFillStyle(3001)
    return g

def graph_to_arrays(graph: ROOT.TGraphErrors) -> Tuple[List[float], List[float], List[float]]:
    xs: List[float] = []
    ys: List[float] = []
    eys: List[float] = []
    if graph is None:
        return xs, ys, eys
    for i in range(graph.GetN()):
        x = ctypes.c_double(0.0)
        y = ctypes.c_double(0.0)
        graph.GetPoint(i, x, y)
        xs.append(x.value)
        ys.append(y.value)
        eys.append(graph.GetErrorY(i))
    return xs, ys, eys

def export_ampt_csv(root_file: ROOT.TFile, output_csv_path: str) -> None:
    # Build graphs from TProfiles, following the same definitions used in plotting
    d_ss_g = profile_to_graph(root_file.Get("Delta_p_lambda"), name="Delta_p_lambda")
    d_os_g = profile_to_graph(root_file.Get("Delta_pbar_lambda"), name="Delta_pbar_lambda")
    d_lh_os_g = profile_to_graph(root_file.Get("Delta_lambda_pi-"), name="Delta_lambda_pi-")
    d_lh_ss_g = profile_to_graph(root_file.Get("Delta_lambda_pi+"), name="Delta_lambda_pi+")

    g_ss_g = profile_to_graph(root_file.Get("Gamma_p_lambda"), name="Gamma_p_lambda")
    g_os_g = profile_to_graph(root_file.Get("Gamma_pbar_lambda"), name="Gamma_pbar_lambda")
    g_lh_os_g = profile_to_graph(root_file.Get("Gamma_lambda_pi-"), name="Gamma_lambda_pi-")
    g_lh_ss_g = profile_to_graph(root_file.Get("Gamma_lambda_pi+"), name="Gamma_lambda_pi+")

    # Ensure required series exist
    required_graphs = {
        "Delta_p_lambda": d_ss_g,
        "Delta_pbar_lambda": d_os_g,
        "Delta_lambda_pi-": d_lh_os_g,
        "Delta_lambda_pi+": d_lh_ss_g,
        "Gamma_p_lambda": g_ss_g,
        "Gamma_pbar_lambda": g_os_g,
        "Gamma_lambda_pi-": g_lh_os_g,
        "Gamma_lambda_pi+": g_lh_ss_g,
    }
    missing = [k for k, v in required_graphs.items() if v is None]
    if missing:
        print(f"[Error] Missing required profiles for export: {', '.join(missing)}", file=sys.stderr)
        return

    # Extract arrays
    cent, d_ss, d_ss_stat = graph_to_arrays(d_ss_g)
    cent2, d_os, d_os_stat = graph_to_arrays(d_os_g)
    cent3, d_lh_os, d_lh_os_stat = graph_to_arrays(d_lh_os_g)
    cent4, d_lh_ss, d_lh_ss_stat = graph_to_arrays(d_lh_ss_g)

    cent5, g_ss, g_ss_stat = graph_to_arrays(g_ss_g)
    cent6, g_os, g_os_stat = graph_to_arrays(g_os_g)
    cent7, g_lh_os, g_lh_os_stat = graph_to_arrays(g_lh_os_g)
    cent8, g_lh_ss, g_lh_ss_stat = graph_to_arrays(g_lh_ss_g)

    # Determine number of rows to export (ensure alignment)
    n = min(len(cent), len(cent2), len(cent3), len(cent4), len(cent5), len(cent6), len(cent7), len(cent8))
    if n == 0:
        print("[Error] No data points available for export.", file=sys.stderr)
        return

    # Sanity check for x consistency
    def check_x(label: str, xs: List[float]) -> None:
        for i in range(n):
            if abs(xs[i] - cent[i]) > 1e-6:
                print(f"[Warning] Centrality x mismatch in {label} at index {i}: {xs[i]} vs {cent[i]}")

    check_x("Delta_OS", cent2)
    check_x("Delta_LH_OS", cent3)
    check_x("Delta_LH_SS", cent4)
    check_x("Gamma_SS", cent5)
    check_x("Gamma_OS", cent6)
    check_x("Gamma_LH_OS", cent7)
    check_x("Gamma_LH_SS", cent8)

    header = [
        "Centrality (%)",
        "Delta_SS","Delta_SS_StatErr","Delta_SS_SystErr",
        "Delta_OS","Delta_OS_StatErr","Delta_OS_SystErr",
        "Delta_LH_OS","Delta_LH_OS_StatErr","Delta_LH_OS_SystErr",
        "Delta_LH_SS","Delta_LH_SS_StatErr","Delta_LH_SS_SystErr",
        "Gamma_SS","Gamma_SS_StatErr","Gamma_SS_SystErr",
        "Gamma_OS","Gamma_OS_StatErr","Gamma_OS_SystErr",
        "Gamma_LH_OS","Gamma_LH_OS_StatErr","Gamma_LH_OS_SystErr",
        "Gamma_LH_SS","Gamma_LH_SS_StatErr","Gamma_LH_SS_SystErr",
    ]

    with open(output_csv_path, "w", newline="") as fp:
        writer = csv.writer(fp)
        writer.writerow(header)
        for i in range(n):
            row = [
                cent[i],
                d_ss[i], d_ss_stat[i], 0.0,
                d_os[i], d_os_stat[i], 0.0,
                d_lh_os[i], d_lh_os_stat[i], 0.0,
                d_lh_ss[i], d_lh_ss_stat[i], 0.0,
                g_ss[i], g_ss_stat[i], 0.0,
                g_os[i], g_os_stat[i], 0.0,
                g_lh_os[i], g_lh_os_stat[i], 0.0,
                g_lh_ss[i], g_lh_ss_stat[i], 0.0,
            ]
            writer.writerow(row)
    print(f"[Info] Exported AMPT results to CSV: {output_csv_path}")

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("input", help="Input ROOT file")
    parser.add_argument("--alice-csv", help="Path to ALICE preliminary CSV", default="cve_alice_preliminary.csv")
    parser.add_argument("--export-csv", help="Path to export AMPT results as CSV (matching ALICE format)", default="ampt_from_root.csv")
    args = parser.parse_args()

    f = ROOT.TFile.Open(args.input)
    if not f or f.IsZombie():
        print(f"Error: Cannot open file: {args.input}", file=sys.stderr)
        sys.exit(1)

    alice_delta_ss = None
    alice_delta_os = None
    alice_gamma_ss = None
    alice_gamma_os = None
    alice_lh_ss = None
    alice_lh_os = None
    alice_glh_ss = None
    alice_glh_os = None
    if args.alice_csv:
        ac, dss, dssErr, dos, dosErr, gss, gssErr, gos, gosErr, lh_ss, lh_ssErr, lh_os, lh_osErr, glh_ss, glh_ssErr, glh_os, glh_osErr = read_alice_preliminary(args.alice_csv)
        alice_delta_ss = ROOT.TGraphErrors(len(ac))
        alice_delta_os = ROOT.TGraphErrors(len(ac))
        alice_gamma_ss = ROOT.TGraphErrors(len(ac))
        alice_gamma_os = ROOT.TGraphErrors(len(ac))
        alice_lh_ss   = ROOT.TGraphErrors(len(ac))
        alice_lh_os   = ROOT.TGraphErrors(len(ac))
        alice_glh_ss  = ROOT.TGraphErrors(len(ac))
        alice_glh_os  = ROOT.TGraphErrors(len(ac))
        for i, x in enumerate(ac):
            alice_delta_ss.SetPoint(i, x, dss[i]);  alice_delta_ss.SetPointError(i, 0, dssErr[i])
            alice_delta_os.SetPoint(i, x, dos[i]);  alice_delta_os.SetPointError(i, 0, dosErr[i])
            alice_gamma_ss.SetPoint(i, x, gss[i]);  alice_gamma_ss.SetPointError(i, 0, gssErr[i])
            alice_gamma_os.SetPoint(i, x, gos[i]);  alice_gamma_os.SetPointError(i, 0, gosErr[i])
            alice_lh_ss.SetPoint(i, x, lh_ss[i]);    alice_lh_ss.SetPointError(i, 0, lh_ssErr[i])
            alice_lh_os.SetPoint(i, x, lh_os[i]);    alice_lh_os.SetPointError(i, 0, lh_osErr[i])
            alice_glh_ss.SetPoint(i, x, glh_ss[i]);  alice_glh_ss.SetPointError(i, 0, glh_ssErr[i])
            alice_glh_os.SetPoint(i, x, glh_os[i]);  alice_glh_os.SetPointError(i, 0, glh_osErr[i])
        alice_delta_ss.SetMarkerStyle(24); alice_delta_ss.SetMarkerColor(ROOT.kBlack)
        alice_delta_os.SetMarkerStyle(25); alice_delta_os.SetMarkerColor(ROOT.kBlack)
        alice_gamma_ss.SetMarkerStyle(24); alice_gamma_ss.SetMarkerColor(ROOT.kBlack)
        alice_gamma_os.SetMarkerStyle(25); alice_gamma_os.SetMarkerColor(ROOT.kBlack)
        alice_lh_ss.SetMarkerStyle(26); alice_lh_ss.SetMarkerColor(ROOT.kBlack)
        alice_lh_os.SetMarkerStyle(27); alice_lh_os.SetMarkerColor(ROOT.kBlack)
        alice_glh_ss.SetMarkerStyle(26); alice_glh_ss.SetMarkerColor(ROOT.kBlack)
        alice_glh_os.SetMarkerStyle(27); alice_glh_os.SetMarkerColor(ROOT.kBlack)

        alice_lh_dd = subtract_graphs(alice_lh_os, alice_lh_ss, "alice_lh_dd", ROOT.kBlack, 28)
        alice_lh_gg = subtract_graphs(alice_glh_os, alice_glh_ss, "alice_lh_gg", ROOT.kBlack, 29)

    delta_graphs = []
    gamma_graphs = []
    labels = []
    for entry in style_table:
        a, b = entry["a"], entry["b"]
        label = entry["label"]
        d = f.Get(f"Delta_{a}_{b}")
        g = f.Get(f"Gamma_{a}_{b}")
        if not d or not g:
            continue
        # convert profiles
        dg = profile_to_graph(d, name=f"Delta_{a}_{b}")
        gg = profile_to_graph(g, name=f"Gamma_{a}_{b}")
        # apply left style to dg
        ls = entry["left"]
        dg.SetLineColor(ls["color"])
        dg.SetMarkerStyle(ls["marker"])
        # apply right style to gg
        rs = entry["right"]
        gg.SetLineColor(rs["color"])
        gg.SetMarkerStyle(rs["marker"])
        # collect
        delta_graphs.append(dg)
        gamma_graphs.append(gg)
        labels.append(label)

    c1 = draw_dual_panel("panel_base", delta_graphs, gamma_graphs, "Delta/Gamma base", labels,
                         alice_delta_ss, alice_delta_os, alice_gamma_ss, alice_gamma_os,
                         alice_lh_ss, alice_lh_os, alice_glh_ss, alice_glh_os)
    c1.Print("panel_base.pdf")

    # 构造 AMPT 的 Δdelta 和 Δgamma
    d_ss = f.Get("Delta_p_lambda")
    d_os = f.Get("Delta_pbar_lambda")
    g_ss = f.Get("Gamma_p_lambda")
    g_os = f.Get("Gamma_pbar_lambda")
    dg = subtract_graphs(profile_to_graph(d_os), profile_to_graph(d_ss), "Delta_diff", ROOT.kAzure+7, 20)
    gg = subtract_graphs(profile_to_graph(g_os), profile_to_graph(g_ss), "Gamma_diff", ROOT.kPink+6, 25)

    # 构造 ALICE Δdelta 和 Δgamma
    alice_dd, alice_gg = None, None
    if alice_delta_ss and alice_delta_os and alice_gamma_ss and alice_gamma_os:
        alice_dd = subtract_graphs(alice_delta_os, alice_delta_ss, "alice_dd", ROOT.kBlack, 33)
        alice_gg = subtract_graphs(alice_gamma_os, alice_gamma_ss, "alice_gg", ROOT.kBlack, 34)

    # Add Lambda-pi OS-SS graphs to the diff panel
    d_pi_plus = f.Get("Delta_lambda_pi+")
    d_pi_minus = f.Get("Delta_lambda_pi-")
    g_pi_plus = f.Get("Gamma_lambda_pi+")
    g_pi_minus = f.Get("Gamma_lambda_pi-")
    dg_lh = subtract_graphs(profile_to_graph(d_pi_minus), profile_to_graph(d_pi_plus), "Delta_lh_diff", ROOT.kGreen+2, 24)
    gg_lh = subtract_graphs(profile_to_graph(g_pi_minus), profile_to_graph(g_pi_plus), "Gamma_lh_diff", ROOT.kMagenta+2, 25)
    delta_graphs = [dg, dg_lh]
    gamma_graphs = [gg, gg_lh]
    labels = ["AMPT p-#Lambda OS-SS", "AMPT #Lambda-#pi OS-SS"]
    c2 = draw_dual_panel("panel_diff", delta_graphs, gamma_graphs, "OS-SS subtraction", labels,
                         alice_delta_ss=None, alice_delta_os=None,
                         alice_gamma_ss=None, alice_gamma_os=None,
                         alice_lh_ss=None, alice_lh_os=None,
                         alice_glh_ss=None, alice_glh_os=None,
                         delta_frame_range=(0., -0.002, 60., 0.02),
                         gamma_frame_range=(0., -0.00050, 60., 0.005),
                         is_diff=True)
    # Draw ALICE points for both p-Lambda and Lambda-pi, with pad switching
    c2.cd(1)
    if alice_dd:
        alice_dd.Draw("PE SAME")
        c2._leg1.AddEntry(alice_dd, "ALICE p-#Lambda OS-SS #delta", "pe")
    alice_lh_dd.Draw("PE SAME")
    c2._leg1.AddEntry(alice_lh_dd, "ALICE #Lambda-#pi OS-SS #delta", "pe")

    c2.cd(2)
    if alice_gg:
        alice_gg.Draw("PE SAME")
        c2._leg2.AddEntry(alice_gg, "ALICE p-#Lambda OS-SS #gamma", "pe")
    alice_lh_gg.Draw("PE SAME")
    c2._leg2.AddEntry(alice_lh_gg, "ALICE #Lambda-#pi OS-SS #gamma", "pe")
    c2.Update()
    c2.Print("panel_diff.pdf")

    # panel_diff_lh is now merged into panel_diff; remove separate panel_diff_lh
    # Export AMPT results to CSV in ALICE format if requested
    if args.export_csv:
        export_ampt_csv(f, args.export_csv)

if __name__ == "__main__":
    ROOT.gROOT.SetBatch(True) 
    main()