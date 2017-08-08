#include "AliAnalysisTaskTPCTOFPID.h"
#include "AliESDEvent.h"
#include "AliMCEvent.h"
//#include "AliMCParticle.h"
//#include "AliStack.h"
#include "AliPhysicsSelection.h"
#include "AliESDtrackCuts.h"
#include "AliESDpid.h"
#include "AliTOFcalib.h"
#include "AliTOFT0maker.h"
#include "TList.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TH1D.h"
#include "AliCDBManager.h"
#include "AliLog.h"
#include "AliESDtrack.h"
#include "TObjArray.h"
#include "TLorentzVector.h"
#include "TParticle.h"
#include "TDatabasePDG.h"
#include "TParticlePDG.h"
#include "AliAnalysisPIDTrack.h"
#include "AliAnalysisPIDParticle.h"
#include "AliAnalysisPIDEvent.h"
#include "TClonesArray.h"
#include "AliAnalysisManager.h"
#include "AliAODHandler.h"
#include "TTree.h"
#include "AliCentrality.h"
#include "AliInputEventHandler.h"
#include "AliVEvent.h"
#include "AliPIDResponse.h"
#include "AliAnalysisUtils.h"
#include "AliMultSelection.h"
#include "TRandom.h"
#include "AliESDVertex.h"
#include "AliESDv0.h"
#include "AliAnalysisPIDV0.h"
#include "AliAODVertex.h"
#include "AliKFParticle.h"
#include "AliKFVertex.h"
#include "AliVVZERO.h"
#include "AliVCluster.h"
#include "TMath.h"

ClassImp(AliAnalysisTaskTPCTOFPID)
  
//_______________________________________________________
  
AliAnalysisTaskTPCTOFPID::AliAnalysisTaskTPCTOFPID() :
  AliAnalysisTaskSE("AnalysisResults"),
  fInitFlag(kFALSE),
  fMCFlag(kFALSE),
  fMCTuneFlag(kFALSE),
  fPbPbFlag(kFALSE),
  fVertexSelectionFlag(kFALSE),
  fPrimaryDCASelectionFlag(kFALSE),
  fPIDTree(0),
  fEvHist(0),
  fPIDResponse(0),
  fAnUtils(0),
  fRunNumber(0),
  fStartTime(0),
  fEndTime(0),
  fESDEvent(NULL),
  fMCEvent(NULL),
//fMCStack(NULL),
  fTrackCuts2010(NULL),
  fTrackCuts2011(NULL),
  fTrackCutsTPCRefit(NULL),
  fTrackCuts2011Sys(NULL),
  fESDpid(new AliESDpid()),
  fIsCollisionCandidate(kFALSE),
  fIsEventSelected(0),
  fIsPileupFromSPD(kFALSE),
  fHasVertex(kFALSE),
  fVertexZ(0.),
  fMCTimeZero(0.),
  fCentrality(NULL),
  fAnalysisEvent(new AliAnalysisPIDEvent()),
  fAnalysisTrackArray(new TClonesArray("AliAnalysisPIDTrack")),
  fAnalysisTrack(new AliAnalysisPIDTrack()),
  fAnalysisParticleArray(new TClonesArray("AliAnalysisPIDParticle")),
  fAnalysisParticle(new AliAnalysisPIDParticle()),
  fAnalysisV0TrackArray(new TClonesArray("AliAnalysisPIDV0")),
  fAnalysisV0Track(new AliAnalysisPIDV0()),
  fTOFcalib(new AliTOFcalib()),
  fTOFT0maker(new AliTOFT0maker(fESDpid)),
  fTimeResolution(80.),
  fVertexCut(10.0),
  fRapidityCut(1.0),
  V0MBinCount(0),
  fHistoList(new TList()),
  fMCHistoList(new TList())
{
  /* 
   * default constructor 
   */
  fTrackCuts2010 = new AliESDtrackCuts("AliESDtrackCuts2010","AliESDtrackCuts2010");
  fTrackCuts2010 = AliESDtrackCuts::GetStandardITSTPCTrackCuts2010(kFALSE); //If not running, set to kFALSE;
  fTrackCuts2010->SetEtaRange(-0.8,0.8);
  fTrackCuts2011 = new AliESDtrackCuts("AliESDtrackCuts2011","AliESDtrackCuts2011");
  fTrackCuts2011 = AliESDtrackCuts::GetStandardITSTPCTrackCuts2011(kFALSE); //If not running, set to kFALSE;
  fTrackCuts2011->SetEtaRange(-0.8,0.8);
  fTrackCutsTPCRefit = new AliESDtrackCuts("AliESDtrackCutsTPCRefit","AliESDtrackCutsTPCRefit");
  fTrackCutsTPCRefit = AliESDtrackCuts::GetStandardTPCOnlyTrackCuts(); //If not running, set to kFALSE;
  fTrackCutsTPCRefit->SetRequireTPCRefit(kTRUE);
  fTrackCutsTPCRefit->SetEtaRange(-0.8,0.8);
  //Following is TC for systematics estimation. To compensate, once should probably reduce gamma DeltaM even further :)
  fTrackCuts2011Sys = new AliESDtrackCuts("AliESDtrackCuts2011Sys","AliESDtrackCuts2011Sys");
  fTrackCuts2011Sys = AliESDtrackCuts::GetStandardITSTPCTrackCuts2011(kFALSE); //If not running, set to kFALSE;                                                                                            
  fTrackCuts2011Sys->SetEtaRange(-0.8,0.8);
  fTrackCuts2011Sys->SetMinNCrossedRowsTPC(60);
  fTrackCuts2011Sys->SetMaxChi2PerClusterTPC(5);
  fTrackCuts2011Sys->SetMaxDCAToVertexZ(3);

}




