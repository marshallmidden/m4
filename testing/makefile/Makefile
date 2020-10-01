# How to loop to do many targets from a list.
#-----------------------------------------------------------------------------
# Devices IP:
I	= smb-server-2012.ad.parsec.lab \
	  172.22.14.116 \
	  172.22.13.100

# Brief name for device.
B	= w2012 \
	  na116 \
	  is100

# Source share name:
S	= m4-v1 \
	  cifs_v1 \
	  m4_v1

# Destination share name:
D	= m4-v2 \
	  cifs_v2 \
	  m4_v2

# Mount option(s) required:
O	= "-" \
	  "vers=1.0" \
	  "-"
#-----------------------------------------------------------------------------
wordsI := ${words $(I)}
wordsB := ${words $(B)}
wordsS := ${words $(S)}
wordsD := ${words $(D)}
ifneq "${wordsI}" "${wordsS}"
    $(error Number of Devices list (${wordsI}) != source share names (${wordsS}))
endif
ifneq "${wordsI}" "${wordsB}"
    $(error Number of Devices list (${wordsI}) != brief device names (${wordsB}))
endif
ifneq "${wordsI}" "${wordsD}"
    $(error Number of Devices list (${wordsI}) != source share names (${wordsD}))
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
	@echo "setup_${word $1,${B}}          - Execute setup script for ${word $1,${I}}"
	@echo "info_${word $1,${B}}_${word $1,${S}}     - Get information from ${word $1,${I}} share ${word $1,${S}}"
	@echo "info_${word $1,${B}}_${word $1,${D}}     - Get information from ${word $1,${I}} share ${word $1,${D}}"
	@echo "rm_files_${word $1,${B}}_${word $1,${S}} - Remove all files on ${word $1,${I}} share ${word $1,${S}}"
	@echo "rm_files_${word $1,${B}}_${word $1,${D}} - Remove all files on ${word $1,${I}} share ${word $1,${D}}"
	@echo "mount_${word $1,${B}}          - Mount both shares on /mnt/${word $1,${I}}/"
	@echo "mount_${word $1,${B}}_${word $1,${S}}    - Mount share on /mnt/${word $1,${I}}/${word $1,${S}}"
	@echo "mount_${word $1,${B}}_${word $1,${D}}    - Mount share on /mnt/${word $1,${I}}/${word $1,${D}}"
	@echo "umount_${word $1,${B}}         - Un-mount both shares from /mnt/${word $1,${I}}/"
	@echo "umount_${word $1,${B}}_${word $1,${S}}   - Un-mount share from /mnt/${word $1,${I}}/${word $1,${S}}"
	@echo "umount_${word $1,${B}}_${word $1,${D}}   - Un-mount share from /mnt/${word $1,${I}}/${word $1,${D}}"
	@echo ' . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .'

    .PHONY: setup_${word $1,${B}}
    setup_${word $1,${B}}:
	@echo NOTDONYET
	@echo setup_${word $1,${B}} I=${word $1,${I}} S=${word $1,${S}} D=${word $1,${D}}
# . . . . . . . . . . . . . . . . .
    info_${word $1,${B}}_${word $1,${S}}:
	@echo NOTDONYET
    info_${word $1,${B}}_${word $1,${D}}:
	@echo NOTDONYET
    rm_files_${word $1,${B}}_${word $1,${S}}:
	@echo NOTDONYET
    rm_files_${word $1,${B}}_${word $1,${D}}:
	@echo NOTDONYET
# . . . . . . . . . . . . . . . . .
    .PHONY: mount${word $1,${B}}
    mount_${word $1,${B}}:
	./mount "${word $1,${I}}" "${word $1,${S}}" "${word $1,"${O}}" "${word $1,${D}}"
    .PHONY: mount_${word $1,${B}}_${word $1,${S}}
    mount_${word $1,${B}}_${word $1,${S}}:
	./mount "${word $1,${I}}" "${word $1,${S}}" "${word $1,"${O}}"
    .PHONY: mount_${word $1,${B}}_${word $1,${D}}
    mount_${word $1,${B}}_${word $1,${D}}:
	./mount "${word $1,${I}}" "${word $1,${D}}" "${word $1,"${O}}"
# . . . . . . . . . . . . . . . . .
    .PHONY: umount${word $1,${B}}
    umount_${word $1,${B}}:
	./umount ${word $1,${B}} ${word $1,${S}} ${word $1,${D}}
    .PHONY: umount_${word $1,${B}}_${word $1,${S}}
    umount_${word $1,${B}}_${word $1,${S}}:
	./umount ${word $1,${B}} ${word $1,${S}}
    .PHONY: umount_${word $1,${B}}_${word $1,${D}}
    umount_${word $1,${B}}_${word $1,${D}}:
	./umount ${word $1,${B}} ${word $1,${D}}
# . . . . . . . . . . . . . . . . .
    see::
	@echo setup_${word $1,${B}} I=${word $1,${I}} S=${word $1,${S}} D=${word $1,${D}}
endef
#-----------------------------------------------------------------------------
# $(foreach e,$(D),$(eval $(call many_targets,$(e))))
$(foreach e,$(s),$(eval $(call many_targets,$(e))))
#-----------------------------------------------------------------------------
.PHONY: all2
all2:
	@echo "I=${I}"
	@echo "S=${S}"
	@echo "D=${D}"
	@echo "wordsI=${wordsI}"
	@echo "s='${s}'"
	@echo "I[1]=${word 1,${I}} and S/D= ${word 1,${S}}/${word 1,${D}} -- first entry"
	@echo "I[2]=${word 2,${I}} and S/D= ${word 2,${S}}/${word 1,${D}}"
	@echo "I[3]=${word 3,${I}} and S/D= ${word 3,${S}}/${word 1,${D}}"

#
#-- 	@echo "  setup-2012          - Execute script setup.${W2012}"
#-- 	@echo "                        Removes files from ${W2012_V1} and ${W2012_V2}, and executes: SETUP.wind
#-- ows.share"
#-- 	@echo "  info-2012-${W2012_V1}     - Grab information for ${W2012_V1} volume via script getinfo.${W2012
#-- }.${W2012_V1}"
#-- 	@echo "  info-2012-${W2012_V2}     - Grab information for ${W2012_V2} volume via script getinfo.${W2012
#-- }.${W2012_V2}"
#-- 	@echo "  rm-2012-${W2012_V1}       - Remove files on ${W2012} volume ${W2012_V1}"
#-- 	@echo "  rm-2012-${W2012_V2}       - Remove files on ${W2012} volume ${W2012_V2}"
#-- 	@echo "  mount-2012-${W2012_V1}    - Mount directory ${W2012_V1} on /mnt/${W2012}/${W2012_V1} read/writ
#-- e."
#-- 	@echo "  mount-2012-${W2012_V2}    - Mount directory ${W2012_V2} on /mnt/${W2012}/${W2012_V2} read/writ
#-- e."
#-- 	@echo "  umount-2012-${W2012_V1}   - Un-mount /mnt/${W2012}/${W2012_V1}"
#-- 	@echo "  umount-2012-${W2012_V2}   - Un-mount /mnt/${W2012}/${W2012_V2}"
#-- 	@echo "  umount-2012         - Un-mount both ${W2012_V1} and ${W2012_V2} network mounts."
#-- # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#-- 	@echo
#-- 	@echo "  setup-116           - Execute script setup.${NA116}"
#-- 	@echo "                        Removes files from ${NA116_V1} and ${NA116_V2}, then executes: SETUP.win
