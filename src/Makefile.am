bin_PROGRAMS = gooroom-guide

BUILT_SOURCES =	\
	guide-resources.c	\
	guide-resources.h

AM_CPPFLAGS =	\
	-I$(top_srcdir)	\
	-DLOCALEDIR=\"$(localedir)\"	\
	-DPACKAGE_DATA_DIR=\""$(datadir)"\"	\
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\"	\
	-DPACKAGE_GUIDEDIR=\""$(datadir)/gooroom-guide/"\"	\
	$(GLIB_CFLAGS)	\
	$(GTK_CFLAGS)	\
	$(WEBKIT2GTK_CFLAGS)	\
	$(JSON_CFLAGS)

gooroom_guide_LDADD =	\
	$(GTK_LIBS)	\
	$(GLIB_LIBS)	\
	$(WEBKIT2GTK_LIBS)	\
	$(JSON_LIBS)

gooroom_guide_SOURCES=	\
	guide.cpp	\
	stdatx.h	\
	$(BUILT_SOURCES)

resource_files = $(shell glib-compile-resources --sourcedir=$(srcdir) --generate-dependencies $(srcdir)/resource.xml)
guide-resources.c: resource.xml $(resource_files)
	$(AM_V_GEN) glib-compile-resources --target=$@ --sourcedir=$(srcdir) --generate-source --c-name guide $<
guide-resources.h: resource.xml $(resource_files)
	$(AM_V_GEN) glib-compile-resources --target=$@ --sourcedir=$(srcdir) --generate-header --c-name guide $<
