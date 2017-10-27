/**********************************************************************************
* Copyright (C) 2016, Copyright Holders of the ALICE Collaboration                *
* All rights reserved.                                                            *
*                                                                                 *
* Redistribution and use in source and binary forms, with or without              *
* modification, are permitted provided that the following conditions are met:     *
*   * Redistributions of source code must retain the above copyright              *
*     notice, this list of conditions and the following disclaimer.               *
*   * Redistributions in binary form must reproduce the above copyright           *
*     notice, this list of conditions and the following disclaimer in the         *
*     documentation and/or other materials provided with the distribution.        *
*   * Neither the name of the <organization> nor the                              *
*     names of its contributors may be used to endorse or promote products        *
*     derived from this software without specific prior written permission.       *
*                                                                                 *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND *
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED   *
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          *
* DISCLAIMED. IN NO EVENT SHALL ALICE COLLABORATION BE LIABLE FOR ANY             *
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES      *
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;    *
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND     *
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT      *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS   *
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                    *
* *********************************************************************************/

#include <vector>

#include <TClonesArray.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TH3.h>
#include <TList.h>
#include <THnSparse.h>
#include <TRandom3.h>
#include <TGrid.h>
#include <TFile.h>

#include <AliVCluster.h>
#include <AliVParticle.h>
#include <AliLog.h>

#include "AliTLorentzVector.h"
#include "AliEmcalJet.h"
#include "AliRhoParameter.h"
#include "AliJetContainer.h"
#include "AliParticleContainer.h"
#include "AliClusterContainer.h"
#include "AliEMCALGeometry.h"
#include "AliOADBContainer.h"
#include "AliEmcalDownscaleFactorsOCDB.h"
#include "AliEMCALTriggerPatchInfo.h"
#include "AliAnalysisTaskEmcalEmbeddingHelper.h"
#include "AliMCAnalysisUtils.h"

#include "AliAnalysisTaskEmcalJetPerformance.h"

/// \cond CLASSIMP
ClassImp(AliAnalysisTaskEmcalJetPerformance);
/// \endcond

/**
 * Default constructor. Needed by ROOT I/O
 */
AliAnalysisTaskEmcalJetPerformance::AliAnalysisTaskEmcalJetPerformance() : 
  AliAnalysisTaskEmcalJet(),
  fPlotJetHistograms(kFALSE),
  fPlotClusterHistograms(kFALSE),
  fComputeBackground(kFALSE),
  fDoTriggerSimulation(kFALSE),
  fComputeMBDownscaling(kFALSE),
  fMaxPt(200),
  fNEtaBins(40),
  fNPhiBins(200),
  fNCentHistBins(0),
  fCentHistBins(0),
  fNPtHistBins(0),
  fPtHistBins(0),
  fNM02HistBins(0),
  fM02HistBins(0),
  fMBUpscaleFactor(1.),
  fMedianEMCal(0.),
  fMedianDCal(0.),
  fkEMCEJE(kFALSE),
  fEmbeddingQA(),
  fUseAliEventCuts(kTRUE),
  fEventCuts(0),
  fEventCutList(0),
  fUseManualEventCuts(kFALSE),
  fGeneratorLevel(0),
  fHistManager()
{
  GenerateHistoBins();
}

/**
 * Standard constructor. Should be used by the user.
 *
 * @param[in] name Name of the task
 */
AliAnalysisTaskEmcalJetPerformance::AliAnalysisTaskEmcalJetPerformance(const char *name) : 
  AliAnalysisTaskEmcalJet(name, kTRUE),
  fPlotJetHistograms(kFALSE),
  fPlotClusterHistograms(kFALSE),
  fComputeBackground(kFALSE),
  fDoTriggerSimulation(kFALSE),
  fComputeMBDownscaling(kFALSE),
  fMaxPt(200),
  fNEtaBins(40),
  fNPhiBins(200),
  fNCentHistBins(0),
  fCentHistBins(0),
  fNPtHistBins(0),
  fPtHistBins(0),
  fNM02HistBins(0),
  fM02HistBins(0),
  fMBUpscaleFactor(1.),
  fMedianEMCal(0.),
  fMedianDCal(0.),
  fkEMCEJE(kFALSE),
  fEmbeddingQA(),
  fUseAliEventCuts(kTRUE),
  fEventCuts(0),
  fEventCutList(0),
  fUseManualEventCuts(kFALSE),
  fGeneratorLevel(0),
  fHistManager(name)
{
  GenerateHistoBins();
}

/**
 * Destructor
 */
AliAnalysisTaskEmcalJetPerformance::~AliAnalysisTaskEmcalJetPerformance()
{
}

/**
 * Generate histogram binning arrays
 */
void AliAnalysisTaskEmcalJetPerformance::GenerateHistoBins()
{
  fNCentHistBins = 4;
  fCentHistBins = new Double_t[fNCentHistBins+1];
  fCentHistBins[0] = 0;
  fCentHistBins[1] = 10;
  fCentHistBins[2] = 30;
  fCentHistBins[3] = 50;
  fCentHistBins[4] = 90;
  
  fNPtHistBins = 82;
  fPtHistBins = new Double_t[fNPtHistBins+1];
  GenerateFixedBinArray(6, 0, 0.3, fPtHistBins);
  GenerateFixedBinArray(7, 0.3, 1, fPtHistBins+6);
  GenerateFixedBinArray(10, 1, 3, fPtHistBins+13);
  GenerateFixedBinArray(14, 3, 10, fPtHistBins+23);
  GenerateFixedBinArray(10, 10, 20, fPtHistBins+37);
  GenerateFixedBinArray(15, 20, 50, fPtHistBins+47);
  GenerateFixedBinArray(20, 50, 150, fPtHistBins+62);
  
  fNM02HistBins = 81;
  fM02HistBins = new Double_t[fNM02HistBins+1];
  GenerateFixedBinArray(35, 0, 0.7, fM02HistBins);
  GenerateFixedBinArray(6, 0.7, 1., fM02HistBins+35);
  GenerateFixedBinArray(20, 1., 3., fM02HistBins+41);
  GenerateFixedBinArray(10, 3., 5., fM02HistBins+61);
  GenerateFixedBinArray(10, 5., 10., fM02HistBins+71);
}

/**
 * Performing run-independent initialization.
 * Here the histograms should be instantiated.
 */
void AliAnalysisTaskEmcalJetPerformance::UserCreateOutputObjects()
{
  AliAnalysisTaskEmcalJet::UserCreateOutputObjects();
  
  // Intialize AliEventCuts
  if (fUseAliEventCuts) {
    fEventCutList = new TList();
    fEventCutList ->SetOwner();
    fEventCutList ->SetName("EventCutOutput");
    
    fEventCuts.OverrideAutomaticTriggerSelection(fOffTrigger);
    if(fUseManualEventCuts==1)
    {
      fEventCuts.SetManualMode();
      // Configure manual settings here
      // ...
    }
    fEventCuts.AddQAplotsToList(fEventCutList);
    fOutput->Add(fEventCutList);
  }
  
  // Get the MC particle branch, in case it exists
  fGeneratorLevel = GetMCParticleContainer("mcparticles");
  
  if (fPlotJetHistograms) {
    AllocateJetHistograms();
  }
  if (fPlotClusterHistograms) {
    AllocateClusterHistograms();
  }
  if (fComputeBackground) {
    AllocateBackgroundHistograms();
  }
  if (fDoTriggerSimulation) {
    AllocateTriggerSimHistograms();
  }
  
  // Initialize embedding QA
  const AliAnalysisTaskEmcalEmbeddingHelper * embeddingHelper = AliAnalysisTaskEmcalEmbeddingHelper::GetInstance();
  if (embeddingHelper) {
    bool res = fEmbeddingQA.Initialize();
    if (res) {
      fEmbeddingQA.AddQAPlotsToList(fOutput);
    }
  }
  
  TIter next(fHistManager.GetListOfHistograms());
  TObject* obj = 0;
  while ((obj = next())) {
    fOutput->Add(obj);
  }
  
  PostData(1, fOutput); // Post data for ALL output slots > 0 here.
}

