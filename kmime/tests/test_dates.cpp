#include <kmime_dateformatter.h>
#include <kmime_header_parsing.h>
#include <kdebug.h>
#include <kcomponentdata.h>
using namespace KMime;


int
main()
{
  KComponentData app("# ");
  DateFormatter t;

  time_t ntime = time(0);
  kDebug()<<"Time now:"<<endl;
  kDebug()<<"\tFancy : \t"<<t.dateString(ntime)<<endl;
  t.setFormat(DateFormatter::Localized);
  kDebug()<<"\tLocalized : \t"<<t.dateString(ntime)<<endl;
  t.setFormat(DateFormatter::CTime);
  kDebug()<<"\tCTime : \t"<<t.dateString(ntime)<<endl;
  t.setFormat(DateFormatter::Iso);
  kDebug()<<"\tIso   : \t"<<t.dateString(ntime)<<endl;
  t.setFormat(DateFormatter::Rfc);
  kDebug()<<"\trfc2822 : \t"<<t.dateString(ntime)<<endl;
  QString rfcd = t.formatDate( DateFormatter::Rfc, ntime );
  KDateTime dt;
  QDateTime qdt;
  const char *str = rfcd.toLatin1();
  if ( HeaderParsing::parseDateTime( str, str + rfcd.length(), dt ) ) {
      kDebug()<<"@@@ ntime = "<<(ntime)<<", dt = "<<(dt.toTime_t())<<endl;
      qdt.setTime_t( dt.toTime_t() );
      kDebug()<<"@@@ qq = "<< qdt.toString("ddd, dd MMM yyyy hh:mm:ss") <<endl;
      kDebug()<<"@@@ rfc2822 : "<<t.formatDate( DateFormatter::Rfc, dt.toTime_t() )<<endl;
  }
  QString ddd = "Mon, 05 Aug 2002 01:57:51 -0700";
  str = ddd.toLatin1();
  if ( HeaderParsing::parseDateTime( str, str + ddd.length(), dt ) ) {
      kDebug()<<"dt = "<<(dt.toTime_t())<<endl;
      kDebug()<<"@@@ rfc2822 : "<<t.formatDate( DateFormatter::Rfc, dt.toTime_t() )<<endl;
  }

  t.setCustomFormat("MMMM dddd yyyy Z");
  kDebug()<<"\tCustom : \t"<<t.dateString(ntime)<<endl;

  ntime -= (24 * 3600 + 1);
  kDebug()<<"Time 24 hours and 1 second ago:"<<endl;
  t.setFormat( DateFormatter::Fancy );
  kDebug()<<"\tFancy : \t"<<t.dateString(ntime)<<endl;
  t.setFormat(DateFormatter::Localized);
  kDebug()<<"\tLocalized : \t"<<t.dateString(ntime)<<endl;
  t.setFormat(DateFormatter::CTime);
  kDebug()<<"\tCTime : \t"<<t.dateString(ntime)<<endl;
  t.setFormat(DateFormatter::Iso);
  kDebug()<<"\tIso   : \t"<<t.dateString(ntime)<<endl;
  t.setFormat(DateFormatter::Rfc);
  kDebug()<<"\trfc2822 : \t"<<t.dateString(ntime)<<endl;
  t.setCustomFormat("MMMM dddd Z yyyy");
  kDebug()<<"\tCustom : \t"<<t.dateString(ntime)<<endl;

  t.setFormat(DateFormatter::Fancy);
  ntime -= (24*3600 *30 + 59);
  kDebug()<<"Time 31 days and 1 minute ago:"<<endl;
  kDebug()<<"\tFancy : \t"<<t.dateString(ntime)<<endl;
  t.setFormat(DateFormatter::Localized);
  kDebug()<<"\tLocalized : \t"<<t.dateString(ntime)<<endl;
  t.setFormat(DateFormatter::CTime);
  kDebug()<<"\tCTime : \t"<<t.dateString(ntime)<<endl;
  t.setFormat(DateFormatter::Iso);
  kDebug()<<"\tIso   : \t"<<t.dateString(ntime)<<endl;
  t.setFormat(DateFormatter::Rfc);
  kDebug()<<"\trfc2822 : \t"<<t.dateString(ntime)<<endl;
  t.setCustomFormat("MMMM Z dddd yyyy");
  kDebug()<<"\tCustom : \t"<<t.dateString(ntime)<<endl;


  kDebug()<<"Static functions (dates like in the last test):"<<endl;
  kDebug()<<"\tFancy : \t"<< DateFormatter::formatDate( DateFormatter::Fancy, ntime) <<endl;
  kDebug()<<"\tLocalized : \t"<< DateFormatter::formatDate( DateFormatter::Localized, ntime) <<endl;
  kDebug()<<"\tCTime : \t"<< DateFormatter::formatDate( DateFormatter::CTime, ntime ) <<endl;
  kDebug()<<"\tIso   : \t"<< DateFormatter::formatDate( DateFormatter::Iso, ntime ) <<endl;
  kDebug()<<"\trfc2822 : \t"<< DateFormatter::formatDate( DateFormatter::Rfc, ntime ) <<endl;
  kDebug()<<"\tCustom : \t"<< DateFormatter::formatDate( DateFormatter::Custom, ntime,
							  "Z MMMM dddd yyyy") <<endl;
  t.setFormat(DateFormatter::Fancy);
  kDebug()<<"QDateTime taking: (dates as in first test)"<<endl;
  kDebug()<<"\tFancy : \t"<<t.dateString((QDateTime::currentDateTime()))<<endl;
  t.setFormat(DateFormatter::Localized);
  kDebug()<<"\tLocalized : \t"<<t.dateString(QDateTime::currentDateTime())<<endl;
  t.setFormat(DateFormatter::CTime);
  kDebug()<<"\tCTime : \t"<<t.dateString(QDateTime::currentDateTime())<<endl;
  t.setFormat(DateFormatter::Iso);
  kDebug()<<"\tIso   : \t"<<t.dateString(QDateTime::currentDateTime())<<endl;
  t.setFormat(DateFormatter::Rfc);
  kDebug()<<"\tIso   : \t"<<t.dateString(QDateTime::currentDateTime())<<endl;
  t.setCustomFormat("MMMM d dddd yyyy Z");
  kDebug()<<"\tCustom : \t"<<t.dateString(QDateTime::currentDateTime())<<endl;

}
