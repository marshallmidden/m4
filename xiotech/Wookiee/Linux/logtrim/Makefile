# ----------------------------------------------------------------------------
# Makefile for logtrim
# ----------------------------------------------------------------------------
FILES           = $(OBJECTS:%.o=./%.c)
FLAGS           = -c -O3 -pthread -Wall -W -Werror -Wno-unused-parameter
GCC             = gcc
LDFLAGS         = -lz -lbz2 -pthread -Wl,-Map,$@.map
OBJECTS         = logtrim.o
SPEW_OBJECTS    = spew.o

all: deps spew logtrim

deps: depspre depstrim
	@touch .deps
	@echo "Dependencies complete"

depspre:
	@echo "Checking dependencies"
	@rm -f spew logtrim makefile.deps .deps

depstrim: $(FILES)
	@gcc $(FLAGS) -MM $^ | sed -e "s/^\([^ ][^ ]*\)/$$\(OBJECTS\)\1 Makefile/" >> makefile.deps

%.o : ./%.c
	@echo $@
	@$(GCC) $(FLAGS) -o $@ $<

logtrim: ${OBJECTS}
	$(GCC) $(LDFLAGS) -o $@ $^

spew: ${SPEW_OBJECTS}
	$(GCC) $(LDFLAGS) -o $@ $^

clean:
	rm -f spew logtrim *.o *.map makefile.deps .deps

#-----------------------------------------------------------------------------
# Include the compile dependencies.
sinclude makefile.deps