/*
 * This function allocates the histograms for single jets.
 * A set of histograms is allocated per each jet container.
 */
void AliAnalysisTaskEmcalJetPerformance::AllocateJetHistograms()
{
  TString histname;
  TString title;
  
  Int_t nPtBins = TMath::CeilNint(fMaxPt/2);
  
  AliJetContainer* jets = 0;
  TIter nextJetColl(&fJetCollArray);
  while ((jets = static_cast<AliJetContainer*>(nextJetColl()))) {
    
    // Jet rejection reason
    histname = TString::Format("%s/JetHistograms/hJetRejectionReason", jets->GetArrayName().Data());
    title = histname + ";Rejection reason;#it{p}_{T,jet} (GeV/#it{c});counts";
    TH2* hist = fHistManager.CreateTH2(histname.Data(), title.Data(), 32, 0, 32, 50, 0, fMaxPt);
    SetRejectionReasonLabels(hist->GetXaxis());
    
    // Rho vs. Centrality
    if (!jets->GetRhoName().IsNull()) {
      histname = TString::Format("%s/JetHistograms/hRhoVsCent", jets->GetArrayName().Data());
      title = histname + ";Centrality (%);#rho (GeV/#it{c});counts";
      fHistManager.CreateTH2(histname.Data(), title.Data(), 50, 0, 100, 100, 0, 500);
    }
    
    // (Centrality, pT, NEF, calo type)
    histname = TString::Format("%s/JetHistograms/hNEFVsPt", jets->GetArrayName().Data());
    title = histname + ";Centrality (%);#it{p}_{T}^{corr} (GeV/#it{c});NEF;type";
    Int_t nbins1[6]  = {20, nPtBins, 50, 2};
    Double_t min1[6] = {0, 0, 0, -0.5};
    Double_t max1[6] = {100, fMaxPt, 1., 1.5};
    fHistManager.CreateTHnSparse(histname.Data(), title.Data(), 4, nbins1, min1, max1);
    
    // (Centrality, pT upscaled, calo type)
    if (fComputeMBDownscaling) {
      histname = TString::Format("%s/JetHistograms/hPtUpscaledMB", jets->GetArrayName().Data());
      title = histname + ";Centrality (%);#it{p}_{T}^{corr} (GeV/#it{c});type";
      fHistManager.CreateTH3(histname.Data(), title.Data(), 20, 0, 100, nPtBins, 0, fMaxPt, 2, -0.5, 1.5, "s");
    }
    
    // pT-leading vs. pT
    histname = TString::Format("%s/JetHistograms/hPtLeadingVsPt", jets->GetArrayName().Data());
    title = histname + ";#it{p}_{T}^{corr} (GeV/#it{c});#it{p}_{T,particle}^{leading} (GeV/#it{c})";
    fHistManager.CreateTH2(histname.Data(), title.Data(), nPtBins, 0, fMaxPt, nPtBins, 0, fMaxPt);
    
    // A vs. pT
    histname = TString::Format("%s/JetHistograms/hAreaVsPt", jets->GetArrayName().Data());
    title = histname + ";#it{p}_{T}^{corr} (GeV/#it{c});#it{A}_{jet}";
    fHistManager.CreateTH2(histname.Data(), title.Data(), nPtBins, 0, fMaxPt, fMaxPt/3, 0, 0.5);
    
    // (Centrality, pT, z-leading (charged), calo type)
    histname = TString::Format("%s/JetHistograms/hZLeadingVsPt", jets->GetArrayName().Data());
    title = histname + ";Centrality (%);#it{p}_{T}^{corr} (GeV/#it{c});#it{z}_{leading};type";
    Int_t nbins2[4]  = {20, nPtBins, 50, 2};
    Double_t min2[4] = {0, 0, 0, -0.5};
    Double_t max2[4] = {100, fMaxPt, 1., 1.5};
    fHistManager.CreateTHnSparse(histname.Data(), title.Data(), 4, nbins2, min2, max2);
    
    // (Centrality, pT, z (charged), calo type)
    histname = TString::Format("%s/JetHistograms/hZVsPt", jets->GetArrayName().Data());
    title = histname + ";Centrality (%);#it{p}_{T}^{corr} (GeV/#it{c});#it{z};type";
    Int_t nbins3[4]  = {20, nPtBins, 50, 2};
    Double_t min3[4] = {0, 0, 0, -0.5};
    Double_t max3[4] = {100, fMaxPt, 1., 1.5};
    fHistManager.CreateTHnSparse(histname.Data(), title.Data(), 4, nbins3, min3, max3);
    
    // (Centrality, pT, Nconst, calo type)
    histname = TString::Format("%s/JetHistograms/hNConstVsPt", jets->GetArrayName().Data());
    title = histname + ";Centrality (%);#it{p}_{T}^{corr} (GeV/#it{c});No. of constituents;type";
    Int_t nbins4[4]  = {20, nPtBins, 50, 2};
    Double_t min4[4] = {0, 0, 0, -0.5};
    Double_t max4[4] = {100, fMaxPt, fMaxPt, 1.5};
    fHistManager.CreateTHnSparse(histname.Data(), title.Data(), 4, nbins4, min4, max4);
    
    // (Median patch energy, calo type, jet pT, centrality)
    if (fDoTriggerSimulation) {
      histname = TString::Format("%s/JetHistograms/hMedPatchJet", jets->GetArrayName().Data());
      title = histname + ";#it{E}_{patch,med};type;#it{p}_{T}^{corr} (GeV/#it{c});Centrality (%)";
      Int_t nbins5[4]  = {100, 2, nPtBins, 50};
      Double_t min5[4] = {0,-0.5, 0, 0};
      Double_t max5[4] = {50,1.5, fMaxPt, 100};
      fHistManager.CreateTHnSparse(histname.Data(), title.Data(), 4, nbins5, min5, max5);
    }
    
  }
  
  // MB downscale factor histogram
  if (fComputeMBDownscaling) {
    histname = "Trigger/hMBDownscaleFactor";
    title = histname + ";Downscale factor;counts";
    fHistManager.CreateTH1(histname.Data(), title.Data(), 200, 0, 200);
  }
  
}

/*
 * This function allocates the histograms for the calorimeter performance study.
 */
