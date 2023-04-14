QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

QMAKE_CXXFLAGS += -Werror=return-type

INCLUDEPATH += ./ \
    ./project \
    ./script \
    ./editor \
    ./editor/script \
    ./editor/ui \
    ./editor/device \    
    ./editor/driver \
    ./3rd/qtpropertybrowser

SOURCES += \
    $$files(./project/*.cpp) \
    $$files(./script/*.cpp) \
    $$files(./editor/*.cpp) \
    $$files(./editor/script/*.cpp) \
    $$files(./editor/ui/*.cpp) \
    $$files(./editor/device/*.cpp) \    
    $$files(./editor/driver/*.cpp) \
    $$files(./3rd/qtpropertybrowser/*.cpp) \
    $$files(./tests/*.cpp)

HEADERS += \
    $$files(./project/*.h) \
    $$files(./script/*.h) \
    $$files(./editor/*.h) \
    $$files(./editor/script/*.h) \
    $$files(./editor/ui/*.h) \
    $$files(./editor/device/*.h) \    
    $$files(./editor/driver/*.h) \    
    $$files(./3rd/qtpropertybrowser/*.h) \
    $$files(./tests/*.h)

FORMS += 
