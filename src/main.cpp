/*
    SPDX-FileCopyrightText: 2025 Dolphin Color Tags contributors
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "colortag.h"
#include "colorpicker.h"

#include <QApplication>
#include <QCoreApplication>
#include <QCursor>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>
#include <QTextStream>
#include <QWidget>

#include <cstdio>
#include <memory>

#ifndef COLORTAG_VERSION
#define COLORTAG_VERSION "development"
#endif

namespace
{

QTextStream &out()
{
    static QTextStream stream(stdout);
    return stream;
}

QTextStream &err()
{
    static QTextStream stream(stderr);
    return stream;
}

QStringList collectPaths(int argc, char **argv, int first)
{
    QStringList paths;
    for (int i = first; i < argc; ++i) {
        const QString argument = QString::fromLocal8Bit(argv[i]);
        if (argument == QLatin1String("--json") || argument == QLatin1String("--quiet")) {
            continue;
        }
        paths.append(argument);
    }
    return paths;
}

bool hasFlag(int argc, char **argv, const QString &flag)
{
    for (int i = 2; i < argc; ++i) {
        if (QString::fromLocal8Bit(argv[i]) == flag) {
            return true;
        }
    }
    return false;
}

void printUsage()
{
    out() << "colortag " << COLORTAG_VERSION << " — color tags for files and folders\n\n"
          << "Usage:\n"
          << "  colortag list                          Show the palette\n"
          << "  colortag get [--json] PATH...          Show the color tag of each path\n"
          << "  colortag set COLOR PATH...             Tag paths with COLOR\n"
          << "  colortag clear PATH...                 Remove the color tag\n"
          << "  colortag pick PATH...                  Open the color palette at the cursor\n\n"
          << "COLOR is one of:";
    for (const ColorTag::PaletteEntry &entry : ColorTag::palette()) {
        out() << ' ' << entry.id;
    }
    out() << ", or \"none\".\n\n"
          << "Tags are stored in the user.colortag extended attribute.\n"
          << "Dolphin shows them as a colored dot on the file icon.\n";
    out().flush();
}

void printPalette()
{
    for (const ColorTag::PaletteEntry &entry : ColorTag::palette()) {
        out() << entry.id.leftJustified(8) << entry.label.leftJustified(8) << entry.color.name() << '\n';
    }
    out().flush();
}

QString commonColor(const QStringList &paths, bool *mixed)
{
    *mixed = false;
    QString common;
    bool first = true;
    for (const QString &path : paths) {
        const QString color = ColorTag::readColor(path);
        if (first) {
            common = color;
            first = false;
        } else if (color != common) {
            *mixed = true;
        }
    }
    return *mixed ? QString() : common;
}

int reportResult(const ColorTag::ApplyResult &result, const QString &colorId, bool quiet)
{
    for (const QString &failure : result.failures) {
        err() << failure << '\n';
    }
    err().flush();

    if (!quiet && (!result.changedPaths.isEmpty() || result.failures.isEmpty())) {
        if (!colorId.isEmpty()) {
            out() << "Tagged " << result.tagged << " item(s) as " << ColorTag::labelForId(colorId) << ".\n";
        } else {
            out() << "Removed the color tag from " << result.cleared << " item(s).\n";
        }
        out().flush();
    }

    return result.failures.isEmpty() ? 0 : 1;
}

} // namespace

int main(int argc, char **argv)
{
    const QString command = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QString();
    const bool wantsPicker = command == QLatin1String("pick");

    std::unique_ptr<QCoreApplication> application;
    if (wantsPicker) {
        application = std::make_unique<QApplication>(argc, argv);
    } else {
        application = std::make_unique<QCoreApplication>(argc, argv);
    }
    QCoreApplication::setApplicationName(QStringLiteral("colortag"));
    QCoreApplication::setApplicationVersion(QStringLiteral(COLORTAG_VERSION));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));

    if (command.isEmpty() || command == QLatin1String("help") || command == QLatin1String("--help")
        || command == QLatin1String("-h")) {
        printUsage();
        return command.isEmpty() ? 2 : 0;
    }
    if (command == QLatin1String("--version") || command == QLatin1String("-V")) {
        out() << "colortag " << COLORTAG_VERSION << '\n';
        out().flush();
        return 0;
    }
    if (command == QLatin1String("list")) {
        printPalette();
        return 0;
    }

    if (command == QLatin1String("set")) {
        if (argc < 3) {
            printUsage();
            return 2;
        }
        QString color = ColorTag::normalizeId(QString::fromLocal8Bit(argv[2]));
        if (color == QLatin1String("none")) {
            color.clear();
        }
        if (!color.isEmpty() && !ColorTag::isValidId(color)) {
            err() << "Unknown color \"" << color << "\".\n";
            err().flush();
            return 2;
        }
        const QStringList paths = collectPaths(argc, argv, 3);
        if (paths.isEmpty()) {
            printUsage();
            return 2;
        }
        const bool quiet = hasFlag(argc, argv, QStringLiteral("--quiet"));
        return reportResult(ColorTag::apply(paths, color), color, quiet);
    }

    if (command == QLatin1String("clear") || command == QLatin1String("untag")) {
        const QStringList paths = collectPaths(argc, argv, 2);
        if (paths.isEmpty()) {
            printUsage();
            return 2;
        }
        const bool quiet = hasFlag(argc, argv, QStringLiteral("--quiet"));
        return reportResult(ColorTag::apply(paths, QString()), QString(), quiet);
    }

    if (command == QLatin1String("get")) {
        const bool json = hasFlag(argc, argv, QStringLiteral("--json"));
        const QStringList paths = collectPaths(argc, argv, 2);
        if (paths.isEmpty()) {
            printUsage();
            return 2;
        }

        QJsonArray array;
        int exitCode = 0;
        for (const QString &path : paths) {
            if (!QFileInfo::exists(path)) {
                err() << path << ": File not found\n";
                exitCode = 1;
                continue;
            }
            const QString color = ColorTag::readColor(path);
            if (json) {
                array.append(QJsonObject{
                    {QStringLiteral("path"), path},
                    {QStringLiteral("color"), color},
                    {QStringLiteral("label"), ColorTag::labelForId(color)},
                });
            } else {
                out() << path << '\t' << (color.isEmpty() ? QStringLiteral("none") : color) << '\n';
            }
        }
        if (json) {
            out() << QJsonDocument(array).toJson(QJsonDocument::Compact) << '\n';
        }
        out().flush();
        err().flush();
        return exitCode;
    }

    if (command == QLatin1String("pick")) {
        const QStringList paths = collectPaths(argc, argv, 2);
        if (paths.isEmpty()) {
            printUsage();
            return 2;
        }

        bool mixed = false;
        const QString current = commonColor(paths, &mixed);

        // On Wayland a popup needs a parent surface. Use a tiny invisible
        // anchor placed at the cursor so the palette appears under the mouse
        // when the CLI is used outside Dolphin.
        QWidget anchor;
        anchor.setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        anchor.setAttribute(Qt::WA_TranslucentBackground);
        anchor.setAttribute(Qt::WA_ShowWithoutActivating);
        anchor.setGeometry(QRect(QCursor::pos(), QSize(1, 1)));
        anchor.show();

        const ColorTag::PickOutcome outcome = ColorTag::pickColor(&anchor, current, mixed);
        anchor.hide();

        switch (outcome.result) {
        case ColorTag::PickResult::Picked:
            return reportResult(ColorTag::apply(paths, outcome.colorId), outcome.colorId, false);
        case ColorTag::PickResult::Cleared:
            return reportResult(ColorTag::apply(paths, QString()), QString(), false);
        case ColorTag::PickResult::Cancelled:
            return 0;
        }
    }

    err() << "Unknown command \"" << command << "\".\n\n";
    err().flush();
    printUsage();
    return 2;
}
