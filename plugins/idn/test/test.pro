include(../../../variables.pri)
include(../../../coverage.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = idn_test

QT      += core testlib network
QT      -= gui
LIBS    += -L../src -lidn

INCLUDEPATH += ../../interfaces
INCLUDEPATH += ../src
DEPENDPATH  += ../src

# Test sources
HEADERS += idn_test.h ../../interfaces/qlcioplugin.h
SOURCES += idn_test.cpp  ../src/idnpacketizer.cpp ../../interfaces/qlcioplugin.cpp
