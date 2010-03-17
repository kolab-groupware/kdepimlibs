/*
  Copyright (C) 2009 Stephen Kelly <steveire@gmail.com>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "fakeakonadiservercommand.h"

#include <QStringList>
#include "fakeserverdata.h"
#include <akonadi/entitydisplayattribute.h>

using namespace Akonadi;

FakeAkonadiServerCommand::FakeAkonadiServerCommand( FakeAkonadiServerCommand::Type type, FakeServerData *serverData )
  : m_type( type ), m_serverData( serverData ), m_model( serverData->model() )
{
  connect(this, SIGNAL(emit_itemsFetched(Akonadi::Item::List)), m_model, SLOT(itemsFetched(Akonadi::Item::List)));
  connect(this, SIGNAL(emit_collectionsFetched(Akonadi::Collection::List)), m_model, SLOT(collectionsFetched(Akonadi::Collection::List)));
  connect(this, SIGNAL(emit_collectionMoved(Akonadi::Collection,Akonadi::Collection,Akonadi::Collection)),
          m_model, SLOT(monitoredCollectionMoved(Akonadi::Collection,Akonadi::Collection,Akonadi::Collection)));
  connect(this, SIGNAL(emit_itemMoved(Akonadi::Item,Akonadi::Collection,Akonadi::Collection)),
          m_model, SLOT(monitoredItemMoved(Akonadi::Item,Akonadi::Collection,Akonadi::Collection)));
}

Collection FakeAkonadiServerCommand::getCollectionByDisplayName(const QString& displayName) const
{
  QModelIndexList list = m_model->match( m_model->index( 0, 0 ), Qt::DisplayRole, displayName, 1, Qt::MatchRecursive );
  if ( list.isEmpty() )
    return Collection();
  return list.first().data( EntityTreeModel::CollectionRole ).value<Collection>();
}

Item FakeAkonadiServerCommand::getItemByDisplayName(const QString& displayName) const
{
  QModelIndexList list = m_model->match( m_model->index( 0, 0 ), Qt::DisplayRole, displayName, 1, Qt::MatchRecursive );
  if ( list.isEmpty() )
    return Item();
  return list.first().data( EntityTreeModel::ItemRole ).value<Item>();
}

void FakeJobResponse::doCommand()
{
  if ( m_type == RespondToCollectionFetch )
  {
    emit_collectionsFetched( m_collections.values() );
  }
  else if ( m_type == RespondToItemFetch )
  {
    setProperty( "FetchCollectionId", m_parentCollection.id() );
    emit_itemsFetched( m_items.values() );
  }
}

QList<FakeJobResponse::Token> FakeJobResponse::tokenize(const QString& treeString)
{
  QStringList parts = treeString.split("-");

  QList<Token> tokens;
  const QStringList::const_iterator begin = parts.constBegin();
  const QStringList::const_iterator end = parts.constEnd();

  QStringList::const_iterator it = begin;
  ++it;
  for (; it != end; ++it)
  {
    Token token;
    if (it->trimmed().isEmpty())
    {
      token.type = Token::Branch;
    } else {
      token.type = Token::Leaf;
      token.content = it->trimmed();
    }
    tokens.append(token);
  }
  return tokens;
}

QList<FakeAkonadiServerCommand *> FakeJobResponse::interpret( FakeServerData *fakeServerData, const QString& serverData )
{
  QList<FakeAkonadiServerCommand *> list;
  QList<FakeJobResponse *> response = parseTreeString( fakeServerData, serverData );

  foreach( FakeJobResponse *command, response )
    list.append( command );
  return list;
}

QList<FakeJobResponse *> FakeJobResponse::parseTreeString( FakeServerData *fakeServerData, const QString& treeString)
{
  int depth = 0;

  QList<FakeJobResponse *> collectionResponseList;
  QHash<Collection::Id, FakeJobResponse *> itemResponseMap;

  Collection::List recentCollections;

  recentCollections.append( Collection::root() );

  QList<Token> tokens = tokenize( treeString );
  while(!tokens.isEmpty())
  {
    Token token = tokens.takeFirst();

    if (token.type == Token::Branch)
    {
      ++depth;
      continue;
    }
    Q_ASSERT(token.type == Token::Leaf);
    parseEntityString( collectionResponseList, itemResponseMap, recentCollections, fakeServerData, token.content, depth );

    depth = 0;
  }
  return collectionResponseList;
}

void FakeJobResponse::parseEntityString( QList<FakeJobResponse *> &collectionResponseList, QHash<Collection::Id, FakeJobResponse *> &itemResponseMap, Collection::List &recentCollections, FakeServerData *fakeServerData, const QString& _entityString, int depth )
{
  QString entityString = _entityString;
  if ( entityString.startsWith( "C" ) )
  {
    Collection collection;
    entityString.remove( 0, 2 );
    Q_ASSERT( entityString.startsWith( '(' ) );
    entityString.remove( 0, 1 );
    QStringList parts = entityString.split( ')' );

    if ( !parts.first().isEmpty() )
    {
      QString typesString = parts.takeFirst();

      QStringList types = typesString.split(',');
      types.replaceInStrings(" ", "");
      collection.setContentMimeTypes( types );
    } else {
      parts.removeFirst();
    }

    collection.setId( fakeServerData->nextCollectionId() );
    collection.setName( QString("Collection %1").arg( collection.id() ) );
    collection.setRemoteId( QString( "remoteId %1" ).arg( collection.id() ) );

    if ( depth == 0 )
      collection.setParentCollection( Collection::root() );
    else
      collection.setParentCollection( recentCollections.at( depth ) );

    if ( recentCollections.size() == ( depth + 1) )
      recentCollections.append( collection );
    else
      recentCollections[ depth + 1 ] = collection;

    int order = 0;
    if ( !parts.first().isEmpty() )
    {
      QString displayName;
      QString optionalSection = parts.first().trimmed();
      if ( optionalSection.startsWith( QLatin1Char( '\'' ) ) )
      {
        optionalSection.remove( 0, 1 );
        QStringList optionalParts = optionalSection.split( QLatin1Char( '\'' ) );
        displayName = optionalParts.takeFirst();
        EntityDisplayAttribute *eda = new EntityDisplayAttribute();
        eda->setDisplayName( displayName );
        collection.addAttribute( eda );
        optionalSection = optionalParts.first();
      }

      QString orderString = optionalSection.trimmed();
      if ( !orderString.isEmpty() )
      {
        bool ok;
        order = orderString.toInt(&ok);
        Q_ASSERT(ok);
      }
    } else
    {
      order = 1;
    }
    while ( collectionResponseList.size() < order )
    {
      collectionResponseList.append( new FakeJobResponse( recentCollections[ depth ], FakeJobResponse::RespondToCollectionFetch, fakeServerData ) );
    }
    collectionResponseList[ order - 1 ]->appendCollection( collection );
  }
  if ( entityString.startsWith( "I" ) )
  {
    Item item;
    entityString.remove( 0, 2 );
    entityString = entityString.trimmed();
    QString type;
    int order = 0;
    int iFirstSpace = entityString.indexOf( QLatin1Char( ' ' ) );
    type = entityString.left( iFirstSpace );
    entityString.remove( 0, iFirstSpace + 1 );
    if ( iFirstSpace > 0 && !entityString.isEmpty() )
    {
      QString displayName;
      QString optionalSection = entityString;
      if ( optionalSection.startsWith( QLatin1Char( '\'' ) ) )
      {
        optionalSection.remove( 0, 1 );
        QStringList optionalParts = optionalSection.split( QLatin1Char( '\'' ) );
        displayName = optionalParts.takeFirst();
        EntityDisplayAttribute *eda = new EntityDisplayAttribute();
        eda->setDisplayName( displayName );
        item.addAttribute( eda );
        optionalSection = optionalParts.first();
      }
      QString orderString = optionalSection.trimmed();
      if ( !orderString.isEmpty() )
      {
        bool ok;
        order = orderString.toInt( &ok );
        Q_ASSERT( ok );
      }
    } else
      type = entityString;

    item.setMimeType( type );
    item.setId( fakeServerData->nextItemId() );
    item.setRemoteId( QString( "RId_%1 %2" ).arg( item.id() ).arg(type) );
    item.setParentCollection( recentCollections.at( depth ) );

    Collection::Id colId = recentCollections[ depth ].id();
    if ( !itemResponseMap.contains( colId ) )
    {
      FakeJobResponse *newResponse = new FakeJobResponse( recentCollections[ depth ], FakeJobResponse::RespondToItemFetch, fakeServerData );
      itemResponseMap.insert( colId, newResponse );
      collectionResponseList.append( newResponse );
    }
    itemResponseMap[ colId ]->appendItem( item );
  }
}

void FakeCollectionMovedCommand::doCommand()
{
  Collection collection = getCollectionByDisplayName( m_collectionName );
  Collection source = getCollectionByDisplayName( m_sourceName );
  Collection target = getCollectionByDisplayName( m_targetName );

  collection.setParentCollection( target );

  emit_collectionMoved( collection, source, target );
}

void FakeItemMovedCommand::doCommand()
{
  Item item = getItemByDisplayName( m_itemName );
  Collection source = getCollectionByDisplayName( m_sourceName );
  Collection target = getCollectionByDisplayName( m_targetName );

  item.setParentCollection( target );

  emit_itemMoved( item, source, target );
}

