# Source file structure

| File | Description |
|---|---|
| `main.cpp` | Initializes stuff and decides what to do |
| `global.cpp` | Holds global settings to be accessed everywhere |
| `opts.cpp` | Parses command line options |
| `cache.cpp` | Handles any direct interactions with the local cache |
| `update.cpp` | Manages updating the local cache by downloading and extracting `tldr.zip` in memory |
| `page.cpp` | Holds a single page as object to parse, print or analyze it |
| `find/find.cpp` | Full-text search algorithm |
| `find/stem.cpp` | Porter stemmer for stemming a page before performing a full-text search |