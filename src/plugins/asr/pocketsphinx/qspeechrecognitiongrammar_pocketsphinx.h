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

#ifndef QSPEECHRECOGNITIONGRAMMAR_POCKETSPHINX_H
#define QSPEECHRECOGNITIONGRAMMAR_POCKETSPHINX_H

#include "qspeechrecognitionengine_pocketsphinx.h"
#include "qspeechrecognitionplugingrammar.h"

class QSpeechRecognitionGrammarPocketSphinx : public QSpeechRecognitionPluginGrammar
{
    Q_OBJECT
public:
    explicit QSpeechRecognitionGrammarPocketSphinx(const QString &name, const QUrl &location, QSpeechRecognitionEnginePocketSphinx *engine);
    ~QSpeechRecognitionGrammarPocketSphinx();
    bool exists();
    bool load();
    bool loaded();
signals:

public slots:

private:
    QUrl m_fileLocation;
    QString m_localFilePath;
    QString m_resourcePath;
    QSpeechRecognitionEnginePocketSphinx *m_engine;
    bool m_loaded;
};

#endif // QSPEECHRECOGNITIONGRAMMAR_POCKETSPHINX_H
