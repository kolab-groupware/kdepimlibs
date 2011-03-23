/*
 * This file is part of the syndication library
 *
 * Copyright (C) 2005 Frank Osterfeld <osterfeld@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include <rss2/document.h>
#include <rss2/category.h>
#include <rss2/cloud.h>
#include <rss2/image.h>
#include <rss2/item.h>
#include <rss2/textinput.h>

#include <constants.h>
#include <documentvisitor.h>
#include <tools.h>

#include <QtXml/QDomDocument>
#include <QtCore/QList>
#include <QtCore/QSet>
#include <QtCore/QString>

namespace Syndication {
namespace RSS2 {

class Document::DocumentPrivate
{
    public:
    DocumentPrivate() : itemDescriptionIsCDATA(false),
                        itemDescriptionContainsMarkup(false),
                        itemDescGuessed(false),
                        itemTitleIsCDATA(false),
                        itemTitleContainsMarkup(false),
                        itemTitlesGuessed(false)
    {}
    mutable bool itemDescriptionIsCDATA;
    mutable bool itemDescriptionContainsMarkup;
    mutable bool itemDescGuessed;
    mutable bool itemTitleIsCDATA;
    mutable bool itemTitleContainsMarkup;
    mutable bool itemTitlesGuessed;
};

Document::Document(const QDomElement& element) : SpecificDocument(),
                                                 ElementWrapper(element),
                                                 d(new DocumentPrivate)
{
}

Document Document::fromXML(const QDomDocument& doc)
{
    QDomNode channelNode = doc.namedItem(QString::fromUtf8("rss")).namedItem(QString::fromUtf8("channel"));

    return Document(channelNode.toElement());
}

Document::Document() : SpecificDocument(), ElementWrapper(), d(new DocumentPrivate)
{
}

Document::Document(const Document& other) : SpecificDocument(other), ElementWrapper(other)
{
    d = other.d;
}

Document::~Document()
{
}

Document& Document::operator=(const Document& other)
{
    ElementWrapper::operator=(other);
    d = other.d;
    return *this;
}
bool Document::isValid() const
{
    return !isNull();
}

QString Document::title() const
{
    return extractElementTextNS(QString(), QString::fromUtf8("title"));
}

QString Document::link() const
{
    return extractElementTextNS(QString(), QString::fromUtf8("link") );
}

QString Document::description() const
{
    QString desc = extractElementTextNS(QString(), QString::fromUtf8("description"));
    return normalize(desc);
}

QString Document::language() const
{
    QString lang = extractElementTextNS(QString(),
                                        QString::fromUtf8("language"));

    if (!lang.isNull())
    {
        return lang;
    }
    else
    {
        return extractElementTextNS(
            dublinCoreNamespace(), QString::fromUtf8("language"));
    }

}

QString Document::copyright() const
{
    QString rights = extractElementTextNS(QString(),
                                          QString::fromUtf8("copyright"));
    if (!rights.isNull())
    {
        return rights;
    }
    else
    {
        // if <copyright> is not provided, use <dc:rights>
        return extractElementTextNS(dublinCoreNamespace(),
                                    QString::fromUtf8("rights"));
    }
}

QString Document::managingEditor() const
{
    return extractElementTextNS(QString(), QString::fromUtf8("managingEditor"));
}

QString Document::webMaster() const
{
    return extractElementTextNS(QString(), QString::fromUtf8("webMaster"));
}

time_t Document::pubDate() const
{
    QString str = extractElementTextNS(QString(), QString::fromUtf8("pubDate"));

    if (!str.isNull())
    {
        return parseDate(str, RFCDate);
    }
    else
    {   // if there is no pubDate, check for dc:date
        str = extractElementTextNS(dublinCoreNamespace(), QString::fromUtf8("date"));
        return parseDate(str, ISODate);
    }
}

time_t Document::lastBuildDate() const
{
    QString str = extractElementTextNS(QString(), QString::fromUtf8("lastBuildDate"));

    return parseDate(str, RFCDate);
}

QList<Category> Document::categories() const
{
    QList<Category> categories;

    QList<QDomElement> catNodes = elementsByTagNameNS(QString(),
            QString::fromUtf8("category"));
    QList<QDomElement>::ConstIterator it = catNodes.constBegin();
    for ( ; it != catNodes.constEnd(); ++it)
    {
        categories.append(Category(*it));
    }

    return categories;
}

QString Document::generator() const
{
    return extractElementTextNS(QString(), QString::fromUtf8("generator"));
}

QString Document::docs() const
{
    return extractElementTextNS(QString(), QString::fromUtf8("docs"));
}

Cloud Document::cloud() const
{
    return Cloud(firstElementByTagNameNS(QString(), QString::fromUtf8("cloud")));
}

int Document::ttl() const
{
    bool ok;
    int c;

    QString text = extractElementTextNS(QString(), QString::fromUtf8("ttl"));
    c = text.toInt(&ok);
    return ok ? c : 0;
}

Image Document::image() const
{
    return Image(firstElementByTagNameNS(QString(), QString::fromUtf8("image")));
}

TextInput Document::textInput() const
{
    TextInput ti = TextInput(firstElementByTagNameNS(QString(), QString::fromUtf8("textInput")));

    if (!ti.isNull())
        return ti;

    // Netscape's version of RSS 0.91 has textinput, not textInput
    return TextInput(firstElementByTagNameNS(QString(), QString::fromUtf8("textinput")));
}

QSet<int> Document::skipHours() const
{
    QSet<int> skipHours;
    QDomElement skipHoursNode = firstElementByTagNameNS(QString(),
            QString::fromUtf8("skipHours"));
    if (!skipHoursNode.isNull())
    {
        ElementWrapper skipHoursWrapper(skipHoursNode);
        bool ok = false;
        QList<QDomElement> hours =
                skipHoursWrapper.elementsByTagNameNS(QString(),
                QString::fromUtf8("hour"));
        QList<QDomElement>::ConstIterator it = hours.constBegin();
        for ( ; it != hours.constEnd(); ++it)
        {
            int h = (*it).text().toInt(&ok);
            if (ok)
                skipHours.insert(h);
        }
    }

    return skipHours;
}

QSet<Document::DayOfWeek> Document::skipDays() const
{
    QSet<DayOfWeek> skipDays;
    QDomElement skipDaysNode = firstElementByTagNameNS(QString(), QString::fromUtf8("skipDays"));
    if (!skipDaysNode.isNull())
    {
        ElementWrapper skipDaysWrapper(skipDaysNode);
        QHash<QString, DayOfWeek> weekDays;

        weekDays[QString::fromUtf8("Monday")] = Monday;
        weekDays[QString::fromUtf8("Tuesday")] = Tuesday;
        weekDays[QString::fromUtf8("Wednesday")] = Wednesday;
        weekDays[QString::fromUtf8("Thursday")] = Thursday;
        weekDays[QString::fromUtf8("Friday")] = Friday;
        weekDays[QString::fromUtf8("Saturday")] = Saturday;
        weekDays[QString::fromUtf8("Sunday")] = Sunday;

        QList<QDomElement> days = skipDaysWrapper.elementsByTagNameNS(QString(), QString::fromUtf8("day"));
        for (QList<QDomElement>::ConstIterator it = days.constBegin(); it != days.constEnd(); ++it)
        {
            if (weekDays.contains((*it).text()))
                skipDays.insert(weekDays[(*it).text()]);
        }
    }

    return skipDays;
}

QList<Item> Document::items() const
{
    QList<Item> items;

    QList<QDomElement> itemNodes = elementsByTagNameNS(QString(), QString::fromUtf8("item"));

    DocumentPtr doccpy(new Document(*this));

    for (QList<QDomElement>::ConstIterator it = itemNodes.constBegin(); it != itemNodes.constEnd(); ++it)
    {
        items.append(Item(*it, doccpy));
    }

    return items;
}
QList<QDomElement> Document::unhandledElements() const
{
    // TODO: do not hardcode this list here
    QList<ElementType> handled;
    handled.append(QString::fromUtf8("title"));
    handled.append(QString::fromUtf8("link"));
    handled.append(QString::fromUtf8("description"));
    handled.append(QString::fromUtf8("language"));
    handled.append(QString::fromUtf8("copyright"));
    handled.append(QString::fromUtf8("managingEditor"));
    handled.append(QString::fromUtf8("webMaster"));
    handled.append(QString::fromUtf8("pubDate"));
    handled.append(QString::fromUtf8("lastBuildDate"));
    handled.append(QString::fromUtf8("skipDays"));
    handled.append(QString::fromUtf8("skipHours"));
    handled.append(QString::fromUtf8("item"));
    handled.append(QString::fromUtf8("textinput"));
    handled.append(QString::fromUtf8("textInput"));
    handled.append(QString::fromUtf8("image"));
    handled.append(QString::fromUtf8("ttl"));
    handled.append(QString::fromUtf8("generator"));
    handled.append(QString::fromUtf8("docs"));
    handled.append(QString::fromUtf8("cloud"));
    handled.append(ElementType(QString::fromUtf8("language"), dublinCoreNamespace()));
    handled.append(ElementType(QString::fromUtf8("rights"), dublinCoreNamespace()));
    handled.append(ElementType(QString::fromUtf8("date"), dublinCoreNamespace()));

    QList<QDomElement> notHandled;

    QDomNodeList children = element().childNodes();
    for (int i = 0; i < children.size(); ++i)
    {
        QDomElement el = children.at(i).toElement();
        if (!el.isNull()
             && !handled.contains(ElementType(el.localName(), el.namespaceURI())))
        {
            notHandled.append(el);
        }
    }

    return notHandled;
}

QString Document::debugInfo() const
{
    QString info;
    info += QLatin1String("### Document: ###################\n");
    if (!title().isNull())
        info += QLatin1String("title: #") + title() + QLatin1String("#\n");
    if (!description().isNull())
        info += QLatin1String("description: #") + description() + QLatin1String("#\n");
    if (!link().isNull())
        info += QLatin1String("link: #") + link() + QLatin1String("#\n");
    if (!language().isNull())
        info += QLatin1String("language: #") + language() + QLatin1String("#\n");
    if (!copyright().isNull())
        info += QLatin1String("copyright: #") + copyright() + QLatin1String("#\n");
    if (!managingEditor().isNull())
        info += QLatin1String("managingEditor: #") + managingEditor() + QLatin1String("#\n");
    if (!webMaster().isNull())
        info += QLatin1String("webMaster: #") + webMaster() + QLatin1String("#\n");

    QString dpubdate = dateTimeToString(pubDate());
    if (!dpubdate.isNull())
        info += QLatin1String("pubDate: #") + dpubdate + QLatin1String("#\n");

    QString dlastbuilddate = dateTimeToString(lastBuildDate());
    if (!dlastbuilddate.isNull())
        info += QLatin1String("lastBuildDate: #") + dlastbuilddate + QLatin1String("#\n");

    if (!textInput().isNull())
        info += textInput().debugInfo();
    if (!cloud().isNull())
        info += cloud().debugInfo();
    if (!image().isNull())
        info += image().debugInfo();

    QList<Category> cats = categories();

    for (QList<Category>::ConstIterator it = cats.constBegin(); it != cats.constEnd(); ++it)
        info += (*it).debugInfo();
    QList<Item> litems = items();
    for (QList<Item>::ConstIterator it = litems.constBegin(); it != litems.constEnd(); ++it)
        info += (*it).debugInfo();
    info += QLatin1String("### Document end ################\n");
    return info;
}

void Document::getItemTitleFormatInfo(bool* isCDATA, bool* containsMarkup) const
{
    if (!d->itemTitlesGuessed)
    {
        QString titles;
        QList<Item> litems = items();

        if (litems.isEmpty())
        {
            d->itemTitlesGuessed = true;
            return;
        }

        QDomElement titleEl = (*litems.begin()).firstElementByTagNameNS(QString(), QString::fromUtf8("title"));
        d->itemTitleIsCDATA = titleEl.firstChild().isCDATASection();

        int nmax = litems.size() < 10 ? litems.size() : 10; // we check a maximum of 10 items
        int i = 0;

        QList<Item>::ConstIterator it = litems.constBegin();

        while (i < nmax)
        {
            titles += (*it).originalTitle();
            ++it;
            ++i;
        }

        d->itemTitleContainsMarkup = stringContainsMarkup(titles);
        d->itemTitlesGuessed = true;
    }

    if (isCDATA != 0L)
        *isCDATA = d->itemTitleIsCDATA;
    if (containsMarkup != 0L)
        *containsMarkup = d->itemTitleContainsMarkup;
}

void Document::getItemDescriptionFormatInfo(bool* isCDATA, bool* containsMarkup) const
{
    if (!d->itemDescGuessed)
    {
        QString desc;
        QList<Item> litems = items();


        if (litems.isEmpty())
        {
            d->itemDescGuessed = true;
            return;
        }

        QDomElement descEl = (*litems.begin()).firstElementByTagNameNS(QString(), QString::fromUtf8("description"));
        d->itemDescriptionIsCDATA = descEl.firstChild().isCDATASection();

        int nmax = litems.size() < 10 ? litems.size() : 10; // we check a maximum of 10 items
        int i = 0;

        QList<Item>::ConstIterator it = litems.constBegin();

        while (i < nmax)
        {
            desc += (*it).originalDescription();
            ++it;
            ++i;
        }

        d->itemDescriptionContainsMarkup = stringContainsMarkup(desc);
        d->itemDescGuessed = true;
    }

    if (isCDATA != 0L)
        *isCDATA = d->itemDescriptionIsCDATA;
    if (containsMarkup != 0L)
        *containsMarkup = d->itemDescriptionContainsMarkup;
}

bool Document::accept(DocumentVisitor* visitor)
{
    return visitor->visitRSS2Document(this);
}

} // namespace RSS2
} // namespace Syndication
