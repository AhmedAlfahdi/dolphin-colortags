/*
    SPDX-FileCopyrightText: 2025 Dolphin Color Tags contributors
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "colortag.h"

#include <QFile>
#include <QSet>
#include <QTemporaryDir>
#include <QTest>

class ColorTagTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void paletteIsConsistent();
    void normalizesAndLooksUp();
    void readsAndWritesTheTag();
    void appliesToBatches();
    void rejectsUnknownColors();
};

void ColorTagTest::paletteIsConsistent()
{
    const auto &entries = ColorTag::palette();
    QCOMPARE(entries.size(), 7);

    QSet<QString> ids;
    for (const ColorTag::PaletteEntry &entry : entries) {
        QVERIFY(!entry.id.isEmpty());
        QCOMPARE(entry.id, entry.id.toLower());
        QVERIFY(!entry.label.isEmpty());
        QVERIFY(entry.color.isValid());
        QVERIFY(!ids.contains(entry.id));
        ids.insert(entry.id);
    }

    QVERIFY(ColorTag::isValidId(QStringLiteral("red")));
    QVERIFY(ColorTag::isValidId(QStringLiteral("  BLUE ")));
    QVERIFY(!ColorTag::isValidId(QStringLiteral("pink")));
}

void ColorTagTest::normalizesAndLooksUp()
{
    QCOMPARE(ColorTag::normalizeId(QStringLiteral("  ReD ")), QStringLiteral("red"));
    const auto entry = ColorTag::entryById(QStringLiteral("PURPLE"));
    QVERIFY(entry.has_value());
    QCOMPARE(entry->id, QStringLiteral("purple"));
    QCOMPARE(entry->color, QColor(0x80, 0x00, 0x80));
    QCOMPARE(ColorTag::labelForId(QStringLiteral("purple")), QStringLiteral("Purple"));
    QVERIFY(!ColorTag::entryById(QString()).has_value());
    QVERIFY(!ColorTag::entryById(QStringLiteral("nope")).has_value());
}

void ColorTagTest::readsAndWritesTheTag()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString path = dir.filePath(QStringLiteral("file.txt"));
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("hello");
    file.close();

    QCOMPARE(ColorTag::readColor(path), QString());

    QString error;
    if (!ColorTag::writeColor(path, QStringLiteral("green"), &error)) {
        if (error.contains(QStringLiteral("not support"), Qt::CaseInsensitive)) {
            QSKIP("The temporary filesystem does not support user extended attributes");
        }
        QFAIL(qPrintable(error));
    }
    QCOMPARE(ColorTag::readColor(path), QStringLiteral("green"));
    QCOMPARE(ColorTag::readColor(dir.filePath(QStringLiteral("missing"))), QString());

    QVERIFY(ColorTag::clearColor(path, &error));
    QCOMPARE(ColorTag::readColor(path), QString());
    // Clearing an untagged file is not an error.
    QVERIFY(ColorTag::clearColor(path, &error));
}

void ColorTagTest::appliesToBatches()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QStringList paths;
    for (int i = 0; i < 2; ++i) {
        const QString path = dir.filePath(QStringLiteral("file%1.txt").arg(i));
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("x");
        file.close();
        paths.append(path);
    }

    ColorTag::ApplyResult result = ColorTag::apply(paths, QStringLiteral("red"));
    if (result.tagged == 0 && !result.failures.isEmpty()
        && result.failures.first().contains(QStringLiteral("not support"), Qt::CaseInsensitive)) {
        QSKIP("The temporary filesystem does not support user extended attributes");
    }
    QCOMPARE(result.tagged, 2);
    QVERIFY(result.failures.isEmpty());
    QCOMPARE(ColorTag::readColor(paths.at(0)), QStringLiteral("red"));
    QCOMPARE(ColorTag::readColor(paths.at(1)), QStringLiteral("red"));

    // Duplicate paths are applied once.
    result = ColorTag::apply(QStringList{paths.at(0), paths.at(0)}, QStringLiteral("blue"));
    QCOMPARE(result.tagged, 1);
    QCOMPARE(ColorTag::readColor(paths.at(0)), QStringLiteral("blue"));

    result = ColorTag::apply(paths, QString());
    QCOMPARE(result.cleared, 2);
    QCOMPARE(ColorTag::readColor(paths.at(0)), QString());
    QCOMPARE(ColorTag::readColor(paths.at(1)), QString());
}

void ColorTagTest::rejectsUnknownColors()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("file.txt"));
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.close();

    QString error;
    QVERIFY(!ColorTag::writeColor(path, QStringLiteral("chartreuse"), &error));
    QVERIFY(!error.isEmpty());

    const ColorTag::ApplyResult result = ColorTag::apply({path}, QStringLiteral("chartreuse"));
    QCOMPARE(result.tagged, 0);
    QCOMPARE(result.failures.size(), 1);
}

QTEST_GUILESS_MAIN(ColorTagTest)

#include "test_colortag.moc"
