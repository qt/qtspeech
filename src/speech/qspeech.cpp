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


#include "qspeech.h"
#include "qspeech_p.h"

#include <qdebug.h>

QT_BEGIN_NAMESPACE


// FIXME: check that the private copying actually works
// FIXME: disentangle SPD from the generic stuff


// /*!
//  \class QSpeechVoice
//  \brief The QSpeechVoice class represents a voice in QSpeech.

//  A voice is bound to a language.
//*/

//QSpeechVoice::QSpeechVoice(const QSpeechVoice &other)
//    : d(other.d)
//{
//}

//QSpeechVoice::~QSpeechVoice()
//{}

//QString QSpeechVoice::name() const
//{
//    return d->name();
//}

//QLocale QSpeechVoice::locale() const
//{
//    return d->locale();
//}


QSpeechPrivate::QSpeechPrivate(QSpeech *speech)
    : m_speech(speech), m_state(QSpeech::Ready)
{
}

/*!
  \class QSpeech
  \brief The QSpeech class provides a convenient access to text-to-speech engines

  On Linux by default speech-dispatcher is used.
*/



QSpeech::State QSpeech::state() const
{
    Q_D(const QSpeech);
    return d->state();
}

void QSpeech::say(const QString &text)
{
    Q_D(QSpeech);
    d->say(text);
}

void QSpeech::stop()
{
    Q_D(QSpeech);
    d->stop();
}

/*!
  Pause the current speech.
  \note this function depends on the platform and backend and may not work at all,
  take several seconds until it takes effect or may pause instantly.
  Some synthesizers will look for a break that they can later resume from, such as
  a sentence end.
 */
void QSpeech::pause()
{
    Q_D(QSpeech);
    d->pause();
}

void QSpeech::resume()
{
    Q_D(QSpeech);
    d->resume();
}

//QSpeechVoice QSpeech::currentVoice() const
//{
//    Q_D(const QSpeech);
//    return d->currentVoice();
//}

//void QSpeech::setVoice(const QSpeechVoice &voice)
//{
//    Q_D(QSpeech);
//    d->setVoice(voice);
//}

//QVector<QSpeechVoice> QSpeech::availableVoices() const
//{
//    Q_D(const QSpeech);
//    return d->availableVoices();
//}

//QVector<QString> QSpeech::availableVoiceTypes() const
//{
//    Q_D(const QSpeech);
//    return d->availableVoiceTypes();
//}

//void QSpeech::setVoiceType(const QString& type)
//{
//    Q_D(QSpeech);
//    d->setVoiceType(type);
//}
//QString QSpeech::currentVoiceType() const
//{
//    Q_D(const QSpeech);
//    return d->currentVoiceType();
//}


/*!
 Sets the voice \a pitch to a value between -1.0 and 1.0.
 The default of 0.0 is normal speech.
*/
void QSpeech::setPitch(double pitch)
{
    Q_D(QSpeech);
    d->setPitch(pitch);
}

/*!
 Sets the voice \a rate to a value between -1.0 and 1.0.
 The default of 0.0 is normal speech flow.
*/
void QSpeech::setRate(double rate)
{
    Q_D(QSpeech);
    d->setRate(rate);
}

/*!
 Sets the \a volume to a value between -1.0 and 1.0.
 The default is 0.0.
*/
void QSpeech::setVolume(double volume)
{
    Q_D(QSpeech);
    d->setVolume(volume);
}

QT_END_NAMESPACE

#include "moc_qspeech.cpp"
