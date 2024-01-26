// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "modules_loader.h"
#include "plugin_manager.h"
#include "base_module_interface.h"
#include "public_func.h"
#include "dconfig_helper.h"

#include <QDir>
#include <QLibrary>
#include <QPluginLoader>
#include <QApplication>

const QString ModulesDir = "/usr/lib/dde-session-shell/modules";
const QString LOWEST_VERSION = "1.1.0";
const QString LoginType = "Login";
const QString TrayType = "Tray";

ModulesLoader::ModulesLoader(QObject *parent)
    : QThread(parent)
{
}

ModulesLoader::~ModulesLoader()
{
    quit();
    wait();
}

ModulesLoader &ModulesLoader::instance()
{
    static ModulesLoader modulesLoader;
    return modulesLoader;
}

void ModulesLoader::run()
{
    findModule(ModulesDir);
}

void ModulesLoader::findModule(const QString &path)
{
    QDir dir(path);
    if (!dir.exists()) {
        qCDebug(DDE_SHELL) << "Find module, Path: " << path << "is not exists.";
        return;
    }

    auto blackList = DConfigHelper::instance()->getConfig("pluginBlackList", QStringList()).toStringList();
    qCInfo(DDE_SHELL) << "Find module, plugin black list:" << blackList;
    const QFileInfoList modules = dir.entryInfoList();
    for (QFileInfo module : modules) {
        const QString path = module.absoluteFilePath();
        if (!QLibrary::isLibrary(path)) {
            continue;
        }
        qCInfo(DDE_SHELL) << "About to process " << module;
        QPluginLoader loader(path);

        // 检查兼容性
        const QJsonObject meta = loader.metaData().value("MetaData").toObject();
        const QString version = meta.value("api").toString();
        // 版本过低则不加载，可能会导致登录器崩溃
        if (!checkVersion(version, LOWEST_VERSION)) {
            qCWarning(DDE_SHELL) << "The module version is too low.";
            continue;
        }

        QString pluginType = meta.value("pluginType").toString();

        // 分类加载
        if ((pluginType == LoginType && !m_loadLoginModule) || (pluginType == TrayType && m_loadLoginModule)) {
            continue;
        }

        if (pluginType.isEmpty()) {
            qCWarning(DDE_SHELL) << "plugin has no pluginType in json file, will not load:" << module;
            continue;
        }

        auto *moduleInstance = dynamic_cast<dss::module::BaseModuleInterface *>(loader.instance());
        if (!moduleInstance) {
            qCWarning(DDE_SHELL) << "Load plugin failed, error:" << loader.errorString();
            loader.unload();
            continue;
        }

        qCInfo(DDE_SHELL) << "Current plugin key:" << moduleInstance->key();
        if (blackList.contains(moduleInstance->key())) {
            qCInfo(DDE_SHELL) << "The plugin is in black list, won't be loaded.";
            loader.unload();
            continue;
        }

        int loadPluginType = moduleInstance->loadPluginType();
        if (loadPluginType != dss::module::BaseModuleInterface::Load) {
            qCInfo(DDE_SHELL) << "The plugin dose not want to be loaded.";
            loader.unload();
            continue;
        }

        if (PluginManager::instance()->contains(moduleInstance->key())) {
            qCWarning(DDE_SHELL) << "The plugin has been loaded.";
            loader.unload();
            continue;
        }

        QObject *obj = dynamic_cast<QObject*>(moduleInstance);
        if (obj)
            obj->moveToThread(qApp->thread());

        PluginManager::instance()->addPlugin(moduleInstance, version);
    }
}
