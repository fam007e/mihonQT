#ifndef CHAPTERSORT_H
#define CHAPTERSORT_H

#include "Chapter.h"
#include "Manga.h"
#include <functional>

/**
 * Chapter sorting matching Android Mihon (tachiyomi.domain.chapter.service.ChapterSort.kt)
 */
inline std::function<bool(const Chapter&, const Chapter&)> getChapterComparator(const Manga& manga, bool sortDescending)
{
    long mode = manga.sortingMode();
    return [mode, sortDescending](const Chapter& c1, const Chapter& c2) {
        if (mode == Manga::CHAPTER_SORTING_NUMBER) {
            if (c1.chapterNumber() != c2.chapterNumber()) {
                return sortDescending ? (c1.chapterNumber() > c2.chapterNumber())
                                      : (c1.chapterNumber() < c2.chapterNumber());
            }
        } else if (mode == Manga::CHAPTER_SORTING_UPLOAD_DATE) {
            if (c1.dateUpload() != c2.dateUpload()) {
                return sortDescending ? (c1.dateUpload() > c2.dateUpload())
                                      : (c1.dateUpload() < c2.dateUpload());
            }
        } else if (mode == Manga::CHAPTER_SORTING_ALPHABET) {
            int cmp = QString::compare(c1.name(), c2.name(), Qt::CaseInsensitive);
            if (cmp != 0) {
                return sortDescending ? (cmp > 0) : (cmp < 0);
            }
        }

        // Default: CHAPTER_SORTING_SOURCE
        // Source order: 0 is the newest chapter from source (e.g. Chapter 1076).
        // If sortDescending (typical UI view): sourceOrder 0 comes first (c1 < c2).
        // If sortAscending (reading order): sourceOrder N comes first (c1 > c2).
        if (c1.sourceOrder() != c2.sourceOrder()) {
            return sortDescending ? (c1.sourceOrder() < c2.sourceOrder())
                                  : (c1.sourceOrder() > c2.sourceOrder());
        }

        // Secondary fallback to chapter number
        if (c1.chapterNumber() != c2.chapterNumber()) {
            return sortDescending ? (c1.chapterNumber() > c2.chapterNumber())
                                  : (c1.chapterNumber() < c2.chapterNumber());
        }

        return c1.id() < c2.id();
    };
}

#endif // CHAPTERSORT_H
