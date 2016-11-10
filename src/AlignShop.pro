# Note: By default all paths are relative to this AlignShop.pro file
include(common.pri)

debug:message(Debug build: $$MY_OS-$$MY_ARCH)
release:message(Release build: $$MY_OS-$$MY_ARCH)

TEMPLATE = subdirs
SUBDIRS = version-tool \
          fann \
          app
CONFIG += ordered
OBJECTS_DIR =           # Because the common defines objs for all projects including this one

# --------------------
# Copy the NN-Sec neural networks
nn-sec.files = app/resources/data/nn-sec-stage1.net app/resources/data/nn-sec-stage2.net
nn-sec.path = $${OUT_PWD}/data

rebase.files = app/resources/data/link_emboss_e
rebase.path = $${OUT_PWD}/data

EXT_ARCH_PATH = ../external/$${MY_OS}-$${MY_ARCH}
BLAST_PLUS_PATH = $${EXT_ARCH_PATH}/blast+
CLUSTALW_PATH = $${EXT_ARCH_PATH}/clustalw
KALIGN_PATH = $${EXT_ARCH_PATH}/kalign

unix {
    QMAKE_DISTCLEAN += -r app fann data lib

    blastplus.files = $$BLAST_PLUS_PATH/psiblast $$BLAST_PLUS_PATH/blastdbcmd $$BLAST_PLUS_PATH/makeblastdb
    clustalw.files = $$CLUSTALW_PATH/clustalw2
    kalign.files = $$KALIGN_PATH/kalign
}

win32 {
    blastplus.files = $$BLAST_PLUS_PATH/psiblast.exe $$BLAST_PLUS_PATH/blastdbcmd.exe $$BLAST_PLUS_PATH/makeblastdb.exe
    clustalw.files = $$CLUSTALW_PATH/clustalw2.exe
    kalign.files = $$KALIGN_PATH/kalign.exe
}

# --------------------
# Copy the blast+ binaries
blastplus.path = $${OUT_PWD}/tools
clustalw.path = $${OUT_PWD}/tools
kalign.path = $${OUT_PWD}/tools

license.files = app/BetaLicense.html
license.path = $${OUT_PWD}

INSTALLS += nn-sec \
            rebase \
            blastplus \
            clustalw \
            kalign \
            license


