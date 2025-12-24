-- MihonQT Database Schema
--
-- This schema is adapted from the original Mihon Android application's
-- SQLDelight schema. Triggers and some complex types are omitted for
-- the initial MVP and will be handled by C++ application logic where necessary.

CREATE TABLE IF NOT EXISTS mangas(
    _id INTEGER NOT NULL PRIMARY KEY,
    source INTEGER NOT NULL,
    url TEXT NOT NULL,
    artist TEXT,
    author TEXT,
    description TEXT,
    genre TEXT, -- Stored as a comma-separated string
    title TEXT NOT NULL,
    status INTEGER NOT NULL,
    thumbnail_url TEXT,
    favorite INTEGER NOT NULL, -- Boolean (0 or 1)
    last_update INTEGER,
    next_update INTEGER,
    initialized INTEGER NOT NULL, -- Boolean (0 or 1)
    viewer INTEGER NOT NULL,
    chapter_flags INTEGER NOT NULL,
    cover_last_modified INTEGER NOT NULL,
    date_added INTEGER NOT NULL,
    update_strategy INTEGER NOT NULL,
    calculate_interval INTEGER NOT NULL,
    last_modified_at INTEGER NOT NULL,
    favorite_modified_at INTEGER,
    version INTEGER NOT NULL,
    is_syncing INTEGER NOT NULL,
    notes TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS chapters(
    _id INTEGER PRIMARY KEY AUTOINCREMENT,
    manga_id INTEGER NOT NULL,
    url TEXT NOT NULL,
    name TEXT NOT NULL,
    scanlator TEXT,
    read INTEGER NOT NULL, -- Boolean (0 or 1)
    bookmark INTEGER NOT NULL, -- Boolean (0 or 1)
    last_page_read INTEGER NOT NULL,
    chapter_number REAL NOT NULL,
    source_order INTEGER NOT NULL,
    date_fetch INTEGER NOT NULL,
    date_upload INTEGER NOT NULL,
    last_modified_at INTEGER NOT NULL,
    version INTEGER NOT NULL DEFAULT 0,
    is_syncing INTEGER NOT NULL DEFAULT 0,
    FOREIGN KEY(manga_id) REFERENCES mangas(_id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS categories (
    _id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    sort_order INTEGER,
    flags INTEGER
);

CREATE TABLE IF NOT EXISTS mangas_categories (
    _id INTEGER PRIMARY KEY AUTOINCREMENT,
    manga_id INTEGER NOT NULL,
    category_id INTEGER NOT NULL,
    FOREIGN KEY(manga_id) REFERENCES mangas(_id) ON DELETE CASCADE,
    FOREIGN KEY(category_id) REFERENCES categories(_id) ON DELETE CASCADE
);

-- History table for tracking reading progress
CREATE TABLE IF NOT EXISTS history (
    _id INTEGER PRIMARY KEY AUTOINCREMENT,
    chapter_id INTEGER NOT NULL UNIQUE,
    last_read INTEGER NOT NULL,
    time_read INTEGER NOT NULL DEFAULT 0,
    FOREIGN KEY(chapter_id) REFERENCES chapters(_id) ON DELETE CASCADE
);
