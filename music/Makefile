#-----------------------------------------------------------------------------
DIRS  = musicomp2abc songs ims ims/tc-testing/gershwin t b
#-----------------------------------------------------------------------------
.PHONY: help
help:
	@echo 'Targets:'
	@echo '  all       - Build all songs.'
	@echo '  mp4       - Create .mp4 YouTube videos from all songs.'
	@echo '  mp4_2     - Create _2.mp4 YouTube videos (imscomp variants) from all songs.'
	@echo '  sfz       - Render songs to SFZ VPO mix WAVs (sfz-mix/).'
	@echo '  sfz-mp4   - Encode SFZ mixes to MP4 video.'
	@echo '  sfz-clean - Remove SFZ artifacts (sfz-csv/, sfz-mix/).'
	@echo '  setup     - Check for missing tools/soundfonts (read-only).'
	@echo '  clean     - Remove intermediate .csv,.mid,.abc,.ps,_m.ps,.fs, and .wav files.'
#-----------------------------------------------------------------------------
.PHONY: setup
setup:
	./setup.sh
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
.PHONY: sfz
sfz:
	@for d in t b; do echo "=== make sfz in $$d ===" ; $(MAKE) -C $$d sfz ; done
#-----------------------------------------------------------------------------
.PHONY: sfz-mp4
sfz-mp4:
	@for d in t b; do echo "=== make sfz-mp4 in $$d ===" ; $(MAKE) -C $$d sfz-mp4 ; done
#-----------------------------------------------------------------------------
.PHONY: sfz-clean
sfz-clean:
	@for d in t b; do echo "=== make sfz-clean in $$d ===" ; $(MAKE) -C $$d sfz-clean ; done
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	@for d in ${DIRS}; do echo "=== make clean in $$d ===" ; $(MAKE) -C $$d clean ; done
#-----------------------------------------------------------------------------
