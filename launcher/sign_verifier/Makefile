BUILDDIR = build

all: $(BUILDDIR) $(BUILDDIR)/verifier

$(BUILDDIR):
	mkdir -p $@

$(BUILDDIR)/verifier: ./src/verifier.c $(BUILDDIR)
	gcc -o$@ $<

clean:
	rm -rf $(BUILDDIR)

.PHONY: all clean
