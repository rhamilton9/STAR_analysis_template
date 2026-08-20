// ------------------------------------------------------------------------
//
// Simple STAR Jet Interfacer class 
// Made by R. Hamilton Aug. 19, 2026
// No GenAI tools were used in the making of this file!
//
// Changelog: 
// 
// ------------------------------------------------------------------------


// Header/base class
#include "StJetInterface.h"

// Root classes
#include "TClass.h"
#include "TDirectory.h"
#include "TNtuple.h"
#include "TFile.h"
#include "TChain.h"
#include "TNtuple.h"
#include "TTree.h"
#include "TSystem.h"
#include "TH1D.h"

// FastJet classes
#include "fastjet/JetDefinition.hh"
#include "fastjet/PseudoJet.hh"
#include "fastjet/ClusterSequence.hh"

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

// One can include local classes from other packages, so long as they are in the StRoot directory
// Any code or external packages not implemented in StRoot (https://github.com/star-bnl/star-sw/tree/main/StRoot)
// should be stored in the local StRoot directory, like this class which lives in [analysis_dir]/StRoot/StJetInterface





// Declare ROOT class impelentation
// This labels all contents of this file as body for the class.
//
// Must correspond to ClassDef in header class
ClassImp(StJetInterface);

// Global pointer to current StJetInterface
StJetInterface* gStJetInterface;

// Static declarations of jet algo/definition to 
// avoid redefinition on every event
fastjet::JetAlgorithm gJetAlgorithm;
fastjet::RecombinationScheme gJetRecombinationScheme;
fastjet::JetDefinition gJetDefinition;

//----------------------------------------------------------------------------------------------------- Constructor
StJetInterface::StJetInterface() :
// *--- Default initializations for class variables, defined in the header file StJetInterface.h
//      These variables can be changed only by interacting with class setters, defined in the header file.
//      Defining these variables here declares them available as external objects to other methods in this file
//      that live outside the class body
        fJetRadius(0.4),                              // Radius of jets in eta-phi space
        fJetMinPt(0),                                 // Minimum pT of jet to cluster
	fCollectJetHistograms(true),                  // Switch for collecting Jet data histograms
	fOutputFile(nullptr) {                        // TFile pointer to output file
  // Set global pointer to &this on construction
  gStJetInterface = this;
  
  std::cout << "StJetInterface:INFO  - Initializing Jet helper for interfacing FastJet." << endl;

  // Write zeros to the location of this object in memory,
  // so that it is not initialized with random data that could cause 
  // issues if random data were accessed on accident. 
  // In principle this is in std namespace but current rcf has GNU implementation instead
  memset(memory_zero_begin, 0, memory_zero_end - memory_zero_begin + 1);
  

  // Add any initializers that should be run on object creation (i.e. as a constructor)
  fOutputFile = gFile;

  // Initialize jet algorithm, definition
  gJetAlgorithm = fastjet::antikt_algorithm;
  gJetRecombinationScheme = fastjet::E_scheme;
  gJetDefinition = fastjet::JetDefinition(gJetAlgorithm, fJetRadius, gJetRecombinationScheme);
  std::cout << "StJetInterface:INFO  - Starting FastJet with JetDefinition :: ";
  PrintJetDefinition();
}// End StJetInterface::Constructor



//----------------------------------------------------------------------------------------------------- Deconstructor



// SafeDelete() all pointers initialized with "new" keyword
// because the memory is not erased outside of the relevant scope
//
// SafeDeleta() checks that the given pointer is not NULL before deleting it.
StJetInterface::~StJetInterface() {
  SafeDelete(fOutputFile);

}// End of StJetInterface::~StJetInterface



//----------------------------------------------------------------------------------------------------- Plot Setup

// Create and store histograms for Primary Vertex reconstruction/quality
void StJetInterface::BookJetHistograms() {
  std::cout << "StJetInterface:INFO  - Booking Particle Identification plots in subdirectory \033[31mJetInfoPlots\033[39m of output TFile. " << endl;
  
  // Set up TDirectory within the output file
  TDirectory *dirs[2] = {0};
  dirs[0] = TDirectory::CurrentDirectory(); 
  assert(dirs[0]);
  dirs[0]->cd();
  
  // Make subdirectory "JetInfoPlots" within the parent directory
  if ( !dirs[0]->GetDirectory("JetInfoPlots") ) dirs[0]->mkdir("JetInfoPlots");
  dirs[1] = dirs[0]->GetDirectory("JetInfoPlots"); 
  assert(dirs[1]);
  dirs[1]->cd();
  
  std::cout << "In the Jet helper class!" << std::endl;

  // Set up Jet plots in this directory
  
  // Jet pT
  fJetHistograms[0] = new TH1D("hChargedJetConstituents",";Constituent Multiplicity;Charged jet count",
                               20, 0, 20);
  fJetHistograms[1] = new TH1D("hChargedJetPt",";#it{p}_{T} [GeV/c];Charged jet count",
                               100, 0, 100); 
  fJetHistograms[2] = new TH1D("hChargedJetPhi",";#phi [rad];Charged jet count",
                               100, -TMath::Pi(), TMath::Pi());
  fJetHistograms[3] = new TH1D("hChargedJetEta",";#eta [rad];Charged jet count",
                               100, -1, 1);
  fJetHistograms[4] = new TH1D("hChargedJetArea",";Jet Area [rad^{2}];Charged jet count",
                               100, 0, TMath::Pi());

  // End of Jet plot construction
  


  // Return to parent directory
  dirs[0]->cd();
}// End of StJetInterface::BookJetHistograms



