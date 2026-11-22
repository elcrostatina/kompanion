// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QMetaType>
#include <QString>

struct ClientError {
    enum class Kind {
        ServerUnreachable,
        Unauthorized,
        NoModelLoaded,
        ServerError,
        InvalidResponse,
    };

    Kind kind = Kind::ServerError;
    QString message;
    int httpStatus = 0;
};

Q_DECLARE_METATYPE(ClientError)
