#!/bin/bash
#
# A helper file to remind one how to use get_file_list.pl, the perl script
# used to find file locations on RCF, which has a complicated set of inputs. 
#
# See URL :
#       https://ucr-rhic.github.io/how-tos/get_file_list.html
# for more information, and file list
#       https://www.star.bnl.gov/public/comp/prod/localdata/ProdDDstreams.html
# for a full list of data on disk for STAR.
#
# !!!!!!!!!!
# Singularity should be enabled to find files on distributed disk!
# !!!!!!!!!!


# Use "0" to get all files
total_files="10"

# Either "daq_reco_picoDst" or "daq_reco_muDst"
# Input is not case sensitive.
file_type="daq_reco_PicoDst"

# Trigger setup, typically "AA{Energy}_production_{year} or similar, 
# see file list {https://www.star.bnl.gov/public/comp/prod/localdata/ProdDDstreams.html} to find the desired dataset
trigger_setup_name="pp200_production_2012"

# Software production key, since some datasets are produced multiple times
# as changes are made and newer tools are added to the production process
production_key="P12id"

# Name of the file to output
outfile_name="pp2012.list"


# Stat directories, remove previous file if it exists
if [ ! -d "lists" ]; then 
    mkdir lists
fi
fullfilename="lists/${outfile_name}"
if [ -f $fullfilename ]; then
    rm $fullfilename
fi

# Run
echo "Beginning search for files in dataset ${trigger_setup_name}"
get_file_list.pl -keys 'path,filename' -cond "storage!=hpss,filetype=${file_type},trgsetupname=${trigger_setup_name},production=${production_key}" -limit $total_files -delim '/' > $fullfilename

# If files are not on HPSS, add the relevant prefix "root://xrdstar.rcf.bnl.gov:1095/"
sed -i 's/\/home\/starlib/root:\/\/xrdstar.rcf.bnl.gov:1095\/\/home\/starlib/g' $fullfilename

# Report on found files
nfiles=$( grep -c ^ ${fullfilename} )
echo "Wrote a total of ${nfiles} files to ${fullfilename}."


