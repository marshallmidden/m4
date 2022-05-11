#-----------------------------------------------------------------------------
# Devices IP name (needed for windows boxes): Used for mounting name in /mnt.
# Used for ssh login of local administrator: SMB-Server-2012+Administator
#-- I	= 172.22.12.140		SMB-Server-2008.ad.parsec.lab
#-- 	  172.22.12.143		SMB-Server-2012.ad.parsec.lab
#-- 	  172.22.12.144		SMB-Server-2016.ad.parsec.lab
#-- 	  172.22.12.112		SMB-Server-2019.ad.parsec.lab
#-- 	  172.22.14.116		NetApp @ 172.22.14.116 - 7mode
#-- 	  172.22.13.100		Isilon @ 172.22.13.100
#-- 	  172.22.15.113		NetApp @ 172.22.15.113 - 9.

#-- gone 2021-09-08 	  SMB-Server-2016.ad.parsec.lab
I	= SMB-Server-2008.ad.parsec.lab \
	  SMB-Server-2012.ad.parsec.lab \
 	  KRB-TEST2-PDC.krb.test2.lab	\
	  SMB-Server-2019.ad.parsec.lab \
	  172.22.14.116 \
	  172.22.13.100 \
	  172.22.15.113

# Brief name for device.
B	= w2008 \
	  w2012 \
	  w2016 \
	  w2019 \
	  na116 \
	  is100 \
	  cdot113

# Source share name:
S	= m4-v1 \
	  m4-v1 \
	  m4-v1 \
	  m4-v1 \
	  cifs_v1 \
	  m4_v1 \
	  m4_cdot_01
#--	  m4_smb_v1

# Destination share name:
D	= m4-v2 \
	  m4-v2 \
	  m4-v2 \
	  m4-v2 \
	  cifs_v2 \
	  m4_v2 \
	  m4_cdot_02
#--	  m4_smb_v2

# Mount option(s) required:
#--	  "vers=1.0" default, 2.0
O	= "vers=default" \
	  "vers=default" \
	  "vers=default" \
	  "vers=default" \
	  "vers=2.0" \
	  "vers=default" \
	  "vers=default"
#--na103 2020-10-08	  "vers=default"  nor 1.0, 2.0, 2.1, 3.0, 3.02, 3.11

# The windows server to ssh into and setup windows file systems on.
# setup-files script needs it to call scripts-2-copy/SETUP.windowsserver.share.
# First does not need, as is a windows server.
W	= "-" \
	  "-" \
	  "-" \
	  "-" \
	  smb-server-2012.ad.parsec.lab \
	  smb-server-2012.ad.parsec.lab \
	  smb-server-2012.ad.parsec.lab

#-- gone 2021-09-08		  "E:" 
# Windows mount letter to mount on, if not already mounted.
V	= "E:" \
	  "E:" \
	  "E:" \
	  "E:" \
	  "M:" \
	  "N:" \
	  "O:"

# Windows administration login + administrator. 
A	= "SMB-Server-2008" \
	  "SMB-Server-2012" \
	  "SMB-Server-2016" \
	  "SMB-Server-2019" \
	  "smb-server-2012" \
	  "smb-server-2012" \
	  "smb-server-2012"

#-----------------------------------------------------------------------------
wordsI := ${words $(I)}
wordsB := ${words $(B)}
ifneq "${wordsI}" "${wordsB}"
    $(error Number of Devices list (${wordsI}) != brief device names (${wordsB}))
endif
wordsS := ${words $(S)}
ifneq "${wordsI}" "${wordsS}"
    $(error Number of Devices list (${wordsI}) != source share names (${wordsS}))
endif
wordsD := ${words $(D)}
ifneq "${wordsI}" "${wordsD}"
    $(error Number of Devices list (${wordsI}) != source share names (${wordsD}))
endif
wordsO := ${words $(O)}
ifneq "${wordsI}" "${wordsO}"
    $(error Number of Devices list (${wordsI}) != mount options (${wordsO}))
