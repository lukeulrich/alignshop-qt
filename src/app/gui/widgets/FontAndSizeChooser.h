/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef FONTANDSIZECHOOSER_H
#define FONTANDSIZECHOOSER_H

#include <QtGui/QWidget>
#include "../../core/global.h"

class QFontComboBox;
class QComboBox;

/**
  * Combines a font combo box and font size combobox in a single control. When either the font family or point size is
  * changed, emits a single signal denoting that the font has changed.
  */
class FontAndSizeChooser : public QWidget
{
    Q_OBJECT

public:
    FontAndSizeChooser(QWidget *parent = nullptr);


    QFont currentFont() const;
    int pointSize() const;

public Q_SLOTS:
    void setCurrentFont(const QFont &font);
    void setFamily(const QString &family);
    void setPointSize(const int newPointSize);


Q_SIGNALS:
    void fontChanged(QFont font);


private Q_SLOTS:
    void onFontComboBoxFontChanged();
    void onPointSizeComboBoxCurrentChanged();


private:
    QComboBox *makePointSizeComboBox() const;
    int findIndexWithBestPointSizeMatch(const int queryPointSize) const;
    int pointSizeFromIndex(const int index) const;
    int minimumAllowedPointSize() const;
    int maximumAllowedPointSize() const;
    QStringList allowedPointSizesAsStrings() const;

    QFontComboBox *fontComboBox_;
    QComboBox *pointSizeComboBox_;
};

#endif // FONTANDSIZECHOOSER_H
