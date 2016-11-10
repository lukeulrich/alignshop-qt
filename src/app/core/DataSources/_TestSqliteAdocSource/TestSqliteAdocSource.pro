#-------------------------------------------------
#
# Project created by QtCreator 2011-04-28T17:25:23
#
#-------------------------------------------------

QT       -= gui
QT       += sql testlib

TARGET = TestSqliteAdocSource
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += TestSqliteAdocSource.cpp \
           ../SqliteAdocSource.cpp \
           ../../BioString.cpp \
           ../../Msa.cpp \
           ../../Seq.cpp \
           ../../UngappedSubseq.cpp \
           ../../Subseq.cpp \
           ../../misc.cpp \
           ../../constants.cpp \
           ../Crud/DbAstringCrud.cpp \
           ../Crud/DbAminoSeqCrud.cpp \
           ../Crud/DbDstringCrud.cpp \
           ../Crud/DbDnaSeqCrud.cpp \
           ../Crud/DbAminoMsaCrud.cpp \
           ../Crud/DbDnaMsaCrud.cpp \
           ../Crud/DbBlastReportCrud.cpp \
           ../AbstractDbSource.cpp \
           ../../Entities/Astring.cpp \
           ../../Entities/AminoSeq.cpp \
           ../../Entities/AbstractBasicEntity.cpp \
           ../../Entities/AbstractSeq.cpp \
           ../../Entities/AbstractMsa.cpp \
           ../../Entities/BlastReport.cpp \
           ../../Entities/DnaSeq.cpp \
           ../../AdocTreeNode.cpp

DEFINES += SRCDIR=\\\"$$PWD/\\\"
DEFINES += TESTING

LIBS += -lsqlite3
