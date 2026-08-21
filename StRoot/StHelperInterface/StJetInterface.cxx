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
#include "TH2D.h"

// FastJet classes
#include "fastjet/JetDefinition.hh"
#include "fastjet/PseudoJet.hh"
// Area and Clustering
#include "fastjet/ClusterSequence.hh"
#include "fastjet/ClusterSequenceArea.hh"
#include "fastjet/ClusterSequence1GhostPassiveArea.hh"
#include "fastjet/ClusterSequenceActiveArea.hh"
#include "fastjet/ClusterSequenceActiveAreaExplicitGhosts.hh"
#include "fastjet/ClusterSequencePassiveArea.hh"
#include "fastjet/GhostedAreaSpec.hh"
#include "fastjet/AreaDefinition.hh"
// Background estimation
#include "fastjet/Selector.hh"
#include "fastjet/tools/BackgroundEstimatorBase.hh"
#include "fastjet/tools/JetMedianBackgroundEstimator.hh"
#include "fastjet/tools/Subtractor.hh"

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
fastjet::JetAlgorithm            gJetAlgorithm;
fastjet::RecombinationScheme     gJetRecombinationScheme;
fastjet::JetDefinition           gJetDefinition;
fastjet::AreaType                gJetAreaType;

// For background/Rho estimation
fastjet::JetDefinition           gBgJetDefinition;

//----------------------------------------------------------------------------------------------------- Constructor
StJetInterface::StJetInterface() :
// *--- Default initializations for class variables, defined in the header file StJetInterface.h
//      These variables can be changed only by interacting with class setters, defined in the header file.
//      Defining these variables here declares them available as external objects to other methods in this file
//      that live outside the class body
        fMaxRapidity(1.0),                            // Maximum (pseudo)rapidity range to accept tracks
	fJetRadius(0.4),                              // Radius of jets in eta-phi space
        fJetMinPt(0),                                 // Minimum pT of jet to cluster
	fGhostArea(0.01),                             // Area of ghosts in area grid
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

  // Initialize jet algorithm, definition, area definition
  gJetAlgorithm = fastjet::antikt_algorithm;
  gJetRecombinationScheme = fastjet::E_scheme;
  gJetDefinition = fastjet::JetDefinition(gJetAlgorithm, fJetRadius, gJetRecombinationScheme);
 
  gJetAreaType = fastjet::active_area_explicit_ghosts;

  // Initialize background estimator 
  gBgJetDefinition = fastjet::JetDefinition(fastjet::kt_algorithm, 0.4, fastjet::E_scheme);

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
  
  // Set up Jet plots in this directory
  // 1D Hist
  fBgInfoHistograms[0]      = new TH1D("hBackgroundRho",";BG Estimate #rho [GeV/c];Event count",                100, 0, 100);
  fBgInfoHistograms[1]      = new TH1D("hBackgroundSigma",";BG Estimate #sigma [GeV/c];Event count",            25, 0, 50);
  fCJetNConstituentsHist    = new TH1D("hChargedJetConstituents",";Constituent Multiplicity;Charged jet count", 20, 0, 20);
  fCJetPtHist[0]            = new TH1D("hChargedJetPtRaw",";Raw #it{p}_{T} [GeV/c];Charged jet count",          150, 0, 150); 
  fCJetPtHist[1]            = new TH1D("hChargedJetPt",";#it{p}_{T} [GeV/c];Charged jet count",                 100, 0, 100); 
  fCJetArea                 = new TH1D("hChargedJetArea",";Jet Area [rad^{2}];Charged jet count",               100, 0, TMath::Pi());
  
  // 2D hist
  fCJetRapPhiHist           = new TH2D("hChargedJetEtaPhi",";#eta;#phi;Charged jet count",                      120,-1.1,1.1, 100, -TMath::Pi(), TMath::Pi());
  
  // End of Jet plot construction
  


  // Return to parent directory
  dirs[0]->cd();
  return;
}// End of StJetInterface::BookJetHistograms



