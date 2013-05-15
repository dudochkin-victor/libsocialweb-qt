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

#include "swclientitemsortfilterproxymodel.h"
#include "swclientitem.h"

SwClientItemSortFilterProxyModel::SwClientItemSortFilterProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent),
    msCrit(SORT_RCVDTIME_DESC)
{
}


bool SwClientItemSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    QVariant leftData = left.data(Qt::UserRole);//SwClientItem *
    QVariant rightData = right.data(Qt::UserRole);//SwClientItem *

    SwClientItem * leftItem = leftData.value<SwClientItem *>();
    SwClientItem * rightItem = rightData.value<SwClientItem *>();

    switch (msCrit) {
    case SwClientItemSortFilterProxyModel::SORT_RCVDTIME_ASC:
        return leftItem->getDateTime() < rightItem->getDateTime();
        break;
    case SwClientItemSortFilterProxyModel::SORT_RCVDTIME_DESC:
    default:
        return leftItem->getDateTime() > rightItem->getDateTime();
        break;
    }
    //GCC mistakenly throws a "control reaches end of non-void function"
    //warning without the following...
    return false;
}

void SwClientItemSortFilterProxyModel::setSort(SortCriteria sCrit)
{
    msCrit = sCrit;
}

