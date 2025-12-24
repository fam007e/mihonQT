#include "Category.h"

Category::Category()
    : m_id(-1), m_order(0), m_flags(0)
{
}

Category::Category(long id, const QString& name, int order, int flags)
    : m_id(id), m_name(name), m_order(order), m_flags(flags)
{
}

long Category::id() const { return m_id; }
void Category::setId(long id) { m_id = id; }

QString Category::name() const { return m_name; }
void Category::setName(const QString& name) { m_name = name; }

int Category::order() const { return m_order; }
void Category::setOrder(int order) { m_order = order; }

int Category::flags() const { return m_flags; }
void Category::setFlags(int flags) { m_flags = flags; }
