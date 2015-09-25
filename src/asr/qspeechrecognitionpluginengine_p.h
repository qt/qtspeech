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

#ifndef QSPEECHRECOGNITIONPLUGINENGINE_P_H
#define QSPEECHRECOGNITIONPLUGINENGINE_P_H

#include <QtCore/QObject>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QLocale>
#include <QtCore/QLoggingCategory>
#include <QtCore/private/qobject_p.h>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(lcSpeechAsr)

class QSpeechRecognitionPluginEnginePrivate : public QObjectPrivate
{
public:
    static QStringList findFilesWithWildcards(const QString &filePath);
    QString m_name;
    QVariantMap m_parameters;
};

class QSpeechRecognitionDebugAudioFile : public QFile
{
    Q_OBJECT
public:
    QSpeechRecognitionDebugAudioFile(const QString &filePath, int sampleRate, int sampleSize, int channelCount);
    ~QSpeechRecognitionDebugAudioFile();
    bool open(OpenMode flags = QIODevice::WriteOnly) Q_DECL_OVERRIDE;
private slots:
    void onAboutToClose();
private:
    bool writeWavHeader();
    int m_sampleRate;
    int m_sampleSize;
    int m_channelCount;
    qint64 m_totalSizeOffset;
    qint64 m_dataSizeOffset;
    qint64 m_dataOffset;
};

QT_END_NAMESPACE

#endif // QSPEECHRECOGNITIONPLUGINENGINE_P_H
