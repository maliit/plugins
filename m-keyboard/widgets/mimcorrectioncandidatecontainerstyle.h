/* * This file is part of meego-keyboard *
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * Contact: Nokia Corporation (directui@nokia.com)
 *
 * If you have questions regarding the use of this file, please contact
 * Nokia at directui@nokia.com.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * and appearing in the file LICENSE.LGPL included in the packaging
 * of this file.
 */

#ifndef MIMCORRECTIONCANDIDATECONTAINERSTYLE_H
#define MIMCORRECTIONCANDIDATECONTAINERSTYLE_H

#include <MWidgetStyle>

class MImCorrectionCandidateContainerStyle : public MWidgetStyle
{
    Q_OBJECT
    M_STYLE(MImCorrectionCandidateContainerStyle)

    M_STYLE_PTR_ATTRIBUTE(MScalableImage *,  wordtrackerPointerImage, WordtrackPointerImage)
    M_STYLE_ATTRIBUTE(QSize, wordtrackerPointerSize, WordtrackerPointerSize)
    M_STYLE_ATTRIBUTE(int, wordtrackerPointerOverlap, WordtrackerPointerOverlap)
};

class M_EXPORT MImCorrectionCandidateContainerStyleContainer : public MWidgetStyleContainer
{
    M_STYLE_CONTAINER(MImCorrectionCandidateContainerStyle)
};

#endif

