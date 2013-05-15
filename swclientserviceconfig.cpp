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

#include "swclientserviceconfig.h"
#include "swclientservice.h"

#include <QSettings>
#include <QDesktopServices>
#include <QtXml/QDomDocument>
#include <QLocale>

#include <qjson/parser.h>


extern "C" {
    //For standard username/password service support
    #include <glib-object.h>

    //For Flickr support:
    #include <rest/rest-xml-parser.h>
    #include <libsocialweb-keystore/sw-keystore.h>

    // For Facebook support:
    #include <rest/oauth2-proxy.h>
    #include <gnome-keyring.h>

    // For OAuth support:
    #include <rest/oauth-proxy.h>
}

static const GnomeKeyringPasswordSchema sFlickrSchema = {
    GNOME_KEYRING_ITEM_GENERIC_SECRET,
    {
        { "server", GNOME_KEYRING_ATTRIBUTE_TYPE_STRING },
        { "api-key", GNOME_KEYRING_ATTRIBUTE_TYPE_STRING },
        { NULL, (GnomeKeyringAttributeType)0}
    },
    0,
    0,
    0
};

#define FLICKR_SERVER "http://flickr.com/"

static const GnomeKeyringPasswordSchema sFacebookSchema = {
    GNOME_KEYRING_ITEM_GENERIC_SECRET,
    {
        { "server", GNOME_KEYRING_ATTRIBUTE_TYPE_STRING },
        { "client-id", GNOME_KEYRING_ATTRIBUTE_TYPE_STRING },
        { NULL, (GnomeKeyringAttributeType)0}
    },
    0,
    0,
    0
};

static const GnomeKeyringPasswordSchema sOAuthSchema = {
    GNOME_KEYRING_ITEM_GENERIC_SECRET,
    {
        { "server", GNOME_KEYRING_ATTRIBUTE_TYPE_STRING },
        { "consumer-key", GNOME_KEYRING_ATTRIBUTE_TYPE_STRING },
        { NULL, (GnomeKeyringAttributeType)0 }
    },
    0,
    0,
    0
};

#define KEY_FILE_PATH QString("/usr/share/libsocialweb/services/%1.keys")
#define ICON_FILE_PATH QString("/usr/share/libsocialweb/services/%1.png")

#define SW_KEY_GROUP QString("LibSocialWebService")

#define SW_KEY_NAME QString("Name")
#define SW_KEY_AUTHTYPE QString("AuthType")
#define SW_KEY_DESCRIPTION QString("Description")
#define SW_KEY_LINK QString("Link")
#define SW_KEY_AUTHPASSWORDSERVER QString("AuthPasswordServer")

#define SW_GKR_KEY_USERNAME QString("user")
#define SW_GKR_KEY_PASSWORD QString("password")

extern "C" {

    void FacebookRestCallback(RestProxyCall *call,
                              const GError *error,
                              GObject *weak_object,
                              gpointer userdata)
    {
        qDebug() << "FacebookRestCallback called";
        Q_UNUSED(weak_object);
        SwClientServiceConfig *self = (SwClientServiceConfig *) userdata;
        self->facebookRestCallResult(call, error);
    }

    void FlickrRestCallback(RestProxyCall *call,
                            const GError *error,
                            GObject *weak_object,
                            gpointer userdata)
    {
        qDebug() << "FlickrRestCallback called";
        Q_UNUSED(weak_object);
        SwClientServiceConfig *self = (SwClientServiceConfig *) userdata;
        self->flickrRestCallResult(call, error);
    }

    void PhotobucketRestCallback(RestProxyCall *call,
                                 const GError *error,
                                 GObject *weak_object,
                                 gpointer userdata)
    {
        qDebug() << "PhotobucketRestCallback called";
        Q_UNUSED(weak_object);
        SwClientServiceConfig *self = (SwClientServiceConfig *) userdata;
        self->photobucketRestCallResult(call, error);
    }

    void SmugmugRestCallback(RestProxyCall *call,
                             const GError *error,
                             GObject *weak_object,
                             gpointer userdata)
    {
        qDebug() << "SmugmugRestCallback called";
        Q_UNUSED(weak_object);
        SwClientServiceConfig *self = (SwClientServiceConfig *) userdata;
        self->smugmugRestCallResult(call, error);
    }

    void VimeoRestCallback(RestProxyCall *call,
                             const GError *error,
                             GObject *weak_object,
                             gpointer userdata)
    {
        qDebug() << "VimeoRestCallback called";
        Q_UNUSED(weak_object);
        SwClientServiceConfig *self = (SwClientServiceConfig *) userdata;
        self->vimeoRestCallResult(call, error);
    }
}

SwClientServiceConfig::SwClientServiceConfig(QObject *parent):
    QObject(parent)
{
    init();
}

SwClientServiceConfig::SwClientServiceConfig(SwClientService *swService, QObject *parent) :
    QObject(parent)
{
    init();
    setService(swService);
}

