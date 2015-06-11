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

#include "qspeechrecognitiongrammar_pocketsphinx.h"

#include <QFile>
#include <QTextStream>

#include <pocketsphinx.h>

QT_BEGIN_NAMESPACE

QSpeechRecognitionGrammarPocketSphinx::QSpeechRecognitionGrammarPocketSphinx(const QString &name, const QUrl &location, QSpeechRecognitionEnginePocketSphinx *engine):
    QSpeechRecognitionPluginGrammar(name, engine),
    m_fileLocation(location),
    m_engine(engine),
    m_loaded(false)
{
    if (m_engine && m_fileLocation.isLocalFile())
        m_localFilePath = m_engine->localizedFilePath(m_fileLocation.toLocalFile());
    else if (m_fileLocation.scheme() == "qrc")
        m_resourcePath = ":" + m_fileLocation.toString(QUrl::RemoveScheme);
}

bool QSpeechRecognitionGrammarPocketSphinx::exists()
{
    if (!m_localFilePath.isEmpty())
        return QFile::exists(m_localFilePath);
    else if (!m_resourcePath.isEmpty())
        return QFile::exists(m_resourcePath);
    return false;
}

bool QSpeechRecognitionGrammarPocketSphinx::load()
{
    if (!m_loaded && m_engine) {
        // Local files are directly supported. For embedded resources read the file to memory first.
        if (!m_localFilePath.isEmpty()) {
            if (m_engine->createGrammarFromFile(name(), m_localFilePath)) {
                m_loaded = true;
            }
        } else if (!m_resourcePath.isEmpty()) {
            QFile file(m_resourcePath);
            if (file.open(QFile::ReadOnly | QFile::Text)) {
                QTextStream in(&file);
                if (m_engine->createGrammarFromString(name(), in.readAll())) {
                    m_loaded = true;
                }
            }
        }
    }
    return m_loaded;
}

bool QSpeechRecognitionGrammarPocketSphinx::loaded()
{
    return m_loaded;
}

QSpeechRecognitionGrammarPocketSphinx::~QSpeechRecognitionGrammarPocketSphinx()
{
    if (m_loaded && m_engine)
        m_engine->deleteGrammar(name());
}

QT_END_NAMESPACE
