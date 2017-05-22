BUILDDIR = build

all: $(BUILDDIR) $(BUILDDIR)/tester

$(BUILDDIR):
	mkdir -p $@

$(BUILDDIR)/right_github: ./src/right_github.c $(BUILDDIR)
	gcc -o$@ $<

$(BUILDDIR)/wrong_github: ./src/wrong_github.c $(BUILDDIR)
	gcc -o$@ $<

$(BUILDDIR)/tester: ./src/sla_tester.c $(BUILDDIR)
	gcc -o$@ $<

clean:
	rm -rf $(BUILDDIR)

.PHONY: all clean
