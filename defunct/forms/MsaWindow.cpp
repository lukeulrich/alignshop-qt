/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "MsaWindow.h"
#include "ui_MsaWindow.h"

#include <QtGui/QCloseEvent>
#include <QtGui/QComboBox>
#include <QtGui/QFontComboBox>
#include <QtGui/QFontDatabase>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QScrollBar>
#include <QtGui/QTableView>

#include "../LiveMsaCharCountDistribution.h"
#include "../LiveSymbolString.h"
#include "../SymbolColorProvider.h"
#include "../BioSymbolGroup.h"
#include "../BioSymbol.h"
#include "../models/MsaSubseqModel.h"
#include "../widgets/PercentSpinBox.h"

#include "../Msa.h"

MsaWindow::MsaWindow(QWidget *parent) :
    QMainWindow(parent), ui_(new Ui::MsaWindow), liveMsaCharCountDistribution_(0), liveSymbolString_(0), symbolColorProvider_(0)
{
    ui_->setupUi(this);

    msa_ = 0;
    msaSubseqModel_ = new MsaSubseqModel(this);

    ui_->alignmentView->setZoomMinimum(.1);
    ui_->alignmentView->setZoomMaximum(5.);

    // --------------
    // Tool bar setup
    // Right aligned controls:
    QWidget *spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui_->toolBar->addWidget(spacer);
    saveButton_ = new QPushButton();
    saveButton_->setText("Save it!");
    ui_->toolBar->addWidget(saveButton_);
    zoomLabel_ = new QLabel();
    zoomLabel_->setText("Zoom:");
    ui_->toolBar->addWidget(zoomLabel_);

    zoomSpinBox_ = new PercentSpinBox();
    zoomSpinBox_->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    zoomSpinBox_->setKeyboardTracking(false);
    zoomSpinBox_->setSuffix("%");
    zoomSpinBox_->setDecimals(5);
    zoomSpinBox_->setStepFactor(ui_->alignmentView->zoomFactor());
    zoomSpinBox_->setRange(ui_->alignmentView->zoomMinimum() * 100., ui_->alignmentView->zoomMaximum() * 100.);
    zoomSpinBox_->setValue(ui_->alignmentView->zoom() * 100.);
    ui_->toolBar->addWidget(zoomSpinBox_);

    fontComboBox_ = new QFontComboBox();
    int fontIndex = fontComboBox_->findText(ui_->alignmentView->font().family());
    if (fontIndex != -1)
        fontComboBox_->setCurrentIndex(fontIndex);
    ui_->toolBar->addWidget(fontComboBox_);

    fontSizeComboBox_ = new QComboBox();
    QStringList fontSizes;
    foreach (int size, QFontDatabase::standardSizes())
        fontSizes << QString::number(size);
    fontSizeComboBox_->insertItems(0, fontSizes);
    fontSizeComboBox_->setEditable(true);
    // Select the current size
    int fontSizeIndex = fontSizes.indexOf(QString::number(QApplication::font().pointSize()));
    if (fontSizeIndex == -1)
    {
        fontSizeComboBox_->insertItem(0, QString::number(QApplication::font().pointSize()));
        fontSizeIndex = 0;
    }
    fontSizeComboBox_->setCurrentIndex(fontSizeIndex);
    ui_->toolBar->addWidget(fontSizeComboBox_);

    msaSubseqTableView_ = new QTableView();
    msaSubseqTableView_->setModel(msaSubseqModel_);
    ui_->msaSubseqDock->setWidget(msaSubseqTableView_);

    // CSS styling
    ui_->toolBar->setStyleSheet("QToolBar#toolBar { border: 0; padding: 4px; }");

    // -------------------
    // Hook up the signals
    connect(fontComboBox_, SIGNAL(currentFontChanged(QFont)), SLOT(fontComboBoxFontChanged(QFont)));
    connect(fontSizeComboBox_, SIGNAL(activated(QString)), SLOT(fontSizeComboBoxActivated(QString)));

    connect(saveButton_, SIGNAL(clicked()), SLOT(saveSvg()));

    connect(zoomSpinBox_, SIGNAL(decimalPercentChanged(double)), ui_->alignmentView, SLOT(setZoom(double)));
    connect(ui_->alignmentView, SIGNAL(zoomChanged(double)), zoomSpinBox_, SLOT(setDecimalPercent(double)));

    // To respond to user events (esp keyboard)
    ui_->alignmentView->viewport()->setFocus();
}

