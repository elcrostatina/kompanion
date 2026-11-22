// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>

namespace AppPaths {

QString profilesRoot();
QString profileDirectory(const QString &providerId);
QString faviconsDirectory();
QString chatDatabaseFile();
QString overridesDirectory();
QString downloadsDirectory();

}
