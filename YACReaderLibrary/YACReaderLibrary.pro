######################################################################
# Automatically generated by qmake (2.01a) dom 12. oct 20:47:48 2008
######################################################################

TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .
INCLUDEPATH += ../common \
	           ./server 
CONFIG += release
CONFIG -= flat
QT += sql network

# Input
HEADERS += comic_flow.h \
           create_library_dialog.h \
           library_creator.h \
           library_window.h \
           ../common/pictureflow.h \
           add_library_dialog.h \
           ../common/custom_widgets.h \
		   rename_library_dialog.h \
		   properties_dialog.h \
		   options_dialog.h \
		   export_library_dialog.h \
		   import_library_dialog.h \
		   package_manager.h \
		   ../common/qnaturalsorting.h \
		   data_base_management.h \
		   bundle_creator.h
SOURCES += comic_flow.cpp \
           create_library_dialog.cpp \
           library_creator.cpp \
           library_window.cpp \
           main.cpp \
           ../common/pictureflow.cpp \
           add_library_dialog.cpp \
           ../common/custom_widgets.cpp \
		   rename_library_dialog.cpp \
		   properties_dialog.cpp \
		   options_dialog.cpp \
		   export_library_dialog.cpp \
		   import_library_dialog.cpp \
		   package_manager.cpp \
		   ../common/qnaturalsorting.cpp \
		   data_base_management.cpp \
		   bundle_creator.cpp
		   
include(./server/server.pri)


RESOURCES += images.qrc files.qrc
RC_FILE = icon.rc

TRANSLATIONS    = yacreaderlibrary_es.ts

Release:DESTDIR = ../release
Debug:DESTDIR = ../debug

