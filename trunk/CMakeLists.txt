CMAKE_MINIMUM_REQUIRED (VERSION 2.4.7)

PROJECT (FILE_BROWSER_APPLET C)

#compiler flags
SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -g -Wall -ansi")
SET (EXEC file-browser-applet)

#install path options
################################################################################
IF (NOT DEFINED CMAKE_INSTALL_PREFIX)
	SET (CMAKE_INSTALL_PREFIX "/usr")
ENDIF (NOT DEFINED CMAKE_INSTALL_PREFIX)
MESSAGE ("Using CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")
################################################################################
IF (NOT DEFINED CMAKE_INSTALL_LIB_PREFIX)
	SET (CMAKE_INSTALL_LIB_PREFIX "${CMAKE_INSTALL_PREFIX}/lib")
ENDIF (NOT DEFINED CMAKE_INSTALL_LIB_PREFIX)
MESSAGE ("Using CMAKE_INSTALL_LIB_PREFIX=${CMAKE_INSTALL_LIB_PREFIX}")
################################################################################
IF (NOT DEFINED CMAKE_INSTALL_LIB_EXEC_DIR)
	SET (CMAKE_INSTALL_LIB_EXEC_DIR "${CMAKE_INSTALL_LIB_PREFIX}/${EXEC}")
ENDIF (NOT DEFINED CMAKE_INSTALL_LIB_EXEC_DIR)
MESSAGE ("Using CMAKE_INSTALL_LIB_EXEC_DIR=${CMAKE_INSTALL_LIB_EXEC_DIR}")
################################################################################
IF (NOT DEFINED CMAKE_INSTALL_BONOBO_DIR)
	SET (CMAKE_INSTALL_BONOBO_DIR "${CMAKE_INSTALL_LIB_PREFIX}/bonobo/servers")
ENDIF (NOT DEFINED CMAKE_INSTALL_BONOBO_DIR)
MESSAGE ("Using CMAKE_INSTALL_BONOBO_DIR=${CMAKE_INSTALL_BONOBO_DIR}")
################################################################################
IF (NOT DEFINED CMAKE_INSTALL_GCONF_SCHEMA_DIR)
	SET(CMAKE_INSTALL_GCONF_SCHEMA_DIR "${CMAKE_INSTALL_PREFIX}/share/gconf/schemas")
ENDIF(NOT DEFINED CMAKE_INSTALL_GCONF_SCHEMA_DIR)
MESSAGE ("Using CMAKE_INSTALL_GCONF_SCHEMA_DIR=${CMAKE_INSTALL_GCONF_SCHEMA_DIR}")
################################################################################
IF (NOT DEFINED CMAKE_INSTALL_GLADEUI_DIR)
	SET(CMAKE_INSTALL_GLADEUI_DIR "${CMAKE_INSTALL_PREFIX}/share/${EXEC}/glade")
ENDIF(NOT DEFINED CMAKE_INSTALL_GLADEUI_DIR)
MESSAGE ("Using CMAKE_INSTALL_GLADEUI_DIR=${CMAKE_INSTALL_GLADEUI_DIR}")
################################################################################

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
SET (CPACK_PACKAGE_VERSION_PATCH 8)
SET (CPACK_GENERATOR TGZ)
SET (CPACK_SOURCE_PACKAGE_FILE_NAME
	"${EXEC}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
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

ADD_CUSTOM_TARGET (deb checkinstall -D --pkgname="${EXEC}"
									   --pkgversion="${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}"
									   --install=0
									   --deldoc=1
									   --backup=0
									   --pkglicense="GPLv2"
									   --maintainer="bertoldia@gmail.com"
									   --pkggroup="GNOME Desktop Environment"
									   --requires="libpanel-applet2-0, libglade2-0")

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
									   src/${EXEC}
									   src/cmake_install.cmake
									   src/CMakeFiles
									   src/Makefile)
 
