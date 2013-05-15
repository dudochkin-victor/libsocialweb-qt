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

#ifndef SWCLIENTITEMVIEW_H
#define SWCLIENTITEMVIEW_H

#include <QObject>
#include <QtDBus>

#include "swclientdbustypes.h"

class ComMeegoLibsocialwebItemViewInterface;
class ComMeegoLibsocialwebBanishableInterface;
class SwClientService;
class SwClientItem;

class SwClientItemView : public QObject
{
Q_OBJECT
public:
    explicit SwClientItemView(QObject *parent = 0);
    explicit SwClientItemView(const SwClientService *swService,
            const QString &path,
            const QDBusConnection &connection = QDBusConnection::sessionBus(),
            QObject *parent = 0);

signals:
    void ItemsAdded(QList<SwClientItem *> newItems);
    void ItemsChanged(QList<SwClientItem *> changedItems);
    void ItemsRemoved(ArrayOfSwItemId removedItems);
    void itemsAdded(QList<SwClientItem *> newItems);
    void itemsChanged(QList<SwClientItem *> changedItems);
    void itemsRemoved(ArrayOfSwItemId removedItems);

public slots:
    Q_INVOKABLE void startView();
    Q_INVOKABLE void stopView();
    Q_INVOKABLE void refreshView();
    Q_INVOKABLE void closeView();

private slots:
    void onItemsAdded(ArrayOfSwItemStruct);
    void onItemsChanged(ArrayOfSwItemStruct);
    void onItemsRemoved(ArrayOfSwItemId);

private:
    const SwClientService * mSwService;
    ComMeegoLibsocialwebItemViewInterface *mItemViewIf;
    ComMeegoLibsocialwebBanishableInterface *mBanishIf;

};

#endif // SWCLIENTITEMVIEW_H
