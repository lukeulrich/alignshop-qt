/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef FLEXTABWIDGET_H
#define FLEXTABWIDGET_H

#include <QtCore/QHash>
#include <QtCore/QVector>

#include <QtGui/QTabWidget>
#include "../../core/global.h"

/**
  * FlexTabWidget extends QTabWidget by allowing the user to specify an ordered group of tabs that should be displayed
  * when a user based type (identified by an integer) is activated.
  *
  * For example, when a BLAST report entity is selected, there should be two tabs: BLAST and Notes. The BLAST tab should
  * be first and currently focused. For an amino sequence nodes, only the Notes tab should be visible. FlexTabWidget
  * permits such a configuration using integer based identification for the relevant tabs and their corresponding
  * widgets.
  *
  * If a widget has not been configured for a specific tab id, it will not be shown and a warning message displayed in
  * debug mode. All registered widgets are owned by FlexTabWidget.
  *
  * While it is possible to modify the tabs via the QTabWidget methods, using such methods as insertTab() or addTab()
  * is not advised as this may cause undefined behavior.
  */
class FlexTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    explicit FlexTabWidget(QWidget *parent = nullptr);
    ~FlexTabWidget();


    // ------------------------------------------------------------------------------------------------
    // Public methods
    int activeTabGroup() const;                                 //!< Returns the active tab group identifier
    //! Associates widget and label with the tab identified by tabId
    void registerWidget(const int tabId, QWidget *widget, const QString &label);
    void removeTabGroup(const int tabGroupId);                  //!< Removes the tab group identified by tabGroupId but does not remove any widgets associated with tabs
    void setActiveTabGroup(const int tabGroupId);               //!< Sets the active tab group to tabGroupid
    //! Sets the list of tabs that should be displayed for tabGroupId
    void setTabGroup(const int tabGroupId, const QVector<int> &tabIds);
    QVector<int> tabIds(const int tabGroupId) const;            //!< Returns the list of tab identifiers associated with tabGroupId
    QWidget *unregister(const int tabId);                       //!< Unregisters tabId and returns the associated widget (passing ownership to the caller)
    QWidget *widget(const int tabId) const;                     //!< Returns the widget associated with tabId


private:
    struct TabWidgetLabel
    {
        QWidget *widget_;
        QString label_;

        TabWidgetLabel()
            : widget_(nullptr)
        {
        }

        TabWidgetLabel(QWidget *widget, const QString &label)
            : widget_(widget),
              label_(label)
        {
        }
    };

    QHash<int, QVector<int> > groupTabIdsHash_; // { tab group id -> [ tab ids ] }
    QHash<int, TabWidgetLabel> tabIdHash_;      // { tab id -> corresponding widget and label }
    int activeTabGroupId_;
};

#endif // FLEXTABWIDGET_H
