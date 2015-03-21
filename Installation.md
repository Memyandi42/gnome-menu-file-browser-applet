Instructions for installing menu-file-browser-applet

**NOTE 1: From now on .Deb packages will be created and hosted [here](https://launchpad.net/~file-browser-applet-dev/+archive/ppa)**

# Dependencies #
You will need to have the binary and development files (and all their dependencies) for the following packages installed to compile this program:

  * gtk-2.0 >= 2.14
  * libpanel-applet-2.0
  * libgio-2.0

  * Some of the context menu actions require File-roller, Brasero and Rubber. You can compile and run the applet without them though.

  * Optionally requires libgtkhotkey-1.0 for keyboard activation.

You will also need the following packages installed to compile this program:
  * pkg-config
  * CMake >= 2.4.7

# Configuration #

There are 5 configuration options available:
  * CMAKE\_INSTALL\_PREFIX
  * CMAKE\_INSTALL\_LIB\_PREFIX
  * CMAKE\_INSTALL\_LIB\_EXEC\_DIR
  * CMAKE\_INSTALL\_BONOBO\_DIR
  * CMAKE\_INSTALL\_GCONF\_SCHEMA\_DIR
  * CMAKE\_INSTALL\_BUILDER\_UI\_DIR

CMAKE\_INSTALL\_PREFIX specifies the base installation prefix. The following other specific installation paths depend on this prefix:
> - CMAKE\_INSTALL\_LIB\_PREFIX
> - CMAKE\_INSTALL\_GCONF\_SCHEMA\_DIR
> - CMAKE\_INSTALL\_BUILDER\_UI\_DIR
NOTE!!! The default is /usr/local but should be /usr for the applet-loader to find the applet.

CMAKE\_INSTALL\_LIB\_PREFIX specifies the installation prefix of the binary application file. Two other specific installation paths depend on this prefix:
> - CMAKE\_INSTALL\_LIB\_EXEC\_DIR
> - CMAKE\_INSTALL\_BONOBO\_DIR
The default is CMAKE\_INSTALL\_PREFIX/lib.

CMAKE\_INSTALL\_LIB\_EXEC\_DIR specifies the installation path of the binary application file.
The default is CMAKE\_INSTALL\_LIB\_EXEC\_DIR=CMAKE\_INSTALL\_LIB\_PREFIX/file-browser-applet

CMAKE\_INSTALL\_GCONF\_SCHEMA\_DIR specifies the installation prefix of the gconf schema file.
The default is CMAKE\_INSTALL\_PREFIX/share/gconf/schemas

CMAKE\_INSTALL\_BUILDER\_UI\_DIR specifies the installation prefix of the grk+ builder ui file.
The default is CMAKE\_INSTALL\_PREFIX/share/file-browser-applet/builder

CMAKE\_INSTALL\_BONOBO\_DIR specifies the installation bonobo Server file.
The default is CMAKE\_INSTALL\_LIB\_PREFIX/bonobo/servers

"Out of source" builds are recommended. To configure using the defaults, run the following command at the project root folder:
```
mkdir build
cd build
cmake ../
```

To specify an option, run:
```
cmake -D CMAKE_OPTION=/new/option/value ../
```

# Compiling #
To compile execute:
```
make
```

# Installing/Uninstalling #
To install, with root privileges execute:
```
make install
```
Similarly, to uninstall...
```
make uninstall
```