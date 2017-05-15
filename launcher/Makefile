BUILDDIR = build
SRCDIR = src

all: $(BUILDDIR) $(SRCDIR) $(BUILDDIR)/launcher

$(BUILDDIR):
	mkdir -p $@

$(BUILDDIR)/launcher: $(SRCDIR)/launcher.c $(BUILDDIR)
	$(CC) $(SRCDIR)/base64.c $(SRCDIR)/jsmn.c -o$@ $<

clean:
	rm -rf $(BUILDDIR)

.PHONY: all clean
