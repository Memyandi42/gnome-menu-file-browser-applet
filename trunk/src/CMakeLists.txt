INCLUDE (UsePkgConfig)

CONFIGURE_FILE ("${FILE_BROWSER_APPLET_SOURCE_DIR}/src/config.h.in" 
				"${FILE_BROWSER_APPLET_SOURCE_DIR}/src/config.h")

PKGCONFIG (libpanelapplet-2.0
		   LIBPANEL_INCLUDE_DIR
		   LIBPANEL_LINK_DIR
		   LIBPANEL_LINK_FLAGS
		   LIBPANEL_CFLAGS)

PKGCONFIG (libglade-2.0
		   LIBGLADE_INCLUDE_DIR
		   LIBGLADE_LINK_DIR
		   LIBGLADE_LINK_FLAGS
		   LIBGLADE_CFLAGS)

PKGCONFIG (gio-unix-2.0
		   LIBGIO_INCLUDE_DIR
		   LIBGIO_LINK_DIR
		   LIBGIO_LINK_FLAGS
		   LIBGIO_CFLAGS)

INCLUDE_DIRECTORIES (/usr/local/include/gtkhotkey-1.0)
LINK_DIRECTORIES (/usr/local/lib)

SET (SRCS main.c
	 panel-menu-bar.c
 	 menu-browser.c
 	 utils.c
 	 preferences.c
	 context-menu.c
	 vfs.c)

SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${LIBPANEL_CFLAGS} ${LIBGLADE_CFLAGS} ${LIBGIO_CFLAGS}")
ADD_EXECUTABLE (${EXEC} ${SRCS})

# it seems to link file without the dozens of libs this introduces!?
#TARGET_LINK_LIBRARIES (${EXEC} ${LIBPANEL_LINK_FLAGS} ${LIBGLADE_LINK_FLAGS} ${LIBDESKTOP_LINK_FLAGS})
SET (LIBS panel-applet-2 glade-2.0 gtkhotkey gio-2.0)
TARGET_LINK_LIBRARIES (${EXEC} ${LIBS})

INSTALL (TARGETS ${EXEC} DESTINATION "${CMAKE_INSTALL_LIB_DIR}/${EXEC}")
