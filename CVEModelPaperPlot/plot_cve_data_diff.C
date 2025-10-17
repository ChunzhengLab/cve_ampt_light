#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>

void setROOTStyle() {
    gROOT->SetStyle("Plain");
    gStyle->SetOptStat(0);
    gStyle->SetOptFit(0);
    gStyle->SetOptTitle(0);
    gStyle->SetPadTickX(1);
    gStyle->SetPadTickY(1);
}

void plotPanel_SumPtDeltaDelta(bool drawAMPT = true, bool showXLabels = true, bool showYLabels = true) {
    
    // Read BW data
    ifstream bw_file("diff_sumPt_blastwave.csv");
    string header;
    getline(bw_file, header);
    
    vector<double> cent_bw_1, dd_bw_1, dd_err_bw_1;
    vector<double> cent_bw_3, dd_bw_3, dd_err_bw_3;
    vector<double> cent_bw_5, dd_bw_5, dd_err_bw_5;
    
    string line;
    while (getline(bw_file, line)) {
        if (line.empty()) continue;
        
        stringstream ss(line);
        string tag, bin_label;
        double cent, dd, dd_err, dg, dg_err;
        char comma;
        
        getline(ss, tag, ',');
        getline(ss, bin_label, ',');
        ss >> cent >> comma >> dd >> comma >> dd_err >> comma >> dg >> comma >> dg_err;
        
        if (dd == 0.0) continue;
        
        if (bin_label.find("1 < #Sigma p_{T} < 3") != string::npos) {
            cent_bw_1.push_back(cent);
            dd_bw_1.push_back(dd);
            dd_err_bw_1.push_back(dd_err);
        } else if (bin_label.find("3 < #Sigma p_{T} < 5") != string::npos) {
            cent_bw_3.push_back(cent);
            dd_bw_3.push_back(dd);
            dd_err_bw_3.push_back(dd_err);
        } else if (bin_label.find("5 < #Sigma p_{T} < 8") != string::npos) {
            cent_bw_5.push_back(cent);
            dd_bw_5.push_back(dd);
            dd_err_bw_5.push_back(dd_err);
        }
    }
    bw_file.close();
    
    // Read AMPT data (optional)
    vector<double> cent_ampt_1, dd_ampt_1, dd_err_ampt_1;
    vector<double> cent_ampt_3, dd_ampt_3, dd_err_ampt_3;
    vector<double> cent_ampt_5, dd_ampt_5, dd_err_ampt_5;
    
    if (drawAMPT) {
        ifstream ampt_file("diff_sumPt_ampt.csv");
        getline(ampt_file, header);
        
        while (getline(ampt_file, line)) {
            if (line.empty()) continue;
            
            stringstream ss(line);
            string tag, bin_label;
            double cent, dd, dd_err, dg, dg_err;
            char comma;
            
            getline(ss, tag, ',');
            getline(ss, bin_label, ',');
            ss >> cent >> comma >> dd >> comma >> dd_err >> comma >> dg >> comma >> dg_err;
            
            if (dd == 0.0) continue;
            
            if (bin_label.find("1 < #Sigma p_{T} < 3") != string::npos) {
                cent_ampt_1.push_back(cent);
                dd_ampt_1.push_back(dd);
                dd_err_ampt_1.push_back(dd_err);
            } else if (bin_label.find("3 < #Sigma p_{T} < 5") != string::npos) {
                cent_ampt_3.push_back(cent);
                dd_ampt_3.push_back(dd);
                dd_err_ampt_3.push_back(dd_err);
            } else if (bin_label.find("5 < #Sigma p_{T} < 8") != string::npos) {
                cent_ampt_5.push_back(cent);
                dd_ampt_5.push_back(dd);
                dd_err_ampt_5.push_back(dd_err);
            }
        }
        ampt_file.close();
    }
    
    // Create frame - sumPt DeltaDelta: original range * 1.5
    TH1F *frame = gPad->DrawFrame(0, -0.0015, 59.9, 0.0195);
    if (showXLabels) {
        frame->GetXaxis()->SetTitle("Centrality (%)");
        frame->GetXaxis()->SetTitleOffset(1.2);
        frame->GetXaxis()->SetLabelSize(0.05);
        frame->GetXaxis()->SetTitleSize(0.05);
    } else {
        frame->GetXaxis()->SetLabelSize(0);
        frame->GetXaxis()->SetTitle("");
    }
    if (showYLabels) {
        frame->GetYaxis()->SetTitle("#Delta#delta (OS-SS)");
        frame->GetYaxis()->SetTitleOffset(1.2);
        frame->GetYaxis()->SetLabelSize(0.05);
        frame->GetYaxis()->SetTitleSize(0.05);
    } else {
        frame->GetYaxis()->SetLabelSize(0);
        frame->GetYaxis()->SetTitle("");
    }
    frame->GetYaxis()->SetMaxDigits(2);
    
    // Create graphs for BW - red, blue, green (opaque)
    TGraphErrors *gr_bw_1 = new TGraphErrors(cent_bw_1.size(), cent_bw_1.data(), dd_bw_1.data(), nullptr, dd_err_bw_1.data());
    gr_bw_1->SetFillColor(kRed+1);
    gr_bw_1->SetLineColor(kRed+1);
    gr_bw_1->SetLineWidth(2);
    
    TGraphErrors *gr_bw_3 = new TGraphErrors(cent_bw_3.size(), cent_bw_3.data(), dd_bw_3.data(), nullptr, dd_err_bw_3.data());
    gr_bw_3->SetFillColor(kBlue+1);
    gr_bw_3->SetLineColor(kBlue+1);
    gr_bw_3->SetLineWidth(2);
    
    TGraphErrors *gr_bw_5 = new TGraphErrors(cent_bw_5.size(), cent_bw_5.data(), dd_bw_5.data(), nullptr, dd_err_bw_5.data());
    gr_bw_5->SetFillColor(kGreen+2);
    gr_bw_5->SetLineColor(kGreen+2);
    gr_bw_5->SetLineWidth(2);
    
    // Draw BW graphs - largest first (5-8 GeV first)
    gr_bw_5->Draw("E3");
    gr_bw_3->Draw("E3 same");
    gr_bw_1->Draw("E3 same");
    
    // Create and draw AMPT graphs if enabled
    TGraphErrors *gr_ampt_1 = nullptr;
    TGraphErrors *gr_ampt_3 = nullptr;
    TGraphErrors *gr_ampt_5 = nullptr;
    
    if (drawAMPT) {
        gr_ampt_1 = new TGraphErrors(cent_ampt_1.size(), cent_ampt_1.data(), dd_ampt_1.data(), nullptr, dd_err_ampt_1.data());
        gr_ampt_1->SetFillColorAlpha(kOrange+1, 0.2);
        gr_ampt_1->SetLineColor(kOrange+1);
        gr_ampt_1->SetLineWidth(2);
        gr_ampt_1->SetLineStyle(2);
        
        gr_ampt_3 = new TGraphErrors(cent_ampt_3.size(), cent_ampt_3.data(), dd_ampt_3.data(), nullptr, dd_err_ampt_3.data());
        gr_ampt_3->SetFillColorAlpha(kMagenta+1, 0.2);
        gr_ampt_3->SetLineColor(kMagenta+1);
        gr_ampt_3->SetLineWidth(2);
        gr_ampt_3->SetLineStyle(2);
        
        gr_ampt_5 = new TGraphErrors(cent_ampt_5.size(), cent_ampt_5.data(), dd_ampt_5.data(), nullptr, dd_err_ampt_5.data());
        gr_ampt_5->SetFillColorAlpha(kCyan+1, 0.2);
        gr_ampt_5->SetLineColor(kCyan+1);
        gr_ampt_5->SetLineWidth(2);
        gr_ampt_5->SetLineStyle(2);
        
        gr_ampt_5->Draw("E3 same");
        gr_ampt_3->Draw("E3 same");
        gr_ampt_1->Draw("E3 same");
    }
    
    // Create legend only for upper left panel
    if (!showXLabels && showYLabels) {
        TLegend *leg = new TLegend(0.15, 0.65, 0.8, 0.85);
        leg->SetBorderSize(0);
        leg->SetFillStyle(0);
        leg->SetNColumns(drawAMPT ? 2 : 1);
        
        leg->AddEntry(gr_bw_5, "BW-LBC 5<#Sigma p_{T}<8 GeV", "f");
        if (drawAMPT) leg->AddEntry(gr_ampt_5, "AMPT-SM 5<#Sigma p_{T}<8 GeV", "f");
        
        leg->AddEntry(gr_bw_3, "BW-LBC 3<#Sigma p_{T}<5 GeV", "f");
        if (drawAMPT) leg->AddEntry(gr_ampt_3, "AMPT-SM 3<#Sigma p_{T}<5 GeV", "f");
        
        leg->AddEntry(gr_bw_1, "BW-LBC 1<#Sigma p_{T}<3 GeV", "f");
        if (drawAMPT) leg->AddEntry(gr_ampt_1, "AMPT-SM 1<#Sigma p_{T}<3 GeV", "f");
        
        leg->Draw();
    }
}

