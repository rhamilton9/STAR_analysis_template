// ------------------------------------------------------------------------
//
// Template Star StMaker class 
// Made by R. Hamilton Jul. 27, 2026 by studing other codes including:
//   * StKFParticleMaker by Yuri Fisyak, Maksym Zyzak, (others?)
//   * https://github.com/joelmazer/star-jetframework/blob/master/StJet.cxx
// No GenAI tools were used in the making of this file!
//
// Changelog: 
//   - Aug. 04, 2026 : Template maker class completed
// 
// ------------------------------------------------------------------------
// 
// To create a new class using this template it is easiest to use
// terminal commands to replace all instances of StTemplateMaker with 
// the desired new class name. For convenience this is implemented in the 
// local bash script in the base directory for the template analysis,
//
// 	../../changeTemplateName.sh
// 
// ------------------------------------------------------------------------


// Header/base class
#include "StTemplateMaker.h"

// Root classes
#include "TDirectory.h"
#include "TNtuple.h"
#include "TFile.h"
#include "TChain.h"
#include "TNtuple.h"
#include "TTree.h"
#include "TSystem.h"

// TMVA classes (ROOT machine learning package)
#include "TMVA/GeneticAlgorithm.h"
#include "TMVA/GeneticFitter.h"
#include "TMVA/IFitterTarget.h"
#include "TMVA/Factory.h"

// PicoDST classes 
#include "StPicoDstMaker/StPicoDstMaker.h"
#include "StPicoEvent/StPicoDst.h"
#include "StPicoEvent/StPicoEvent.h"
#include "StPicoEvent/StPicoTrack.h"
#include "StPicoEvent/StPicoBTofPidTraits.h"

// MuDST classes
#include "StMuDSTMaker/COMMON/StMuDst.h"
#include "StMuDSTMaker/COMMON/StMuTrack.h"

// StRefMult classes
#include "StRefMultCorr/StRefMultCorr.h"
#include "StRefMultCorr/CentralityMaker.h"

// FastJet classes
#include "fastjet/PseudoJet.hh"
#include "fastjet/ClusterSequence.hh"

// StHelperInterface classes
#include "StHelperInterface/StHelperInterface.h"

// One can include local classes from other packages, so long as they are in the StRoot directory
// Any code or external packages not implemented in StRoot (https://github.com/star-bnl/star-sw/tree/main/StRoot)
// should be stored in the local StRoot directory, like this class which lives in [analysis_dir]/StRoot/StTemplateMaker
/*const double PION_MASS = 0.13957039;

const bool ghost_grid_area = false; // Compute A for rho * A subtraction
const double ghost_pt = 1e-100;
const int nGhost_rap = 100;
const int nGhost_phi = 314;8*/

#define TEMP_JET_CLUSTER
const float jet_radius = 0.4;      // Radians
const float jetcut_minpT_full = 5; // GeV


// Declare ROOT class impelentation
// This labels all contents of this file as body for the class.
//
// Must correspond to ClassDef in header class
ClassImp(StTemplateMaker);


//----------------------------------------------------------------------------------------------------- Constructor
StTemplateMaker::StTemplateMaker(const char *name) : StMaker(name), // Inheritance
// *--- Default initializations for class variables, defined in the header file StTemplateMaker.h
//      These variables can be changed only by interacting with class setters, defined in the header file.
//      Defining these variables here declares them available as external objects to other methods in this file
//      that live outside the class body
	fIsPicoAnalysis(true),                        // Flag for input data format (PicoDST, MuDST) 
	fCollectTrackHistograms(true),                // Switch for collecting track data histograms
	fCollectPIDHistograms(true),                  // Switch for collecting PID data histograms
	fCollectJetHistograms(true),                  // Switch for collecting Jet data histograms
	fCollectPVHistograms(true),                   // Switch for collecting primary vertex histograms
	fWriteDataTree(true),                         // Switch for collecting data to TTree
	fOutputFileName("output.root"),               // Name of output file where histograms are written
	fOutputFile(nullptr) {                        // TFile pointer to output file
  
  // Write zeros to the location of this object in memory,
  // so that it is not initialized with random data that could cause 
  // issues if random data were accessed on accident. 
  // In principle this is in std namespace but current rcf has GNU implementation instead
  memset(memory_zero_begin, 0, memory_zero_end - memory_zero_begin + 1);
  
  std::cout << "attempting to initialize PID interface..." << std::endl;

  // Add any initializers that should be run on object creation (i.e. as a constructor)
  fPIDInterface = new StPIDInterface();
  fJetInterface = new StJetInterface();
  
  // Set remaining pointers as NULL
  fOutputFile = NULL;
}// End StTemplateMaker::Constructor