endif
wordsW := ${words $(W)}
ifneq "${wordsI}" "${wordsW}"
    $(error Number of Devices list (${wordsI}) != windows server to run scripts on (${wordsW}))
endif
wordsV := ${words $(V)}
ifneq "${wordsI}" "${wordsV}"
    $(error Number of Devices list (${wordsI}) != window mount volume letter (${wordsV}))
endif
wordsA := ${words $(A)}
ifneq "${wordsI}" "${wordsA}"
    $(error Number of Devices list (${wordsI}) != window mount volume letter (${wordsA}))
endif

s := $(shell seq 1 $(wordsI))
#-----------------------------------------------------------------------------
.PHONY: all
all: help
.PHONY: help
help::
	@echo 
.PHONY: see
#-----------------------------------------------------------------------------
define many_targets
    help::
	@echo "${word $1,${B}}"
	@echo "  setup_${word $1,${B}}          - Execute setup script for ${word $1,${I}}"
	@echo "  info_${word $1,${B}}_${word $1,${S}}     - Get information from ${word $1,${I}} share ${word $1,${S}}"
	@echo "  info_${word $1,${B}}_${word $1,${D}}     - Get information from ${word $1,${I}} share ${word $1,${D}}"
	@echo "  rm_files_${word $1,${B}}_${word $1,${S}} - Remove all files on ${word $1,${I}} share ${word $1,${S}}"
	@echo "  rm_files_${word $1,${B}}_${word $1,${D}} - Remove all files on ${word $1,${I}} share ${word $1,${D}}"
	@echo "  mount_${word $1,${B}}          - Mount both shares on /mnt/${word $1,${I}}/"
	@echo "    mount_${word $1,${B}}_${word $1,${S}}  - Mount share on /mnt/${word $1,${I}}/${word $1,${S}}"
	@echo "    mount_${word $1,${B}}_${word $1,${D}}  - Mount share on /mnt/${word $1,${I}}/${word $1,${D}}"
	@echo "  umount_${word $1,${B}}         - Un-mount both shares from /mnt/${word $1,${I}}/"
	@echo "    umount_${word $1,${B}}_${word $1,${S}} - Un-mount share from /mnt/${word $1,${I}}/${word $1,${S}}"
	@echo "    umount_${word $1,${B}}_${word $1,${D}} - Un-mount share from /mnt/${word $1,${I}}/${word $1,${D}}"
	@echo ' . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .'
# . . . . . . . . . . . . . . . . .
    .PHONY: setup_${word $1,${B}}
    setup_${word $1,${B}}:
	./MAKEFILE-COMMANDS/setup-files "${word $1,${I}}" "${word $1,${S}}" "${word $1,${D}}" \
		      "${word $1,${V}}" "${word $1,${O}}" "${word $1,${W}}" \
		      "${word $1,${A}}"
# . . . . . . . . . . . . . . . . .
    .PHONY: info_${word $1,${B}}_${word $1,${S}}
    info_${word $1,${B}}_${word $1,${S}}:
	./MAKEFILE-COMMANDS/getinfo "${word $1,${I}}" "${word $1,${S}}" "${word $1,${O}}" \
		  "${word $1,${V}}" "${word $1,${W}}"
    .PHONY: info_${word $1,${B}}_${word $1,${D}}
    info_${word $1,${B}}_${word $1,${D}}:
	./MAKEFILE-COMMANDS/getinfo "${word $1,${I}}" "${word $1,${D}}" "${word $1,${O}}" \
		  "${word $1,${V}}" "${word $1,${W}}"
# . . . . . . . . . . . . . . . . .
    # Remove files must mount the directories in rw, which is default.
    .PHONY: rm_files_${word $1,${B}}_${word $1,${S}}
    rm_files_${word $1,${B}}_${word $1,${S}}:
	./MAKEFILE-COMMANDS/rm-files "${word $1,${I}}" "${word $1,${S}}" "${word $1,${O}}" \
		   "${word $1,${W}}" "${word $1,${V}}"
    .PHONY: rm_files_${word $1,${B}}_${word $1,${D}}
    rm_files_${word $1,${B}}_${word $1,${D}}:
	./MAKEFILE-COMMANDS/rm-files "${word $1,${I}}" "${word $1,${D}}" "${word $1,${O}}" \
		   "${word $1,${W}}" "${word $1,${V}}"