void plotPanel_SumPtDeltaGamma(bool drawAMPT = true, bool showXLabels = true, bool showYLabels = true) {
    
    // Read BW data
    ifstream bw_file("diff_sumPt_blastwave.csv");
    string header;
    getline(bw_file, header);
    
    vector<double> cent_bw_1, dg_bw_1, dg_err_bw_1;
    vector<double> cent_bw_3, dg_bw_3, dg_err_bw_3;
    vector<double> cent_bw_5, dg_bw_5, dg_err_bw_5;
    
    string line;
    while (getline(bw_file, line)) {
        if (line.empty()) continue;
        
        stringstream ss(line);
        string tag, bin_label;
        double cent, dd, dd_err, dg, dg_err;
        char comma;
        
        getline(ss, tag, ',');
        getline(ss, bin_label, ',');
        ss >> cent >> comma >> dd >> comma >> dd_err >> comma >> dg >> comma >> dg_err;
        
        if (dg == 0.0) continue;
        
        if (bin_label.find("1 < #Sigma p_{T} < 3") != string::npos) {
            cent_bw_1.push_back(cent);
            dg_bw_1.push_back(dg);
            dg_err_bw_1.push_back(dg_err);
        } else if (bin_label.find("3 < #Sigma p_{T} < 5") != string::npos) {
            cent_bw_3.push_back(cent);
            dg_bw_3.push_back(dg);
            dg_err_bw_3.push_back(dg_err);
        } else if (bin_label.find("5 < #Sigma p_{T} < 8") != string::npos) {
            cent_bw_5.push_back(cent);
            dg_bw_5.push_back(dg);
            dg_err_bw_5.push_back(dg_err);
        }
    }
    bw_file.close();
    
    // Read AMPT data (optional)
    vector<double> cent_ampt_1, dg_ampt_1, dg_err_ampt_1;
    vector<double> cent_ampt_3, dg_ampt_3, dg_err_ampt_3;
    vector<double> cent_ampt_5, dg_ampt_5, dg_err_ampt_5;
    
    if (drawAMPT) {
        ifstream ampt_file("diff_sumPt_ampt.csv");
        getline(ampt_file, header);
        
        while (getline(ampt_file, line)) {
            if (line.empty()) continue;
            
            stringstream ss(line);
            string tag, bin_label;
            double cent, dd, dd_err, dg, dg_err;
            char comma;
            
            getline(ss, tag, ',');
            getline(ss, bin_label, ',');
            ss >> cent >> comma >> dd >> comma >> dd_err >> comma >> dg >> comma >> dg_err;
            
            if (dg == 0.0) continue;
            
            if (bin_label.find("1 < #Sigma p_{T} < 3") != string::npos) {
                cent_ampt_1.push_back(cent);
                dg_ampt_1.push_back(dg);
                dg_err_ampt_1.push_back(dg_err);
            } else if (bin_label.find("3 < #Sigma p_{T} < 5") != string::npos) {
                cent_ampt_3.push_back(cent);
                dg_ampt_3.push_back(dg);
                dg_err_ampt_3.push_back(dg_err);
            } else if (bin_label.find("5 < #Sigma p_{T} < 8") != string::npos) {
                cent_ampt_5.push_back(cent);
                dg_ampt_5.push_back(dg);
                dg_err_ampt_5.push_back(dg_err);
            }
        }
        ampt_file.close();
    }
    
    // Create frame - sumPt DeltaGamma: original range * 1.5
    TH1F *frame = gPad->DrawFrame(0, -0.0006, 59.9, 0.00525);
    if (showXLabels) {
        frame->GetXaxis()->SetTitle("Centrality (%)");
        frame->GetXaxis()->SetTitleOffset(1.2);
        frame->GetXaxis()->SetLabelSize(0.05);
        frame->GetXaxis()->SetTitleSize(0.05);
    } else {
        frame->GetXaxis()->SetLabelSize(0);
        frame->GetXaxis()->SetTitle("");
    }
    if (showYLabels) {
        frame->GetYaxis()->SetTitle("#Delta#gamma (OS-SS)");
        frame->GetYaxis()->SetTitleOffset(1.2);
        frame->GetYaxis()->SetLabelSize(0.05);
        frame->GetYaxis()->SetTitleSize(0.05);
    } else {
        frame->GetYaxis()->SetLabelSize(0);
        frame->GetYaxis()->SetTitle("");
    }
    frame->GetYaxis()->SetMaxDigits(2);
    
    // Create graphs for BW - red, blue, green
    TGraphErrors *gr_bw_1 = new TGraphErrors(cent_bw_1.size(), cent_bw_1.data(), dg_bw_1.data(), nullptr, dg_err_bw_1.data());
    gr_bw_1->SetFillColor(kRed+1);
    gr_bw_1->SetLineColor(kRed+1);
    gr_bw_1->SetLineWidth(2);
    
    TGraphErrors *gr_bw_3 = new TGraphErrors(cent_bw_3.size(), cent_bw_3.data(), dg_bw_3.data(), nullptr, dg_err_bw_3.data());
    gr_bw_3->SetFillColor(kBlue+1);
    gr_bw_3->SetLineColor(kBlue+1);
    gr_bw_3->SetLineWidth(2);
    
    TGraphErrors *gr_bw_5 = new TGraphErrors(cent_bw_5.size(), cent_bw_5.data(), dg_bw_5.data(), nullptr, dg_err_bw_5.data());
    gr_bw_5->SetFillColor(kGreen+2);
    gr_bw_5->SetLineColor(kGreen+2);
    gr_bw_5->SetLineWidth(2);
    
    // Draw BW graphs - largest first (5-8 GeV first)
    gr_bw_5->Draw("E3");
    gr_bw_3->Draw("E3 same");
    gr_bw_1->Draw("E3 same");
    
    // Create and draw AMPT graphs if enabled
    TGraphErrors *gr_ampt_1 = nullptr;
    TGraphErrors *gr_ampt_3 = nullptr;
    TGraphErrors *gr_ampt_5 = nullptr;
    
    if (drawAMPT) {
        gr_ampt_1 = new TGraphErrors(cent_ampt_1.size(), cent_ampt_1.data(), dg_ampt_1.data(), nullptr, dg_err_ampt_1.data());
        gr_ampt_1->SetFillColorAlpha(kOrange+1, 0.2);
        gr_ampt_1->SetLineColor(kOrange+1);
        gr_ampt_1->SetLineWidth(2);
        gr_ampt_1->SetLineStyle(2);
        
        gr_ampt_3 = new TGraphErrors(cent_ampt_3.size(), cent_ampt_3.data(), dg_ampt_3.data(), nullptr, dg_err_ampt_3.data());
        gr_ampt_3->SetFillColorAlpha(kMagenta+1, 0.2);
        gr_ampt_3->SetLineColor(kMagenta+1);
        gr_ampt_3->SetLineWidth(2);
        gr_ampt_3->SetLineStyle(2);
        
        gr_ampt_5 = new TGraphErrors(cent_ampt_5.size(), cent_ampt_5.data(), dg_ampt_5.data(), nullptr, dg_err_ampt_5.data());
        gr_ampt_5->SetFillColorAlpha(kCyan+1, 0.2);
        gr_ampt_5->SetLineColor(kCyan+1);
        gr_ampt_5->SetLineWidth(2);
        gr_ampt_5->SetLineStyle(2);
        
        gr_ampt_5->Draw("E3 same");
        gr_ampt_3->Draw("E3 same");
        gr_ampt_1->Draw("E3 same");
    }
    
    // No legend for other panels
}

