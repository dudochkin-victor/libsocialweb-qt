/*
 * libsocialweb-qt - Qt4 API for libsocialweb client DBUS functionality
 *
 * Copyright (c) 2010, Intel Corporation.
 *
 * Author: James Ausmus <james.ausmus@intel.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "swclientitem.h"
#include "swclientservice.h"

SwClientItem::SwClientItem(QObject *parent) :
    QObject(parent)
{
}

SwClientItem::SwClientItem(const SwClientService *service, SwItemStruct item, QObject *parent) :
    QObject(parent),
    mService(service),
    mSwItem(item),
    mItemType(SwClientItem::ItemTypeText),
    mDateTime(QDateTime::fromTime_t(mSwItem.date))
{
    //TODO - see if we can get a more robust method of determining this...
    if (mSwItem.props.contains("thumbnail"))
        mItemType = SwClientItem::ItemTypePic;
}

//Public functions

SwClientItem::ItemType SwClientItem::getItemType() const
{
    return mItemType;
}

QVariant SwClientItem::getDataByRole(int role)
{
    switch (role)
    {
    case Qt::DisplayRole:
        return QVariant::fromValue<QString>(
                (mItemType == ItemTypePic) ?
                getTitle() :
                getContent());
        break;
    case Qt::UserRole:
        return QVariant::fromValue<SwClientItem *>(this);
        break;
    default:
        return QVariant();
        break;
    }
}

SwParams SwClientItem::getSwItemProps() const
{
    return mSwItem.props;
}

bool SwClientItem::hideItem()
{
    if (!mService->hasBanishItems())
        return false;
    return mService->hideItem(mSwItem.uuid);
}

const SwClientService * SwClientItem::getService() const
{
    return mService;
}

QString SwClientItem::getServiceName() const
{
    return mSwItem.serviceName;
}

QString SwClientItem::getSwUUID() const
{
    return mSwItem.uuid;
}

QDateTime SwClientItem::getDateTime() const
{
    return mDateTime;
}

bool SwClientItem::getCached() const
{
    return (mSwItem.props.value("cached", "0") == "1");
}

//Note: vimeo doesn't give us an "authorid", so we fall back to
//"author" if "authorid" doesn't exist
QString SwClientItem::getAuthorID() const
{
    return mSwItem.props.value("authorid",
                               mSwItem.props.value("author", QString()));
}

QString SwClientItem::getLocation() const
{
    return mSwItem.props.value("location", QString());
}

QString SwClientItem::getURL() const
{
    return mSwItem.props.value("url", QString());
}

QString SwClientItem::getAuthorName() const
{
    //Fall back to authorid if author doesn't exist
    return mSwItem.props.value("author",
                               mSwItem.props.value("authorid", QString()));
}

QString SwClientItem::getID() const
{
    return mSwItem.props.value("id", QString());
}

QString SwClientItem::getAuthorIconPath() const
{
    return mSwItem.props.value("authoricon", QString());
}

/* ItemTypeText Only */
//"title" param also exists in vimeo posts, whereas "content" does not,
//so if we don't have a "content" param, try to return "title" instead.
QString SwClientItem::getContent() const
{
    return mSwItem.props.value("content", getTitle());
}

/* ItemTypePic Only*/
//"title" param also exists in vimeo posts, whereas "content" does not...
QString SwClientItem::getTitle() const
{
    return mSwItem.props.value("title", QString());
}

QString SwClientItem::getThumbnailPath() const
{
   return mSwItem.props.value("thumbnail", QString());
}
