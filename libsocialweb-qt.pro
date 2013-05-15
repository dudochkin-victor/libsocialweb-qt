VERSION = 0.1.14
unix { 
    PREFIX = /usr
    LIBDIR = $$PREFIX/lib
    INCLUDEDIR = $$PREFIX/include/libsocialweb
}
QT += declarative dbus xml
TEMPLATE = lib
TARGET = socialweb-qt
MOC_DIR = .moc
OBJECTS_DIR = .obj
MGEN_OUTDIR = .gen
DEPENDPATH += .
INCLUDEPATH += /usr/include/glib-2.0 \
    /usr/include/libsocialweb \
    /usr/include/gnome-keyring-1 \
    /usr/include/rest-0.7
CONFIG += qdbus link_pkgconfig \
	debug
PKGCONFIG += glib-2.0 libsocialweb-keystore rest-0.7 rest-extras-0.7 gnome-keyring-1 QJson
SUBDIRS += tests

# Having to do this by hand right now, as qdbusxml2cpp has some bugs in it...
system(./generate-interfaces.sh)

# DBUS_INTERFACES += interfaces/lastfm.xml \
# interfaces/sw-avatar.xml \
# interfaces/sw-core.xml \
# interfaces/sw-item-view.xml \
# interfaces/sw-query.xml \
# interfaces/sw-service.xml \
# interfaces/sw-status-update.xml \
# interfaces/sw-photo-upload.xml \
# interfaces/sw-banishable.xml
INSTALL_HEADERS = swclientdbustypes.h \
    swclient.h \
    swclientitemview.h \
    swclientitem.h \
    swclientitemmodel.h \
    swclientserviceconfig.h \
    swclientitemsortfilterproxymodel.h \
    swclientservice.h \
    swclientitemaggregatemodel.h
HEADERS += interfaces/*_interface.h \
    $$INSTALL_HEADERS
SOURCES += interfaces/*_interface.cpp \
    swclient.cpp \
    swclientdbustypes.cpp \
    swclientitemview.cpp \
    swclientservice.cpp \
    swclientitem.cpp \
    swclientitemmodel.cpp \
    swclientserviceconfig.cpp \
    swclientitemsortfilterproxymodel.cpp \
    swclientitemaggregatemodel.cpp
target.path = $$LIBDIR
headers.files = $$INSTALL_HEADERS
headers.path = $$INCLUDEDIR/libsocialweb-qt
pcfile.files = libsocialweb-qt.pc
pcfile.path = $$LIBDIR/pkgconfig
INSTALLS += target \
    headers \
    pcfile
OTHER_FILES += README \
    interfaces/*.xml

PROJECT_NAME=libsocialweb-qt
dist.commands += rm -fR $${PROJECT_NAME}-$${VERSION} &&
dist.commands += git clone . $${PROJECT_NAME}-$${VERSION} &&
dist.commands += rm -fR $${PROJECT_NAME}-$${VERSION}/.git &&
#dist.commands += mkdir -p $${PROJECT_NAME}-$${VERSION}/ts &&
#dist.commands += lupdate $${TRANSLATIONS} -ts $${PROJECT_NAME}-$${VERSION}/ts/$${PROJECT_NAME}.ts &&
dist.commands += tar jcpvf $${PROJECT_NAME}-$${VERSION}.tar.bz2 $${PROJECT_NAME}-$${VERSION}
QMAKE_EXTRA_TARGETS += dist
