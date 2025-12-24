#ifndef CATEGORY_H
#define CATEGORY_H

#include <QString>

class Category
{
public:
    Category();
    Category(long id, const QString& name, int order, int flags);

    long id() const;
    void setId(long id);

    QString name() const;
    void setName(const QString& name);

    int order() const;
    void setOrder(int order);

    int flags() const;
    void setFlags(int flags);

private:
    long m_id;
    QString m_name;
    int m_order;
    int m_flags;
};

#endif // CATEGORY_H