//----------------------------------------------------------------------------------------------------- Deconstructor

// SafeDelete() all pointers initialized with "new" keyword
// because the memory is not erased outside of the relevant scope
//
// SafeDeleta() checks that the given pointer is not NULL before deleting it.
StTemplateMaker::~StTemplateMaker() {
  // Custom classes
  SafeDelete(fPIDInterface);
  SafeDelete(fJetInterface);

  // Files
  SafeDelete(fOutputFile);
  
  // ROOT objects
  SafeDelete(fSampleHist);
  SafeDelete(fSampleTree);
}// End of StTemplateMaker::~StTemplateMaker



//----------------------------------------------------------------------------------------------------- Init()

// This method is called by StRoot when BFChain is created.
// This is where output files/TObjects (hists, trees) should be initialized
Int_t StTemplateMaker::Init() {
  
  // restore to gFile after booking the plots here
  TFile* curFile = gFile;
  TDirectory* curDirectory = gDirectory;
  
  fOutputFile = new TFile(fOutputFileName, "RECREATE"); 
  
  // Set up quality control histograms
  fOutputFile->cd();
  if (fCollectPVHistograms)     BookVertexHistograms();
  if (fCollectTrackHistograms)  BookTrackHistograms();
  if (fCollectPIDHistograms)    BookPIDHistograms();
  if (fCollectJetHistograms)    BookJetHistograms();
  

  // Set up TTree for run/event information
  // Can be easily modified to include other desired information
  if (fWriteDataTree) { 
    // TTrees must be created after the TFile to store them in the correct directory.
    fSampleTree = new TTree("sample_tree","sample_tree");
    // Overall run/event info (may be repetitions if events have multiple D0 candidates)
    //                 TREE BRANCH NAME           LOCAL ADDRESS           TREE BRANCH TYPE
    fSampleTree->Branch("bRun_id",                &bRun_id,               "bRun_id/I");	                // Run : ID
    fSampleTree->Branch("bEvent_id",              &bEvent_id,             "bEvent_id/I");               // Event : ID
    fSampleTree->Branch("bEvent_Vz",              &bEvent_Vz,             "bEvent_Vz/F");               // Event : vertex z position
    fSampleTree->Branch("bEvent_vertexrank",      &bEvent_vertexrank,     "bEvent_vertexrank/I");       // Event : vertex ranking (more positive is a better PV)
    fSampleTree->Branch("bEvent_refmult",         &bEvent_refmult,        "bEvent_refmult/I");          // Event : Reference Multiplicity in eta [-0.5,0.5]
    fSampleTree->Branch("bEvent_tofmult",         &bEvent_tofmult,        "bEvent_tofmult/I");          // Event : ToF Multiplicity
    std::cout << "StTemplateMaker:INFO  - Data TTree initialized." << std::endl;

#ifdef TEMP_JET_CLUSTER
    fSampleTree->Branch("bJet_nconstituents",     &bJet_nconstituents,    "bJet_nconstituents/I");      // Jet : Number of constituents
    fSampleTree->Branch("bJet_pt",                &bJet_pt,               "bJet_pt/F");                 // Jet : Transverse momentum
    fSampleTree->Branch("bJet_phi",               &bJet_phi,              "bJet_phi/F");                // Jet : Azimuthal angle
    fSampleTree->Branch("bJet_eta",               &bJet_eta,              "bJet_eta/F");                // Jet : Pseudorapidity
    fSampleTree->Branch("bJet_area",              &bJet_area,             "bJet_area/F");               // Jet : Area in phi/eta solid angle space
#endif
  }
  
  // Initialize TObjects that should always be initialized
  fSampleHist = new TH1D("sample_histogram","Sample Hist;x;Counts;",50,-3.0,3.0);
  
  std::cout << "StTemplateMaker:INFO  - Data Histograms initialized." << std::endl;

  
  
  std::cout << "StTemplateMaker:INFO  - Initialization successful. " << endl;
  std::cout << "==============================End of StTemplateMaker::Init()=============================" << std::endl;

  // Pass global file pointers back to what they were before
  gFile = curFile;
  gDirectory = curDirectory;
  return kStOK;
}// End of StTemplateMaker::Init



