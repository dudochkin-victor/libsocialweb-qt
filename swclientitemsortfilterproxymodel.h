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

#ifndef SWCLIENTITEMSORTFILTERPROXYMODEL_H
#define SWCLIENTITEMSORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class SwClientItemSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit SwClientItemSortFilterProxyModel(QObject *parent = 0);

    enum SortCriteria {
        SORT_RCVDTIME_ASC,
        SORT_RCVDTIME_DESC,
    };

    Q_ENUMS(SortCriteria)

    Q_INVOKABLE void setSort(SortCriteria sCrit);

protected:
    virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

signals:

public slots:

private:
    SortCriteria msCrit;

};

Q_DECLARE_METATYPE(SwClientItemSortFilterProxyModel *);

#endif // SWCLIENTITEMSORTFILTERPROXYMODEL_H
