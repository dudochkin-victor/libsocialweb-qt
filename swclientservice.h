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

#ifndef SWCLIENTSERVICE_H
#define SWCLIENTSERVICE_H

#include "swclientitemview.h"

class ComMeegoLibsocialwebQueryInterface;
class ComMeegoLibsocialwebServiceInterface;
class ComMeegoLibsocialwebPhotoUploadInterface;
class ComMeegoLibsocialwebBanishableInterface;
class ComMeegoLibsocialwebStatusUpdateInterface;
class ComMeegoLibsocialwebAvatarInterface;
class ComMeegoLibsocialwebServiceLastfmInterface;
class ComMeegoLibsocialwebVideoUploadInterface;
class ComMeegoLibsocialwebCollectionsInterface;

class SwClientItem;
class SwClientServiceConfig;

class SwClientService : public QObject
{
Q_OBJECT
public:
    explicit SwClientService(const QDBusConnection &connection = QDBusConnection::sessionBus(),
                             QObject *parent = 0);

    explicit SwClientService(const QString &serviceName,
                             const QDBusConnection &connection = QDBusConnection::sessionBus(),
                             QObject *parent = 0);

    enum CredsState {
        CredsValid,
        CredsInvalid,
        CredsUnknown
    };
    enum CollectionMediaTypes {
        MediaTypeCollections = 1,   //With some services, you can have a collection of collections, or even mixed...
        MediaTypePhotos = 2,
        MediaTypePhotosAndCollections = 3,
        MediaTypeVideos = 4,
        MediaTypeVideosAndCollections = 5,
        MediaTypeVideosAndPhotos = 6,
	MediaTypeVideosPhotosAndCollections = 7,
    };

    Q_ENUMS(CollectionMediaTypes);

    Q_ENUMS(CredsState);

    Q_PROPERTY(QString serviceName READ getServiceName WRITE setServiceName NOTIFY ServiceNameChanged);
    Q_PROPERTY(bool configured READ isConfigured NOTIFY IsConfiguredChanged);
    Q_PROPERTY(CredsState creds READ credsState NOTIFY CredsStateChanged);

    //Capabilities-independant functions:

    Q_INVOKABLE void setServiceName(QString svcName);

    //Get the SwClientServiceConfig for this service
    Q_INVOKABLE SwClientServiceConfig *getServiceConfig();

    //Get full capabilities list of the service
    Q_INVOKABLE const QStringList getStaticCaps();
    //Get current capabilities list of the service
    Q_INVOKABLE const QStringList getDynCaps();
    //Get the name of the service
    Q_INVOKABLE const QString getServiceName() const;
    Q_INVOKABLE const QString getDisplayName() const;

    Q_INVOKABLE const QString getUserAvatarPath() const;

    //These tell us if this service has this capability at all
    Q_INVOKABLE bool hasOpenView() const;
    Q_INVOKABLE bool hasPhotoUpload() const;
    Q_INVOKABLE bool hasVideoUpload() const;
    Q_INVOKABLE bool hasBanishItems() const;
    Q_INVOKABLE bool hasUpdateStatus() const;
    Q_INVOKABLE bool hasUpdateStatusWithGeotag() const;
    Q_INVOKABLE bool hasRequestAvatar() const;
    Q_INVOKABLE bool hasCollections() const;
    Q_INVOKABLE bool hasVerifyCreds() const;
    Q_INVOKABLE bool isLastFm() const;

    //These tell us if this service can perform this functionality right now
    Q_INVOKABLE bool canUpdateStatus() const;
    Q_INVOKABLE bool canUpdateStatusWithGeotag() const;
    Q_INVOKABLE bool canRequestAvatar() const;

    //Current status dynamic properties
    Q_INVOKABLE bool isConfigured() const;
    Q_INVOKABLE CredsState credsState() const;


    //Capabilities-dependant functions:

    //Query interface:
    //Open a view (feed, typically, but the query can be used for various other
    //custom functionality. See libsocialweb documentation (sw-query object) for details
    Q_INVOKABLE SwClientItemView *openView(const QString &query, const SwParams &params = SwParams()) const;

