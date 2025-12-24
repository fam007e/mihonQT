#ifndef CATEGORYREPOSITORY_H
#define CATEGORYREPOSITORY_H

#include <QSqlDatabase>
#include <QList>
#include "../model/Category.h"

class CategoryRepository
{
public:
    explicit CategoryRepository(); // Removed QSqlDatabase argument

    QList<Category> getAllCategories();
    bool insertCategory(Category& category); // Updates ID
    bool updateCategory(const Category& category);
    bool deleteCategory(long id);
    
    QList<long> getCategoriesForManga(long mangaId);
    bool setCategoriesForManga(long mangaId, const QList<long>& categoryIds);

private:
    // QSqlDatabase m_db; // Removed member // Store by value
    Category categoryFromQuery(class QSqlQuery& query);
};

#endif // CATEGORYREPOSITORY_H
