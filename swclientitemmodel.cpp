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

#include "swclientitemmodel.h"
#include "swclientitem.h"
#include "swclientitemview.h"

SwClientItemModel::SwClientItemModel(QObject *parent) :
    QAbstractListModel(parent),
    mItemView(0)
{
}

SwClientItemModel::SwClientItemModel(SwClientItemView *itemView,
                                     QObject *parent) :
    QAbstractListModel(parent),
    mItemView(0)
{
    this->setView(itemView);
}

void SwClientItemModel::setView(SwClientItemView *itemView)
{
    mItemView = itemView;
    connect(mItemView,
            SIGNAL(ItemsAdded(QList<SwClientItem*>)),
            this,
            SLOT(onItemsAdded(QList<SwClientItem*>)));
    connect(mItemView,
            SIGNAL(ItemsChanged(QList<SwClientItem*>)),
            this,
            SLOT(onItemsChanged(QList<SwClientItem*>)));
    connect(mItemView,
            SIGNAL(ItemsRemoved(ArrayOfSwItemId)),
            this,
            SLOT(onItemsRemoved(ArrayOfSwItemId)));

    emit this->viewChanged();
    emit this->ViewChanged();
}

SwClientItemView * SwClientItemModel::getView() const
{
    return mItemView;
}

int SwClientItemModel::rowCount(const QModelIndex &parent = QModelIndex()) const
{
    Q_UNUSED(parent);
    return mItemList.size();
}

QVariant SwClientItemModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        return mItemList.at(index.row())->getDataByRole(role);
    }
    return QVariant();
}

//Private slots

void SwClientItemModel::onItemsAdded(QList<SwClientItem *>itemsAdded)
{
    emit beginInsertRows(QModelIndex(),
                         mItemList.count(),
                         mItemList.count()+itemsAdded.count()-1);
    mItemList.append(itemsAdded);
    emit endInsertRows();
}

void SwClientItemModel::onItemsChanged(QList<SwClientItem *>itemsChanged)
{
    int idx;
    foreach(SwClientItem *newItem, itemsChanged) {
        //Ugly ugly ugly!
        foreach (SwClientItem *listItem, mItemList) {
            if (newItem->getSwUUID() == listItem->getSwUUID()) {
                idx = mItemList.indexOf(listItem);
                QModelIndex qmi = createIndex(idx, 0, 0);
                mItemList.replace(idx, newItem);
                emit dataChanged(qmi, qmi);
                break;
            }
        }
    }
}

void SwClientItemModel::onItemsRemoved(ArrayOfSwItemId itemsRemoved)
{
    int idx;
    foreach(SwItemId itemID, itemsRemoved) {
        //Ugly ugly ugly!
        foreach (SwClientItem *listItem, mItemList) {
            if (listItem->getSwUUID() == itemID.uuid) {
                idx = mItemList.indexOf(listItem);
                emit beginRemoveRows(QModelIndex(), idx, idx);
                mItemList.removeAt(idx);
                emit endRemoveRows();
                break;
            }
        }
    }
}