AliAnalysisTaskTPCTOFPID::AliAnalysisTaskTPCTOFPID(Bool_t isMC) :
  AliAnalysisTaskSE("AnalysisResults"),
  fInitFlag(kFALSE),
  fMCFlag(kFALSE),
  fMCTuneFlag(kFALSE),
  fPbPbFlag(kFALSE),
  fVertexSelectionFlag(kFALSE),
  fPrimaryDCASelectionFlag(kFALSE),
  fPIDTree(0),
  fEvHist(0),
  fPIDResponse(0),
  fAnUtils(0),
  fRunNumber(0),
  fStartTime(0),
  fEndTime(0),
  fESDEvent(NULL),
  fMCEvent(NULL),
  //fMCStack(NULL),
  fTrackCuts2010(NULL),
  fTrackCuts2011(NULL),
  fTrackCutsTPCRefit(NULL),
  fTrackCuts2011Sys(NULL),
  fESDpid(new AliESDpid()),
  fIsCollisionCandidate(kFALSE),
  fIsEventSelected(0),
  fIsPileupFromSPD(kFALSE),
  fHasVertex(kFALSE),
  fVertexZ(0.),
  fMCTimeZero(0.),
  fCentrality(NULL),
  fAnalysisEvent(new AliAnalysisPIDEvent()),
  fAnalysisTrackArray(new TClonesArray("AliAnalysisPIDTrack")),
  fAnalysisTrack(new AliAnalysisPIDTrack()),
  fAnalysisParticleArray(new TClonesArray("AliAnalysisPIDParticle")),
  fAnalysisParticle(new AliAnalysisPIDParticle()),
  fAnalysisV0TrackArray(new TClonesArray("AliAnalysisPIDV0")),
  fAnalysisV0Track(new AliAnalysisPIDV0()),
  fTOFcalib(new AliTOFcalib()),
  fTOFT0maker(new AliTOFT0maker(fESDpid)),
  fTimeResolution(80.),
  fVertexCut(10.0),
  fRapidityCut(1.0),
  V0MBinCount(0),
  fHistoList(new TList()),
  fMCHistoList(new TList())
{
  /* 
   * default constructor 
   */
  fTrackCuts2010 = new AliESDtrackCuts("AliESDtrackCuts2010","AliESDtrackCuts2010");
  fTrackCuts2010 = AliESDtrackCuts::GetStandardITSTPCTrackCuts2010(kFALSE); //If not running, set to kFALSE;
  fTrackCuts2010->SetEtaRange(-0.8,0.8);
  fTrackCuts2011 = new AliESDtrackCuts("AliESDtrackCuts2011","AliESDtrackCuts2011");
  fTrackCuts2011 = AliESDtrackCuts::GetStandardITSTPCTrackCuts2011(kFALSE); //If not running, set to kFALSE;
  fTrackCuts2011->SetEtaRange(-0.8,0.8);
  fTrackCutsTPCRefit = new AliESDtrackCuts("AliESDtrackCutsTPCRefit","AliESDtrackCutsTPCRefit");
  fTrackCutsTPCRefit = AliESDtrackCuts::GetStandardTPCOnlyTrackCuts(); //If not running, set to kFALSE;
  fTrackCutsTPCRefit->SetRequireTPCRefit(kTRUE);
  fTrackCutsTPCRefit->SetEtaRange(-0.8,0.8);
  //Following is TC for systematics estimation. To compensate, once should probably reduce gamma DeltaM even further :)                                                                                    
  fTrackCuts2011Sys = new AliESDtrackCuts("AliESDtrackCuts2011Sys","AliESDtrackCuts2011Sys");
  fTrackCuts2011Sys = AliESDtrackCuts::GetStandardITSTPCTrackCuts2011(kFALSE); //If not running, set to kFALSE;                                                                                            
  fTrackCuts2011Sys->SetEtaRange(-0.8,0.8);
  fTrackCuts2011Sys->SetMinNCrossedRowsTPC(60);
  fTrackCuts2011Sys->SetMaxChi2PerClusterTPC(5);
  fTrackCuts2011Sys->SetMaxDCAToVertexZ(3);

  fMCFlag = isMC;
  DefineOutput(1, TTree::Class());
  DefineOutput(2, TH1D::Class());
}