void AliAnalysisTaskEmcalJetPerformance::AllocateClusterHistograms()
{
  TString histname;
  TString htitle;
  Int_t nPtBins = TMath::CeilNint(fMaxPt/2);
  
  const Int_t nRejBins = 32;
  Double_t* rejReasonBins = new Double_t[nRejBins+1];
  GenerateFixedBinArray(nRejBins, 0, nRejBins, rejReasonBins);
  
  AliEmcalContainer* cont = 0;
  TIter nextClusColl(&fClusterCollArray);
  while ((cont = static_cast<AliEmcalContainer*>(nextClusColl()))) {
  
    // rejection reason plot, to make efficiency correction
    histname = TString::Format("%s/hClusterRejectionReasonEMCal", cont->GetArrayName().Data());
    htitle = histname + ";Rejection reason;#it{E}_{clus} (GeV/)";
    TH2* hist = fHistManager.CreateTH2(histname.Data(), htitle.Data(), nRejBins, rejReasonBins, fNPtHistBins, fPtHistBins);
    SetRejectionReasonLabels(hist->GetXaxis());
    
    histname = TString::Format("%s/hClusterRejectionReasonMC", cont->GetArrayName().Data());
    htitle = histname + ";Rejection reason;#it{E}_{clus} (GeV/)";
    TH2* histMC = fHistManager.CreateTH2(histname.Data(), htitle.Data(), nRejBins, rejReasonBins, fNPtHistBins, fPtHistBins);
    SetRejectionReasonLabels(histMC->GetXaxis());
  }
  
  const Int_t nParticleTypes = 8;
  Double_t *particleTypeBins = GenerateFixedBinArray(nParticleTypes, -0.5, 7.5);
  
  // M02 vs. Energy vs. Particle type
  histname = "JetPerformance/hM02VsParticleType";
  htitle = histname + ";M02;#it{E}_{clus} (GeV); Particle type";
  fHistManager.CreateTH3(histname.Data(), htitle.Data(), fNM02HistBins, fM02HistBins, fNPtHistBins, fPtHistBins, nParticleTypes, particleTypeBins);
  
  // M02 vs. Energy vs. Particle type vs. Jet pT, for particles inside jets
  Int_t dim = 0;
  TString title[20];
  Int_t nbins[20] = {0};
  Double_t min[30] = {0.};
  Double_t max[30] = {0.};
  Double_t *binEdges[20] = {0};
  
  title[dim] = "M02";
  nbins[dim] = fNM02HistBins;
  binEdges[dim] = fM02HistBins;
  min[dim] = fM02HistBins[0];
  max[dim] = fM02HistBins[fNM02HistBins];
  dim++;
  
  title[dim] = "#it{E}_{clus} (GeV)";
  nbins[dim] = fNPtHistBins;
  binEdges[dim] = fPtHistBins;
  min[dim] = fPtHistBins[0];
  max[dim] = fPtHistBins[fNPtHistBins];
  dim++;
  
  title[dim] = "Particle type";
  nbins[dim] = nParticleTypes;
  min[dim] = -0.5;
  max[dim] = 7.5;
  binEdges[dim] = particleTypeBins;
  dim++;
  
  title[dim] = "#it{p}_{T,jet}^{corr}";
  nbins[dim] = nPtBins;
  min[dim] = 0;
  max[dim] = fMaxPt;
  binEdges[dim] = GenerateFixedBinArray(nbins[dim], min[dim], max[dim]);
  dim++;
  
  TString thnname = "JetPerformance/hM02VsParticleTypeJets";
  THnSparse* hn = fHistManager.CreateTHnSparse(thnname.Data(), thnname.Data(), dim, nbins, min, max);
  for (Int_t i = 0; i < dim; i++) {
    hn->GetAxis(i)->SetTitle(title[i]);
    hn->SetBinEdges(i, binEdges[i]);
  }
  
  // Particle composition inside each jet -- jet pT vs. particle type vs. particle number vs. particle pT sum
  // (One entry per jet for each particle type)
  dim = 0;
  
  title[dim] = "#it{p}_{T,jet}^{corr}";
  nbins[dim] = nPtBins;
  min[dim] = 0;
  max[dim] = fMaxPt;
  binEdges[dim] = GenerateFixedBinArray(nbins[dim], min[dim], max[dim]);
  dim++;
  
  title[dim] = "Particle type";
  nbins[dim] = nParticleTypes;
  min[dim] = -0.5;
  max[dim] = 7.5;
  binEdges[dim] = particleTypeBins;
  dim++;
  
  title[dim] = "N";
  nbins[dim] = 30;
  min[dim] = -0.5;
  max[dim] = 29.5;
  binEdges[dim] = GenerateFixedBinArray(nbins[dim], min[dim], max[dim]);
  dim++;
  
  title[dim] = "#it{p}_{T,sum} (GeV)";
  nbins[dim] = fNPtHistBins;
  binEdges[dim] = fPtHistBins;
  min[dim] = fPtHistBins[0];
  max[dim] = fPtHistBins[fNPtHistBins];
  dim++;
  
  thnname = "JetPerformance/hJetComposition";
  THnSparse* thn = fHistManager.CreateTHnSparse(thnname.Data(), thnname.Data(), dim, nbins, min, max);
  for (Int_t i = 0; i < dim; i++) {
    thn->GetAxis(i)->SetTitle(title[i]);
    thn->SetBinEdges(i, binEdges[i]);
  }
  
  // Hadronic calo energy in each jet
  
  // Jet pT vs. Summed energy of hadronic clusters without a matched track
  histname = "JetPerformance/hHadCaloEnergyUnmatched";
  htitle = histname + ";#it{p}_{T,jet} (GeV);#it{p}_{T,had} (GeV)";
  fHistManager.CreateTH2(histname.Data(), htitle.Data(), fNPtHistBins, fPtHistBins, fNPtHistBins, fPtHistBins);
  
  // Jet pT vs. Summed energy of hadronic clusters with a matched track (before hadronic correction)
  histname = "JetPerformance/hHadCaloEnergyMatchedNonlincorr";
  htitle = histname + ";#it{p}_{T,jet} (GeV);#it{p}_{T,had} (GeV)";
  fHistManager.CreateTH2(histname.Data(), htitle.Data(), fNPtHistBins, fPtHistBins, fNPtHistBins, fPtHistBins);
  
  // Jet pT vs. Summed energy of hadronic clusters with a matched track (after hadronic correction)
  histname = "JetPerformance/hHadCaloEnergyMatchedHadCorr";
  htitle = histname + ";#it{p}_{T,jet} (GeV);#it{p}_{T,had} (GeV)";
  fHistManager.CreateTH2(histname.Data(), htitle.Data(), fNPtHistBins, fPtHistBins, fNPtHistBins, fPtHistBins);
  
}
      
/*
 * This function allocates background subtraction histograms, if enabled.
 * A set of histograms is allocated per each jet container.
 */
void AliAnalysisTaskEmcalJetPerformance::AllocateBackgroundHistograms()
{
  TString histname;
  TString title;
  
  AliJetContainer* jets = 0;
  TIter nextJetColl(&fJetCollArray);
  while ((jets = static_cast<AliJetContainer*>(nextJetColl()))) {
    
    histname = TString::Format("%s/BackgroundHistograms/hScaleFactorEMCal", jets->GetArrayName().Data());
    title = histname + ";Centrality;Scale factor;counts";
    fHistManager.CreateTH2(histname.Data(), title.Data(), 50, 0, 100, 100, 0, 5);
    
    histname = TString::Format("%s/BackgroundHistograms/hDeltaPtEMCal", jets->GetArrayName().Data());
    title = histname + ";Centrality (%);#delta#it{p}_{T} (GeV/#it{c});counts";
    fHistManager.CreateTH2(histname.Data(), title.Data(), 10, 0, 100, 400, -50, 150);
    
  }
}

/*
 * This function allocates the histograms for single jets, when the "simulated" trigger has been fired.
 * A set of histograms is allocated per each jet container.
 */
