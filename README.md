# STAR Analysis Template
An StMaker-based analysis template for STAR data analysis. 

Made by R. Hamilton in July 2026 by studing other codes, presentations including:
 - [StKFParticleMaker](https://github.com/fisyak/star-sw/tree/TFG/StRoot/StKFParticleAnalysisMaker) by Yuri Fisyak, Maksym Zyzak, (others?)
 - [StJet Framework](https://github.com/joelmazer/star-jetframework/blob/master/StJet.cxx) by Joel Mazer
 - [My experience of data analysis at STAR](https://drupal.star.bnl.gov/STAR/system/files/RMa\_ColMetg\_Junior.pdf) by Rongrong Ma
 - [Introduction to STAR software and makers](https://drupal.star.bnl.gov/STAR/system/files/STAR_soft_BNL_LK_2015_6_1.pdf) by Leszek Kosarzewski
 - [A common-MuDst tutorial](https://www.star.bnl.gov/public/comp/meet/RM200311/MuDstTutorial.pdf) by Sergey Panitkin
No GenAI tools were used in the making of this template!

## Overview
This code interfaces principally with StMuDstMaker and StPicoDstMaker for reading STAR data formats (muDST and picoDST). The execution of analysis tasks is handled by BFC chains which call Make() for any maker (any class inheriting StMaker which was initialized along with the chain). To add one's own analysis tasks, one should modify Init() and Finish() to initialize and write objects for storing analyzed data, and add to the body of Make() the analysis tasks. 


## How to Run
In the current STAR software framework, all analysis must be run in a singularity container

> singularity exec -e --env DISPLAY=$DISPLAY -B /direct -B /gpfs -B /star -B /cvmfs -B /sdcc/lustre02 /cvmfs/star.sdcc.bnl.gov/containers/rhic\_sl7.sif csh

After initializing singularity, use 

> setDEV2.csh

to initialize the relevant library versions. Compile the code together with the new StTemplateMaker class via

> cons

and initialize the analysis chain with root

> root4star -l -q -b TemplateAnalysis.cxx

to run interactively. 
