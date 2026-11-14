#include "FlowLayout.h"
#include <QWidget>

FlowLayout::FlowLayout(QWidget *parent, int hSpacing, int vSpacing)
    : QLayout(parent), m_hSpace(hSpacing), m_vSpace(vSpacing)
{
    setContentsMargins(0, 0, 0, 0);
}

FlowLayout::~FlowLayout()
{
    qDeleteAll(m_items);
    m_items.clear();
}

void FlowLayout::addItem(QLayoutItem *item)
{
    m_items.append(item);
}

int FlowLayout::count() const
{
    return m_items.size();
}

QLayoutItem *FlowLayout::itemAt(int index) const
{
    return m_items.value(index);
}

QLayoutItem *FlowLayout::takeAt(int index)
{
    return (index >= 0 && index < m_items.size()) ? m_items.takeAt(index) : nullptr;
}

bool FlowLayout::hasHeightForWidth() const
{
    return true;
}

int FlowLayout::heightForWidth(int width) const
{
    return doLayout(QRect(0, 0, width, 0), true);
}

QSize FlowLayout::sizeHint() const
{
    return minimumSize();
}

QSize FlowLayout::minimumSize() const
{
    QSize size;
    for (QLayoutItem *item : m_items)
        size = size.expandedTo(item->minimumSize());
    const QMargins m = contentsMargins();
    size += QSize(m.left() + m.right(), m.top() + m.bottom());
    return size;
}

void FlowLayout::setGeometry(const QRect &rect)
{
    QLayout::setGeometry(rect);
    doLayout(rect, false);
}

int FlowLayout::doLayout(const QRect &rect, bool testOnly) const
{
    const QMargins margins = contentsMargins();
    const int availWidth = rect.width() - margins.left() - margins.right();

    // First pass: greedily pack items into rows and measure each row
    QList<QList<QLayoutItem*>> rows;
    rows.append(QList<QLayoutItem*>());
    QList<int> rowWidths, rowHeights;
    int rowW = 0, rowH = 0;
    for (QLayoutItem *item : m_items) {
        const int itemW = item->sizeHint().width();
        const int itemH = item->sizeHint().height();
        const int nextW = rowW + (rowW ? m_hSpace : 0) + itemW;
        if (nextW > availWidth && !rows.last().isEmpty()) {
            rowWidths.append(rowW);
            rowHeights.append(rowH);
            rows.append(QList<QLayoutItem*>());
            rowW = 0;
            rowH = 0;
        }
        rows.last().append(item);
        rowW += itemW + (rowW ? m_hSpace : 0);
        rowH = qMax(rowH, itemH);
    }
    rowWidths.append(rowW);
    rowHeights.append(rowH);

    // Second pass: place rows vertically, centering each row horizontally
    int y = rect.y() + margins.top();
    for (int r = 0; r < rows.size(); ++r) {
        int x = rect.x() + margins.left()
            + qMax(0, (availWidth - rowWidths[r]) / 2);
        for (QLayoutItem *item : rows[r]) {
            if (!testOnly)
                item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));
            x += item->sizeHint().width() + m_hSpace;
        }
        y += rowHeights[r] + m_vSpace;
    }
    return y - m_vSpace - rect.y() + margins.bottom();
}
