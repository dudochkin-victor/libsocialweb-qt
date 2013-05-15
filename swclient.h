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

#ifndef SWCLIENT_H
#define SWCLIENT_H

#include <QObject>
#include <QtDBus>

#include "swclientservice.h"

class ComMeegoLibsocialwebInterface;

class SwClient : public QObject
{
Q_OBJECT
public:
    explicit SwClient(const QDBusConnection &connection = QDBusConnection::sessionBus(),
                      QObject *parent = 0);

    Q_PROPERTY(bool online READ isOnline NOTIFY OnlineChanged);
    Q_PROPERTY(QStringList services READ getServices);

    static QString GetSwServiceName();
    Q_INVOKABLE QStringList getServices();
    Q_INVOKABLE SwClientService * getService(QString serviceName);
    Q_INVOKABLE bool isOnline();

signals:
    void OnlineChanged(bool online);
    void onlineChanged(bool online);

public slots:

private slots:
    void onGetServicesReply(QDBusPendingCallWatcher *call);
    void onIsOnlineReply(QDBusPendingCallWatcher *call);
    void onIsOnlineChanged(bool online);

private:
    ComMeegoLibsocialwebInterface *mCoreIf;
    QStringList mServices;
    bool mHasServices;
    bool mOnline;

};

#endif // SWCLIENT_H
