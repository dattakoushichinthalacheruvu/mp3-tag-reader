# MP3 Tag Reader and Editor

A C-based project to read and display ID3 tags from MP3 files, and edit them. Parses MP3 metadata without any external library.

### Features
- Reads ID3v2 tags (Title, Artist, Album, Year, Genre, Comment)
- Displays metadata of any MP3 file
- Edits MP3 tags (Change Title, Artist etc.)
- Handles both ID3v1 and ID3v2
- Command-line interface

### How it Works
- Reads MP3 file in binary mode
- Parses ID3 header and frames
- Extracts and displays tag information
- Allows editing by rewriting tag frames

### Tech Stack
- Language: C
- Concepts: File I/O, Binary Files, Structures, ID3 Tag Format
- Compiler: GCC

### How to Run
```bash
gcc main.c mp3.c -o mp3tag
./mp3tag -r song.mp3        # Read tags
./mp3tag -e -t "New Title" song.mp3  # Edit title
