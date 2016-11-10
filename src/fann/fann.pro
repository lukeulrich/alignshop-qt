include(../common.pri)
CONFIG = release
CONFIG += embed_manifest_dll
QT -= core gui

TARGET = fann
TEMPLATE = lib

QMAKE_CFLAGS += -O2

unix {
    # This builds shared library output file without version number
    CONFIG += plugin

    # Copy the shared library to the lib folder
    !mac {
        QMAKE_POST_LINK = mkdir -p ../lib; cp libfann.so ../lib
    }
    mac {
        QMAKE_POST_LINK = mkdir -p ../lib; cp libfann.dylib ../lib
    }
}

win32 {
    DEFINES += FANN_LIBRARY FANN_DLL_EXPORTS NDEBUG _WINDOWS WIN32 _USRDLL _WINDLL _MBCS

    QMAKE_CFLAGS += -GL
    QMAKE_LFLAGS += /LTCG
    CONFIG += dll

    # Windows requires that implicitly linked DLLs are in the same folder or PATH as the executable
    # lib file is only needed for linking an executable - not executing it once it has been built
    # Copy the dll to the parent folder where the AlignShop executable is located
    QMAKE_POST_LINK = $$QMAKE_COPY fann.dll ..
}

INCLUDEPATH += include

SOURCES += \
    floatfann.c \
    fann_train_data.c \
    fann_train.c \
    fann_io.c \
    fann_error.c \
    fann_cascade.c \
    fann.c

HEADERS +=\
    include/fann_error.h \
    include/fann_activation.h \
    include/compat_time.h \
    include/floatfann.h \
    include/fann_train.h \
    include/fann_io.h \
    include/fann_internal.h \
    include/fann_data.h \
    include/fann_cpp.h \
    include/fann_cascade.h \
    include/fann.h \
    include/config.h
