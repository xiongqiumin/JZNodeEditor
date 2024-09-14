QT       += core gui network uitools serialport testlib designer

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

gcc | clang{
	QMAKE_CXXFLAGS += -Werror=return-type -Werror=maybe-uninitialized
}

LIBS += -L$$[QT_INSTALL_LIBS] -lQt5DesignerComponents

INCLUDEPATH += ./ \
    ./widgets \
    ./project \
    ./script \
    ./editor \
    ./editor/script \
    ./editor/ui \
    ./3rd/jznet

HEADERS += \
    $$files(./*.h) \
    $$files(./widgets/*.h,true) \
    $$files(./project/*.h,true) \
    $$files(./script/*.h,true) \
    $$files(./editor/*.h,true) \
    $$files(./3rd/*.h,true) \
    $$files(./tests/*.h)

SOURCES += \
    $$files(./*.cpp) \
    $$files(./widgets/*.cpp,true) \
    $$files(./project/*.cpp,true) \
    $$files(./script/*.cpp,true) \
    $$files(./editor/*.cpp,true) \
    $$files(./sample/*.cpp,true) \
    $$files(./3rd/*.cpp,true) \
    $$files(./tests/*.cpp)

FORMS += $$files(./editor/*.ui,true)

RESOURCES = JZNodeEditor.qrc