void plotPanel_EtaGapDeltaDelta(bool drawAMPT = true, bool showXLabels = true, bool showYLabels = true) {
    
    // Read BW data
    ifstream bw_file("diff_etaGap_blastwave.csv");
    string header;
    getline(bw_file, header);
    
    vector<double> cent_bw_02, dd_bw_02, dd_err_bw_02;
    vector<double> cent_bw_04, dd_bw_04, dd_err_bw_04;
    vector<double> cent_bw_06, dd_bw_06, dd_err_bw_06;
    
    string line;
    while (getline(bw_file, line)) {
        if (line.empty()) continue;
        
        stringstream ss(line);
        string tag, bin_label;
        double cent, dd, dd_err, dg, dg_err;
        char comma;
        
        getline(ss, tag, ',');
        getline(ss, bin_label, ',');
        ss >> cent >> comma >> dd >> comma >> dd_err >> comma >> dg >> comma >> dg_err;
        
        if (dd == 0.0) continue;
        
        if (bin_label.find("0.2<#Delta#eta<0.4") != string::npos) {
            cent_bw_02.push_back(cent);
            dd_bw_02.push_back(dd);
            dd_err_bw_02.push_back(dd_err);
        } else if (bin_label.find("0.4<#Delta#eta<0.6") != string::npos) {
            cent_bw_04.push_back(cent);
            dd_bw_04.push_back(dd);
            dd_err_bw_04.push_back(dd_err);
        } else if (bin_label.find("0.6<#Delta#eta<0.8") != string::npos) {
            cent_bw_06.push_back(cent);
            dd_bw_06.push_back(dd);
            dd_err_bw_06.push_back(dd_err);
        }
    }
    bw_file.close();
    
    // Read AMPT data (optional)
    vector<double> cent_ampt_03, dd_ampt_03, dd_err_ampt_03;
    vector<double> cent_ampt_38, dd_ampt_38, dd_err_ampt_38;
    vector<double> cent_ampt_8, dd_ampt_8, dd_err_ampt_8;
    
    if (drawAMPT) {
        ifstream ampt_file("diff_etaGap_ampt.csv");
        getline(ampt_file, header);
        
        while (getline(ampt_file, line)) {
            if (line.empty()) continue;
            
            stringstream ss(line);
            string tag, bin_label;
            double cent, dd, dd_err, dg, dg_err;
            char comma;
            
            getline(ss, tag, ',');
            getline(ss, bin_label, ',');
            ss >> cent >> comma >> dd >> comma >> dd_err >> comma >> dg >> comma >> dg_err;
            
            if (dd == 0.0) continue;
            
            if (bin_label.find("#Delta#eta < 0.3") != string::npos) {
                cent_ampt_03.push_back(cent);
                dd_ampt_03.push_back(dd);
                dd_err_ampt_03.push_back(dd_err);
            } else if (bin_label.find("0.3 < #Delta#eta < 0.8") != string::npos) {
                cent_ampt_38.push_back(cent);
                dd_ampt_38.push_back(dd);
                dd_err_ampt_38.push_back(dd_err);
            } else if (bin_label.find("#Delta#eta > 0.8") != string::npos) {
                cent_ampt_8.push_back(cent);
                dd_ampt_8.push_back(dd);
                dd_err_ampt_8.push_back(dd_err);
            }
        }
        ampt_file.close();
    }
    
    // Create frame - etaGap DeltaDelta: original range * 1.5
    TH1F *frame = gPad->DrawFrame(0, -0.0015, 59.9, 0.0195);
    if (showXLabels) {
        frame->GetXaxis()->SetTitle("Centrality (%)");
        frame->GetXaxis()->SetTitleOffset(1.2);
        frame->GetXaxis()->SetLabelSize(0.05);
        frame->GetXaxis()->SetTitleSize(0.05);
    } else {
        frame->GetXaxis()->SetLabelSize(0);
        frame->GetXaxis()->SetTitle("");
    }
    if (showYLabels) {
        frame->GetYaxis()->SetTitle("#Delta#delta (OS-SS)");
        frame->GetYaxis()->SetTitleOffset(1.2);
        frame->GetYaxis()->SetLabelSize(0.05);
        frame->GetYaxis()->SetTitleSize(0.05);
    } else {
        frame->GetYaxis()->SetLabelSize(0);
        frame->GetYaxis()->SetTitle("");
    }
    frame->GetYaxis()->SetMaxDigits(2);
    
    // Create graphs for BW - red, blue, green
    TGraphErrors *gr_bw_02 = new TGraphErrors(cent_bw_02.size(), cent_bw_02.data(), dd_bw_02.data(), nullptr, dd_err_bw_02.data());
    gr_bw_02->SetFillColor(kRed+1);
    gr_bw_02->SetLineColor(kRed+1);
    gr_bw_02->SetLineWidth(2);
    
    TGraphErrors *gr_bw_04 = new TGraphErrors(cent_bw_04.size(), cent_bw_04.data(), dd_bw_04.data(), nullptr, dd_err_bw_04.data());
    gr_bw_04->SetFillColor(kBlue+1);
    gr_bw_04->SetLineColor(kBlue+1);
    gr_bw_04->SetLineWidth(2);
    
    TGraphErrors *gr_bw_06 = new TGraphErrors(cent_bw_06.size(), cent_bw_06.data(), dd_bw_06.data(), nullptr, dd_err_bw_06.data());
    gr_bw_06->SetFillColor(kGreen+2);
    gr_bw_06->SetLineColor(kGreen+2);
    gr_bw_06->SetLineWidth(2);
    
    // Draw BW graphs - largest eta range first (0.6-0.8 first)
    gr_bw_06->Draw("E3");
    gr_bw_04->Draw("E3 same");
    gr_bw_02->Draw("E3 same");
    
    // Create and draw AMPT graphs if enabled
    TGraphErrors *gr_ampt_03 = nullptr;
    TGraphErrors *gr_ampt_38 = nullptr;
    TGraphErrors *gr_ampt_8 = nullptr;
    
    if (drawAMPT) {
        gr_ampt_03 = new TGraphErrors(cent_ampt_03.size(), cent_ampt_03.data(), dd_ampt_03.data(), nullptr, dd_err_ampt_03.data());
        gr_ampt_03->SetFillColorAlpha(kOrange+1, 0.2);
        gr_ampt_03->SetLineColor(kOrange+1);
        gr_ampt_03->SetLineWidth(2);
        gr_ampt_03->SetLineStyle(2);
        
        gr_ampt_38 = new TGraphErrors(cent_ampt_38.size(), cent_ampt_38.data(), dd_ampt_38.data(), nullptr, dd_err_ampt_38.data());
        gr_ampt_38->SetFillColorAlpha(kMagenta+1, 0.2);
        gr_ampt_38->SetLineColor(kMagenta+1);
        gr_ampt_38->SetLineWidth(2);
        gr_ampt_38->SetLineStyle(2);
        
        gr_ampt_8 = new TGraphErrors(cent_ampt_8.size(), cent_ampt_8.data(), dd_ampt_8.data(), nullptr, dd_err_ampt_8.data());
        gr_ampt_8->SetFillColorAlpha(kCyan+1, 0.2);
        gr_ampt_8->SetLineColor(kCyan+1);
        gr_ampt_8->SetLineWidth(2);
        gr_ampt_8->SetLineStyle(2);
        
        gr_ampt_8->Draw("E3 same");
        gr_ampt_38->Draw("E3 same");
        gr_ampt_03->Draw("E3 same");
    }
    
    // Create legend for upper right panel (etaGap DeltaDelta)
    if (!showXLabels && !showYLabels) {
        TLegend *leg = new TLegend(0.15, 0.65, 0.8, 0.85);
        leg->SetBorderSize(0);
        leg->SetFillStyle(0);
        leg->SetNColumns(drawAMPT ? 2 : 1);
        
        leg->AddEntry(gr_bw_06, "BW-LBC 0.6<#Delta#eta<0.8", "f");
        if (drawAMPT) leg->AddEntry(gr_ampt_8, "AMPT-SM #Delta#eta>0.8", "f");
        
        leg->AddEntry(gr_bw_04, "BW-LBC 0.4<#Delta#eta<0.6", "f");
        if (drawAMPT) leg->AddEntry(gr_ampt_38, "AMPT-SM 0.3<#Delta#eta<0.8", "f");
        
        leg->AddEntry(gr_bw_02, "BW-LBC 0.2<#Delta#eta<0.4", "f");
        if (drawAMPT) leg->AddEntry(gr_ampt_03, "AMPT-SM #Delta#eta<0.3", "f");
        
        leg->Draw();
    }
}

