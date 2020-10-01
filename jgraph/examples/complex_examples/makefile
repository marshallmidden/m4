ALL = 	\
	seq.jps		\
	conc.jps	\
	cow.jps		\
	cll.jps		\
	alg.jps		\
	tree1.jps	\
	tree2.jps	\
	wedmap.jps	\
	diskarray.jps	\
	mlti.jps	\
	ckpov.jps	\
	timeline.jps	\
	grex.jps

TMPFILES = \
	regs.awk 	\
	srm.awk 	\
	disk.awk

all: $(ALL)

clean:
	rm -f $(ALL) $(TMPFILES)


seq.jps:  regs.awk convert.awk disk.awk
conc.jps: regs.awk convert.awk disk.awk
cow.jps:  regs.awk convert.awk disk.awk
cll.jps:  regs.awk convert.awk disk.awk

alg.jps: convert.awk regs.awk cow.jgr cll.jgr seq.jgr conc.jgr disk.awk

diskarray.jps: disk.awk convert.awk

ckpov.jps: srm.awk disk.awk convert.awk

tree1.jps: tree.awk
	( echo "4 3" | awk -f tree.awk ; echo "xaxis size 5.4" ) |\
		jgraph -P > tree1.jps
	( echo "5 2" | awk -f tree.awk ; echo "xaxis size 5" ) |\
		jgraph -P > tree2.jps

.SUFFIXES: .gtj .jgr .jps .awk

.jgr.jps:
	jgraph -P < $*.jgr > $*.jps

.jgr.awk:
	sed 's/\\/\\\\/g' $*.jgr | awk -f convert.awk > $*.awk

.gtj.jps:
	sh grtoj.sh < $*.gtj | jgraph -P > $*.jps
