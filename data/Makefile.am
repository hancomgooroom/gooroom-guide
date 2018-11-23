SUBDIRS = etc

help_confdir = $(datadir)/gooroom-guide/guide
help_conf_DATA = \
	guide/toc.json	\
	guide/common.css

help_intro_confdir = $(help_confdir)/intro
help_intro_conf_DATA = $(wildcard guide/intro/*.html)

help_desktop_confdir = $(help_confdir)/desktop
help_desktop_in_files = $(wildcard guide/desktop/*.html)
help_desktop_conf_DATA = $(help_desktop_in_files)

help_secure_confdir = $(help_confdir)/secure
help_secure_in_files = $(wildcard guide/secure/*.*)
help_secure_conf_DATA = $(help_secure_in_files)

help_font_confdir = $(help_confdir)/fonts
help_font_in_files = $(wildcard guide/fonts/*.ttf)
help_font_conf_DATA = $(help_font_in_files)

help_images_confdir = $(help_confdir)/images
help_images_in_files = $(wildcard guide/images/*.*)
help_images_conf_DATA = $(help_images_in_files)

#delete norun file
#norundir = $(HOME)/.gooroom
removedir = $(sysconfdir)/gooroom/gooroom-guide
SHELL = /bin/bash

@INTLTOOL_DESKTOP_RULE@
desktopdir = $(datadir)/applications
desktop_in_files = gooroom-guide.desktop.in.in
desktop_DATA = $(desktop_in_files:.desktop.in.in=.desktop)

EXTRA_DIST = $(desktop_in_files)
DISTCLEANFILES = $(desktop_DATA) $(removedir)