// Write jet plots to output TDirectory
void StJetInterface::WriteJetHistograms() {
  TDirectory* holddir = TDirectory::CurrentDirectory();
  
  TDirectory* outdir = holddir->GetDirectory("JetInfoPlots"); 
  assert(outdir);
  outdir->cd();

  for (int i = 0; i <= 4; ++i) {
    fJetHistograms[i]->Write(fJetHistograms[i]->GetName(), TObject::kOverwrite);
  }

  holddir->cd();
}


//----------------------------------------------------------------------------------------------------- Setters, Getters and FastJet Interfacing



// Define the algorithm for jet clustering
// For more details on the options, see
//   https://fastjet.fr/repo/fastjet-doc-3.5.1.pdf
//   https://fastjet.fr/repo/doxygen-3.5.1/namespacefastjet.html#a46fcc48dcb00a10557d773e328153bcb
void StJetInterface::SetJetAlgorithm(TString algo) {
  algo.ToLower(); 
  if (algo.Contains("anti") && algo.Contains("kt"))     
    gJetAlgorithm = fastjet::antikt_algorithm;
  else if (algo.Contains("gen") && algo.Contains("kt"))
    gJetAlgorithm = fastjet::genkt_algorithm;
  else if (algo.Contains("ee") && algo.Contains("gen")) 
    gJetAlgorithm = fastjet::ee_genkt_algorithm;
  else if (algo.Contains("ee") && algo.Contains("kt")) 
    gJetAlgorithm = fastjet::ee_kt_algorithm;
  else if (algo.Contains("kt"))
    gJetAlgorithm = fastjet::kt_algorithm;
  else if (algo.Contains("cambridge") || algo.Contains("aachen") || algo.Contains("ca"))
    gJetAlgorithm = fastjet::cambridge_algorithm;
  else if (algo.Contains("plugin"))    
    gJetAlgorithm = fastjet::plugin_algorithm;
  else {
    std::cout << "\033[31mWarning\033[39m:: Unrecognized jet algorithm " << algo << "! Check inputs." << std::endl;
    return;
  } 
  
  // Re-initialize jet algorithm, definition
  gJetDefinition = fastjet::JetDefinition(gJetAlgorithm, fJetRadius, gJetRecombinationScheme);
  std::cout << "StJetInterface:INFO  - Modified jet algorithm, current JetDefinition :: ";
  PrintJetDefinition();
  
  return;
}// End of StJetInterface::SetJetAlgorithm



// Set the jet radius, in eta-phi space
void StJetInterface::SetJetRadius(double radius) {
  fJetRadius = radius;
  
  // Re-initialize jet algorithm, definition
  gJetDefinition = fastjet::JetDefinition(gJetAlgorithm, fJetRadius, gJetRecombinationScheme);
  std::cout << "StJetInterface:INFO  - Modified jet radius, current JetDefinition :: ";
  PrintJetDefinition();

  return;
}// End of StJetInterface::SetJetRadius



// Set the recombination scheme, the variable 
// through which particles are merged in clustering
void StJetInterface::SetRecombinationScheme(TString scheme) {
  scheme.ToLower();
  if (scheme.Contains("external"))
    gJetRecombinationScheme = fastjet::external_scheme; 
  else if (scheme.Contains("bipt2"))
    gJetRecombinationScheme = fastjet::BIpt2_scheme; 
  else if (scheme.Contains("bipt"))
    gJetRecombinationScheme = fastjet::BIpt_scheme; 
  else if (scheme.Contains("pt2"))
    gJetRecombinationScheme = fastjet::pt2_scheme; 
  else if (scheme.Contains("pt"))
    gJetRecombinationScheme = fastjet::pt_scheme; 
  else if (scheme.Contains("et2"))
    gJetRecombinationScheme = fastjet::Et2_scheme; 
  else if (scheme.Contains("et"))
    gJetRecombinationScheme = fastjet::Et_scheme; 
  else if (scheme.Contains("e"))
    gJetRecombinationScheme = fastjet::E_scheme; 
  else {
    std::cout << "\033[31mWarning\033[39m:: Unrecognized recombination scheme " << scheme << "! Check inputs." << std::endl;
    return;
  }// End of setting recombination scheme

  // Re-initialize jet algorithm, definition
  gJetDefinition = fastjet::JetDefinition(gJetAlgorithm, fJetRadius, gJetRecombinationScheme);
  std::cout << "StJetInterface:INFO  - Modified jet radius, current JetDefinition :: ";
  PrintJetDefinition();
  
  return;
}// End of StJetInterface::SetRecombinationScheme



