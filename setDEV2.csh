# Set STAR environment for analysis
# Must be run before running interactively or in a job
# To run, use 
#	source setDEV2.csh

setenv NODEBUG yes

# Star library version
starver .DEV2
source $STAR/setupDEV2.csh
setenv NODEBUG yes

# Tracking Forward Group build
starver TFG20k
setup 64b
setenv STARFPE NO
