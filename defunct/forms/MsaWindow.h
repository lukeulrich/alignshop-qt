/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef MSAWINDOW_H
#define MSAWINDOW_H

#include <QtGui/QMainWindow>


// ------------------------------------------------------------------------------------------------
// Forward declarations
class QComboBox;
class QFontComboBox;
class QLabel;
class QPushButton;
class QTableView;

class LiveMsaCharCountDistribution;
class LiveSymbolString;
class Msa;
class MsaSubseqModel;
class PercentSpinBox;
class SymbolColorProvider;

namespace Ui {
    class MsaWindow;
}

class MsaWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MsaWindow(QWidget *parent = 0);
    ~MsaWindow();

    void setMsa(Msa *msa);
    MsaSubseqModel *msaSubseqModel() const;

signals:
    void closed();

protected:
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *event);

private slots:
    void fontComboBoxFontChanged(const QFont &font);
    void fontSizeComboBoxActivated(const QString &size);

    void saveSvg();

private:
    Ui::MsaWindow *ui_;

    Msa *msa_;
    MsaSubseqModel *msaSubseqModel_;

    LiveMsaCharCountDistribution *liveMsaCharCountDistribution_;
    LiveSymbolString *liveSymbolString_;
    SymbolColorProvider *symbolColorProvider_;


    // UI toolbar controls
    QPushButton *saveButton_;
    QLabel *zoomLabel_;
    PercentSpinBox *zoomSpinBox_;
    QFontComboBox *fontComboBox_;
    QComboBox *fontSizeComboBox_;
    QTableView *msaSubseqTableView_;
};

#endif // MSAWINDOW_H
