/*
    SPDX-FileCopyrightText: 2025 Dolphin Color Tags contributors
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "colortag.h"

#include <KOverlayIconPlugin>

#include <QDBusConnection>
#include <QSet>
#include <QUrl>

/**
 * Dolphin asks every overlay plugin for a list of icon names per file and
 * paints them into the corners of the file icon. We return a single
 * "colortag-<id>" icon, which is installed into hicolor/scalable/emblems.
 */
class ColorTagOverlay : public KOverlayIconPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.colortag.overlay")

public:
    ColorTagOverlay()
    {
        // Tags may be changed by the context menu plugin, by the colortag CLI
        // or by a script. All of them emit this signal, which lets us refresh
        // the badge without reloading the folder.
        QDBusConnection::sessionBus().connect(QString(),
                                              QString::fromLatin1(ColorTag::DbusPath),
                                              QString::fromLatin1(ColorTag::DbusInterface),
                                              QString::fromLatin1(ColorTag::DbusSignal),
                                              this,
                                              SLOT(slotTagsChanged(QStringList)));
    }

    QStringList getOverlays(const QUrl &url) override
    {
        if (!url.isLocalFile()) {
            return {};
        }
        const QString color = ColorTag::readColor(url.toLocalFile());
        if (color.isEmpty()) {
            return {};
        }
        return {QStringLiteral("colortag-") + color};
    }

private Q_SLOTS:
    void slotTagsChanged(const QStringList &paths)
    {
        // Dolphin re-queries every plugin for the URL, so the second argument
        // of the signal is informational only.
        QSet<QString> seen;
        for (const QString &path : paths) {
            if (path.isEmpty() || seen.contains(path)) {
                continue;
            }
            seen.insert(path);
            const QUrl url = QUrl::fromLocalFile(path);
            emit overlaysChanged(url, getOverlays(url));
        }
    }
};

#include "colortagoverlay.moc"
