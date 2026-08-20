// ------------------------------------------------------------------------
//
// Template Star Analysis code
// Made by R. Hamilton Aug. 4, 2026 by studing other codes including:
//   * FemtoAnalysis.C made by Maksym Zyzak
// No GenAI tools were used in the making of this file!
//
// Changelog: 
//   - Aug. 04, 2026 : Template maker class completed
//   - Aug. 12, 2026 : TemplateAnalysis.C completed/interfacing with data properly
// 
// ------------------------------------------------------------------------
// 
// To create a new class using this template it is easiest to use
// terminal commands to replace all instances of StTemplateMaker with 
// the desired new class name. For convenience this is implemented in the 
// local bash script in the base directory for the template analysis,
//
// 	changeTemplateName.sh
// 
// ------------------------------------------------------------------------


// Global variables, forward declarations for helper methods, compiler flags, etc. can be placed here



// The main control through which the analysis is executed
//
// Inputs:
//  - arg1 (N): Upper limit on events to run, in case a smaller test is desirable. <= 0 gives all events
//  - arg2 (input): List of files to run over, see README for details on generating this list
//  - arg3 (output): Name for the output file, default is "output.root".
//  - arg4 (isPico): Bool which controls whether the code should expect MuDST format or PicoDST format. See README for more information.
// Outputs: 
//  - Root file with histograms and data specified by StTemplateAnalysis.cxx
//  	- QA plots for Primary Vertex quality and reconstruction position
//  	- Track level information from the events run over
//  	- PID performance plots for reconstructed tracks
//  	- Optional TTree output for event-level information
//  	- (More functionality to be added for specific analyses) 
//
// Compile the StRoot libraries together with local additions to StRoot
//
//      cons
//
// Then run this analysis script as a root macro
//
//	root -l -q -b 'femtoAnalysis.C({Event number limit}, "{input file list}.list", "{Output file name}.root", {Bool for PicoDST/MuDST})'
//
// The single/double quotes are necessary for the file names to be properly read as strings
void TemplateAnalysis(Long64_t N = 0, 
                      const Char_t* input = "pp2012.list",
                      const Char_t* output = "template_out.root",
                      bool isPico = true) {
  // Check for necessary complier information
#if !defined(__CINT__)
  std::cout << "This code cannot be compiled" << std::endl;
#else
  

  // *----------------------------- Primary File I/O
  
  // If input file is a list file, add list 
  char final_input_file[100];
  if (std::strstr(input, "list") != NULL) {
    snprintf(final_input_file, 100, "lists/%s",input);
  } else {
    snprintf(final_input_file, 100, "%s", input);
  }

  // Report on output file location
  std::cout << "Beginning analysis with StTemplateMaker for input list {\033[31m" << final_input_file << "\033[39m}." << endl;
  std::cout << "Output will be saved to {\033[31m" << output << "\033[39m}." << std::endl;
  
  
  
  // *----------------------------- System Control / Load Libraries
  
  // Set ROOT to mask certain outputs
  //  gSystem->SetFPEMask(kInvalid | kDivByZero | kOverflow );
  
  // Read in the MuDST or PicoDST as desired            
  gROOT->LoadMacro("lMuDst.C");
  if (isPico) {
    std::cout << "Input data is assumed to be of type \033[31mPicoDST\033[39m." << std::endl;
    lMuDst(-1,final_input_file,"ry2016,RpicoDst,mysql,quiet,nodefault",output);
  } else {
    std::cout << "Input data is assumed to be of type \033[31mMuDST\033[39m." << std::endl;
    lMuDst(-1,final_input_file,"ry2016,picoEvt,RMuDst,mysql,quiet,nodefault",output);
  }
  

  // *----------------------------- Interface with Custom Analysis Class

  // Load all custom classes not included in root standard libraries
  // Include custom libraries
  gSystem->Load("StHelperInterface");
  gSystem->Load("StTemplateMaker");

  // Construct the StTemplateMaker and initialize its variables
  StTemplateMaker* templateAnalysisMaker = new StTemplateMaker();
  templateAnalysisMaker->SetOutputFileName(output);
  
  templateAnalysisMaker->SetCollectPIDHistograms();
  templateAnalysisMaker->SetCollectTrackHistograms();
  templateAnalysisMaker->SetCollectPVHistograms();
  templateAnalysisMaker->SetCollectJetHistograms();
  
  // Specialized settings for PicoDST, MuDST
  if(isPico) {
    templateAnalysisMaker->SetAnalysePicoDst();
    
    // PicoDST-specific settings here
  } else {
    templateAnalysisMaker->SetAnalyseMuDst();
    
    // MuDST-specific settings here
  }


  // *----------------------------- Begin the Analysis Loop
  
  // Initialize the file chain BFC
  chain->Init();
  
  // Find the number of successfully reconstructed event from the file maker
  Long64_t nentries = 0;
  if(isPico) {//PicoDST
    StPicoDstMaker* maker = (StPicoDstMaker *) StMaker::GetTopChain()->Maker("PicoDst");
    if (!maker) return;
    maker->SetStatus("*",1);
    nentries = maker->chain()->GetEntries();
  }// End of PicoDST
  
  else {// MuDST
    StMuDstMaker* maker = (StMuDstMaker *) StMaker::GetTopChain()->Maker("MuDst");
    if (!maker) return;
    maker->SetStatus("*",1);
    nentries = maker->chain()->GetEntries();
  }// End of MuDST
  
  // Allow for small sample size if requested/applicable
  Long64_t nevent = TMath::Min(N,nentries);;
  if (N <= 0) nevent = nentries;
  cout << "\033[32m" << nevent << "\033[39m events will be read from the chain." << endl;

  // Begin the event loop
  chain->EventLoop(nevent);

#endif 
  return;
}// End of TemplateAnalysis.cxx
