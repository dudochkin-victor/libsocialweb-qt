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

#ifndef SWCLIENTITEM_H
#define SWCLIENTITEM_H

#include <QObject>
#include <QMetaType>
#include "swclientdbustypes.h"

class SwClientService;

class SwClientItem : public QObject
{
Q_OBJECT
public:
    explicit SwClientItem(QObject *parent = 0);
    explicit SwClientItem(const SwClientService *service, struct SwItemStruct item, QObject *parent = 0);

    enum ItemType {
        ItemTypeText,
        ItemTypePic,
        ItemTypeLast
    };

    Q_ENUMS(ItemType);

    Q_INVOKABLE ItemType getItemType() const;
    Q_INVOKABLE QVariant getDataByRole(int role);
    Q_INVOKABLE SwParams getSwItemProps() const;
    Q_INVOKABLE bool hideItem();

    Q_INVOKABLE const SwClientService * getService() const;

    Q_INVOKABLE QString getServiceName() const;
    Q_INVOKABLE QString getSwUUID() const;
    Q_INVOKABLE QDateTime getDateTime() const;
    Q_INVOKABLE bool getCached() const;
    Q_INVOKABLE QString getAuthorID() const;
    Q_INVOKABLE QString getLocation() const;
    Q_INVOKABLE QString getURL() const;
    Q_INVOKABLE QString getAuthorName() const;
    Q_INVOKABLE QString getID() const;
    Q_INVOKABLE QString getAuthorIconPath() const;

    /* ItemTypeText */
    Q_INVOKABLE QString getContent() const;

    /* ItemTypePic */
    Q_INVOKABLE QString getTitle() const;
    Q_INVOKABLE QString getThumbnailPath() const;


signals:

public slots:

private slots:

private:
    const SwClientService *mService;
    struct SwItemStruct mSwItem;
    ItemType mItemType;
    QDateTime mDateTime;

};

Q_DECLARE_METATYPE(SwClientItem *);

#endif // SWCLIENTITEM_H