    //Photo Upload interface:
    //Upload a photo. Returned int is the "Operation ID" from libsocialweb,
    //or -1 if we failed.
    //Photo upload progress will be signalled by the PhotoUploadProgress signal
    Q_INVOKABLE int uploadPhoto(const QString &localFilename, const SwParams &fields = SwParams()) const;

    //Video Upload interface
    //Upload a video. Returned int is the "Operation ID" from libsocialweb,
    //or -1 if we failed.
    //Video upload progress will be signalled by the VideoUploadProgress signal
    Q_INVOKABLE int uploadVideo(const QString &localFilename, const SwParams &fields = SwParams()) const;

    //Banishable interface:
    //Hide ("Banish") an item
    //Returns true if we can successfully call the appropriate
    //banishable interface call, returns false otherwise.
    //NOTE: The "banish" operation is async - we will emit a
    //"ItemHidden" signal w/ the swUUID when it actually happens.
    Q_INVOKABLE bool hideItem(const SwClientItem *item) const;
    Q_INVOKABLE bool hideItem(const QString &swUUID) const;

    //Status Update interface
    //Update status on the service (aka send a Tweet, for twitter)
    //Returns true if we can successfully call the appropriate
    //StatusUpdate interface call, returns false otherwise.
    //NOTE: The "UpdateStatus" operation is async - we will emit a
    //"StatusUpdated(bool)" signal when it actually happens.
    Q_INVOKABLE bool updateStatus(const QString &statusMessage, const SwParams &fields = SwParams());

    //Request Avatar interface
    //Requests that the service send us the users avatar
    //Returns true if we can successfully call the appropriate
    //Avatar interface call, returns false otherwise.
    //NOTE: The "RequestAvatar" operation is async - we will emit
    //the "AvatarRetrieved(QString path)" signal when
    //the avatar is actually retrieved
    Q_INVOKABLE bool requestAvatar() const;


    //Collections interface
    //Methods for working with Collections (AKA Albums)
    //All return true if we can successfully deal w/ the interface
    //returns false otherwise.
    //NOTE: All the Collections interface methods are async - we will emit
    //the "collListRetrieved", "collDetailsRetrieved", and "collCreated"
    //signals (respectively) when the operations complete.
    Q_INVOKABLE bool getCollList();
    Q_INVOKABLE bool getCollDetails(const QString &collID);
    Q_INVOKABLE bool createColl(const SwCollectionDetails &collDetails);
    Q_INVOKABLE bool createColl(const QString &collName, CollectionMediaTypes collMediaTypes,
                                const QString &collParentID, SwParams collExtraParams);
    Q_INVOKABLE const ArrayOfUInt getCreatableTypes() const;

    //Lastfm interface
    //Both of these return true if we can successfully call the appropriate
    //Lastfm interface call, returns false otherwise.
    Q_INVOKABLE bool nowPlaying(const QString &artist, const QString &album,
                    const QString &track, uint length, uint tracknumber,
                    const QString &musicbrainz) const;
    Q_INVOKABLE bool submitTrack(const QString &artist, const QString &album,
                     const QString &track, qlonglong time,
                     const QString &source, const QString &rating,
                     uint length, uint tracknumber,
                     const QString &musicbrainz) const;

signals:
    //QML property signals
    void serviceNameChanged();
    void ServiceNameChanged();

    //Service interface signals
    void dynCapsChanged(SwClientService *service, QStringList newDynCaps);
    void DynCapsChanged(SwClientService *service, QStringList newDynCaps);
    void userChanged(SwClientService *service);
    void UserChanged(SwClientService *service);

    //Dynamic caps signals
    void credsStateChanged(SwClientService *service, SwClientService::CredsState credsState);
    void CredsStateChanged(SwClientService *service, SwClientService::CredsState credsState);
    void isConfiguredChanged(SwClientService *service, bool isConfigured);
    void IsConfiguredChanged(SwClientService *service, bool isConfigured);
    void canUpdateStatusChanged(SwClientService *service, bool canUpdateStatus);
    void CanUpdateStatusChanged(SwClientService *service, bool canUpdateStatus);
    void canRequestAvatarChanged(SwClientService *service, bool canReqAvatar);
    void CanRequestAvatarChanged(SwClientService *service, bool canReqAvatar);
    void canGeotagChanged(SwClientService *service, bool canGeotag);
    void CanGeotagChanged(SwClientService *service, bool canGeotag);

