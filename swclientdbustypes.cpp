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

#include "swclientdbustypes.h"


QDBusArgument &operator<<(QDBusArgument &arg, const SwItemStruct &item)
{
    arg.beginStructure();
    arg << item.serviceName << item.uuid << item.date << item.props;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, SwItemStruct &item)
{
    arg.beginStructure();
    arg >> item.serviceName >> item.uuid >> item.date >> item.props;
    arg.endStructure();
    return arg;
}

QDBusArgument &operator<<(QDBusArgument &arg, const SwItemId &itemID)
{
    arg.beginStructure();
    arg << itemID.serviceName << itemID.uuid;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, SwItemId &itemID)
{
    arg.beginStructure();
    arg >> itemID.serviceName >> itemID.uuid;
    arg.endStructure();
    return arg;
}

QDBusArgument &operator<<(QDBusArgument &arg, const SwCollectionDetails &coll)
{
    arg.beginStructure();
    arg << coll.collID << coll.collName << coll.collParentID
        << coll.collMediaTypes << coll.collItemCount << coll.collAttributes;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, SwCollectionDetails &coll)
{
    arg.beginStructure();
    arg >> coll.collID >> coll.collName >> coll.collParentID
        >> coll.collMediaTypes >> coll.collItemCount >> coll.collAttributes;
    arg.endStructure();
    return arg;
}