void SwClientServiceConfig::setService(SwClientService *swService)
{
    mSwService = swService;
    //Initialize the glib/gobject types for GConf...
    g_type_init();

    //Init our FlickrData struct
    mFlickrData.restProxy = 0;
    mFlickrData.apiKey = 0;
    mFlickrData.sharedSecret = 0;
    mFlickrData.frob = 0;
    mFlickrData.token = 0;

    // Init our FacebookData struct
    mFacebookData.restProxy = 0;
    mFacebookData.apiKey = 0;
    mFacebookData.client_secret = 0;
    mFacebookData.base_url = 0;
    mFacebookData.auth_endpoint = 0;
    mFacebookData.authRedirectUrl = 0;
    mFacebookData.token = 0;

    // Init our OAuthData struct
    mOAuthData.restProxy = 0;
    mOAuthData.apiKey = 0;
    mOAuthData.sharedSecret = 0;
    mOAuthData.request_token = 0;
    mOAuthData.auth_token = 0;
    mOAuthData.auth_token_secret = 0;
    mOAuthData.base_url = 0;
    mOAuthData.request_token_function = 0;
    mOAuthData.authorize_function = 0;
    mOAuthData.access_token_function = 0;
    mOAuthData.verifier = 0;
    mOAuthData.callback_url = 0;

    if (!mAuthTypeMap.count())
        fillAuthTypeMap();

    //TODO: Figure out a better way to handle/indicate construction/KeyFile read/parse failure...

    if (!mSwService) {
        qCritical("SwClientServiceConfig got invalid swService!");
        return;
    }

    mSettings = new QSettings(KEY_FILE_PATH.arg(mSwService->getServiceName()), QSettings::IniFormat);
    mSettings->setIniCodec("UTF-8");
    if (!mSettings) {
        qCritical() << QString("SwClientServiceConfig couldn't load QSettings file %1!").arg(
                KEY_FILE_PATH.arg(mSwService->getServiceName()));
        return;
    }

    if (!loadFromKeyFile())
        return;

    setupConfigParamsFromAuthType();
    if (!loadConfigParams())
        return;

    mIconPath = ICON_FILE_PATH.arg(mSwService->getServiceName());

    mValid = true;
    emit this->serviceChanged();
    emit this->ServiceChanged();
}

SwClientService * SwClientServiceConfig::getService() const
{
    return mSwService;
}

SwClientServiceConfig::~SwClientServiceConfig()
{
    if (mFlickrData.token)
        gnome_keyring_free_password(mFlickrData.token);
    if (mFlickrData.frob)
        g_free(mFlickrData.frob);
}

bool SwClientServiceConfig::isValid() const
{
    return mValid;
}

SwClientServiceConfig::AuthType SwClientServiceConfig::getAuthtype() const
{
    return mAuthType;
}

QString SwClientServiceConfig::getCustomAuthtype() const
{
    return mCustomAuthType;
}

QHash<QString, QString> SwClientServiceConfig::getConfigParams() const
{
    qDebug() << "getConfigParams called with params " << mConfigParams;
    return mConfigParams;
}

QStringList SwClientServiceConfig::getConfigParamNames() const
{
    return mConfigParams.keys();
}

QString SwClientServiceConfig::getParamValue(QString paramName) const
{
    return mConfigParams.value(paramName, QString());
}

void SwClientServiceConfig::setParamValue(QString paramName, QString value)
{
    mConfigParams.insert(paramName, value);
}

bool SwClientServiceConfig::saveConfigParams()
{
    return (validateConfigParams(mConfigParams)
            && saveConfigParamsToGKR(mConfigParams));
}

QString SwClientServiceConfig::getIconPath() const
{
    return mIconPath;
}

const QString SwClientServiceConfig::getDisplayName() const
{
    return mDisplayName;
}

QString SwClientServiceConfig::getDescription() const
{
    return mDescription;
}

QString SwClientServiceConfig::getLink() const
{
    return mLink;
}

bool SwClientServiceConfig::setConfigParams(const QHash<QString, QString> &newParams)
{
    qDebug() << newParams;
    if (validateConfigParams(newParams)
        && saveConfigParamsToGKR(newParams)) {
        mConfigParams = newParams;
        return true;
    } else {
        return false;
    }
}

bool SwClientServiceConfig::reloadConfigParams()
{
    if (!loadFromKeyFile())
        return false;
    setupConfigParamsFromAuthType();
    return loadConfigParams();
}

//Flickr support:

QString SwClientServiceConfig::flickrOpenLoginUrl()
{
    char *url;
    RestProxyCall *call;
    RestXmlNode *root;
    GError *error = NULL;

    call = rest_proxy_new_call(mFlickrData.restProxy);
    if (!call) {
        return QString();
    }
    rest_proxy_call_set_function(call, "flickr.auth.getFrob");
    if (!rest_proxy_call_sync(call, &error)) {
        qWarning("Couldn't get Flickr frob: %s!", error->message);
        g_error_free(error);
        return QString();
    } else {
        root = GetFlickrXmlFromRestCall(call);
        mFlickrData.frob = g_strdup(rest_xml_node_find(root, "frob")->content);
        rest_xml_node_unref(root);
    }

    url = flickr_proxy_build_login_url(FLICKR_PROXY(mFlickrData.restProxy), mFlickrData.frob, "write");

    QUrl qurl = QUrl::fromEncoded(url);
    qDebug() << qurl;
    return qurl.toString();
}

bool SwClientServiceConfig::flickrOpenLogin()
{
    QString qurl = flickrOpenLoginUrl();
    if (!qurl.isEmpty())
        return QDesktopServices::openUrl(QUrl(qurl));
    else
        return false;
}

