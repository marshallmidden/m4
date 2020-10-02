#
# hybrid makefile -- used until we switch to using scons everywhere...
#
SCONS=../../scons

all:
	${SCONS}

CMD = suid_wrapper
OS_DIR = /px/bin/suid
REPO_DIR = ../../PX/bin/suid

BIN = "ipmi packageutil pxstore pxpkgrepoManage pxuseradd pxuserdel pxuserupdate timeset pxcrypt-cli migReport projReport network_cli registration"

install: all repo_install

repo_install: all # copy binary to ../../PX/bin
	mkdir -p $(REPO_DIR)
	for file in "$(BIN)" ; do \
		install -D -m 4755 $(CMD) $(REPO_DIR)/$$file;\
	done

os_install: install # copy binary to ../../PX/bin and /px/bin/
	mkdir -p $(OS_DIR)
	for file in "$(BIN)" ; do \
		install -D -m 4755 $(CMD) $(OS_DIR)/$$file;\
	done

.PHONY: clean clobber
clean clobber:
	${SCONS} -c && rm -f .sconsign.dblite
