void plot_cve_data() {
    // Set ROOT style
    gROOT->SetStyle("Plain");
    gStyle->SetOptStat(0);
    gStyle->SetOptFit(0);
    gStyle->SetOptTitle(0);
    gStyle->SetPadTickX(1);
    gStyle->SetPadTickY(1);
    
    // Create canvas with two panels
    TCanvas *c1 = new TCanvas("c1", "CVE Results", 600, 800);
    
    // Create two pads for upper and lower panels
    TPad *pad1 = new TPad("pad1", "Upper panel", 0, 0.5, 1, 1);
    TPad *pad2 = new TPad("pad2", "Lower panel", 0, 0, 1, 0.5);
    
    // Set margins for pads
    pad1->SetBottomMargin(0.0);
    pad1->SetTopMargin(0.12);
    pad1->SetLeftMargin(0.15);
    pad1->SetRightMargin(0.05);
    
    pad2->SetTopMargin(0.0);
    pad2->SetBottomMargin(0.15);
    pad2->SetLeftMargin(0.15);
    pad2->SetRightMargin(0.05);
    
    pad1->Draw();
    pad2->Draw();
    
    // Read experimental data (ALICE Preliminary)
    ifstream alice_file("cve_alice_preliminary.csv");
    string header;
    getline(alice_file, header); // Skip header
    
    vector<double> cent_alice, delta_ss, delta_ss_stat, delta_ss_syst;
    vector<double> delta_os, delta_os_stat, delta_os_syst;
    vector<double> gamma_ss, gamma_ss_stat, gamma_ss_syst;
    vector<double> gamma_os, gamma_os_stat, gamma_os_syst;
    
    // Lambda-h data vectors
    vector<double> delta_lh_ss, delta_lh_ss_stat, delta_lh_ss_syst;
    vector<double> delta_lh_os, delta_lh_os_stat, delta_lh_os_syst;
    vector<double> gamma_lh_ss, gamma_lh_ss_stat, gamma_lh_ss_syst;
    vector<double> gamma_lh_os, gamma_lh_os_stat, gamma_lh_os_syst;
    
    string line;
    while (getline(alice_file, line)) {
        if (line.empty()) continue;
        
        double cent, d_ss, d_ss_st, d_ss_sy, d_os, d_os_st, d_os_sy;
        double d_lh_os, d_lh_os_st, d_lh_os_sy, d_lh_ss, d_lh_ss_st, d_lh_ss_sy;
        double g_ss, g_ss_st, g_ss_sy, g_os, g_os_st, g_os_sy;
        double g_lh_os, g_lh_os_st, g_lh_os_sy, g_lh_ss, g_lh_ss_st, g_lh_ss_sy;
        
        char comma;
        stringstream ss(line);
        ss >> cent >> comma >> d_ss >> comma >> d_ss_st >> comma >> d_ss_sy >> comma
           >> d_os >> comma >> d_os_st >> comma >> d_os_sy >> comma
           >> d_lh_os >> comma >> d_lh_os_st >> comma >> d_lh_os_sy >> comma
           >> d_lh_ss >> comma >> d_lh_ss_st >> comma >> d_lh_ss_sy >> comma
           >> g_ss >> comma >> g_ss_st >> comma >> g_ss_sy >> comma
           >> g_os >> comma >> g_os_st >> comma >> g_os_sy >> comma
           >> g_lh_os >> comma >> g_lh_os_st >> comma >> g_lh_os_sy >> comma
           >> g_lh_ss >> comma >> g_lh_ss_st >> comma >> g_lh_ss_sy;
        
        cent_alice.push_back(cent);
        delta_ss.push_back(d_ss);
        delta_ss_stat.push_back(d_ss_st);
        delta_ss_syst.push_back(d_ss_sy);
        delta_os.push_back(d_os);
        delta_os_stat.push_back(d_os_st);
        delta_os_syst.push_back(d_os_sy);
        gamma_ss.push_back(g_ss);
        gamma_ss_stat.push_back(g_ss_st);
        gamma_ss_syst.push_back(g_ss_sy);
        gamma_os.push_back(g_os);
        gamma_os_stat.push_back(g_os_st);
        gamma_os_syst.push_back(g_os_sy);
        
        // Lambda-h data
        delta_lh_ss.push_back(d_lh_ss);
        delta_lh_ss_stat.push_back(d_lh_ss_st);
        delta_lh_ss_syst.push_back(d_lh_ss_sy);
        delta_lh_os.push_back(d_lh_os);
        delta_lh_os_stat.push_back(d_lh_os_st);
        delta_lh_os_syst.push_back(d_lh_os_sy);
        gamma_lh_ss.push_back(g_lh_ss);
        gamma_lh_ss_stat.push_back(g_lh_ss_st);
        gamma_lh_ss_syst.push_back(g_lh_ss_sy);
        gamma_lh_os.push_back(g_lh_os);
        gamma_lh_os_stat.push_back(g_lh_os_st);
        gamma_lh_os_syst.push_back(g_lh_os_sy);
    }
    alice_file.close();
    
    // Compute OS-SS (deltaDelta and deltaGamma) for ALICE Lambda-p with stat error propagation
    vector<double> delta_diff_alice, delta_diff_alice_stat;
    vector<double> gamma_diff_alice, gamma_diff_alice_stat;
    delta_diff_alice.reserve(delta_os.size());
    delta_diff_alice_stat.reserve(delta_os_stat.size());
    gamma_diff_alice.reserve(gamma_os.size());
    gamma_diff_alice_stat.reserve(gamma_os_stat.size());
    for (size_t i = 0; i < delta_os.size(); ++i) {
        delta_diff_alice.push_back(delta_os[i] - delta_ss[i]);
        double err_delta = sqrt(delta_os_stat[i] * delta_os_stat[i] +
                                delta_ss_stat[i] * delta_ss_stat[i]);
        delta_diff_alice_stat.push_back(err_delta);
    }
    for (size_t i = 0; i < gamma_os.size(); ++i) {
        gamma_diff_alice.push_back(gamma_os[i] - gamma_ss[i]);
        double err_gamma = sqrt(gamma_os_stat[i] * gamma_os_stat[i] +
                                gamma_ss_stat[i] * gamma_ss_stat[i]);
        gamma_diff_alice_stat.push_back(err_gamma);
    }
    
    // Compute OS-SS for ALICE Lambda-h with stat error propagation
    vector<double> delta_lh_diff_alice, delta_lh_diff_alice_stat;
    vector<double> gamma_lh_diff_alice, gamma_lh_diff_alice_stat;
    delta_lh_diff_alice.reserve(delta_lh_os.size());
    delta_lh_diff_alice_stat.reserve(delta_lh_os_stat.size());
    gamma_lh_diff_alice.reserve(gamma_lh_os.size());
    gamma_lh_diff_alice_stat.reserve(gamma_lh_os_stat.size());
    for (size_t i = 0; i < delta_lh_os.size(); ++i) {
        delta_lh_diff_alice.push_back(delta_lh_os[i] - delta_lh_ss[i]);
        double err_delta = sqrt(delta_lh_os_stat[i] * delta_lh_os_stat[i] +
                                delta_lh_ss_stat[i] * delta_lh_ss_stat[i]);
        delta_lh_diff_alice_stat.push_back(err_delta);
    }
    for (size_t i = 0; i < gamma_lh_os.size(); ++i) {
        gamma_lh_diff_alice.push_back(gamma_lh_os[i] - gamma_lh_ss[i]);
        double err_gamma = sqrt(gamma_lh_os_stat[i] * gamma_lh_os_stat[i] +
                                gamma_lh_ss_stat[i] * gamma_lh_ss_stat[i]);
        gamma_lh_diff_alice_stat.push_back(err_gamma);
    }
    
    // Read Blast Wave data
    ifstream bw_file("blast_wave.csv");
    getline(bw_file, header); // Skip header
    
    vector<double> cent_bw, delta_ss_bw, delta_ss_bw_err;
    vector<double> delta_os_bw, delta_os_bw_err;
    vector<double> gamma_ss_bw, gamma_ss_bw_err;
    vector<double> gamma_os_bw, gamma_os_bw_err;
    
    while (getline(bw_file, line)) {
        if (line.empty()) continue;
        
        double cent, d_ss, d_ss_st, d_ss_sy, d_os, d_os_st, d_os_sy;
        double g_ss, g_ss_st, g_ss_sy, g_os, g_os_st, g_os_sy;
        
        char comma;
        stringstream ss(line);
        ss >> cent >> comma >> d_ss >> comma >> d_ss_st >> comma >> d_ss_sy >> comma
           >> d_os >> comma >> d_os_st >> comma >> d_os_sy >> comma
           >> g_ss >> comma >> g_ss_st >> comma >> g_ss_sy >> comma
           >> g_os >> comma >> g_os_st >> comma >> g_os_sy;
        
        cent_bw.push_back(cent);
        delta_ss_bw.push_back(d_ss);
        delta_ss_bw_err.push_back(d_ss_st);
        delta_os_bw.push_back(d_os);
        delta_os_bw_err.push_back(d_os_st);
        gamma_ss_bw.push_back(g_ss);
        gamma_ss_bw_err.push_back(g_ss_st);
        gamma_os_bw.push_back(g_os);
        gamma_os_bw_err.push_back(g_os_st);
    }
    bw_file.close();
    
    // Compute OS-SS for Blast Wave (stat errors only)
    vector<double> delta_diff_bw, delta_diff_bw_err;
    vector<double> gamma_diff_bw, gamma_diff_bw_err;
    delta_diff_bw.reserve(delta_os_bw.size());
    delta_diff_bw_err.reserve(delta_os_bw_err.size());
    gamma_diff_bw.reserve(gamma_os_bw.size());
    gamma_diff_bw_err.reserve(gamma_os_bw_err.size());
    for (size_t i = 0; i < delta_os_bw.size(); ++i) {
        delta_diff_bw.push_back(delta_os_bw[i] - delta_ss_bw[i]);
        double err_delta_bw = sqrt(delta_os_bw_err[i] * delta_os_bw_err[i] +
                                   delta_ss_bw_err[i] * delta_ss_bw_err[i]);
        delta_diff_bw_err.push_back(err_delta_bw);
    }
    for (size_t i = 0; i < gamma_os_bw.size(); ++i) {
        gamma_diff_bw.push_back(gamma_os_bw[i] - gamma_ss_bw[i]);
        double err_gamma_bw = sqrt(gamma_os_bw_err[i] * gamma_os_bw_err[i] +
                                   gamma_ss_bw_err[i] * gamma_ss_bw_err[i]);
        gamma_diff_bw_err.push_back(err_gamma_bw);
    }
    
    // Read AMPT data
    ifstream ampt_file("ampt.csv");
    getline(ampt_file, header); // Skip header
    
    vector<double> cent_ampt, delta_ss_ampt, delta_ss_ampt_err;
    vector<double> delta_os_ampt, delta_os_ampt_err;
    vector<double> gamma_ss_ampt, gamma_ss_ampt_err;
    vector<double> gamma_os_ampt, gamma_os_ampt_err;
    
    // Lambda-h data vectors for AMPT
    vector<double> delta_lh_ss_ampt, delta_lh_ss_ampt_err;
    vector<double> delta_lh_os_ampt, delta_lh_os_ampt_err;
    vector<double> gamma_lh_ss_ampt, gamma_lh_ss_ampt_err;
    vector<double> gamma_lh_os_ampt, gamma_lh_os_ampt_err;
    
    while (getline(ampt_file, line)) {
        if (line.empty()) continue;
        
        double cent, d_ss, d_ss_st, d_ss_sy, d_os, d_os_st, d_os_sy;
        double d_lh_os, d_lh_os_st, d_lh_os_sy, d_lh_ss, d_lh_ss_st, d_lh_ss_sy;
        double g_ss, g_ss_st, g_ss_sy, g_os, g_os_st, g_os_sy;
        double g_lh_os, g_lh_os_st, g_lh_os_sy, g_lh_ss, g_lh_ss_st, g_lh_ss_sy;
        
        char comma;
        stringstream ss(line);
        ss >> cent >> comma >> d_ss >> comma >> d_ss_st >> comma >> d_ss_sy >> comma
           >> d_os >> comma >> d_os_st >> comma >> d_os_sy >> comma
           >> d_lh_os >> comma >> d_lh_os_st >> comma >> d_lh_os_sy >> comma
           >> d_lh_ss >> comma >> d_lh_ss_st >> comma >> d_lh_ss_sy >> comma
           >> g_ss >> comma >> g_ss_st >> comma >> g_ss_sy >> comma
           >> g_os >> comma >> g_os_st >> comma >> g_os_sy >> comma
           >> g_lh_os >> comma >> g_lh_os_st >> comma >> g_lh_os_sy >> comma
           >> g_lh_ss >> comma >> g_lh_ss_st >> comma >> g_lh_ss_sy;
        
        cent_ampt.push_back(cent);
        delta_ss_ampt.push_back(d_ss);
        delta_ss_ampt_err.push_back(d_ss_st);
        delta_os_ampt.push_back(d_os);
        delta_os_ampt_err.push_back(d_os_st);
        gamma_ss_ampt.push_back(g_ss);
        gamma_ss_ampt_err.push_back(g_ss_st);
        gamma_os_ampt.push_back(g_os);
        gamma_os_ampt_err.push_back(g_os_st);
        
        // Lambda-h data for AMPT
        delta_lh_ss_ampt.push_back(d_lh_ss);
        delta_lh_ss_ampt_err.push_back(d_lh_ss_st);
        delta_lh_os_ampt.push_back(d_lh_os);
        delta_lh_os_ampt_err.push_back(d_lh_os_st);
        gamma_lh_ss_ampt.push_back(g_lh_ss);
        gamma_lh_ss_ampt_err.push_back(g_lh_ss_st);
        gamma_lh_os_ampt.push_back(g_lh_os);
        gamma_lh_os_ampt_err.push_back(g_lh_os_st);
    }
    ampt_file.close();
    
    // Compute OS-SS for AMPT Lambda-p (stat errors only)
    vector<double> delta_diff_ampt, delta_diff_ampt_err;
    vector<double> gamma_diff_ampt, gamma_diff_ampt_err;
    delta_diff_ampt.reserve(delta_os_ampt.size());
    delta_diff_ampt_err.reserve(delta_os_ampt_err.size());
    gamma_diff_ampt.reserve(gamma_os_ampt.size());
    gamma_diff_ampt_err.reserve(gamma_os_ampt_err.size());
    for (size_t i = 0; i < delta_os_ampt.size(); ++i) {
        delta_diff_ampt.push_back(delta_os_ampt[i] - delta_ss_ampt[i]);
        double err_delta_ampt = sqrt(delta_os_ampt_err[i] * delta_os_ampt_err[i] +
                                     delta_ss_ampt_err[i] * delta_ss_ampt_err[i]);
        delta_diff_ampt_err.push_back(err_delta_ampt);
    }
    for (size_t i = 0; i < gamma_os_ampt.size(); ++i) {
        gamma_diff_ampt.push_back(gamma_os_ampt[i] - gamma_ss_ampt[i]);
        double err_gamma_ampt = sqrt(gamma_os_ampt_err[i] * gamma_os_ampt_err[i] +
                                     gamma_ss_ampt_err[i] * gamma_ss_ampt_err[i]);
        gamma_diff_ampt_err.push_back(err_gamma_ampt);
    }
    
    // Compute OS-SS for AMPT Lambda-h (stat errors only)
    vector<double> delta_lh_diff_ampt, delta_lh_diff_ampt_err;
    vector<double> gamma_lh_diff_ampt, gamma_lh_diff_ampt_err;
    delta_lh_diff_ampt.reserve(delta_lh_os_ampt.size());
    delta_lh_diff_ampt_err.reserve(delta_lh_os_ampt_err.size());
    gamma_lh_diff_ampt.reserve(gamma_lh_os_ampt.size());
    gamma_lh_diff_ampt_err.reserve(gamma_lh_os_ampt_err.size());
    for (size_t i = 0; i < delta_lh_os_ampt.size(); ++i) {
        delta_lh_diff_ampt.push_back(delta_lh_os_ampt[i] - delta_lh_ss_ampt[i]);
        double err_delta_ampt = sqrt(delta_lh_os_ampt_err[i] * delta_lh_os_ampt_err[i] +
                                     delta_lh_ss_ampt_err[i] * delta_lh_ss_ampt_err[i]);
        delta_lh_diff_ampt_err.push_back(err_delta_ampt);
    }
    for (size_t i = 0; i < gamma_lh_os_ampt.size(); ++i) {
        gamma_lh_diff_ampt.push_back(gamma_lh_os_ampt[i] - gamma_lh_ss_ampt[i]);
        double err_gamma_ampt = sqrt(gamma_lh_os_ampt_err[i] * gamma_lh_os_ampt_err[i] +
                                     gamma_lh_ss_ampt_err[i] * gamma_lh_ss_ampt_err[i]);
        gamma_lh_diff_ampt_err.push_back(err_gamma_ampt);
    }
    
    // Upper panel - Delta observables
    pad1->cd();
    
    // Create frame for upper panel  
    TH1F *frame1 = pad1->DrawFrame(0, -0.001, 60, 0.013);
    frame1->GetXaxis()->SetLabelSize(0);
    frame1->GetYaxis()->SetTitle("#Delta#delta (OS-SS)");
    // frame1->GetYaxis()->SetTitleSize(0.06);
    frame1->GetYaxis()->SetTitleOffset(1.0);
    // frame1->GetYaxis()->SetLabelSize(0.05);
    frame1->GetYaxis()->SetMaxDigits(2);
    frame1->GetYaxis()->SetNdivisions(505);
    
    // OS-SS (deltaDelta) for ALICE Lambda-p
    TGraphErrors *gr_delta_diff_alice = new TGraphErrors(cent_alice.size(),
                                                         cent_alice.data(),
                                                         delta_diff_alice.data(),
                                                         nullptr,
                                                         delta_diff_alice_stat.data());
    gr_delta_diff_alice->SetMarkerStyle(20);
    gr_delta_diff_alice->SetMarkerColor(kRed+1);
    gr_delta_diff_alice->SetLineColor(kRed+1);
    gr_delta_diff_alice->SetMarkerSize(1.2);
    
    // OS-SS (deltaDelta) for ALICE Lambda-h
    TGraphErrors *gr_delta_lh_diff_alice = new TGraphErrors(cent_alice.size(),
                                                            cent_alice.data(),
                                                            delta_lh_diff_alice.data(),
                                                            nullptr,
                                                            delta_lh_diff_alice_stat.data());
    gr_delta_lh_diff_alice->SetMarkerStyle(21);
    gr_delta_lh_diff_alice->SetMarkerColor(kAzure+4);
    gr_delta_lh_diff_alice->SetLineColor(kAzure+4);
    gr_delta_lh_diff_alice->SetMarkerSize(1.2);
    
    // OS-SS band for Blast Wave Lambda-p
    TGraphErrors *gr_delta_diff_bw = new TGraphErrors(cent_bw.size(),
                                                      cent_bw.data(),
                                                      delta_diff_bw.data(),
                                                      nullptr,
                                                      delta_diff_bw_err.data());
    gr_delta_diff_bw->SetFillColorAlpha(kGreen+2, 0.3);
    gr_delta_diff_bw->SetFillStyle(1001);
    gr_delta_diff_bw->SetLineColor(kGreen+2);
    gr_delta_diff_bw->SetLineWidth(2);
    
    // OS-SS band for AMPT Lambda-p
    TGraphErrors *gr_delta_diff_ampt = new TGraphErrors(cent_ampt.size(),
                                                        cent_ampt.data(),
                                                        delta_diff_ampt.data(),
                                                        nullptr,
                                                        delta_diff_ampt_err.data());
    gr_delta_diff_ampt->SetFillColorAlpha(kOrange+7, 0.3);
    gr_delta_diff_ampt->SetFillStyle(1001);
    gr_delta_diff_ampt->SetLineColor(kOrange+7);
    gr_delta_diff_ampt->SetLineWidth(2);
    gr_delta_diff_ampt->SetLineStyle(2);
    
    // OS-SS band for AMPT Lambda-h
    TGraphErrors *gr_delta_lh_diff_ampt = new TGraphErrors(cent_ampt.size(),
                                                           cent_ampt.data(),
                                                           delta_lh_diff_ampt.data(),
                                                           nullptr,
                                                           delta_lh_diff_ampt_err.data());
    gr_delta_lh_diff_ampt->SetFillColorAlpha(kAzure+4, 0.3);
    gr_delta_lh_diff_ampt->SetFillStyle(1001);
    gr_delta_lh_diff_ampt->SetLineColor(kAzure+4);
    gr_delta_lh_diff_ampt->SetLineWidth(2);
    gr_delta_lh_diff_ampt->SetLineStyle(2);
    
    // Draw bands first (background)
    gr_delta_diff_bw->Draw("E3");
    gr_delta_diff_ampt->Draw("E3 same");
    gr_delta_lh_diff_ampt->Draw("E3 same");
    
    // Draw experimental points on top
    gr_delta_diff_alice->Draw("P same");
    gr_delta_lh_diff_alice->Draw("P same");
    
    // Add legend for upper panel
    TLegend *leg1 = new TLegend(0.20, 0.60, 0.7, 0.8);
    leg1->SetBorderSize(0);
    leg1->SetFillStyle(0);
    // leg1->SetTextSize(0.04);
    leg1->SetNColumns(2);
    leg1->AddEntry(gr_delta_diff_alice, "ALICE Prelim. #Lambda-p", "p");
    leg1->AddEntry(gr_delta_lh_diff_alice, "ALICE Prelim. #Lambda-h", "p");
    leg1->AddEntry(gr_delta_diff_ampt, "AMPT-SM #Lambda-p", "f");
    leg1->AddEntry(gr_delta_lh_diff_ampt, "AMPT-SM #Lambda-h", "f");
    leg1->AddEntry(gr_delta_diff_bw, "BW-LBC #Lambda-p", "f");
    leg1->AddEntry((TObject*)0, "", "");
    leg1->Draw();
    
    // // Add text
    // TLatex latex;
    // // latex.SetTextSize(0.05);
    // latex.DrawLatexNDC(0.2, 0.80, "Pb-Pb #sqrt{s_{NN}} = 5.02 TeV");
    
    // Lower panel - Gamma observables
    pad2->cd();
    
    // Create frame for lower panel
    TH1F *frame2 = pad2->DrawFrame(0, -0.0004, 60, 0.0035);
    frame2->GetXaxis()->SetTitle("Centrality (%)");
    // frame2->GetXaxis()->SetTitleSize(0.06);
    frame2->GetXaxis()->SetTitleOffset(0.9);
    // frame2->GetXaxis()->SetLabelSize(0.05);
    frame2->GetYaxis()->SetTitle("#Delta#gamma (OS-SS)");
    // frame2->GetYaxis()->SetTitleSize(0.06);
    frame2->GetYaxis()->SetTitleOffset(1.0);
    // frame2->GetYaxis()->SetLabelSize(0.05);
    frame2->GetYaxis()->SetMaxDigits(2);
    frame2->GetYaxis()->SetNdivisions(505);
    
    // OS-SS (deltaGamma) for ALICE Lambda-p
    TGraphErrors *gr_gamma_diff_alice = new TGraphErrors(cent_alice.size(),
                                                         cent_alice.data(),
                                                         gamma_diff_alice.data(),
                                                         nullptr,
                                                         gamma_diff_alice_stat.data());
    gr_gamma_diff_alice->SetMarkerStyle(20);
    gr_gamma_diff_alice->SetMarkerColor(kRed+1);
    gr_gamma_diff_alice->SetLineColor(kRed+1);
    gr_gamma_diff_alice->SetMarkerSize(1.2);
    
    // OS-SS (deltaGamma) for ALICE Lambda-h
    TGraphErrors *gr_gamma_lh_diff_alice = new TGraphErrors(cent_alice.size(),
                                                            cent_alice.data(),
                                                            gamma_lh_diff_alice.data(),
                                                            nullptr,
                                                            gamma_lh_diff_alice_stat.data());
    gr_gamma_lh_diff_alice->SetMarkerStyle(21);
    gr_gamma_lh_diff_alice->SetMarkerColor(kAzure+4);
    gr_gamma_lh_diff_alice->SetLineColor(kAzure+4);
    gr_gamma_lh_diff_alice->SetMarkerSize(1.2);
    
    // OS-SS band for Blast Wave Lambda-p
    TGraphErrors *gr_gamma_diff_bw = new TGraphErrors(cent_bw.size(),
                                                      cent_bw.data(),
                                                      gamma_diff_bw.data(),
                                                      nullptr,
                                                      gamma_diff_bw_err.data());
    gr_gamma_diff_bw->SetFillColorAlpha(kGreen+2, 0.3);
    gr_gamma_diff_bw->SetFillStyle(1001);
    gr_gamma_diff_bw->SetLineColor(kGreen+2);
    gr_gamma_diff_bw->SetLineWidth(2);
    
    // OS-SS band for AMPT Lambda-p
    TGraphErrors *gr_gamma_diff_ampt = new TGraphErrors(cent_ampt.size(),
                                                        cent_ampt.data(),
                                                        gamma_diff_ampt.data(),
                                                        nullptr,
                                                        gamma_diff_ampt_err.data());
    gr_gamma_diff_ampt->SetFillColorAlpha(kOrange+7, 0.3);
    gr_gamma_diff_ampt->SetFillStyle(1001);
    gr_gamma_diff_ampt->SetLineColor(kOrange+7);
    gr_gamma_diff_ampt->SetLineWidth(2);
    gr_gamma_diff_ampt->SetLineStyle(2);
    
    // OS-SS band for AMPT Lambda-h
    TGraphErrors *gr_gamma_lh_diff_ampt = new TGraphErrors(cent_ampt.size(),
                                                           cent_ampt.data(),
                                                           gamma_lh_diff_ampt.data(),
                                                           nullptr,
                                                           gamma_lh_diff_ampt_err.data());
    gr_gamma_lh_diff_ampt->SetFillColorAlpha(kAzure+4, 0.3);
    gr_gamma_lh_diff_ampt->SetFillStyle(1001);
    gr_gamma_lh_diff_ampt->SetLineColor(kAzure+4);
    gr_gamma_lh_diff_ampt->SetLineWidth(2);
    gr_gamma_lh_diff_ampt->SetLineStyle(2);
    
    // Draw bands first (background)
    gr_gamma_diff_bw->Draw("E3");
    gr_gamma_diff_ampt->Draw("E3 same");
    gr_gamma_lh_diff_ampt->Draw("E3 same");
    
    // Draw experimental points on top
    gr_gamma_diff_alice->Draw("P same");
    gr_gamma_lh_diff_alice->Draw("P same");
    
    // No legend for lower panel
    
    // Update canvas
    c1->Update();
    
    // Save to file
    c1->SaveAs("cve_results.pdf");
    c1->SaveAs("cve_results.png");
    
    cout << "Plot saved as cve_results.pdf and cve_results.png" << endl;
}