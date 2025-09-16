// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QTEXTTOSPEECHENGINE_OHOS_H
#define QTEXTTOSPEECHENGINE_OHOS_H

#include <QtTextToSpeech/qtexttospeechengine.h>

#include <QtCore/qobject.h>
#include <QtCore/qstring.h>
#include <QtCore/qvariant.h>

QT_BEGIN_NAMESPACE

QTextToSpeechEngine *createQTextToSpeechEngineOhos(
    const QVariantMap &parameters, QObject *parent, QString *errorString);

QT_END_NAMESPACE

#endif
