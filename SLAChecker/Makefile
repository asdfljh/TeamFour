BUILDDIR = build

all: $(BUILDDIR) $(BUILDDIR)/tester

$(BUILDDIR):
	mkdir -p $@

$(BUILDDIR)/tester: ./src/sla_tester.c $(BUILDDIR)
	gcc -o$@ $<

clean:
	rm -rf $(BUILDDIR)

.PHONY: all clean
