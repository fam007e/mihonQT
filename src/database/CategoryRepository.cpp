#include "CategoryRepository.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QVariant>

CategoryRepository::CategoryRepository()
{
}

QList<Category> CategoryRepository::getAllCategories()
{
    QList<Category> categories;
    QSqlQuery query;
    query.prepare("SELECT * FROM categories ORDER BY sort_order ASC, name ASC");
    if (query.exec()) {
        while (query.next()) {
            categories.append(categoryFromQuery(query));
        }
    } else {
        qDebug() << "Error getting categories:" << query.lastError().text();
    }
    return categories;
}

bool CategoryRepository::insertCategory(Category& category)
{
    QSqlQuery query;
    query.prepare("INSERT INTO categories (name, sort_order, flags) VALUES (:name, :sort_order, :flags)");
    query.bindValue(":name", category.name());
    query.bindValue(":sort_order", category.order());
    query.bindValue(":flags", category.flags());

    if (!query.exec()) {
        qDebug() << "Error inserting category:" << query.lastError().text();
        return false;
    }
    category.setId(query.lastInsertId().toLongLong());
    return true;
}

bool CategoryRepository::updateCategory(const Category& category)
{
    QSqlQuery query;
    query.prepare("UPDATE categories SET name = :name, sort_order = :sort_order, flags = :flags WHERE _id = :id");
    query.bindValue(":name", category.name());
    query.bindValue(":sort_order", category.order());
    query.bindValue(":flags", category.flags());
    query.bindValue(":id", static_cast<qlonglong>(category.id()));

    if (!query.exec()) {
        qDebug() << "Error updating category:" << query.lastError().text();
        return false;
    }
    return true;
}

bool CategoryRepository::deleteCategory(long id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM categories WHERE _id = :id");
    query.bindValue(":id", static_cast<qlonglong>(id));
    if (!query.exec()) {
        qDebug() << "Error deleting category:" << query.lastError().text();
        return false;
    }
    return true;
}

QList<long> CategoryRepository::getCategoriesForManga(long mangaId)
{
    QList<long> categoryIds;
    QSqlQuery query;
    query.prepare("SELECT category_id FROM mangas_categories WHERE manga_id = :manga_id");
    query.bindValue(":manga_id", static_cast<qlonglong>(mangaId));
    
    if (query.exec()) {
        while (query.next()) {
            categoryIds.append(query.value(0).toLongLong());
        }
    } else {
        qDebug() << "Error getting categories for manga:" << query.lastError().text();
    }
    return categoryIds;
}

bool CategoryRepository::setCategoriesForManga(long mangaId, const QList<long>& categoryIds)
{
    QSqlDatabase::database().transaction();
    
    // 1. Remove existing mappings
    QSqlQuery deleteQuery;
    deleteQuery.prepare("DELETE FROM mangas_categories WHERE manga_id = :manga_id");
    deleteQuery.bindValue(":manga_id", static_cast<qlonglong>(mangaId));
    if (!deleteQuery.exec()) {
        qDebug() << "Error clearing categories for manga:" << deleteQuery.lastError().text();
        QSqlDatabase::database().rollback();
        return false;
    }

    // 2. Insert new mappings
    QSqlQuery insertQuery;
    insertQuery.prepare("INSERT INTO mangas_categories (manga_id, category_id) VALUES (:manga_id, :category_id)");
    
    for (long catId : categoryIds) {
        insertQuery.bindValue(":manga_id", static_cast<qlonglong>(mangaId));
        insertQuery.bindValue(":category_id", static_cast<qlonglong>(catId));
        if (!insertQuery.exec()) {
             qDebug() << "Error adding category mapping:" << insertQuery.lastError().text();
             QSqlDatabase::database().rollback();
             return false;
        }
    }

    QSqlDatabase::database().commit();
    return true;
}

Category CategoryRepository::categoryFromQuery(QSqlQuery& query)
{
    return Category(
        query.value("_id").toLongLong(),
        query.value("name").toString(),
        query.value("sort_order").toInt(),
        query.value("flags").toInt()
    );
}
