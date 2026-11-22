// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/SiteDataScripts.h"

QString SiteDataScripts::clearPageStorage()
{
    return QStringLiteral(R"js((async () => {
  try { localStorage.clear(); sessionStorage.clear(); } catch (error) {}
  try {
    const databases = await indexedDB.databases();
    databases.forEach(database => indexedDB.deleteDatabase(database.name));
  } catch (error) {}
  try {
    const cacheNames = await caches.keys();
    cacheNames.forEach(name => caches.delete(name));
  } catch (error) {}
  try {
    const registrations = await navigator.serviceWorker.getRegistrations();
    registrations.forEach(registration => registration.unregister());
  } catch (error) {}
})();)js");
}
