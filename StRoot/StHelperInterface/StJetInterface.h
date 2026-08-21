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
// terminal commands to replace all instances of StJetInterface with 
// the desired new class name. For convenience this is implemented in the 
// local bash script in the base directory for the template analysis,
//
// 	../../changeTemplateName.sh
// 
// ------------------------------------------------------------------------

// Compiler flags to avoid recompiling/redefining the classes
#ifndef STAR_StJetInterface
#define STAR_StJetInterface

#ifdef HLTCA_STANDALONE
#include "RootTypesDef.h"
#else
#include "TObject.h"
#endif

// Other compiler, debug flags


// Classes that should also be recognized in the
// class implentation, but don't need to be included here
// (Part of the weirdness for ROOT ClassDef/ClassImp?)

// ROOT classes
class TNtuple;
class TFile;
class TChain;
class TTree;
class TH1D;
class TH2D;
class TString;

// STAR classes
class StPicoDst;
class StMuDst;
class StRefMultCorr;

// Static variables used elsewhere in the class 
const double PION_MASS = 0.13957039;

const bool ghost_grid_area = false; // Compute A for rho * A subtraction
const double ghost_pt = 1e-100;
const int nGhost_rap = 100;
const int nGhost_phi = 314;




// ----------------------------------------------------------- Class definition
class StJetInterface : public TObject {
private:
  
  // *----------- BEGIN class variables with bitwise overwrite
  // The pointer to this array points to the location in memory where 
  // the local class variables of this object will be stored. This
  // will be used in the constructor to overwrite these initial values
  // as zero when the object is initialized, in a bitwise manner rather
  // than by iterating over each object which would be tedious.
  //
  // All variables which don't have sensible default definitions should
  // be instantiated within this overwrite window. 
  Char_t memory_zero_begin[1];
  
  // Add class variables here that should be written as zero under the 
  // default constructor, e.g. pointers to objects that may not be used
  // or where it is dangerous to have random data on initialization. 
  // Input data files
  StMuDst*                         	fMuDst; 					// Pointer to MuDST file to read
  StPicoDst*                        	fPicoDst;                          		// Pointer to PicoDST file to read
  
  // TObject pointers
  TH1D*                                 fBgInfoHistograms[2];                          // Histograms for BG rho and sigma
  // Charged jet histograms
  TH1D*                                 fCJetNConstituentsHist;                         // Histograms for jet constituent count
  TH1D*					fCJetPtHist[2];                                 // Histograms for jet pT, before/after subtraction
  TH2D*                                 fCJetRapPhiHist;                                // Histogram for jet eta/phi position
  TH1D*                                 fCJetArea;                                      // Histograms for jet area

  // *----------- END class variables with bitwise overwrite
  Char_t memory_zero_end[1];
  

  // *----------- BEGIN class variables with default initializers
  // Variables which are initialized in the constructor with a default value should be defined here,
  // so that they are not overwritten by zeros in the memory
  
  // Jet settings that take numerical values
  double   fMaxRapidity;
  double   fJetRadius;
  double   fJetMinPt;
  double   fGhostArea;
  double   fHadCorrCoeff;

  // Class behavior flags
  Bool_t   fCollectJetHistograms;  // Control whether the code writes summary histograms
  Bool_t   fClusterEMCalTowers;    // Include EMCal towers in jet clustering, for full jets
  Bool_t   fHadCorrExactTowMatch;  // Include in hadronic correction BEMC towers with exact matches
  BooL_t   fHadCorrCloseTowMatch;  // Include in hadronic correction BEMC towers with close matches

  // Output data files/formatting
  TFile*   fOutputFile;
  
  // Private class methods -- must be defined and can only be accessed within StJetInterface.cxx
  void TemplateHelper();
  

public: 
  // Public class variables
  

  // Public class methods
  
  // Jet definitions and setup
  void    SetJetAlgorithm(TString algo);
  void    SetJetRadius(double radius);
  void    SetRecombinationScheme(TString scheme);
  void    SetJetAreaType(TString area);
  void    SetMaxRapidity(double max_rap);
  void    SetGhostArea(double ghost_area);
  
  // Performing clustering
  int     ClusterPicoJets(StPicoDst* picoDst);
  
  // Constructor
  StJetInterface();
  // Destructor
  virtual       ~StJetInterface();
  // Plotting, QA, debug, memory, etc. utility
  void           BookJetHistograms();
  void           WriteJetHistograms();
  void           PrintJetDefinition();
  // Setters
  void SetJetMinPt(double minpt)                { fJetMinPt = minpt; };
  void SetCollectJetHistograms(bool fHist)      { fCollectJetHistograms = fHist; }
  void SetOutputFile(TFile* newfile)            { fOutputFile = newfile; }

  // Getters
  TFile* GetOutputFile() 			{ return fOutputFile; }
  
  // Declare ROOT class definition
  ClassDef(StJetInterface,1)
};

#endif // STAR_StJetInterface