// Initializer for an individual run
Int_t StTemplateMaker::InitRun(Int_t runumber) {
  // Just pass onto StMaker, 
  // could in principle add other functionality
  return StMaker::InitRun(runumber);
}// End StTemplateMaker::InitRun



// Plot bookers
void StTemplateMaker::BookPIDHistograms() { fPIDInterface->BookPIDHistograms();   return;}
void StTemplateMaker::BookJetHistograms() { fJetInterface->BookJetHistograms();   return;}



// Create and store histograms for Primary Vertex reconstruction/quality
void StTemplateMaker::BookVertexHistograms() {
  std::cout << "StTemplateMaker:INFO  - Booking PV plots in subdirectory \033[31mPrimaryVertexPlots\033[39m of output TFile. " << endl;
  
  // Set up TDirectory within the output file
  TDirectory *dirs[2] = {0};
  dirs[0] = TDirectory::CurrentDirectory(); assert(dirs[0]);
  dirs[0]->cd();
  
  // Make subdirectory "PrimaryVertexPlots" within the parent directory
  if ( !dirs[0]->GetDirectory("PrimaryVertexPlots") ) dirs[0]->mkdir("PrimaryVertexPlots");
  dirs[1] = dirs[0]->GetDirectory("PrimaryVertexPlots"); assert(dirs[1]);
  dirs[1]->cd();
  PrintMem(dirs[1]->GetPath());
  
  // Set up vertex plots in this directory
  
  // End of vertex plot construction
  


  // Report on total used memory in parent directory
  dirs[0]->cd();
  PrintMem(dirs[1]->GetPath());
}// End of StTemplateMaker::BookVertexHistograms



// Create and store histograms for reconstructed track information
void StTemplateMaker::BookTrackHistograms() {
  std::cout << "StTemplateMaker:INFO  - Booking Track info plots in subdirectory \033[31mTrackInfoPlots\033[39m of output TFile. " << endl;
  
  // Set up TDirectory within the output file
  TDirectory *dirs[2] = {0};
  dirs[0] = TDirectory::CurrentDirectory(); assert(dirs[0]);
  dirs[0]->cd();

  // Make subdirectory "TrackInfoPlots" within the parent directory
  if ( !dirs[0]->GetDirectory("TrackInfoPlots") ) dirs[0]->mkdir("TrackInfoPlots");
  dirs[1] = dirs[0]->GetDirectory("TrackInfoPlots"); assert(dirs[1]);
  dirs[1]->cd();
  PrintMem(dirs[1]->GetPath());
  
  // Set up track plots in this directory
  
  // End of track plot construction
  
  
  
  // Report on total used memory in parent directory
  dirs[0]->cd();
  PrintMem(dirs[1]->GetPath());
}// End of StTemplateMaker::BookTrackHistograms






// Print information about memory usage
void StTemplateMaker::PrintMem(const Char_t *opt) {
  // Get ROOT memory structure
  MemInfo_t info;
  gSystem->GetMemInfo(&info);

  // Report on used memory
  std::cout << opt 
            << "\tMemory : Total = " << info.fMemTotal 
            << "\tUsed = " << info.fMemUsed
            << "\tFree = " << info.fMemFree
            << "\tSwap Total = " << info.fSwapTotal
            << "\tUsed = " << info.fSwapUsed
            << "\tFree = " << info.fSwapFree << std::endl;
}// End StTemplateMaker::PrintMem



//----------------------------------------------------------------------------------------------------- Make() 