bool SwClientServiceConfig::flickrContinueLogin()
{
    RestProxyCall *call;
    RestXmlNode *node;
    GError *error = NULL;

    if (!mFlickrData.frob) {
        qWarning("No frob found in flickrContinueLogin, cannot continue!");
        return false;
    }

    call = rest_proxy_new_call(mFlickrData.restProxy);
    rest_proxy_call_set_function(call, "flickr.auth.getToken");
    rest_proxy_call_add_param(call, "frob", mFlickrData.frob);
    if (!rest_proxy_call_sync(call, &error)) {
        qWarning("Cannot get Flickr token: %s!", error->message);
        g_error_free(error);
        return false;
    }

    node = GetFlickrXmlFromRestCall(call);
    if (!node) {
        qWarning("Couldn't get RestXmlNode from Flickr getToken call!");
        return false;
    }

    mFlickrData.token = g_strdup(rest_xml_node_find(node, "token")->content);
    flickr_proxy_set_token(FLICKR_PROXY(mFlickrData.restProxy), mFlickrData.token);
    flickrGotAuth(node);

    GnomeKeyringResult result;
    GnomeKeyringAttributeList *attrs;
    guint32 krID;

    attrs = gnome_keyring_attribute_list_new();
    gnome_keyring_attribute_list_append_string(attrs, "server", FLICKR_SERVER);
    gnome_keyring_attribute_list_append_string(attrs, "api-key", mFlickrData.apiKey);

    //Instanciate the QBA so that we don't end up with an invalid transient char *
    QByteArray qba = mDisplayName.toLatin1();

    result = gnome_keyring_item_create_sync(NULL,
                                            GNOME_KEYRING_ITEM_GENERIC_SECRET,
                                            qba.data(),
                                            attrs,
                                            mFlickrData.token,
                                            TRUE,
                                            &krID);

    if (result == GNOME_KEYRING_RESULT_OK) {
        //Deprecated, according to the gnome ref documentation
        /*GnomeKeyringResult result2 =
        gnome_keyring_item_grant_access_rights_sync(NULL,
                                                    "libsocialweb",
                                                    "/usr/libexec/libsocialweb-core",
                                                    krID,
                                                    GNOME_KEYRING_ACCESS_READ);
        qDebug("Results from grant_access_rights_sync: %s",
               gnome_keyring_result_to_message(result2));*/
        GnomeKeyringApplicationRef * appRef = gnome_keyring_application_ref_new();
        GnomeKeyringAccessControl *ac = gnome_keyring_access_control_new(appRef,
                                                                         GNOME_KEYRING_ACCESS_READ);
        gnome_keyring_item_ac_set_display_name(ac, "libsocialweb");
        gnome_keyring_item_ac_set_path_name(ac, "/usr/libexec/libsocialweb-core");
        gnome_keyring_item_ac_set_access_type(ac, GNOME_KEYRING_ACCESS_READ);
        GList *acl = g_list_alloc();
        acl = g_list_append(acl, ac);
        GnomeKeyringResult result2 = gnome_keyring_item_set_acl_sync(NULL, krID, acl);
/*        qDebug("Results from set_acl_sync: %s",
               gnome_keyring_result_to_message(result2));*/
        if (result2 != GNOME_KEYRING_RESULT_OK) {
            qWarning("Cannot grant Flickr keyring access rights to libsocialweb-core: %s",
                     gnome_keyring_result_to_message(result2));
        }
        gnome_keyring_access_control_free(ac);
        gnome_keyring_application_ref_free(appRef);
        g_list_free(acl);

    } else {
        qWarning("Cannot create/update gnome keyring for Flickr credentials: %s",
                 gnome_keyring_result_to_message(result));
    }
    rest_xml_node_unref(node);
    mSwService->onCredentialsUpdated();
    return (result == GNOME_KEYRING_RESULT_OK);

}

bool SwClientServiceConfig::flickrDeleteLogin()
{
    if (gnome_keyring_delete_password_sync(&sFlickrSchema,
                                           "server", FLICKR_SERVER,
                                           "api-key", mFlickrData.apiKey,
                                           NULL) != GNOME_KEYRING_RESULT_OK) {
        qWarning("Couldn't delete Flickr login credentials!");
        return false;
    }
    mSwService->onCredentialsUpdated();
    return true;
}

// Facebook support:

QString SwClientServiceConfig::facebookOpenLogin()
{
    QString url;
    url = QString("https://graph.facebook.com/oauth/authorize?"
                  "client_id=%1&redirect_uri=%2&response_type=token&"
                  "scope=read_stream,publish_stream,email,xmpp_login,user_photos")
        .arg(mFacebookData.apiKey)
        .arg(mFacebookData.authRedirectUrl);
    return url;
}

bool SwClientServiceConfig::facebookLoggedIn(QString url)
{
    bool retval = false;
    QString access_token;

    QUrl loggedInUrl = QUrl::fromEncoded(url.toUtf8());
    QString fragment = loggedInUrl.fragment();
    QStringList bits = fragment.split('&');
    for (int i = 0; i < bits.size(); ++i) {
        QStringList split = bits[i].split('=');
        if (split.at(0) == "access_token")
            access_token = split.at(1);
    }

    if (!access_token.isEmpty()) {
        mFacebookData.token = g_strdup(access_token.toStdString().c_str());
        qDebug() << "facebook logged in with access token const char * " << mFacebookData.token;
        char * encoded = g_base64_encode ((guchar*) mFacebookData.token, strlen(mFacebookData.token));
        qDebug() << "encoded access token " << encoded;
        /* TODO async */
        GnomeKeyringResult result =
            gnome_keyring_store_password_sync (&sFacebookSchema,
                                               NULL,
                                               mFacebookData.base_url,
                                               encoded,
                                               "server", mFacebookData.base_url,
                                               "client-id", mFacebookData.apiKey,
                                               NULL);
        g_free(encoded);

        GError *error = NULL;
        RestProxyCall *call;
        oauth2_proxy_set_access_token (OAUTH2_PROXY (mFacebookData.restProxy),
                                       mFacebookData.token);
        call = rest_proxy_new_call(mFacebookData.restProxy);
        rest_proxy_call_set_function(call, "me");
        if (!rest_proxy_call_async(call, FacebookRestCallback, NULL, this, &error)) {
            qWarning("Error in rest_proxy_call_async: %s! Cannot get user info.", error->message);
            g_error_free(error);
        }

        mSwService->onCredentialsUpdated();
        retval = (result == GNOME_KEYRING_RESULT_OK);
    }

    return retval;
}

bool SwClientServiceConfig::facebookDeleteLogin()
{
    if (gnome_keyring_delete_password_sync(&sFacebookSchema,
                                           "server", mFacebookData.base_url,
                                           "client-id", mFacebookData.apiKey,
                                           NULL) != GNOME_KEYRING_RESULT_OK) {
        qWarning("Couldn't delete Facebook login credentials!");
        return false;
    }
    mSwService->onCredentialsUpdated();
    return true;
}

