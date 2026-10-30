MP3 TAG READER - BEGINNER VERSION

Files:
    main.c
    sample.mp3
    dup_sample.mp3

Compile:
    gcc main.c -o mp3tag

View tags:
    ./mp3tag -v sample.mp3

Edit examples:
    ./mp3tag -e -t "Believer" sample.mp3
    ./mp3tag -e -a "Imagine Dragons" sample.mp3
    ./mp3tag -e -A "Evolve" sample.mp3
    ./mp3tag -e -y "2017" sample.mp3
    ./mp3tag -e -m "Rock" sample.mp3
    ./mp3tag -e -c "My song" sample.mp3

After editing, the changed MP3 is written to:
    dup_sample.mp3

Note:
This is intentionally kept beginner-friendly and is designed around
the simple ID3v2 frame layout used by the supplied project.
