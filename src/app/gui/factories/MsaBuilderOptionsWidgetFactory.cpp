/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "MsaBuilderOptionsWidgetFactory.h"
#include "../widgets/EmptyMsaBuilderOptionsWidget.h"
#include "../widgets/ClustalWMsaBuilderOptionsWidget.h"
#include "../widgets/KalignMsaBuilderOptionsWidget.h"
#include "../../core/constants/MsaBuilderIds.h"
#include "../../core/constants/KalignConstants.h"
#include "../../core/macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
MsaBuilderOptionsWidgetFactory::MsaBuilderOptionsWidgetFactory(QObject *parent)
    : QObject(parent)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
IMsaBuilderOptionsWidget *MsaBuilderOptionsWidgetFactory::makeMsaBuilderOptionsWidget(const QString &msaBuilderId, const Grammar grammar) const
{
    using namespace constants::MsaBuilder;

    IMsaBuilderOptionsWidget *msaBuilderOptionsWidget = nullptr;
    if (msaBuilderId == kKalignBuilderId)
    {
        KalignMsaBuilderOptionsWidget *optionsWidget = new KalignMsaBuilderOptionsWidget;
        setKalignGrammarBasedDefaultOptions(optionsWidget, grammar);
        msaBuilderOptionsWidget = optionsWidget;
    }
    else if (msaBuilderId == kClustalWBuilderId)
    {
        ClustalWMsaBuilderOptionsWidget *optionsWidget = new ClustalWMsaBuilderOptionsWidget(grammar);
        msaBuilderOptionsWidget = optionsWidget;
    }
    else
    {
        msaBuilderOptionsWidget = new EmptyMsaBuilderOptionsWidget;
    }

    msaBuilderOptionsWidget->setMsaBuilderId(msaBuilderId);
    return msaBuilderOptionsWidget;
}

QVector<IMsaBuilderOptionsWidget *> MsaBuilderOptionsWidgetFactory::makeMsaBuilderOptionsWidgets(const QStringList &msaBuilderIds, const Grammar grammar) const
{
    QVector<IMsaBuilderOptionsWidget *> optionWidgets;
    foreach (const QString &msaBuilderId, msaBuilderIds)
        optionWidgets << makeMsaBuilderOptionsWidget(msaBuilderId, grammar);
    return optionWidgets;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
void MsaBuilderOptionsWidgetFactory::setKalignGrammarBasedDefaultOptions(KalignMsaBuilderOptionsWidget *kalignMsaBuilderOptionsWidget, const Grammar grammar) const
{
    ASSERT(kalignMsaBuilderOptionsWidget != nullptr);

    switch(grammar)
    {
    case eDnaGrammar:
        setKalignDnaDefaultOptions(kalignMsaBuilderOptionsWidget);
        break;
    case eAminoGrammar:
        setKalignAminoDefaultOptions(kalignMsaBuilderOptionsWidget);
        break;

    default:
        break;
    }
}

void MsaBuilderOptionsWidgetFactory::setKalignDnaDefaultOptions(KalignMsaBuilderOptionsWidget *kalignMsaBuilderOptionsWidget) const
{
    ASSERT(kalignMsaBuilderOptionsWidget != nullptr);

    using namespace constants::Kalign;
    kalignMsaBuilderOptionsWidget->setGapOpenPenalty(kDefaultDnaGapOpenPenalty);
    kalignMsaBuilderOptionsWidget->setGapExtendPenalty(kDefaultDnaGapExtendPenalty);
    kalignMsaBuilderOptionsWidget->setTerminalGapPenalty(kDefaultDnaTerminalGapPenalty);
}

void MsaBuilderOptionsWidgetFactory::setKalignAminoDefaultOptions(KalignMsaBuilderOptionsWidget *kalignMsaBuilderOptionsWidget) const
{
    ASSERT(kalignMsaBuilderOptionsWidget != nullptr);

    using namespace constants::Kalign;
    kalignMsaBuilderOptionsWidget->setGapOpenPenalty(kDefaultAminoGapOpenPenalty);
    kalignMsaBuilderOptionsWidget->setGapExtendPenalty(kDefaultAminoGapExtendPenalty);
    kalignMsaBuilderOptionsWidget->setTerminalGapPenalty(kDefaultAminoTerminalGapPenalty);
}