void MsaWindow::saveSvg()
{
    ui_->alignmentView->saveSvg("msa.svg", ui_->alignmentView->selection());
}

MsaWindow::~MsaWindow()
{
    delete ui_;

    if (msa_)
    {
        msaSubseqModel_->setMsa(0);
        delete msa_;
    }
}

void MsaWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui_->retranslateUi(this);
        break;
    default:
        break;
    }
}


MsaSubseqModel *MsaWindow::msaSubseqModel() const
{
    return msaSubseqModel_;
}

void MsaWindow::setMsa(Msa *msa)
{
    Msa *oldMsa = msa_;
    msa_ = msa;
    msaSubseqModel_->setMsa(msa_);

    if (oldMsa)
    {
        delete oldMsa;
        oldMsa = 0;
    }

    if (msa_)
    {
        BioSymbolGroup clustalSymbols;
        clustalSymbols << BioSymbol('%', "WLVIMAFCYHP", .6)
                       << BioSymbol('#', "WLVIMAFCYHP", .8)
                       << BioSymbol('-', "DE", .5)
                       << BioSymbol('+', "KR", .6)
                       << BioSymbol('g', "G", .5)
                       << BioSymbol('n', "N", .5)
                       << BioSymbol('q', "QE", .5)
                       << BioSymbol('p', "P", .5)
                       << BioSymbol('t', "ST", .5)
                       << BioSymbol('A', "A", .85)
                       << BioSymbol('C', "C", .85)
                       << BioSymbol('D', "D", .85)
                       << BioSymbol('E', "E", .85)
                       << BioSymbol('F', "F", .85)
                       << BioSymbol('G', "G", .85)
                       << BioSymbol('H', "H", .85)
                       << BioSymbol('I', "I", .85)
                       << BioSymbol('K', "K", .85)
                       << BioSymbol('L', "L", .85)
                       << BioSymbol('M', "M", .85)
                       << BioSymbol('N', "N", .85)
                       << BioSymbol('P', "P", .85)
                       << BioSymbol('Q', "Q", .85)
                       << BioSymbol('R', "R", .85)
                       << BioSymbol('S', "S", .85)
                       << BioSymbol('T', "T", .85)
                       << BioSymbol('V', "V", .85)
                       << BioSymbol('W', "W", .85)
                       << BioSymbol('Y', "Y", .85);

        QColor red(.9 * 255, .2 * 255, .1 * 255);
        QColor blue(.1 * 255, .5 * 255, .9 * 255);
        QColor green(.1 * 255, .8 * 255, .1 * 255);
        QColor cyan(.1 * 255, .7 * 255, .7 * 255);
        QColor pink(.9 * 255, .5 * 255, .5 * 255);
        QColor magenta(.8 * 255, .3 * 255, .8 * 255);
        QColor yellow(.8 * 255, .8 * 255, 0);
        QColor orange(.9 * 255, .6 * 255, .3 * 255);

        SymbolColorScheme clustalColorScheme;
        clustalColorScheme.setTextColorStyle('G', TextColorStyle(Qt::black, orange));
        clustalColorScheme.setTextColorStyle('P', TextColorStyle(Qt::black, yellow));
        clustalColorScheme.setSymbolsTextColorStyle('T', "tST%#", TextColorStyle(Qt::black, green));
        clustalColorScheme.setSymbolsTextColorStyle('S', "tST#", TextColorStyle(Qt::black, green));
        clustalColorScheme.setSymbolsTextColorStyle('N', "nND", TextColorStyle(Qt::black, green));
        clustalColorScheme.setSymbolsTextColorStyle('Q', "qQE+KR", TextColorStyle(Qt::black, green));

        clustalColorScheme.setSymbolsTextColorStyle('W', "%#ACFHILMVWYPp", TextColorStyle(Qt::black, blue));
        clustalColorScheme.setSymbolsTextColorStyle('L', "%#ACFHILMVWYPp", TextColorStyle(Qt::black, blue));
        clustalColorScheme.setSymbolsTextColorStyle('V', "%#ACFHILMVWYPp", TextColorStyle(Qt::black, blue));
        clustalColorScheme.setSymbolsTextColorStyle('I', "%#ACFHILMVWYPp", TextColorStyle(Qt::black, blue));
        clustalColorScheme.setSymbolsTextColorStyle('M', "%#ACFHILMVWYPp", TextColorStyle(Qt::black, blue));
        clustalColorScheme.setSymbolsTextColorStyle('A', "%#ACFHILMVWYPpTSsG", TextColorStyle(Qt::black, blue));
        clustalColorScheme.setSymbolsTextColorStyle('F', "%#ACFHILMVWYPp", TextColorStyle(Qt::black, blue));
        clustalColorScheme.setSymbolsTextColorStyle('C', "%#ACFHILMVWYPp", TextColorStyle(Qt::black, blue));
        clustalColorScheme.setSymbolsTextColorStyle('C', "C", TextColorStyle(Qt::black, pink));
        clustalColorScheme.setSymbolsTextColorStyle('H', "%#ACFHILMVWYPp", TextColorStyle(Qt::black, cyan));
        clustalColorScheme.setSymbolsTextColorStyle('Y', "%#ACFHILMVWYPp", TextColorStyle(Qt::black, cyan));
        clustalColorScheme.setSymbolsTextColorStyle('E', "-DEqQ", TextColorStyle(Qt::black, magenta));
        clustalColorScheme.setSymbolsTextColorStyle('D', "-DEnN", TextColorStyle(Qt::black, magenta));
        clustalColorScheme.setSymbolsTextColorStyle('K', "+KRQ", TextColorStyle(Qt::black, red));
        clustalColorScheme.setSymbolsTextColorStyle('R', "+KRQ", TextColorStyle(Qt::black, red));

        if (liveMsaCharCountDistribution_)
            delete liveMsaCharCountDistribution_;
        liveMsaCharCountDistribution_ = new LiveMsaCharCountDistribution(msa_, this);
        if (liveSymbolString_)
            delete liveSymbolString_;
        liveSymbolString_ = new LiveSymbolString(liveMsaCharCountDistribution_, clustalSymbols, this);
        if (symbolColorProvider_)
            delete symbolColorProvider_;
        symbolColorProvider_ = new SymbolColorProvider(liveSymbolString_, clustalColorScheme);

        ui_->alignmentView->setPositionalMsaColorProvider(symbolColorProvider_);
    }

    // It is important that the live char count distributions are updated in response to msa changes *before* the
    // the view receives these events
    ui_->alignmentView->setMsa(msa_);
}

void MsaWindow::closeEvent(QCloseEvent *event)
{
    emit closed();
    event->accept();
}

void MsaWindow::fontComboBoxFontChanged(const QFont &font)
{
    QFont newFont(font);
    int curPointSize = fontSizeComboBox_->currentText().toInt();
    newFont.setPointSize(curPointSize);
    ui_->alignmentView->setFont(newFont);
}

void MsaWindow::fontSizeComboBoxActivated(const QString &size)
{
    QFont newFont = fontComboBox_->currentFont();
    newFont.setPointSize(size.toInt());
    ui_->alignmentView->setFont(newFont);
}