//_______________________________________________________

AliAnalysisTaskTPCTOFPID::~AliAnalysisTaskTPCTOFPID()
{
  /*
   * default destructor
   */

  if (fTrackCuts2010) delete fTrackCuts2010;
  if (fTrackCuts2011) delete fTrackCuts2011;
  if (fTrackCutsTPCRefit) delete fTrackCutsTPCRefit;
  if (fTrackCuts2011Sys) delete fTrackCuts2011Sys;
  delete fESDpid;
  delete fTOFcalib;
  delete fTOFT0maker;
  delete fHistoList;
  delete fMCHistoList;
}


//________________________________________________________________________

void
AliAnalysisTaskTPCTOFPID::UserCreateOutputObjects()
{
  /*
   * user create output objects
   */
  OpenFile(1);
  OpenFile(2);
  /* output tree */
  fPIDTree = new TTree("PIDTree","PIDTree");
  fPIDTree->Branch("AnalysisEvent", "AliAnalysisPIDEvent", &fAnalysisEvent);  
  fPIDTree->Branch("AnalysisTrack", "TClonesArray", &fAnalysisTrackArray); 
  fPIDTree->Branch("AnalysisV0Track","TClonesArray",&fAnalysisV0TrackArray);
  if (fMCFlag)
    fPIDTree->Branch("AnalysisParticle", "TClonesArray", &fAnalysisParticleArray);


  AliAnalysisManager *man=AliAnalysisManager::GetAnalysisManager();
  AliInputEventHandler* inputHandler = (AliInputEventHandler*) (man->GetInputEventHandler());
  fPIDResponse = inputHandler->GetPIDResponse(); 
  fAnUtils = new AliAnalysisUtils();
  Double_t V0MbinsDefault[13] = {0, 0.01, 0.1, 1, 5, 10, 15, 20, 30, 40, 50, 70, 100};
  V0MBinCount = new TH1F("V0MBinCount","V0MBinCount",12,V0MbinsDefault);

  fEvHist = new TH1D("StatHist","StatHist",10,-0.5,9.5);
  PostData(1,fPIDTree);
  PostData(2,fEvHist);
}

//_______________________________________________________





Bool_t
AliAnalysisTaskTPCTOFPID::InitRun()
{
  /*
   * init run
   */

  /* get ESD event */
  fESDEvent = dynamic_cast<AliESDEvent *>(InputEvent());
  if (!fESDEvent) {
    AliError("cannot get ESD event");
    return kFALSE;
  }

  /* get run number */
  Int_t runNb = fESDEvent->GetRunNumber();
  /* check run already initialized */
  if (fInitFlag && fRunNumber == runNb) return kTRUE;
  /* init cdb */
  AliCDBManager *cdb = AliCDBManager::Instance();
  cdb->SetDefaultStorage("raw://");
  cdb->SetRun(runNb);
  /* init TOF calib */
  if (!fTOFcalib->Init()) {
    AliError("cannot init TOF calib");
    return kFALSE;
  }
  AliInfo(Form("initialized for run %d", runNb));
  fInitFlag = kTRUE;
  fRunNumber = runNb;
  return kTRUE;
}

