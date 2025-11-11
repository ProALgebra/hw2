EXECUTABLE = src/lamac
INSTALL ?= install -v
MKDIR ?= mkdir

.PHONY: all regression

all:
	$(MAKE) -C src
	$(MAKE) -C runtime
	$(MAKE) -C stdlib
	$(MAKE) -C runtime32
	$(MAKE) -C byterun

STD_FILES=$(shell ls stdlib/*.[oi] stdlib/*.lama runtime/runtime.a runtime/Std.i)

install: all
	$(INSTALL) $(EXECUTABLE) `opam var bin`
	$(MKDIR) -p `opam var share`/Lama
	$(INSTALL) $(STD_FILES) `opam var share`/Lama/

uninstall:
	$(RM) -r `opam var share`/Lama
	$(RM) `opam var bin`/$(EXECUTABLE)

regression: install
	$(MAKE)  -C regression

clean:
	@dune clean