// OAuth support functions

QString SwClientServiceConfig::oauthOpenLogin()
{
    GError *error = NULL;

    if (!oauth_proxy_request_token(OAUTH_PROXY(mOAuthData.restProxy),
        mOAuthData.request_token_function, mOAuthData.callback_url, &error)) {
        qWarning("Couldn't get OAuth request token: %s!", error->message);
        g_error_free(error);
        return QString();
    } else {
        // now we need to extract the oauth_token from the result
        mOAuthData.request_token = g_strdup(oauth_proxy_get_token(OAUTH_PROXY(mOAuthData.restProxy)));
        qDebug() << "got auth_token " << mOAuthData.request_token;
    }

    // First see if the authorize_function is a valid url alone.
    QUrl url(mOAuthData.authorize_function);
    // Otherwise, add it to the end of the base_url.
    if (!url.isValid() || url.isRelative())
        url = QString("%1%2").arg(mOAuthData.base_url)
              .arg(mOAuthData.authorize_function);
    url.addQueryItem("oauth_token", mOAuthData.request_token);

    return url.toString();
}

bool SwClientServiceConfig::oauthDeleteLogin()
{
    if (gnome_keyring_delete_password_sync(&sOAuthSchema,
                                           "server", mOAuthData.base_url,
                                           "consumer-key", mOAuthData.apiKey,
                                           NULL) != GNOME_KEYRING_RESULT_OK) {
        qDebug() << "Couldn't delete " << mDisplayName << " OAuth login credentials!";
        return false;
    }
    mSwService->onCredentialsUpdated();
    return true;
}

bool SwClientServiceConfig::oauthNeedsVerifier()
{
    return (oauth_proxy_is_oauth10a (OAUTH_PROXY (mOAuthData.restProxy)) &&
           strcmp(mOAuthData.callback_url, "oob") == 0);
}

void SwClientServiceConfig::oauthSetVerifier(const QString verifier)
{
    mOAuthData.verifier = g_strdup(verifier.toStdString().c_str());
}

bool SwClientServiceConfig::oauthContinueLogin()
{
    GError *error = NULL;

    if (!oauth_proxy_access_token(OAUTH_PROXY(mOAuthData.restProxy),
        mOAuthData.access_token_function, mOAuthData.verifier, &error)) {
        qDebug() << "Unable to continue login, couldn't get access_token";
        if (error)
            qDebug() << "Error message: " << error->message;
        return false;
    } else {
        mOAuthData.auth_token =
            g_strdup(oauth_proxy_get_token(OAUTH_PROXY(mOAuthData.restProxy)));
        mOAuthData.auth_token_secret =
            g_strdup(oauth_proxy_get_token_secret(OAUTH_PROXY(mOAuthData.restProxy)));
        qDebug() << "got auth_token " << mOAuthData.auth_token;
        qDebug() << "got auth token secret " << mOAuthData.auth_token_secret;
        char * encoded_token = g_base64_encode ((guchar*) mOAuthData.auth_token, strlen(mOAuthData.auth_token));
        char * encoded_secret = g_base64_encode((guchar*)mOAuthData.auth_token_secret, strlen(mOAuthData.auth_token_secret));
        char *encoded = g_strconcat (encoded_token, " ", encoded_secret, NULL);
        qDebug() << "encoded access token " << encoded;

        GnomeKeyringResult result;
        GnomeKeyringAttributeList *attrs;
        guint32 id;
        attrs = gnome_keyring_attribute_list_new ();
        gnome_keyring_attribute_list_append_string (attrs, "server", mOAuthData.base_url);
        gnome_keyring_attribute_list_append_string (attrs, "consumer-key", mOAuthData.apiKey);

        result = gnome_keyring_item_create_sync (NULL,
                 GNOME_KEYRING_ITEM_GENERIC_SECRET,
                 mDisplayName.toStdString().c_str(),
                 attrs, encoded,
                 TRUE, &id);
        g_free(encoded_token);
        g_free(encoded_secret);
        g_free(encoded);

        if (result == GNOME_KEYRING_RESULT_OK) {
            gnome_keyring_item_grant_access_rights_sync (NULL,
                    "libsocialweb",
                    "/usr/libexec/libsocialweb-core",
                    id, GNOME_KEYRING_ACCESS_READ);
        }
        mSwService->onCredentialsUpdated();

        oauthGetUserInfo();

        return (result == GNOME_KEYRING_RESULT_OK);
    }
}

//Private functions

void SwClientServiceConfig::init()
{
    mIconPath = QString();
    mCurGKRID = 0;
    mAuthType = AuthTypeUnknown;
    mValid = false;
}

void SwClientServiceConfig::setupConfigParamsFromAuthType()
{
    mConfigParams.clear();
    switch (mAuthType) {
    case SwClientServiceConfig::AuthTypePassword:
        mConfigParams.insert(SW_GKR_KEY_PASSWORD, QString());
    case SwClientServiceConfig::AuthTypeUsername:
        mConfigParams.insert(SW_GKR_KEY_USERNAME, QString());
        break;
    case SwClientServiceConfig::AuthTypeFlickr:
        //We'll pass username here, as pulled from the RestFlickrProxy, but
        //the client app can't set the data. TODO - look into read/write markings
        //for these params
/*        qDebug("Hit SwClientServiceConfig::setupConfigParamsFromAuthType "
            "with AuthTypeFlickr");*/
        mConfigParams.insert(SW_GKR_KEY_USERNAME, QString());
        break;
    case SwClientServiceConfig::AuthTypeFacebook:
        mConfigParams.insert(SW_GKR_KEY_USERNAME, QString());
        break;
    case SwClientServiceConfig::AuthTypeOAuth:
        mConfigParams.insert(SW_GKR_KEY_USERNAME, QString());
        break;
    case SwClientServiceConfig::AuthTypeCustom:
        //TODO
        qDebug()
            << QString(
            "Hit SwClientServiceConfig::setupConfigParamsFromAuthType "
            "with AuthTypeCustom: %1").arg(mCustomAuthType);
        break;
    case SwClientServiceConfig::AuthTypeUnknown:
        qCritical()
            << QString("Unknown auth type in SwClientServiceConfig for service %1!"
               ).arg(mSwService->getServiceName());
        break;
    }
}

