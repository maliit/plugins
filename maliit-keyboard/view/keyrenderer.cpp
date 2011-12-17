/*
 * This file is part of Maliit Plugins
 *
 * Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: Mohammad Anwari <Mohammad.Anwari@nokia.com>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this list
 * of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * Neither the name of Nokia Corporation nor the names of its contributors may be
 * used to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "keyrenderer.h"
#include <qdrawutil.h>

namespace MaliitKeyboard {

namespace {
const char *const g_images_dir(MALIIT_KEYBOARD_IMAGES_DIR);
QHash<QByteArray, QPixmap> g_cache;

QPixmap loadPixmap(const QByteArray &id)
{
    if (id.isEmpty()) {
        return QPixmap();
    }

    const QPixmap &result(g_cache.value(id));

    if (not result.isNull()) {
        return result;
    }

    QByteArray filename(g_images_dir);
    filename.append('/');
    filename.append(id);

    QPixmap new_pixmap(filename);
    g_cache.insert(id, new_pixmap);

    return new_pixmap;
}
}

void KeyRenderer::render(QPainter *painter,
                         const Key &key,
                         const QPoint &origin)
{
    const QMargins &m(key.margins());
    const QRect &key_rect(key.rect().translated(origin).adjusted(m.left(), m.top(), -m.right(), -m.bottom()));

    qDrawBorderPixmap(painter, key_rect, key.backgroundBorders(),
                      loadPixmap(key.background()));

    if (QFont *font = key.label().font().data()) {
        painter->setFont(*font);
    }

    painter->setPen(key.label().color());
    const QString &text(key.label().text());
    const QPixmap &icon(loadPixmap(key.icon()));

    if (not text.isEmpty()) {
        painter->drawText(key_rect, Qt::AlignCenter, text);
    } else if (not icon.isNull()) {
        const QPoint &c(key_rect.center());
        const QPoint tl(c.x() - icon.width() / 2, c.y() - icon.height() / 2);
        painter->drawPixmap(tl, icon);
    }
}

} // namespace MaliitKeyboard
