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
#include "swclientservice.h"
#include "swclientitem.h"
#include "swclientserviceconfig.h"

#include "interfaces/sw-service_interface.h"
#include "interfaces/sw-query_interface.h"
#include "interfaces/sw-avatar_interface.h"
#include "interfaces/sw-banishable_interface.h"
#include "interfaces/sw-photo-upload_interface.h"
#include "interfaces/sw-video-upload_interface.h"
#include "interfaces/sw-status-update_interface.h"
#include "interfaces/lastfm_interface.h"
#include "interfaces/sw-collections_interface.h"

#include <QFile>

#define SWCLIENTSERVICEPATH QString("/com/meego/libsocialweb/Service/%1")

SwClientService::SwClientService(const QDBusConnection &connection, QObject *parent) :
    QObject(parent),
    mConnection(connection)
{
    this->init();
}

SwClientService::SwClientService(const QString &serviceName,
                                 const QDBusConnection &connection,
                                 QObject *parent) :
    QObject(parent),
    mConnection(connection)
{
    this->init();
    this->setServiceName(serviceName);
}

void SwClientService::setServiceName(QString svcName)
{
    mServiceName = svcName;
    mServicePath = SWCLIENTSERVICEPATH.arg(svcName);
    mIsLastFM = (mServiceName == "lastfm");

    mServiceIf = new ComMeegoLibsocialwebServiceInterface(
            SwClient::GetSwServiceName(), mServicePath, mConnection, this);

    connect(mServiceIf,
            SIGNAL(CapabilitiesChanged(QStringList)),
            this,
            SLOT(onDynCapsChanged(QStringList)));

    connect(mServiceIf,
            SIGNAL(UserChanged()),
            this,
            SLOT(onUserChanged()));

    //Grab this synchronously, so client apps can know caps as soon as they instantiate the class...
    getStaticCaps();
    getDynCaps();
    populateInterfaces();
    //We're pre-pop'ing the ServiceConfig for now, so we
    //can use it for isConfigured() replacement for vimeo for now,
    //as vimeo doesn't implement DynCaps as of lsw 0.26.2
    getServiceConfig();
    emit this->serviceNameChanged();
    emit this->ServiceNameChanged();
}

SwClientServiceConfig * SwClientService::getServiceConfig()
{
    if (!mServiceConfig) {
        mServiceConfig = new SwClientServiceConfig(this);
        if (!mServiceConfig->isValid()) {
            mServiceConfig = 0;
            qDebug() << QString("Could not create valid SwClientServiceConfig for service %1!").arg(mServiceName);
        }
    }
    return mServiceConfig;
}

const QStringList SwClientService::getStaticCaps()
{
    if (mHaveStaticCaps)
        return mStaticCaps;

    QDBusPendingReply<QStringList> reply = mServiceIf->GetStaticCapabilities();
    reply.waitForFinished();
    if (reply.isError()) {
        qDebug() << QString("Error getting static caps: %1").arg(reply.error().message());
    } else {
        mHaveStaticCaps = true;
        mStaticCaps = reply.value();
        checkStaticCaps();
    }
    return mStaticCaps;
}

const QStringList SwClientService::getDynCaps()
{
    /* We can use this to force a refresh
       of our dyn caps...
    if (mHaveDynCaps)
        return mDynCaps;
    */

    QDBusPendingReply<QStringList> reply = mServiceIf->GetDynamicCapabilities();
    reply.waitForFinished();
    if (reply.isError()) {
        qDebug() << QString("Error getting dyn caps: %1").arg(reply.error().message());
    } else {
        mHaveDynCaps = true;
        mDynCaps = reply.value();
        checkDynCaps();
    }
    return mDynCaps;
}

const QString SwClientService::getServiceName() const
{
    return mServiceName;
}

const QString SwClientService::getDisplayName() const
{
    if (!mServiceConfig)
        return mServiceName;
    return mServiceConfig->getDisplayName();
}

const QString SwClientService::getUserAvatarPath() const
{
    if (!mUserAvatarPath.isEmpty())
        return mUserAvatarPath;

    requestAvatar();
    return QString();
}