void AliAnalysisTaskEmcalJetPerformance::AllocateTriggerSimHistograms()
{
  TString histname;
  TString title;
  Int_t nPtBins = TMath::CeilNint(fMaxPt/2);
  
  //----------------------------------------------
  // Trigger patch histograms
  
  // patch eta vs. phi
  histname = "TriggerSimHistograms/hEtaVsPhi";
  title = histname + ";#eta_{patch} (rad);#phi_{patch} (rad)";
  fHistManager.CreateTH2(histname.Data(), title.Data(), 140, -0.7, 0.7, 500, 1., 6.);
  
  // N patches
  histname = "TriggerSimHistograms/hNPatches";
  title = histname + ";#it{N}_{patches};type";
  fHistManager.CreateTH2(histname.Data(), title.Data(), 200, 0, 200, 2, -0.5, 1.5);
  
  // patch E vs. centrality
  histname = "TriggerSimHistograms/hPatchE";
  title = histname + ";Centrality (%);#it{E}_{patch} (GeV)";
  fHistManager.CreateTH2(histname.Data(), title.Data(), 50, 0, 100, nPtBins, 0, fMaxPt);
  
  // patch median vs. Centrality
  histname = "TriggerSimHistograms/hPatchMedianE";
  title = histname + ";Centrality (%);#it{E}_{patch,med} (GeV);type";
  fHistManager.CreateTH3(histname.Data(), title.Data(), 50, 0, 100, 100, 0, 50, 2, -0.5, 1.5);
  
  //----------------------------------------------
  // Jet histograms for "triggered" events
  AliJetContainer* jets = 0;
  TIter nextJetColl(&fJetCollArray);
  while ((jets = static_cast<AliJetContainer*>(nextJetColl()))) {
    
    // Jet rejection reason
    histname = TString::Format("%s/TriggerSimHistograms/hJetRejectionReason", jets->GetArrayName().Data());
    title = histname + ";Rejection reason;#it{p}_{T,jet} (GeV/#it{c});counts";
    TH2* hist = fHistManager.CreateTH2(histname.Data(), title.Data(), 32, 0, 32, 50, 0, fMaxPt);
    SetRejectionReasonLabels(hist->GetXaxis());
    
    // Rho vs. Centrality
    if (!jets->GetRhoName().IsNull()) {
      histname = TString::Format("%s/TriggerSimHistograms/hRhoVsCent", jets->GetArrayName().Data());
      title = histname + ";Centrality (%);#rho (GeV/#it{c});counts";
      fHistManager.CreateTH2(histname.Data(), title.Data(), 50, 0, 100, 100, 0, 500);
    }
    
    // (Centrality, pT, NEF, calo type)
    histname = TString::Format("%s/TriggerSimHistograms/hNEFVsPt", jets->GetArrayName().Data());
    title = histname + ";Centrality (%);#it{p}_{T}^{corr} (GeV/#it{c});NEF;type";
    Int_t nbins1[6]  = {20, nPtBins, 50, 2};
    Double_t min1[6] = {0, 0, 0, -0.5};
    Double_t max1[6] = {100, fMaxPt, 1., 1.5};
    fHistManager.CreateTHnSparse(histname.Data(), title.Data(), 4, nbins1, min1, max1);
    
    // pT-leading vs. pT
    histname = TString::Format("%s/TriggerSimHistograms/hPtLeadingVsPt", jets->GetArrayName().Data());
    title = histname + ";#it{p}_{T}^{corr} (GeV/#it{c});#it{p}_{T,particle}^{leading} (GeV/#it{c})";
    fHistManager.CreateTH2(histname.Data(), title.Data(), nPtBins, 0, fMaxPt, nPtBins, 0, fMaxPt);
    
    // A vs. pT
    histname = TString::Format("%s/TriggerSimHistograms/hAreaVsPt", jets->GetArrayName().Data());
    title = histname + ";#it{p}_{T}^{corr} (GeV/#it{c});#it{A}_{jet}";
    fHistManager.CreateTH2(histname.Data(), title.Data(), nPtBins, 0, fMaxPt, fMaxPt/3, 0, 0.5);
    
    // (Centrality, pT, z-leading (charged), calo type)
    histname = TString::Format("%s/TriggerSimHistograms/hZLeadingVsPt", jets->GetArrayName().Data());
    title = histname + ";Centrality (%);#it{p}_{T}^{corr} (GeV/#it{c});#it{z}_{leading};type";
    Int_t nbins2[4]  = {20, nPtBins, 50, 2};
    Double_t min2[4] = {0, 0, 0, -0.5};
    Double_t max2[4] = {100, fMaxPt, 1., 1.5};
    fHistManager.CreateTHnSparse(histname.Data(), title.Data(), 4, nbins2, min2, max2);
    
    // z (charged) vs. pT
    histname = TString::Format("%s/TriggerSimHistograms/hZVsPt", jets->GetArrayName().Data());
    title = histname + ";Centrality (%);#it{p}_{T}^{corr} (GeV/#it{c});#it{z};type";
    Int_t nbins3[4]  = {20, nPtBins, 50, 2};
    Double_t min3[4] = {0, 0, 0, -0.5};
    Double_t max3[4] = {100, fMaxPt, 1., 1.5};
    fHistManager.CreateTHnSparse(histname.Data(), title.Data(), 4, nbins3, min3, max3);
    
    // (Centrality, pT, Nconst, calo type)
    histname = TString::Format("%s/TriggerSimHistograms/hNConstVsPt", jets->GetArrayName().Data());
    title = histname + ";Centrality (%);#it{p}_{T}^{corr} (GeV/#it{c});No. of constituents;type";
    Int_t nbins4[4]  = {20, nPtBins, 50, 2};
    Double_t min4[4] = {0, 0, 0, -0.5};
    Double_t max4[4] = {100, fMaxPt, fMaxPt, 1.5};
    fHistManager.CreateTHnSparse(histname.Data(), title.Data(), 4, nbins4, min4, max4);
    
  }
}

/**
 * This function is executed automatically for the first event.
 * Some extra initialization can be performed here.
 */
void AliAnalysisTaskEmcalJetPerformance::ExecOnce()
{
  // Configure base class to set fTriggerPatchInfo to array of trigger patches, each event
  // (Need to call this before base class ExecOnce)
  if (fDoTriggerSimulation) {
    this->SetCaloTriggerPatchInfoName("EmcalTriggers");
  }
  
  AliAnalysisTaskEmcalJet::ExecOnce();
  
  fNeedEmcalGeom = kTRUE;
  
  // Check if trigger patches are loaded
  if (fDoTriggerSimulation) {
    if (fTriggerPatchInfo) {
      TString objname(fTriggerPatchInfo->GetClass()->GetName());
      TClass cls(objname);
      if (!cls.InheritsFrom("AliEMCALTriggerPatchInfo")) {
        AliError(Form("%s: Objects of type %s in %s are not inherited from AliEMCALTriggerPatchInfo!",
                      GetName(), cls.GetName(), "EmcalTriggers"));
        fTriggerPatchInfo = 0;
      }
    }
    if (!fTriggerPatchInfo) {
      AliError(Form("%s: Unable to get trigger patch container with name %s. Aborting", GetName(), "EmcalTriggers"));
      return;
    }
  }
}

/**
 * This function is called automatically when the run number changes.
 */
void AliAnalysisTaskEmcalJetPerformance::RunChanged(Int_t run){
  
  // Get the downscaling factors for MB triggers (to be used to calculate trigger efficiency)
  
  if (fPlotJetHistograms && fComputeMBDownscaling) {
    
    // Get instance of the downscale factor helper class
    AliEmcalDownscaleFactorsOCDB *downscaleOCDB = AliEmcalDownscaleFactorsOCDB::Instance();
    downscaleOCDB->SetRun(InputEvent()->GetRunNumber());
    
    // There are two possible min bias triggers for LHC15o
    TString triggerNameMB1 = "CINT7-B-NOPF-CENT";
    TString triggerNameMB2 = "CV0L7-B-NOPF-CENT";
    TString triggerNameJE = "CINT7EJ1-B-NOPF-CENTNOPMD";
    
    // Get the downscale factor for whichever MB trigger exists in the given run
    std::vector<TString> runtriggers = downscaleOCDB->GetTriggerClasses();
    Double_t downscalefactor;
    for (auto i : runtriggers) {
      if (i.EqualTo(triggerNameMB1) || i.EqualTo(triggerNameMB2)) {
        downscalefactor = downscaleOCDB->GetDownscaleFactorForTriggerClass(i.Data());
        break;
      }
    }
    
    // Store the inverse of the downscale factor, used later to weight the pT spectrum
    fMBUpscaleFactor = 1/downscalefactor;
    
    TString histname = "Trigger/hMBDownscaleFactor";
    fHistManager.FillTH1(histname.Data(), fMBUpscaleFactor);
    
  }
  
}

/**
 * This function (overloading the base class) uses AliEventCuts to perform event selection.
 */
Bool_t AliAnalysisTaskEmcalJetPerformance::IsEventSelected()
{
  if (fUseAliEventCuts) {
    if (!fEventCuts.AcceptEvent(InputEvent()))
    {
      PostData(1, fOutput);
      return kFALSE;
    }
  }
  else {
    AliAnalysisTaskEmcal::IsEventSelected();
  }
  return kTRUE;
}

