# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl (Thu May 12 08:48:50 2011)
#
# Copyright (C) 2011  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = TestTreeNode
DEPENDPATH += . ..
INCLUDEPATH += . ..

HEADERS += TreeNode.h
SOURCES += TestTreeNode.cpp

DEFINES += TESTING
