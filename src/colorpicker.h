/*
    SPDX-FileCopyrightText: 2025 Dolphin Color Tags contributors
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QString>

class QWidget;

namespace ColorTag
{

enum class PickResult {
    Cancelled,
    Picked,
    Cleared,
};

struct PickOutcome {
    PickResult result = PickResult::Cancelled;
    QString colorId;
};

/**
 * Show the horizontal color palette at the current mouse position.
 *
 * \param currentColor   The color shared by all selected items, or empty.
 * \param mixedSelection True if the selected items have different colors;
 *                       no swatch is marked as current in that case.
 */
PickOutcome pickColor(QWidget *parent, const QString &currentColor, bool mixedSelection);

} // namespace ColorTag