void plotPanel_EtaGapDeltaGamma(bool drawAMPT = true, bool showXLabels = true, bool showYLabels = true) {
    
    // Read BW data
    ifstream bw_file("diff_etaGap_blastwave.csv");
    string header;
    getline(bw_file, header);
    
    vector<double> cent_bw_02, dg_bw_02, dg_err_bw_02;
    vector<double> cent_bw_04, dg_bw_04, dg_err_bw_04;
    vector<double> cent_bw_06, dg_bw_06, dg_err_bw_06;
    
    string line;
    while (getline(bw_file, line)) {
        if (line.empty()) continue;
        
        stringstream ss(line);
        string tag, bin_label;
        double cent, dd, dd_err, dg, dg_err;
        char comma;
        
        getline(ss, tag, ',');
        getline(ss, bin_label, ',');
        ss >> cent >> comma >> dd >> comma >> dd_err >> comma >> dg >> comma >> dg_err;
        
        if (dg == 0.0) continue;
        
        if (bin_label.find("0.2<#Delta#eta<0.4") != string::npos) {
            cent_bw_02.push_back(cent);
            dg_bw_02.push_back(dg);
            dg_err_bw_02.push_back(dg_err);
        } else if (bin_label.find("0.4<#Delta#eta<0.6") != string::npos) {
            cent_bw_04.push_back(cent);
            dg_bw_04.push_back(dg);
            dg_err_bw_04.push_back(dg_err);
        } else if (bin_label.find("0.6<#Delta#eta<0.8") != string::npos) {
            cent_bw_06.push_back(cent);
            dg_bw_06.push_back(dg);
            dg_err_bw_06.push_back(dg_err);
        }
    }
    bw_file.close();
    
    // Read AMPT data (optional)
    vector<double> cent_ampt_03, dg_ampt_03, dg_err_ampt_03;
    vector<double> cent_ampt_38, dg_ampt_38, dg_err_ampt_38;
    vector<double> cent_ampt_8, dg_ampt_8, dg_err_ampt_8;
    
    if (drawAMPT) {
        ifstream ampt_file("diff_etaGap_ampt.csv");
        getline(ampt_file, header);
        
        while (getline(ampt_file, line)) {
            if (line.empty()) continue;
            
            stringstream ss(line);
            string tag, bin_label;
            double cent, dd, dd_err, dg, dg_err;
            char comma;
            
            getline(ss, tag, ',');
            getline(ss, bin_label, ',');
            ss >> cent >> comma >> dd >> comma >> dd_err >> comma >> dg >> comma >> dg_err;
            
            if (dg == 0.0) continue;
            
            if (bin_label.find("#Delta#eta < 0.3") != string::npos) {
                cent_ampt_03.push_back(cent);
                dg_ampt_03.push_back(dg);
                dg_err_ampt_03.push_back(dg_err);
            } else if (bin_label.find("0.3 < #Delta#eta < 0.8") != string::npos) {
                cent_ampt_38.push_back(cent);
                dg_ampt_38.push_back(dg);
                dg_err_ampt_38.push_back(dg_err);
            } else if (bin_label.find("#Delta#eta > 0.8") != string::npos) {
                cent_ampt_8.push_back(cent);
                dg_ampt_8.push_back(dg);
                dg_err_ampt_8.push_back(dg_err);
            }
        }
        ampt_file.close();
    }
    
    // Create frame - etaGap DeltaGamma: original range * 1.5
    TH1F *frame = gPad->DrawFrame(0, -0.0006, 59.9, 0.00525);
    if (showXLabels) {
        frame->GetXaxis()->SetTitle("Centrality (%)");
        frame->GetXaxis()->SetTitleOffset(1.2);
        frame->GetXaxis()->SetLabelSize(0.05);
        frame->GetXaxis()->SetTitleSize(0.05);
    } else {
        frame->GetXaxis()->SetLabelSize(0);
        frame->GetXaxis()->SetTitle("");
    }
    if (showYLabels) {
        frame->GetYaxis()->SetTitle("#Delta#gamma (OS-SS)");
        frame->GetYaxis()->SetTitleOffset(1.2);
        frame->GetYaxis()->SetLabelSize(0.05);
        frame->GetYaxis()->SetTitleSize(0.05);
    } else {
        frame->GetYaxis()->SetLabelSize(0);
        frame->GetYaxis()->SetTitle("");
    }
    frame->GetYaxis()->SetMaxDigits(2);
    
    // Create graphs for BW - red, blue, green
    TGraphErrors *gr_bw_02 = new TGraphErrors(cent_bw_02.size(), cent_bw_02.data(), dg_bw_02.data(), nullptr, dg_err_bw_02.data());
    gr_bw_02->SetFillColor(kRed+1);
    gr_bw_02->SetLineColor(kRed+1);
    gr_bw_02->SetLineWidth(2);
    
    TGraphErrors *gr_bw_04 = new TGraphErrors(cent_bw_04.size(), cent_bw_04.data(), dg_bw_04.data(), nullptr, dg_err_bw_04.data());
    gr_bw_04->SetFillColor(kBlue+1);
    gr_bw_04->SetLineColor(kBlue+1);
    gr_bw_04->SetLineWidth(2);
    
    TGraphErrors *gr_bw_06 = new TGraphErrors(cent_bw_06.size(), cent_bw_06.data(), dg_bw_06.data(), nullptr, dg_err_bw_06.data());
    gr_bw_06->SetFillColor(kGreen+2);
    gr_bw_06->SetLineColor(kGreen+2);
    gr_bw_06->SetLineWidth(2);
    
    // Draw BW graphs - largest eta range first (0.6-0.8 first)
    gr_bw_06->Draw("E3");
    gr_bw_04->Draw("E3 same");
    gr_bw_02->Draw("E3 same");
    
    // Create and draw AMPT graphs if enabled
    TGraphErrors *gr_ampt_03 = nullptr;
    TGraphErrors *gr_ampt_38 = nullptr;
    TGraphErrors *gr_ampt_8 = nullptr;
    
    if (drawAMPT) {
        gr_ampt_03 = new TGraphErrors(cent_ampt_03.size(), cent_ampt_03.data(), dg_ampt_03.data(), nullptr, dg_err_ampt_03.data());
        gr_ampt_03->SetFillColorAlpha(kOrange+1, 0.2);
        gr_ampt_03->SetLineColor(kOrange+1);
        gr_ampt_03->SetLineWidth(2);
        gr_ampt_03->SetLineStyle(2);
        
        gr_ampt_38 = new TGraphErrors(cent_ampt_38.size(), cent_ampt_38.data(), dg_ampt_38.data(), nullptr, dg_err_ampt_38.data());
        gr_ampt_38->SetFillColorAlpha(kMagenta+1, 0.2);
        gr_ampt_38->SetLineColor(kMagenta+1);
        gr_ampt_38->SetLineWidth(2);
        gr_ampt_38->SetLineStyle(2);
        
        gr_ampt_8 = new TGraphErrors(cent_ampt_8.size(), cent_ampt_8.data(), dg_ampt_8.data(), nullptr, dg_err_ampt_8.data());
        gr_ampt_8->SetFillColorAlpha(kCyan+1, 0.2);
        gr_ampt_8->SetLineColor(kCyan+1);
        gr_ampt_8->SetLineWidth(2);
        gr_ampt_8->SetLineStyle(2);
        
        gr_ampt_8->Draw("E3 same");
        gr_ampt_38->Draw("E3 same");
        gr_ampt_03->Draw("E3 same");
    }
    
    // No legend for lower right panel
}

