#include <TChain.h>
#include <TFile.h>
#include <TH1D.h>
#include <TLorentzVector.h>
#include <TMath.h>
#include <TProfile.h>
#include <TROOT.h>
#include <TRandom3.h>
#include <TTree.h>

#include <iostream>
#include <map>
#include <vector>
#include <fstream>
#include <algorithm>

using namespace std;

// 1. 粒子定义及属性
const std::vector<std::pair<int, std::string>> species = {{2212, "p"}, {-2212, "pbar"}, {3122, "lambda"}, {-3122, "lambdabar"}, {211, "pi+"}, {-211, "pi-"}};
const std::map<int, double> massMap = {{2212, 0.938272}, {-2212, 0.938272}, {3122, 1.115683}, {-3122, 1.115683}, {211, 0.139570}, {-211, 0.139570}};
const std::vector<double> dropProbs = {0.3, 0.5, 0.8};
const std::vector<std::pair<double, double>> sumPtBins = {{1, 2}, {2, 4}, {4, 8}};
const std::vector<std::pair<double, double>> etaGapBins = {{0, 0.3}, {0.3, 0.8}, {0.8, 5}};
inline int getSpeciesIndex(int pdg) {
  for (size_t i = 0; i < species.size(); ++i)
    if (species[i].first == pdg) return i;
  return -1;
}
inline int GetCentrality(double b) {
  for (int i = 1; i <= 10; ++i) {
    double rMax = sqrt(10.0 * i / 100.0) * 2.0 * pow(197., 1.0 / 3.0) * 1.2;
    if (b <= rMax) return i - 1;
  }
  return -1;
}

