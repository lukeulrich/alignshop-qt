/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QSet>

#include "FlexTabWidget.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QWidget *]
  */
FlexTabWidget::FlexTabWidget(QWidget *parent)
    : QTabWidget(parent),
      activeTabGroupId_(-1)
{
}

/**
  */
FlexTabWidget::~FlexTabWidget()
{
    // De-allocate unparented widgets
    QHash<int, TabWidgetLabel>::ConstIterator it = tabIdHash_.constBegin();
    for (; it != tabIdHash_.constEnd(); ++it)
    {
        // it.value() -> TabWidgetLabel
        const QWidget *widget = (it.value()).widget_;
        if (widget->parent() == nullptr)
            delete widget;
    }
    tabIdHash_.clear();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns int
  */
int FlexTabWidget::activeTabGroup() const
{
    return activeTabGroupId_;
}

/**
  * @param tabId [const int]
  * @param widget [QWidget *]
  * @param label [const QString &]
  */
void FlexTabWidget::registerWidget(const int tabId, QWidget *widget, const QString &label)
{
    if (tabIdHash_.contains(tabId))
    {
        TabWidgetLabel old = tabIdHash_.take(tabId);
        delete old.widget_;
    }

    tabIdHash_.insert(tabId, TabWidgetLabel(widget, label));
}

/**
  * @param tabGroupId [const int]
  */
void FlexTabWidget::removeTabGroup(const int tabGroupId)
{
    bool updateActiveTabGroup = (tabGroupId == activeTabGroupId_);
    groupTabIdsHash_.remove(tabGroupId);
    if (updateActiveTabGroup)
    {
        if (groupTabIdsHash_.size() > 0)
            setActiveTabGroup(groupTabIdsHash_.keys().first());
        else
            setActiveTabGroup(-1);
    }
}

/**
  * @param tabGroupId [const int]
  */
void FlexTabWidget::setActiveTabGroup(const int tabGroupId)
{
    if (tabGroupId == activeTabGroupId_)
        return;

    activeTabGroupId_ = tabGroupId;

    setUpdatesEnabled(false);
    clear();

    foreach (const int tabId, tabIds(activeTabGroupId_))
    {
        if (!tabIdHash_.contains(tabId))
            continue;

        TabWidgetLabel data = tabIdHash_.value(tabId);
        if (data.widget_ == nullptr)
            continue;

        addTab(data.widget_, data.label_);
    }

    setUpdatesEnabled(true);
}

/**
  * The vector of tabIds should not contain duplicates or otherwise only the first unique tabId will be utilized.
  *
  * @param tabGroupId [const int]
  * @param tabIds [const QVector<int> &]
  */
void FlexTabWidget::setTabGroup(const int tabGroupId, const QVector<int> &tabIds)
{
    QSet<int> observed;
    QVector<int> uniqueTabIds;
    foreach (const int tabId, tabIds)
    {
        if (observed.contains(tabId))
            continue;

        observed << tabId;
        uniqueTabIds << tabId;
    }

    groupTabIdsHash_.insert(tabGroupId, uniqueTabIds);
}

/**
  * @param tabGroupId [const int]
  * @returns QVector<int>
  */
QVector<int> FlexTabWidget::tabIds(const int tabGroupId) const
{
    return groupTabIdsHash_.value(tabGroupId);
}

/**
  * Also removes from any associated tab group ids.
  *
  * @param tabId [const int]
  */
QWidget *FlexTabWidget::unregister(const int tabId)
{
    if (!tabIdHash_.contains(tabId))
        return nullptr;

    // Check if this tab id is in the current tab group and remove the tab if it is; do not remove from the tab group
    // but merely remove from the visible set of tabs.
    QVector<int> activeTabs = groupTabIdsHash_.value(activeTabGroupId_);
    int index = activeTabs.indexOf(tabId);
    if (index != -1)
        removeTab(index);

    return tabIdHash_.take(tabId).widget_;
}

/**
  * @param tabId [const int]
  * @returns QWidget *
  */
QWidget *FlexTabWidget::widget(const int tabId) const
{
    if (!tabIdHash_.contains(tabId))
        return nullptr;

    return tabIdHash_.value(tabId).widget_;
}
