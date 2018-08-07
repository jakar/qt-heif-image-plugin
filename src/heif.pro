TARGET  = qheif

HEADERS += qheifhandler_p.h contextwriter_p.h
SOURCES += main.cpp qheifhandler.cpp contextwriter.cpp
OTHER_FILES += heif.json

config_libheif {
    unix|win32-g++*: LIBS += -lheif
    else:win32: LIBS += libheif.lib
}

PLUGIN_TYPE = imageformats
PLUGIN_CLASS_NAME = QHeifPlugin
load(qt_plugin)
