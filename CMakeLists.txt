PROJECT (MENU_FILE_BROWSER_APPLET C)

#compiler flags
SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -g -Wall -ansi")

#install path options
IF (NOT DEFINED CMAKE_INSTALL_PATH)
	SET (CMAKE_INSTALL_PATH "/usr")
ENDIF (NOT DEFINED CMAKE_INSTALL_PATH)
MESSAGE ("Using CMAKE_INSTALL_PATH=${CMAKE_INSTALL_PATH}")

IF (NOT DEFINED CMAKE_INSTALL_LIBDIR)
	SET (CMAKE_INSTALL_LIBDIR "/usr/lib")
ENDIF (NOT DEFINED CMAKE_INSTALL_LIBDIR)
MESSAGE ("Using CMAKE_INSTALL_LIBDIR=${CMAKE_INSTALL_LIBDIR}")

IF(NOT DEFINED CMAKE_INSTALL_GCONF_SCHEMA_DIR)
	SET(CMAKE_INSTALL_GCONF_SCHEMA_DIR "/usr/share/gconf/schemas")
ENDIF(NOT DEFINED CMAKE_INSTALL_GCONF_SCHEMA_DIR)
MESSAGE ("Using CMAKE_INSTALL_GCONF_SCHEMA_DIR=${CMAKE_INSTALL_GCONF_SCHEMA_DIR}")

SUBDIRS (src
		 data
		 pixmaps)

#uninstall target
CONFIGURE_FILE ("${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
				"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
				IMMEDIATE @ONLY)
ADD_CUSTOM_TARGET (uninstall
				   "${CMAKE_COMMAND}" -P 
				   "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

#target to generate a distribution-ready tarball
SET (CPACK_PACKAGE_DESCRIPTION_SUMMARY "File Browser Applet")
SET (CPACK_PACKAGE_VENDOR "Axel von Bertoldi")
SET (CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
SET (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
SET (CPACK_PACKAGE_VERSION_MAJOR 0)
SET (CPACK_PACKAGE_VERSION_MINOR 5)
SET (CPACK_PACKAGE_VERSION_PATCH 5)
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
	 "install_manifest.txt"
)
INCLUDE (CPack)

ADD_CUSTOM_TARGET (deb checkinstall -D --pkgname="menu-file-browser-applet"
									   --pkgversion="${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}"
									   --install=0
									   --deldoc=1
									   --backup=0
									   --pkglicense="GPLv2"
									   --maintainer="bertoldia@gmail.com"
									   --pkggroup="GNOME Desktop Environment"
									   --requires="libpanel-applet2-0, libgnomevfs2-0")

ADD_CUSTOM_TARGET (really-clean rm -fr CMakeCache.txt
									   cmake_install.cmake
									   CPackConfig.cmake
									   cmake_uninstall.cmake
									   CMakeFiles
									   CPackSourceConfig.cmake
									   *.deb
									   install_manifest.txt
									   Makefile
									   pixmaps/CMakeFiles
									   pixmaps/cmake_install.cmake
									   pixmaps/Makefile
									   src/menu-file-browser-applet
									   src/cmake_install.cmake
									   src/CMakeFiles
									   src/Makefile)
 