// Write jet plots to output TDirectory
void StJetInterface::WriteJetHistograms() {
  TDirectory* holddir = TDirectory::CurrentDirectory();
  
  // Check that the directory exists
  //if ( !holddir->GetDirectory("JetInfoPlots") ) holddir->mkdir("JetInfoPlots");
  TDirectory* outdir = holddir->GetDirectory("JetInfoPlots"); 
  assert(outdir);
  outdir->cd();

  // Write TObjects to JetInfoPlots directory
  for (int i = 0; i < 2; ++i) {
    fBgInfoHistograms[i]->Write();
    fCJetPtHist[i]->Write();
  }
  
  fCJetNConstituentsHist->Write();
  fCJetRapPhiHist->Write();
  fCJetArea->Write();
  // return to base directory
  
  
  holddir->cd();
  std::cout << "Test debug, in StJetInterface::WriteJetHistograms." << std::endl;
  return;
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



// Set the jet area type
//     https://fastjet.fr/repo/fastjet-doc-3.5.1.pdf
//     https://fastjet.fr/repo/doxygen-3.5.1/namespacefastjet.html#a89eb2ee22a1eaa58768a38be0bf77b45
void StJetInterface::SetJetAreaType(TString area) {
  area.ToLower();
  if (area.Contains("active") && area.Contains("explicit")) 
    gJetAreaType = fastjet::active_area_explicit_ghosts;
  else if (area.Contains("active")) 
    gJetAreaType = fastjet::active_area; 
  else if (area.Contains("passive") && area.Contains("one")) 
    gJetAreaType = fastjet::one_ghost_passive_area; 
  else if (area.Contains("passive")) 
    gJetAreaType = fastjet::passive_area;
  else {
    std::cout << "\033[31mWarning\033[39m:: Unrecognized jet area type " << area << "! Check inputs." << std::endl;
    return;
  }// End of setting area type

  // Re-initialize jet area
  std::cout << "StJetInterface:INFO  - Modified jet area type, current JetDefinition :: ";
  PrintJetDefinition(); 

  return;
}// End of StJetInterface::SetJetAreaType



// Set the rapidity maximum acceptance of the detector
// This requires a modification of the AreaDefinition
//    https://fastjet.fr/repo/doxygen-3.5.1/classfastjet_1_1GhostedAreaSpec.html#adbb378c9a5916085df61c23b65ab68df
void StJetInterface::SetMaxRapidity(double max_rap) {
  fMaxRapidity = max_rap;
  
  std::cout << "StJetInterface:INFO  - Modified max rapidity, current JetDefinition :: ";
  PrintJetDefinition(); 
  
  return;
}// End of StJetInterface::SetMaxRapidity



// Set the requested eta-phi area occupied by ghosts
// This requires a modification of the AreaDefinition
//    https://fastjet.fr/repo/doxygen-3.5.1/classfastjet_1_1GhostedAreaSpec.html#adbb378c9a5916085df61c23b65ab68df
void StJetInterface::SetGhostArea(double ghost_area) {
  fGhostArea = ghost_area;
  
  std::cout << "StJetInterface:INFO  - Modified ghost area, current JetDefinition :: ";
  PrintJetDefinition(); 
  
  return;
}// End of StJetInterface::SetGhostArea


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
  
  std::cout << "; Area [ ";
  
  switch (gJetAreaType) {
    case (fastjet::active_area):           std::cout << "Active(grid), ";   break;
    case (fastjet::passive_area):          std::cout << "Passive(random), ";break;
    case (fastjet::active_area_explicit_ghosts):       std::cout << "explicit active, ";     break;
    case (fastjet::one_ghost_passive_area):            std::cout << "one-ghost passive, ";   break;
    default:                                           std::cout << "undefined";             break;
  }
  
  std::cout << "ghosts to y = " << fMaxRapidity;
  std::cout << ", ghost area = " << fGhostArea;
  
  std::cout << " ] }" << std::endl;
  return;
}// End of StJetInterface::PrintJetDefinition



