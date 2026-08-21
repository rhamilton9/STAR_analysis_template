// ------------------------------------------------------------------------
//
// Simple STAR PID Interfacer class 
// Made by R. Hamilton [date]
// No GenAI tools were used in the making of this file!
//
// Changelog: 
// 
// ------------------------------------------------------------------------


// Header/base class
#include "StPIDInterface.h"

// Root classes
#include "TClass.h"
#include "TDirectory.h"
#include "TNtuple.h"
#include "TFile.h"
#include "TChain.h"
#include "TNtuple.h"
#include "TTree.h"
#include "TSystem.h"

// STAR PID classes

// PicoDST classes 
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
// should be stored in the local StRoot directory, like this class which lives in [analysis_dir]/StRoot/StPIDInterface





// Declare ROOT class impelentation
// This labels all contents of this file as body for the class.
//
// Must correspond to ClassDef in header class
ClassImp(StPIDInterface);

// Global pointer to current StPIDInterface
StPIDInterface* gStPIDInterface;

//----------------------------------------------------------------------------------------------------- Constructor
StPIDInterface::StPIDInterface() :
// *--- Default initializations for class variables, defined in the header file StPIDInterface.h
//      These variables can be changed only by interacting with class setters, defined in the header file.
//      Defining these variables here declares them available as external objects to other methods in this file
//      that live outside the class body
	fCollectPIDHistograms(true),                  // Switch for collecting PID data histograms
	fOutputFile(nullptr) {                        // TFile pointer to output file
  // Set global pointer to &this on construction
  gStPIDInterface = this;
  
  std::cout << "Initializing StPIDInterface!" << std::endl;

  // Write zeros to the location of this object in memory,
  // so that it is not initialized with random data that could cause 
  // issues if random data were accessed on accident. 
  // In principle this is in std namespace but current rcf has GNU implementation instead
  memset(memory_zero_begin, 0, memory_zero_end - memory_zero_begin + 1);
  

  // Add any initializers that should be run on object creation (i.e. as a constructor)
  fOutputFile = NULL;
}// End StPIDInterface::Constructor

//----------------------------------------------------------------------------------------------------- Deconstructor

// SafeDelete() all pointers initialized with "new" keyword
// because the memory is not erased outside of the relevant scope
//
// SafeDelete() checks that the given pointer is not NULL before deleting it.
StPIDInterface::~StPIDInterface() {
  SafeDelete(fOutputFile);

}// End of StPIDInterface::~StPIDInterface




// Create and store histograms for Primary Vertex reconstruction/quality
void StPIDInterface::BookPIDHistograms() {
  std::cout << "StPIDInterface:INFO  - Booking Particle Identification plots in subdirectory \033[31mPIDInfoPlots\033[39m of output TFile. " << endl;
  
  // Set up TDirectory within the output file
  TDirectory *dirs[2] = {0};
  dirs[0] = TDirectory::CurrentDirectory(); 
  assert(dirs[0]);
  dirs[0]->cd();
  
  // Make subdirectory "PIDInfoPlots" within the parent directory
  if ( !dirs[0]->GetDirectory("PIDInfoPlots") ) dirs[0]->mkdir("PIDInfoPlots");
  dirs[1] = dirs[0]->GetDirectory("PIDInfoPlots"); 
  assert(dirs[1]);
  dirs[1]->cd();
  
  std::cout << "In the PID helper class!" << std::endl;

  // Set up PID plots in this directory
  
  // End of PID plot construction
  


  // Return to parent directory
  dirs[0]->cd();
}// End of StPIDInterface::BookPIDHistograms




//----------------------------------------------------------------------------------------------------- Class Helper Methods
// Note that the methods here should be definitions of the virtual/forward declared methods
// which are included in the class definition StPIDInterface.h. They can be public or
// private methods; Public methods are accessible to other files while private ones 
// can be passed only between methods internal to the class (i.e. other methods in this file)


// A sample helper method
// Note that the method should be declared virtually in the
// header file, where the class implementation is defined!
// 
// This method is "private"
void StPIDInterface::TemplateHelper(/* inputs to the helper function */) {
  // Insert functionality here
  return;
}// End of StPIDInterface::TemplateHelper



//----------------------------------------------------------------------------------------------------- End of StPIDInterface.cxx 
