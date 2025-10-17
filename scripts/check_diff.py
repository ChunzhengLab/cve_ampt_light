import ROOT
import argparse
import math
import ctypes
import csv

ROOT.gROOT.SetBatch(True)

style_table = [
    {"a": "p",      "b": "lambda", "label": "p-#Lambda",
     "color": ROOT.kBlue+1, "marker": ROOT.kFullCircle},
    {"a": "pbar",   "b": "lambda", "label": "#bar{p}-#Lambda",
     "color": ROOT.kRed+1,  "marker": ROOT.kFullSquare},
]

bin_labels_sumPt = [
    "1 < #Sigma p_{T} < 3 GeV",
    "3 < #Sigma p_{T} < 5 GeV",
    "5 < #Sigma p_{T} < 8 GeV"
]

bin_labels_etaGap = [
    "#Delta#eta < 0.3",
    "0.3 < #Delta#eta < 0.8",
    "#Delta#eta > 0.8"
]

def subtract_graphs(g1, g2):
    n = min(g1.GetN(), g2.GetN())
    g = ROOT.TGraphErrors(n)
    for i in range(n):
        x1 = ctypes.c_double(0.0)
        y1 = ctypes.c_double(0.0)
        x2 = ctypes.c_double(0.0)
        y2 = ctypes.c_double(0.0)
        g1.GetPoint(i, x1, y1)
        g2.GetPoint(i, x2, y2)
        x1 = x1.value; y1 = y1.value
        x2 = x2.value; y2 = y2.value
        y = y1 - y2
        ey = math.hypot(g1.GetErrorY(i), g2.GetErrorY(i))
        g.SetPoint(i, x1, y)
        g.SetPointError(i, 0, ey)
    return g

def profile_to_graph(p):
    g = ROOT.TGraphErrors(p.GetNbinsX())
    for i in range(1, p.GetNbinsX()+1):
        g.SetPoint(i-1, p.GetBinCenter(i) * 10, p.GetBinContent(i))
        g.SetPointError(i-1, 0, p.GetBinError(i))
    return g

def draw_dual_panel(name, delta_graphs, gamma_graphs, labels, title, outfile,
                    deltadelta_frame_range=(0., -0.002, 60., 0.02),
                    deltagamma_frame_range=(0., -0.0005, 60., 0.005)):
    c = ROOT.TCanvas(name, name, 1200, 500)
    c.Divide(2, 1)

    pad1 = c.cd(1)
    frame1 = pad1.DrawFrame(*deltadelta_frame_range, ";Centrality (%);#Delta#delta")
    leg1 = ROOT.TLegend(0.15, 0.6, 0.65, 0.88)
    leg1.SetTextSize(0.03); leg1.SetBorderSize(0); leg1.SetFillStyle(0)

    for g, lbl in zip(delta_graphs, labels):
        g.Draw("PE SAME")
        leg1.AddEntry(g, lbl, "pe")

    leg1.Draw()

    pad2 = c.cd(2)
    frame2 = pad2.DrawFrame(*deltagamma_frame_range, ";Centrality (%);#Delta#gamma")
    leg2 = ROOT.TLegend(0.15, 0.6, 0.65, 0.88)
    leg2.SetTextSize(0.03); leg2.SetBorderSize(0); leg2.SetFillStyle(0)

    for g, lbl in zip(gamma_graphs, labels):
        g.Draw("PE SAME")
        leg2.AddEntry(g, lbl, "pe")

    leg2.Draw()

    c.Print(outfile)

def write_csv(outfile, delta_graphs, gamma_graphs, labels, tag):
    if not delta_graphs or not gamma_graphs:
        return
    with open(outfile, "w", newline="") as fp:
        writer = csv.writer(fp)
        writer.writerow(["tag", "bin_label", "centrality_percent", "DeltaDelta", "DeltaDelta_err", "DeltaGamma", "DeltaGamma_err"]) 
        for idx, (dg, gg, lbl) in enumerate(zip(delta_graphs, gamma_graphs, labels)):
            n = min(dg.GetN(), gg.GetN())
            for i in range(n):
                x = ctypes.c_double(0.0)
                y_d = ctypes.c_double(0.0)
                dg.GetPoint(i, x, y_d)
                e_d = dg.GetErrorY(i)

                xg = ctypes.c_double(0.0)
                y_g = ctypes.c_double(0.0)
                gg.GetPoint(i, xg, y_g)
                e_g = gg.GetErrorY(i)

                writer.writerow([tag, lbl, x.value, y_d.value, e_d, y_g.value, e_g])

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("input", help="Input ROOT file")
    args = parser.parse_args()

    f = ROOT.TFile.Open(args.input)
    if not f or f.IsZombie():
        raise RuntimeError(f"Cannot open ROOT file {args.input}")

    for tag, bin_labels, outname in [("etaGap", bin_labels_etaGap, "diff_etaGap.pdf"),
                                     ("sumPt", bin_labels_sumPt, "diff_sumPt.pdf")]:
        delta_graphs = []
        gamma_graphs = []
        legends = []

        for style in style_table:
            for i in range(3):
                base = f"{style['a']}_{style['b']}_{tag}{i}"
                bar_base = f"{style['a'].replace('p','pbar') if 'p' in style['a'] else style['a']}_{style['b']}_{tag}{i}"

                d1 = f.Get(f"Delta_{bar_base}")
                d2 = f.Get(f"Delta_{base}")
                g1 = f.Get(f"Gamma_{bar_base}")
                g2 = f.Get(f"Gamma_{base}")

                if not d1 or not d2 or not g1 or not g2:
                    continue

                dg = subtract_graphs(profile_to_graph(d1), profile_to_graph(d2))
                gg = subtract_graphs(profile_to_graph(g1), profile_to_graph(g2))

                color = [ROOT.kBlue+1, ROOT.kOrange+7, ROOT.kGreen+2][i]
                marker = style["marker"]
                dg.SetMarkerColor(color); dg.SetLineColor(color); dg.SetMarkerStyle(marker)
                gg.SetMarkerColor(color); gg.SetLineColor(color); gg.SetMarkerStyle(marker)

                delta_graphs.append(dg)
                gamma_graphs.append(gg)
                legends.append(f"{bin_labels[i]}")

        draw_dual_panel(f"panel_{tag}", delta_graphs, gamma_graphs, legends,
                        f"#Delta#delta/#gamma: {tag}", outname)
        csv_out = outname.replace(".pdf", ".csv")
        write_csv(csv_out, delta_graphs, gamma_graphs, legends, tag)

if __name__ == "__main__":
    main()
