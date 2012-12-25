################################################################
# Qwt Widget Library
# Copyright (C) 1997   Josef Wilgen
# Copyright (C) 2002   Uwe Rathmann
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the Qwt License, Version 1.0
################################################################

include( $${PWD}/../playground.pri )

TARGET       = timescale

HEADERS = \
    timescaledraw.h \
    panel.h \
    plot.h \
    mainwindow.h

SOURCES = \
    timescaledraw.cpp \
    panel.cpp \
    plot.cpp \
    mainwindow.cpp \
    main.cpp