// Print the current jet definition in legible terms
void StJetInterface::PrintJetDefinition() {
  std::cout << "{ ";
  switch (gJetDefinition.jet_algorithm()) {
    case (fastjet::antikt_algorithm):      std::cout << "anti-kt";     break;
    case (fastjet::kt_algorithm):          std::cout << "kt";          break;
    case (fastjet::genkt_algorithm):       std::cout << "gen-kt";      break;
    case (fastjet::cambridge_algorithm):   std::cout << "Camb/Aach";   break;
    case (fastjet::ee_kt_algorithm):       std::cout << "e+e- kt";     break;
    case (fastjet::ee_genkt_algorithm):    std::cout << "e+e- gen-kt"; break;
    case (fastjet::plugin_algorithm):      std::cout << "plugin";      break;
    case (fastjet::cambridge_for_passive_algorithm):   std::cout << "Camb/Aach (passive)";   break;
    case (fastjet::genkt_for_passive_algorithm ):      std::cout << "gen-kt (passive)";      break;
    default:                                           std::cout << "undefined";             break;
  }// End of jet algorithm
  
  std::cout << ", R = " << fJetRadius << ", ";

  switch (gJetDefinition.recombination_scheme()) {
    case (fastjet::E_scheme):              std::cout << "E-scheme";         break;
    case (fastjet::pt_scheme):             std::cout << "pt-scheme";        break;
    case (fastjet::pt2_scheme):            std::cout << "pt2-scheme";       break;
    case (fastjet::Et_scheme):             std::cout << "Et-scheme";        break;
    case (fastjet::Et2_scheme):            std::cout << "Et2-scheme";       break;
    case (fastjet::BIpt_scheme):           std::cout << "BIpt-scheme";      break;
    case (fastjet::BIpt2_scheme):          std::cout << "BIpt2-scheme";     break;
    case (fastjet::external_scheme):       std::cout << "external-scheme";  break;
    default:                               std::cout << "unknown scheme";   break;
  }// End of recombination scheme
  std::cout << " }" << std::endl;
}// End of StJetInterface::PrintJetDefinition



//----------------------------------------------------------------------------------------------------- Jet Clustering



// Perform the clustering on a PicoDST event
int StJetInterface::ClusterPicoJets(StPicoDst* picoDst) {
  // Add tracks to stable particle vector
  std::vector<fastjet::PseudoJet> stable_particles;
  StPicoEvent* pico_event = picoDst->event();
  TVector3 pos_PV = pico_event->primaryVertex();
  
  

  uint32_t nTracks_global = pico_event->numberOfGlobalTracks();
  //uint32_t nTracks_primary = picoDst->numberOfPrimaryTracks();
  
  for (uint32_t i_track = 0; i_track < nTracks_global; i_track++) {
    StPicoTrack *pTrack = picoDst->track(i_track);
    
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
  
  /*
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
  */


  // Perform the clustering
  fastjet::ClusterSequence clustering(stable_particles, gJetDefinition);
  std::vector<fastjet::PseudoJet> final_jets = fastjet::sorted_by_pt(clustering.inclusive_jets(fJetMinPt));

  for (fastjet::PseudoJet jet : final_jets) {
    if (std::fabs(jet.eta()) >= 1.0 - fJetRadius) continue; 
    
    // Compute area and tabulate constituents, removing ghosts
    int jet_nconstituents = 0;
    int jet_area = 0;
    double dA = (2.0 / nGhost_rap) * (TMath::TwoPi() / nGhost_phi);
    for (fastjet::PseudoJet c : jet.constituents()) {
      // Increment area for ghost
      if (c.pt() <= 1e-50) {jet_area += dA; continue;}
      
      // Otherwise, a real track
      ++jet_nconstituents;
    }// End of constituent loop

    // populate histograms
    if (fCollectJetHistograms) {
      fJetHistograms[0]->Fill(jet_nconstituents);
      fJetHistograms[1]->Fill(jet.pt());
      fJetHistograms[2]->Fill(jet.phi_std());
      fJetHistograms[3]->Fill(jet.pseudorapidity());
    }// End of hist fill
  }// End of jet loop
  
  
  return 1;
}// End of StJetInterface::ClusterPicoJets

//----------------------------------------------------------------------------------------------------- Class Helper Methods
// Note that the methods here should be definitions of the virtual/forward declared methods
// which are included in the class definition StJetInterface.h. They can be public or
// private methods; Public methods are accessible to other files while private ones 
// can be passed only between methods internal to the class (i.e. other methods in this file)


// A sample helper method
// Note that the method should be declared virtually in the
// header file, where the class implementation is defined!
// 
// This method is "private"
void StJetInterface::TemplateHelper(/* inputs to the helper function */) {
  // Insert functionality here
  return;
}// End of StJetInterface::TemplateHelper



//----------------------------------------------------------------------------------------------------- End of StJetInterface.cxx 