bool SwClientServiceConfig::loadConfigParams()
{
    if (mAuthType == SwClientServiceConfig::AuthTypeFlickr) {
        return loadConfigParamsForFlickr();
    } else if (mAuthType == SwClientServiceConfig::AuthTypeFacebook) {
        return loadConfigParamsForFacebook();
    } else if (mAuthType == SwClientServiceConfig::AuthTypeOAuth) {
        return loadConfigParamsForOAuth();
    } else {
        return loadConfigParamsFromGKR();
    }
}

bool SwClientServiceConfig::loadConfigParamsFromGKR()
{
    if (mAuthPasswordServer.isEmpty())
        return false;
    if (mConfigParams.count() > 0) {
        const gchar *server = NULL;
        GList *list = NULL;
        GnomeKeyringResult res;

        const QByteArray qbaServer = mAuthPasswordServer.toLatin1();
        server = qbaServer.data();

        if (((res = gnome_keyring_find_network_password_sync(NULL, NULL, server, NULL,
                                                      NULL, NULL, 0, &list))
            == GNOME_KEYRING_RESULT_OK) && (list)) {

            GnomeKeyringNetworkPasswordData *data = (GnomeKeyringNetworkPasswordData *)list->data;
            mCurGKRID = data->item_id;
            mConfigParams.insert(SW_GKR_KEY_USERNAME, QString(data->user));
            emit usernameChanged();
            if (mAuthType == AuthTypePassword)
                mConfigParams.insert(SW_GKR_KEY_PASSWORD, QString(data->password));
        } else {
            if (res == GNOME_KEYRING_RESULT_NO_MATCH)
                return true;
            qDebug("Didn't get GKR_RES_OK or GKR_RES_NOTFOUND on find_network_password_sync, or got NULL list!");
            return false;
        }
    }

    return true;
}

bool SwClientServiceConfig::saveConfigParamsToGKR(const QHash<QString, QString> &newParams)
{
    if (mAuthPasswordServer.isEmpty())
        return false;
    if (newParams.count() > 0) {
        const QByteArray qbaUser = newParams.value(SW_GKR_KEY_USERNAME).toLatin1();
        const gchar *guser = qbaUser.data();

        const gchar *gpass = NULL;
        const QByteArray qbaPass = newParams.value(SW_GKR_KEY_PASSWORD).toLatin1();
        if (mAuthType == AuthTypePassword) {
            gpass = qbaPass.data();
        }

        const QByteArray qbaServer = mAuthPasswordServer.toLatin1();
        const gchar *gserver = qbaServer.data();

        if (mCurGKRID != 0) {
            gnome_keyring_item_delete_sync(NULL, mCurGKRID);
            mCurGKRID = 0;
        }

        if (gnome_keyring_set_network_password_sync(NULL, guser, NULL, gserver,
                                                    NULL, NULL, NULL, 0, gpass, &mCurGKRID)
            != GNOME_KEYRING_RESULT_OK) {
            qDebug("Bad result from gnome_keyring_set_network_password_sync!");
            return false;
        }
    }
    mSwService->onCredentialsUpdated();
    return true;
}

bool SwClientServiceConfig::validateConfigParams(const QHash<QString, QString> &params)
{
    switch(mAuthType) {
    case SwClientServiceConfig::AuthTypePassword:
        if (!params.contains(SW_GKR_KEY_PASSWORD))
            return false;
    case SwClientServiceConfig::AuthTypeUsername:
        if (!params.contains(SW_GKR_KEY_USERNAME))
            return false;
        break;
    case SwClientServiceConfig::AuthTypeFlickr:
        //With Flickr, it's all handled in the Flickr-specific functions,
        //nothing to do here...
        return true;
        break;
    case SwClientServiceConfig::AuthTypeFacebook:
        //With Facebook, it's all handled in the Flickr-specific functions,
        //nothing to do here...
        return true;
        break;
    case SwClientServiceConfig::AuthTypeOAuth:
        return true;
        break;
    case SwClientServiceConfig::AuthTypeCustom:
        //TODO
        qDebug()
            << QString(
                "Hit unimplemented SwClientServiceConfig::validateConfigParams"
                "with AuthTypeCustom: %1").arg(mCustomAuthType);
        return false;
        break;
    case SwClientServiceConfig::AuthTypeUnknown:
        qCritical() <<
            QString("Got AuthTypeUnknown in validateConfigParams for service %1"
                    ).arg(mSwService->getServiceName());
        return false;
        break;
    }
    return true;
}

