/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
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

#ifndef QVOICE_H
#define QVOICE_H

#include <QtTextToSpeech/qtexttospeech_global.h>
#include <QtCore/qshareddata.h>
#include <QtCore/qmetatype.h>
#include <QtCore/qvariant.h>

QT_BEGIN_NAMESPACE

class QVoicePrivate;

QT_DECLARE_QESDP_SPECIALIZATION_DTOR_WITH_EXPORT(QVoicePrivate, Q_TEXTTOSPEECH_EXPORT)

class Q_TEXTTOSPEECH_EXPORT QVoice
{
    Q_GADGET
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(Gender gender READ gender CONSTANT)
    Q_PROPERTY(Age age READ age CONSTANT)

public:
    enum Gender {
        Male,
        Female,
        Unknown
    };
    Q_ENUM(Gender)

    enum Age {
        Child,
        Teenager,
        Adult,
        Senior,
        Other
    };
    Q_ENUM(Age)

    QVoice();
    ~QVoice();
    QVoice(const QVoice &other) noexcept;
    QVoice &operator=(const QVoice &other) noexcept;
    QVoice(QVoice &&other) noexcept = default;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_PURE_SWAP(QVoice)

    void swap(QVoice &other) noexcept
    { d.swap(other.d); }

    friend inline bool operator==(const QVoice &lhs, const QVoice &rhs) noexcept
    { return lhs.isEqual(rhs); }
    friend inline bool operator!=(const QVoice &lhs, const QVoice &rhs) noexcept
    { return !lhs.isEqual(rhs); }

    QString name() const;
    Gender gender() const;
    Age age() const;

    static QString genderName(QVoice::Gender gender);
    static QString ageName(QVoice::Age age);

private:
    struct EngineData
    {
        QString engineName;
        QVariant data;
    };

    QVoice(const QString &name, Gender gender, Age age, const EngineData &data);
    bool isEqual(const QVoice &other) const noexcept;

    QVariant data() const { return engineData().data; }
    EngineData engineData() const;

    QExplicitlySharedDataPointer<QVoicePrivate> d;
    friend class QVoicePrivate;
    friend class QTextToSpeechEngine;
    friend Q_TEXTTOSPEECH_EXPORT QDebug operator<<(QDebug, const QVoice &);
    friend Q_TEXTTOSPEECH_EXPORT QDataStream &operator<<(QDataStream &, const QVoice &);
    friend Q_TEXTTOSPEECH_EXPORT QDataStream &operator>>(QDataStream &, QVoice &);
};

#ifndef QT_NO_DATASTREAM
Q_TEXTTOSPEECH_EXPORT QDataStream &operator<<(QDataStream &, const QVoice &);
Q_TEXTTOSPEECH_EXPORT QDataStream &operator>>(QDataStream &, QVoice &);
#endif

#ifndef QT_NO_DEBUG_STREAM
Q_TEXTTOSPEECH_EXPORT QDebug operator<<(QDebug, const QVoice &);
#endif

Q_DECLARE_SHARED(QVoice)

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QVoice)
Q_DECLARE_METATYPE(QVoice::Age)
Q_DECLARE_METATYPE(QVoice::Gender)

#endif
