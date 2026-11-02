/*
    SPDX-FileCopyrightText: 2025 Dolphin Color Tags contributors
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QColor>
#include <QIcon>

namespace ColorTag
{

/**
 * A colored dot with a dark rim, used in the context menu next to the color
 * name. Painted at runtime so it always matches the palette and the menu
 * background, regardless of the active icon theme.
 */
QIcon badgeIcon(const QColor &color, int logicalSize = 22);

/** A dashed, empty circle: "no color". */
QIcon noColorIcon(int logicalSize = 22);

/** Four small color dots: the "Color Tag" entry icon. */
QIcon paletteIcon(int logicalSize = 22);

} // namespace ColorTag
