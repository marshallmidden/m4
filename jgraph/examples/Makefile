ALL = \
	acc.jps \
	alb.jps \
	col1.jps \
	col6.jps \
	fbf.jps \
	g8.jps \
	incimp.jps \
	rej11.jps \
	usa.jps \
	wortman.jps

all: $(ALL)

clean:
	rm -f $(ALL)


.SUFFIXES: .gtj .jgr .jps .awk

.jgr.jps:
	jgraph -P < $*.jgr > $*.jps

.jgr.awk:
	sed 's/\\/\\\\/g' $*.jgr | awk -f convert.awk > $*.awk

.gtj.jps:
	sh grtoj.sh < $*.gtj | jgraph -P > $*.jps
