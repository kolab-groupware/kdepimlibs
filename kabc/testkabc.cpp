#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kstandarddirs.h>

#include <qimage.h>

#include "geo.h"
#include "secrecy.h"
#include "stdaddressbook.h"
#include "timezone.h"
#include "key.h"
#include "agent.h"
#include "vcardconverter.h"

using namespace KABC;

int main(int argc,char **argv)
{
    KAboutData aboutData("testkabc",I18N_NOOP("TestKabc"),"0.1");
    KCmdLineArgs::init(argc, argv, &aboutData);

    KApplication app( false, false );
    AddressBook *ab = StdAddressBook::self();

//#define READ

#ifdef READ
    AddressBook::Iterator it;
    for ( it = ab->begin(); it != ab->end(); ++it ) {
      Agent agent = (*it).agent();
      if ( agent.isIntern() )
        kdDebug() << "Agent=" << agent.addressee()->realName() << endl;
    }
#else
    Addressee addr;

    addr.setGivenName("Tobias");
    addr.setFamilyName("Koenig");

    ab->insertAddressee( addr );

    StdAddressBook::save();
#endif

    return 0;
}
