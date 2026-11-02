/*
    SPDX-FileCopyrightText: 2025 Dolphin Color Tags contributors
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "icontools.h"

#include <QPainter>
#include <QPixmap>

#include <functional>

namespace ColorTag
{

namespace
{

QPixmap renderPixmap(int logicalSize, qreal devicePixelRatio, const std::function<void(QPainter &, qreal)> &draw)
{
    QPixmap pixmap(qRound(logicalSize * devicePixelRatio), qRound(logicalSize * devicePixelRatio));
    pixmap.setDevicePixelRatio(devicePixelRatio);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.scale(devicePixelRatio, devicePixelRatio);
    draw(painter, logicalSize);
    return pixmap;
}

QIcon makeIcon(const std::function<void(QPainter &, qreal)> &draw, int logicalSize)
{
    QIcon icon;
    for (int size : {16, 22, 32, 48}) {
        icon.addPixmap(renderPixmap(size, 1.0, draw));
        icon.addPixmap(renderPixmap(size, 2.0, draw));
    }
    // Also provide the explicitly requested size for unusual menu metrics.
    icon.addPixmap(renderPixmap(logicalSize, 1.0, draw));
    icon.addPixmap(renderPixmap(logicalSize, 2.0, draw));
    return icon;
}

void drawBadge(QPainter &painter, qreal size, const QColor &color)
{
    const qreal penWidth = qMax<qreal>(1.0, size * 0.075);
    const QRectF circle(size * 0.11 + penWidth / 2.0,
                        size * 0.11 + penWidth / 2.0,
                        size * 0.78 - penWidth,
                        size * 0.78 - penWidth);

    painter.setBrush(color);
    painter.setPen(QPen(QColor(0, 0, 0, 115), penWidth));
    painter.drawEllipse(circle);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 55));
    painter.drawEllipse(QPointF(size * 0.36, size * 0.31), size * 0.10, size * 0.075);
}

} // namespace

QIcon badgeIcon(const QColor &color, int logicalSize)
{
    return makeIcon(
        [color](QPainter &painter, qreal size) {
            drawBadge(painter, size, color);
        },
        logicalSize);
}

QIcon noColorIcon(int logicalSize)
{
    return makeIcon(
        [](QPainter &painter, qreal size) {
            QPen pen(QColor(155, 155, 155), qMax<qreal>(1.0, size * 0.075));
            pen.setDashPattern({2.0, 2.2});
            pen.setCapStyle(Qt::RoundCap);
            painter.setPen(pen);
            painter.setBrush(Qt::NoBrush);
            const qreal inset = size * 0.13;
            painter.drawEllipse(QRectF(inset, inset, size - 2 * inset, size - 2 * inset));
        },
        logicalSize);
}

QIcon paletteIcon(int logicalSize)
{
    return makeIcon(
        [](QPainter &painter, qreal size) {
            const qreal radius = size * 0.17;
            const qreal penWidth = qMax<qreal>(0.8, size * 0.045);
            painter.setPen(QPen(QColor(0, 0, 0, 100), penWidth));
            const QColor colors[4] = {QColor(0xff, 0x00, 0x00),
                                      QColor(0xff, 0xd4, 0x00),
                                      QColor(0x00, 0x80, 0x00),
                                      QColor(0x00, 0x60, 0xff)};
            const QPointF centers[4] = {QPointF(size * 0.34, size * 0.34),
                                        QPointF(size * 0.66, size * 0.34),
                                        QPointF(size * 0.34, size * 0.66),
                                        QPointF(size * 0.66, size * 0.66)};
            for (int i = 0; i < 4; ++i) {
                painter.setBrush(colors[i]);
                painter.drawEllipse(centers[i], radius, radius);
            }
        },
        logicalSize);
}

} // namespace ColorTag
