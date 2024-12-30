.PHONY: all
all:
	rm -rf mmtool mmtool.tgz
	mkdir mmtool
	cp migrate mmtool/
	cp -r ../../../CCBE/XIOTech mmtool/
	cp ../../../CCBE/ccbCL.pl mmtool/
	rm -rf mmtool/.svn/ mmtool/XIOTech/.svn/
	tar zcf mmtool.tgz mmtool
	rm -rf mmtool

.PHONY: clean
clean:
	rm -rf mmtool mmtool.tgz
# End of Makefile
