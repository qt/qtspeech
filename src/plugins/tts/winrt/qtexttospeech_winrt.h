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

#ifndef QTEXTTOSPEECHENGINE_WINRT_H
#define QTEXTTOSPEECHENGINE_WINRT_H

#include <QtTextToSpeech/qtexttospeechengine.h>
#include <QtTextToSpeech/qvoice.h>

#include <QtCore/QObject>
#include <QtCore/QVector>
#include <QtCore/QString>
#include <QtCore/QLocale>
#include <QtCore/QScopedPointer>
#include <QtCore/qt_windows.h>
#include <wrl.h>

namespace ABI {
    namespace Windows {
        namespace Media {
            namespace SpeechSynthesis {
                struct IVoiceInformation;
            }
        }
    }
}

QT_BEGIN_NAMESPACE

class QTextToSpeechEngineWinRTPrivate;

class QTextToSpeechEngineWinRT : public QTextToSpeechEngine
{
    Q_OBJECT

public:
    QTextToSpeechEngineWinRT(const QVariantMap &parameters, QObject *parent);
    ~QTextToSpeechEngineWinRT();

    QVector<QLocale> availableLocales() const Q_DECL_OVERRIDE;
    QVector<QVoice> availableVoices() const Q_DECL_OVERRIDE;
    void say(const QString &text) Q_DECL_OVERRIDE;
    void stop() Q_DECL_OVERRIDE;
    void pause() Q_DECL_OVERRIDE;
    void resume() Q_DECL_OVERRIDE;
    double rate() const Q_DECL_OVERRIDE;
    bool setRate(double rate) Q_DECL_OVERRIDE;
    double pitch() const Q_DECL_OVERRIDE;
    bool setPitch(double pitch) Q_DECL_OVERRIDE;
    QLocale locale() const Q_DECL_OVERRIDE;
    bool setLocale(const QLocale &locale) Q_DECL_OVERRIDE;
    double volume() const Q_DECL_OVERRIDE;
    bool setVolume(double volume) Q_DECL_OVERRIDE;
    QVoice voice() const Q_DECL_OVERRIDE;
    bool setVoice(const QVoice &voice) Q_DECL_OVERRIDE;
    QTextToSpeech::State state() const Q_DECL_OVERRIDE;

public slots:
    void checkElementState();
private:
    void init();
    QVoice createVoiceForInformation(Microsoft::WRL::ComPtr<ABI::Windows::Media::SpeechSynthesis::IVoiceInformation> info) const;

    QScopedPointer<QTextToSpeechEngineWinRTPrivate> d_ptr;
    Q_DECLARE_PRIVATE(QTextToSpeechEngineWinRT)
};

QT_END_NAMESPACE

#endif
