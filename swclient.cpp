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

#include "swclient.h"

#include "interfaces/sw-core_interface.h"
#include "interfaces/sw-avatar_interface.h"
#include "interfaces/sw-item-view_interface.h"
#include "interfaces/sw-photo-upload_interface.h"
#include "interfaces/sw-query_interface.h"
#include "interfaces/sw-service_interface.h"
#include "interfaces/sw-status-update_interface.h"

#define SWCLIENTPATH "/com/meego/libsocialweb"

SwClient::SwClient(const QDBusConnection &connection, QObject *parent) :
        QObject(),
        mHasServices(false),
        mOnline(false)
{
    registerDataTypes();
    mCoreIf = new ComMeegoLibsocialwebInterface(this->GetSwServiceName(), SWCLIENTPATH, connection, parent);
    connect(mCoreIf,
            SIGNAL(OnlineChanged(bool)),
            this,
            SIGNAL(onlineChanged(bool)));

    connect(mCoreIf,
            SIGNAL(OnlineChanged(bool)),
            this,
            SIGNAL(OnlineChanged(bool)));

    connect(mCoreIf,
            SIGNAL(OnlineChanged(bool)),
            this,
            SLOT(onIsOnlineChanged(bool)));

    QDBusPendingCall asyncServices = mCoreIf->GetServices();
    QDBusPendingCallWatcher *watcherServices =
            new QDBusPendingCallWatcher(asyncServices, this);
    connect(watcherServices,
            SIGNAL(finished(QDBusPendingCallWatcher*)),
            this,
            SLOT(onGetServicesReply(QDBusPendingCallWatcher*)));

    QDBusPendingCall asyncOnline = mCoreIf->IsOnline();
    QDBusPendingCallWatcher *watcherOnline = new QDBusPendingCallWatcher(asyncOnline, this);
    connect(watcherOnline,
            SIGNAL(finished(QDBusPendingCallWatcher*)),
            this,
            SLOT(onIsOnlineReply(QDBusPendingCallWatcher*)));
}

//Static functions

QString SwClient::GetSwServiceName()
{
    return ComMeegoLibsocialwebInterface::staticInterfaceName();
}

//Public functions

QStringList SwClient::getServices()
{

    //If we've already gotten our services list, then just return them
    //Otherwise, we force a sync call to grab services
    if (mHasServices)
        return this->mServices;

    QDBusPendingReply<QStringList> reply = mCoreIf->GetServices();
    QDBusPendingCallWatcher *watcher =
            new QDBusPendingCallWatcher(reply, this);
    watcher->waitForFinished();
    this->onGetServicesReply(watcher);

    return this->mServices;

}

SwClientService * SwClient::getService(QString serviceName)
{
    if (!mHasServices)
        getServices();
    if (!mServices.contains(serviceName))
        return 0;
    return new SwClientService(serviceName);
}

bool SwClient::isOnline()
{
    return mOnline;
}

//Private slots

void SwClient::onGetServicesReply(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<QStringList> reply = *call;
    if (reply.isError()) {
        qDebug("GetServices call reply error:");
        qDebug() << reply.error().message();
    } else {
        mServices = QStringList(reply.value());
        mHasServices = true;
    }
}

void SwClient::onIsOnlineReply(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<bool> reply = *call;
    if (reply.isError()) {
        qDebug("IsOnline call reply error:");
        qDebug() << reply.error().message();
    } else {
        mOnline = reply.value();
    }
}

void SwClient::onIsOnlineChanged(bool online)
{
    mOnline = online;
}

