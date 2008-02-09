SET (SERVER GNOME_MenuFileBrowserApplet_Factory.server)
SET (SCHEMA menu-file-browser-applet.schemas)
SET (GLADEUI menu-file-browser-applet.glade)

CONFIGURE_FILE (${MENU_FILE_BROWSER_APPLET_SOURCE_DIR}/data/${SERVER}.in 
				${MENU_FILE_BROWSER_APPLET_SOURCE_DIR}/data/${SERVER})

INSTALL (FILES ${SCHEMA} DESTINATION "${CMAKE_INSTALL_GCONF_SCHEMA_DIR}")
INSTALL (FILES ${SERVER} DESTINATION "${CMAKE_INSTALL_LIBDIR}/bonobo/servers")
INSTALL (FILES ${GLADEUI} DESTINATION "${CMAKE_INSTALL_PATH}/share/menu-file-browser-applet/glade")

INSTALL (CODE "EXEC_PROGRAM (\"export GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source` && gconftool-2 --makefile-install-rule ${CMAKE_INSTALL_GCONF_SCHEMA_DIR}/${SCHEMA}\")")