bool SwClientServiceConfig::loadFromKeyFile()
{
    QString authType;

    mSettings->beginGroup(SW_KEY_GROUP);
    if (!mSettings->childKeys().contains(SW_KEY_NAME)
            || !mSettings->childKeys().contains(SW_KEY_AUTHTYPE)) {
        qCritical() << QString("Couldn't find either %1 or %2 keys in group %3 in keys file (%4) for service %5!").arg(
                SW_KEY_NAME, SW_KEY_AUTHTYPE, SW_KEY_GROUP, mSettings->fileName(), mSwService->getServiceName());
        mSettings->endGroup();
        return false;
    }

    authType = mSettings->value(SW_KEY_AUTHTYPE).toString();
    if (!mAuthTypeMap.contains(authType)) {
        mAuthType = AuthTypeCustom;
        mCustomAuthType = authType;
    } else {
        mAuthType = mAuthTypeMap.value(authType);
    }
    mAuthPasswordServer = mSettings->value(SW_KEY_AUTHPASSWORDSERVER).toString();
    mLink = mSettings->value(SW_KEY_LINK).toString();
    //First we try and grab the Display Name & Description for our particular
    //language_country locale name, and if it doesn't exist, try to grab it by
    //just the language code, and finally grab the default as a fallback.

    QString locale = QLocale::system().name();
    QString lang = locale.split("_").at(0);

    mDisplayName = mSettings->value(SW_KEY_NAME.append("[%1]").arg(locale)).toString();
    if (mDisplayName.isEmpty())
        mDisplayName = mSettings->value(SW_KEY_NAME.append("[%1]").arg(lang)).toString();
    if (mDisplayName.isEmpty())
        mDisplayName = mSettings->value(SW_KEY_NAME).toString();

    mDescription = mSettings->value(SW_KEY_DESCRIPTION.append("[%1]").arg(locale)).toStringList().join(" ");
    if (mDescription.isEmpty())
        mDescription = mSettings->value(SW_KEY_DESCRIPTION.append("[%1]").arg(lang)).toStringList().join(" ");
    if (mDescription.isEmpty())
        mDescription = mSettings->value(SW_KEY_DESCRIPTION).toStringList().join(" ");
    mSettings->endGroup();
    return true;
}

void SwClientServiceConfig::fillAuthTypeMap()
{
    mAuthTypeMap.insert("username", AuthTypeUsername);
    mAuthTypeMap.insert("password", AuthTypePassword);
    mAuthTypeMap.insert("flickr", AuthTypeFlickr);
    mAuthTypeMap.insert("oauth", AuthTypeOAuth);
    mAuthTypeMap.insert("oauth2-facebook", AuthTypeFacebook);
}

bool SwClientServiceConfig::loadConfigParamsForFlickr()
{
    if (mAuthType != SwClientServiceConfig::AuthTypeFlickr)
        return false;

    if (!sw_keystore_get_key_secret("flickr", &mFlickrData.apiKey, &mFlickrData.sharedSecret)) {
        qWarning("Flickr API key not installed! Can't configure!");
        return false;
    }

    mFlickrData.restProxy = flickr_proxy_new(mFlickrData.apiKey, mFlickrData.sharedSecret);
    if (!mFlickrData.restProxy) {
        qWarning("Couldn't get Flickr Proxy from flickr_proxy_new!");
        return false;
    }
    //TODO: figure out how to grab the version # from qmake/.pro file
    rest_proxy_set_user_agent(mFlickrData.restProxy, "libsocialweb-qt/0.0.1");

    //TODO - see if we need to async this or not...
    if (gnome_keyring_find_password_sync(&sFlickrSchema,
                                         &mFlickrData.token,
                                         "server", FLICKR_SERVER,
                                         "api-key", mFlickrData.apiKey,
                                         NULL) != GNOME_KEYRING_RESULT_OK) {
        //qDebug("Couldn't get Flickr token - we must be logged out!");
    } else {
        GError *error = NULL;
        RestProxyCall *call;
        flickr_proxy_set_token(FLICKR_PROXY(mFlickrData.restProxy), mFlickrData.token);
        call = rest_proxy_new_call(mFlickrData.restProxy);
        rest_proxy_call_set_function(call, "flickr.auth.checkToken");
        if (!rest_proxy_call_async(call, FlickrRestCallback, NULL, this, &error)) {
            qWarning("Error in rest_proxy_call_async: %s! Cannot check token.", error->message);
            g_error_free(error);
        }
    }


    return true;
}

void SwClientServiceConfig::flickrRestCallResult(RestProxyCall *call,
        const GError *error)
{
    if (error) {
        //TODO - figure out if rest_prxy_call_sync will ever return true while having error set.
        qWarning("Error from rest_proxy_call_async: %s! Cannot check token.", error->message);
    } else {
        flickrGotAuth(GetFlickrXmlFromRestCall(call));
    }
}

//TODO: this is begging to be library-ized for a C++ Rest library wrapper?
//This is a direct copy from Bisho panes/flickr.c, with a couple of tweaks - mainly switching to qWarning and getting rid of the goto...
//We really *have* to library-ize the standard functionality
RestXmlNode * SwClientServiceConfig::GetFlickrXmlFromRestCall(RestProxyCall *call)
{
    static RestXmlParser *parser = NULL;
    RestXmlNode *root;

    if (parser == NULL)
        parser = rest_xml_parser_new ();

    root = rest_xml_parser_parse_from_data (parser,
                                          rest_proxy_call_get_payload (call),
                                          rest_proxy_call_get_payload_length (call));

    if (root == NULL) {
        qWarning ("Invalid XML from Flickr:\n%s",
                   rest_proxy_call_get_payload (call));
    } else if (strcmp (root->name, "rsp") != 0) {
        qWarning ("Unexpected response from Flickr:\n%s",
                   rest_proxy_call_get_payload (call));
        rest_xml_node_unref (root);
        root = NULL;
    } else if (strcmp (rest_xml_node_get_attr (root, "stat"), "ok") != 0) {
        RestXmlNode *node;
        const char *msg;

        node = rest_xml_node_find (root, "err");
        msg = rest_xml_node_get_attr (node, "msg");

        qWarning ("Error from Flickr: %s", msg);

        rest_xml_node_unref (root);
        root = NULL;
    }

    g_object_unref (call);
    return root;
}

void SwClientServiceConfig::flickrGotAuth(RestXmlNode *node)
{
    if (node) {
        RestXmlNode *userNode = rest_xml_node_find(node, "user");
        const char *name = rest_xml_node_get_attr(userNode, "fullname");
        if (!name || (name[0] == '\0'))
            name = rest_xml_node_get_attr(userNode, "username");
        mConfigParams.insert(SW_GKR_KEY_USERNAME, QString(name));
        rest_xml_node_unref(node);
    } else {
        flickrDeleteLogin();
    }
    emit usernameChanged();
}

