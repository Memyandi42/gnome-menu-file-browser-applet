# About File Browser Applet #
File Browser Applet is an applet for the GNOME Panel. It lets you browse
and open files in your computer directly from the panel, without having to
open a file manager. It is _not_ intended as replacement to proper file
managers like nautilus. Sometimes you just want to open a file and don't
want to have to open a file manager, browse to and open the file, then close
the file manager. This Applet just saves you couple of clicks.

**NOTE: As of version 0.6.7 the source repo has moved to http://gitorious.org/gnome-file-browser-applet.**

# Version 0.6.7 #
  * Fixed [bug #84](https://code.google.com/p/gnome-menu-file-browser-applet/issues/detail?id=84) (No Icons shown under Ubuntu 10.10).
  * Fixed [bug #81](https://code.google.com/p/gnome-menu-file-browser-applet/issues/detail?id=81) (Text files with executable bit are run when they shouldn't be.)
  * Fixed [bug #80](https://code.google.com/p/gnome-menu-file-browser-applet/issues/detail?id=80) (Included locale.h in main.c)

# Features #
  * Browse and open files in your computer from the panel.
  * Respects GNOME mime type options and includes mime icon in menu.
  * Middle clicking (or pressing F4) in a directory header will open a terminal in that directory.
  * Middle-clicking (or pressing F4) on file will open with the configured editor, right-clicking will execute the default action (including running binary or .desktop files).
  * Configuration options (directories and labels to show, select icon, show/hide icon, show hidden, select terminal and editor).
  * Support for .desktop files.
  * Displays message for empty directories or failure to open a directory.
  * Optional keyboard activation (Super+h).
  * Context menu with some useful actions such as open-with, new folder here, delete file/folder, create/extract archive, open/edit/run as root, etc.
  * Fast keyboard navigation via mnemonics.
  * Drag 'n Drop from applet to Nautilus or Desktop to MOVE files.
  * Opened files added to "Recent Documents" list.
  * Optionally displays a file's thumbnail if it exist.
  * Option to show folder only (i.e. hide files).
  * Useful tooltips indicating file size or folder contents count.


# Screen Shots #

![http://gnome-menu-file-browser-applet.googlecode.com/svn/web/screenshot.png](http://gnome-menu-file-browser-applet.googlecode.com/svn/web/screenshot.png)

![http://gnome-menu-file-browser-applet.googlecode.com/svn/web/preferences.png](http://gnome-menu-file-browser-applet.googlecode.com/svn/web/preferences.png)

![http://gnome-menu-file-browser-applet.googlecode.com/svn/web/context-menu.png](http://gnome-menu-file-browser-applet.googlecode.com/svn/web/context-menu.png)

# [Installing](http://code.google.com/p/gnome-menu-file-browser-applet/wiki/Installation) #

# [Using](http://code.google.com/p/gnome-menu-file-browser-applet/wiki/Using) #

# ToDo and Planned Features #
Help is welcomed for addressing any of the following:
  * Translation.
  * Drag-n-drop configuration.
  * Documentation
  * A better icon.
  * Generating distribution packages.

# Bugs #
  * [Yes!](http://code.google.com/p/gnome-menu-file-browser-applet/issues/list)
  * Plus lots of memory leaks.

Report bugs using the bug tracker in this site.

# Hacking #
Contributions are welcome, but it might take a while for them to make it into a
release as I (unfortunately) don't always have a lot of time to devote to this
project (and other lame excuses). Send ideas/code/patches via the
[bug tracker](http://code.google.com/p/gnome-menu-file-browser-applet/issues/list).