/*
    SPDX-FileCopyrightText: 2025 Dolphin Color Tags contributors
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "colortag.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QFile>
#include <QSet>

#include <cerrno>
#include <cstring>
#include <sys/xattr.h>

namespace ColorTag
{

const QList<PaletteEntry> &palette()
{
    // Keep in sync with the colortag-*.svg icons in icons/.
    static const QList<PaletteEntry> entries = {
        {QStringLiteral("red"), QStringLiteral("Red"), QColor(0xff, 0x00, 0x00)},
        {QStringLiteral("orange"), QStringLiteral("Orange"), QColor(0xff, 0xa5, 0x00)},
        {QStringLiteral("yellow"), QStringLiteral("Yellow"), QColor(0xff, 0xff, 0x00)},
        {QStringLiteral("green"), QStringLiteral("Green"), QColor(0x00, 0x80, 0x00)},
        {QStringLiteral("blue"), QStringLiteral("Blue"), QColor(0x00, 0x00, 0xff)},
        {QStringLiteral("purple"), QStringLiteral("Purple"), QColor(0x80, 0x00, 0x80)},
        {QStringLiteral("gray"), QStringLiteral("Gray"), QColor(0xb3, 0xb3, 0xb3)},
    };
    return entries;
}

QString normalizeId(const QString &id)
{
    return id.trimmed().toLower();
}

std::optional<PaletteEntry> entryById(const QString &id)
{
    const QString normalized = normalizeId(id);
    if (normalized.isEmpty()) {
        return std::nullopt;
    }
    for (const PaletteEntry &entry : palette()) {
        if (entry.id == normalized) {
            return entry;
        }
    }
    return std::nullopt;
}

bool isValidId(const QString &id)
{
    return entryById(id).has_value();
}

QString labelForId(const QString &id)
{
    const auto entry = entryById(id);
    return entry ? entry->label : QString();
}

namespace
{

QString errorForErrno(int error)
{
    switch (error) {
    case ENOTSUP:
#if defined(EOPNOTSUPP) && EOPNOTSUPP != ENOTSUP
    case EOPNOTSUPP:
#endif
        return QStringLiteral("This filesystem does not support extended attributes");
    case EPERM:
    case EACCES:
        return QStringLiteral("Permission denied");
    case ENOENT:
        return QStringLiteral("File not found");
    case EROFS:
        return QStringLiteral("The filesystem is read-only");
    case ENAMETOOLONG:
        return QStringLiteral("Path is too long");
    default:
        return QString::fromLocal8Bit(std::strerror(error));
    }
}

QByteArray encodedPath(const QString &path)
{
    return QFile::encodeName(path);
}

} // namespace

QString readColor(const QString &path)
{
    if (path.isEmpty()) {
        return {};
    }

    char buffer[128];
    const QByteArray nativePath = encodedPath(path);
    // Follow symbolic links: the tag belongs to the file the user sees.
    const ssize_t length = ::getxattr(nativePath.constData(), XattrName, buffer, sizeof(buffer) - 1);
    if (length <= 0) {
        return {};
    }
    buffer[length] = '\0';

    const QString id = normalizeId(QString::fromUtf8(buffer, static_cast<int>(length)));
    return isValidId(id) ? id : QString();
}

bool writeColor(const QString &path, const QString &id, QString *errorMessage)
{
    const auto entry = entryById(id);
    if (!entry) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Unknown color \"%1\"").arg(id);
        }
        return false;
    }
    if (path.isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Empty path");
        }
        return false;
    }

    const QByteArray nativePath = encodedPath(path);
    const QByteArray value = entry->id.toUtf8();
    if (::setxattr(nativePath.constData(), XattrName, value.constData(), static_cast<size_t>(value.size()), 0) != 0) {
        if (errorMessage) {
            *errorMessage = errorForErrno(errno);
        }
        return false;
    }
    return true;
}

bool clearColor(const QString &path, QString *errorMessage)
{
    if (path.isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Empty path");
        }
        return true;
    }

    const QByteArray nativePath = encodedPath(path);
    if (::removexattr(nativePath.constData(), XattrName) != 0) {
        const int error = errno;
        // Removing a tag that is not there is fine.
        if (error == ENODATA) {
            return true;
        }
        if (errorMessage) {
            *errorMessage = errorForErrno(error);
        }
        return false;
    }
    return true;
}

ApplyResult apply(const QStringList &paths, const QString &colorId)
{
    ApplyResult result;
    const QString normalized = normalizeId(colorId);
    const bool clearing = normalized.isEmpty();

    if (!clearing && !isValidId(normalized)) {
        result.failures.append(QStringLiteral("%1: Unknown color \"%2\"").arg(paths.value(0), colorId));
        return result;
    }

    QSet<QString> seen;
    for (const QString &path : paths) {
        if (path.isEmpty() || seen.contains(path)) {
            continue;
        }
        seen.insert(path);

        QString error;
        const bool ok = clearing ? clearColor(path, &error) : writeColor(path, normalized, &error);
        if (!ok) {
            result.failures.append(QStringLiteral("%1: %2").arg(path, error));
            continue;
        }
        if (clearing) {
            ++result.cleared;
        } else {
            ++result.tagged;
        }
        result.changedPaths.append(path);
    }

    if (!result.changedPaths.isEmpty()) {
        notifyChanged(result.changedPaths);
    }
    return result;
}

void notifyChanged(const QStringList &paths)
{
    if (paths.isEmpty()) {
        return;
    }

    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) {
        return;
    }

    QDBusMessage signal = QDBusMessage::createSignal(QString::fromLatin1(DbusPath),
                                                     QString::fromLatin1(DbusInterface),
                                                     QString::fromLatin1(DbusSignal));
    signal << paths;
    // A signal has no reply; send() is enough and does not require a service.
    bus.send(signal);
}

} // namespace ColorTag
