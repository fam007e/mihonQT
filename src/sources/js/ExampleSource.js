// ExampleSource.js - A proof-of-concept JavaScript source

// Global object representing the source, to be picked up by JavascriptSource C++ class
var source = {
    name: "Example JS Source",
    id: 9999, // Unique ID for this source
    lang: "en",
    supportsLatest: true,

    getPopularManga: function() {
        console.log("JS: getPopularManga called");
        return [
            {
                url: "https://example.com/manga/js1",
                title: "JS Manga One",
                artist: "JS Artist",
                author: "JS Author",
                description: "This is the first manga from the JavaScript example source.",
                genre: "Action, Adventure",
                status: 0, // UNKNOWN
                thumbnail_url: "",
                favorite: false,
                lastUpdate: Date.now(),
                nextUpdate: 0,
                fetchInterval: 0,
                dateAdded: Date.now(),
                viewerFlags: 0,
                chapterFlags: 0,
                coverLastModified: 0,
                updateStrategy: 0, // ALWAYS_UPDATE
                initialized: true,
                lastModifiedAt: Date.now(),
                favoriteModifiedAt: 0,
                version: 1,
                notes: ""
            },
            {
                url: "https://example.com/manga/js2",
                title: "JS Manga Two",
                artist: "Another JS Artist",
                author: "Another JS Author",
                description: "The second manga from the JavaScript example source.",
                genre: "Comedy, Romance",
                status: 1, // ONGOING
                thumbnail_url: "",
                favorite: false,
                lastUpdate: Date.now(),
                nextUpdate: 0,
                fetchInterval: 0,
                dateAdded: Date.now(),
                viewerFlags: 0,
                chapterFlags: 0,
                coverLastModified: 0,
                updateStrategy: 0,
                initialized: true,
                lastModifiedAt: Date.now(),
                favoriteModifiedAt: 0,
                version: 1,
                notes: ""
            }
        ];
    },

    getLatestUpdates: function() {
        console.log("JS: getLatestUpdates called");
        // For simplicity, return the same as popular manga
        return this.getPopularManga();
    },

    getSearchManga: function(query) {
        console.log("JS: getSearchManga called with query:", query);
        // Simulate search
        return this.getPopularManga().filter(manga => manga.title.toLowerCase().includes(query.toLowerCase()));
    },

    getMangaDetails: function(mangaId, mangaUrl) {
        console.log("JS: getMangaDetails called for ID:", mangaId, "URL:", mangaUrl);
        // Find the manga by ID/URL from dummy data
        let foundManga = this.getPopularManga().find(manga => manga.url === mangaUrl);
        if (foundManga) {
            // Add/update some details
            foundManga.description += " (Details fetched from JS)";
        }
        return foundManga || {};
    },

    getChapterList: function(mangaId, mangaUrl) {
        console.log("JS: getChapterList called for ID:", mangaId, "URL:", mangaUrl);
        return [
            {
                url: mangaUrl + "/chapter1",
                name: "Chapter 1: The Beginning",
                dateUpload: Date.now(),
                chapterNumber: 1.0,
                scanlator: "JS Scans"
            },
            {
                url: mangaUrl + "/chapter2",
                name: "Chapter 2: The Middle",
                dateUpload: Date.now(),
                chapterNumber: 2.0,
                scanlator: "JS Scans"
            }
        ];
    }
};

// Example of using the exposed C++ Network object (if implemented)
// var response = Network.get("https://api.example.com/data");
// console.log("JS: Network response:", response);