// The main analysis loop
// This runs for every event
Int_t StTemplateMaker::Make() {  
  // Set up pointers to current event in the DSTs
  if (fIsPicoAnalysis) {// PicoDST
    fPicoDst = StPicoDst::instance();
    if (!fPicoDst) return kStOK;
  } else {// MuDST 
    fMuDst = StMuDst::instance();
    if (!fMuDst) return kStOK;
  }// End of pointer set

#ifndef TEMP_JET_CLUSTER
  //================================================================================= Add analysis functionality below
  if (fSampleHist->GetEntries() == 0) std::cout << "Hello world!" << std::endl;
  fSampleHist->FillRandom("gaus", 1);
  
  if (fWriteDataTree) {
    // Get relevant parameters from MuDST/PicoDST
    if (fIsPicoAnalysis) {// PicoDST
      StPicoEvent* pico_event = fPicoDst->event();

      bRun_id = pico_event->runId();
      bEvent_id = pico_event->eventId();
      bEvent_Vz = pico_event->vzVpd();
      bEvent_vertexrank = pico_event->ranking(); // PV rank, for pileup identification
      bEvent_refmult = pico_event->refMult();
      bEvent_tofmult = pico_event->btofTrayMultiplicity();
    }// End of PicoDST

    else { // MuDST
      StMuEvent* mu_event = fMuDst->event();
      
      // Loop over all candidate primary vertices
      for(uint32_t i_PV = 0; i_PV < fMuDst->numberOfPrimaryVertices(); i_PV++) {
        StMuPrimaryVertex* vertex = fMuDst->primaryVertex(i_PV);
        if(!vertex) continue;
        
        bRun_id = mu_event->runId();
        bEvent_id = mu_event->eventId();
        bEvent_Vz = mu_event->vpdVz();
        bEvent_vertexrank = vertex->ranking(); // PV rank, for pileup identification
        bEvent_refmult = mu_event->refMult(i_PV);
        bEvent_tofmult = mu_event->btofTrayMultiplicity();
      }// End of PV loop
    }// End of MuDST
    
    
    fSampleTree->Fill();
  }// End of tree write

  //================================================================================= Add analysis functionality above
#else 
 
  // Setup jet definiton for fastjet
  // Using algorithm and jet radius according to config file
  // Recombination Scheme options: See
  //   https://fastjet.fr/repo/doxygen-3.5.1/namespacefastjet.html#a46fcc48dcb00a10557d773e328153bcb
  fastjet::JetAlgorithm  jet_algo         = fastjet::antikt_algorithm;
  fastjet::JetDefinition jet_definition   = fastjet::JetDefinition(jet_algo, jet_radius, fastjet::E_scheme);
  
  // Alternate --fJetInterface
  fJetInterface->SetJetMinPt(5);

  // Add tracks to stable particle vector
  std::vector<fastjet::PseudoJet> stable_particles;
  if (fIsPicoAnalysis) {// PicoDST
    StPicoEvent* pico_event = fPicoDst->event();
    TVector3 pos_PV = pico_event->primaryVertex();
    
    // Branch elements for TTree
    bRun_id = pico_event->runId();
    bEvent_id = pico_event->eventId();
    bEvent_Vx = pos_PV[0];
    bEvent_Vy = pos_PV[1];
    bEvent_Vz = pos_PV[2];
    bEvent_vertexrank = pico_event->ranking(); // PV rank, for pileup identification
    bEvent_refmult = pico_event->refMult();
    bEvent_tofmult = pico_event->btofTrayMultiplicity();
    
    // Perform some vertex quality cuts
    if (bEvent_Vz <= -35.0 || bEvent_Vz >= 25.0 ||
        TMath::Hypot(pos_PV[0], pos_PV[1]) >= 2.0) return kStOk;
    

    uint32_t nTracks_global = pico_event->numberOfGlobalTracks();
    //uint32_t nTracks_primary = fPicoDst->numberOfPrimaryTracks();
    
    for (uint32_t i_track = 0; i_track < nTracks_global; i_track++) {
      StPicoTrack *pTrack = fPicoDst->track(i_track);
      
      // Track quality cuts :: 
      if (!pTrack->isPrimary() ||                                                // Primary tracks
          pTrack->pPt() <= 0.2 ||                                                // Track pT must be at least 0.2 GeV
          pTrack->pPt() >= 30. ||                                                // Track pT must be at most 30 GeV
	  std::abs(pTrack->nHitsFit()) <= 15 ||                                  // Must have 15 hits in TPC or other trackers
	  std::abs(static_cast<float>(pTrack->nHitsFit())/
	                              pTrack->nHitsMax()) <= 0.52 ||             // Must have 52% of possible hits
          pTrack->gDCA(pos_PV[0], pos_PV[1], pos_PV[2]) <= 1.0)                  // DCA to PV less than 1cm
        continue;
      
      fastjet::PseudoJet jTrack(pTrack->pMom()[0], 
                                pTrack->pMom()[1],
				pTrack->pMom()[2],
				TMath::Hypot(PION_MASS, pTrack->pPtot()) );
      
      // Check that the pseudorapidity of the track is in bounds
      if (std::fabs(jTrack.pseudorapidity()) >= 1.0) continue;
      
      // Add to FastJet vector
      stable_particles.push_back(jTrack);

    }// End of PicoDst event track loop

    if (ghost_grid_area) {
      fastjet::PseudoJet ghost;
      for (int ir = 0; ir <= nGhost_rap; ++ir) {
	// For massless particles like ghosts, rapidity = pseudorapidity
        double longitude = (ir + 0.5) * (2.0 / nGhost_rap) - 1.0;
        for (int iphi = 1; iphi <= nGhost_phi; ++iphi) {
          double phi = (iphi + 0.5) * (TMath::TwoPi() / nGhost_phi) - TMath::Pi();
          ghost.reset_momentum_PtYPhiM(ghost_pt, longitude, phi, 0);
          stable_particles.push_back(ghost);
        }
      }// End of ghost loop
    }// End of ghost append

    // Perform the clustering
    fastjet::ClusterSequence clustering(stable_particles, jet_definition);
    std::vector<fastjet::PseudoJet> final_jets = sorted_by_pt(clustering.inclusive_jets(jetcut_minpT_full));

    for (fastjet::PseudoJet jet : final_jets) {
      if (std::fabs(jet.eta()) >= 1.0 - jet_radius) continue; 
      
      // Compute area and tabulate constituents, removing ghosts
      bJet_nconstituents = 0;
      bJet_area = 0;
      double dA = (2.0 / nGhost_rap) * (TMath::TwoPi() / nGhost_phi);
      for (fastjet::PseudoJet c : jet.constituents()) {
        // Increment area for ghost
	if (c.pt() <= 1e-50) {bJet_area += dA; continue;}
        
	// Otherwise, a real track
	++bJet_nconstituents;
      }// End of constituent loop

      // Remaining variables
      bJet_pt = jet.pt();
      bJet_phi = jet.phi_std();
      bJet_eta = jet.pseudorapidity();

      // Fill the tree
      fSampleTree->Fill();

    }// End of jet loop
    // concurrently pass to jet interface to test
    fJetInterface->ClusterPicoJets(fPicoDst);
  } else { // MuDST
    for(uint32_t i_PV = 0; i_PV < fMuDst->numberOfPrimaryVertices(); i_PV++) {
      
    }
  }
  
  

#endif
  return kStOK;
}// End of StTemplateMaker::Make()