//_______________________________________________________
void AliAnalysisTaskTPCTOFPID::FillHist(Double_t myflag) {
  fEvHist->Fill(myflag);
};
Bool_t AliAnalysisTaskTPCTOFPID::IsGoodSPDvertexRes(const AliESDVertex * spdVertex)
{
  if (!spdVertex) return kFALSE;
  if (spdVertex->IsFromVertexerZ() && !(spdVertex->GetDispersion()<0.04 && spdVertex->GetZRes()<0.25)) return kFALSE;
  return kTRUE;
};
Bool_t AliAnalysisTaskTPCTOFPID::SelectVertex2015pp(AliESDEvent *esd,  Bool_t *SPDandTrkExists, Bool_t checkSPDres, Bool_t checkProximity) 
{
  if (!esd) return kFALSE;
  const AliESDVertex * trkVertex = esd->GetPrimaryVertexTracks();
  const AliESDVertex * spdVertex = esd->GetPrimaryVertexSPD();
  Bool_t hasSPD = spdVertex->GetStatus();
  Bool_t hasTrk = trkVertex->GetStatus();
 
  //Note that AliVertex::GetStatus checks that N_contributors is > 0
  //reject events if both are explicitly requested and none is available
  //MOD: do not reject if SPD&Trk vtx. not there, but store it to the variable, if requested:
  if(SPDandTrkExists)
    (*SPDandTrkExists) = hasSPD&&hasTrk;
  //  if (requireSPDandTrk && !(hasSPD && hasTrk)) return kFALSE;
  
  //reject events if none between the SPD or track verteces are available
  //if no trk vertex, try to fall back to SPD vertex;
  if (!hasTrk) {
    if (!hasSPD) return kFALSE;
    //on demand check the spd vertex resolution and reject if not satisfied
    if (checkSPDres && !IsGoodSPDvertexRes(spdVertex)) return kFALSE;
  } else {
    if (hasSPD) {
      //if enabled check the spd vertex resolution and reject if not satisfied
      //if enabled, check the proximity between the spd vertex and trak vertex, and reject if not satisfied
      if (checkSPDres && !IsGoodSPDvertexRes(spdVertex)) return kFALSE;
      if ((checkProximity && TMath::Abs(spdVertex->GetZ() - trkVertex->GetZ())>0.5)) return kFALSE; 
    }
  }

  //Cut on the vertex z position
  const AliESDVertex * vertex = esd->GetPrimaryVertex();
  if (TMath::Abs(vertex->GetZ())>10) return kFALSE;
  return kTRUE;
};


