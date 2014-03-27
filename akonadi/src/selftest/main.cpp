/*
    Copyright (c) 2014 Daniel Vrátil <dvratil@redhat.com>

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

#include "selftestdialog.h"

#include <KAboutData>

#include <QApplication>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{
  QCommandLineParser parser;
  KAboutData about(QLatin1String("akonadiselftest"),
                   QLatin1String("akonadiselftest"),
                   i18n("Akonadi Self Test"),
                   QLatin1String("1.0"),
                   i18n("Checks and reports state of Akonadi server"),
                   KAboutData::License_GPL_V2,
                   i18n("(c) 2008 Volker Krause <vkrause@kde.org>"));
  about.setupCommandLine(&parser);

  QApplication app(argc, argv);
  QCoreApplication::setApplicationName(QLatin1String("akonadiselftest"));
  QCoreApplication::setApplicationVersion("1.0");
  parser.process(app);

  SelfTestDialog dlg;
  dlg.show();

  return app.exec();
}
