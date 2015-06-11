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

#ifndef QSPEECHRECOGNITIONGRAMMAR_P_H
#define QSPEECHRECOGNITIONGRAMMAR_P_H

#include "qspeechrecognitiongrammar.h"
#include <QtCore/QObject>
#include <QtCore/QLoggingCategory>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(lcSpeechAsr)

class QSpeechRecognitionGrammarImpl: public QSpeechRecognitionGrammar
{
    Q_OBJECT

public:
    explicit QSpeechRecognitionGrammarImpl(const QString &name, QSpeechRecognitionEngine *engine, QObject *parent = 0);
    QSpeechRecognitionEngine *engine() const;
    QSpeechRecognitionGrammar::State state() const;
    QString name() const;
    void setState(QSpeechRecognitionGrammar::State state);

private:
    QString m_name;
    QSpeechRecognitionEngine *m_engine;
    QSpeechRecognitionGrammar::State m_state;
};

QT_END_NAMESPACE

#endif // QSPEECHRECOGNITIONGRAMMAR_P_H
