// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only



#ifndef QTEXTTOSPEECH_P_H
#define QTEXTTOSPEECH_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qtexttospeech.h>
#include <qtexttospeechplugin.h>
#include <QMutex>
#include <QCborMap>
#include <QtCore/qhash.h>
#include <QtCore/private/qobject_p.h>

QT_BEGIN_NAMESPACE

class QTextToSpeech;
class QTextToSpeechPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QTextToSpeech)
public:
    QTextToSpeechPrivate(QTextToSpeech *speech);
    ~QTextToSpeechPrivate();

    void setEngineProvider(const QString &engine, const QVariantMap &params);
    static QMultiHash<QString, QCborMap> plugins(bool reload = false);

private:
    bool loadMeta();
    void loadPlugin();
    static void loadPluginMetadata(QMultiHash<QString, QCborMap> &list);
    QTextToSpeech *q_ptr;
    QTextToSpeechPlugin *m_plugin = nullptr;
    QTextToSpeechEngine *m_engine = nullptr;
    QString m_providerName;
    QCborMap m_metaData;
    static QMutex m_mutex;
};

QT_END_NAMESPACE

#endif