Bool_t
AliAnalysisTaskTPCTOFPID::InitEvent()
{
  /*
   * init event
   */

  /* get ESD event */
  fESDEvent = dynamic_cast<AliESDEvent *>(InputEvent());
  FillHist(0);
  if (!fESDEvent) return kFALSE;
  /* get MC event */
  FillHist(1);
  if (fMCFlag) {
    fMCEvent = dynamic_cast<AliMCEvent *>(MCEvent());
    if (!fMCEvent) return kFALSE;
  }
  /* get stack */
  //Stack is gone. Farewll, stack, you've served us well.
  /*  if (fMCFlag) {
    fMCStack = fMCEvent->Stack();
    if (!fMCStack) return kFALSE;
    }*/
  FillHist(2);
  /* event selection */
  fIsCollisionCandidate = (((AliInputEventHandler*)(AliAnalysisManager::GetAnalysisManager()->GetInputEventHandler()))->IsEventSelected() & AliVEvent::kAny);
  fIsEventSelected = ((AliInputEventHandler*)(AliAnalysisManager::GetAnalysisManager()->GetInputEventHandler()))->IsEventSelected();
  fIsPileupFromSPD = fESDEvent->IsPileupFromSPD();
  FillHist(3);  
  if(fESDEvent->IsIncompleteDAQ()) return kFALSE;
  if(fAnUtils->IsSPDClusterVsTrackletBG(fESDEvent)) return kFALSE;
  /* vertex selection */
  const AliESDVertex *vertex = fESDEvent->GetPrimaryVertexTracks();
  if (vertex->GetNContributors() < 1) {
    vertex = fESDEvent->GetPrimaryVertexSPD();
    if (vertex->GetNContributors() < 1) fHasVertex = kFALSE;
    else fHasVertex = kTRUE;
    TString vtxTyp = vertex->GetTitle();
    Double_t cov[6]={0};
    vertex->GetCovarianceMatrix(cov);
    Double_t zRes = TMath::Sqrt(cov[5]);
    if (vtxTyp.Contains("vertexer:Z") && (zRes>0.25)) fHasVertex = kFALSE;
  }
  else fHasVertex = kTRUE;

  
  fVertexZ = vertex->GetZ();
  /* centrality in PbPb */
  if (fPbPbFlag) {
    fCentrality = fESDEvent->GetCentrality();
  }
  /* calibrate ESD (also in MC to correct TExp) */
  fTOFcalib->CalibrateESD(fESDEvent);

  /* check MC flags */
  if (fMCFlag && fMCTuneFlag)
    fMCTimeZero = fTOFcalib->TuneForMC(fESDEvent, fTimeResolution);

#if 0 /* NEW METHOD */
  /* compute TOF-T0, fill ESD and make PID */
  fTOFT0maker->ComputeT0TOF(fESDEvent);
  fTOFT0maker->WriteInESD(fESDEvent);
  fESDpid->SetTOFResponse(fESDEvent, AliESDpid::kTOF_T0);
  fESDpid->MakePID(fESDEvent, kFALSE, 0.);
#endif

#if 1 /* OLD METHOD */
  /* compute and apply TOF-T0 */
  fTOFT0maker->ComputeT0TOF(fESDEvent);
  fESDpid->MakePID(fESDEvent, kFALSE, 0.);
#endif
  
  return kTRUE;
}

//_______________________________________________________

