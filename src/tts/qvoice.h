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




#ifndef QVOICE_H
#define QVOICE_H

#include <QtTextToSpeech/qtexttospeech_global.h>
#include <QtCore/qshareddata.h>

QT_BEGIN_NAMESPACE

class QVoicePrivate;
class QVariant;

class Q_TEXTTOSPEECH_EXPORT QVoice
{
public:
    enum Gender {
        Male,
        Female,
        Unknown
    };

    enum Age {
        Child,
        Teenager,
        Adult,
        Senior,
        Other
    };

    QVoice();
    QVoice(const QVoice &other);
    ~QVoice();

    void operator=(const QVoice &other);

    bool operator==(const QVoice &other);
    bool operator!=(const QVoice &other);

    QString name() const;
    Gender gender() const;
    Age age() const;

    static QString genderName(QVoice::Gender gender);
    static QString ageName(QVoice::Age age);
private:
    QVoice(const QString &name, Gender gender, Age age, const QVariant &data);

    void setName(const QString &name);
    void setGender(Gender gender);
    void setAge(Age age);
    void setData(const QVariant &data);
    QVariant data() const;

    QSharedDataPointer<QVoicePrivate> d;
    friend class QTextToSpeechEngine;
};

QT_END_NAMESPACE

#endif

