#-----------------------------------------------------------------------------
.PHONY: all help
all help:
	@echo "targets:"
	@echo "    help        This message."
	@echo "    ls          Run with command 'ls'"
	@echo "    echo        Run with command 'echo'"
	@echo "    date        Run with command 'date +%s'"
	@echo "    clean       Remove generated output files starting with AAA."
#-----------------------------------------------------------------------------
.PHONY: ls
ls:
	taskset 1 ./checking ls /tmp/
#-----------------------------------------------------------------------------
.PHONY: echo
echo:
	taskset 1 ./checking echo
#-----------------------------------------------------------------------------
.PHONY: date
date:
	taskset 1 ./checking date '+%s'
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f AAA.* /tmp/AAA.*
#-----------------------------------------------------------------------------
