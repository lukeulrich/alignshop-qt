/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef CONSENSUSVIEW_H
#define CONSENSUSVIEW_H

#include "VerticalMsaMarginWidget.h"
#include "../../core/global.h"

class QModelIndex;

class IRenderEngine;
class MsaConsensusModel;

/**
  * Displays a series of conensus strings relative to an associated AbstractMsaView.
  */
class ConsensusView : public VerticalMsaMarginWidget
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    explicit ConsensusView(QWidget *parent = nullptr);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    IRenderEngine *renderEngine() const;
    void setConsensusModel(MsaConsensusModel *msaConsensusModel);
    void setRenderEngine(IRenderEngine *renderEngine);


public Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Public slots
    void updateHeight();
    void updateScrollBarRanges();


protected:
    // ------------------------------------------------------------------------------------------------
    // Protected
    virtual void paintEvent(QPaintEvent *paintEvent);


private Q_SLOTS:
    void onConsensusDataChanged(const QModelIndex &topLeft);
    void refresh();


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    void clearConsensusSymbolStrings();
    void initializeConsensusSymbolStrings();
    int numberOfColumns() const;
    QSize renderSize() const;


    // ------------------------------------------------------------------------------------------------
    // Private members
    IRenderEngine *renderEngine_;
    MsaConsensusModel *msaConsensusModel_;
};

#endif // CONSENSUSVIEW_H
