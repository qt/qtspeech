// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QDECLARATIVETEXTTOSPEECH_H
#define QDECLARATIVETEXTTOSPEECH_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtTextToSpeech/qtexttospeech.h>

#include <QtQml/qqml.h>
#include <QtQml/qqmlparserstatus.h>

QT_BEGIN_NAMESPACE

class QVoiceSelectorAttached;

class QDeclarativeTextToSpeech : public QTextToSpeech, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    QML_NAMED_ELEMENT(TextToSpeech)

public:
    explicit QDeclarativeTextToSpeech(QObject *parent = nullptr);

    Q_INVOKABLE QList<QVoice> findVoices(const QVariantMap &criteria) const;

    QVoiceSelectorAttached *m_voiceSelector = nullptr;

    void selectVoice();

protected:
    void classBegin() override;
    void componentComplete() override;

private:
    bool m_complete = false;
};

QT_END_NAMESPACE

#endif // QDECLARATIVETEXTTOSPEECH_H