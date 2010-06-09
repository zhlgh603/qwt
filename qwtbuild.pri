################################################################
# Qwt Widget Library
# Copyright (C) 1997   Josef Wilgen
# Copyright (C) 2002   Uwe Rathmann
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the Qwt License, Version 1.0
################################################################

######################################################################
# qmake internal options
######################################################################

CONFIG           += qt     
CONFIG           += warn_on
CONFIG           += thread
CONFIG           += no_keywords
CONFIG           += silent

######################################################################
# release/debug mode
######################################################################

win32 {
	# On Windows you can't mix release and debug libraries.
	# The designer is built in release mode. If you like to use it
	# you need a release version. For your own application development you
	# might need a debug version. 
	# Enable debug_and_release + build_all if you want to build both.

	CONFIG           += debug_and_release
	CONFIG           += build_all
}
else {

	CONFIG           += debug
}

linux-g++ {
	# CONFIG           += separate_debug_info
}

######################################################################
# paths for building qwt
######################################################################

MOC_DIR      = moc
RCC_DIR      = resources
!debug_and_release {
	OBJECTS_DIR       = obj
}
