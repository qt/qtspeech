/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSpeech module of the Qt Toolkit.
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


#include "qvoice.h"
#include "qvoice_p.h"

QT_BEGIN_NAMESPACE

QVoice::QVoice()
{
    d = new QVoicePrivate();
}

QVoice::QVoice(const QVoice &other)
    :d(other.d)
{
}

QVoice::QVoice(const QString &name, Gender gender, Age age, const QVariant &data)
    :d(new QVoicePrivate(name, gender, age, data))
{
}

QVoice::~QVoice()
{
}

void QVoice::operator=(const QVoice &other)
{
    d->name = other.d->name;
    d->gender = other.d->gender;
    d->age = other.d->age;
    d->data = other.d->data;
}

void QVoice::setName(const QString &name)
{
    d->name = name;
}

void QVoice::setGender(Gender gender)
{
    d->gender = gender;
}

void QVoice::setAge(Age age)
{
    d->age = age;
}

void QVoice::setData(const QVariant &data)
{
    d->data = data;
}

QString QVoice::name() const
{
    return d->name;
}

QVoice::Age QVoice::age() const
{
    return d->age;
}

QVoice::Gender QVoice::gender() const
{
    return d->gender;
}

QVariant QVoice::data() const
{
    return d->data;
}

QT_END_NAMESPACE
