/*
    Copyright (c) 2006-2008 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef AKONADI_AGENTINSTANCEWIDGET_H
#define AKONADI_AGENTINSTANCEWIDGET_H

#include "akonadi_export.h"

#include <QtGui/QWidget>

namespace Akonadi {

class AgentInstance;
class AgentFilterProxyModel;

/**
 * This class provides a widget of all available agent instances.
 *
 * Since the widget is listening to the dbus for changes, the
 * widget is updated automatically as soon as a new agent instance
 * is added or removed to/from the system.
 *
 * @author Tobias Koenig <tokoe@kde.org>
 */
class AKONADI_EXPORT AgentInstanceWidget : public QWidget
{
  Q_OBJECT

  public:
    /**
     * Creates a new agent instance widget.
     *
     * @param parent The parent widget.
     */
    explicit AgentInstanceWidget( QWidget *parent = 0 );

    /**
     * Destroys the agent instance widget.
     */
    ~AgentInstanceWidget();

    /**
     * Returns the current agent instance or an invalid agent instance
     * if no agent instance is selected.
     */
    AgentInstance currentAgentInstance() const;

    /**
      Returns the agent filter proxy model, use this to filter by
      agent mimetype or capabilities.
    */
    AgentFilterProxyModel* agentFilterProxyModel() const;

  Q_SIGNALS:
    /**
     * This signal is emitted whenever the current agent instance changes.
     *
     * @param current The current agent instance.
     * @param previous The previous agent instance.
     */
    void currentChanged( const Akonadi::AgentInstance &current, const Akonadi::AgentInstance &previous );

    /**
     * This signal is emitted whenever the there is a double click on an instance.
     *
     * @param current The current agent instance.
     */
    void doubleClicked( const Akonadi::AgentInstance &current );

  private:
    class Private;
    Private* const d;

    Q_PRIVATE_SLOT( d, void currentAgentInstanceChanged( const QModelIndex&, const QModelIndex& ) )
    Q_PRIVATE_SLOT( d, void currentAgentInstanceDoubleClicked( const QModelIndex& ) )
};

}

#endif
