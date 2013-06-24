SOURCES += \
    main.cpp \
    MainWindow.cpp \
    ViewerWidget.cpp \
    LDrawParser.cpp \
    AuxiliaryViewUpdater.cpp \
    PickHandler.cpp \
    Utilities.cpp

HEADERS += \
    MainWindow.h \
    ViewerWidget.h \
    LDrawParser.h \
    AuxiliaryViewUpdater.h \
    PickHandler.h \
    Utilities.h

LIBS += \
    -losgQt \
    -losg \
    -losgGA \
    -losgDB \
    -losgViewer \
    -lOpenThreads \
    -losgUtil \
    -losgText \
    -L

QT += opengl

CONFIG += thread