/**
 * Run analysis code here, if needed.
 * It will be executed before FillHistograms().
 * If this function return kFALSE, FillHistograms() will *not*
 * be executed for the current event
 * @return Always kTRUE
 */
Bool_t AliAnalysisTaskEmcalJetPerformance::Run()
{
  TString histname;
  AliJetContainer* jetCont = 0;
  TIter next(&fJetCollArray);
  while ((jetCont = static_cast<AliJetContainer*>(next()))) {
    TString jetContName = jetCont->GetName();
    
    // Compute the full jet background scale factor and delta-pt
    if (fComputeBackground) {
      ComputeBackground();
    }
    
    // Do a simple trigger simulation (if requested)
    if (fDoTriggerSimulation) {
      DoTriggerSimulation();
    }
    
  }
  
  // Only fill the embedding qa plots if:
  //  - We are using the embedding helper
  //  - The class has been initialized
  //  - Both jet collections are available
  if (fEmbeddingQA.IsInitialized()) {
    fEmbeddingQA.RecordEmbeddedEventProperties();
  }
  
  return kTRUE;
}

/**
 * Do a simple trigger simulation, mimicking the median-subtraction method using cell amplitudes.
 */
void AliAnalysisTaskEmcalJetPerformance::DoTriggerSimulation()
{
  TString histname;
  
  // Check if trigger patches are loaded
  if (fTriggerPatchInfo) {
    TString objname(fTriggerPatchInfo->GetClass()->GetName());
    TClass cls(objname);
    if (!cls.InheritsFrom("AliEMCALTriggerPatchInfo")) {
      AliError(Form("%s: Objects of type %s in %s are not inherited from AliEMCALTriggerPatchInfo!",
                    GetName(), cls.GetName(), "EmcalTriggers"));
      fTriggerPatchInfo = 0;
    }
  }
  if (!fTriggerPatchInfo) {
    AliError(Form("%s: Unable to get trigger patch container with name %s. Aborting", GetName(), "EmcalTriggers"));
    return;
  }
  
  // Compute patches in EMCal, DCal (I want offline simple trigger patch, i.e. patch calculated using FEE energy)
  std::vector<Double_t> vecEMCal;
  std::vector<Double_t> vecDCal;
  for(auto p : *fTriggerPatchInfo){
    AliEMCALTriggerPatchInfo *recpatch = static_cast<AliEMCALTriggerPatchInfo *>(p);
    if (recpatch) {
      
      if(!recpatch->IsJetHighSimple()) continue;
      
      histname = "TriggerSimHistograms/hEtaVsPhi";
      fHistManager.FillTH2(histname.Data(), recpatch->GetEtaGeo(), recpatch->GetPhiGeo());
      
      histname = "TriggerSimHistograms/hPatchE";
      fHistManager.FillTH2(histname.Data(), fCent, recpatch->GetPatchE());
      
      if (recpatch->IsEMCal()) {
        vecEMCal.push_back(recpatch->GetPatchE());
      } else {
        vecDCal.push_back(recpatch->GetPatchE());
      }
      
    }
  }
  
  // Compute the median in each calorimeter
  const Int_t nBkgPatchesEMCal = vecEMCal.size(); // 6*8;
  const Int_t nBkgPatchesDCal = vecDCal.size();   // 4*5;
  fMedianEMCal = TMath::Median(nBkgPatchesEMCal, &vecEMCal[0]); // point to array used internally by vector
  fMedianDCal = TMath::Median(nBkgPatchesDCal, &vecDCal[0]);
  
  histname = "TriggerSimHistograms/hPatchMedianE";
  fHistManager.FillTH3(histname.Data(), fCent, fMedianEMCal, kEMCal);
  fHistManager.FillTH3(histname.Data(), fCent, fMedianDCal, kDCal);
  
  histname = "TriggerSimHistograms/hNPatches";
  fHistManager.FillTH2(histname.Data(), nBkgPatchesEMCal, kEMCal);
  fHistManager.FillTH2(histname.Data(), nBkgPatchesDCal, kDCal);
  
  // Then compute background subtracted patches, by subtracting from each patch the median patch E from the opposite hemisphere
  // If a patch is above threshold, the event is "triggered"
  Bool_t fkEMCEJE = kFALSE;
  Double_t threshold = 20;
  for(auto p : *fTriggerPatchInfo){
    AliEMCALTriggerPatchInfo *recpatch = static_cast<AliEMCALTriggerPatchInfo *>(p);
    if (recpatch) {
      
      if(!recpatch->IsJetHighSimple()) continue;
      
      if (recpatch->IsEMCal()) {
        if ((recpatch->GetPatchE() - fMedianDCal) > threshold) {
          fkEMCEJE = kTRUE;
          break;
        }
      } else {
        if ((recpatch->GetPatchE() - fMedianEMCal) > threshold) {
          fkEMCEJE = kTRUE;
          break;
        }
      }
    }
  }
  
  if (fkEMCEJE) {
    FillTriggerSimHistograms();
  }
  
}

/**
 * The body of this function should contain instructions to fill the output histograms.
 * This function is called inside the event loop, after the function Run() has been
 * executed successfully (i.e. it returned kTRUE).
 * @return Always kTRUE
 */
Bool_t AliAnalysisTaskEmcalJetPerformance::FillHistograms()
{
  
  if (fPlotJetHistograms) {
    FillJetHistograms();
  }
  if (fPlotClusterHistograms) {
    FillClusterHistograms();
  }
  
  return kTRUE;
}

/**
 * This function performs a loop over the reconstructed jets
 * in the current event and fills the relevant histograms.
 */
void AliAnalysisTaskEmcalJetPerformance::FillJetHistograms()
{
  TString histname;
  AliJetContainer* jets = 0;
  TIter nextJetColl(&fJetCollArray);
  while ((jets = static_cast<AliJetContainer*>(nextJetColl()))) {
    TString jetContName = jets->GetName();
    
    Double_t rhoVal = 0;
    if (jets->GetRhoParameter()) {
      rhoVal = jets->GetRhoVal();
      histname = TString::Format("%s/JetHistograms/hRhoVsCent", jets->GetArrayName().Data());
      fHistManager.FillTH2(histname.Data(), fCent, rhoVal);
    }
    
    for (auto jet : jets->all()) {
      
      Float_t ptLeading = jets->GetLeadingHadronPt(jet);
      Float_t corrPt = GetJetPt(jet, rhoVal);
      
      // A vs. pT (fill before area cut)
      histname = TString::Format("%s/JetHistograms/hAreaVsPt", jets->GetArrayName().Data());
      fHistManager.FillTH2(histname.Data(), corrPt, jet->Area());
      
      
      // Rejection reason
      UInt_t rejectionReason = 0;
      if (!jets->AcceptJet(jet, rejectionReason)) {
        histname = TString::Format("%s/JetHistograms/hJetRejectionReason", jets->GetArrayName().Data());
        fHistManager.FillTH2(histname.Data(), jets->GetRejectionReasonBitPosition(rejectionReason), jet->Pt());
        continue;
      }
      
      // compute jet acceptance type
      Double_t type = GetJetType(jet);
      
      // (Centrality, pT, NEF, calo type)
      histname = TString::Format("%s/JetHistograms/hNEFVsPt", jets->GetArrayName().Data());
      Double_t x[6] = {fCent, corrPt, jet->NEF(), type};
      fHistManager.FillTHnSparse(histname, x);
      
      // (Centrality, pT upscaled, calo type)
      if (fComputeMBDownscaling) {
        histname = TString::Format("%s/JetHistograms/hPtUpscaledMB", jets->GetArrayName().Data());
        fHistManager.FillTH3(histname.Data(), fCent, corrPt, type, fMBUpscaleFactor);
      }
      
      // pT-leading vs. pT
      histname = TString::Format("%s/JetHistograms/hPtLeadingVsPt", jets->GetArrayName().Data());
      fHistManager.FillTH2(histname.Data(), corrPt, ptLeading);
      
      // (Centrality, pT, z-leading (charged), calo type)
      TLorentzVector leadPart;
      jets->GetLeadingHadronMomentum(leadPart, jet);
      Double_t z = GetParallelFraction(leadPart.Vect(), jet);
      if (z == 1 || (z > 1 && z - 1 < 1e-3)) z = 0.999; // so that it will contribute to the bin 0.9-1 rather than 1-1.1
      histname = TString::Format("%s/JetHistograms/hZLeadingVsPt", jets->GetArrayName().Data());
      Double_t y[4] = {fCent, corrPt, z, type};
      fHistManager.FillTHnSparse(histname, y);
      
      // (Centrality, pT, z (charged), calo type)
      histname = TString::Format("%s/JetHistograms/hZVsPt", jets->GetArrayName().Data());
      AliVTrack* track;
      for (Int_t i=0; i<jet->GetNumberOfTracks(); i++) {
        track = static_cast<AliVTrack*>(jet->Track(i));
        z = track->Pt() / TMath::Abs(corrPt);
        Double_t y2[4] = {fCent, corrPt, z, type};
        fHistManager.FillTHnSparse(histname, y2);
      }
      
      // (Centrality, pT, Nconst, calo type)
      histname = TString::Format("%s/JetHistograms/hNConstVsPt", jets->GetArrayName().Data());
      Double_t a[4] = {fCent, corrPt, 1.*jet->GetNumberOfConstituents(), type};
      fHistManager.FillTHnSparse(histname, a);
      
      // (Median patch energy, calo type, jet pT, centrality)
      if (fDoTriggerSimulation) {
        histname = TString::Format("%s/JetHistograms/hMedPatchJet", jets->GetArrayName().Data());
        Double_t x[4] = {fMedianEMCal, kEMCal, corrPt, fCent};
        fHistManager.FillTHnSparse(histname, x);
        Double_t y[4] = {fMedianDCal, kDCal, corrPt, fCent};
        fHistManager.FillTHnSparse(histname, y);
      }
      
    } //jet loop
    
  }
}
      
