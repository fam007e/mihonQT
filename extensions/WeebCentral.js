var source = {
  name: "WeebCentral",
  id: 1234567, // Unique ID
  lang: "en",
  supportsLatest: true,
  baseUrl: "https://weebcentral.com",

  getPopularManga: function (page) {
    page = page || 1;
    var offset = (page - 1) * 32;
    var url = this.baseUrl + "/search/data?text=&sort=Popularity&order=Descending&official=Any&display_mode=Full%20Display&limit=32&offset=" + offset;
    console.log("WeebCentral: Fetching popular " + url);
    var html = Network.get(url);
    if (!html) {
      console.log("WeebCentral: Network returned empty");
      return [];
    }
    return this._parseMangaList(html);
  },

  getLatestUpdates: function (page) {
    page = page || 1;
    var offset = (page - 1) * 32;
    var url = this.baseUrl + "/search/data?text=&sort=Latest%20Updates&order=Descending&official=Any&display_mode=Full%20Display&limit=32&offset=" + offset;
    console.log("WeebCentral: Fetching latest " + url);
    var html = Network.get(url);
    if (!html) {
      console.log("WeebCentral: Network returned empty");
      return [];
    }
    return this._parseMangaList(html);
  },

  getSearchManga: function (query) {
    var url = this.baseUrl + "/search/data?text=" + encodeURIComponent(query) + "&sort=Best%20Match&order=Ascending&official=Any&display_mode=Full%20Display&limit=32&offset=0";
    console.log("WeebCentral: Searching " + url);
    var html = Network.get(url);
    if (!html) {
      console.log("WeebCentral: Network returned empty");
      return [];
    }
    return this._parseMangaList(html);
  },

  _parseMangaList: function (html) {
    if (html.length < 1000) {
      console.log("WeebCentral: Received short HTML: " + html);
    } else {
      console.log("WeebCentral: Received HTML preview: " + html.substring(0, 1000));
    }

    // Debug raw string presence
    console.log("WeebCentral: Index of <article>: " + html.indexOf("<article"));

    var doc = Html.parse(html);
    var items = doc.select("article");
    console.log("WeebCentral: Found " + items.length + " manga items");
    var mangaList = [];
    var seenUrls = [];

    for (var i = 0; i < items.length; i++) {
      var item = items[i];

      // Android selector: article > section > a
      var seriesLink = item.select("a[href*='/series/']");
      if (seriesLink.length === 0) continue;

      var href = seriesLink[0].attr("href");
      if (seenUrls.indexOf(href) !== -1) continue;
      seenUrls.push(href);

      var thumbnail = "";
      var sources = item.select("source");
      if (sources.length > 0) {
        thumbnail = sources[0].attr("srcset");
      }
      if (!thumbnail) {
        var img = item.select("img");
        if (img.length > 0) thumbnail = img[0].attr("src");
      }

      var title = "Unknown Title";

      // Title logic: Look for explicit link in text section matching series href
      // OR data-tip

      var titleLinks = item.select("a.link-hover");
      for (var j = 0; j < titleLinks.length; j++) {
        var tl = titleLinks[j];
        if (tl.attr("href") === href && tl.text.trim().length > 0) {
          title = tl.text.trim();
          break;
        }
      }

      if (title === "Unknown Title") {
        var spans = item.select("span[data-tip]");
        for (var k = 0; k < spans.length; k++) {
          var tip = spans[k].attr("data-tip");
          if (tip && tip !== "Official Translation" && tip !== "Doujinshi") {
            title = tip;
            break;
          }
        }
      }

      mangaList.push({
        url: href,
        title: title,
        thumbnail_url: thumbnail,
        status: 0,
        initialized: false
      });
    }
    return mangaList;
  },

  getMangaDetails: function (id, url) {
    if (!url.startsWith("http")) url = this.baseUrl + url;
    var html = Network.get(url);
    var doc = Html.parse(html);

    var manga = {
      url: url,
      initialized: true
    };

    var lis = doc.select("li");
    for (var i = 0; i < lis.length; i++) {
      var li = lis[i];
      var text = li.text;

      if (text.indexOf("Description") !== -1) {
        var p = li.first("p");
        if (p) manga.description = p.text;
      }
      if (text.indexOf("Author") !== -1) {
        var a = li.first("a");
        if (a) manga.author = a.text;
      }
      if (text.indexOf("Status") !== -1) {
        var a = li.first("a");
        if (a) {
          var status = a.text.toLowerCase();
          if (status.includes("ongoing")) manga.status = 1;
          else if (status.includes("complete")) manga.status = 2;
          else manga.status = 0;
        }
      }
    }

    if (!manga.description) {
      var meta = doc.select("meta[name='description']");
      if (meta.length > 0) manga.description = meta[0].attr("content");
    }

    return manga;
  },

  getChapterList: function (id, url) {
    if (!url.startsWith("http")) url = this.baseUrl + url;

    // Direct extraction matching Android counterpart (WeebCentral.kt):
    // Android fetches /series/{seriesId}/full-chapter-list directly
    var seriesIdMatch = url.match(/\/series\/([^\/]+)/);
    var targetUrl = url;
    if (seriesIdMatch) {
      targetUrl = this.baseUrl + "/series/" + seriesIdMatch[1] + "/full-chapter-list";
    }

    console.log("WeebCentral: Fetching chapter list from " + targetUrl);
    var html = Network.get(targetUrl);
    if (!html && targetUrl !== url) {
      // Fallback to base url if full-chapter-list failed
      html = Network.get(url);
    }
    if (!html) return [];

    var doc = Html.parse(html);
    var chapters = [];

    // Android selector: div[x-data] > a
    var links = doc.select("div[x-data] > a");
    if (links.length === 0) {
      links = doc.select("a[href*='/chapters/']");
    }

    var seenUrls = {};
    for (var i = 0; i < links.length; i++) {
      var link = links[i];
      var href = link.attr("href");
      if (!href || href.indexOf("/chapters/") === -1) continue;
      if (seenUrls[href]) continue;
      seenUrls[href] = true;

      // Extract chapter name
      var name = "";
      var span = link.select("span.flex > span");
      if (span.length > 0 && span[0].text && span[0].text.trim().length > 0) {
        name = span[0].text.trim();
      } else {
        var grow = link.select("span.grow");
        if (grow.length > 0 && grow[0].text && grow[0].text.trim().length > 0) {
          name = grow[0].text.trim();
        } else {
          name = link.text.trim();
        }
      }

      name = name.replace("Last Read", "").trim();
      name = name.replace(/\s+/g, " ");
      if (!name) continue; // Skip non-chapter links

      var dateUpload = Date.now();
      var time = link.select("time");
      if (time.length > 0) {
        var dt = time[0].attr("datetime") || time[0].text;
        if (dt) {
          dateUpload = Date.parse(dt) || Date.now();
        }
      }

      var num = 0.0;
      var match = name.match(/(?:Chapter|Episode)\s+([\d.]+)/i);
      if (match) {
        num = parseFloat(match[1]);
      } else {
        var numMatch = name.match(/[\d.]+/);
        if (numMatch) num = parseFloat(numMatch[0]);
      }

      var isOfficial = false;
      var imgs = link.select("img");
      for (var imgIdx = 0; imgIdx < imgs.length; imgIdx++) {
        var src = imgs[imgIdx].attr("src") || "";
        if (src.toLowerCase().indexOf("official") !== -1) {
          isOfficial = true;
          break;
        }
      }

      chapters.push({
        url: href,
        name: name,
        dateUpload: dateUpload,
        chapterNumber: num,
        scanlator: isOfficial ? "Official" : "WeebCentral"
      });
    }

    console.log("WeebCentral: Extracted " + chapters.length + " chapters in source order");
    return chapters;
  },

  getPageList: function (chapterId, url) {
    if (!url.startsWith("http")) url = this.baseUrl + url;

    // Android "pageListRequest" logic:
    // Append /images?is_prev=False&reading_style=long_strip
    var pageListUrl = url;
    if (pageListUrl.endsWith("/")) pageListUrl = pageListUrl.slice(0, -1);
    pageListUrl += "/images?is_prev=False&reading_style=long_strip";

    console.log("WeebCentral: Fetching pages from " + pageListUrl);

    var html = Network.get(pageListUrl);

    // Debug HTML if short
    if (html.length < 500) {
      console.log("WeebCentral: PageHTML: " + html);
    }

    var doc = Html.parse(html);
    var pages = [];

    // Android selector: document.select("section[x-data~=scroll] > img")
    // Our CSS selector engine might strictly handle ~= so let's try strict or just section > img
    // The key is it returns a list of images.

    var imgs = doc.select("section[x-data] > img");
    console.log("WeebCentral: Found " + imgs.length + " images with selector section[x-data] > img");

    if (imgs.length === 0) {
      imgs = doc.select("section#chapter-images > img");
    }
    if (imgs.length === 0) {
      imgs = doc.select("img");
      console.log("WeebCentral: Found " + imgs.length + " total images, filtering...");
    }

    var pageIndex = 0;
    for (var i = 0; i < imgs.length; i++) {
      var img = imgs[i];
      var src = img.attr("src");
      if (!src) continue;

      if (src.startsWith("/")) {
        src = this.baseUrl + src;
      }

      if (src.startsWith("http")) {
        // Exclude broken image fallback or tiny icons
        if (src.indexOf("broken_image") === -1 && src.indexOf("icon") === -1) {
          pages.push({ index: pageIndex++, url: src });
        }
      }
    }
    return pages;
  }
};
