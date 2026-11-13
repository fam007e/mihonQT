#include <stdint.h>
#include <stddef.h>
#include <vector>
#include <algorithm>
#include <cstring>
#include "../src/model/Chapter.h"
#include "../src/model/Manga.h"
#include "../src/model/ChapterSort.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < 8) return 0;

    // Use first 4 bytes for manga flags & mode
    uint32_t rawFlags;
    std::memcpy(&rawFlags, data, 4);
    data += 4;
    size -= 4;

    Manga manga;
    manga.setChapterFlags(rawFlags);
    bool sortDescending = (rawFlags & 1) != 0;

    auto comp = getChapterComparator(manga, sortDescending);

    // Build vector of chapters from remaining fuzzer data
    QList<Chapter> chapters;
    int index = 0;
    while (size >= 16 && chapters.size() < 100) {
        float chapterNumber;
        std::memcpy(&chapterNumber, data, 4);
        int sourceOrder = static_cast<int>(data[4]);
        long dateUpload = static_cast<long>(data[5]) * 1000;
        long id = static_cast<long>(data[6]);

        QString name = QString("Ch %1").arg(data[7]);

        chapters.append(Chapter(id, 1, QString("/ch/%1").arg(index), name, "scan", false, false, 0, chapterNumber, sourceOrder, 0, dateUpload, 0, 0, 0));

        data += 8;
        size -= 8;
        index++;
    }

    if (chapters.size() >= 2) {
        // Test comparator and sort
        std::sort(chapters.begin(), chapters.end(), comp);

        // Verify strict weak ordering on sample
        for (int i = 0; i < chapters.size() - 1; ++i) {
            bool forward = comp(chapters[i], chapters[i+1]);
            bool reverse = comp(chapters[i+1], chapters[i]);
            // Both cannot be true simultaneously (asymmetry)
            if (forward && reverse) {
                __builtin_trap();
            }
        }
    }

    return 0;
}
