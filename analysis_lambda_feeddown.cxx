#include <TChain.h>
#include <TFile.h>
#include <TLorentzVector.h>
#include <TMath.h>
#include <TProfile.h>
#include <TROOT.h>
#include <TRandom3.h>
#include <TTree.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <map>

using namespace std;

// Lambda质量和衰变产物质量
const double MASS_LAMBDA = 1.115683;  // GeV/c^2
const double MASS_PROTON = 0.938272;  // GeV/c^2
const double MASS_PION   = 0.139570;  // GeV/c^2

// 中心度计算 (基于碰撞参数b)
inline int GetCentrality(double b) {
  for (int i = 1; i <= 10; ++i) {
    double rMax = sqrt(10.0 * i / 100.0) * 2.0 * pow(197., 1.0 / 3.0) * 1.2;
    if (b <= rMax) return i - 1;
  }
  return -1;
}

// Lambda粒子结构
struct Lambda {
  double phi;      // 方位角
  double eta;      // 赝快度
  double pt;       // 横动量
  TLorentzVector p4;  // 四动量
};

int main(int argc, char** argv) {
  if (argc != 3) {
    cerr << "Usage: " << argv[0] << " <input.root|.list> <output.root>\n";
    cerr << "\nThis program analyzes Lambda-Lambda correlations and Lambda feeddown effects.\n";
    cerr << "It calculates:\n";
    cerr << "  1. Lambda-Lambda correlation (denominator)\n";
    cerr << "  2. p from Lambda - Lambda correlation (numerator)\n";
    cerr << "  3. π from Lambda - Lambda correlation (numerator)\n";
    cerr << "Output includes TProfiles and event-by-event data for Bootstrap analysis.\n";
    return 1;
  }

  TString inputFile = argv[1], outputFile = argv[2];
  TRandom3 rng(0);  // 随机数生成器用于衰变模拟

  // ========== 1. 读取输入数据 ==========
  TChain chain("AMPT");
  if (inputFile.EndsWith(".list")) {
    ifstream fin(inputFile.Data());
    string f;
    while (getline(fin, f))
      if (!f.empty()) chain.Add(f.c_str());
  } else {
    chain.Add(inputFile.Data());
  }

  const int MAXTRKS = 99999;

  // AMPT事件结构
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

  chain.SetBranchAddress("Event", &eventbuf);
  chain.SetBranchAddress("ID", IDArr);
  chain.SetBranchAddress("Px", PxArr);
  chain.SetBranchAddress("Py", PyArr);
  chain.SetBranchAddress("Pz", PzArr);

  // ========== 2. 创建输出TProfile ==========
  // Lambda-Lambda关联 (分母)
  TProfile* prof_Delta_Lambda_Lambda = new TProfile("Delta_lambda_lambda",
                                                     "Delta correlation Lambda-Lambda;Centrality;#delta",
                                                     10, 0, 10);
  TProfile* prof_Gamma_Lambda_Lambda = new TProfile("Gamma_lambda_lambda",
                                                     "Gamma correlation Lambda-Lambda;Centrality;#gamma",
                                                     10, 0, 10);

  // p from Lambda - Lambda关联 (分子)
  TProfile* prof_Delta_pFromLambda = new TProfile("Delta_pFromLambda_vs_Lambda",
                                                   "Delta correlation p(from Lambda)-Lambda;Centrality;#delta",
                                                   10, 0, 10);
  TProfile* prof_Gamma_pFromLambda = new TProfile("Gamma_pFromLambda_vs_Lambda",
                                                   "Gamma correlation p(from Lambda)-Lambda;Centrality;#gamma",
                                                   10, 0, 10);

  // π from Lambda - Lambda关联 (分子)
  TProfile* prof_Delta_piFromLambda = new TProfile("Delta_piFromLambda_vs_Lambda",
                                                    "Delta correlation pi(from Lambda)-Lambda;Centrality;#delta",
                                                    10, 0, 10);
  TProfile* prof_Gamma_piFromLambda = new TProfile("Gamma_piFromLambda_vs_Lambda",
                                                    "Gamma correlation pi(from Lambda)-Lambda;Centrality;#gamma",
                                                    10, 0, 10);

  // ========== 3. 创建TTree保存event-by-event数据用于Bootstrap ==========
  TTree* evtTree = new TTree("EventData", "Event-by-event correlation data for bootstrap analysis");

  Int_t evt_centrality;
  Int_t evt_n_lambda;  // Number of Lambda in this event

  Double_t evt_delta_lambda_lambda;
  Double_t evt_delta_pFromLambda;
  Double_t evt_delta_piFromLambda;

  Double_t evt_gamma_lambda_lambda;
  Double_t evt_gamma_pFromLambda;
  Double_t evt_gamma_piFromLambda;

  Int_t evt_npairs_lambda_lambda;
  Int_t evt_npairs_pFromLambda;
  Int_t evt_npairs_piFromLambda;

  evtTree->Branch("centrality", &evt_centrality, "centrality/I");
  evtTree->Branch("n_lambda", &evt_n_lambda, "n_lambda/I");

  evtTree->Branch("delta_lambda_lambda", &evt_delta_lambda_lambda, "delta_lambda_lambda/D");
  evtTree->Branch("delta_pFromLambda", &evt_delta_pFromLambda, "delta_pFromLambda/D");
  evtTree->Branch("delta_piFromLambda", &evt_delta_piFromLambda, "delta_piFromLambda/D");

  evtTree->Branch("gamma_lambda_lambda", &evt_gamma_lambda_lambda, "gamma_lambda_lambda/D");
  evtTree->Branch("gamma_pFromLambda", &evt_gamma_pFromLambda, "gamma_pFromLambda/D");
  evtTree->Branch("gamma_piFromLambda", &evt_gamma_piFromLambda, "gamma_piFromLambda/D");

  evtTree->Branch("npairs_lambda_lambda", &evt_npairs_lambda_lambda, "npairs_lambda_lambda/I");
  evtTree->Branch("npairs_pFromLambda", &evt_npairs_pFromLambda, "npairs_pFromLambda/I");
  evtTree->Branch("npairs_piFromLambda", &evt_npairs_piFromLambda, "npairs_piFromLambda/I");

  // ========== 4. 事件循环 ==========
  Long64_t nEvents = chain.GetEntries();
  cout << "Processing " << nEvents << " events..." << endl;

  for (Long64_t ev = 0; ev < nEvents; ++ev) {
    if (ev % 1000 == 0) {
      cout << "Processing event " << ev << " / " << nEvents << " ("
           << 100.0*ev/nEvents << "%)" << "\r" << flush;
    }

    chain.GetEntry(ev);

    if (eventbuf.Event_multi > MAXTRKS) continue;

    int cent = GetCentrality(eventbuf.Event_impactpar);
    if (cent < 0) continue;

    // ========== 4.1 选择Lambda粒子 ==========
    vector<Lambda> lambdas;

    for (int k = 0; k < eventbuf.Event_multi; ++k) {
      int pdg = IDArr[k];

      // 只选择Lambda (PDG = 3122)
      if (pdg != 3122) continue;

      double px = PxArr[k], py = PyArr[k], pz = PzArr[k];
      double pt = TMath::Sqrt(px*px + py*py);

      // pT cut for Lambda
      if (pt < 0.5 || pt > 10.0) continue;

      // η cut
      double eta = 0.5 * TMath::Log((TMath::Sqrt(pt*pt + pz*pz) + pz) /
                                     (TMath::Sqrt(pt*pt + pz*pz) - pz));
      if (fabs(eta) > 0.8) continue;

      // Rapidity cut for Lambda
      double E = TMath::Sqrt(pt*pt + pz*pz + MASS_LAMBDA*MASS_LAMBDA);
      double y = 0.5 * TMath::Log((E + pz) / (E - pz));
      if (y < -0.5 || y > 0.5) continue;

      // 计算方位角
      double phi = TMath::ATan2(py, px);

      // 构建四动量
      TLorentzVector p4(px, py, pz, E);

      lambdas.push_back({phi, eta, pt, p4});
    }

    // 初始化事件变量
    evt_centrality = cent;
    evt_n_lambda = lambdas.size();

    // 需要至少2个Lambda才能计算关联
    if (lambdas.size() < 2) {
      evt_delta_lambda_lambda = 0;
      evt_delta_pFromLambda = 0;
      evt_delta_piFromLambda = 0;
      evt_gamma_lambda_lambda = 0;
      evt_gamma_pFromLambda = 0;
      evt_gamma_piFromLambda = 0;
      evt_npairs_lambda_lambda = 0;
      evt_npairs_pFromLambda = 0;
      evt_npairs_piFromLambda = 0;
      evtTree->Fill();
      continue;
    }

    // ========== 4.2 计算Lambda-Lambda关联 (分母) ==========
    double sum_delta_LL = 0, sum_gamma_LL = 0;
    int npairs_LL = 0;

    for (size_t i = 0; i < lambdas.size(); ++i) {
      for (size_t j = i + 1; j < lambdas.size(); ++j) {
        double dphi = lambdas[i].phi - lambdas[j].phi;
        double sphi = lambdas[i].phi + lambdas[j].phi;

        double delta = TMath::Cos(dphi);
        double gamma = TMath::Cos(sphi);

        prof_Delta_Lambda_Lambda->Fill(cent, delta);
        prof_Gamma_Lambda_Lambda->Fill(cent, gamma);

        sum_delta_LL += delta;
        sum_gamma_LL += gamma;
        npairs_LL++;
      }
    }

    evt_delta_lambda_lambda = (npairs_LL > 0) ? sum_delta_LL / npairs_LL : 0;
    evt_gamma_lambda_lambda = (npairs_LL > 0) ? sum_gamma_LL / npairs_LL : 0;
    evt_npairs_lambda_lambda = npairs_LL;

    // ========== 4.3 & 4.4 计算p和π from Lambda - Lambda关联 (分子) ==========
    // 关键：p和π必须来自同一次衰变！
    double sum_delta_pL = 0, sum_gamma_pL = 0;
    double sum_delta_piL = 0, sum_gamma_piL = 0;
    int npairs_pL = 0;
    int npairs_piL = 0;

    for (size_t i = 0; i < lambdas.size(); ++i) {
      // 模拟Lambda衰变: Λ → p + π⁻ (只衰变一次！)
      double m0 = MASS_LAMBDA;
      double p_mag = sqrt((m0*m0 - (MASS_PROTON + MASS_PION)*(MASS_PROTON + MASS_PION)) *
                          (m0*m0 - (MASS_PROTON - MASS_PION)*(MASS_PROTON - MASS_PION))) / (2*m0);

      // 各向同性衰变 (同一次衰变产生p和π)
      double cos_theta = 2*rng.Rndm() - 1;
      double sin_theta = sqrt(1 - cos_theta*cos_theta);
      double phi_rand = 2*M_PI*rng.Rndm();
      TVector3 p_rf(p_mag * sin_theta * cos(phi_rand),
                    p_mag * sin_theta * sin(phi_rand),
                    p_mag * cos_theta);

      // 构建质子和π介子的四动量（背靠背）
      TLorentzVector proton_p4(p_rf, sqrt(p_rf.Mag2() + MASS_PROTON*MASS_PROTON));
      TLorentzVector pion_p4(-p_rf, sqrt(p_rf.Mag2() + MASS_PION*MASS_PION));

      // Boost到实验室系
      TVector3 beta = lambdas[i].p4.BoostVector();
      proton_p4.Boost(beta);
      pion_p4.Boost(beta);

      double proton_pt = proton_p4.Pt();

      // 只有质子pt满足cut时才计入p from Lambda关联
      bool proton_pass_cut = (proton_pt >= 0.2);

      // 计算衰变产物与其他Lambda的关联
      for (size_t j = 0; j < lambdas.size(); ++j) {
        if (j == i) continue;  // 跳过自己

        // p from Lambda - Lambda correlation (只有质子通过cut时才计入)
        if (proton_pass_cut) {
          double dphi_p = proton_p4.Phi() - lambdas[j].phi;
          double sphi_p = proton_p4.Phi() + lambdas[j].phi;

          double delta_p = TMath::Cos(dphi_p);
          double gamma_p = TMath::Cos(sphi_p);

          prof_Delta_pFromLambda->Fill(cent, delta_p);
          prof_Gamma_pFromLambda->Fill(cent, gamma_p);

          sum_delta_pL += delta_p;
          sum_gamma_pL += gamma_p;
          npairs_pL++;
        }

        // π from Lambda - Lambda correlation (总是计入)
        double dphi_pi = pion_p4.Phi() - lambdas[j].phi;
        double sphi_pi = pion_p4.Phi() + lambdas[j].phi;

        double delta_pi = TMath::Cos(dphi_pi);
        double gamma_pi = TMath::Cos(sphi_pi);

        prof_Delta_piFromLambda->Fill(cent, delta_pi);
        prof_Gamma_piFromLambda->Fill(cent, gamma_pi);

        sum_delta_piL += delta_pi;
        sum_gamma_piL += gamma_pi;
        npairs_piL++;
      }
    }

    evt_delta_pFromLambda = (npairs_pL > 0) ? sum_delta_pL / npairs_pL : 0;
    evt_gamma_pFromLambda = (npairs_pL > 0) ? sum_gamma_pL / npairs_pL : 0;
    evt_npairs_pFromLambda = npairs_pL;

    evt_delta_piFromLambda = (npairs_piL > 0) ? sum_delta_piL / npairs_piL : 0;
    evt_gamma_piFromLambda = (npairs_piL > 0) ? sum_gamma_piL / npairs_piL : 0;
    evt_npairs_piFromLambda = npairs_piL;

    // ========== 4.5 填充TTree ==========
    evtTree->Fill();
  }

  cout << "\nProcessing completed!" << endl;

  // ========== 5. 保存结果 ==========
  TFile fout(outputFile, "RECREATE");

  // 保存TProfile
  prof_Delta_Lambda_Lambda->Write();
  prof_Gamma_Lambda_Lambda->Write();
  prof_Delta_pFromLambda->Write();
  prof_Gamma_pFromLambda->Write();
  prof_Delta_piFromLambda->Write();
  prof_Gamma_piFromLambda->Write();

  // 保存event-by-event数据
  evtTree->Write();

  fout.Close();

  cout << "Results saved to " << outputFile << endl;
  cout << "\nOutput contains:" << endl;
  cout << "  - TProfiles: Delta/Gamma correlations vs centrality" << endl;
  cout << "  - TTree 'EventData': Event-by-event data for Bootstrap error analysis" << endl;

  return 0;
}
