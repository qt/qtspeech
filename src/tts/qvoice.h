/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWidgets module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/



#ifndef QVOICE_H
#define QVOICE_H

#include <QtTextToSpeech/qtexttospeech_global.h>
#include <QtCore/qshareddata.h>

QT_BEGIN_NAMESPACE

class QVoicePrivate;

class QTEXTTOSPEECH_EXPORT QVoice
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

    QString name() const;
    Gender gender() const;
    Age age() const;
private:
    QVoice(const QString &name, Gender gender, Age age, const QVariant &data);

    void setName(const QString &name);
    void setGender(Gender gender);
    void setAge(Age age);
    void setData(const QVariant &data);
    QVariant data() const;

    QSharedDataPointer<QVoicePrivate> d;

    friend class QTextToSpeechPrivateSpeechDispatcher;
    friend class QTextToSpeechPrivateMac;
};

QT_END_NAMESPACE

#endif

