#include "swclientapp.h"

SwClientApp::SwClientApp(QObject *parent) :
    QObject(parent)
{
    //swClient->
    swClient = new SwClient();
    //OrgMoblinLibsocialwebItemViewInterface * mView = swClient->openService("twitter", 20);
    qDebug() << "Services:" << swClient->getServices();
    foreach (QString service, swClient->getServices()) {
        qDebug() << QString("Opening service for service %1").arg(service);
        SwClientService *swService = swClient->getService(service);

        connect(swService,
                SIGNAL(DynCapsChanged(SwClientService*,QStringList)),
                this,
                SLOT(onDynCapsChanged(SwClientService*,QStringList)));

        connect(swService,
                SIGNAL(UserChanged(SwClientService*)),
                this,
                SLOT(onUserChanged(SwClientService*)));

        qDebug() << QString("Static props for %1").arg(service);

        QStringList props = swService->getStaticCaps();
        qDebug() << props;

        qDebug() << QString("Dynamic props for %1").arg(service);
        props = swService->getDynCaps();
        qDebug() << props;

        SwClientServiceConfig *svcCfg = swService->getServiceConfig();
        if (!svcCfg) {
            qDebug("Didn't get a valid service config!");
        } else {
            qDebug("ServiceConfig props for service:");
            qDebug() << QString("\tAuthType: %1").arg(svcCfg->getAuthtype());
            qDebug() << QString("\tCustom Authtype: %1").arg(svcCfg->getCustomAuthtype());
            qDebug() << QString("\tDisplay Name: %1").arg(svcCfg->getDisplayName());
            qDebug() << QString("\tDescription: %1").arg(svcCfg->getDescription());
            qDebug() << QString("\tLink: %1").arg(svcCfg->getLink());
            qDebug() << QString("\tIcon file: %1").arg(svcCfg->getIconPath());
            QHash<QString, QString> params = svcCfg->getConfigParams();
            qDebug() << QString("\tConfig Params: ") << params;
            /*qDebug("Trying to change username");
            params.insert("user", "ausmusj");
            if (svcCfg->setConfigParams(params))
                qDebug("setConfigParams successfull!");
            else
                qDebug("setConfigParams failed!");
		*/
        }

        if (swService->isConfigured()
                && (swService->credsState() == SwClientService::CredsValid)
                && (swService->hasOpenView())) {

            SwClientItemView *view = swService->openView("feed");
            if (view) {
                swViews.insert(service, view);
            } else {
                qDebug("Didn't get a valid view!");
            }
        }
    }
    if (swViews.count()) {
        foreach (SwClientItemView *view, swViews) {
            connect(view,
                    SIGNAL(ItemsAdded(QList<SwClientItem*>)),
                    this,
                    SLOT(onItemsAdded(QList<SwClientItem*>)));
            view->startView();
        }
    }
}

//Public slots
void SwClientApp::CloseUpShop()
{
    foreach (SwClientItemView *swView, swViews) {
        swView->stopView();
        swView->closeView();
    }
}

//Private slots
void SwClientApp::onItemsAdded(QList<SwClientItem *> itemsAdded)
{
    foreach(SwClientItem *item, itemsAdded) {
        qDebug("Received a new item!");
        qDebug() << QString("\tService: %1").arg(item->getServiceName());
        qDebug() << QString("\tUUID: %1").arg(item->getSwUUID());
        qDebug() << QString("\tDate: %1").arg(item->getDateTime().toString());
        qDebug() << QString("\tType: %1").arg(item->getItemType());
        qDebug() << QString("\tCached: %1").arg(item->getCached());
        qDebug("\tProps:");
        QHash<QString, QString> propsHash = item->getSwItemProps();
        QHash<QString, QString>::iterator i;
        for (i = propsHash.begin(); i != propsHash.end(); ++i) {
            qDebug() << QString("\t\t%1 - %2").arg(i.key(), i.value());
        }
        qDebug("---------------------------------\n");
    }
}

void SwClientApp::onDynCapsChanged(SwClientService *swService, QStringList newDynCaps)
{
    qDebug() << QString("Dyn Caps Changed signal for service %1:").arg(swService->getServiceName());
    qDebug() << newDynCaps;
    if (swService->isConfigured()
            && (swService->credsState() == SwClientService::CredsValid)
            && (swService->hasOpenView())) {
        if (!swViews.contains(swService->getServiceName())) {
            SwClientItemView *view = swService->openView("feed");
            if (view) {
                swViews.insert(swService->getServiceName(), view);
                connect(view,
                        SIGNAL(ItemsAdded(QList<SwClientItem*>)),
                        this,
                        SLOT(onItemsAdded(QList<SwClientItem*>)));
                qDebug() << QString("Added new view for %1 based on DynCaps change!").arg(swService->getServiceName());
            } else {
                qDebug("Didn't get a valid view!");
            }
        }
    } else {
        if (swViews.contains(swService->getServiceName())) {
            SwClientItemView *view = swViews.value(swService->getServiceName());
            view->stopView();
            view->closeView();
            swViews.remove(swService->getServiceName());
            delete view;
            qDebug() << QString("Stopped/closed/removed view for %1 based on DynCaps change!").arg(swService->getServiceName());
        }
    }
}

void SwClientApp::onUserChanged(SwClientService *swService)
{
    qDebug() << QString("User Changed signal for service %1").arg(swService->getServiceName());
}
