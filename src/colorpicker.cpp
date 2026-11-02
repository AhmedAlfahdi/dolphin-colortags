/*
    SPDX-FileCopyrightText: 2025 Dolphin Color Tags contributors
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "colorpicker.h"
#include "colortag.h"

#include <QApplication>
#include <QCursor>
#include <QKeyEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QTimer>
#include <QToolTip>
#include <QWidgetAction>

namespace ColorTag
{

namespace
{

constexpr int kSwatch = 30;
constexpr int kGap = 4;
constexpr int kGroupGap = 10;
constexpr int kPadding = 10;
constexpr int kBarHeight = kSwatch + 2 * kPadding;

int paletteCount()
{
    return int(palette().size());
}

int barWidth()
{
    const int n = paletteCount();
    return 2 * kPadding + n * kSwatch + (n - 1) * kGap + kGroupGap + kSwatch + kGroupGap + kSwatch;
}

QVector<QRectF> swatchRects()
{
    QVector<QRectF> rects;
    qreal x = kPadding;
    const qreal y = kPadding;
    for (int i = 0; i < paletteCount(); ++i) {
        rects.append(QRectF(x, y, kSwatch, kSwatch));
        x += kSwatch + kGap;
    }
    // "No color"
    x += kGroupGap - kGap;
    rects.append(QRectF(x, y, kSwatch, kSwatch));
    // Close
    x += kSwatch + kGroupGap;
    rects.append(QRectF(x, y, kSwatch, kSwatch));
    return rects;
}

/**
 * The horizontal palette. It is embedded into a QMenu through a
 * QWidgetAction: that makes the popup behave correctly on Wayland (where a
 * plain top-level window cannot be positioned at the cursor).
 */
class SwatchBar : public QWidget
{
    Q_OBJECT

public:
    SwatchBar(const QString &currentColor, bool mixedSelection, QWidget *parent = nullptr)
        : QWidget(parent)
        , m_currentColor(currentColor)
        , m_mixedSelection(mixedSelection)
    {
        setMouseTracking(true);
        setFocusPolicy(Qt::StrongFocus);
        setAttribute(Qt::WA_TranslucentBackground);
        setFixedSize(barWidth(), kBarHeight);

        const int current = currentIndex();
        m_focusIndex = current >= 0 ? current : 0;
    }

    QSize sizeHint() const override
    {
        return {barWidth(), kBarHeight};
    }

signals:
    void picked(const QString &colorId);
    void cleared();
    void closeRequested();

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        // Rounded popup background.
        const QRectF background = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
        painter.setBrush(QColor(0x20, 0x20, 0x20, 250));
        painter.setPen(QPen(QColor(0x3d, 0x3d, 0x3d), 1.0));
        painter.drawRoundedRect(background, 12.0, 12.0);

        const QVector<QRectF> rects = swatchRects();
        const int noneIndex = paletteCount();
        const int closeIndex = noneIndex + 1;

