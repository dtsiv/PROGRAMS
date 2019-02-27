#!/usr/bin/python
# This Python file uses the following encoding: utf-8
codograms       = "codograms.h"
codograms_linux = "codograms_linux.h"
qavtctrl        = "qavtctrl.h"
qavtctrl_linux  = "qavtctrl_linux.h"

import subprocess
import re
import os
# hand-made modules
import utils
import changes

#========================================================================
#
#   perform initial encoding conversion for codograms.h and qavtctrl.h
#
#========================================================================
utils.convert_from_windows(codograms, codograms+".tmp")
utils.convert_from_windows(qavtctrl, qavtctrl+".tmp")

subprocess.call(["mv", codograms+".tmp", codograms_linux])
subprocess.call(["mv", qavtctrl+".tmp", qavtctrl_linux])

#========================================================================
#
#   read file content
#
#========================================================================
fobj = open(codograms_linux,"rt")
codograms_linux_content = fobj.read()
fobj.close();

fobj = open(qavtctrl_linux,"rt")
qavtctrl_linux_content = fobj.read()
fobj.close();

#========================================================================
#
#   make replacement
#
#========================================================================
for change in changes.codograms_changes:
    pattern = change["pattern"]
    replacement = change["replacement"]
    codograms_linux_content.index(pattern)
    codograms_linux_content = codograms_linux_content.replace(
        pattern,replacement,1)

for change in changes.qavtctrl_changes:
    pattern = change["pattern"]
    replacement = change["replacement"]
    qavtctrl_linux_content.index(pattern)
    qavtctrl_linux_content = qavtctrl_linux_content.replace(
        pattern,replacement,1)

#========================================================================
#
#   write the resulting header files to disk
#
#========================================================================
fobj = open(codograms_linux,"wt")
fobj.write(codograms_linux_content)
fobj.close();

fobj = open(qavtctrl_linux,"wt")
fobj.write(qavtctrl_linux_content)
fobj.close();

print "Finished conversion"


