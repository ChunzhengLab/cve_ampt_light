#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

void setROOTStyle() {
    gROOT->SetStyle("Plain");
    gStyle->SetOptStat(0);
    gStyle->SetOptFit(0);
    gStyle->SetOptTitle(0);
    gStyle->SetPadTickX(1);
    gStyle->SetPadTickY(1);
}

void plot_lambda_feeddown() {
    setROOTStyle();
    
    // Create main canvas with 2 panels
    TCanvas *c = new TCanvas("c", "Lambda Feeddown Analysis", 800, 800);
    
    // Create two pads for upper and lower panels
    TPad *pad1 = new TPad("pad1", "upper", 0.0, 0.3, 1.0, 1.0);
    TPad *pad2 = new TPad("pad2", "lower", 0.0, 0.0, 1.0, 0.3);
    
    // Set margins for shared x-axis
    pad1->SetTopMargin(0.08);
    pad1->SetBottomMargin(0.0);  // No bottom margin for upper pad
    pad1->SetLeftMargin(0.12);
    pad1->SetRightMargin(0.05);
    
    pad2->SetTopMargin(0.0);     // No top margin for lower pad
    pad2->SetBottomMargin(0.35);
    pad2->SetLeftMargin(0.12);
    pad2->SetRightMargin(0.05);
    
    pad1->Draw();
    pad2->Draw();
    
    // Upper panel - correlations
    pad1->cd();
    
    // Read Lambda-Lambda data
    vector<double> cent_ll, val_ll, err_ll;
    ifstream file_ll("lambda_feeddown_ratio_lambda_lambda.csv");
    string line;
    getline(file_ll, line); // skip header
    while (getline(file_ll, line)) {
        stringstream ss(line);
        string label;
        double cent, val, err;
        char comma;
        
        getline(ss, label, ',');
        ss >> cent >> comma >> val >> comma >> err;
        
        cent_ll.push_back(cent);
        val_ll.push_back(val);
        err_ll.push_back(err);
    }
    file_ll.close();
    
    // Read p from Lambda data
    vector<double> cent_p, val_p, err_p;
    ifstream file_p("lambda_feeddown_ratio_p_from_lambda.csv");
    getline(file_p, line); // skip header
    while (getline(file_p, line)) {
        stringstream ss(line);
        string label;
        double cent, val, err;
        char comma;
        
        getline(ss, label, ',');
        ss >> cent >> comma >> val >> comma >> err;
        
        cent_p.push_back(cent - 1); // Offset p points to the left
        val_p.push_back(val);
        err_p.push_back(err);
    }
    file_p.close();
    
    // Read pi from Lambda data
    vector<double> cent_pi, val_pi, err_pi;
    ifstream file_pi("lambda_feeddown_ratio_pi_from_lambda.csv");
    getline(file_pi, line); // skip header
    while (getline(file_pi, line)) {
        stringstream ss(line);
        string label;
        double cent, val, err;
        char comma;
        
        getline(ss, label, ',');
        ss >> cent >> comma >> val >> comma >> err;
        
        cent_pi.push_back(cent + 1); // Offset pi points to the right
        val_pi.push_back(val);
        err_pi.push_back(err);
    }
    file_pi.close();
    
    // Create frame for upper panel
    TH1F *frame1 = pad1->DrawFrame(0, -0.009, 60, 0.001);
    frame1->GetYaxis()->SetTitle("#delta");
    frame1->GetYaxis()->SetTitleOffset(1.0);
    frame1->GetYaxis()->SetLabelSize(0.05);
    frame1->GetYaxis()->SetTitleSize(0.05);
    frame1->GetXaxis()->SetLabelSize(0);  // Hide x-axis labels for upper panel
    frame1->GetYaxis()->SetMaxDigits(2);
    frame1->GetYaxis()->SetNdivisions(505);
    
    // Create graphs
    TGraphErrors *gr_ll = new TGraphErrors(cent_ll.size(), cent_ll.data(), val_ll.data(), 
                                          nullptr, err_ll.data());
    gr_ll->SetMarkerColor(kBlue+1);
    gr_ll->SetLineColor(kBlue+1);
    gr_ll->SetMarkerStyle(20);
    gr_ll->SetMarkerSize(1.2);
    
    TGraphErrors *gr_p = new TGraphErrors(cent_p.size(), cent_p.data(), val_p.data(), 
                                         nullptr, err_p.data());
    gr_p->SetMarkerColor(kMagenta+1);
    gr_p->SetLineColor(kMagenta+1);
    gr_p->SetMarkerStyle(21);
    gr_p->SetMarkerSize(1.2);
    
    TGraphErrors *gr_pi = new TGraphErrors(cent_pi.size(), cent_pi.data(), val_pi.data(), 
                                          nullptr, err_pi.data());
    gr_pi->SetMarkerColor(kOrange+7);
    gr_pi->SetLineColor(kOrange+7);
    gr_pi->SetMarkerStyle(22);
    gr_pi->SetMarkerSize(1.2);
    
    // Draw graphs - only points, no lines (like in plot_cve_data.C)
    gr_ll->Draw("P");
    gr_p->Draw("P same");
    gr_pi->Draw("P same");
    
    // Add legend
    TLegend *leg1 = new TLegend(0.55, 0.15, 0.92, 0.40);
    leg1->SetBorderSize(0);
    leg1->SetFillStyle(0);
    leg1->SetTextSize(0.04);
    leg1->AddEntry(gr_ll, "#Lambda - #Lambda", "lp");
    leg1->AddEntry(gr_p, "#Lambda - p from #Lambda", "lp");
    leg1->AddEntry(gr_pi, "#Lambda - #pi- from #Lambda", "lp");
    leg1->Draw();
    
    // Lower panel - ratios
    pad2->cd();

    // Read bootstrap errors for p ratio
    vector<double> cent_bootstrap, err_p_bootstrap;
    ifstream file_bootstrap("lambda_feeddown_bootstrap_errors.csv");
    getline(file_bootstrap, line); // skip header
    while (getline(file_bootstrap, line)) {
        stringstream ss(line);
        double cent, p_err;
        char comma;

        ss >> cent >> comma >> p_err;

        cent_bootstrap.push_back(cent);
        err_p_bootstrap.push_back(p_err);
    }
    file_bootstrap.close();

    // Calculate p/Lambda ratios from the original data
    vector<double> cent_p_ratio, val_p_ratio, err_p_ratio;

    // Calculate ratios for each centrality bin
    for (int i = 0; i < cent_ll.size(); i++) {
        double cent = cent_ll[i];
        double ll_val = val_ll[i];

        // Find corresponding p values at this centrality
        for (int j = 0; j < cent_p.size(); j++) {
            if (abs(cent_p[j] - (cent - 1)) < 0.1) { // Match offset centrality
                double p_val = val_p[j];

                if (ll_val != 0) { // Avoid division by zero
                    double ratio = p_val / ll_val;

                    // Find bootstrap error for this centrality
                    double ratio_err = 0;
                    for (int k = 0; k < cent_bootstrap.size(); k++) {
                        if (abs(cent_bootstrap[k] - cent) < 0.1) {
                            ratio_err = err_p_bootstrap[k];
                            break;
                        }
                    }

                    cent_p_ratio.push_back(cent - 1); // Keep same offset as upper panel
                    val_p_ratio.push_back(ratio);
                    err_p_ratio.push_back(ratio_err);
                }
                break;
            }
        }
    }
    
    // Create frame for lower panel
    TH1F *frame2 = pad2->DrawFrame(0, 0.5, 60, 1.5);
    frame2->GetXaxis()->SetTitle("Centrality (%)");
    frame2->GetXaxis()->SetTitleOffset(1.0);
    frame2->GetXaxis()->SetLabelSize(0.12);
    frame2->GetXaxis()->SetTitleSize(0.12);
    frame2->GetYaxis()->SetTitle("Ratio to #Lambda - #Lambda");
    frame2->GetYaxis()->SetTitleOffset(0.4);
    frame2->GetYaxis()->SetLabelSize(0.12);
    frame2->GetYaxis()->SetTitleSize(0.12);
    frame2->GetYaxis()->SetNdivisions(505);
    
    // Create ratio graph for p only
    TGraphErrors *gr_p_ratio = new TGraphErrors(cent_p_ratio.size(), cent_p_ratio.data(),
                                               val_p_ratio.data(), nullptr, err_p_ratio.data());
    gr_p_ratio->SetLineColor(kMagenta+1);
    gr_p_ratio->SetMarkerColor(kMagenta+1);
    gr_p_ratio->SetMarkerStyle(21);
    gr_p_ratio->SetMarkerSize(1.2);
    gr_p_ratio->SetLineWidth(2);

    // Draw ratio graph
    gr_p_ratio->Draw("P");

    // Perform fit on the ratio data (using same range as original: 0-55)
    TF1 *fit_p = new TF1("fit_p", "[0]", 0, 55);
    fit_p->SetLineColor(kMagenta+1);
    fit_p->SetLineStyle(2);
    fit_p->SetLineWidth(2);
    gr_p_ratio->Fit(fit_p, "Q");  // Quiet fit

    // Get fit results
    double p_fit_new = fit_p->GetParameter(0);
    double p_fit_err_new = fit_p->GetParError(0);

    fit_p->Draw("same");
    
    // Draw reference line at ratio = 1
    TLine *line1 = new TLine(0, 1.0, 60, 1.0);
    line1->SetLineStyle(3);
    line1->SetLineColor(kGray+1);
    line1->Draw();
    
    // Add legend for lower panel
    TLegend *leg2 = new TLegend(0.15, 0.70, 0.65, 0.90);
    leg2->SetBorderSize(0);
    leg2->SetFillStyle(0);
    leg2->SetTextSize(0.10);
    leg2->AddEntry(gr_p_ratio, Form("p ratio = %.4f #pm %.5f", p_fit_new, p_fit_err_new), "lp");
    leg2->Draw();
    
    // Update canvas
    c->Update();
    
    // Save to files
    c->SaveAs("lambda_feeddown_analysis.pdf");
    c->SaveAs("lambda_feeddown_analysis.png");
    
    cout << "Lambda feeddown analysis plots saved." << endl;
}