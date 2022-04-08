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



#include "qvoice.h"
#include "qvoice_p.h"
#include "qtexttospeech.h"

QT_BEGIN_NAMESPACE

QT_DEFINE_QESDP_SPECIALIZATION_DTOR(QVoicePrivate)

/*!
  \class QVoice
  \brief The QVoice class represents a particular voice.
  \inmodule QtSpeech
*/

/*!
    \enum QVoice::Age

    The age of a voice.

    \value Child    Voice of a child
    \value Teenager Voice of a teenager
    \value Adult    Voice of an adult
    \value Senior   Voice of a senior
    \value Other    Voice of unknown age
*/

/*!
    \enum QVoice::Gender

    The gender of a voice.

    \value Male    Voice of a male
    \value Female  Voice of a female
    \value Unknown Voice of unknown gender
*/

/*!
    Constructs an empty QVoice.
*/
QVoice::QVoice()
    : d(nullptr)
{
}

/*!
    Copy-constructs a QVoice from \a other.
*/
QVoice::QVoice(const QVoice &other) noexcept
    : d(other.d)
{}

/*!
    Destroys the QVoice instance.
*/
QVoice::~QVoice()
{}

/*!
    \fn QVoice::QVoice(QVoice &&other)

    Constructs a QVoice object by moving from \a other.
*/

/*!
    \fn QVoice &QVoice::operator=(QVoice &&other)
    Moves \a other into this QVoice object.
*/

/*!
    Assigns \a other to this QVoice object.
*/
QVoice &QVoice::operator=(const QVoice &other) noexcept
{
    d = other.d;
    return *this;
}

/*!
    \internal
*/
QVoice::QVoice(const QString &name, Gender gender, Age age, const QVariant &data)
    :d(new QVoicePrivate(name, gender, age, data))
{
}


/*!
    \internal
    Compares the \l name, \l gender, and \l age of this voice with \a other.
    Returns \c true if all of them match.
*/
bool QVoice::isEqual(const QVoice &other) const noexcept
{
    if (d == other.d)
        return true;
    if (!d || !other.d)
        return false;

    return d->name == other.d->name
        && d->gender == other.d->gender
        && d->age == other.d->age
        && d->data == other.d->data;
}

/*!
    \fn bool QVoice::operator==(const QVoice &lhs, const QVoice &rhs)
    \return whether the \a lhs voice and the \a rhs voice are identical.

    Two voices are identical if \l name, \l gender, and \l age are identical.
*/

/*!
    \fn bool QVoice::operator!=(const QVoice &lhs, const QVoice &rhs)
    \return whether the \a lhs voice and the \a rhs voice are different.
*/

/*!
   Returns the name of a voice.
*/
QString QVoice::name() const
{
    return d ? d->name : QString();
}

/*!
   Returns the gender of a voice.
*/
QVoice::Gender QVoice::gender() const
{
    return d ? d->gender : QVoice::Unknown;
}

/*!
   Returns the age of a voice.
*/
QVoice::Age QVoice::age() const
{
    return d ? d->age : QVoice::Other;
}

/*!
    \internal
*/
QVariant QVoice::data() const
{
    return d ? d->data : QVariant();
}

/*!̈́
   Returns the \a gender name of a voice.
*/
QString QVoice::genderName(QVoice::Gender gender)
{
    QString retval;
    switch (gender) {
    case QVoice::Male:
        retval = QTextToSpeech::tr("Male", "Gender of a voice");
        break;
    case QVoice::Female:
        retval = QTextToSpeech::tr("Female", "Gender of a voice");
        break;
    case QVoice::Unknown:
        retval = QTextToSpeech::tr("Unknown Gender", "Voice gender is unknown");
        break;
    }
    return retval;
}

/*!
   Returns a string representing the \a age class of a voice.
*/
QString QVoice::ageName(QVoice::Age age)
{
    QString retval;
    switch (age) {
    case QVoice::Child:
        retval = QTextToSpeech::tr("Child", "Age of a voice");
        break;
    case QVoice::Teenager:
        retval = QTextToSpeech::tr("Teenager", "Age of a voice");
        break;
    case QVoice::Adult:
        retval = QTextToSpeech::tr("Adult", "Age of a voice");
        break;
    case QVoice::Senior:
        retval = QTextToSpeech::tr("Senior", "Age of a voice");
        break;
    case QVoice::Other:
        retval = QTextToSpeech::tr("Other Age", "Unknown age of a voice");
        break;
    }
    return retval;
}

#ifndef QT_NO_DATASTREAM
QDataStream &QVoice::writeTo(QDataStream &stream) const
{
    stream << name() << gender() << age() << data();
    return stream;
}

QDataStream &QVoice::readFrom(QDataStream &stream)
{
    if (!d)
        d.reset(new QVoicePrivate);

    stream >> d->name >> d->gender >> d->age >> d->data;
    return stream;
}
#endif

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const QVoice &voice)
{
    QDebugStateSaver state(dbg);
    dbg.noquote().nospace();
    dbg << voice.name() << ", " << QVoice::genderName(voice.gender())
                        << ", " << QVoice::ageName(voice.age())
                        << "; data: " << voice.data();
    return dbg;
}
#endif

QT_END_NAMESPACE
