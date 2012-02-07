
bin_PROGRAMS = evolution-mail-factory
evolution_mail_factory_SOURCES = 	\
		main.c			\
		mail-send-recv.c	\
		mail-send-recv.h	\
		utils.c			\
		utils.h			\
		e-dbus-manager.c	\
		e-mail-data-session.c	\
		e-mail-data-store.c	\
		e-mail-data-folder.c	\
		e-dbus-manager.h	\
		e-mail-data-session.h	\
		e-mail-data-store.h	\
		e-mail-data-folder.h	\
		e-mail-data-operation.c	\
		e-mail-data-operation.h

noinst_PROGRAMS = test-email-client

test_email_client_SOURCES = test-email-client.c
test_email_client_CPPFLAGS = \
		$(E_MAIL_FACTORY_CFLAGS)	\
		-Wall -g3 -O0 -ggdb -DPKGDATADIR="\"$(pkgdatadir)\""	\
		-I$(top_srcdir)/libegdbus				\
		-I$(top_srcdir)						\
		-I$(top_builddir)					

test_email_client_LDADD = \
		$(top_builddir)/libegdbus/libegdbus-mail.la	\
		$(E_MAIL_FACTORY_LIBS)				


desktopentry_in_files = evolution-mail-factory.desktop.in
desktopentry_DATA = $(desktopentry_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@


evolution_mail_factory_CPPFLAGS = \
		$(E_MAIL_FACTORY_CFLAGS)	\
		-Wall -g3 -O0 -ggdb -DPKGDATADIR="\"$(pkgdatadir)\""	\
		-I$(top_srcdir)/libegdbus				\
		-I$(top_srcdir)						\
		-I$(top_builddir)					

evolution_mail_factory_LDADD = \
		$(top_builddir)/libegdbus/libegdbus-mail.la	\
		$(E_MAIL_FACTORY_LIBS)				

service_in_files = org.gnome.evolution.dataserver.Mail.service.in
servicedir = $(datadir)/dbus-1/services
service_DATA = $(service_in_files:.service.in=.service)


EXTRA_DIST = $(service_in_files) $(desktopentry_in_files)

CLEANFILES = *~ evolution-mail-factory.desktop $(service_DATA)

