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
class TString;

// STAR classes
class StPicoDst;
class StMuDst;
class StRefMultCorr;

// FastJet classes
class JetAlgorithm;
//class fastjet::JetDefinition;

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
  TH1D*					fJetHistograms[10];                                    // Sample histogram
  

  // *----------- END class variables with bitwise overwrite
  Char_t memory_zero_end[1];
  

  // *----------- BEGIN class variables with default initializers
  // Variables which are initialized in the constructor with a default value should be defined here,
  // so that they are not overwritten by zeros in the memory
  
  // Jet settings that take numerical values
  double   fJetRadius;
  double   fJetMinPt;
  
  // Class behavior flags
  Bool_t   fCollectJetHistograms;
  
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
  void SetCollectJetHistograms()                { fCollectJetHistograms = true; }
  void SetOutputFile(TFile* newfile)            { fOutputFile = newfile; }

  // Getters
  TFile* GetOutputFile() 			{ return fOutputFile; }
  
  // Declare ROOT class definition
  ClassDef(StJetInterface,1)
};

#endif // STAR_StJetInterface