Bool_t
AliAnalysisTaskTPCTOFPID::HasPrimaryDCA(AliESDtrack *track)
{
  /*
   * has primary DCA
   */
  
  // cut on transverse impact parameter
  Float_t d0z0[2],covd0z0[3];
  track->GetImpactParameters(d0z0, covd0z0);
  Float_t sigma= 0.0050 + 0.0060 / TMath::Power(track->Pt(), 0.9);
  Float_t d0max = 7. * sigma;
  //
  Float_t sigmaZ = 0.0146 + 0.0070 / TMath::Power(track->Pt(), 1.114758);
  if (track->Pt() > 1.) sigmaZ = 0.0216;
  Float_t d0maxZ = 5. * sigmaZ;
  //
  if(TMath::Abs(d0z0[0]) > d0max || TMath::Abs(d0z0[1]) > d0maxZ)
    return kFALSE;
  
  /* primary DCA ok */
  return kTRUE;
}
Int_t AliAnalysisTaskTPCTOFPID::GetTrackCutsFlag(AliESDtrack *LocalTrack) {
  Int_t ReturnFlag = 0;
  if(fTrackCuts2010->AcceptTrack(LocalTrack)) ReturnFlag+=1;
  if(fTrackCuts2011->AcceptTrack(LocalTrack)) ReturnFlag+=2;
  if(fTrackCutsTPCRefit->AcceptTrack(LocalTrack)) ReturnFlag+=4;
  if(fTrackCuts2011Sys->AcceptTrack(LocalTrack)) ReturnFlag+=8;
  return ReturnFlag;
};
//_______________________________________________________
void AliAnalysisTaskTPCTOFPID::ProcessV0s() {
  Int_t NV0s = fESDEvent->GetNumberOfV0s();
  if(NV0s<1) return;
  const AliESDVertex *BestPrimaryVertex = fESDEvent->GetPrimaryVertex();
  if(!BestPrimaryVertex) return;
  if(!(BestPrimaryVertex->GetStatus())) return;
  Double_t IPrimaryVtxPosition[3];
  BestPrimaryVertex->GetXYZ(IPrimaryVtxPosition);
  Double_t IPrimaryVtxCov[6];
  BestPrimaryVertex->GetCovMatrix(IPrimaryVtxCov);
  Double_t IPrimaryVtxChi2 = BestPrimaryVertex->GetChi2toNDF();
  AliAODVertex *PrimaryVertex = new AliAODVertex(IPrimaryVtxPosition,IPrimaryVtxCov,IPrimaryVtxChi2,NULL,-1,AliAODVertex::kPrimary);


  Double_t InvMasses[4];
  fAnalysisV0TrackArray->Clear();
  for(Int_t iV0=0;iV0<NV0s;iV0++) {
    AliESDv0 *V0Vertex = fESDEvent->GetV0(iV0);
    if(!V0Vertex) continue;
    //fAnalysisTrack->Update(track, fMCStack, fMCEvent,fPIDResponse, fTrackCuts->AcceptTrack(track));
    AliAnalysisPIDTrack *pTrack = new AliAnalysisPIDTrack();
    AliAnalysisPIDTrack *nTrack = new AliAnalysisPIDTrack();
    AliESDtrack *temptrack = fESDEvent->GetTrack((UInt_t)TMath::Abs(V0Vertex->GetPindex()));
    pTrack->Update(temptrack, fMCEvent,fPIDResponse, GetTrackCutsFlag(temptrack));
    temptrack = fESDEvent->GetTrack((UInt_t)TMath::Abs(V0Vertex->GetNindex()));
    nTrack->Update(temptrack, fMCEvent,fPIDResponse, GetTrackCutsFlag(temptrack));
    
    //    AliESDtrack *nTrack = fESDEvent->GetTrack((UInt_t)TMath::Abs(V0Vertex->GetNindex()));
    if(!pTrack||!nTrack) continue;
    if(pTrack->GetSign()==nTrack->GetSign()) continue; //Remove like-sign
    //if(TMath::Abs(pTrack->GetEta())>0.8 || TMath::Abs(nTrack->GetEta())>0.8) continue; //Eta cut
    //if(pTrack->GetPt()<2) continue; //pT cut on decay product
    //if(nTrack->GetPt()<2) continue; //pT cut on decay product
    Bool_t ChargesSwitched=kFALSE;
    if(pTrack->GetSign()<0) {
      AliAnalysisPIDTrack *ttr = nTrack;
      nTrack = pTrack;//fESDEvent->GetTrack((UInt_t)TMath::Abs(V0Vertex->GetPindex()));
      pTrack = ttr;//nTrack;//fESDEvent->GetTrack((UInt_t)TMath::Abs(V0Vertex->GetNindex()));
      ChargesSwitched=kTRUE;
    };
    Double_t alpha = V0Vertex->AlphaV0(); //Probably save these
    Double_t ptarm = V0Vertex->PtArmV0(); //Probably save these
    Double_t IV0Position[3];
    V0Vertex->GetXYZ(IV0Position[0],IV0Position[1],IV0Position[2]);
    Double_t IV0Radius = TMath::Sqrt(IV0Position[0]*IV0Position[0]+IV0Position[1]*IV0Position[1]);
    if(IV0Radius>100||IV0Radius<5) continue;
    AliKFVertex PrimaryVtxKF(*PrimaryVertex);
    AliKFParticle::SetField(fESDEvent->GetMagneticField());


    Int_t indecies[2] = {211,2212}; //pi,p
    Double_t myMasses[] = {0.498,1.116,1.116}; //K0s, lambda, anti-lambda
    AliKFParticle *negKF[2] = {0,0}; //-pi, -p
    AliKFParticle *posKF[2] = {0,0}; // pi,  p
    if(ChargesSwitched)
      for(Int_t i=0;i<2;i++) {
	negKF[i] = new AliKFParticle(*(V0Vertex->GetParamP()), -indecies[i]);
	posKF[i] = new AliKFParticle(*(V0Vertex->GetParamN()), indecies[i]);
      } else
      for(Int_t i=0;i<2;i++) {
	negKF[i] = new AliKFParticle(*(V0Vertex->GetParamN()), -indecies[i]);
	posKF[i] = new AliKFParticle(*(V0Vertex->GetParamP()), indecies[i]);
      };
    AliKFParticle V0KFs[3];
    Bool_t TrashTracks=kTRUE; //Trash tracks if all below inv. mass
    for(Int_t i=0;i<3;i++) {
      V0KFs[i]+=(*posKF[(i==1)?1:0]);
      V0KFs[i]+=(*negKF[(i==2)?1:0]);
      V0KFs[i].SetProductionVertex(PrimaryVtxKF);
      InvMasses[i] = V0KFs[i].GetMass()-myMasses[i];
	TrashTracks = TrashTracks&&(TMath::Abs(InvMasses[i])>0.06);
    };
    if(TrashTracks) continue;
    Double_t lpT = V0Vertex->Pt();
    Double_t lEta = V0Vertex->Eta();
    fAnalysisV0Track->Update(pTrack,nTrack,InvMasses,IV0Radius,V0Vertex->GetDcaV0Daughters(), V0Vertex->GetV0CosineOfPointingAngle(),lpT,lEta);
    new ((*fAnalysisV0TrackArray)[fAnalysisV0TrackArray->GetEntries()]) AliAnalysisPIDV0(*fAnalysisV0Track);
    
  };

};


