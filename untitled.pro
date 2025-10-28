#-------------------------------------------------
#
# Project created by QtCreator 2025-08-22T15:25:39
#
#-------------------------------------------------

QT += core widgets sql printsupport charts


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = untitled
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        mainwindow.cpp \
    database.cpp \
    loginwindow.cpp \
    registerwindow.cpp \
    billingwidget.cpp \
    productwidget.cpp \
    warehousewidget.cpp \
    permissionwidget.cpp \
    recordwidget.cpp \
    analysiswidget.cpp \
    usermanager.cpp \
    personalcenterwidget.cpp \
    warehousedatabase.cpp \
    permissiondatabase.cpp \
    currentusermanager.cpp \
    productdatabase.cpp \
    recorddatabase.cpp \
    billingdatabase.cpp \
    analysisdatabase.cpp

HEADERS  += mainwindow.h \
    database.h \
    loginwindow.h \
    registerwindow.h \
    billingwidget.h \
    productwidget.h \
    warehousewidget.h \
    permissionwidget.h \
    recordwidget.h \
    analysiswidget.h \
    usermanager.h \
    personalcenterwidget.h \
    warehousedatabase.h \
    permissiondatabase.h \
    currentusermanager.h \
    productdatabase.h \
    recorddatabase.h \
    billingdatabase.h \
    analysisdatabase.h

FORMS    += mainwindow.ui
