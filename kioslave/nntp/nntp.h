
#ifndef _NNTP_H
#define _NNTP_H "$Id$"

#include <sys/types.h>
#include <sys/time.h>

#include <stdio.h>

#include <qstring.h>

#include <kio/slavebase.h>

class NNTPProtocol : public KIO::SlaveBase
{
public:
  NNTPProtocol (const QCString &pool, const QCString &app );
  virtual ~NNTPProtocol();

  virtual void get( const KURL&, bool reload );
  virtual void stat( const KURL& url );
  virtual void del( const KURL& url, bool isfile);
  virtual void listDir( const KURL& url );
  
 protected:

  /**
    * This returns the size of a message as a long integer.
    * This is useful as an internal member, because the "other"
    * getSize command will emit a signal, which would be harder
    * to trap when doing something like listing a directory.
    */
  size_t realGetSize(unsigned int msg_num);

  /**
    *  Send a command to the server, and wait for the one-line-status
    *  reply via getResponse.  Similar rules apply.  If no buffer is
    *  specified, no data is passed back.
    */
  bool command (const char *buf, char *r_buf=0, unsigned int r_len=0);

  /**
    *  All NNTP commands will generate a response.  Each response will
    *  either be prefixed with a "+OK " or a "-ERR ".  The getResponse 
    *  fucntion will wait until there's data to be read, and then read in
    *  the first line (the response), and copy the response sans +OK/-ERR
    *  into a buffer (up to len bytes) if one was passed to it.  It will
    *  return true if the response was affirmitave, or false otherwise.
    */
  bool getResponse (char *buf=0, unsigned int len=0);

  /**
    *  Attempt to properly shut down the NNTP connection by sending
    *  "QUIT\r\n" before closing the socket.
    */
  void nntp_close ();

  /**
    * Attempt to initiate a NNTP connection via a TCP socket.  If no port
    * is passed, port 110 is assumed, if no user || password is
    * specified, the user is prompted for them.
    */
  bool nntp_open( const KURL &_url );

  int m_cmd, m_iSock;
  unsigned short int m_iOldPort;
  struct timeval m_tTimeout;
  QString m_sOldServer, m_sOldPass, m_sOldUser;
  FILE *fp;
};

#endif

