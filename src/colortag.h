/*
    SPDX-FileCopyrightText: 2025 Dolphin Color Tags contributors
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QColor>
#include <QList>
#include <QString>
#include <QStringList>

#include <optional>

namespace ColorTag
{

/**
 * One entry of the fixed color palette.
 *
 * The id is what gets stored in the user.colortag extended attribute, so it
 * must stay stable: the overlay plugin maps it to the "colortag-<id>" icon.
 */
struct PaletteEntry {
    QString id;
    QString label;
    QColor color;
};

/** The palette shown in the menu and in the picker popup. */
const QList<PaletteEntry> &palette();

/** Look up an entry by its id. Comparison is case-insensitive. */
std::optional<PaletteEntry> entryById(const QString &id);

/** Trimmed, lower-cased id. */
QString normalizeId(const QString &id);

bool isValidId(const QString &id);

/** Human readable label for a color id, e.g. "Red". Empty for an empty id. */
QString labelForId(const QString &id);

/**
 * Read the color tag of a local file or folder.
 * Returns an empty string when the item is untagged or the tag is unknown.
 */
QString readColor(const QString &path);

/**
 * Set the color tag of a local file or folder.
 * On failure, errorMessage (when given) receives a user presentable reason.
 */
bool writeColor(const QString &path, const QString &id, QString *errorMessage = nullptr);

/** Remove the color tag. An untagged item is not an error. */
bool clearColor(const QString &path, QString *errorMessage = nullptr);

struct ApplyResult {
    int tagged = 0;
    int cleared = 0;
    QStringList changedPaths;
    /** One "path: reason" entry per item that could not be updated. */
    QStringList failures;
};

/**
 * Tag (colorId non-empty) or untag (colorId empty) a batch of local paths and
 * notify other processes about the change.
 */
ApplyResult apply(const QStringList &paths, const QString &colorId);

/**
 * Tell running file managers that the tag of these paths changed.
 *
 * Dolphin's overlay plugin listens for this signal and refreshes the badge
 * immediately, without a manual reload.
 */
void notifyChanged(const QStringList &paths);

/** Extended attribute used to store the color id. */
inline constexpr char XattrName[] = "user.colortag";

/** D-Bus signal emitted after tags changed. */
inline constexpr char DbusPath[] = "/ColorTag";
inline constexpr char DbusInterface[] = "org.kde.ColorTag";
inline constexpr char DbusSignal[] = "Changed";

} // namespace ColorTag