//----------------------------------------------------------------------------------------------------- Jet Clustering



// Perform the clustering on a PicoDST event
int StJetInterface::ClusterPicoJets(StPicoDst* picoDst) {
  // Add tracks to stable particle vector
  std::vector<fastjet::PseudoJet> stable_particles;
  StPicoEvent* pico_event = picoDst->event();

  if (!pico_event) return 0;

  TVector3 pos_PV = pico_event->primaryVertex();
  
 
  // TODO handle vertex quality, event quality in separate class
  if (pico_event->ranking() < 0) return 0;

  uint32_t nTracks_global = pico_event->numberOfGlobalTracks();
  //uint32_t nTracks_primary = picoDst->numberOfPrimaryTracks();
  
  for (uint32_t i_track = 0; i_track < nTracks_global; i_track++) {
    StPicoTrack *pTrack = picoDst->track(i_track);
    
    if (!pTrack) continue;

    // Track quality cuts :: 
    // TODO move track cuts to separate interface
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
    if (std::fabs(jTrack.pseudorapidity()) >= fMaxRapidity) continue;
    
    // Add to FastJet vector
    stable_particles.push_back(jTrack);

  }// End of PicoDst event track loop
  

  // Perform the clustering
  fastjet::ClusterSequenceArea cluster_area(stable_particles, 
                                            gJetDefinition, 
                                            fastjet::AreaDefinition(gJetAreaType, 
					                            fastjet::GhostedAreaSpec(fMaxRapidity, 1, fGhostArea)));
  std::vector<fastjet::PseudoJet> raw_jets = cluster_area.inclusive_jets(fJetMinPt);
  // End of jet clustering

  // Perform rho background esimtation
  fastjet::ClusterSequenceArea cluster_bg(stable_particles, 
                                          gBgJetDefinition, 
			                  fastjet::AreaDefinition(fastjet::active_area_explicit_ghosts,        
				                                  fastjet::GhostedAreaSpec(fMaxRapidity, 1, fGhostArea)));
  fastjet::JetMedianBackgroundEstimator bg_estimator(fastjet::SelectorAbsRapMax(fMaxRapidity),
                                                     cluster_bg);
  
  // Tabulate BG results
  double bg_rho = bg_estimator.rho();
  fBgInfoHistograms[0]->Fill(bg_rho);
  double bg_sig = bg_estimator.sigma();
  fBgInfoHistograms[1]->Fill(bg_sig);
  
  // Complete the rho * A background pT contribution subtraction
  fastjet::Subtractor subtractor(&bg_estimator);
  std::vector<fastjet::PseudoJet> subtracted_jets = subtractor(raw_jets);
  
  
  
  // Store observables from the resulting jets
  for (uint32_t i = 0; i < subtracted_jets.size(); ++i) {
    fastjet::PseudoJet jet = subtracted_jets[i];
    if (std::fabs(jet.eta()) >= fMaxRapidity - fJetRadius) continue; 
    
    // Tabulate constituents with room for additional cuts
    int jet_nconstituents = 0;
    for (fastjet::PseudoJet c : jet.constituents()) {
      
      // Not a ghost
      if (c.pt() < 1e-50) continue;

      // Could add other selections here, e.g. not an EMCal tower.
      
      ++jet_nconstituents;
    }// End of constituent loop
    
    // populate histograms
    if (fCollectJetHistograms) {
      fCJetPtHist[1]->Fill(jet.pt());
      fCJetNConstituentsHist->Fill(jet_nconstituents);
      fCJetRapPhiHist->Fill(jet.pseudorapidity(), jet.phi());
      fCJetArea->Fill(jet.area());
    }// End of hist fill

    // Tabulate any information for raw jets if desired
    if (i >= raw_jets.size()) continue;
    fCJetPtHist[0]->Fill(raw_jets[i].pt());
  }// End of jet loop
  
  
  return 0;
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
