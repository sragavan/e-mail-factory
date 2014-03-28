NULL =

eutilincludedir = $(privincludedir)/e-util
ruledir = $(privdatadir)

error_DATA = \
	e-system.error \
	filter.error \
	widgets.error \
	$(NULL)
errordir = $(privdatadir)/errors
%.error: %.error.xml
	LC_ALL=C $(INTLTOOL_MERGE) -x -u /tmp/notthere $< $@

privsolib_LTLIBRARIES = libeutil.la

libeutil_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir) \
	-I$(top_builddir) \
	-DLIBEUTIL_COMPILATION \
	-DEVOLUTION_BINDIR=\""$(bindir)"\" \
	-DEVOLUTION_DATADIR=\""$(datadir)"\" \
	-DEVOLUTION_ECPSDIR=\""$(ecpsdir)"\" \
	-DEVOLUTION_ETSPECDIR=\""$(etspecdir)"\" \
	-DEVOLUTION_GALVIEWSDIR=\""$(viewsdir)"\" \
	-DEVOLUTION_HELPDIR=\""$(evolutionhelpdir)"\" \
	-DEVOLUTION_ICONDIR=\""$(icondir)"\" \
	-DEVOLUTION_IMAGESDIR=\""$(imagesdir)"\" \
	-DEVOLUTION_LIBDIR=\""$(datadir)"\" \
	-DEVOLUTION_LIBEXECDIR=\""$(privlibexecdir)"\" \
	-DEVOLUTION_LOCALEDIR=\""$(localedir)"\" \
	-DEVOLUTION_MODULEDIR=\""$(moduledir)"\" \
	-DEVOLUTION_PLUGINDIR=\""$(plugindir)"\" \
	-DEVOLUTION_PREFIX=\""$(prefix)"\" \
	-DEVOLUTION_PRIVDATADIR=\""$(privdatadir)"\" \
	-DEVOLUTION_SOUNDDIR=\""$(soundsdir)"\" \
	-DEVOLUTION_SYSCONFDIR=\""$(sysconfdir)"\" \
	-DEVOLUTION_TOOLSDIR=\""$(privlibexecdir)"\" \
	-DEVOLUTION_UIDIR=\""$(uidir)"\" \
	-DEVOLUTION_RULEDIR=\"$(ruledir)\" \
	-DG_LOG_DOMAIN=\"libeutil\" \
	$(E_MAIL_FACTORY_CFLAGS) \
	$(EVOLUTION_DATA_SERVER_CFLAGS) \
	$(GNOME_PLATFORM_CFLAGS) \
	$(GEO_CFLAGS) \
	$(GTKHTML_CFLAGS) \
	$(NULL)

eutilinclude_HEADERS =  \
	e-util.h \
	e-util-private.h \
	e-alert.h \
	e-alert-dialog.h \
	e-alert-sink.h \
	e-filter-code.h \
	e-filter-color.h \
	e-filter-datespec.h \
	e-filter-element.h \
	e-filter-file.h \
	e-filter-input.h \
	e-filter-int.h \
	e-filter-option.h \
	e-filter-part.h \
	e-filter-rule.h \
	e-misc-utils.h \
	e-passwords.h \
	e-rule-context.h \
	e-xml-utils.h \
	$(NULL)


libeutil_la_SOURCES = \
	$(eutilinclude_HEADERS) \
	e-alert.c \
	e-alert-dialog.c \
	e-alert-sink.c \
	e-filter-code.c \
	e-filter-color.c \
	e-filter-datespec.c \
	e-filter-element.c \
	e-filter-file.c \
	e-filter-input.c \
	e-filter-int.c \
	e-filter-option.c \
	e-filter-part.c \
	e-filter-rule.c \
	e-misc-utils.c \
	e-passwords.c \
	e-rule-context.c \
	e-xml-utils.c \
	$(NULL)

libeutil_la_LDFLAGS = -avoid-version $(NO_UNDEFINED)

libeutil_la_LIBADD =  \
	$(ICONV_LIBS) \
	$(E_MAIL_FACTORY_LIBS) \	
	$(EVOLUTION_DATA_SERVER_LIBS) \
	$(GNOME_PLATFORM_LIBS) \
	$(GEO_LIBS) \
	$(GTKHTML_LIBS) \
	$(INTLLIBS) \
	$(MATH_LIB) \
	$(NULL)

EXTRA_DIST = \
	e-system.error.xml \
	filter.error.xml \
	widgets.error.xml \
	e-marshal.list \
	$(ui_DATA) \
	$(xpm_icons) \
	$(NULL)

BUILT_SOURCES = \
	$(error_DATA) \
	$(NULL)

CLEANFILES = $(BUILT_SOURCES)

dist-hook:
	cd $(distdir); rm -f $(BUILT_SOURCES)

-include $(top_srcdir)/git.mk
