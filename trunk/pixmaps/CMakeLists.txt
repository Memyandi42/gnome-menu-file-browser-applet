SET (IMAGE "menu-file-browser-applet.png")

#INSTALL (FILES ${IMAGE}  DESTINATION "/usr/share/pixmaps")
#INSTALL (FILES ${IMAGE}  DESTINATION "/usr/share/icons/hicolor/128x128/apps")
INSTALL (FILES ${IMAGE}  DESTINATION "${CMAKE_INSTALL_PREFIX}/share/pixmaps")
INSTALL (FILES ${IMAGE}  DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/128x128/apps")
