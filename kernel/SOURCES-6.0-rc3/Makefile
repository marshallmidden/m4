# Read 0.INCLUDE comments for variables to change, and how to use this directory.
include 0.INCLUDE
#-----------------------------------------------------------------------------
.PHONY: help
help:
	@echo "Targets:"
	@echo "  all         - copy ${KDO} to ${DIR}/.config (delete top line)"
	@echo "              - make olddefconfig all modules_install install"
	@echo "              - copy ${DIR}/.config to ${KDN} (add top line)"
	@echo ""
	@echo "  clean       - clean up ${DIR}/.config"
	@echo ""
	@echo "  diff        - diff .config and ${KDO} for =m differences."
	@echo ""
	@echo "Running with variables:"
	@echo "  W   =${W}"
	@echo "  DIR =${DIR}"
#-- 	@echo "  MIDDLE=${MIDDLE}"
	@echo "  ARCH=${ARCH}"
	@echo "  OLDV=${OLDV}"
#--	@echo "  NEWV=${NEWV}"
	@echo "  KDO =${KDO}"
#--	@echo "  KDN =${KDN}"
	@echo "  KDOD=${KDOD}"
#--	@echo "  KDND=${KDOD}"
#-----------------------------------------------------------------------------
.PHONY: all
all:
	${MAKE} debug_kernel
	${MAKE} normal_kernel
#-----------------------------------------------------------------------------
.PHONY: debug_kernel
debug_kernel:
	@echo ''
	@echo 'DEBUG KERNEL'
	@echo ''
	tail -n +2 ${KDOD} > ${DIR}/.config	# delete first line
	(cd ${DIR} && \
	    ${MAKE} olddefconfig && \
	    ${MAKE} -j 255 all && \
	    ${MAKE} -j 255 modules_install && \
	    ${MAKE} -j 255 headers_install && \
	    ${MAKE} -j 255 install)
#--	echo "# ${ARCH}" > ${KDND}
#--	cat ${DIR}/.config >> ${KDND}
#-----------------------------------------------------------------------------
.PHONY: normal_kernel
normal_kernel:
	@echo ''
	@echo 'NORMAL KERNEL'
	@echo ''
	tail -n +2 ${KDO} > ${DIR}/.config	# delete first line
	(cd ${DIR} && \
	    ${MAKE} olddefconfig && \
	    ${MAKE} -j 255 all && \
	    ${MAKE} -j 255 modules_install && \
	    ${MAKE} -j 255 headers_install && \
	    ${MAKE} -j 255 install)
#--	echo "# ${ARCH}" > ${KDN}
#--	cat ${DIR}/.config >> ${KDN}
#-----------------------------------------------------------------------------
.PHONY: compile_kernel k
compile_kernel k:
	@echo ''
	@echo 'COMPILE KERNEL'
	@echo ''
	(cd ${DIR} && \
	    ${MAKE} olddefconfig && \
	    ${MAKE} -j 255 all && \
	    ${MAKE} -j 255 modules_install && \
	    ${MAKE} -j 255 headers_install && \
	    ${MAKE} -j 255 install)
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	(cd ${DIR} && \
	    ${MAKE} -j 255 clean)
#-----------------------------------------------------------------------------
#-- .PHONY: diff
#-- diff:
#-- 	rm -f grep.old.=m grep.new.=m
#-- 	grep '=m' ${KDO} | sort > grep.old.=m
#-- 	grep '=m' ${KDN} | sort > grep.new.=m
#-- 	diff -u grep.old.=m grep.new.=m
#-----------------------------------------------------------------------------
# End of file Makefile