void plot_cve_data_diff(bool drawAMPT = true) {
    setROOTStyle();
    
    // Create main canvas with 2x2 panels
    TCanvas *c_main = new TCanvas("c_main", "CVE Data Differential Results", 800, 800);
    
    // Create 4 pads with shared axes
    TPad *pad1 = new TPad("pad1", "sumPt DeltaDelta", 0.0, 0.5, 0.5, 1.0);  // upper left
    TPad *pad2 = new TPad("pad2", "sumPt DeltaGamma", 0.5, 0.5, 1.0, 1.0);  // upper right
    TPad *pad3 = new TPad("pad3", "etaGap DeltaDelta", 0.0, 0.0, 0.5, 0.5); // lower left
    TPad *pad4 = new TPad("pad4", "etaGap DeltaGamma", 0.5, 0.0, 1.0, 0.5); // lower right
    
    // Set margins for shared axes - no gaps between panels
    // Upper pads - no bottom margin for x-axis sharing
    pad1->SetTopMargin(0.08);
    pad1->SetBottomMargin(0.0);
    pad1->SetLeftMargin(0.15);
    pad1->SetRightMargin(0.0);
    
    pad2->SetTopMargin(0.08);
    pad2->SetBottomMargin(0.0);
    pad2->SetLeftMargin(0.0);
    pad2->SetRightMargin(0.08);
    
    // Lower pads - with bottom margin for x-axis labels
    pad3->SetTopMargin(0.0);
    pad3->SetBottomMargin(0.15);
    pad3->SetLeftMargin(0.15);
    pad3->SetRightMargin(0.0);
    
    pad4->SetTopMargin(0.0);
    pad4->SetBottomMargin(0.15);
    pad4->SetLeftMargin(0.0);
    pad4->SetRightMargin(0.08);
    
    pad1->Draw();
    pad2->Draw();
    pad3->Draw();
    pad4->Draw();
    
    // Plot each panel
    // Upper row: DeltaDelta
    pad1->cd();
    plotPanel_SumPtDeltaDelta(drawAMPT, false, true);  // sumPt DeltaDelta - no x-labels, show y-labels
    
    pad2->cd();
    plotPanel_EtaGapDeltaDelta(drawAMPT, false, false);  // etaGap DeltaDelta - no x-labels, no y-labels
    
    // Lower row: DeltaGamma  
    pad3->cd();
    plotPanel_SumPtDeltaGamma(drawAMPT, true, true); // sumPt DeltaGamma - show x-labels, show y-labels
    
    pad4->cd();
    plotPanel_EtaGapDeltaGamma(drawAMPT, true, false); // etaGap DeltaGamma - show x-labels, no y-labels
    
    c_main->Update();
    c_main->SaveAs("cve_data_diff_results.pdf");
    c_main->SaveAs("cve_data_diff_results.png");
    
    cout << "2x2 panel plot saved as cve_data_diff_results.pdf/png" << endl;
}
