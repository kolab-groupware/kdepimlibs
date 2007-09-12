/*
  This file is part of the kblog library.

  Copyright (c) 2007 Christian Weilbach <christian_weilbach@web.de>
  Copyright (c) 2007 Mike Arthur <mike@mikearthur.co.uk>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#ifndef KBLOG_MOVABLETYPE_H
#define KBLOG_MOVABLETYPE_H

#include <kblog/metaweblog.h>

class KUrl;

/**
  @file
  This file is part of the  for accessing Blog Servers
  and defines the MovableType class.

  @author Christian Weilbach \<christian_weilbach\@web.de\>
  @author Mike Arthur \<mike\@mikearthur.co.uk\>
*/

namespace KBlog {

    class MovableTypePrivate;
/**
  @brief
  A class that can be used for access to Movable Type blogs. Almost every
  blog server supports Movable Type.
  @code
  Blog* myblog = new MovableType("http://example.com/xmlrpc/gateway.php");
  myblog->setUsername( "some_user_id" );
  myblog->setPassword( "YoURFunnyPAsSwoRD" );
  KBlog::BlogPost *post = new BlogPost();
  post->setTitle( "This is the title." );
  post->setContent( "Here is some the content..." );
  myblog->createPost( post );
  @endcode

  @author Christian Weilbach \<christian_weilbach\@web.de\>
  @author Mike Arthur \<mike\@mikearthur.co.uk\>
*/
class KBLOG_EXPORT MovableType : public MetaWeblog
{
  Q_OBJECT
  public:
    /**
      Create an object for Movable Type 

      @param server is the url for the xmlrpc gateway.
      @param parent is the parent object.
    */
    explicit MovableType( const KUrl &server, QObject *parent = 0 );

    /**
      Destroy the object.
    */
    virtual ~MovableType();

    /**
      Create a new post on server.

      @param post is send to the server.
    */
    void createPost( KBlog::BlogPost *post );

    /**
      Fetch the Post with postId.
      @param post This is the post with its id set to the
      corresponding post on the server.

      @see BlogPost::setPostId( const QString& )
      @see fetchedPost( KBlog::BlogPost* )
    */
    void fetchPost( KBlog::BlogPost *post );

    /**
      Modify a post on server.

      @param post The post to be modified on the
      server. You need to set its id correctly.

      @see BlogPost::setPostId( const QString& )
      @see modifiedPost( KBlog::BlogPost* )
    */
    void modifyPost( KBlog::BlogPost *post );

    /**
      Returns the  of the inherited object.
    */
    QString interfaceName() const;

    /**
      List recent posts on the server.

     @param number The number of posts to fetch. Latest first.

      @see     void listedRecentPosts( const QList\<KBlog::BlogPost\>& )
    */
    void listRecentPosts( int number );

    /**
      Get the list of trackback pings from the server.

      @param post This is the post to get the trackback pings from.
      You need to set its id correctly.

      @see BlogPost::setPostId( const QString& )
      @see listedTrackBackPings( KBlog::BlogPost *, const QList\<QMap\<QString,QString\> \>& )

    */
    virtual void listTrackBackPings( KBlog::BlogPost *post );

  Q_SIGNALS:
    /**
      This signal is emitted when the trackback pings are fetched completely.

      @param post This is the post of the trackback ping list.
      @param pings This is the list itself. The map contains the keys: id, url, ip.

      @see listTrackBackPings()
    */
    void listedTrackBackPings( KBlog::BlogPost *post, const QList<QMap<QString,QString> > &pings );

  protected:
    /**
      Constructor needed for private inheritance.
    */
    MovableType( const KUrl &server, MovableTypePrivate &dd, QObject *parent = 0 );

  private:
    Q_DECLARE_PRIVATE(MovableType)
    Q_PRIVATE_SLOT( d_func(), void slotListTrackBackPings(
    const QList<QVariant>&, const QVariant& ))
};

} //namespace KBlog
#endif
