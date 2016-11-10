/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef MSABUILDEROPTIONSWIDGETFACTORY_H
#define MSABUILDEROPTIONSWIDGETFACTORY_H

#include <QtCore/QObject>
#include <QtCore/QVector>
#include "../../core/enums.h"
#include "../../core/global.h"

class IMsaBuilderOptionsWidget;
class KalignMsaBuilderOptionsWidget;

class MsaBuilderOptionsWidgetFactory : public QObject
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    MsaBuilderOptionsWidgetFactory(QObject *parent = nullptr);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    IMsaBuilderOptionsWidget *makeMsaBuilderOptionsWidget(const QString &msaBuilderId, const Grammar grammar) const;
    QVector<IMsaBuilderOptionsWidget *> makeMsaBuilderOptionsWidgets(const QStringList &msaBuilderIds, const Grammar grammar) const;


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    void setKalignGrammarBasedDefaultOptions(KalignMsaBuilderOptionsWidget *kalignMsaBuilderOptionsWidget, const Grammar grammar) const;
    void setKalignDnaDefaultOptions(KalignMsaBuilderOptionsWidget *kalignMsaBuilderOptionsWidget) const;
    void setKalignAminoDefaultOptions(KalignMsaBuilderOptionsWidget *kalignMsaBuilderOptionsWidget) const;
};

#endif // MSABUILDEROPTIONSWIDGETFACTORY_H
