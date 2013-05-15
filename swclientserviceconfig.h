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

#ifndef SWCLIENTSERVICECONFIG_H
#define SWCLIENTSERVICECONFIG_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QHash>
#include <QMap>

//For Flickr support
extern "C" {
    #include <rest-extras/flickr-proxy.h>
    #include <gnome-keyring.h>
}

class SwClientService;
class QSettings;

class SwClientServiceConfig : public QObject
{
Q_OBJECT
public:
    explicit SwClientServiceConfig(QObject *parent = 0);
    explicit SwClientServiceConfig(SwClientService *swService, QObject *parent = 0);
    ~SwClientServiceConfig();

    Q_PROPERTY(SwClientService *service READ getService WRITE setService NOTIFY ServiceChanged);
    Q_PROPERTY(AuthType authType READ getAuthtype);

    enum AuthType {
        AuthTypeUsername,
        AuthTypePassword,
        AuthTypeFlickr,
        AuthTypeFacebook,
        AuthTypeOAuth,
        AuthTypeCustom,
        AuthTypeUnknown
    };

    Q_ENUMS(AuthType);

    Q_INVOKABLE void setService(SwClientService *swService);
    Q_INVOKABLE SwClientService * getService() const;

    Q_INVOKABLE bool isValid() const;

    Q_INVOKABLE AuthType getAuthtype() const;
    Q_INVOKABLE QString getCustomAuthtype() const;

    Q_INVOKABLE QHash<QString, QString> getConfigParams() const;
    Q_INVOKABLE QStringList getConfigParamNames() const;
    Q_INVOKABLE QString getParamValue(QString paramName) const;
    Q_INVOKABLE void setParamValue(QString paramName, QString value);
    Q_INVOKABLE bool saveConfigParams();

    Q_INVOKABLE QString getIconPath() const;
    Q_INVOKABLE const QString getDisplayName() const;
    Q_INVOKABLE QString getDescription() const;
    Q_INVOKABLE QString getLink() const;

    Q_INVOKABLE bool setConfigParams(const QHash<QString, QString> &newParams);

    Q_INVOKABLE bool reloadConfigParams();

    //Flickr support:

    Q_INVOKABLE bool flickrOpenLogin();
    Q_INVOKABLE QString flickrOpenLoginUrl();
    Q_INVOKABLE bool flickrContinueLogin();
    Q_INVOKABLE bool flickrDeleteLogin();
    void flickrRestCallResult(RestProxyCall *call, const GError *error);

    // Facebook support:
    Q_INVOKABLE QString facebookOpenLogin();
    Q_INVOKABLE bool facebookLoggedIn(QString url);
    Q_INVOKABLE bool facebookDeleteLogin();
    void facebookRestCallResult(RestProxyCall *call, const GError *error);

    // OAuth support:
    Q_INVOKABLE QString oauthOpenLogin();
    Q_INVOKABLE void oauthSetVerifier(const QString verifier);
    Q_INVOKABLE bool oauthContinueLogin();
    Q_INVOKABLE bool oauthDeleteLogin();
    Q_INVOKABLE bool oauthNeedsVerifier();
    void photobucketRestCallResult(RestProxyCall *call, const GError *error);
    void smugmugRestCallResult(RestProxyCall *call, const GError *error);
    void vimeoRestCallResult(RestProxyCall *call, const GError *error);

signals:
    void ServiceChanged();
    void serviceChanged();

    void usernameChanged();

public slots:

private:
    void init();
    void setupConfigParamsFromAuthType();
    bool loadConfigParams();
    bool loadConfigParamsFromGKR();
    bool saveConfigParamsToGKR(const QHash<QString, QString> &newParams);
    bool validateConfigParams(const QHash<QString, QString> &params);
    bool loadFromKeyFile();
    void fillAuthTypeMap();


    // Flickr support:
    bool loadConfigParamsForFlickr();
    static RestXmlNode * GetFlickrXmlFromRestCall(RestProxyCall *call);
    void flickrGotAuth(RestXmlNode *node);

    // Facebook support:
    bool loadConfigParamsForFacebook();
    void facebookGotAuth(RestProxyCall *call);

    // Oauth support:
    bool loadConfigParamsForOAuth();

    // Get the oauth user information.
    void oauthGetUserInfo();

    QMap<QString, AuthType> mAuthTypeMap;

    SwClientService *mSwService;
    QSettings *mSettings;
    QHash<QString, QString> mConfigParams;
    QString mCustomAuthType;

    QString mDisplayName;
    QString mDescription;
    QString mLink;
    QString mIconPath;
    QString mAuthPasswordServer;
    uint mCurGKRID;

    AuthType mAuthType;
    bool mValid;

    struct FlickrData {
        RestProxy *restProxy;
        const char *apiKey;
        const char *sharedSecret;
        char *frob;
        char *token;
    } mFlickrData;

    struct FacebookData {
        RestProxy *restProxy;
        const char *apiKey;
        const char *client_secret;
        char *base_url;
        char *auth_endpoint;
        char *authRedirectUrl;
        char *token;
    } mFacebookData;

    struct OAuthData {
        RestProxy *restProxy;
        const char *apiKey;
        const char *sharedSecret;
        char *request_token;
        char *auth_token;
        char *auth_token_secret;
        char *base_url;
        char *request_token_function;
        char *authorize_function;
        char *access_token_function;
        char *verifier;
        char *callback_url;
    } mOAuthData;
};

#endif // SWCLIENTSERVICECONFIG_H
