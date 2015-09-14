/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Speech module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QSPEECHRECOGNITIONPLUGINENGINE_H
#define QSPEECHRECOGNITIONPLUGINENGINE_H

#include "qspeechrecognition.h"

#include <QtCore/QObject>
#include <QtCore/QLocale>
#include <QtCore/QDir>

QT_BEGIN_NAMESPACE

class QSpeechRecognitionPluginGrammar;
class QSpeechRecognitionPluginEnginePrivate;

class QSPEECHRECOGNITION_EXPORT QSpeechRecognitionPluginEngine : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QSpeechRecognitionPluginEngine)

public:
    enum {
        NO_SESSION = -1
    };

    explicit QSpeechRecognitionPluginEngine(const QString &name, const QVariantMap &parameters, const QList<QString> &supportedParameters, QObject *parent = 0);
    ~QSpeechRecognitionPluginEngine();
    QSpeechRecognition::Error setParameter(const QString &key, const QVariant &value, QString *errorString);
    const QString &name() const;
    const QVariantMap &parameters() const;

    virtual QSpeechRecognitionPluginGrammar *createGrammar(const QString &name, const QUrl &location, QString *errorString) = 0;
    virtual QSpeechRecognition::Error setGrammar(QSpeechRecognitionPluginGrammar *grammar, QString *errorString) = 0;
    virtual QSpeechRecognition::Error startListening(int session, bool mute, QString *errorString) = 0;
    virtual void stopListening(qint64 timestamp) = 0;
    virtual void abortListening() = 0;
    virtual void unmute(qint64 timestamp) = 0;
    virtual void reset() = 0;
    virtual bool process() = 0;

Q_SIGNALS:
    void requestProcess();
    void requestStop(int session, qint64 timestamp);
    void result(int session, const QSpeechRecognitionPluginGrammar *grammar, const QVariantMap &resultData);
    void error(int session, QSpeechRecognition::Error error, const QVariantMap &parameters);
    void attributeUpdated(int session, const QString &key, const QVariant &value);

protected:
    QLocale locale() const;
    QDir resourceDirectory() const;
    QDir dataDirectory() const;
    QUrl dictionaryLocation() const;
    int audioSampleRate() const;
    QString audioInputFile() const;
    QString localizedFilePath(const QString &filePath) const;
    QFile *openDebugWavFile(const QString &filePath, int sampleRate, int sampleSize, int channelCount);

    virtual QSpeechRecognition::Error updateParameter(const QString &key, const QVariant &value, QString *errorString) = 0;
};

QT_END_NAMESPACE

#endif