    //Photo upload interface signals
    void photoUploadProgress(SwClientService *service, int uploadID, int progress, const QString &errMessage);
    void PhotoUploadProgress(SwClientService *service, int uploadID, int progress, const QString &errMessage);

    //Video upload interface signals
    void videoUploadProgress(SwClientService *service, int uploadID, int progress, const QString &errMessage);
    void VideoUploadProgress(SwClientService *service, int uploadID, int progress, const QString &errMessage);

    //Banishable interface signals
    void itemHidden(SwClientService *service, const QString &uuid);
    void ItemHidden(SwClientService *service, const QString &uuid);

    //StatusUpdate interface signals
    void statusUpdated(SwClientService *service, bool success);
    void StatusUpdated(SwClientService *service, bool success);

    //Avatar interface signals
    void avatarRetrieved(SwClientService *service, const QString &avatarPath);
    void AvatarRetrieved(SwClientService *service, const QString &avatarPath);

    //Collections interface signals
    void collListRetrieved(SwClientService *service, const ArrayOfSwCollectionDetails &collList, bool isError);
    void collDetailsRetrieved(SwClientService *service, const SwCollectionDetails &collDetails, bool isError);
    void collCreated(SwClientService *service, const QString &newCollID, bool isError);

public slots:
    Q_INVOKABLE void onCredentialsUpdated();


private slots:
    void onGetStaticCapsFinished(QDBusPendingCallWatcher *call);
    void onGetDynCapsFinished(QDBusPendingCallWatcher *call);
    void onDynCapsChanged(const QStringList &newDynCaps);
    void onUserChanged();
    void onPhotoUploadProgress(int uploadID, int progress, const QString &errMessage);
    void onVideoUploadProgress(int uploadID, int progress, const QString &errMessage);
    void onItemHidden(const QString &swUUID);
    void onStatusUpdated(bool success);
    void onAvatarRetrieved(const QString &avatarLocation);
    void onGetCollListFinished(QDBusPendingCallWatcher *call);
    void onGetCollDetailsFinished(QDBusPendingCallWatcher *call);
    void onCreateCollFinished(QDBusPendingCallWatcher *call);

private:
    void init();
    void checkStaticCaps();
    void checkDynCaps();
    void populateInterfaces();
    bool credsOK() const;

    QString mServicePath;
    QDBusConnection mConnection;
    ComMeegoLibsocialwebServiceInterface *mServiceIf;
    ComMeegoLibsocialwebQueryInterface *mQueryIf;
    ComMeegoLibsocialwebPhotoUploadInterface *mPhotoUploadIf;
    ComMeegoLibsocialwebVideoUploadInterface *mVideoUploadIf;
    ComMeegoLibsocialwebBanishableInterface *mBanishIf;
    ComMeegoLibsocialwebStatusUpdateInterface *mStatusUpdateIf;
    ComMeegoLibsocialwebAvatarInterface *mAvatarIf;
    ComMeegoLibsocialwebServiceLastfmInterface *mLastFmIf;
    ComMeegoLibsocialwebCollectionsInterface *mCollIf;

    SwClientServiceConfig *mServiceConfig;

    QStringList mStaticCaps;
    QStringList mDynCaps;

    QString mServiceName;
    QString mUserAvatarPath;

    bool mHaveStaticCaps;
    bool mHaveDynCaps;
    bool mPendingStatusUpdate;


    //Static Caps bools
    bool mCanVerifyCreds;
    bool mHasQueryIf;
    bool mHasPhotoUploadIf;
    bool mHasVideoUploadIf;
    bool mHasBanishIf;
    bool mHasStatusUpdateIf;
    bool mHasAvatarIf;
    bool mHasCollIf;
    bool mCanUpdateStatusWithGeotag;
    bool mIsLastFM;

    //Dyn Caps bools - if any of these change, we emit a signal for it
    CredsState mCredsStateDyn;
    bool mIsConfiguredDyn;
    bool mCanUpdateStatusDyn;
    bool mCanRequestAvatarDyn;
    bool mCanGeotagDyn;

};

#endif // SWCLIENTSERVICE_H
