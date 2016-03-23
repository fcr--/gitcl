CFLAGS += -Wall -std=c99
CFLAGS += `pkg-config --cflags gobject-introspection-1.0`
LIBS += `pkg-config --libs gobject-introspection-1.0`

# $(call shellvar,F,V,D): retrieves a variable from an includable shell script
#   F: filename of shell script.
#   V: variable name to retrieve.
#   D: default value to use whever V is undefined or null.
shellvar = $(shell . '$(1)'; printf %s "$${$(2):-$(3)}")

TCLCONFIG ?= $(shell for f in /usr/lib/tclConfig.sh /usr/lib64/tclConfig.sh \
	/usr/lib/tcl8.6/tclConfig.sh /usr/lib/tcl8.5/tclConfig.sh; do \
	[ -f "$$f" ] && echo "$$f" && break; done)

LIBS += $(call shellvar,$(TCLCONFIG),TCL_LIB_SPEC)
CFLAGS += $(call shellvar,$(TCLCONFIG),TCL_SHLIB_CFLAGS)
CFLAGS += $(call shellvar,$(TCLCONFIG),TCL_INCLUDE_SPEC)

all: libgitcl.so

libgitcl.so: gitcl.o
	$(CC) $^ -o $@ $(LIBS) $(LDFLAGS) -shared

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

.PHONY: clean test

clean:
	rm -f libgitcl.so gitcl.o

test tests:
	./test.tcl