bool SwClientService::hasOpenView() const
{
    return mHasQueryIf;
}

bool SwClientService::hasPhotoUpload() const
{
    return mHasPhotoUploadIf;
}

bool SwClientService::hasVideoUpload() const
{
    return mHasVideoUploadIf;
}

bool SwClientService::hasBanishItems() const
{
    return mHasBanishIf;
}

bool SwClientService::hasUpdateStatus() const
{
    return mHasStatusUpdateIf;
}

bool SwClientService::hasUpdateStatusWithGeotag() const
{
    return (mHasStatusUpdateIf && mCanUpdateStatusWithGeotag);
}

bool SwClientService::hasRequestAvatar() const
{
    return mHasAvatarIf;
}

bool SwClientService::hasCollections() const
{
    return mHasCollIf;
}

bool SwClientService::hasVerifyCreds() const
{
    return mCanVerifyCreds;
}

bool SwClientService::isLastFm() const
{
    return mIsLastFM;
}


//Currently available functionality:
bool SwClientService::canUpdateStatus() const
{
    return mCanUpdateStatusDyn;
}

bool SwClientService::canUpdateStatusWithGeotag() const
{
    return (mCanUpdateStatusDyn && mCanGeotagDyn);
}

bool SwClientService::canRequestAvatar() const
{
    return mCanRequestAvatarDyn;
}

//Current dynamic status
bool SwClientService::isConfigured() const
{
    //Vimeo doesn't have dyn caps implemented right
    //now, so we just check if the gconf user key is
    //set
    if (mServiceName == "vimeo") {
        //If we couldn't build a svccfg, something is badly wrong, so return false
        if (!mServiceConfig)
            return false;
        return !mServiceConfig->getConfigParams().value("user").isEmpty();
    }
    return mIsConfiguredDyn;
}

SwClientService::CredsState SwClientService::credsState() const
{
    return mCredsStateDyn;
}


//Query interface
SwClientItemView * SwClientService::openView(const QString &query, const SwParams &params) const
{
    if (!credsOK()
            || !hasOpenView()
            || !mQueryIf)
        return 0;

    QDBusPendingReply<QDBusObjectPath> reply = mQueryIf->OpenView(query, params);
    reply.waitForFinished();
    if (reply.isError()) {
        qDebug("mQueryIf->OpenView call reply error:");
        qDebug() << reply.error().message();
        return 0;
    }
    qDebug("Successful mQueryIf->OpenView call: ");
    qDebug() << reply.value().path();
    return new SwClientItemView(this, reply.value().path(), mServiceIf->connection(), parent());
}

//Photo Upload interface
int SwClientService::uploadPhoto(const QString &localFilename, const SwParams &fields) const
{
    if (!credsOK()
            || !hasPhotoUpload()
            || !mPhotoUploadIf
            || !QFile::exists(localFilename))
        return -1;

    QDBusPendingReply<int> reply = mPhotoUploadIf->UploadPhoto(localFilename, fields);
    reply.waitForFinished();
    if (reply.isError()) {
        qDebug() << QString("Error uploading photo %1 with the following fields:").arg(localFilename);
        qDebug() << fields;
        qDebug() << QString("DBUS error was: %1").arg(reply.error().message());
        return -1;
    } else {
        return reply.value();
    }
}

//Video Upload interface
int SwClientService::uploadVideo(const QString &localFilename, const SwParams &fields) const
{
    if (!credsOK()
            || !hasVideoUpload()
            || !mVideoUploadIf
            || !QFile::exists(localFilename))
        return -1;

    QDBusPendingReply<int> reply = mVideoUploadIf->UploadVideo(localFilename, fields);
    reply.waitForFinished();
    if (reply.isError()) {
        qDebug() << QString("Error uploading video %1 with the following fields:").arg(localFilename);
        qDebug() << fields;
        qDebug() << QString("DBUS error was: %1").arg(reply.error().message());
        return -1;
    } else {
        return reply.value();
    }
}

//Banishable interface
bool SwClientService::hideItem(const SwClientItem *item) const
{
    return hideItem(item->getSwUUID());
}

