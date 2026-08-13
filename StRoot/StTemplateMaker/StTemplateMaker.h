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

// Compiler flags to avoid recompiling/redefining the classes
#ifndef STAR_StTemplateMaker
#define STAR_StTemplateMaker

// Inclusions necessary for the header file
#ifndef StMaker_H
#include "StMaker.h" // STAR base maker header
#endif

// Additional classes
#include "TMVA/Reader.h"

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

// Other classes



// Static variables used elsewhere in the class 



// ----------------------------------------------------------- Class definition
class StTemplateMaker : public StMaker {
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
  TH1D*					fSampleHist;                                    // Sample histogram
  TTree*                                fSampleTree;					// Sample tree
  // *--------------- TTree Hooks ::
  
  // *-- Run level
  
  int bRun_id;                     	// Tree branch hook : run ID
  
  // *-- Event level
  
  int 		bEvent_id;              // Tree branch hook : Event ID
  float 	bEvent_Vx;		// Tree branch hook : Event primary vertex x-coordinate
  float 	bEvent_Vy; 		// Tree branch hook : Event primary vertex y-coordinate
  float 	bEvent_Vr;		// Tree branch hook : Event primary vertex radial coordinate
  float 	bEvent_Vz;		// Tree branch hook : Event primary vertex z-coordinate
  float 	bEvent_Vxerr;		// Tree branch hook : Event primary vertex x error
  float 	bEvent_Vyerr;		// Tree branch hook : Event primary vertex y error
  float 	bEvent_Vrerr;		// Tree branch hook : Event primary vertex radial error
  float 	bEvent_Vzerr;		// Tree branch hook : Event primary vertex z error
  int           bEvent_vertexrank;      // Tree branch hook : Primary vertex rank
  int 		bEvent_refmult;		// Tree branch hook : Event refmult     
  int 		bEvent_tofmult;         // Tree branch hook : Event tofmult
  
  // *----------- END class variables with bitwise overwrite
  Char_t memory_zero_end[1];
  

  // *----------- BEGIN class variables with default initializers
  // Variables which are initialized in the constructor with a default value should be defined here,
  // so that they are not overwritten by zeros in the memory
  
  // Class behavior flags
  Bool_t fIsPicoAnalysis;
  Bool_t fCollectTrackHistograms;
  Bool_t fCollectPIDHistograms;
  Bool_t fCollectPVHistograms;
  Bool_t fWriteDataTree;
  
  // Output data files/formatting
  TString fOutputFileName;
  TFile* fOutputFile;
  
  
  // Private class methods -- must be defined and can only be accessed within StTemplateMaker.cxx
  void TemplateHelper();
  

public: 
  // Public class variables
  

  // Public class methods -- functions available outside of StTempalteMaker.cxx
  
  // Constructor
  StTemplateMaker(const char *name="Template"); // Name inherited from StMaker, hook for finding in BFC
  // Destrictor
  virtual       ~StTemplateMaker();
  // StMaker key hooks
  virtual Int_t Init();
  virtual Int_t InitRun(Int_t runumber);
  virtual Int_t Make();
  virtual Int_t Finish();
  Bool_t        Check();
  // Plotting, QA, debug, memory, etc. utility
  void           BookVertexHistograms();
  void           BookTrackHistograms();
  void           BookPIDHistograms();
  static void    PrintMem(const Char_t *opt = "");
  // Setters
  void SetAnalysePicoDst() 			{ fIsPicoAnalysis = true;  }
  void SetAnalyseMuDst()   			{ fIsPicoAnalysis = false; } 
  void SetCollectTrackHistograms() 		{ fCollectTrackHistograms = true; }
  void SetCollectPIDHistograms() 		{ fCollectPIDHistograms = true; }
  void SetCollectPVHistograms() 		{ fCollectPVHistograms = true; }
  void SetOutputFileName(TString name)          { fOutputFileName = name; }

  // Getters
  TFile* GetOutputFile() 			{ return fOutputFile; }
  
  // Declare ROOT class definition
  ClassDef(StTemplateMaker,0)
};

#endif // STAR_StTemplateMaker
