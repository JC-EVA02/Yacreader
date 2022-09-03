TEMPLATE = app
CONFIG += console

SOURCES += \
    main.cpp \

QT += core

win32 {
    LIBS +=  -loleaut32 -lole32
    msvc {
        QMAKE_CXXFLAGS_RELEASE += /MP /Ob2 /Oi /Ot /GT
        QMAKE_LFLAGS_RELEASE += /LTCG
    }
    CONFIG -= embed_manifest_exe
}

CONFIG(7zip) {
include(../../compressed_archive/wrapper.pri)
} else:CONFIG(unarr) {
include(../../compressed_archive/unarr/unarr-wrapper.pri)
} else:CONFIG(libarchive) {
include(../../compressed_archive/libarchive/libarchive-wrapper.pri)
} else {
include(../../compressed_archive/wrapper.pri)
}