bool SwClientService::hideItem(const QString &swUUID) const
{
    if (!credsOK()
            || !hasBanishItems()
            || !mBanishIf)
        return false;

    QDBusPendingReply<> reply = mBanishIf->HideItem(swUUID);
    reply.waitForFinished();
    if (reply.isError()) {
        qDebug() << QString("Error hiding item %1: %2").arg(swUUID, reply.error().message());
        return false;
    } else {
        return true;
    }
}

//StatusUpdate interface
bool SwClientService::updateStatus(const QString &statusMessage, const SwParams &fields)
{
    if (!credsOK()
            || !hasUpdateStatus()
            || !canUpdateStatus()
            || !mStatusUpdateIf
            || mPendingStatusUpdate)
        return false;

    QDBusPendingReply<> reply = mStatusUpdateIf->UpdateStatus(statusMessage, fields);
    reply.waitForFinished();
    if (reply.isError()) {
        qDebug() << QString("Error updating status: %1").arg(reply.error().message());
        return false;
    } else {
        mPendingStatusUpdate = true;
        return true;
    }
}


//Avatar interface
bool SwClientService::requestAvatar() const
{
    if (!credsOK()
            || !hasRequestAvatar()
            || !canRequestAvatar()
            || !mAvatarIf)
        return false;

    QDBusPendingReply<> reply = mAvatarIf->RequestAvatar();
    reply.waitForFinished();
    if (reply.isError()) {
        qDebug() << QString("Error requesting avatar: %1").arg(reply.error().message());
        return false;
    } else {
        return true;
    }
}


//Collections interface
bool SwClientService::getCollList()
{
    if (!credsOK()
            || !hasCollections()
            || !mCollIf)
        return false;

    QDBusPendingCall async = mCollIf->GetList();
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);
    connect(watcher,
            SIGNAL(finished(QDBusPendingCallWatcher*)),
            this,
            SLOT(onGetCollListFinished(QDBusPendingCallWatcher*)));
    return true;
}

bool SwClientService::getCollDetails(const QString &collID)
{
    if (!credsOK()
            || !hasCollections()
            || !mCollIf)
        return false;

    qDebug() << QString("lsw-qt: Getting details for collID '%1'").arg(collID);
    QDBusPendingCall async = mCollIf->GetDetails(collID);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);
    connect(watcher,
            SIGNAL(finished(QDBusPendingCallWatcher*)),
            this,
            SLOT(onGetCollDetailsFinished(QDBusPendingCallWatcher*)));
    return true;
}

bool SwClientService::createColl(const SwCollectionDetails &collDetails)
{
    return this->createColl(collDetails.collName, (CollectionMediaTypes)collDetails.collMediaTypes,
                            collDetails.collParentID, collDetails.collAttributes);
}

bool SwClientService::createColl(const QString &collName, CollectionMediaTypes collMediaTypes,
                                 const QString &collParentID, SwParams collExtraParams)
{
    if (!credsOK()
            || !hasCollections()
            || !mCollIf)
        return false;

    QDBusPendingCall async = mCollIf->Create(collName, (uint)collMediaTypes, collParentID, collExtraParams);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);
    connect(watcher,
            SIGNAL(finished(QDBusPendingCallWatcher*)),
            this,
            SLOT(onCreateCollFinished(QDBusPendingCallWatcher*)));
    return true;
}

const ArrayOfUInt SwClientService::getCreatableTypes() const
{
    ArrayOfUInt emptyList;
    //In libsocialweb, it's a local function, so we don't have to care if
    //the service is configured or the creds are valid
    if (!hasCollections() || !mCollIf)
        return emptyList;

    QDBusPendingReply<ArrayOfUInt> reply = mCollIf->GetCreatableTypes();
    reply.waitForFinished();
    if (reply.isError()) {
        qDebug() << QString("Error calling Collections.GetCreatableTypes(): %1").arg(reply.error().message());
        return emptyList;
    } else {
        return reply.value();
    }

}

