IMAGE_STORE	:=/home/image-store/cisco
CISCO_BIN_AT	:=xzp/UpdatePackageData
#-----------------------------------------------------------------------------
# VERSION is used for the file name.
# REV is passed tp UpdateSigner.pl for display in icon manager upon applying.
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .-
# These match cisco "show version" output format.
# NOTB means not before.
#	The current usage is to note that if before this value, do not do a
#	'reload' cisco command, because it already has done so with install.
# NOTA means not after -- and is unused.
# MATCH means -- if match this version, do nothing, we are already on it.
#-----------------------------------------------------------------------------
.PHONY: all
all:	4.1.3a 3.1.3a 3.3.1a 4.2.1a 4.2.1b
#-----------------------------------------------------------------------------
.PHONY: 4.1.3a
4.1.3a:
	${MAKE} ${MKOPTS} -C xzp clean
	cp -p ${IMAGE_STORE}/4.1.3a-9100/m9100-s2ek9-kickstart-mz.4.1.3a.bin ${CISCO_BIN_AT}/kickstart.bin
	cp -p ${IMAGE_STORE}/4.1.3a-9100/m9100-s2ek9-mz.4.1.3a.bin ${CISCO_BIN_AT}/system.bin
	${MAKE} ${MKOPTS} VERSION=4_1_3_a REV=4.1.3.a MATCH='4.1(3a)' NOTB='' NOTA='' -C xzp all
	mv xzp/*.xzp ~/
#-----------------------------------------------------------------------------
.PHONY: 3.1.3a
3.1.3a:
	${MAKE} ${MKOPTS} -C xzp clean
	cp -p ${IMAGE_STORE}/3.1.3a-9100/m9100-s2ek9-kickstart-mz.3.1.3a.bin ${CISCO_BIN_AT}/kickstart.bin
	cp -p ${IMAGE_STORE}/3.1.3a-9100/m9100-s2ek9-mz.3.1.3a.bin ${CISCO_BIN_AT}/system.bin
	${MAKE} ${MKOPTS} VERSION=3_1_3_a REV=3.1.3.a MATCH='3.1(3a)' NOTB='' NOTA='' -C xzp all
	mv xzp/*.xzp ~/
#-----------------------------------------------------------------------------
.PHONY: 3.3.1a
3.3.1a:
	${MAKE} ${MKOPTS} -C xzp clean
	cp -p ${IMAGE_STORE}/3.3.1a-9100/m9100-s2ek9-kickstart-mz.3.3.1a.bin ${CISCO_BIN_AT}/kickstart.bin
	cp -p ${IMAGE_STORE}/3.3.1a-9100/m9100-s2ek9-mz.3.3.1a.bin ${CISCO_BIN_AT}/system.bin
	${MAKE} ${MKOPTS} VERSION=3_3_1_a REV=3.3.1.a MATCH='3.3(1a)' NOTB='' NOTA='' -C xzp all
	mv xzp/*.xzp ~/
#-----------------------------------------------------------------------------
.PHONY: 4.2.1a
4.2.1a:
	${MAKE} ${MKOPTS} -C xzp clean
	cp -p ${IMAGE_STORE}/4.2.1a-9100/m9100-s2ek9-kickstart-mz.4.2.1a.bin ${CISCO_BIN_AT}/kickstart.bin
	cp -p ${IMAGE_STORE}/4.2.1a-9100/m9100-s2ek9-mz.4.2.1a.bin ${CISCO_BIN_AT}/system.bin
	${MAKE} ${MKOPTS} VERSION=4_2_1_a REV=4.2.1.a MATCH='4.2(1a)' NOTB='' NOTA='' -C xzp all
	mv xzp/*.xzp ~/
#-----------------------------------------------------------------------------
.PHONY: 4.2.1b
4.2.1b:
	${MAKE} ${MKOPTS} -C xzp clean
	cp -p ${IMAGE_STORE}/4.2.1b-9100/m9100-s2ek9-kickstart-mz.4.2.1b.bin ${CISCO_BIN_AT}/kickstart.bin
	cp -p ${IMAGE_STORE}/4.2.1b-9100/m9100-s2ek9-mz.4.2.1b.bin ${CISCO_BIN_AT}/system.bin
	${MAKE} ${MKOPTS} VERSION=4_2_1_b REV=4.2.1.b MATCH='4.2(1b)' NOTB='3.3.1' NOTA='' -C xzp all
	mv xzp/*.xzp ~/
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	${MAKE} ${MKOPTS} -C xzp clean
#-----------------------------------------------------------------------------