# . . . . . . . . . . . . . . . . .
    # Remove files must mount the directories in rw, which is default.
    .PHONY: mount${word $1,${B}}
    mount_${word $1,${B}}:
	./MAKEFILE-COMMANDS/mount "${word $1,${I}}" "${word $1,${S}}" "${word $1,${O}}" "${word $1,${D}}"
    .PHONY: mount_${word $1,${B}}_${word $1,${S}}
    mount_${word $1,${B}}_${word $1,${S}}:
	./MAKEFILE-COMMANDS/mount "${word $1,${I}}" "${word $1,${S}}" "${word $1,${O}}"
    .PHONY: mount_${word $1,${B}}_${word $1,${D}}
    mount_${word $1,${B}}_${word $1,${D}}:
	./MAKEFILE-COMMANDS/mount "${word $1,${I}}" "${word $1,${D}}" "${word $1,${O}}"
# . . . . . . . . . . . . . . . . .
    .PHONY: umount${word $1,${B}}
    umount_${word $1,${B}}:
	./MAKEFILE-COMMANDS/umount ${word $1,${I}} ${word $1,${S}} ${word $1,${D}}
    .PHONY: umount_${word $1,${B}}_${word $1,${S}}
    umount_${word $1,${B}}_${word $1,${S}}:
	./MAKEFILE-COMMANDS/umount ${word $1,${I}} ${word $1,${S}}
    .PHONY: umount_${word $1,${B}}_${word $1,${D}}
    umount_${word $1,${B}}_${word $1,${D}}:
	./MAKEFILE-COMMANDS/umount ${word $1,${I}} ${word $1,${D}}
# . . . . . . . . . . . . . . . . .
    see::
	@echo setup_${word $1,${B}} I=${word $1,${I}} S=${word $1,${S}} D=${word $1,${D}} O=${word $1,${O}}
endef
#-----------------------------------------------------------------------------
$(foreach e,$(s),$(eval $(call many_targets,$(e))))
#-----------------------------------------------------------------------------
.PHONY: all2
all2:
	@echo "I='${I}'"
	@echo "B='${B}'"
	@echo "S='${S}'"
	@echo "D='${D}'"
	@echo "O='${O}'"
	@echo "W='${W}'"
	@echo "V='${V}'"
	@echo "A='${A}'"
	@echo "wordsI=${wordsI}"
	@echo "s='${s}'"
	@echo "I[1]='${word 1,${I}} and S/D/O= ${word 1,${S}}/${word 1,${D}}/${word 1,${O}}' -- first entry"
	@echo "I[2]='${word 2,${I}} and S/D/O= ${word 2,${S}}/${word 2,${D}}/${word 2,${O}}'"
	@echo "I[3]='${word 3,${I}} and S/D/O= ${word 3,${S}}/${word 3,${D}}/${word 3,${O}}'"
	@echo "I[4]='${word 4,${I}} and S/D/O= ${word 4,${S}}/${word 4,${D}}/${word 4,${O}}'"
	@echo "I[5]='${word 5,${I}} and S/D/O= ${word 5,${S}}/${word 5,${D}}/${word 5,${O}}'"
	@echo "I[6]='${word 6,${I}} and S/D/O= ${word 6,${S}}/${word 6,${D}}/${word 6,${O}}'"
	@echo "I[7]='${word 7,${I}} and S/D/O= ${word 7,${S}}/${word 7,${D}}/${word 7,${O}}'"
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f AAA.* outputs/*
	rm -f DIFF-INFO/{XXX,YYY,ZZZ}* DIFF-SETUPS/{XXX,YYY,ZZZ}*
#-----------------------------------------------------------------------------
.PHONY: tags ctags
tags ctags:
	ctags --language-force=python m4.py
#-----------------------------------------------------------------------------
# End of Makefile
