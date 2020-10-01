# Before installing, you should change the string "JGRAPH_DIR" in 
# jgraph.1 to be the directory containing the example jgraphs.
#-----------------------------------------------------------------------------
CC = gcc -ansi -pedantic
#-----------------------------------------------------------------------------
JGRAPH=../jgraph
#-----------------------------------------------------------------------------
EXAMPLES = \
	bailey.jps	\
	ex1.jps		\
	ex2.jps		\
	mab2.jps

#	acc.jps		\
#	acc.eps		\
#	alb.jps		\
#	ad.jps		\
#	ebars.jps	\
#	g8.jps		\
#	g8col.jps	\
#	g9n10.jps	\
#	gpaper.jps	\
#	hypercube.jps	\
#	nr.jps	\
#	sin.jps		\
#	sin1.jps	\
#	sin2.jps	\
#	sin3.jps	\
#	tree1.jps	\
#	tree2.jps	\
#	wortman.jps
#-----------------------------------------------------------------------------
examples: $(EXAMPLES)
#-----------------------------------------------------------------------------
.SUFFIXES: .jgr .jps .eps .tex .dvi

.jgr.jps:
	${JGRAPH} -P < $*.jgr > $*.jps

.jgr.eps:
	${JGRAPH} < $*.jgr > $*.eps

.tex.dvi:
	latex $*.tex
#-----------------------------------------------------------------------------
sin:	sin.o
	cc -o sin sin.o -lm
#-----------------------------------------------------------------------------
ad.jps: cube.eps disk.eps
bailey.jps: bailey.pts
ex2.jps: data.txt
mab2.jps: mab2.times
sin.jps: sin
sin2.jps: sin2.pts
sin3.jps: sin3.pts
tree1.jps: tree.awk
	( echo "4 3" | awk -f tree.awk ; echo "xaxis size 5.4" ) | \
           ${JGRAPH} -P > tree1.jps
tree2.jps: tree.awk
	( echo "5 2" | awk -f tree.awk ; echo "xaxis size 5.4" ) | \
           ${JGRAPH} -P > tree2.jps
#-----------------------------------------------------------------------------
clean: 
	rm -f sin *.o a.out *.jps
#-----------------------------------------------------------------------------
# End of Makefile
