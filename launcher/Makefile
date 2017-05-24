BUILDDIR = build
FILESDIR = files
SRCDIR = src

all: $(FILESDIR) $(BUILDDIR) $(SRCDIR) $(BUILDDIR)/launcher

$(BUILDDIR):
	mkdir -p $@

$(FILESDIR):
	mkdir -p $@

$(BUILDDIR)/launcher: $(SRCDIR)/launcher.c $(BUILDDIR)
	$(CC) $(SRCDIR)/base64.c $(SRCDIR)/jsmn.c -o$@ $<

clean:
	rm -rf $(BUILDDIR)
	rm -rf $(FILESDIR)

.PHONY: all clean
