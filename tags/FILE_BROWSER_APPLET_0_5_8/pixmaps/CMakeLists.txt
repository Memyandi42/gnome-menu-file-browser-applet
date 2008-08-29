SET (IMAGE "file-browser-applet.png")

#INSTALL (FILES ${IMAGE}  DESTINATION "/usr/share/pixmaps")
#INSTALL (FILES ${IMAGE}  DESTINATION "/usr/share/icons/hicolor/128x128/apps")
INSTALL (FILES ${IMAGE}  DESTINATION "${CMAKE_INSTALL_PATH}/share/pixmaps")
INSTALL (FILES ${IMAGE}  DESTINATION "${CMAKE_INSTALL_PATH}/share/icons/hicolor/128x128/apps")
