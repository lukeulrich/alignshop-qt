# QtCreator apparently defines debug_and_release regardless of the active build configuration. The following
# conditionals, separate it out into the individual debug and release scopes.
# Source: http://www.qtcentre.org/archive/index.php/t-23655.html
#
# Related info:
# o http://www.qtcentre.org/archive/index.php/t-6028.html
# o http://developer.qt.nokia.com/forums/viewthread/3972
debug_and_release {
    CONFIG -= debug_and_release
}
CONFIG(debug, debug|release) {
    CONFIG -= debug release
    CONFIG += debug
}
CONFIG(release, debug|release) {
    CONFIG -= debug release
    CONFIG += release
}

MY_OS = dummy
MY_ARCH = dummy

macx {
    MY_OS = mac
}

unix:!macx     {
    MY_OS = linux
}

unix|macx {
    HARDWARE_PLATFORM = $$system(uname -a)
    contains( HARDWARE_PLATFORM, x86_64 ) {
        MY_ARCH = 64
    } else {
        MY_ARCH = 32
    }

    # In the mkspec, the install program, install, cannot handle symlinks - so it merely copies them and thus creates
    # a duplicate file. To fix this, we simply update the install program to use the linux command cp instead.
    QMAKE_INSTALL_PROGRAM = cp -fPp
}

win32 {
    # http://stackoverflow.com/questions/356666/identifier-for-win64-configuration-in-qmake
    # http://developer.qt.nokia.com/faq/answer/how_can_i_detect_in_the_.pro_file_if_i_am_compiling_for_a_32_bit_or_a_64_bi
    MY_OS = win
    contains(QMAKE_TARGET.arch, x86_64) {
        MY_ARCH = 64
    } else {
        MY_ARCH = 32
    }

    QMAKE_COPY = xcopy /Y /Q
}


# Each project will have separate directories for their objs, mocs, and ui
OBJECTS_DIR = objs
MOC_DIR = moc
UI_DIR = ui
RCC_DIR = rcc