bool SwClientServiceConfig::loadConfigParamsForFacebook()
{
    qDebug() << "loading facebook config params";
    if (mAuthType != SwClientServiceConfig::AuthTypeFacebook)
        return false;

    if (!sw_keystore_get_key_secret("facebook", &mFacebookData.apiKey,
        &mFacebookData.client_secret)) {
        qWarning("Facebook API key not installed! Can't configure!");
        return false;
    }

    mSettings->beginGroup("OAuth2");
    QString redirect = mSettings->value("AuthRedirectUri").toString();
    mFacebookData.auth_endpoint = g_strdup(mSettings->value("AuthEndpoint").toString().toStdString().c_str());
    mFacebookData.base_url = g_strdup(mSettings->value("BaseUri").toString().toStdString().c_str());
    mFacebookData.authRedirectUrl = g_strdup(redirect.toStdString().c_str());
    qDebug() << "authredirecturl " << redirect;
    mSettings->endGroup();

    mFacebookData.restProxy = oauth2_proxy_new(mFacebookData.apiKey,
        mFacebookData.auth_endpoint, mFacebookData.base_url, FALSE);
    if (!mFacebookData.restProxy) {
        qWarning("Couldn't get Facebook Proxy from oauth2_proxy_new!");
        return false;
    }

    //TODO: figure out how to grab the version # from qmake/.pro file
    rest_proxy_set_user_agent(mFacebookData.restProxy, "libsocialweb-qt/0.0.1");

    //TODO - see if we need to async this or not...
    if (gnome_keyring_find_password_sync(&sFacebookSchema,
                                         &mFacebookData.token,
                                         "server", mFacebookData.base_url,
                                         "client-id", mFacebookData.apiKey,
                                         NULL) != GNOME_KEYRING_RESULT_OK) {
        qDebug("Couldn't get Facebook token - we must be logged out!");
    } else {
        gsize len = 0;
        guchar *token = g_base64_decode (mFacebookData.token, &len);
        mFacebookData.token = (char*)token;
        qDebug() << "got access_token " << mFacebookData.token;
        GError *error = NULL;
        RestProxyCall *call;
        oauth2_proxy_set_access_token (OAUTH2_PROXY (mFacebookData.restProxy),
            mFacebookData.token);
        call = rest_proxy_new_call(mFacebookData.restProxy);
        rest_proxy_call_set_function(call, "me");
        if (!rest_proxy_call_async(call, FacebookRestCallback, NULL, this, &error)) {
            qWarning("Error in rest_proxy_call_async: %s! Cannot get user info.", error->message);
            g_error_free(error);
        }
    }

    return true;
}

void SwClientServiceConfig::facebookRestCallResult(RestProxyCall *call,
        const GError *error)
{
    if (error) {
        //TODO - figure out if rest_prxy_call_async will ever return true while having error set.
        qWarning("Error from rest_proxy_call_async: %s! Cannot get user info.", error->message);
    } else {
        QString json = rest_proxy_call_get_payload(call);

        QJson::Parser parser;
        bool ok;
        QVariantMap result = parser.parse(json.toUtf8(), &ok).toMap();
        if (ok) {
            QString name = result.value("name").toString();
            mConfigParams.insert(SW_GKR_KEY_USERNAME, name);
            emit usernameChanged();

        } else {
            qWarning("Error parsing json reply");
        }
    }
}

void SwClientServiceConfig::facebookGotAuth(RestProxyCall *call)
{
    QString json = rest_proxy_call_get_payload(call);

    QJson::Parser parser;
    bool ok;
    QVariantMap result = parser.parse(json.toUtf8(), &ok).toMap();
    if (ok) {
        QString name = result.value("name").toString();
        mConfigParams.insert(SW_GKR_KEY_USERNAME, name);
        emit usernameChanged();
    } else {
        qWarning("Error parsing json reply");
    }
}

bool SwClientServiceConfig::loadConfigParamsForOAuth()
{
    if (mAuthType != SwClientServiceConfig::AuthTypeOAuth)
        return false;

    if (!sw_keystore_get_key_secret(mSwService->getServiceName().toStdString().c_str(),
        &mOAuthData.apiKey, &mOAuthData.sharedSecret)) {
        qWarning(QString("%1 API key not installed! Can't configure!").arg(mSwService->getServiceName()).toStdString().c_str());
        return false;
    }

    mSettings->beginGroup("OAuth");
    mOAuthData.base_url =
        g_strdup(mSettings->value("BaseURL").toString().toStdString().c_str());
    mOAuthData.request_token_function =
        g_strdup(mSettings->value("RequestTokenFunction").toString().toStdString().c_str());
    mOAuthData.authorize_function =
        g_strdup(mSettings->value("AuthoriseFunction").toString().toStdString().c_str());
    mOAuthData.access_token_function =
        g_strdup(mSettings->value("AccessTokenFunction").toString().toStdString().c_str());
    mOAuthData.callback_url =
        g_strdup(mSettings->value("Callback").toString().toStdString().c_str());
    mSettings->endGroup();

    mOAuthData.restProxy = oauth_proxy_new(mOAuthData.apiKey,
                              mOAuthData.sharedSecret, mOAuthData.base_url, FALSE);
    if (!mOAuthData.restProxy) {
        qWarning("Couldn't get OAuth Proxy from oauth_proxy_new!");
        return false;
    }

    //TODO: figure out how to grab the version # from qmake/.pro file
    rest_proxy_set_user_agent(mOAuthData.restProxy, "libsocialweb-qt/0.0.1");

    //TODO - see if we need to async this or not...
    if (gnome_keyring_find_password_sync(&sOAuthSchema,
                                         &mOAuthData.auth_token,
                                         "server", mOAuthData.base_url,
                                         "consumer-key", mOAuthData.apiKey,
        NULL) != GNOME_KEYRING_RESULT_OK) {
        qDebug() << "Couldn't get " << mSwService->getServiceName() <<
            " OAuth auth token - we must be logged out!";
    } else {
        // extract the two parts of the key
        QString string = mOAuthData.auth_token;
        QStringList split = string.split(' ');
        if (split.count() == 2) {
            QString accessToken = QByteArray::fromBase64(split[0].toUtf8());
            QString tokenSecret = QByteArray::fromBase64(split[1].toUtf8());
            qDebug() << "got decoded access token " << accessToken;
            qDebug() << "got decoded secret " << tokenSecret;

            mOAuthData.auth_token = g_strdup(accessToken.toStdString().c_str());
            mOAuthData.auth_token_secret = g_strdup(tokenSecret.toStdString().c_str());

            oauthGetUserInfo();
        }
    }

    return true;
}

