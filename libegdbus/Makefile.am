GDBUS_XML_FILES =			\
	../src/e-mail-data-session.xml	\
	../src/e-mail-data-store.xml

gdbus_files: $(GDBUS_XML_FILES)
	gdbus-codegen --c-namespace=EGdbus --interface-prefix=org.gnome.evolution.dataserver.mail --c-generate-object-manager --generate-c-code=e-gdbus-emailsession

# The library
noinst_LTLIBRARIES = libegdbus-mail.la

libegdbus_mail_la_CPPFLAGS =			\
	$(AM_CPPFLAGS)				\
	-DG_LOG_DOMAIN=\"libegdbus-mail\"	\
	$(E_MAIL_GDBUS_UTILS_CFLAGS)

libegdbus_mail_la_SOURCES =			\
	e-gdbus-emailsession.c			\
	e-gdbus-emailstore.c			\
	e-gdbus-emailfolder.c			\
	e-gdbus-emailsession.h			\
	e-gdbus-emailstore.h			\
	e-gdbus-emailfolder.h


libegdbus_mail_la_LIBADD =			\
	$(E_MAIL_GDBUS_UTILS_LIBS)

libegdbus_mail_la_LDFLAGS = $(NO_UNDEFINED)


-include $(top_srcdir)/git.mk