//Lastfm interface
bool SwClientService::nowPlaying(const QString &artist, const QString &album,
                                 const QString &track, uint length,
                                 uint tracknumber,
                                 const QString &musicbrainz) const
{
    if (!mIsLastFM
            || !credsOK()
            || !mLastFmIf)
        return false;

    QDBusPendingReply<> reply = mLastFmIf->NowPlaying(artist, album, track,
                                                      length, tracknumber,
                                                      musicbrainz);
    reply.waitForFinished();
    if (reply.isError()) {
        qDebug() << QString("Error calling Last.fm NowPlaying: %1").arg(reply.error().message());
        return false;
    } else {
        return true;
    }
}

bool SwClientService::submitTrack(const QString &artist, const QString &album,
                                  const QString &track, qlonglong time,
                                  const QString &source,
                                  const QString &rating,
                                  uint length, uint tracknumber,
                                  const QString &musicbrainz) const
{
    if (!mIsLastFM
            || !credsOK()
            || !mLastFmIf)
        return false;

    QDBusPendingReply<> reply = mLastFmIf->SubmitTrack(artist, album, track,
                                                       time, source, rating,
                                                       length, tracknumber,
                                                       musicbrainz);
    reply.waitForFinished();
    if (reply.isError()) {
        qDebug() << QString("Error calling Last.fm NowPlaying: %1").arg(reply.error().message());
        return false;
    } else {
        return true;
    }
}

//Public slots

void SwClientService::onCredentialsUpdated()
{
    mServiceIf->CredentialsUpdated();
}

//Private slots

void SwClientService::onGetStaticCapsFinished(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<QStringList> reply = *call;
    if (reply.isError()) {
        qDebug() << QString("Error getting static caps: %1").arg(reply.error().message());
    } else {
        mHaveStaticCaps = true;
        mStaticCaps = reply.value();
        checkStaticCaps();
    }
}

void SwClientService::onGetDynCapsFinished(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<QStringList> reply = *call;
    if (reply.isError()) {
        qDebug() << QString("Error getting dyn caps: %1").arg(reply.error().message());
    } else {
        mHaveDynCaps = true;
        mDynCaps = reply.value();
        checkDynCaps();
    }
}

void SwClientService::onDynCapsChanged(const QStringList &newDynCaps)
{
    mDynCaps = newDynCaps;
    checkDynCaps();
    emit dynCapsChanged(this, newDynCaps);
    emit DynCapsChanged(this, newDynCaps);
}

void SwClientService::onUserChanged()
{
    emit userChanged(this);
    emit UserChanged(this);
}

void SwClientService::onPhotoUploadProgress(int uploadID, int progress, const QString &errMessage)
{
    emit photoUploadProgress(this, uploadID, progress, errMessage);
    emit PhotoUploadProgress(this, uploadID, progress, errMessage);
}

void SwClientService::onVideoUploadProgress(int uploadID, int progress, const QString &errMessage)
{
    emit videoUploadProgress(this, uploadID, progress, errMessage);
    emit VideoUploadProgress(this, uploadID, progress, errMessage);
}

void SwClientService::onItemHidden(const QString &swUUID)
{
    emit itemHidden(this, swUUID);
    emit ItemHidden(this, swUUID);
}

void SwClientService::onStatusUpdated(bool success)
{
    mPendingStatusUpdate = false;
    emit statusUpdated(this, success);
    emit StatusUpdated(this, success);
}

void SwClientService::onAvatarRetrieved(const QString &avatarLocation)
{
    mUserAvatarPath = avatarLocation;
    emit avatarRetrieved(this, avatarLocation);
    emit AvatarRetrieved(this, avatarLocation);
}

void SwClientService::onGetCollListFinished(QDBusPendingCallWatcher *call)
{
   QDBusPendingReply<ArrayOfSwCollectionDetails> reply = *call;
   if (reply.isError()) {
       qDebug("GetCollList call reply error:");
       qDebug() << reply.error().message();
       emit this->collListRetrieved(this, ArrayOfSwCollectionDetails(), true);
   } else {
       emit this->collListRetrieved(this, reply.value(), false);
       qDebug("GetCollList call success:");
       ArrayOfSwCollectionDetails adets = reply.value();
       foreach (SwCollectionDetails dets, adets) {
           qDebug() << dets.collID << dets.collName << dets.collParentID
                    << dets.collMediaTypes << dets.collItemCount << dets.collAttributes;
       }
   }
}

