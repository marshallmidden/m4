#-----------------------------------------------------------------------------
DIRS  = musicomp2abc songs ims t b
#-----------------------------------------------------------------------------
.PHONY: help
help:
	@echo 'Targets:'
	@echo '  all       - Build all songs.'
	@echo '  mp4       - Create .mp4 YouTube videos from all songs.'
	@echo '  mp4_2     - Create _2.mp4 YouTube videos (imscomp variants) from all songs.'
	@echo '  clean     - Remove intermediate .csv,.mid,.abc,.ps,_m.ps,.fs, and .wav files.'
#-----------------------------------------------------------------------------
.PHONY: all
all:
	@for d in ${DIRS}; do echo "=== make all in $$d ===" ; $(MAKE) -C $$d all ; done
#-----------------------------------------------------------------------------
.PHONY: mp4
mp4:
	@for d in songs t b; do echo "=== make mp4 in $$d ===" ; $(MAKE) -C $$d mp4 ; done
#-----------------------------------------------------------------------------
.PHONY: mp4_2
mp4_2:
	@for d in songs t b; do echo "=== make mp4_2 in $$d ===" ; $(MAKE) -C $$d mp4_2 ; done
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	@for d in ${DIRS}; do echo "=== make clean in $$d ===" ; $(MAKE) -C $$d clean ; done
#-----------------------------------------------------------------------------
