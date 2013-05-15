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

#ifndef SWCLIENTDBUSTYPES_H
#define SWCLIENTDBUSTYPES_H

#include <QMetaType>
#include <QList>
#include <QHash>
#include <QString>
#include <QtDBus>

typedef QHash<QString,QString> SwParams;


struct SwItemId {
    QString serviceName;
    QString uuid;
};

struct SwItemStruct {
    QString serviceName;
    QString uuid;
    qlonglong date;
    SwParams props;
};

struct SwCollectionDetails {
    QString  collID;            //Service-unique collection ID
    QString  collName;          //Human-readable title of the collection
    QString  collParentID;      //Empty if it's a top-level collection
    uint     collMediaTypes;    //OR of supported media types from SwCollectionMediaTypes
    int      collItemCount;     //-1 if unknown
    SwParams collAttributes;    //Additional collection attributes - per-service
};

typedef QList<SwItemStruct> ArrayOfSwItemStruct;
typedef QList<SwItemId> ArrayOfSwItemId;
typedef QList<SwCollectionDetails> ArrayOfSwCollectionDetails;
typedef QList<uint> ArrayOfUInt;


QDBusArgument &operator<<(QDBusArgument &arg, const SwItemStruct &item);
const QDBusArgument &operator>>(const QDBusArgument &arg, SwItemStruct &item);

QDBusArgument &operator<<(QDBusArgument &arg, const SwItemId &itemID);
const QDBusArgument &operator>>(const QDBusArgument &arg, SwItemId &itemID);

QDBusArgument &operator<<(QDBusArgument &arg, const SwCollectionDetails &coll);
const QDBusArgument &operator>>(const QDBusArgument &arg, SwCollectionDetails &coll);

inline void registerDataTypes() {
    qDBusRegisterMetaType<SwItemId>();
    qDBusRegisterMetaType<ArrayOfSwItemId>();
    qDBusRegisterMetaType<SwItemStruct>();
    qDBusRegisterMetaType<ArrayOfSwItemStruct>();
    qDBusRegisterMetaType<SwParams>();
    qDBusRegisterMetaType<SwCollectionDetails>();
    qDBusRegisterMetaType<ArrayOfSwCollectionDetails>();
    qDBusRegisterMetaType<ArrayOfUInt>();
}

Q_DECLARE_METATYPE(SwParams);
Q_DECLARE_METATYPE(SwItemId);
Q_DECLARE_METATYPE(ArrayOfSwItemId);
Q_DECLARE_METATYPE(SwItemStruct);
Q_DECLARE_METATYPE(ArrayOfSwItemStruct);
Q_DECLARE_METATYPE(SwCollectionDetails);
Q_DECLARE_METATYPE(ArrayOfSwCollectionDetails);
Q_DECLARE_METATYPE(ArrayOfUInt);

#endif // SWCLIENTDBUSTYPES_H