/*
 * This function fills the histograms for the calorimeter performance study.
 */
void AliAnalysisTaskEmcalJetPerformance::FillClusterHistograms()
{
  TString histname;
  
  // If MC, get the MC event
  const AliMCEvent* mcevent = nullptr;
  if (fGeneratorLevel) {
    mcevent = MCEvent();
  }
  else {
    return;
  }
  
  // Loop through clusters, and plot M02 for each particle type
  AliClusterContainer* clusters = GetClusterContainer(0);
  const AliVCluster* clus;
  for (auto it : clusters->accepted_momentum()) {
    
    clus = it.second;
    
    // Include only EMCal clusters (reject DCal and PHOS clusters)
    if (!clus->IsEMCAL()) {
      continue;
    }
    if (it.first.Phi_0_2pi() > fgkEMCalDCalPhiDivide) {
      continue;
    }
    
    // If MC, determine the particle type
    // Each detector-level cluster contains an array of labels of each truth-level particle contributing to the cluster
    ParticleType particleType1 = kUndefined;
    
    Int_t label = TMath::Abs(clus->GetLabel()); // returns mc-label of particle that deposited the most energy in the cluster
    if (label > 0) { // if the particle has a truth-level match, the label is > 0
      
      // Method 1: Use AliMCAnalysisUtils to identify all particles
      particleType1 = GetParticleType1(clus, mcevent, clusters->GetArray());
      
    }
    
    // (M02, Eclus, part type)
    histname = "JetPerformance/hM02VsParticleType";
    fHistManager.FillTH3(histname, clus->GetM02(), clus->GetNonLinCorrEnergy(), particleType1);
    
  }
  
  // Loop through jets, to fill various histograms
  AliJetContainer* jets = GetJetContainer(0); // there is only a single, det-level jet finder here
  for (const auto jet : jets->accepted()) {
    
    Double_t jetPt = GetJetPt(jet, 0);
    
    // Keep track of hadronic calo energy in each jet
    Double_t hadCaloEnergyUnmatched = 0;
    Double_t hadCaloEnergyMatchedNonlincorr = 0;
    Double_t hadCaloEnergyMatchedHadCorr = 0;
    
    // Loop through clusters in each jet, to plot several histograms
    Int_t nClusters = jet->GetNumberOfClusters();
    for (Int_t iClus = 0; iClus < nClusters; iClus++) {
      
      clus = jet->Cluster(iClus);
      
      // Get the particle type of the cluster
      ParticleType particleType1 = kUndefined;
      Int_t label = TMath::Abs(clus->GetLabel());
      if (label > 0) {
        particleType1 = GetParticleType1(clus, mcevent, clusters->GetArray());
      }
      
      // Plot M02 for each particle type
      histname = "JetPerformance/hM02VsParticleTypeJets";
      Double_t x[4] = {clus->GetM02(), clus->GetNonLinCorrEnergy(), particleType1, jetPt};
      fHistManager.FillTHnSparse(histname, x);
      
      // If the cluster is a hadron, sum its energy to compute the jet's hadronic calo energy
      if (particleType1 == kHadron) {
        Bool_t hasMatchedTrack = (clus->GetNTracksMatched() > 0);
        //Bool_t hasMatchedTrack = ((clus->GetNonLinCorrEnergy() - clus->GetHadCorrEnergy()) > 1e-3);
        if (hasMatchedTrack) {
          hadCaloEnergyMatchedNonlincorr += clus->GetNonLinCorrEnergy();
          hadCaloEnergyMatchedHadCorr += clus->GetHadCorrEnergy();
        }
        else {
          hadCaloEnergyUnmatched += clus->GetNonLinCorrEnergy();
        }
      }
      
    }
    
    // Fill hadronic calo energy in each jet
    
    // (Jet pT, Summed energy of hadronic clusters without a matched track)
    histname = "JetPerformance/hHadCaloEnergyUnmatched";
    fHistManager.FillTH2(histname, jetPt, hadCaloEnergyUnmatched);
    
    // (Jet pT vs. Summed energy of hadronic clusters with a matched track (before hadronic correction))
    histname = "JetPerformance/hHadCaloEnergyMatchedNonlincorr";
    fHistManager.FillTH2(histname, jetPt, hadCaloEnergyMatchedNonlincorr);
    
    // (Jet pT vs. Summed energy of hadronic clusters with a matched track (after hadronic correction))
    histname = "JetPerformance/hHadCaloEnergyMatchedHadCorr";
    fHistManager.FillTH2(histname, jetPt, hadCaloEnergyMatchedHadCorr);
    
    // Loop through particle types, and plot jet composition for each particle type
    histname = "JetPerformance/hJetComposition";
    for (Int_t type = 0; type < 8; type++) {
      
      ParticleType particleType1 = kUndefined;
      Double_t nSum = 0;
      Double_t pTsum = 0;
      
      // Loop through clusters in jet, and add to sum if cluster matches particle type
      for (Int_t iClus = 0; iClus < nClusters; iClus++) {
        
        clus = jet->Cluster(iClus);
        
        Int_t label = TMath::Abs(clus->GetLabel());
        if (label > 0) {
          particleType1 = GetParticleType1(clus, mcevent, clusters->GetArray());
        }
        
        if (type == particleType1) {
          nSum++;
          pTsum += clus->GetNonLinCorrEnergy();
        }
      }
      
      // Fill jet composition histogram
      Double_t x[4] = {jetPt, 1.*type, nSum, pTsum};
      fHistManager.FillTHnSparse(histname, x);
      
    }
  }
}

/**
 * This function performs a study of the heavy-ion background.
 */