//----------------------------------------------------------------------------------------------------- Finish()

// This macro is called by BFChain when the analysis is finished
// This is where TObjects should be written and files closed
Int_t StTemplateMaker::Finish() {
  // change to output file main directory
  fOutputFile->cd();
  
  // Write objects from helper interface
  if (fCollectJetHistograms) fJetInterface->WriteJetHistograms();
  
  // Write any objects defined in the analysis scope
  fSampleHist->Write();

  if(fWriteDataTree) fSampleTree->Write();
  
  // Close the file and return
  fOutputFile->Close();
  return kStOK;
}// End of StTemplateMaker::Finish



//----------------------------------------------------------------------------------------------------- Class Helper Methods
// Note that the methods here should be definitions of the virtual/forward declared methods
// which are included in the class definition StTemplateMaker.h. They can be public or
// private methods; Public methods are accessible to other files while private ones 
// can be passed only between methods internal to the class (i.e. other methods in this file)

// A sample helper method
// Note that the method should be declared virtually in the
// header file, where the class implementation is defined!
// 
// This method is "private"
void StTemplateMaker::TemplateHelper(/* inputs to the helper function */) {
  // Insert functionality here
  return;
}// End of StTemplateMaker::TemplateHelper



//----------------------------------------------------------------------------------------------------- End of StTemplateMaker.cxx 
