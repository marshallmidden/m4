#!/bin/bash -ex
# This is to be run after most things are created. It needs root for mknod, etc.
#-----------------------------------------------------------------------------
export ONE_PART=true
export RM_UNPACKED_SOURCES_WHEN_DONE=true
#-----------------------------------------------------------------------------
# Setup the "disk" for getting files, grub, etc.
./017-creatingfilesystem
#-----------------------------------------------------------------------------
# Move the files over (/mnt/new mounted in 017-creatingfilesystem).
. 000.set.variables
tar Scf - --group=0 --owner=0 -C ${CLFS_NEW_ROOT} . | root tar Sxf - --no-same-owner -C ${CLFSMOUNT}
#-----------------------------------------------------------------------------
# GRUB install routine assumes variables are set from 000.set.variables.
./Extra.094-GRUB-install
#-----------------------------------------------------------------------------
root umount ${CLFSMOUNT}
#-----------------------------------------------------------------------------
exit 0
#=============================================================================