void AliAnalysisTaskEmcalJetPerformance::ComputeBackground()
{
  // Loop over tracks and clusters in order to:
  //   (1) Compute scale factor for full jets
  //   (2) Compute delta-pT for full jets, with the random cone method
  
  // Define the acceptance boundaries for the TPC and EMCal/DCal/PHOS
  Double_t etaTPC = 0.9;
  Double_t etaEMCal = 0.7;
  //Double_t etaMinDCal = 0.22;
  Double_t phiMinEMCal = fGeom->GetArm1PhiMin() * TMath::DegToRad(); // 80
  Double_t phiMaxEMCal = fGeom->GetEMCALPhiMax() * TMath::DegToRad(); // ~188
  //Double_t phiMinDCal = fGeom->GetDCALPhiMin() * TMath::DegToRad(); // 260
  //Double_t phiMaxDCal = fGeom->GetDCALPhiMax() * TMath::DegToRad(); // ~327 (1/3 SMs start at 320)
  
  Double_t accTPC = 2 * etaTPC * 2 * TMath::Pi();
  Double_t accEMCal = 2 * etaEMCal * (phiMaxEMCal - phiMinEMCal);
  //Double_t accDCalRegion = 2 * etaEMCal * (phiMaxDCal - phiMinDCal);
  
  // Loop over jet containers
  AliJetContainer* jetCont = 0;
  TIter nextJetColl(&fJetCollArray);
  while ((jetCont = static_cast<AliJetContainer*>(nextJetColl()))) {
    
    // Define fiducial acceptances, to be used to generate random cones
    TRandom3* r = new TRandom3(0);
    Double_t jetR = jetCont->GetJetRadius();
    Double_t etaEMCalfid = etaEMCal - jetR;
    Double_t phiMinEMCalfid = phiMinEMCal + jetR;
    Double_t phiMaxEMCalfid = phiMaxEMCal - jetR;
    
    // Generate EMCal random cone eta-phi
    Double_t etaEMCalRC = r->Uniform(-etaEMCalfid, etaEMCalfid);
    Double_t phiEMCalRC = r->Uniform(phiMinEMCalfid, phiMaxEMCalfid);
    
    // Initialize the various sums to 0
    Double_t trackPtSumTPC = 0;
    Double_t trackPtSumEMCal = 0;
    Double_t trackPtSumEMCalRC = 0;
    Double_t clusESumEMCal = 0;
    Double_t clusESumEMCalRC = 0;
    
    // Define a 2D vector (initialized to 0) to store the sum of track pT, and another for cluster ET
    std::vector<std::vector<Double_t>> trackPtSumDCalRC(fNEtaBins, std::vector<Double_t>(fNPhiBins));
    std::vector<std::vector<Double_t>> clusESumDCalRC(fNEtaBins, std::vector<Double_t>(fNPhiBins));
    
    // Loop over tracks. Sum the track pT:
    // (1) in the entire TPC, (2) in the EMCal, (3) in the EMCal random cone,
    AliTrackContainer* trackCont = dynamic_cast<AliTrackContainer*>(GetParticleContainer("tracks"));
    AliTLorentzVector track;
    Double_t trackEta;
    Double_t trackPhi;
    Double_t trackPt;
    Double_t deltaR;
    for (auto trackIterator : trackCont->accepted_momentum() ) {
      
      track.Clear();
      track = trackIterator.first;
      trackEta = track.Eta();
      trackPhi = track.Phi_0_2pi();
      trackPt = track.Pt();
      
      // (1)
      if (TMath::Abs(trackEta) < etaTPC) {
        trackPtSumTPC += trackPt;
      }
      
      // (2)
      if (TMath::Abs(trackEta) < etaEMCal && trackPhi > phiMinEMCal && trackPhi < phiMaxEMCal) {
        trackPtSumEMCal += trackPt;
      }
      
      // (3)
      deltaR = GetDeltaR(&track, etaEMCalRC, phiEMCalRC);
      if (deltaR < jetR) {
        trackPtSumEMCalRC += trackPt;
      }
      
    }
    
    // Loop over clusters. Sum the cluster ET:
    // (1) in the EMCal, (2) in the EMCal random cone
    AliClusterContainer* clusCont = GetClusterContainer(0);
    AliTLorentzVector clus;
    Double_t clusEta;
    Double_t clusPhi;
    Double_t clusE;
    for (auto clusIterator : clusCont->accepted_momentum() ) {
      
      clus.Clear();
      clus = clusIterator.first;
      clusEta = clus.Eta();
      clusPhi = clus.Phi_0_2pi();
      clusE = clus.E();
      
      // (1)
      if (TMath::Abs(clusEta) < etaEMCal && clusPhi > phiMinEMCal && clusPhi < phiMaxEMCal) {
        clusESumEMCal += clusE;
      }
      
      // (2)
      deltaR = GetDeltaR(&clus, etaEMCalRC, phiEMCalRC);
      if (deltaR < jetR) {
        clusESumEMCalRC += clusE;
      }
      
    }
    
    // Compute the scale factor for EMCal, as a function of centrality
    Double_t numerator = (trackPtSumEMCal + clusESumEMCal) / accEMCal;
    Double_t denominator = trackPtSumTPC / accTPC;
    Double_t scaleFactor = numerator / denominator;
    TString histname = TString::Format("%s/BackgroundHistograms/hScaleFactorEMCal", jetCont->GetArrayName().Data());
    fHistManager.FillTH2(histname, fCent, scaleFactor);
    
    // Compute delta pT for EMCal, as a function of centrality
    Double_t rho = jetCont->GetRhoVal();
    Double_t deltaPt = trackPtSumEMCalRC + clusESumEMCalRC - rho * TMath::Pi() * jetR * jetR;
    histname = TString::Format("%s/BackgroundHistograms/hDeltaPtEMCal", jetCont->GetArrayName().Data());
    fHistManager.FillTH2(histname, fCent, deltaPt);
    
    delete r;
    
  }
  
}
      
/**
 * This function performs a loop over the reconstructed jets, when the "simulated" trigger has been fired.
 * in the current event and fills the relevant histograms.
 */
void AliAnalysisTaskEmcalJetPerformance::FillTriggerSimHistograms()
{
  TString histname;
  AliJetContainer* jets = 0;
  TIter nextJetColl(&fJetCollArray);
  while ((jets = static_cast<AliJetContainer*>(nextJetColl()))) {
    TString jetContName = jets->GetName();

    Double_t rhoVal = 0;
    if (jets->GetRhoParameter()) {
      rhoVal = jets->GetRhoVal();
      histname = TString::Format("%s/TriggerSimHistograms/hRhoVsCent", jets->GetArrayName().Data());
      fHistManager.FillTH2(histname.Data(), fCent, rhoVal);
    }
    
    for (auto jet : jets->all()) {
      
      Float_t ptLeading = jets->GetLeadingHadronPt(jet);
      Float_t corrPt = GetJetPt(jet, rhoVal);
      
      // A vs. pT (fill before area cut)
      histname = TString::Format("%s/TriggerSimHistograms/hAreaVsPt", jets->GetArrayName().Data());
      fHistManager.FillTH2(histname.Data(), corrPt, jet->Area());
      
      // Rejection reason
      UInt_t rejectionReason = 0;
      if (!jets->AcceptJet(jet, rejectionReason)) {
        histname = TString::Format("%s/TriggerSimHistograms/hJetRejectionReason", jets->GetArrayName().Data());
        fHistManager.FillTH2(histname.Data(), jets->GetRejectionReasonBitPosition(rejectionReason), jet->Pt());
        continue;
      }
      
      // compute jet acceptance type
      Double_t type = GetJetType(jet);
      
      // (Centrality, pT, NEF, calo type)
      histname = TString::Format("%s/TriggerSimHistograms/hNEFVsPt", jets->GetArrayName().Data());
      Double_t x[6] = {fCent, corrPt, jet->NEF(), type};
      fHistManager.FillTHnSparse(histname, x);
      
      // pT-leading vs. pT
      histname = TString::Format("%s/TriggerSimHistograms/hPtLeadingVsPt", jets->GetArrayName().Data());
      fHistManager.FillTH2(histname.Data(), corrPt, ptLeading);
      
      // (Centrality, pT, z-leading (charged), calo type)
      TLorentzVector leadPart;
      jets->GetLeadingHadronMomentum(leadPart, jet);
      Double_t z = GetParallelFraction(leadPart.Vect(), jet);
      if (z == 1 || (z > 1 && z - 1 < 1e-3)) z = 0.999; // so that it will contribute to the bin 0.9-1 rather than 1-1.1
      histname = TString::Format("%s/TriggerSimHistograms/hZLeadingVsPt", jets->GetArrayName().Data());
      Double_t y[4] = {fCent, corrPt, z, type};
      fHistManager.FillTHnSparse(histname, y);
      
      // (Centrality, pT, z (charged), calo type)
      histname = TString::Format("%s/TriggerSimHistograms/hZVsPt", jets->GetArrayName().Data());
      AliVTrack* track;
      for (Int_t i=0; i<jet->GetNumberOfTracks(); i++) {
        track = static_cast<AliVTrack*>(jet->Track(i));
        z = track->Pt() / TMath::Abs(corrPt);
        Double_t y2[4] = {fCent, corrPt, z, type};
        fHistManager.FillTHnSparse(histname, y2);
      }
      
      // (Centrality, pT, Nconst, calo type)
      histname = TString::Format("%s/TriggerSimHistograms/hNConstVsPt", jets->GetArrayName().Data());
      Double_t a[4] = {fCent, corrPt, 1.*jet->GetNumberOfConstituents(), type};
      fHistManager.FillTHnSparse(histname, a);
      
    } //jet loop
    
  }
}

