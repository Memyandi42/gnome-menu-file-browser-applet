INCLUDE (UsePkgConfig)

PKGCONFIG (libpanelapplet-2.0	LIBPANEL_INCLUDE_DIR
								LIBPANEL_LINK_DIR
								LIBPANEL_LINK_FLAGS
								LIBPANEL_CFLAGS)

SET (SERVER GNOME_MenuFileBrowserApplet_Factory.server)
SET (SCHEMA menu-file-browser-applet.schemas)
SET (EXEC menu-file-browser-applet)
SET (SRCS main.c
	 panel-menu-bar.c
 	 menu-browser.c
 	 utils.c
 	 preferences.c)

SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${LIBPANEL_CFLAGS}")
ADD_EXECUTABLE (${EXEC} ${SRCS})

# it seems to link file without the dozens of libs this introduces!?
#TARGET_LINK_LIBRARIES (${EXEC} ${LIBPANEL_LINK_FLAGS})
SET (LIBS panel-applet-2)
TARGET_LINK_LIBRARIES (${EXEC} ${LIBS})

#INSTALL (TARGETS ${EXEC} DESTINATION "/usr/lib/gnome-panel")
#INSTALL (FILES ${SERVER} DESTINATION "/usr/lib/bonobo/servers")
#INSTALL (FILES ${SCHEMA} DESTINATION "/usr/share/gconf/schemas")
INSTALL (TARGETS ${EXEC} DESTINATION "${CMAKE_INSTALL_LIBDIR}/gnome-panel")
INSTALL (FILES ${SCHEMA} DESTINATION "${CMAKE_INSTALL_GCONFDIR}")
CONFIGURE_FILE (${MENU_FILE_BROWSER_APPLET_SOURCE_DIR}/src/${SERVER}.in
				${MENU_FILE_BROWSER_APPLET_SOURCE_DIR}/src/${SERVER})
INSTALL (FILES ${SERVER} DESTINATION "${CMAKE_INSTALL_LIBDIR}/bonobo/servers")

#INSTALL (CODE "EXEC_PROGRAM (\"gconf-schemas --register ${SCHEMA}\")")
INSTALL (CODE "EXEC_PROGRAM (\"export GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source` && gconftool-2 --makefile-uninstall-rule ${CMAKE_INSTALL_GCONFDIR}/menu-file-browser-applet.schemas\")")
