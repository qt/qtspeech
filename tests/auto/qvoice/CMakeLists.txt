# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

qt_internal_add_test(tst_qvoice
    SOURCES
        tst_qvoice.cpp
    PUBLIC_LIBRARIES
        Qt::TextToSpeechPrivate
)
