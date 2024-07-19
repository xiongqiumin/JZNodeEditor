QT       += core gui network uitools testlib designer

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

QMAKE_CXXFLAGS += -Werror=return-type -Werror=maybe-uninitialized -Werror=shadow

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
    $$files(./widgets/*.h) \
    $$files(./project/*.h) \
    $$files(./script/*.h) \
    $$files(./script/angelscript/*.h) \
    $$files(./editor/*.h) \
    $$files(./editor/script/*.h) \
    $$files(./editor/ui/*.h) \
    $$files(./sample/*.h,true) \
    $$files(./3rd/jznet/*.h) \
    $$files(./tests/*.h)

SOURCES += \
    $$files(./*.cpp) \
    $$files(./widgets/*.cpp) \
    $$files(./project/*.cpp) \
    $$files(./script/*.cpp) \
    $$files(./script/angelscript/*.cpp) \
    $$files(./editor/*.cpp) \
    $$files(./editor/script/*.cpp) \
    $$files(./editor/ui/*.cpp) \
    $$files(./sample/*.cpp,true) \
    $$files(./3rd/qtpropertybrowser/*.cpp) \
    $$files(./3rd/jznet/*.cpp) \
    $$files(./tests/*.cpp)

FORMS += $$files(./editor/*.ui) \
    $$files(./editor/script/*.ui)

RESOURCES = JZNodeEditor.qrc
