GDBUS_XML_FILES =					\
	../src/e-mail-data-session.xml	\
	../src/e-mail-data-store.xml		\
	../src/e-mail-data-folder.xml

gdbus-files: $(GDBUS_XML_FILES)
	~/opt/master/bin/gdbus-codegen --namespace=EGdbus --strip-prefix=org.gnome.evolution.dataserver.mail --generate-c-code=$^ $^.xml

# The library
noinst_LTLIBRARIES = libegdbus-mail.la

libegdbus_mail_la_CPPFLAGS =			\
	$(AM_CPPFLAGS)				\
	-DG_LOG_DOMAIN=\"libegdbus-mail\"	\
	$(E_MAIL_GDBUS_UTILS_CFLAGS)

libegdbus_mail_la_SOURCES =			
#	e-gdbus-emailfolder.h			\
#	e-gdbus-emailfolder.c			\
#	e-gdbus-emailstore.h		\
#	e-gdbus-emailstore.c		\
#	e-gdbus-emailsession.h		\
#	e-gdbus-emailsession.c		

libegdbus_mail_la_LIBADD =			\
	$(E_MAIL_GDBUS_UTILS_LIBS)

libegdbus_mail_la_LDFLAGS = $(NO_UNDEFINED)


-include $(top_srcdir)/git.mk
