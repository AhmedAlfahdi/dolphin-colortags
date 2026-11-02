/*!***********************************************************************
    SPDX-FileCopyrightText: 2025 Dolphin Color Tags contributors
    SPDX-License-Identifier: GPL-2.0-or-later
*************************************************************************/

#include "colortag.h"
#include "colorpicker.h"
#include "icontools.h"

#include <KAbstractFileItemActionPlugin>
#include <KFileItem>
#include <KFileItemListProperties>
#include <KPluginFactory>

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariantList>

#include <memory>

/**
 * Adds a top level "Color Tag" submenu to Dolphin's context menu, with one
 * entry per palette color, a "No Color" entry and a picker popup.
 */
class ColorTagAction : public KAbstractFileItemActionPlugin
{
    Q_OBJECT

public:
    explicit ColorTagAction(QObject *parent = nullptr, const QVariantList &args = {})
        : KAbstractFileItemActionPlugin(parent)
    {
        Q_UNUSED(args)
    }

    QList<QAction *> actions(const KFileItemListProperties &properties, QWidget *parentWidget) override;

private:
    void applyColor(const QStringList &paths, const QString &colorId, QWidget *parentWidget);
    void chooseColor(const QStringList &paths, QWidget *parentWidget);

    std::unique_ptr<QMenu> m_menu;
    QAction *m_rootAction = nullptr;
};

QList<QAction *> ColorTagAction::actions(const KFileItemListProperties &properties, QWidget *parentWidget)
{
    const KFileItemList items = properties.items();
    if (items.isEmpty()) {
        return {};
    }

    QStringList paths;
    paths.reserve(items.size());
    for (const KFileItem &item : items) {
        // Extended attributes only exist on local files. Hide the menu for
        // remote URLs and archives rather than failing after the click.
        if (!item.isLocalFile()) {
            return {};
        }
        const QString path = item.localPath();
        if (path.isEmpty()) {
            return {};
        }
        paths.append(path);
    }

    if (!m_rootAction) {
        m_rootAction = new QAction(ColorTag::paletteIcon(), tr("Color Tag"), this);
        m_menu = std::make_unique<QMenu>();
        m_rootAction->setMenu(m_menu.get());
    }
    m_menu->clear();

    QString commonColor;
    bool first = true;
    bool mixed = false;
    bool anyTagged = false;
    for (const QString &path : paths) {
        const QString color = ColorTag::readColor(path);
        anyTagged = anyTagged || !color.isEmpty();
        if (first) {
            commonColor = color;
            first = false;
        } else if (color != commonColor) {
            mixed = true;
        }
    }
    if (mixed) {
        commonColor.clear();
    }

    for (const ColorTag::PaletteEntry &entry : ColorTag::palette()) {
        QAction *action = m_menu->addAction(ColorTag::badgeIcon(entry.color), entry.label);
        action->setCheckable(true);
        action->setChecked(!mixed && commonColor == entry.id);
        const QString id = entry.id;
        connect(action, &QAction::triggered, this, [this, paths, id, parentWidget] {
            applyColor(paths, id, parentWidget);
        });
    }

    m_menu->addSeparator();

    QAction *none = m_menu->addAction(ColorTag::noColorIcon(), tr("No Color"));
    none->setEnabled(anyTagged);
    connect(none, &QAction::triggered, this, [this, paths, parentWidget] {
        applyColor(paths, QString(), parentWidget);
    });

    m_menu->addSeparator();

    QAction *choose = m_menu->addAction(ColorTag::paletteIcon(), tr("Choose Color…"));
    connect(choose, &QAction::triggered, this, [this, paths, parentWidget] {
        chooseColor(paths, parentWidget);
    });

    return {m_rootAction};
}

void ColorTagAction::applyColor(const QStringList &paths, const QString &colorId, QWidget *parentWidget)
{
    const ColorTag::ApplyResult result = ColorTag::apply(paths, colorId);
    if (result.failures.isEmpty()) {
        return;
    }

    QMessageBox box(QMessageBox::Warning,
                    tr("Color Tags"),
                    colorId.isEmpty() ? tr("The color tag could not be removed from some items.")
                                      : tr("The color tag could not be set on some items."),
                    QMessageBox::Ok,
                    parentWidget);
    box.setDetailedText(result.failures.join(QLatin1Char('\n')));
    box.exec();
}

void ColorTagAction::chooseColor(const QStringList &paths, QWidget *parentWidget)
{
    QString commonColor;
    bool first = true;
    bool mixed = false;
    for (const QString &path : paths) {
        const QString color = ColorTag::readColor(path);
        if (first) {
            commonColor = color;
            first = false;
        } else if (color != commonColor) {
            mixed = true;
        }
    }
    if (mixed) {
        commonColor.clear();
    }

    const ColorTag::PickOutcome outcome = ColorTag::pickColor(parentWidget, commonColor, mixed);
    switch (outcome.result) {
    case ColorTag::PickResult::Picked:
        applyColor(paths, outcome.colorId, parentWidget);
        break;
    case ColorTag::PickResult::Cleared:
        applyColor(paths, QString(), parentWidget);
        break;
    case ColorTag::PickResult::Cancelled:
        break;
    }
}

K_PLUGIN_CLASS_WITH_JSON(ColorTagAction, "colortagaction.json")

#include "colortagaction.moc"