        // Color swatches.
        for (int i = 0; i < paletteCount(); ++i) {
            const QRectF r = rects.at(i);
            painter.setPen(QPen(QColor(0, 0, 0, 70), 1.0));
            painter.setBrush(ColorTag::palette().at(i).color);
            painter.drawEllipse(r.adjusted(1.0, 1.0, -1.0, -1.0));

            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(255, 255, 255, 55));
            painter.drawEllipse(QPointF(r.left() + 9.0, r.top() + 8.0), 3.6, 2.6);
        }

        // "No color" swatch: dashed circle, like the reference palette.
        {
            const QRectF r = rects.at(noneIndex).adjusted(1.0, 1.0, -1.0, -1.0);
            QPen pen(QColor(236, 236, 236), 1.8);
            pen.setDashPattern({2.0, 2.2});
            pen.setCapStyle(Qt::RoundCap);
            painter.setPen(pen);
            painter.setBrush(QColor(0x10, 0x10, 0x10, 140));
            painter.drawEllipse(r.adjusted(2.0, 2.0, -2.0, -2.0));
        }

        // Close button.
        {
            const QRectF r = rects.at(closeIndex);
            if (m_hoverIndex == closeIndex) {
                painter.setPen(Qt::NoPen);
                painter.setBrush(QColor(255, 255, 255, 26));
                painter.drawRoundedRect(r.adjusted(1.0, 1.0, -1.0, -1.0), 6.0, 6.0);
            }
            QPen pen(QColor(0xea, 0xea, 0xea), 2.0);
            pen.setCapStyle(Qt::RoundCap);
            painter.setPen(pen);
            const QPointF c = r.center();
            painter.drawLine(QPointF(c.x() - 5.0, c.y() - 5.0), QPointF(c.x() + 5.0, c.y() + 5.0));
            painter.drawLine(QPointF(c.x() - 5.0, c.y() + 5.0), QPointF(c.x() + 5.0, c.y() - 5.0));
        }

        // Highlight rings are painted last so they sit on top of the swatches.
        const int current = currentIndex();
        for (int i = 0; i <= noneIndex; ++i) {
            const bool isCurrent = (i == current);
            const bool isHovered = (i == m_hoverIndex);
            if (!isCurrent && !isHovered) {
                continue;
            }
            QPen pen(isCurrent ? QColor(255, 255, 255, 235) : QColor(255, 255, 255, 90), 2.0);
            painter.setPen(pen);
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(rects.at(i).adjusted(-3.0, -3.0, 3.0, 3.0));
        }

        // Keyboard focus indicator.
        if (hasFocus() && m_focusIndex >= 0) {
            QPen pen(QColor(255, 255, 255, 120), 1.2);
            pen.setDashPattern({2.0, 2.0});
            painter.setPen(pen);
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(rects.at(m_focusIndex).adjusted(-4.5, -4.5, 4.5, 4.5));
        }
    }

    void mouseMoveEvent(QMouseEvent *event) override
    {
        const int index = indexAt(event->position());
        if (index == m_hoverIndex) {
            return;
        }
        m_hoverIndex = index;
        update();

        if (index < 0) {
            QToolTip::hideText();
            return;
        }
        QString text;
        if (index < paletteCount()) {
            text = ColorTag::palette().at(index).label;
        } else if (index == paletteCount()) {
            text = tr("No color");
        } else {
            text = tr("Close");
        }
        QToolTip::showText(event->globalPosition().toPoint(), text, this);
    }

    void mousePressEvent(QMouseEvent *event) override
    {
        if (event->button() != Qt::LeftButton) {
            return;
        }
        const int index = indexAt(event->position());
        if (index >= 0) {
            activate(index);
        }
    }

    void leaveEvent(QEvent *) override
    {
        m_hoverIndex = -1;
        update();
        QToolTip::hideText();
    }

    void keyPressEvent(QKeyEvent *event) override
    {
        switch (event->key()) {
        case Qt::Key_Escape:
            emit closeRequested();
            return;
        case Qt::Key_Left:
            m_focusIndex = (m_focusIndex <= 0) ? paletteCount() + 1 : m_focusIndex - 1;
            update();
            return;
        case Qt::Key_Right:
            m_focusIndex = (m_focusIndex >= paletteCount() + 1) ? 0 : m_focusIndex + 1;
            update();
            return;
        case Qt::Key_Return:
        case Qt::Key_Enter:
        case Qt::Key_Space:
            activate(m_focusIndex);
            return;
        case Qt::Key_Delete:
        case Qt::Key_Backspace:
            emit cleared();
            return;
        default:
            break;
        }

        if (event->key() >= Qt::Key_1 && event->key() < Qt::Key_1 + paletteCount()) {
            activate(event->key() - Qt::Key_1);
            return;
        }
        if (event->key() == Qt::Key_0) {
            emit cleared();
            return;
        }
        QWidget::keyPressEvent(event);
    }

    void showEvent(QShowEvent *event) override
    {
        QWidget::showEvent(event);
        // The menu grabs the keyboard; make sure the bar gets the key events.
        QTimer::singleShot(0, this, [this] {
            setFocus(Qt::PopupFocusReason);
        });
    }

private:
    int currentIndex() const
    {
        if (m_mixedSelection) {
            return -1;
        }
        if (m_currentColor.isEmpty()) {
            return paletteCount();
        }
        for (int i = 0; i < paletteCount(); ++i) {
            if (ColorTag::palette().at(i).id == m_currentColor) {
                return i;
            }
        }
        return paletteCount();
    }

    int indexAt(const QPointF &position) const
    {
        const QVector<QRectF> rects = swatchRects();
        for (int i = 0; i < rects.size(); ++i) {
            if (rects.at(i).adjusted(-2.0, -2.0, 2.0, 2.0).contains(position)) {
                return i;
            }
        }
        return -1;
    }

    void activate(int index)
    {
        if (index < 0) {
            return;
        }
        if (index < paletteCount()) {
            emit picked(ColorTag::palette().at(index).id);
        } else if (index == paletteCount()) {
            emit cleared();
        } else {
            emit closeRequested();
        }
    }

    QString m_currentColor;
    bool m_mixedSelection = false;
    int m_hoverIndex = -1;
    int m_focusIndex = 0;
};

class PaletteMenu : public QMenu
{
public:
    explicit PaletteMenu(QWidget *parent)
        : QMenu(parent)
    {
        setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
        setAttribute(Qt::WA_TranslucentBackground);
        setContentsMargins(0, 0, 0, 0);
        setStyleSheet(QStringLiteral("QMenu { background: transparent; border: none; padding: 0px; margin: 0px; }"));
    }
};

} // namespace

PickOutcome pickColor(QWidget *parent, const QString &currentColor, bool mixedSelection)
{
    PickOutcome outcome;

    auto *menu = new PaletteMenu(parent);
    auto *bar = new SwatchBar(currentColor, mixedSelection);
    auto *action = new QWidgetAction(menu);
    action->setDefaultWidget(bar);
    menu->addAction(action);

    QObject::connect(bar, &SwatchBar::picked, menu, [&outcome, menu](const QString &colorId) {
        outcome.result = PickResult::Picked;
        outcome.colorId = colorId;
        menu->close();
    });
    QObject::connect(bar, &SwatchBar::cleared, menu, [&outcome, menu] {
        outcome.result = PickResult::Cleared;
        menu->close();
    });
    QObject::connect(bar, &SwatchBar::closeRequested, menu, &QMenu::close);

    menu->exec(QCursor::pos());
    delete menu;

    return outcome;
}

} // namespace ColorTag

#include "colorpicker.moc"
