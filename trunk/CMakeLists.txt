PROJECT (MENU_FILE_BROWSER_APPLET)

#compiler flags
SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -g -Wall -ansi")

SUBDIRS (src pixmaps)

#uninstall target
CONFIGURE_FILE ("${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
				"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
				IMMEDIATE @ONLY)
ADD_CUSTOM_TARGET (uninstall
				   "${CMAKE_COMMAND}" -P 
				   "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

#target to generate a distribution-ready tarball
SET (CPACK_PACKAGE_DESCRIPTION_SUMMARY "Menu File Browser Applet")
SET (CPACK_PACKAGE_VENDOR "Axel von Bertoldi")
SET (CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
SET (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
SET (CPACK_PACKAGE_VERSION_MAJOR 0)
SET (CPACK_PACKAGE_VERSION_MINOR 5)
SET (CPACK_PACKAGE_VERSION_PATCH 3)
SET (CPACK_GENERATOR TGZ)
SET (CPACK_SOURCE_PACKAGE_FILE_NAME
	"menu-file-browser-applet-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
SET (CPACK_SOURCE_GENERATOR TGZ)
SET (CPACK_SOURCE_IGNORE_FILES
	 "CMakeFiles"
	 "Makefile"
	 "progress.make"
	 "CPack*"
	 "CMakeCache.txt"
	 "cmake_install.cmake"
	 #"cmake_uninstall.cmake"
	 "/*.tar.gz"
	 ".svn"
	 "install_manifest.txt"
)
INCLUDE (CPack)

ADD_CUSTOM_TARGET (deb checkinstall -D --pkglicense GPLv2 --maintainer bertoldia@gmail.com --pkggroup "GNOME Desktop Environment" --requires "libpanel-applet2-0, libgnomevfs2-0")