void SwClientServiceConfig::oauthGetUserInfo()
{
    // Now we need to get the user info.  Check which web service we are
    // using, and make the appropriate rest call.
    if (mDisplayName == "SmugMug") {
        GError *error = NULL;
        RestProxy *proxy;
        RestProxyCall *call;

        proxy = oauth_proxy_new_with_token (mOAuthData.apiKey,
            mOAuthData.sharedSecret, mOAuthData.auth_token,
            mOAuthData.auth_token_secret,
            "https://secure.smugmug.com/services/api/rest/1.2.2/", FALSE);

        call = rest_proxy_new_call(proxy);
        rest_proxy_call_add_param (call, "method",
                                   "smugmug.auth.checkAccessToken");
        if (!rest_proxy_call_async(call, SmugmugRestCallback, NULL, this, &error)) {
            qWarning("Error in smugmug rest_proxy_call_async: %s! Cannot get user info.", error->message);
            g_error_free(error);
        }
    }
    else if (mDisplayName == "Photobucket") {
        GError *error = NULL;
        RestProxy *proxy;
        RestProxyCall *call;
        proxy = oauth_proxy_new_with_token (mOAuthData.apiKey,
                                            mOAuthData.sharedSecret, mOAuthData.auth_token,
                                            mOAuthData.auth_token_secret,
                                            mOAuthData.base_url, FALSE);
        call = rest_proxy_new_call(proxy);
        rest_proxy_call_set_function(call, "user/-");
        if (!rest_proxy_call_async(call, PhotobucketRestCallback, NULL, this, &error)) {
            qWarning("Error in Photobucket rest_proxy_call_async: %s! Cannot get user info.", error->message);
            g_error_free(error);
        }
    }
    else if (mDisplayName == "Vimeo") {
        GError *error = NULL;
        RestProxy *proxy;
        RestProxyCall *call;
        proxy = oauth_proxy_new_with_token (mOAuthData.apiKey,
                                            mOAuthData.sharedSecret, mOAuthData.auth_token,
                                            mOAuthData.auth_token_secret,
                                            "http://vimeo.com/api/rest/v2/", FALSE);
        call = rest_proxy_new_call(proxy);
        rest_proxy_call_add_param(call, "method", "vimeo.oauth.checkAccessToken");
        rest_proxy_call_add_param(call, "format", "xml");
        if (!rest_proxy_call_async(call, VimeoRestCallback, NULL, this, &error)) {
            qWarning("Error in Vimeo rest_proxy_call_async: %s! Cannot get user info.", error->message);
            g_error_free(error);
        }
    }
}

void SwClientServiceConfig::photobucketRestCallResult(RestProxyCall *call,
        const GError *error)
{
    if (error) {
        //TODO - figure out if rest_prxy_call_sync will ever return true while having error set.
        qWarning("Error with true call photobucket rest_proxy_call_async: %s! Cannot get user info.", error->message);
    } else {
        QString response = rest_proxy_call_get_payload(call);
        QDomDocument document;
        document.setContent(response);
        QDomNodeList elements = document.elementsByTagName("username");
        QDomNode userNode = elements.item(0);
        QString name = userNode.toElement().text();
        qDebug() << "photobucket username is " << name;
        mConfigParams.insert(SW_GKR_KEY_USERNAME, QString(name));
        emit usernameChanged();
    }
}

void SwClientServiceConfig::smugmugRestCallResult(RestProxyCall *call,
        const GError *error)
{
    if (error) {
        //TODO - figure out if rest_prxy_call_sync will ever return true while having error set.
        qWarning("Error with true call smugmug rest_proxy_call_async: %s! Cannot get user info.", error->message);
    } else {
        QString response = rest_proxy_call_get_payload(call);
        QDomDocument document;
        document.setContent(response);
        QDomNodeList elements = document.elementsByTagName("User");

        qDebug() << "got " << elements.count() << " User elements";
        QDomNode userNode = elements.item(0);
        QString name = userNode.attributes().namedItem("DisplayName").toAttr().value();
        qDebug() << "got DisplayName attribute of " << name;
        mConfigParams.insert(SW_GKR_KEY_USERNAME, QString(name));
        emit usernameChanged();
    }
}

void SwClientServiceConfig::vimeoRestCallResult(RestProxyCall *call,
        const GError *error)
{
    if (error) {
        //TODO - figure out if rest_prxy_call_sync will ever return true while having error set.
        qWarning("Error with true call vimeo rest_proxy_call_async: %s! Cannot get user info.", error->message);
    } else {
        QString response = rest_proxy_call_get_payload(call);
        QDomDocument document;
        document.setContent(response);
        QDomNodeList elements = document.elementsByTagName("user");

        qDebug() << "got " << elements.count() << " user elements";
        QDomNode userNode = elements.item(0);
        QString name = userNode.attributes().namedItem("display_name").toAttr().value();
        qDebug() << "got display_name attribute of " << name;
        mConfigParams.insert(SW_GKR_KEY_USERNAME, QString(name));
        emit usernameChanged();
    }
}
