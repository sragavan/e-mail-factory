AC_PREREQ(2.61)

# package version number (not shared library version)
# odd micro numbers indicate in-progress development
# even micro numbers indicate released versions
m4_define(evolution_mail_factory_version_major, 0)
m4_define(evolution_mail_factory_version_minor, 1)
m4_define(evolution_mail_factory_version_micro, 6)

m4_define([evolution_mail_factory_version],
          [evolution_mail_factory_version_major.evolution_mail_factory_version_minor.evolution_mail_factory_version_micro])
m4_define([evolution_mail_factory_api_version],
          [evolution_mail_factory_version_major.evolution_mail_factory_version_minor])

AC_INIT(evolution-mail-factory, evolution_mail_factory_version)
AM_INIT_AUTOMAKE(e-mail-factory, evolution_mail_factory_version)
AC_CONFIG_HEADERS(config.h)
#AC_CONFIG_SRCDIR(libe-mail-factory/e-mail-factory.cpp)
AC_CONFIG_MACRO_DIR([m4])

# Check for programs
AC_PROG_LIBTOOL
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL

IT_PROG_INTLTOOL([0.36.3])

# enable pkg-config
PKG_PROG_PKG_CONFIG

# Check for function availability
# AC_CHECK_FUNCS([memset])

# all libraries listed are associated with "E_MAIL_FACTORY" tag
PKG_CHECK_MODULES(E_MAIL_FACTORY, glib-2.0 gio-unix-2.0 gtk+-3.0 camel-1.2 libedataserver-1.2)

AC_SUBST(E_MAIL_FACTORY_CFLAGS)
AC_SUBST(E_MAIL_FACTORY_LIBS)

PKG_CHECK_MODULES(E_MAIL_GDBUS_UTILS, glib-2.0 gtk+-3.0 gio-2.0 gio-unix-2.0 camel-1.2 libedataserver-1.2 )
AC_SUBST(E_MAIL_GDBUS_UTILS_CFLAGS)
AC_SUBST(E_MAIL_GDBUS_UTILS_LIBS)


# i18n
ALL_LINGUAS=""
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
AC_DEFINE(GETTEXT_PACKAGE, ["e-mail-factory"], [Package name for gettext])

dnl *******************
dnl GObject marshalling
dnl *******************
AM_PATH_GLIB_2_0

dnl We use AC_SUBST_FILE because AC_SUBST won't deal with newlines
EVO_MARSHAL_RULE=$srcdir/marshal.mk
AC_SUBST_FILE(EVO_MARSHAL_RULE)

desktopentrydir=${datadir}/applications
AC_SUBST(desktopentrydir)

icondir=${datadir}/icons/hicolor/32x32/apps
AC_SUBST(icondir)

privsolibdir='${libdir}'/e-mail-factory
AC_SUBST(privsolibdir)

# Output files
AC_OUTPUT([
Makefile
libegdbus/Makefile
e-util/Makefile
libemail-engine/Makefile
libemail-engine/libemail-engine.pc
src/Makefile
po/Makefile.in
src/evolution-mail-factory.desktop
src/org.gnome.evolution.dataserver.Mail.service
])

echo ""
echo "   e-mail-factory $VERSION"
echo "   ====================="
echo "   Documentation: ${enable_gtk_doc}"
echo ""
echo "   To build the project, run \"make\""
echo ""

