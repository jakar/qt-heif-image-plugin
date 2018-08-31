TARGET  = qheif

HEADERS += qheifhandler_p.h
SOURCES += main.cpp qheifhandler.cpp
OTHER_FILES += heif.json

warning("QtImageFormat QHeifHandler plugin is enabled. It is only valid under LGPL v3. More info at ...")

config_libheif {
    unix|win32-g++*: LIBS += -lheif
    else:win32: LIBS += libheif.lib
}

PLUGIN_TYPE = imageformats
PLUGIN_CLASS_NAME = QHeifPlugin
load(qt_plugin)
