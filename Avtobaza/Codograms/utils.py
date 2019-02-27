#!/usr/bin/python
# This Python file uses the following encoding: utf-8
import subprocess
import re
import os

#========================================================================
#
#   helper conversion routine
#
#========================================================================
def convert_from_windows(from_file_name=None,to_file_name=None):
    if from_file_name is None:
        return
    if to_file_name is None:
        return
    subprocess.call(["cp", from_file_name, to_file_name+".1"])
    subprocess.call(["dos2unix", to_file_name+".1"])
    fout = open(to_file_name,'wt')
    subprocess.call([  
		       "iconv" \
		     , "-f" \
		     , "WINDOWS-1251" \
		     , "-t" \
		     , "UTF-8" \
		     , to_file_name+".1" \
		    ], \
		     stdout=fout \
		   )
    fout.close()
    subprocess.call(["rm", to_file_name+".1"])

#========================================================================
#
#   the replacement util
#
#========================================================================
def perform_the_change(pattern=None,replacement=None,content=None):
    if (pattern==None or replacement==None or content==None):
        raise Exception("Invalid argument for perform_the_change")
    if (not re.search(pattern,content)):
	raise Exception("Found no match for replacement\n"+replacement) 
    return re.sub(pattern,replacement,content)

