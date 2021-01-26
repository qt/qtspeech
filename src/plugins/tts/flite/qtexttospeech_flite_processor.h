/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Speech module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
****************************************************************************/

#ifndef QTEXTTOSPEECHPROCESSOR_FLITE_H
#define QTEXTTOSPEECHPROCESSOR_FLITE_H

#include "../common/qtexttospeechprocessor_p.h"

#include "qtexttospeechengine.h"
#include "qvoice.h"

#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtCore/QSharedPointer>
#include <QtCore/QMutex>

#include <flite/flite.h>

QT_BEGIN_NAMESPACE

// This is a reference counted singleton class.
// The instance is automatically deleted when no users remain.
class QTextToSpeechProcessorFlite : public QTextToSpeechProcessor {
    Q_OBJECT

public:
    static QSharedPointer<QTextToSpeechProcessorFlite> instance();
    ~QTextToSpeechProcessorFlite() override;
    const QVector<VoiceInfo> &voices() const override;

private:
    QTextToSpeechProcessorFlite();
    static int fliteOutputCb(const cst_wave *w, int start, int size,
                            int last, cst_audio_streaming_info *asi);
    int fliteOutput(const cst_wave *w, int start, int size,
                    int last, cst_audio_streaming_info *asi);
    int processText(const QString &text, int voiceId) override;
    void setRateForVoice(cst_voice *voice, float rate);
    void setPitchForVoice(cst_voice *voice, float pitch);
    bool init();
    void deinit();

private:
    struct FliteVoice {
        cst_voice *vox;
        void (*unregister_func)(cst_voice *vox);
        QString name;
        QString locale;
        QVoice::Gender gender;
        QVoice::Age age;
    };
    static QWeakPointer<QTextToSpeechProcessorFlite> m_instance;
    static QMutex m_instanceLock;
    bool m_initialized;
    QVector<VoiceInfo> m_voices;
    QVector<FliteVoice> m_fliteVoices;
    int m_currentVoice;
};

QT_END_NAMESPACE

#endif
