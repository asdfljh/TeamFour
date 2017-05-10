BUILDDIR = build

all: $(BUILDDIR) $(BUILDDIR)/executer

$(BUILDDIR):
	mkdir -p $@

$(BUILDDIR)/executer: ./src/executer.c $(BUILDDIR)
	gcc -o$@ $<

clean:
	rm -rf $(BUILDDIR)

.PHONY: all clean