void
AliAnalysisTaskTPCTOFPID::UserExec(Option_t *option)
{
  /*
   * user exec
   */

  /*** INITIALIZATION ***/

  /* init run */
  if (!InitRun()) return;
  FillHist(4);
  /* init event */
  if (!InitEvent()) return;
  FillHist(5);
  fAnalysisEvent->Reset();
  Int_t EventSelectionFlag = 0;
  Float_t V0MPercentile = -1000;
  AliMultSelection *ams = (AliMultSelection*)fESDEvent->FindListObject("MultSelection");
  if(!ams)
    V0MPercentile = -999;
  else {
    V0MPercentile = ams->GetMultiplicityPercentile("V0M");
    if(ams->GetThisEventIsNotPileup()) EventSelectionFlag += AliAnalysisPIDEvent::kNotPileupInSPD;
    if(ams->GetThisEventIsNotPileupMV()) EventSelectionFlag += AliAnalysisPIDEvent::kNotPileupInMV;
    if(ams->GetThisEventIsNotPileupInMultBins()) EventSelectionFlag += AliAnalysisPIDEvent::kNotPileupInMB;
    if(ams->GetThisEventINELgtZERO()) EventSelectionFlag+=AliAnalysisPIDEvent::kINELgtZERO;
    if(ams->GetThisEventHasNoInconsistentVertices()) EventSelectionFlag+=AliAnalysisPIDEvent::kNoInconsistentVtx;
    if(ams->GetThisEventIsNotAsymmetricInVZERO()) EventSelectionFlag+=AliAnalysisPIDEvent::kNoV0Asym;
  };
  Bool_t lSPDandTrkVtxExists=kFALSE;
  if(SelectVertex2015pp(fESDEvent,&lSPDandTrkVtxExists)) EventSelectionFlag+=AliAnalysisPIDEvent::kVertexSelected2015pp;
  if(lSPDandTrkVtxExists) EventSelectionFlag+=AliAnalysisPIDEvent::kSPDandTrkVtxExists;
  fAnalysisEvent->SetV0Mmultiplicity(V0MPercentile);
  fAnalysisEvent->SetEventFlags(EventSelectionFlag);
  AliVVZERO *v0 = fESDEvent->GetVZEROData();
  for(Int_t i=0;i<32;i++) fAnalysisEvent->SetV0CellAmplitude(i,v0->GetMultiplicityV0A(i));
  for(Int_t i=32;i<64;i++) fAnalysisEvent->SetV0CellAmplitude(i,v0->GetMultiplicityV0C(i-32));
  

  /*** MC PRIMARY PARTICLES ***/

  Int_t mcmulti = 0;
  if (fMCFlag) {

    /* reset track array */
    fAnalysisParticleArray->Clear();
    
    /* loop over primary particles */
    Int_t nPrimaries = fMCEvent->GetNumberOfPrimaries();//fMCStack->GetNprimary();
    TParticle *particle;
    TParticlePDG *particlePDG;
    /* loop over primary particles */
    for (Int_t ipart = 0; ipart < nPrimaries; ipart++) {
      Bool_t OWSave=kFALSE; //Overwrite save -- used to add other particle than primaries
      /* get particle */
      particle = fMCEvent->Particle(ipart);//((AliMCParticle*)fMCEvent->GetTrack(ipart))->Particle();//fMCStack->Particle(ipart);
      if (!particle) continue;
      /* get particlePDG */
      particlePDG = particle->GetPDG();
      Int_t pdgcode = TMath::Abs(particle->GetPdgCode());
      if (!particlePDG) continue;
      OWSave = ((pdgcode==333)||(pdgcode==310)||(pdgcode==3122)||(pdgcode==11));

      /* check primary */
      if ((!fMCEvent->IsPhysicalPrimary(ipart))&&(!OWSave)) continue;

      /* check charged */
      if ((particlePDG->Charge()==0.)&&(!OWSave)) continue;
      mcmulti++;
      /* check rapidity and pt cuts */
      if (TMath::Abs(particle->Y()) > fRapidityCut) continue;
      if (particle->Pt() < 0.15) continue;
      /* update and add analysis particle */
      fAnalysisParticle->Update(particle, ipart);
      new ((*fAnalysisParticleArray)[fAnalysisParticleArray->GetEntries()]) AliAnalysisPIDParticle(*fAnalysisParticle);
    } /* end of loop over primary particles */

    
  }

  /*** GLOBAL EVENT INFORMATION ***/

  //  fAnalysisEvent->Reset(); // Moved up
  /* update global event info */
  fAnalysisEvent->SetIsCollisionCandidate(fIsCollisionCandidate);
  fAnalysisEvent->SetIsEventSelected(fIsEventSelected);
  fAnalysisEvent->SetIsPileupFromSPD(fIsPileupFromSPD);
  fAnalysisEvent->SetHasVertex(fHasVertex);
  fAnalysisEvent->SetVertexZ(fVertexZ);
  fAnalysisEvent->SetMCTimeZero(fMCTimeZero);
  fAnalysisEvent->SetRunNumber(fRunNumber);
  fAnalysisEvent->SetMagneticField(fESDEvent->GetMagneticField());
				
  /* update TOF event info */
  for (Int_t i = 0; i < 10; i++) {
    fAnalysisEvent->SetTimeZeroTOF(i, fESDpid->GetTOFResponse().GetT0bin(i));
    fAnalysisEvent->SetTimeZeroTOFSigma(i, fESDpid->GetTOFResponse().GetT0binRes(i));
  }
  /* update T0 event info */
  for (Int_t i = 0; i < 3; i++)
    fAnalysisEvent->SetTimeZeroT0(i, fESDEvent->GetT0TOF(i));


  Int_t refmulti;
  refmulti = AliESDtrackCuts::GetReferenceMultiplicity(fESDEvent, AliESDtrackCuts::kTrackletsITSTPC,0.8);  
  fAnalysisEvent->SetReferenceMultiplicity(refmulti);
  fAnalysisEvent->SetMCMultiplicity(mcmulti);
  /*** RECONSTRUCTED TRACKS ***/

  /* reset track array */
  fAnalysisTrackArray->Clear();
  // fAnalysisV0TrackArray->Clear();
  /* loop over ESD tracks */
  Int_t nTracks = fESDEvent->GetNumberOfTracks();
  AliESDtrack *track;
  for (Int_t itrk = 0; itrk < nTracks; itrk++) {
    /* get track */
    track = fESDEvent->GetTrack(itrk);
    if (!track) continue;
    /* check accept track */
    Int_t trflag = GetTrackCutsFlag(track);
    if(!trflag) continue;
    
    /* update and add analysis track */
    fAnalysisTrack->Update(track, fMCEvent,fPIDResponse, trflag);
    if(track->IsEMCAL()) {
      AliVCluster *lvcl = fESDEvent->GetCaloCluster(track->GetEMCALcluster());
      if(lvcl)
	fAnalysisTrack->SetEMCalPars(lvcl->E(),track->GetTrackPOnEMCal());
    };
    new ((*fAnalysisTrackArray)[fAnalysisTrackArray->GetEntries()]) AliAnalysisPIDTrack(*fAnalysisTrack);
    //fAnalysisV0Track = (V0Track*)fAnalysisTrack;
    /*fAnalysisV0Track->SetExtraParam(3);
      new ((*fAnalysisV0TrackArray)[fAnalysisV0TrackArray->GetEntries()]) V0Track(*fAnalysisV0Track);*/
    

  } /* end of loop over ESD tracks */
  ProcessV0s();
  fPIDTree->Fill();

  PostData(1,fPIDTree);
  PostData(2,fEvHist);
}

void AliAnalysisTaskTPCTOFPID::Terminate(Option_t *) {
  printf("Terminate!\n");
}