void SwClientService::onGetCollDetailsFinished(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<SwCollectionDetails> reply = *call;
    if (reply.isError()) {
        qDebug("GetCollDetails call reply error:");
        qDebug() << reply.error().message();
        SwCollectionDetails emptyDetails;
        emit this->collDetailsRetrieved(this, emptyDetails, true);
    } else {
        emit this->collDetailsRetrieved(this, reply.value(), false);
        SwCollectionDetails dets = reply.value();
        qDebug("GetCollDetails call success:");
        qDebug() << dets.collID << dets.collName << dets.collParentID
                 << dets.collMediaTypes << dets.collItemCount << dets.collAttributes;
    }
}

void SwClientService::onCreateCollFinished(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<QString> reply = *call;
    if (reply.isError()) {
        qDebug("CreateColl call reply error:");
        qDebug() << reply.error().message();
        emit this->collCreated(this, QString(), true);
    } else {
        emit this->collCreated(this, reply.value(), false);
        qDebug("CreateColl call success:");
        qDebug() << reply.value();
    }
}


//Private functions

void SwClientService::init()
{
    //libsocialweb DBUS interface proxies
    mQueryIf = 0;
    mPhotoUploadIf = 0;
    mVideoUploadIf = 0;
    mBanishIf = 0;
    mStatusUpdateIf = 0;
    mAvatarIf = 0;
    mCollIf = 0;
    mServiceConfig = 0;
    mStaticCaps = QStringList();
    mDynCaps = QStringList();
    mUserAvatarPath = QString();
    //Begin static caps...
    mHaveStaticCaps = false;
    mHaveDynCaps = false;
    mPendingStatusUpdate = false;
    mCanVerifyCreds = false;
    mHasQueryIf = false;
    mHasPhotoUploadIf = false;
    mHasVideoUploadIf = false;
    mHasBanishIf = false;
    mHasStatusUpdateIf = false;
    mHasAvatarIf = false;
    mHasCollIf = false;
    mCanUpdateStatusWithGeotag = false;
    //Begin dyn caps...
    mCredsStateDyn = SwClientService::CredsUnknown;
    mIsConfiguredDyn = false;
    mCanUpdateStatusDyn = false;
    mCanRequestAvatarDyn = false;
    mCanGeotagDyn = false;
}

void SwClientService::checkStaticCaps()
{
    mCanVerifyCreds = mStaticCaps.contains("can-verify-credentials");

    mHasQueryIf = mStaticCaps.contains("has-query-iface");
    mHasPhotoUploadIf = mStaticCaps.contains("has-photo-upload-iface");
    mHasVideoUploadIf = mStaticCaps.contains("has-video-upload-iface");
    mHasBanishIf = mStaticCaps.contains("has-banishable-iface");

    mHasStatusUpdateIf = mStaticCaps.contains("has-update-status-iface");
    mHasAvatarIf = mStaticCaps.contains("has-avatar-iface");
    mHasCollIf = mStaticCaps.contains("has-collections-iface");

    mCanUpdateStatusWithGeotag = mStaticCaps.contains("can-update-status-with-geotag");
}

