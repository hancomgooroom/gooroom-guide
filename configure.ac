AC_INIT([gooroom-guide],[0.1.0])
AC_PREREQ([2.60])
AC_CONFIG_MACRO_DIR([m4])

dnl ***************************
dnl *** Initialize automake ***
dnl ***************************
AM_INIT_AUTOMAKE([1.9 no-dist-gzip dist-xz tar-ustar -Wall])
AC_CONFIG_HEADERS([config.h])
AM_MAINTAINER_MODE()
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(unistd.h libintl.h iconv.h)

# Checks for header files.
AC_LANG([C++])

AC_STDC_HEADERS
AC_PROG_CC
AC_PROG_CXX
AC_PROG_CC_C_O
IT_PROG_INTLTOOL([0.35.0])
AM_PROG_CC_C_O

dnl **************************
dnl *** Initialize libtool ***
dnl **************************
#LT_PREREQ([2.2.6])
#LT_INIT([disable-static])

dnl ******************************
dnl *** Check for i18n support ***
dnl ******************************
XDT_I18N([en_GB ko ])


dnl ---------------------------------------------------------------------------
dnl - Dependencies
dnl ---------------------------------------------------------------------------
PKG_CHECK_MODULES([GLIB], glib-2.0)
PKG_CHECK_MODULES([GOBJECT], gobject-2.0)
PKG_CHECK_MODULES([GTK], gtk+-3.0 >= 2.91.0)
PKG_CHECK_MODULES([WEBKIT2GTK], webkit2gtk-4.0)
PKG_CHECK_MODULES([JSON], jsoncpp)

AC_FUNC_STRCOLL

AC_CONFIG_FILES([Makefile
src/Makefile
po/Makefile.in
data/Makefile
data/gooroom-guide.desktop.in
data/etc/Makefile
data/etc/xdg/Makefile
data/etc/xdg/autostart/Makefile
app-icons/Makefile
app-icons/scalable/Makefile
app-icons/scalable/apps/Makefile
])
AC_OUTPUT

