/****************************************************************************
**
** Copyright (c) 2013 - 2021 Jolla Ltd.
** Contact: Raine Makelainen <raine.makelainen@jolla.com>
** Contact: Siteshwar Vashisht <siteshwar@gmail.com>
**
****************************************************************************/

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "declarativewebcontainer.h"
#include "persistenttabmodel.h"
#include "dbmanager.h"

PersistentTabModel::PersistentTabModel(int nextTabId, DeclarativeWebContainer *webContainer)
    : DeclarativeTabModel(nextTabId, webContainer)
{
    connect(DBManager::instance(), &DBManager::tabsAvailable,
            this, &PersistentTabModel::tabsAvailable);

    DBManager::instance()->getAllTabs();
}

PersistentTabModel::~PersistentTabModel()
{
}

void PersistentTabModel::tabsAvailable(const QList<Tab> &tabs)
{
    beginResetModel();
    int oldCount = count();

    // Clear always previous tabs
    clear();

    if (tabs.count() > 0) {
        m_tabs = tabs;
        QString activeTabId = DBManager::instance()->getSetting("activeTabId");
        bool ok = false;
        int tabId = activeTabId.toInt(&ok);
        int index = findTabIndex(tabId);
        if (index >= 0) {
            m_activeTabId = tabId;
        } else {
            // Fallback for browser update as this "activeTabId" is a new setting.
            m_activeTabId = m_tabs.at(0).tabId();
        }
        emit activeTabIndexChanged();
    }

    endResetModel();

    if (count() != oldCount) {
        emit countChanged();
    }

    int maxTabId(0);
    for (const Tab &tab : tabs) {
        if (maxTabId < tab.tabId()) {
            maxTabId = tab.tabId();
        }
    }

    if (m_nextTabId != maxTabId + 1) {
        m_nextTabId = maxTabId + 1;
    }

    // Startup should be synced to this.
    if (!m_loaded) {
        m_loaded = true;
        emit loadedChanged();
    }

    connect(this, &PersistentTabModel::activeTabIndexChanged,
            this, &PersistentTabModel::saveActiveTab, Qt::UniqueConnection);
}

void PersistentTabModel::createTab(const Tab &tab) {
    DBManager::instance()->createTab(tab);
}

void PersistentTabModel::updateTitle(int tabId, const QString &url, const QString &title)
{
    DBManager::instance()->updateTitle(tabId, url, title);
}

void PersistentTabModel::removeTab(int tabId)
{
    DBManager::instance()->removeTab(tabId);
}

void PersistentTabModel::updateRequestedUrl(int tabId, const QString &requestedUrl, const QString &resolvedUrl)
{
    DBManager::instance()->updateUrl(tabId, requestedUrl, resolvedUrl);
}

void PersistentTabModel::navigateTo(int tabId, const QString &url, const QString &title, const QString &path) {
    Q_UNUSED(title)
    Q_UNUSED(path)

    DBManager::instance()->navigateTo(tabId, url, "", "");
}

void PersistentTabModel::updateThumbPath(int tabId, const QString &path)
{
    DBManager::instance()->updateThumbPath(tabId, path);
}

void PersistentTabModel::saveActiveTab() const
{
    DBManager::instance()->saveSetting("activeTabId", QString("%1").arg(m_activeTabId));
}
