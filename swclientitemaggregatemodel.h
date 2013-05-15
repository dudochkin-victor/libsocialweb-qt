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

#ifndef SWCLIENTITEMAGGREGATEMODEL_H
#define SWCLIENTITEMAGGREGATEMODEL_H

#include <QAbstractListModel>
#include "swclientdbustypes.h"

class SwClientItem;
class SwClientItemView;

class SwClientItemAggregateModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit SwClientItemAggregateModel(QObject *parent = 0);

    Q_INVOKABLE int rowCount(const QModelIndex &) const;
    Q_INVOKABLE QVariant data(const QModelIndex &, int) const;

    Q_INVOKABLE void addView(SwClientItemView *view);

signals:

public slots:

protected slots:
    void onItemsAdded(QList<SwClientItem *> itemsAdded);
    void onItemsChanged(QList<SwClientItem *> itemsChanged);
    void onItemsRemoved(ArrayOfSwItemId itemsRemoved);

private:
    QList<SwClientItem *> mItemList;

};

#endif // SWCLIENTITEMAGGREGATEMODEL_H
