// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial



#ifndef QTEXTTOSPEECH_GLOBAL_H
#define QTEXTTOSPEECH_GLOBAL_H

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

#ifndef QT_STATIC
# if defined(QTEXTTOSPEECH_LIBRARY)
#   define Q_TEXTTOSPEECH_EXPORT Q_DECL_EXPORT
# else
#   define Q_TEXTTOSPEECH_EXPORT Q_DECL_IMPORT
# endif
#else
# define Q_TEXTTOSPEECH_EXPORT
#endif

QT_END_NAMESPACE

#endif
