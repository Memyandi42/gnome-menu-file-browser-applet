INCLUDE (UsePkgConfig)

PKGCONFIG (libpanelapplet-2.0
		   LIBPANEL_INCLUDE_DIR
		   LIBPANEL_LINK_DIR
		   LIBPANEL_LINK_FLAGS
		   LIBPANEL_CFLAGS)

PKGCONFIG (gnome-desktop-2.0
		   LIBDESKTOP_INCLUDE_DIR
		   LIBDESKTOP_LINK_DIR
		   LIBDESKTOP_LINK_FLAGS
		   LIBDESKTOP_CFLAGS)

PKGCONFIG (libglade-2.0
		   LIBGLADE_INCLUDE_DIR
		   LIBGLADE_LINK_DIR
		   LIBGLADE_LINK_FLAGS
		   LIBGLADE_CFLAGS)

SET (EXEC menu-file-browser-applet)
SET (SRCS main.c
	 panel-menu-bar.c
 	 menu-browser.c
 	 utils.c
 	 preferences.c
	 vfs.c)

SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${LIBPANEL_CFLAGS} ${LIBGLADE_CFLAGS} ${LIBDESKTOP_CFLAGS}")
ADD_EXECUTABLE (${EXEC} ${SRCS})

# it seems to link file without the dozens of libs this introduces!?
#TARGET_LINK_LIBRARIES (${EXEC} ${LIBPANEL_LINK_FLAGS} ${LIBGLADE_LINK_FLAGS} ${LIBDESKTOP_LINK_FLAGS})
SET (LIBS panel-applet-2 glade-2.0 gnome-desktop-2 gnomevfs-2)
TARGET_LINK_LIBRARIES (${EXEC} ${LIBS})

INSTALL (TARGETS ${EXEC} DESTINATION "${CMAKE_INSTALL_LIBDIR}/gnome-panel")

#FIXME this tries to configure the glade xml file path 
#CONFIGURE_FILE (${MENU_FILE_BROWSER_APPLET_SOURCE_DIR}/src/preferences.h ${MENU_FILE_BROWSER_APPLET_SOURCE_DIR}/src/preferences.h)