int main(int argc, char** argv) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <input.root|.list> <output.root>\n";
    return 1;
  }
  TString inputFile = argv[1], outputFile = argv[2];
  TRandom3 rng(0);
  TChain chain("AMPT");
  if (inputFile.EndsWith(".list")) {
    std::ifstream fin(inputFile.Data());
    std::string f;
    while (std::getline(fin, f))
      if (!f.empty()) chain.Add(f.c_str());
  } else {
    chain.Add(inputFile.Data());
    chain.SetMakeClass(1);
  }

  const int MAXTRKS = 99999;
  struct Event {
   Int_t   Event_nevent;
   Int_t   Event_nrun;
   Int_t   Event_multi;
   Float_t Event_impactpar;
   Int_t   Event_NpartP;
   Int_t   Event_NpartT;
   Int_t   Event_NELP;
   Int_t   Event_NINP;
   Int_t   Event_NELT;
   Int_t   Event_NINT;
  } eventbuf;

  Int_t   IDArr[MAXTRKS];
  Float_t PxArr[MAXTRKS], PyArr[MAXTRKS], PzArr[MAXTRKS];
  // Bind individual Event leaves
  chain.SetBranchAddress("Event",     &eventbuf);
  chain.SetBranchAddress("ID",        IDArr);
  chain.SetBranchAddress("Px",        PxArr);
  chain.SetBranchAddress("Py",        PyArr);
  chain.SetBranchAddress("Pz",        PzArr);

  const size_t N = species.size();
  // 2. 创建所有 profiles
  std::vector<TProfile*> profDeltaG(N * N), profGammaG(N * N);
  for (size_t i = 0; i < N; ++i)
    for (size_t j = i; j < N; ++j) {
      auto nm = species[i].second + "_" + species[j].second;
      profDeltaG[i * N + j] = new TProfile(Form("Delta_%s", nm.c_str()), "", 10, 0, 10);
      profGammaG[i * N + j] = new TProfile(Form("Gamma_%s", nm.c_str()), "", 10, 0, 10);
    }
  std::vector<std::vector<TProfile*>> profDeltaSP(sumPtBins.size(), std::vector<TProfile*>(N * N));
  std::vector<std::vector<TProfile*>> profGammaSP(sumPtBins.size(), std::vector<TProfile*>(N * N));
  std::vector<std::vector<TProfile*>> profDeltaEG(etaGapBins.size(), std::vector<TProfile*>(N * N));
  std::vector<std::vector<TProfile*>> profGammaEG(etaGapBins.size(), std::vector<TProfile*>(N * N));
  for (size_t b = 0; b < sumPtBins.size(); ++b)
    for (size_t i = 0; i < N; ++i)
      for (size_t j = i; j < N; ++j) {
        auto nm = species[i].second + "_" + species[j].second;
        profDeltaSP[b][i * N + j] = new TProfile(Form("Delta_%s_sumPt%zu", nm.c_str(), b), "", 10, 0, 10);
        profGammaSP[b][i * N + j] = new TProfile(Form("Gamma_%s_sumPt%zu", nm.c_str(), b), "", 10, 0, 10);
      }
  for (size_t b = 0; b < etaGapBins.size(); ++b)
    for (size_t i = 0; i < N; ++i)
      for (size_t j = i; j < N; ++j) {
        auto nm = species[i].second + "_" + species[j].second;
        profDeltaEG[b][i * N + j] = new TProfile(Form("Delta_%s_etaGap%zu", nm.c_str(), b), "", 10, 0, 10);
        profGammaEG[b][i * N + j] = new TProfile(Form("Gamma_%s_etaGap%zu", nm.c_str(), b), "", 10, 0, 10);
      }
  std::vector<std::vector<TProfile*>> profDeltaD(dropProbs.size(), std::vector<TProfile*>(N * N));
  std::vector<std::vector<TProfile*>> profGammaD(dropProbs.size(), std::vector<TProfile*>(N * N));
  for (size_t dp = 0; dp < dropProbs.size(); ++dp) {
    int pct = int(dropProbs[dp] * 100);
    for (size_t i = 0; i < N; ++i)
      for (size_t j = i; j < N; ++j) {
        auto nm = species[i].second + "_" + species[j].second;
        profDeltaD[dp][i * N + j] = new TProfile(Form("Delta_%s_drop%d", nm.c_str(), pct), "", 10, 0, 10);
        profGammaD[dp][i * N + j] = new TProfile(Form("Gamma_%s_drop%d", nm.c_str(), pct), "", 10, 0, 10);
      }
  }
  // pT & v2
  std::vector<TH1D*> hPt(N);
  std::vector<TProfile*> profV2(N);
  for (size_t i = 0; i < N; ++i) {
    auto nm = species[i].second;
    hPt[i] = new TH1D(Form("hPt_%s", nm.c_str()), "", 50, 0, 5);
    profV2[i] = new TProfile(Form("v2_%s", nm.c_str()), "", 50, 0, 5);
  }
  // Lambda衰变: 正/反 与 正/反交叉
  auto makeDecayProfiles = [&](const std::string& tag) {
    return std::tuple<TProfile*, TProfile*, TProfile*, TProfile*,  // to same
                      TProfile*, TProfile*, TProfile*, TProfile*   // to opposite
                      >(
        new TProfile(Form("Delta_pFrom%s_vs_%s", tag.c_str(), tag.c_str()), "", 10, 0, 10), new TProfile(Form("Gamma_pFrom%s_vs_%s", tag.c_str(), tag.c_str()), "", 10, 0, 10),
        new TProfile(Form("Delta_piFrom%s_vs_%s", tag.c_str(), tag.c_str()), "", 10, 0, 10), new TProfile(Form("Gamma_piFrom%s_vs_%s", tag.c_str(), tag.c_str()), "", 10, 0, 10),
        new TProfile(Form("Delta_pFrom%s_vs_%s", tag.c_str(), tag == "Lambda" ? "LambdaBar" : "Lambda"), "", 10, 0, 10),
        new TProfile(Form("Gamma_pFrom%s_vs_%s", tag.c_str(), tag == "Lambda" ? "LambdaBar" : "Lambda"), "", 10, 0, 10),
        new TProfile(Form("Delta_piFrom%s_vs_%s", tag.c_str(), tag == "Lambda" ? "LambdaBar" : "Lambda"), "", 10, 0, 10),
        new TProfile(Form("Gamma_piFrom%s_vs_%s", tag.c_str(), tag == "Lambda" ? "LambdaBar" : "Lambda"), "", 10, 0, 10));
  };
  auto [d_pLam, g_pLam, d_piLam, g_piLam, d_pLam_x, g_pLam_x, d_piLam_x, g_piLam_x] = makeDecayProfiles("Lambda");
  auto [d_pBarLam, g_pBarLam, d_piBarLam, g_piBarLam, d_pBarLam_x, g_pBarLam_x, d_piBarLam_x, g_piBarLam_x] = makeDecayProfiles("LambdaBar");

  // 3. 创建 TTree 保存 event-by-event 数据用于 Bootstrap
  TTree* evtTree = new TTree("EventData", "Event-by-event correlation data for bootstrap");
  Int_t evt_centrality;
  Double_t evt_delta_lambda_lambda;      // Lambda-Lambda Delta correlation
  Double_t evt_delta_pFromLambda;        // p from Lambda - Lambda Delta correlation
  Double_t evt_delta_piFromLambda;       // pi from Lambda - Lambda Delta correlation
  Int_t evt_npairs_lambda_lambda;        // number of pairs for normalization
  Int_t evt_npairs_pFromLambda;
  Int_t evt_npairs_piFromLambda;

  evtTree->Branch("centrality", &evt_centrality, "centrality/I");
  evtTree->Branch("delta_lambda_lambda", &evt_delta_lambda_lambda, "delta_lambda_lambda/D");
  evtTree->Branch("delta_pFromLambda", &evt_delta_pFromLambda, "delta_pFromLambda/D");
  evtTree->Branch("delta_piFromLambda", &evt_delta_piFromLambda, "delta_piFromLambda/D");
  evtTree->Branch("npairs_lambda_lambda", &evt_npairs_lambda_lambda, "npairs_lambda_lambda/I");
  evtTree->Branch("npairs_pFromLambda", &evt_npairs_pFromLambda, "npairs_pFromLambda/I");
  evtTree->Branch("npairs_piFromLambda", &evt_npairs_piFromLambda, "npairs_piFromLambda/I");

  // 4. 循环事件
  Long64_t nEv = chain.GetEntries();
  for (Long64_t ev = 0; ev < nEv; ++ev) {
    chain.GetEntry(ev);
    if (eventbuf.Event_multi > MAXTRKS) continue;
    int cent = GetCentrality(eventbuf.Event_impactpar);
    bool doPt = (cent == 3);
    struct P {
      int idx, pdg;
      double phi, eta, pt;
      TLorentzVector p4;
    };
    std::vector<P> list;
    list.reserve(eventbuf.Event_multi);
    for (int k = 0; k < eventbuf.Event_multi; ++k) {
      int pdg = IDArr[k];
      int idx = getSpeciesIndex(pdg);
      if (idx < 0) continue;
      double pxv = PxArr[k], pyv = PyArr[k], pzv = PzArr[k];
      double ptv = TMath::Sqrt(pxv * pxv + pyv * pyv);
      if (ptv < 0.2 || ptv > 5.) continue;
      double eta = 0.5 * TMath::Log((TMath::Sqrt(ptv * ptv + pzv * pzv) + pzv) / (TMath::Sqrt(ptv * ptv + pzv * pzv) - pzv));
      if (fabs(eta) > 0.8) continue;
      double phi = TMath::ATan2(pyv, pxv);
      if (abs(pdg) == 3122) {
        double m = massMap.at(pdg);
        double E = TMath::Sqrt(ptv * ptv + pzv * pzv + m * m);
        double y = 0.5 * TMath::Log((E + pzv) / (E - pzv));
        if (y < -0.5 || y > 0.5) continue;
      }
      TLorentzVector p4(pxv, pyv, pzv, TMath::Sqrt(ptv * ptv + pzv * pzv + massMap.at(pdg) * massMap.at(pdg)));
      if (doPt) {
        hPt[idx]->Fill(ptv);
        profV2[idx]->Fill(ptv, TMath::Cos(2 * phi));
      }
      list.push_back({idx, pdg, phi, eta, ptv, p4});
    }
    // a) 全局/分类/丢弃关联
    for (size_t i = 0; i < list.size(); ++i)
      for (size_t j = i + 1; j < list.size(); ++j) {
        auto &a = list[i], &b = list[j];
        size_t i1 = min(a.idx, b.idx), i2 = max(a.idx, b.idx), key = i1 * N + i2;
        double d = TMath::Cos(a.phi - b.phi), g = TMath::Cos(a.phi + b.phi);
        profDeltaG[key]->Fill(cent, d);
        profGammaG[key]->Fill(cent, g);
        double sp = a.pt + b.pt;
        for (size_t sb = 0; sb < sumPtBins.size(); ++sb)
          if (sp >= sumPtBins[sb].first && sp < sumPtBins[sb].second) profDeltaSP[sb][key]->Fill(cent, d), profGammaSP[sb][key]->Fill(cent, g);
        double de = fabs(a.eta - b.eta);
        for (size_t eb = 0; eb < etaGapBins.size(); ++eb)
          if (de >= etaGapBins[eb].first && de < etaGapBins[eb].second) profDeltaEG[eb][key]->Fill(cent, d), profGammaEG[eb][key]->Fill(cent, g);
      }
    // (c) 粒子丢弃后再计算关联
    for (size_t dp = 0; dp < dropProbs.size(); ++dp) {
        std::vector<P> filt;
        filt.reserve(list.size());
        // 先随机丢弃粒子
        for (auto &p : list) {
            if (rng.Rndm() >= dropProbs[dp]) filt.push_back(p);
        }
        // 对剩余粒子两两配对
        for (size_t i2 = 0; i2 < filt.size(); ++i2) {
            for (size_t j2 = i2 + 1; j2 < filt.size(); ++j2) {
                auto &a2 = filt[i2], &b2 = filt[j2];
                size_t i1 = std::min(a2.idx, b2.idx), i3 = std::max(a2.idx, b2.idx);
                size_t key2 = i1 * N + i3;
                double d2 = TMath::Cos(a2.phi - b2.phi);
                double g2 = TMath::Cos(a2.phi + b2.phi);
                profDeltaD[dp][key2]->Fill(cent, d2);
                profGammaD[dp][key2]->Fill(cent, g2);
            }
        }
    }
    // b) Lambda decay关联
    std::vector<size_t> lamIdx, lamBarIdx;
    for (size_t i = 0; i < list.size(); ++i) {
      if (list[i].pdg == 3122) lamIdx.push_back(i);
      if (list[i].pdg == -3122) lamBarIdx.push_back(i);
    }
    auto processDecay = [&](size_t ii, const std::vector<size_t>& src, const std::vector<size_t>& tgt, double m1, double m2, TProfile* d_same, TProfile* g_same, TProfile* d_opp,
                            TProfile* g_opp) {
      auto& lam = list[ii];
      double m0 = massMap.at(lam.pdg);
      double p_mag = sqrt((m0 * m0 - (m1 + m2) * (m1 + m2)) * (m0 * m0 - (m1 - m2) * (m1 - m2))) / (2 * m0);
      double cth = 2 * rng.Rndm() - 1, sth = sqrt(1 - cth * cth), phir = 2 * M_PI * rng.Rndm();
      TVector3 p_rf(p_mag * sth * cos(phir), p_mag * sth * sin(phir), p_mag * cth);
      TLorentzVector d1(p_rf, sqrt(p_rf.Mag2() + m1 * m1));
      TLorentzVector d2(-p_rf, sqrt(p_rf.Mag2() + m2 * m2));
      TVector3 beta = lam.p4.BoostVector();
      d1.Boost(beta);
      d2.Boost(beta);
      for (size_t jj : src)
        if (jj != ii) {
          double phi_o = list[jj].phi;
          double dval = cos(d1.Phi() - phi_o), gval = cos(d1.Phi() + phi_o);
          d_same->Fill(cent, dval);
          g_same->Fill(cent, gval);
          dval = cos(d2.Phi() - phi_o);
          gval = cos(d2.Phi() + phi_o);
          d_same->Fill(cent, dval);
          g_same->Fill(cent, gval);
        }
      for (size_t jj : tgt) {
        double phi_o = list[jj].phi;
        double dval = cos(d1.Phi() - phi_o), gval = cos(d1.Phi() + phi_o);
        d_opp->Fill(cent, dval);
        g_opp->Fill(cent, gval);
        dval = cos(d2.Phi() - phi_o);
        gval = cos(d2.Phi() + phi_o);
        d_opp->Fill(cent, dval);
        g_opp->Fill(cent, gval);
      }
    };
    for (size_t ii : lamIdx) {
      processDecay(ii, lamIdx, lamBarIdx, massMap.at(2212), massMap.at(211), d_pLam, g_pLam, d_pLam_x, g_pLam_x);
      processDecay(ii, lamIdx, lamBarIdx, massMap.at(211), massMap.at(2212), d_piLam, g_piLam, d_piLam_x, g_piLam_x);
    }
    for (size_t ii : lamBarIdx) {
      processDecay(ii, lamBarIdx, lamIdx, massMap.at(-2212), massMap.at(-211), d_pBarLam, g_pBarLam, d_pBarLam_x, g_pBarLam_x);
      processDecay(ii, lamBarIdx, lamIdx, massMap.at(-211), massMap.at(-2212), d_piBarLam, g_piBarLam, d_piBarLam_x, g_piBarLam_x);
    }

    // c) 计算本事件的平均 correlation 并保存到 TTree
    evt_centrality = cent;

    // Lambda-Lambda pairs
    double sum_delta_lam_lam = 0;
    int npairs_lam_lam = 0;
    for (size_t i = 0; i < lamIdx.size(); ++i) {
      for (size_t j = i + 1; j < lamIdx.size(); ++j) {
        double phi1 = list[lamIdx[i]].phi;
        double phi2 = list[lamIdx[j]].phi;
        sum_delta_lam_lam += TMath::Cos(phi1 - phi2);
        npairs_lam_lam++;
      }
    }
    evt_delta_lambda_lambda = (npairs_lam_lam > 0) ? sum_delta_lam_lam / npairs_lam_lam : 0;
    evt_npairs_lambda_lambda = npairs_lam_lam;

    // p from Lambda - Lambda pairs (模拟衰变)
    double sum_delta_pFromLam = 0;
    int npairs_pFromLam = 0;
    for (size_t ii : lamIdx) {
      auto& lam = list[ii];
      double m0 = massMap.at(lam.pdg);
      double m1 = massMap.at(2212), m2 = massMap.at(211);
      double p_mag = sqrt((m0*m0 - (m1+m2)*(m1+m2)) * (m0*m0 - (m1-m2)*(m1-m2))) / (2*m0);
      double cth = 2*rng.Rndm() - 1, sth = sqrt(1-cth*cth), phir = 2*M_PI*rng.Rndm();
      TVector3 p_rf(p_mag*sth*cos(phir), p_mag*sth*sin(phir), p_mag*cth);
      TLorentzVector d1(p_rf, sqrt(p_rf.Mag2() + m1*m1));
      TVector3 beta = lam.p4.BoostVector();
      d1.Boost(beta);

      for (size_t jj : lamIdx) {
        if (jj != ii) {
          double phi_o = list[jj].phi;
          sum_delta_pFromLam += cos(d1.Phi() - phi_o);
          npairs_pFromLam++;
        }
      }
    }
    evt_delta_pFromLambda = (npairs_pFromLam > 0) ? sum_delta_pFromLam / npairs_pFromLam : 0;
    evt_npairs_pFromLambda = npairs_pFromLam;

    // pi from Lambda - Lambda pairs (模拟衰变)
    double sum_delta_piFromLam = 0;
    int npairs_piFromLam = 0;
    for (size_t ii : lamIdx) {
      auto& lam = list[ii];
      double m0 = massMap.at(lam.pdg);
      double m1 = massMap.at(211), m2 = massMap.at(2212);
      double p_mag = sqrt((m0*m0 - (m1+m2)*(m1+m2)) * (m0*m0 - (m1-m2)*(m1-m2))) / (2*m0);
      double cth = 2*rng.Rndm() - 1, sth = sqrt(1-cth*cth), phir = 2*M_PI*rng.Rndm();
      TVector3 p_rf(p_mag*sth*cos(phir), p_mag*sth*sin(phir), p_mag*cth);
      TLorentzVector d1(p_rf, sqrt(p_rf.Mag2() + m1*m1));
      TVector3 beta = lam.p4.BoostVector();
      d1.Boost(beta);

      for (size_t jj : lamIdx) {
        if (jj != ii) {
          double phi_o = list[jj].phi;
          sum_delta_piFromLam += cos(d1.Phi() - phi_o);
          npairs_piFromLam++;
        }
      }
    }
    evt_delta_piFromLambda = (npairs_piFromLam > 0) ? sum_delta_piFromLam / npairs_piFromLam : 0;
    evt_npairs_piFromLambda = npairs_piFromLam;

    // 填充 TTree
    evtTree->Fill();
  }
  // 5. 写出所有 histograms 和 profiles
  TFile fout(outputFile, "RECREATE");

  // 全局 Delta/Gamma profiles
  for (auto p : profDeltaG)    if (p) p->Write();
  for (auto p : profGammaG)    if (p) p->Write();

  // sumPt 相关 profiles
  for (auto &v : profDeltaSP)  for (auto p : v) if (p) p->Write();
  for (auto &v : profGammaSP)  for (auto p : v) if (p) p->Write();

  // etaGap 相关 profiles
  for (auto &v : profDeltaEG)  for (auto p : v) if (p) p->Write();
  for (auto &v : profGammaEG)  for (auto p : v) if (p) p->Write();

  // 丢弃概率 profiles
  for (auto &v : profDeltaD)   for (auto p : v) if (p) p->Write();
  for (auto &v : profGammaD)   for (auto p : v) if (p) p->Write();

  // pT 分布和 v2 profiles
  for (auto h : hPt)           if (h) h->Write();
  for (auto p : profV2)        if (p) p->Write();

  // Lambda 衰变相关 Delta/Gamma profiles
  // p from Lambda vs Lambda
  if (d_pLam)       d_pLam->Write();
  if (g_pLam)       g_pLam->Write();
  // pi from Lambda vs Lambda
  if (d_piLam)      d_piLam->Write();
  if (g_piLam)      g_piLam->Write();
  // p from Lambda vs LambdaBar
  if (d_pLam_x)     d_pLam_x->Write();
  if (g_pLam_x)     g_pLam_x->Write();
  // pi from Lambda vs LambdaBar
  if (d_piLam_x)    d_piLam_x->Write();
  if (g_piLam_x)    g_piLam_x->Write();

  // pbar from LambdaBar vs LambdaBar
  if (d_pBarLam)    d_pBarLam->Write();
  if (g_pBarLam)    g_pBarLam->Write();
  // pi from LambdaBar vs LambdaBar
  if (d_piBarLam)   d_piBarLam->Write();
  if (g_piBarLam)   g_piBarLam->Write();
  // pbar from LambdaBar vs Lambda
  if (d_pBarLam_x)  d_pBarLam_x->Write();
  if (g_pBarLam_x)  g_pBarLam_x->Write();
  // pi from LambdaBar vs Lambda
  if (d_piBarLam_x) d_piBarLam_x->Write();
  if (g_piBarLam_x) g_piBarLam_x->Write();

  // Event-by-event data for Bootstrap
  if (evtTree) evtTree->Write();

  fout.Close();
  return 0;
}