void SwClientService::checkDynCaps()
{
    SwClientService::CredsState newCredsState;
    bool tmp;
    if (mDynCaps.contains("credentials-valid")) {
        newCredsState = SwClientService::CredsValid;
    } else if (mDynCaps.contains("credentials-invalid")) {
        newCredsState = SwClientService::CredsInvalid;
    } else {
        newCredsState = SwClientService::CredsUnknown;
    }
    if (newCredsState != mCredsStateDyn) {
        mCredsStateDyn = newCredsState;
        emit credsStateChanged(this, mCredsStateDyn);
        emit CredsStateChanged(this, mCredsStateDyn);
    }

    tmp = mDynCaps.contains("is-configured");
    if (tmp != mIsConfiguredDyn) {
        mIsConfiguredDyn = tmp;
        emit isConfiguredChanged(this, mIsConfiguredDyn);
        emit IsConfiguredChanged(this, mIsConfiguredDyn);
    }

    tmp = mDynCaps.contains("can-update-status");
    if (tmp != mCanUpdateStatusDyn) {
        mCanUpdateStatusDyn = tmp;
        emit canUpdateStatusChanged(this, mCanUpdateStatusDyn);
        emit CanUpdateStatusChanged(this, mCanUpdateStatusDyn);
    }

    tmp = mDynCaps.contains("can-request-avatar");
    if (tmp != mCanRequestAvatarDyn) {
        mCanRequestAvatarDyn = tmp;
        emit canRequestAvatarChanged(this, mCanRequestAvatarDyn);
        emit CanRequestAvatarChanged(this, mCanRequestAvatarDyn);
    }

    tmp = mDynCaps.contains("can-geotag");
    if (tmp != mCanGeotagDyn) {
        mCanGeotagDyn = tmp;
        emit canGeotagChanged(this, mCanGeotagDyn);
        emit CanGeotagChanged(this, mCanGeotagDyn);
    }

}

void SwClientService::populateInterfaces()
{
    if (mHasQueryIf) {
        mQueryIf = new ComMeegoLibsocialwebQueryInterface(SwClient::GetSwServiceName(),
                                                           mServicePath, mConnection, this);
    }

    if (mHasPhotoUploadIf) {
        mPhotoUploadIf = new ComMeegoLibsocialwebPhotoUploadInterface(SwClient::GetSwServiceName(),
                                                                       mServicePath, mConnection, this);
        connect(mPhotoUploadIf,
                SIGNAL(PhotoUploadProgress(int,int,QString)),
                this,
                SLOT(onPhotoUploadProgress(int,int,QString)));
    }

    if (mHasVideoUploadIf) {
        mVideoUploadIf = new ComMeegoLibsocialwebVideoUploadInterface(SwClient::GetSwServiceName(),
                                                                      mServicePath, mConnection, this);
        connect(mVideoUploadIf,
                SIGNAL(VideoUploadProgress(int,int,QString)),
                this,
                SLOT(onVideoUploadProgress(int,int,QString)));
    }

    if (mHasBanishIf) {
        mBanishIf = new ComMeegoLibsocialwebBanishableInterface(SwClient::GetSwServiceName(),
                                                                 mServicePath, mConnection, this);
        connect(mBanishIf,
                SIGNAL(ItemHidden(QString)),
                this,
                SLOT(onItemHidden(QString)));
    }

    if (mHasStatusUpdateIf) {
        mStatusUpdateIf = new ComMeegoLibsocialwebStatusUpdateInterface(SwClient::GetSwServiceName(),
                                                                         mServicePath, mConnection, this);
        connect(mStatusUpdateIf,
                SIGNAL(StatusUpdated(bool)),
                this,
                SLOT(onStatusUpdated(bool)));
    }

    if (mHasAvatarIf) {
        mAvatarIf = new ComMeegoLibsocialwebAvatarInterface(SwClient::GetSwServiceName(),
                                                             mServicePath, mConnection, this);

        connect(mAvatarIf,
                SIGNAL(AvatarRetrieved(QString)),
                this,
                SLOT(onAvatarRetrieved(QString)));
    }

    if (mHasCollIf) {
        mCollIf = new ComMeegoLibsocialwebCollectionsInterface(SwClient::GetSwServiceName(),
                                                               mServicePath, mConnection, this);
    }

    //This is ugly...
    if (mIsLastFM) {
        mLastFmIf = new ComMeegoLibsocialwebServiceLastfmInterface(SwClient::GetSwServiceName(),
                                                                    mServicePath, mConnection, this);
    }

}

bool SwClientService::credsOK() const
{
    //If the service isn't configured, return false
    if (!isConfigured())
        return false;
    //If this service doesn't support verifying creds, then we just assume they
    //are OK
    if (!mCanVerifyCreds)
        return true;

    return (mCredsStateDyn == SwClientService::CredsValid);
}
