#-----------------------------------------------------------------------------
DIRS  = musicomp2abc songs ims t b
#-----------------------------------------------------------------------------
.PHONY: help
help:
	@echo 'Targets:'
	@echo '  all       - Build all songs.'
	@echo '  clean     - Remove intermediate .csv,.mid,.abc,.ps,_m.ps,.fs, and .wav files.'
#-----------------------------------------------------------------------------
.PHONY: all
all:
	@for d in ${DIRS}; do echo "=== make all in $$d ===" ; $(MAKE) -C $$d all ; done
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	@for d in ${DIRS}; do echo "=== make clean in $$d ===" ; $(MAKE) -C $$d clean ; done
#-----------------------------------------------------------------------------
