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

#include "swclientitemview.h"
#include "swclient.h"
#include "swclientservice.h"
#include "swclientdbustypes.h"
#include "swclientitem.h"

#include "interfaces/sw-item-view_interface.h"
#include "interfaces/sw-banishable_interface.h"

SwClientItemView::SwClientItemView(QObject *parent):
    QObject(parent)
{
}

SwClientItemView::SwClientItemView(const SwClientService *swService,
                                   const QString &path,
                                   const QDBusConnection &connection,
                                   QObject *parent) :
    QObject(parent),
    mSwService(swService),
    mItemViewIf(new ComMeegoLibsocialwebItemViewInterface(
                SwClient::GetSwServiceName(),
                path,
                connection,
                parent))
{
    connect(mItemViewIf,
            SIGNAL(ItemsAdded(ArrayOfSwItemStruct)),
            this,
            SLOT(onItemsAdded(ArrayOfSwItemStruct)));
    connect(mItemViewIf,
            SIGNAL(ItemsChanged(ArrayOfSwItemStruct)),
            this,
            SLOT(onItemsChanged(ArrayOfSwItemStruct)));
    connect(mItemViewIf,
            SIGNAL(ItemsRemoved(ArrayOfSwItemId)),
            this,
            SLOT(onItemsRemoved(ArrayOfSwItemId)));
}

//Public slots

void SwClientItemView::closeView()
{
    QDBusPendingReply<> reply = mItemViewIf->Close();
    reply.waitForFinished();
    if (reply.isError()) {
        qDebug() << QString("Error closing view: ") << reply.error().message();
    }
}

void SwClientItemView::startView()
{
    QDBusPendingReply<> reply = mItemViewIf->Start();
    reply.waitForFinished();
    if (reply.isError()) {
        qDebug() << QString("Error starting view: ") << reply.error().message();
    }
}

void SwClientItemView::stopView()
{
    QDBusPendingReply<> reply = mItemViewIf->Stop();
    reply.waitForFinished();
    if (reply.isError()) {
        qDebug() << QString("Error stopping view: ") << reply.error().message();
    }
}

void SwClientItemView::refreshView()
{
    QDBusPendingReply<> reply = mItemViewIf->Refresh();
    reply.waitForFinished();
    if (reply.isError()) {
        qDebug() << QString("Error refreshing view: ") << reply.error().message();
    }
}

//Private slots

void SwClientItemView::onItemsAdded(ArrayOfSwItemStruct arrItems)
{
    QList<SwClientItem *> swItems;
    foreach (struct SwItemStruct item, arrItems) {
        swItems.append(new SwClientItem(mSwService, item, this));
    }
    emit itemsAdded(swItems);
    emit ItemsAdded(swItems);
}

void SwClientItemView::onItemsChanged(ArrayOfSwItemStruct arrItems)
{
    QList<SwClientItem *> swItems;
    foreach (struct SwItemStruct item, arrItems) {
        swItems.append(new SwClientItem(mSwService, item, this));
    }
    emit itemsChanged(swItems);
    emit ItemsChanged(swItems);
}

//No special handling right now...
void SwClientItemView::onItemsRemoved(ArrayOfSwItemId arrItemIDs)
{
    emit itemsRemoved(arrItemIDs);
    emit ItemsRemoved(arrItemIDs);
}
