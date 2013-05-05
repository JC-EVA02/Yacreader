######################################################################
# Automatically generated by qmake (2.01a) dom 12. oct 20:47:48 2008
######################################################################

TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .
INCLUDEPATH += ../common \
	           ./server \
			   ./db \
			   ../YACReader 

DEFINES += SERVER_RELEASE
			   
win32 {
INCLUDEPATH += ../dependencies/poppler/include
LIBS += -L../dependencies/poppler/lib -lpoppler-qt4
QMAKE_CXXFLAGS_RELEASE += /MP /Ob2 /Oi /Ot /GT /GL
QMAKE_LFLAGS_RELEASE += /LTCG
CONFIG -= embed_manifest_exe
}

unix:!macx{
INCLUDEPATH  += /usr/include/poppler/qt4
LIBS         += -L/usr/lib -lpoppler-qt4
LIBS	     += -lGLU
}

macx{
#INCLUDEPATH  += "/Volumes/Mac OS X Lion/usr/X11/include"
INCLUDEPATH  += /usr/local/include/poppler/qt4
LIBS         += -L/usr/local/lib -lpoppler-qt4
}

CONFIG += release
CONFIG -= flat
QT += sql network opengl



# Input
HEADERS += comic_flow.h \
           create_library_dialog.h \
           library_creator.h \
           library_window.h \
           add_library_dialog.h \
		   rename_library_dialog.h \
		   properties_dialog.h \
		   options_dialog.h \
		   export_library_dialog.h \
		   import_library_dialog.h \
		   package_manager.h \
		   bundle_creator.h \
		   export_comics_info_dialog.h \
		   import_comics_info_dialog.h \
		   server_config_dialog.h \
		   comic_flow_widget.h \
		   db_helper.h \
		   ./db/data_base_management.h \
		   ./db/treeitem.h \
		   ./db/treemodel.h \
		   ./db/tablemodel.h \
		   ./db/tableitem.h \
		   ./db/comic_db.h \
		   ./db/folder.h \
		   ./db/library_item.h \
		   ../YACReader/comic.h \
		   ../YACReader/bookmarks.h \
		   ../common/pictureflow.h \
           ../common/custom_widgets.h \
		   ../common/qnaturalsorting.h \
		   ../common/yacreader_flow_gl.h \
		   ../common/yacreader_global.h \
		   ../common/onstart_flow_selection_dialog.h \
    no_libraries_widget.h
		   
SOURCES += comic_flow.cpp \
           create_library_dialog.cpp \
           library_creator.cpp \
           library_window.cpp \
           main.cpp \
           add_library_dialog.cpp \
		   rename_library_dialog.cpp \
		   properties_dialog.cpp \
		   options_dialog.cpp \
		   export_library_dialog.cpp \
		   import_library_dialog.cpp \
		   package_manager.cpp \
		   bundle_creator.cpp \
		   export_comics_info_dialog.cpp \
		   import_comics_info_dialog.cpp \
		   server_config_dialog.cpp \
		   comic_flow_widget.cpp \
		   db_helper.cpp \
		   ./db/data_base_management.cpp \
		   ./db/treeitem.cpp \
		   ./db/treemodel.cpp \
		   ./db/tablemodel.cpp \
		   ./db/tableitem.cpp \
		   ./db/comic_db.cpp \
		   ./db/folder.cpp \
		   ./db/library_item.cpp \
		   ../YACReader/comic.cpp \
		   ../YACReader/bookmarks.cpp \
           ../common/pictureflow.cpp \
           ../common/custom_widgets.cpp \
		   ../common/qnaturalsorting.cpp \
		   ../common/yacreader_flow_gl.cpp \
		   ../common/onstart_flow_selection_dialog.cpp \
    no_libraries_widget.cpp
		   
include(./server/server.pri)


RESOURCES += images.qrc files.qrc
RC_FILE = icon.rc

macx {
	ICON = YACReaderLibrary.icns
}

TRANSLATIONS    = yacreaderlibrary_es.ts \ 
									yacreaderlibrary_ru.ts \
									yacreaderlibrary_pt.ts

Release:DESTDIR = ../release
Debug:DESTDIR = ../debug

