INCLUDE (UsePkgConfig)

PKGCONFIG (libpanelapplet-2.0	LIBPANEL_INCLUDE_DIR
								LIBPANEL_LINK_DIR
								LIBPANEL_LINK_FLAGS
								LIBPANEL_CFLAGS)

SET (SERVER GNOME_MenuFileBrowserApplet_Factory.server)
SET (EXEC menu-file-browser-applet)
SET (SRCS main.c
	menu-browser.c
	utils.c
	preferences.c)

SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${LIBPANEL_CFLAGS}")
ADD_EXECUTABLE (${EXEC} ${SRCS})
TARGET_LINK_LIBRARIES (${EXEC} ${LIBPANEL_LINK_FLAGS})
INSTALL (TARGETS ${EXEC} DESTINATION "/usr/lib/gnome-panel")
INSTALL (FILES ${SERVER} DESTINATION "/usr/lib/bonobo/servers")