/*
 * Compute the MC particle type using AliMCAnalysisUtils
 */
AliAnalysisTaskEmcalJetPerformance::ParticleType AliAnalysisTaskEmcalJetPerformance::GetParticleType1(const AliVCluster* clus, const AliMCEvent* mcevent, const TClonesArray* clusArray)
{
  ParticleType particleType = kUndefined;
  
  AliMCAnalysisUtils mcUtils;
  Int_t tag = mcUtils.CheckOrigin(clus->GetLabels(), clus->GetNLabels(), mcevent, clusArray);

  Bool_t isPhoton = mcUtils.CheckTagBit(tag, AliMCAnalysisUtils::kMCPhoton);
  Bool_t isPi0 = mcUtils.CheckTagBit(tag, AliMCAnalysisUtils::kMCPi0);
  Bool_t isConversion = mcUtils.CheckTagBit(tag, AliMCAnalysisUtils::kMCConversion);
  Bool_t isEta = mcUtils.CheckTagBit(tag, AliMCAnalysisUtils::kMCEta);
  Bool_t isPion = mcUtils.CheckTagBit(tag, AliMCAnalysisUtils::kMCPion);
  Bool_t isKaon = mcUtils.CheckTagBit(tag, AliMCAnalysisUtils::kMCKaon);
  Bool_t isProton = mcUtils.CheckTagBit(tag, AliMCAnalysisUtils::kMCProton);
  Bool_t isAntiProton = mcUtils.CheckTagBit(tag, AliMCAnalysisUtils::kMCAntiProton);
  Bool_t isNeutron = mcUtils.CheckTagBit(tag, AliMCAnalysisUtils::kMCNeutron);
  Bool_t isAntiNeutron = mcUtils.CheckTagBit(tag, AliMCAnalysisUtils::kMCAntiNeutron);
  Bool_t isElectron = mcUtils.CheckTagBit(tag, AliMCAnalysisUtils::kMCElectron);
  Bool_t isMuon = mcUtils.CheckTagBit(tag, AliMCAnalysisUtils::kMCMuon);
  
  if (isPi0) {
    if (isConversion) {
      particleType = kPi0Conversion;
    }
    else {
      particleType = kPi0;
    }
  }
  else if (isEta) {
    particleType = kEta;
  }
  else if (isPhoton) {
    particleType = kPhoton;
  }
  else if (isPion || isKaon || isProton || isAntiProton || isNeutron || isAntiNeutron) {
    particleType = kHadron;
  }
  else if (isElectron) {
    particleType = kElectron;
  }
  else if (isMuon) {
    particleType = kMuon;
  }
  else {
    particleType = kOther;
  }
  return particleType;
}

/*
 * Compute the MC particle type using the MC particle container (and only AliMCAnalysisUtils to find merged pi0)
 */
AliAnalysisTaskEmcalJetPerformance::ParticleType AliAnalysisTaskEmcalJetPerformance::GetParticleType2(const AliVCluster* clus, const AliMCEvent* mcevent, Int_t label, const AliClusterContainer* clusters)
{
  ParticleType particleType = kUndefined;

  AliAODMCParticle *part = fGeneratorLevel->GetMCParticleWithLabel(label);
  if (part) {
  
    TString histname = TString::Format("%s/hClusterRejectionReasonMC", clusters->GetArrayName().Data());
    UInt_t rejectionReason = 0;
    if (!fGeneratorLevel->AcceptMCParticle(part, rejectionReason)) {
      fHistManager.FillTH2(histname, fGeneratorLevel->GetRejectionReasonBitPosition(rejectionReason), clus->GetNonLinCorrEnergy());
      return particleType;
    }

    if (part->GetGeneratorIndex() == 0) { // generator index in cocktail
      
      // select charged pions, protons, kaons, electrons, muons
      Int_t pdg = TMath::Abs(part->PdgCode()); // abs value ensures both particles and antiparticles are included
      
      if (pdg == 22) { // gamma 22
        
        AliMCAnalysisUtils mcUtils;
        Int_t tag;
        mcUtils.CheckOverlapped2GammaDecay(clus->GetLabels(), clus->GetNLabels(), part->GetMother(), mcevent, tag);
        
        if (mcUtils.CheckTagBit(tag, AliMCAnalysisUtils::kMCPi0)) {
          if (mcUtils.CheckTagBit(tag, AliMCAnalysisUtils::kMCConversion)) {
            particleType = kPi0Conversion;
          }
          else {
            particleType = kPi0;
          }
        }
        else if (mcUtils.CheckTagBit(tag, AliMCAnalysisUtils::kMCEta)) {
          particleType = kEta;
        }
        else { // direct photon
          particleType = kPhoton;
        }
        
      }
      else if (pdg == 211 || 2212 || 321 || 2112) { // pi+ 211, proton 2212, K+ 321, neutron 2112
        particleType = kHadron;
      }
      else if (pdg == 11) { // e- 11
        particleType = kElectron;
      }
      else if (pdg == 13) { // mu- 13
        particleType = kMuon;
      }
      else {
        particleType = kOther;
      }
    }
  }
  return particleType;
}

/**
 * Get deltaR of a track/cluster and a reference point.
 */
Double_t AliAnalysisTaskEmcalJetPerformance::GetDeltaR(const AliTLorentzVector* part, Double_t etaRef, Double_t phiRef)
{
  Double_t deltaPhi = TMath::Abs(part->Phi_0_2pi() - phiRef);
  Double_t deltaEta = TMath::Abs(part->Eta() - etaRef);
  Double_t deltaR = TMath::Sqrt( deltaPhi*deltaPhi + deltaEta*deltaEta );
  return deltaR;
}

/**
 * Get calo acceptance type of jet
 */
Double_t AliAnalysisTaskEmcalJetPerformance::GetJetType(const AliEmcalJet* jet)
{
  UInt_t jetType = jet->GetJetAcceptanceType();
  Double_t type = -1;
  if (jetType & AliEmcalJet::kEMCAL) {
    type = kEMCal;
  }
  else if (jetType & AliEmcalJet::kDCALonly) {
    type = kDCal;
  }
  
  return type;
}

/**
 * Get pT of jet -- background subtracted
 */
Double_t AliAnalysisTaskEmcalJetPerformance::GetJetPt(const AliEmcalJet* jet, Double_t rho)
{
  Double_t pT = jet->Pt() - rho * jet->Area();
  return pT;
}
