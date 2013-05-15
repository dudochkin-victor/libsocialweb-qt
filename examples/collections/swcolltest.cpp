#include "swcolltest.h"

SwCollTest::SwCollTest(QObject *parent) :
    QObject(parent)
{
    //swClient->
    registerDataTypes();
    mArgs = QCoreApplication::arguments();
    swClient = new SwClient();
    //OrgMoblinLibsocialwebItemViewInterface * mView = swClient->openService("twitter", 20);
    QString rservice = mArgs.at(1);
    QString command = mArgs.at(2);
    qDebug() << "Services:" << swClient->getServices();

    SwClientService *swService = swClient->getService(rservice);
    if (swService->isConfigured() && (swService->credsState() == SwClientService::CredsValid)
            && swService->hasCollections()) {
        qDebug("Service %s is configured, valid, and has collections!", rservice.toStdString().c_str());
    } else {
        qDebug("Service %s is either unconfigured, invalid creds, or doesn't have collections interface!", rservice.toStdString().c_str());
        QCoreApplication::exit(-1);
    }
    ArrayOfUInt cTypes = swService->getCreatableTypes();
    qDebug() << QString("Creatable collection types: ") << cTypes;

    connect(swService,
            SIGNAL(collListRetrieved(SwClientService*,ArrayOfSwCollectionDetails,bool)),
            this,
            SLOT(onCollListRetrieved(SwClientService*,ArrayOfSwCollectionDetails,bool)));
    connect(swService,
            SIGNAL(collDetailsRetrieved(SwClientService*,SwCollectionDetails,bool)),
            this,
            SLOT(onCollDetailsRetrieved(SwClientService*,SwCollectionDetails,bool)));
    connect(swService,
            SIGNAL(collCreated(SwClientService*,QString,bool)),
            this,
            SLOT(onCollCreated(SwClientService*,QString,bool)));

    if (command == "getlist") {
        if (!swService->getCollList()) {
            qDebug("Failed to invoke swService->getCollList()!");
            QCoreApplication::exit(-1);
        }
    } else if (command == "getdetails") {
        if (mArgs.count() < 4) {
            qDebug("You must supply a collection ID for getdetails!");
            QCoreApplication::exit(-1);
            return;
        }
        QString collID = mArgs.at(3);
        qDebug("Attempting to retrieve details for collID %s", collID.toStdString().c_str());
        if (!swService->getCollDetails(collID)) {
            qDebug("Failed to invoke swService->getCollDetails(collID)!");
            QCoreApplication::exit(-1);
        }
    } else if (command == "create") {
        if (mArgs.count() < 5) {
            qDebug("You must supply a collection name and media types (|| of {1 = Collections, 2 = Photos, 4 = Videos}) for create!");
            QCoreApplication::exit(-1);
            return;
        }
        QString collName = mArgs.at(3);
        bool ok = false;
        SwClientService::CollectionMediaTypes collTypes = (SwClientService::CollectionMediaTypes)mArgs.at(4).toUInt(&ok);
        if (!ok || collTypes == 0 || collTypes > 7) {
            qDebug("You must supply a valid integer between 1 and 7 for the media types!");
            QCoreApplication::exit(-1);
            return;
        }
        QString collParentID = QString();
        if (mArgs.count() >= 6)
            collParentID = mArgs.at(5);
        qDebug("Attempting to create coll %s", collName.toStdString().c_str());
        SwParams emptyParms;
        if (!swService->createColl(collName, collTypes, collParentID, emptyParms)) {
            qDebug("Failed to invoke swService->createColl(...)!");
            QCoreApplication::exit(-1);
            return;
        }
    } else {
        qDebug() << QString("Invalid command %1").arg(command);
        QCoreApplication::exit(-1);
        return;
    }
}

//Public slots
void SwCollTest::CloseUpShop()
{
}

//Private slots

void SwCollTest::onCollCreated(SwClientService *svc, QString collID, bool error)
{
    if (error) {
        qDebug("Error in return of create!");
        QCoreApplication::exit(-2);
    }
    qDebug("Collection created, id: %s", collID.toStdString().c_str());
    QCoreApplication::exit();
}

void SwCollTest::onCollDetailsRetrieved(SwClientService *svc, SwCollectionDetails dets, bool error)
{
    if (error) {
        qDebug("Error in return of getdetails!");
        QCoreApplication::exit(-2);
    }
    qDebug("Collection details retrieved:");
    qDebug("\tID: %s", dets.collID.toStdString().c_str());
    qDebug("\tName: %s", dets.collName.toStdString().c_str());
    qDebug("\tParentID: %s", dets.collParentID.toStdString().c_str());
    qDebug("\tMedia Types: %u", dets.collMediaTypes);
    qDebug("\tItem Count: %u", dets.collItemCount);
    qDebug("\tAttribs:");
    SwParams::iterator i;
    for (i = dets.collAttributes.begin(); i != dets.collAttributes.end(); ++i)
        qDebug("\t\t %s : %s", i.key().toStdString().c_str(), i.value().toStdString().c_str());
    QCoreApplication::exit();
}

void SwCollTest::onCollListRetrieved(SwClientService *svc, ArrayOfSwCollectionDetails list, bool error)
{
    if (error) {
        qDebug("Error in return of getlist!");
        QCoreApplication::exit(-2);
    }
    qDebug("Count: %u", list.count());
    foreach(SwCollectionDetails dets, list) {
        qDebug("Collection details retrieved:");
        qDebug("\tID: %s", dets.collID.toStdString().c_str());
        qDebug("\tName: %s", dets.collName.toStdString().c_str());
        qDebug("\tParentID: %s", dets.collParentID.toStdString().c_str());
        qDebug("\tMedia Types: %u", dets.collMediaTypes);
        qDebug("\tItem Count: %u", dets.collItemCount);
        qDebug("\tAttribs:");
        SwParams::iterator i;
        for (i = dets.collAttributes.begin(); i != dets.collAttributes.end(); ++i)
            qDebug("\t\t %s : %s", i.key().toStdString().c_str(), i.value().toStdString().c_str());
    }

    QCoreApplication::exit();
}
