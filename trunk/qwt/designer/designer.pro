################################################################
# Qwt Widget Library
# Copyright (C) 1997   Josef Wilgen
# Copyright (C) 2002   Uwe Rathmann
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the Qwt License, Version 1.0
################################################################

QWT_ROOT = $${PWD}/..

include ( $${QWT_ROOT}/qwtconfig.pri )
include ( $${QWT_ROOT}/qwtbuild.pri )
include ( $${QWT_ROOT}/qwtfunctions.pri )


CONFIG(debug_and_release, debug|release|debug_and_release) {

    # When building debug_and_release the designer plugin is built
    # for release only. If you want to have a debug version it has to been
    # done with "CONFIG += debug" only.

    CONFIG(debug, debug|release) {

        message("debug_and_release: building the Qwt designer plugin in release mode only")
        QWT_CONFIG -= QwtDesigner
    }
}

contains(QWT_CONFIG, QwtDesigner) {

    CONFIG    += qt plugin 
    CONFIG    += warn_on

    greaterThan(QT_MAJOR_VERSION, 4) {

        QT += designer
    }
    else {

        CONFIG    += designer
    }


    TEMPLATE        = lib
    TARGET          = qwt_designer_plugin

    DESTDIR         = plugins/designer

    INCLUDEPATH    += $${QWT_ROOT}/src 
    DEPENDPATH     += $${QWT_ROOT}/src 

    # compile the path for finding the Qwt library
    # into the plugin. Not supported on Windows !

    QMAKE_RPATHDIR *= $${QWT_INSTALL_PREFIX}/lib

    contains(QWT_CONFIG, QwtFramework) {

        LIBS      += -F$${QWT_ROOT}/lib 
    }
    else {

        LIBS      += -L$${QWT_ROOT}/lib
    }

    qwtAddLibrary(qwt)

    contains(QWT_CONFIG, QwtDll) {

        win32 {
            DEFINES += QT_DLL QWT_DLL
        }
    }

    !contains(QWT_CONFIG, QwtPlot) {
        DEFINES += NO_QWT_PLOT
    }

    !contains(QWT_CONFIG, QwtWidgets) {
        DEFINES += NO_QWT_WIDGETS
    }

    HEADERS += qwt_designer_plugin.h
    SOURCES += qwt_designer_plugin.cpp

    contains(QWT_CONFIG, QwtPlot) {

        HEADERS += qwt_designer_plotdialog.h
        SOURCES += qwt_designer_plotdialog.cpp
    }

    RESOURCES += qwt_designer_plugin.qrc

    target.path = $${QWT_INSTALL_PLUGINS}
    INSTALLS += target
}
else {
    TEMPLATE        = subdirs # do nothing
}
