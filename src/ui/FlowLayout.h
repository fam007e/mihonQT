#ifndef FLOWLAYOUT_H
#define FLOWLAYOUT_H

#include <QLayout>
#include <QList>

// Flow layout: children are laid out left-to-right, wrapping onto new rows
// as the available width shrinks (Qt's classic FlowLayout, modernized for
// Qt 6 without the deprecated copy boilerplate).
class FlowLayout : public QLayout
{
public:
    explicit FlowLayout(QWidget *parent = nullptr, int hSpacing = 12, int vSpacing = 12);
    ~FlowLayout() override;

    void addItem(QLayoutItem *item) override;
    Qt::Orientations expandingDirections() const override { return {}; }
    bool hasHeightForWidth() const override;
    int heightForWidth(int width) const override;
    int count() const override;
    QLayoutItem *itemAt(int index) const override;
    QLayoutItem *takeAt(int index) override;
    void setGeometry(const QRect &rect) override;
    QSize sizeHint() const override;
    QSize minimumSize() const override;

private:
    int doLayout(const QRect &rect, bool testOnly) const;

    QList<QLayoutItem *> m_items;
    int m_hSpace;
    int m_vSpace;
};

#endif // FLOWLAYOUT_H
