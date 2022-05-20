CC := gcc
DFLAGS := -g -gdwarf
CCFLAGS := $(DFLAGS)
LDFLAGS :=

TARGETS:= main
MAIN   := main.o
OBJ    := main.o persytree.o
DEPS   :=

.PHONY: all, clean

all: $(TARGETS)

$(OBJ): %.o : %.c $(DEPS)
		$(CC) -c -o $@ $< $(CCFLAGS)

$(TARGETS): % : $(filter-out $(MAIN), $(OBJ)) %.o
		$(CC) -o $@ $(LIBS) $^ $(CCFLAGS) $(LDFLAGS)

clean:
		rm -f $(TARGETS) $(OBJ)

