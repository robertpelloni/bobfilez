# Submodule Dashboard

**Last Updated:** 2026-04-17 05:34:31

## Project Structure

The project is organized as follows:

- **core/**: Contains the core logic of the filez application (scanning, hashing, database, etc.).
- **cli/**: Contains the Command Line Interface (CLI) application.
- **libs/**: Contains all external dependencies included as git submodules.
- **docs/**: Project documentation.
- **benchmarks/**: Performance benchmarks.
- **tests/**: Unit and integration tests.
- **vcpkg/**: The vcpkg package manager submodule.

## Submodules Status

| Name | Path | Version (Tag) | Commit | Date | Build # | URL |
|---|---|---|---|---|---|---|
| libs/fstlib | `libs/fstlib` | v0.1.1 | `d27a8ee` | 2020-10-01 | 170 | [Link](https://github.com/fstpackage/fstlib) |
| libs/hash-library | `libs/hash-library` | hash_library_v8 | `d389d18` | 2021-09-29 | 11 | [Link](https://github.com/stbrumme/hash-library) |
| libs/libheif | `libs/libheif` | v1.21.2 | `41ed4e31` | 2026-04-14 | 5305 | [Link](https://github.com/strukturag/libheif) |
| libs/heif | `libs/heif` | v3.7.1 | `503194e` | 2025-03-11 | 59 | [Link](https://github.com/nokiatech/heif) |
| libs/imageinfo | `libs/imageinfo` | - | `0a97e63` | 2026-03-19 | 39 | [Link](https://github.com/xiaozhuai/imageinfo) |
| libs/image_info | `libs/image_info` | v1.2.2 | `1b6222d` | 2025-07-23 | 31 | [Link](https://github.com/gottfrois/image_info) |
| libs/Windows | `libs/Windows` | build-binaries-2025-08-30 | `52fc4d3` | 2025-08-23 | 3 | [Link](https://github.com/ImageMagick/Windows) |
| libs/Dependencies | `libs/Dependencies` | 2026.04.04.0030 | `baa6b8c` | 2026-04-06 | 178 | [Link](https://github.com/ImageMagick/Dependencies) |
| libs/ImageMagick | `libs/ImageMagick` | 7.1.2-19 | `3da25c642` | 2026-04-14 | 23900 | [Link](https://github.com/ImageMagick/ImageMagick) |
| libs/gdk-pixbuf | `libs/gdk-pixbuf` | - | `18a067b` | 2026-04-03 | 104 | [Link](https://github.com/ImageMagick/gdk-pixbuf) |
| libs/Imath | `libs/Imath` | v3.2.0 | `54b2326` | 2026-03-26 | 705 | [Link](https://github.com/AcademySoftwareFoundation/Imath) |
| libs/openexr | `libs/openexr` | v3.4-alpha | `5ab3dc3b` | 2026-04-13 | 2899 | [Link](https://github.com/AcademySoftwareFoundation/openexr) |
| libs/OpenImageIO | `libs/OpenImageIO` | v3.2.0.0-dev | `b56d00dea` | 2026-04-14 | 6520 | [Link](https://github.com/AcademySoftwareFoundation/OpenImageIO) |
| libs/OpenColorIO | `libs/OpenColorIO` | v2.1.0 | `044f66ba` | 2026-04-03 | 1594 | [Link](https://github.com/AcademySoftwareFoundation/OpenColorIO) |
| libs/OpenTimelineIO | `libs/OpenTimelineIO` | v0.18.1 | `6e13de4` | 2026-04-07 | 835 | [Link](https://github.com/AcademySoftwareFoundation/OpenTimelineIO) |
| libs/OpenCue | `libs/OpenCue` | v1.19.0 | `7b9f53da` | 2026-04-10 | 1310 | [Link](https://github.com/AcademySoftwareFoundation/OpenCue) |
| libs/OpenRV | `libs/OpenRV` | v3.1.0 | `4c487b1` | 2026-04-13 | 628 | [Link](https://github.com/AcademySoftwareFoundation/OpenRV) |
| libs/openfx | `libs/openfx` | OFX_Release_1.5.1 | `b362ac6` | 2026-04-02 | 806 | [Link](https://github.com/AcademySoftwareFoundation/openfx) |
| libs/openapv | `libs/openapv` | v0.2.1.2 | `4aad1d1` | 2026-03-20 | 222 | [Link](https://github.com/AcademySoftwareFoundation/openapv) |
| libs/libxml2 | `libs/libxml2` | v2.15.0 | `310564cc` | 2026-04-13 | 7777 | [Link](https://github.com/GNOME/libxml2) |
| libs/libxmlplusplus | `libs/libxmlplusplus` | 5.6.0 | `cbb0362` | 2026-04-05 | 563 | [Link](https://github.com/libxmlplusplus/libxmlplusplus) |
| libs/json-c | `libs/json-c` | - | `52ddfb3` | 2026-02-19 | 1423 | [Link](https://github.com/json-c/json-c) |
| libs/SysmonForLinux | `libs/SysmonForLinux` | 1.5.1.0 | `a261f9e` | 2026-04-08 | 136 | [Link](https://github.com/microsoft/SysmonForLinux) |
| libs/fast-lzma2 | `libs/fast-lzma2` | v1.0.1 | `a793db9` | 2026-01-11 | 245 | [Link](https://github.com/conor42/fast-lzma2) |
| libs/pxz | `libs/pxz` | - | `824de5d` | 2026-03-17 | 86 | [Link](https://github.com/jnovy/pxz) |
| libs/openh264 | `libs/openh264` | v2.6.0 | `cf568c83` | 2025-10-28 | 5264 | [Link](https://github.com/cisco/openh264) |
| libs/libffi | `libs/libffi` | v3.5.2 | `10056e7` | 2026-04-10 | 1927 | [Link](https://github.com/libffi/libffi) |
| libs/dragonffi | `libs/dragonffi` | dffi-0.9.4 | `90c66b3` | 2025-02-08 | 166 | [Link](https://github.com/aguinet/dragonffi) |
| libs/WizardsToolkit | `libs/WizardsToolkit` | - | `98a7190` | 2026-02-15 | 998 | [Link](https://github.com/ImageMagick/WizardsToolkit) |
| libs/openjpeg | `libs/openjpeg` | v2.5.4 | `0486b86b` | 2026-04-11 | 3159 | [Link](https://github.com/uclouvain/openjpeg) |
| libs/libzip | `libs/libzip` | v1.11.4 | `e04cda2b` | 2026-03-20 | 3680 | [Link](https://github.com/nih-at/libzip) |
| libs/libiconv | `libs/libiconv` | libiconv-1.17-1 | `880a1fa` | 2024-10-27 | 44 | [Link](https://github.com/winlibs/libiconv) |
| libs/ckmame | `libs/ckmame` | v2.1 | `4cffb0c0` | 2026-02-20 | 3211 | [Link](https://github.com/nih-at/ckmame) |
| libs/nihtest | `libs/nihtest` | v1.10.1 | `02a7d5d` | 2026-01-22 | 149 | [Link](https://github.com/nih-at/nihtest) |
| libs/ziptools | `libs/ziptools` | - | `24ccea5` | 2020-05-03 | 13 | [Link](https://github.com/nih-at/ziptools) |
| libs/nihtest-cpp | `libs/nihtest-cpp` | - | `8eafcb2` | 2023-03-22 | 205 | [Link](https://github.com/nih-at/nihtest-cpp) |
| libs/openssl | `libs/openssl` | openssl-1.1.1t | `58bea78d` | 2023-02-13 | 47 | [Link](https://github.com/winlibs/openssl) |
| libs/sqlite3 | `libs/sqlite3` | sqlite3-3.51.3 | `b19a890` | 2026-03-27 | 25 | [Link](https://github.com/winlibs/sqlite3) |
| libs/postgresql | `libs/postgresql` | libpq-16.12 | `2ca5f220` | 2026-02-24 | 57 | [Link](https://github.com/winlibs/postgresql) |
| libs/cURL | `libs/cURL` | libcurl-8.19.0-3 | `2ee666d31` | 2026-03-25 | 113 | [Link](https://github.com/winlibs/cURL) |
| libs/libtidy | `libs/libtidy` | libtidy-5.6.0-1 | `cfff694` | 2025-01-21 | 14 | [Link](https://github.com/winlibs/libtidy) |
| libs/libjpeg | `libs/libjpeg` | libjpeg-turbo-3.1.4 | `067201c` | 2026-03-27 | 21 | [Link](https://github.com/winlibs/libjpeg) |
| libs/glib | `libs/glib` | glib-2.53.3-1 | `e9f0a75` | 2024-11-18 | 21 | [Link](https://github.com/winlibs/glib) |
| libs/enchant | `libs/enchant` | enchant-2.2.8 | `f8238b4` | 2024-11-18 | 17 | [Link](https://github.com/winlibs/enchant) |
| libs/net-snmp | `libs/net-snmp` | net-snmp-5.9.4 | `0c47808` | 2024-09-30 | 18 | [Link](https://github.com/winlibs/net-snmp) |
| libs/openldap | `libs/openldap` | openldap-2.4.47-1 | `b2ffd90` | 2024-10-31 | 44 | [Link](https://github.com/winlibs/openldap) |
| libs/nghttp2 | `libs/nghttp2` | nghttp2-1.68.1 | `4964a9d` | 2026-03-25 | 24 | [Link](https://github.com/winlibs/nghttp2) |
| libs/libxpm | `libs/libxpm` | libxpm-3.5.18 | `7e1166c` | 2026-03-27 | 20 | [Link](https://github.com/winlibs/libxpm) |
| libs/cyrus-sasl | `libs/cyrus-sasl` | libsasl-2.1.27-3 | `d933c03` | 2022-06-11 | 10 | [Link](https://github.com/winlibs/cyrus-sasl) |
| libs/icu4c | `libs/icu4c` | icu4c-78.2 | `25b56cd3` | 2026-01-10 | 41 | [Link](https://github.com/winlibs/icu4c) |
| libs/mpir | `libs/mpir` | mpir-3.0.0-1 | `62672a0` | 2024-10-30 | 18 | [Link](https://github.com/winlibs/mpir) |
| libs/gettext | `libs/gettext` | libintl-0.18.3-9 | `04c409c` | 2024-10-29 | 17 | [Link](https://github.com/winlibs/gettext) |
| libs/libxslt | `libs/libxslt` | libxslt-1.1.39 | `dc7d30c` | 2024-02-10 | 30 | [Link](https://github.com/winlibs/libxslt) |
| libs/libwebp | `libs/libwebp` | libwebp-1.6.0 | `1b7801d` | 2026-03-27 | 15 | [Link](https://github.com/winlibs/libwebp) |
| libs/freetype | `libs/freetype` | freetype-2.14.3 | `3d8e776` | 2026-03-27 | 29 | [Link](https://github.com/winlibs/freetype) |
| libs/libavif | `libs/libavif` | libavif-1.4.1 | `fcfda0d` | 2026-03-27 | 9 | [Link](https://github.com/winlibs/libavif) |
| libs/libssh2 | `libs/libssh2` | libssh2-1.11.1 | `a768549` | 2024-10-27 | 32 | [Link](https://github.com/winlibs/libssh2) |
| libs/libpng | `libs/libpng` | libpng-1.6.56 | `d7e9c8a` | 2026-03-27 | 29 | [Link](https://github.com/winlibs/libpng) |
| libs/libsodium | `libs/libsodium` | libsodium-1.0.21 | `298db89` | 2026-02-26 | 11 | [Link](https://github.com/winlibs/libsodium) |
| libs/qdbm | `libs/qdbm` | libqdbm-1.8.78 | `586aa47` | 2016-09-26 | 2 | [Link](https://github.com/winlibs/qdbm) |
| libs/oniguruma | `libs/oniguruma` | libonig-6.9.9 | `ac76cdc` | 2024-10-26 | 11 | [Link](https://github.com/winlibs/oniguruma) |
| libs/argon2 | `libs/argon2` | libargon2-20190702 | `fb53ef3` | 2019-07-02 | 9 | [Link](https://github.com/winlibs/argon2) |
| libs/lmdb | `libs/lmdb` | liblmdb-0.9.33 | `46aef27` | 2024-10-26 | 8 | [Link](https://github.com/winlibs/lmdb) |
| libs/libbzip2 | `libs/libbzip2` | libbzip2-1.0.8-1 | `99d3c48` | 2020-02-11 | 8 | [Link](https://github.com/winlibs/libbzip2) |
| libs/zlib | `libs/zlib` | zlib-1.3.2 | `0089522` | 2026-02-25 | 21 | [Link](https://github.com/winlibs/zlib) |
| libs/ssdeep | `libs/ssdeep` | libfuzzy-2.14.1 | `a326864` | 2026-04-09 | 9 | [Link](https://github.com/winlibs/ssdeep) |
| libs/wineditline | `libs/wineditline` | WinEditLine-2.206 | `60eb709` | 2022-05-03 | 18 | [Link](https://github.com/winlibs/wineditline) |
| libs/pthreads | `libs/pthreads` | pthreads-3.0.0 | `648488d` | 2022-03-22 | 2 | [Link](https://github.com/winlibs/pthreads) |
| libs/imap | `libs/imap` | imap-2007f-1 | `4f5969e` | 2022-02-01 | 4 | [Link](https://github.com/winlibs/imap) |
| libs/pslib | `libs/pslib` | pslib-0.4.6 | `4c6b93b` | 2021-11-19 | 1 | [Link](https://github.com/winlibs/pslib) |
| libs/libmcrypt | `libs/libmcrypt` | libmcrypt-2.5.8-3.4 | `1e93cbe` | 2019-09-17 | 4 | [Link](https://github.com/winlibs/libmcrypt) |
| libs/dirent | `libs/dirent` | dirent-20080629 | `f7d4841` | 2019-09-17 | 6 | [Link](https://github.com/winlibs/dirent) |
| libs/c-ares | `libs/c-ares` | - | `6c783a9` | 2016-09-16 | 1 | [Link](https://github.com/winlibs/c-ares) |
| libs/httpd | `libs/httpd` | - | `15cc845c25` | 2026-04-14 | 34375 | [Link](https://github.com/apache/httpd) |
| libs/fribidi | `libs/fribidi` | - | `cbcc997` | 2013-09-16 | 2 | [Link](https://github.com/winlibs/fribidi) |
| libs/libvpx | `libs/libvpx` | libvpx-1.1.0 | `5e76714` | 2013-04-09 | 4 | [Link](https://github.com/winlibs/libvpx) |
| libs/libvbucket | `libs/libvbucket` | 2.1.1r | `5590fff` | 2014-03-20 | 126 | [Link](https://github.com/membase/libvbucket) |
| libs/FFmpeg | `libs/FFmpeg` | n8.2-dev | `3f9e04b489` | 2026-04-14 | 123961 | [Link](https://github.com/FFmpeg/FFmpeg) |
| libs/hashcat | `libs/hashcat` | v7.1.2 | `2d71af371` | 2025-11-20 | 11055 | [Link](https://github.com/hashcat/hashcat) |
| libs/radare2 | `libs/radare2` | 6.1.4 | `43dcccffe3` | 2026-04-14 | 35483 | [Link](https://github.com/radareorg/radare2) |
| libs/C | `libs/C` | - | `e5dad3fa` | 2023-09-27 | 1536 | [Link](https://github.com/TheAlgorithms/C) |
| libs/vlc | `libs/vlc` | 4.0.0-dev | `093ad617ba` | 2026-04-14 | 109844 | [Link](https://github.com/videolan/vlc) |
| libs/sumatrapdf | `libs/sumatrapdf` | 3.2 | `125c97b6d` | 2026-04-14 | 17321 | [Link](https://github.com/sumatrapdfreader/sumatrapdf) |
| libs/mpv | `libs/mpv` | v0.41.0 | `da4789c2dd` | 2026-04-12 | 54905 | [Link](https://github.com/mpv-player/mpv) |
| libs/sqlite | `libs/sqlite` | major-release | `a10e79d91a` | 2026-04-14 | 31650 | [Link](https://github.com/sqlite/sqlite) |
| libs/tinyxml2 | `libs/tinyxml2` | 11.0.0 | `3dcad8e` | 2026-03-09 | 1282 | [Link](https://github.com/leethomason/tinyxml2) |
| libs/libvips | `libs/libvips` | v8.18.2 | `d2c3ed235` | 2026-04-09 | 9252 | [Link](https://github.com/libvips/libvips) |
| libs/opencv | `libs/opencv` | 4.13.0 | `bc0dff8c3e` | 2026-04-14 | 36085 | [Link](https://github.com/opencv/opencv) |
| libs/MediaInfoLib | `libs/MediaInfoLib` | v26.01 | `56152550f` | 2026-04-14 | 9784 | [Link](https://github.com/MediaArea/MediaInfoLib) |
| libs/MediaInfo | `libs/MediaInfo` | v26.01 | `c98a8f48d` | 2026-04-14 | 2798 | [Link](https://github.com/MediaArea/MediaInfo) |
| libs/mm_file | `libs/mm_file` | v1.0.0 | `5b52913` | 2024-08-22 | 15 | [Link](https://github.com/jermp/mm_file) |
| libs/TinyEXIF | `libs/TinyEXIF` | 1.0.4 | `b6ea1b7` | 2026-03-19 | 40 | [Link](https://github.com/cdcseacave/TinyEXIF) |
| libs/libjpeg-turbo | `libs/libjpeg-turbo` | 3.1.90 | `9b09d516` | 2026-04-14 | 3001 | [Link](https://github.com/libjpeg-turbo/libjpeg-turbo) |
| libs/jhead | `libs/jhead` | 3.08 | `536137e` | 2026-04-01 | 540 | [Link](https://github.com/Matthias-Wandel/jhead) |
| libs/libexif | `libs/libexif` | libexif-0_6_26-release | `8fd2d12` | 2026-04-14 | 1514 | [Link](https://github.com/libexif/libexif) |
| libs/getopt-win32 | `libs/getopt-win32` | - | `b3de5c2` | 2017-08-30 | 3 | [Link](https://github.com/nanoporetech/getopt-win32) |
| libs/raylib | `libs/raylib` | 5.5 | `019cc889` | 2026-04-13 | 9738 | [Link](https://github.com/raysan5/raylib) |
| libs/lvgl | `libs/lvgl` | v9.5.0 | `cf300e48a` | 2026-04-14 | 12442 | [Link](https://github.com/lvgl/lvgl) |
| libs/getopt-win | `libs/getopt-win` | v2.42.0 | `36e55dd` | 2025-10-21 | 33 | [Link](https://github.com/ludvikjerabek/getopt-win) |
| libs/libunistd | `libs/libunistd` | v1.4 | `3f269df` | 2026-02-16 | 551 | [Link](https://github.com/robinrowe/libunistd) |
| libs/ShazamAPI | `libs/ShazamAPI` | - | `ff8f89d` | 2024-05-15 | 30 | [Link](https://github.com/Numenorean/ShazamAPI) |
| libs/seek-tune | `libs/seek-tune` | - | `a6507e8` | 2025-11-19 | 337 | [Link](https://github.com/cgzirim/seek-tune) |
| libs/ShazamIO | `libs/ShazamIO` | 0.8.1 | `b5321b5` | 2025-06-11 | 315 | [Link](https://github.com/shazamio/ShazamIO) |
| libs/audio-recognizer | `libs/audio-recognizer` | - | `3028b61` | 2016-03-16 | 10 | [Link](https://github.com/wsieroci/audio-recognizer) |
| libs/pHash | `libs/pHash` | - | `e98831e` | 2026-01-14 | 55 | [Link](https://github.com/nihas101/pHash) |
| libs/image-hash | `libs/image-hash` | v0.0.7 | `a9dc7a9` | 2026-03-17 | 233 | [Link](https://github.com/ytetsuro/image-hash) |
| libs/imghash-viewer | `libs/imghash-viewer` | v0.1.0 | `8b72a9f` | 2024-04-28 | 28 | [Link](https://github.com/YannickAlex07/imghash-viewer) |
| libs/imagehash | `libs/imagehash` | v1.0.7 | `1a775c3` | 2024-03-30 | 43 | [Link](https://github.com/vitali-fedulov/imagehash) |
| libs/securecopy | `libs/securecopy` | v9.7.0 | `a82171b` | 2025-08-14 | 28 | [Link](https://github.com/hansij66/securecopy) |
| libs/xattrs | `libs/xattrs` | - | `9fd00c6` | 2019-08-23 | 44 | [Link](https://github.com/hrbrmstr/xattrs) |
| libs/fuse_xattrs | `libs/fuse_xattrs` | v0.3 | `d1e3046` | 2021-12-02 | 36 | [Link](https://github.com/fbarriga/fuse_xattrs) |
| libs/xattrlib | `libs/xattrlib` | - | `286d38b` | 2011-06-03 | 5 | [Link](https://github.com/amdf/xattrlib) |
| libs/RenStrm | `libs/RenStrm` | 1.0 | `3874e39` | 2018-06-13 | 7 | [Link](https://github.com/hernandp/RenStrm) |
| libs/metastore | `libs/metastore` | v1.1.2 | `8f0dd4c` | 2023-02-01 | 174 | [Link](https://github.com/przemoc/metastore) |
| libs/lsads | `libs/lsads` | 2013-12-29_release | `e2d1599` | 2023-10-02 | 29 | [Link](https://github.com/assarbad/lsads) |
| libs/Powershell-ADS | `libs/Powershell-ADS` | - | `5227a03` | 2020-01-07 | 3 | [Link](https://github.com/phlashko/Powershell-ADS) |
| libs/SharpADS | `libs/SharpADS` | v1.0 | `ea6d0b9` | 2023-08-25 | 12 | [Link](https://github.com/ricardojoserf/SharpADS) |
| libs/ADSIdentifier | `libs/ADSIdentifier` | - | `099e808` | 2025-03-18 | 22 | [Link](https://github.com/HannahVernon/ADSIdentifier) |
| libs/ads | `libs/ads` | - | `87fc3aa` | 2019-04-14 | 3 | [Link](https://github.com/katahiromz/ads) |
| libs/AlternateDataStreams | `libs/AlternateDataStreams` | - | `34cd1cb` | 2016-10-31 | 7 | [Link](https://github.com/narasimha14/AlternateDataStreams) |
| libs/DataStreamBrowser | `libs/DataStreamBrowser` | - | `88db018` | 2020-08-19 | 1 | [Link](https://github.com/Thibstars/DataStreamBrowser) |
| libs/libevent | `libs/libevent` | release-2.2.1-alpha | `48296514` | 2026-03-25 | 5173 | [Link](https://github.com/libevent/libevent) |
| libs/xxHash | `libs/xxHash` | v0.7.4 | `e573d4d` | 2026-04-13 | 2073 | [Link](https://github.com/Cyan4973/xxHash) |
| libs/libgit2 | `libs/libgit2` | v1.9.0 | `1f34e2a57` | 2026-03-31 | 16143 | [Link](https://github.com/libgit2/libgit2) |
| libs/dokany | `libs/dokany` | v2.3.1.1000 | `767da4b` | 2026-03-22 | 1760 | [Link](https://github.com/dokan-dev/dokany) |
| libs/pngquant | `libs/pngquant` | 3.0.3 | `5b4e91f` | 2026-03-28 | 1210 | [Link](https://github.com/kornelski/pngquant) |
| libs/dunst | `libs/dunst` | v1.13.2 | `bca10d8` | 2026-03-28 | 2631 | [Link](https://github.com/dunst-project/dunst) |
| libs/audiocraft | `libs/audiocraft` | v1.3.0 | `896ec7c` | 2025-03-13 | 253 | [Link](https://github.com/facebookresearch/audiocraft) |
| libs/hashingImage | `libs/hashingImage` | - | `f00adc9` | 2017-05-08 | 2 | [Link](https://github.com/goongong/hashingImage) |
| libs/util-linux | `libs/util-linux` | v2.43-devel | `2cc4450d8` | 2026-04-09 | 22216 | [Link](https://github.com/util-linux/util-linux) |
| libs/libimghash | `libs/libimghash` | - | `ec97409` | 2014-01-31 | 40 | [Link](https://github.com/shuttie/libimghash) |
| libs/perceptual-dct-hash | `libs/perceptual-dct-hash` | - | `9646476` | 2019-06-04 | 28 | [Link](https://github.com/alangshur/perceptual-dct-hash) |
| libs/pHash.c | `libs/pHash.c` | v1.1.0 | `1d7c056` | 2025-02-08 | 31 | [Link](https://github.com/folksable/pHash.c) |
| libs/tinyphash | `libs/tinyphash` | - | `7c7c503` | 2020-09-24 | 14 | [Link](https://github.com/Wassasin/tinyphash) |
| libs/ADSman | `libs/ADSman` | - | `187e647` | 2025-06-18 | 2 | [Link](https://github.com/8gudbits/ADSman) |
| libs/ADSFileSystem | `libs/ADSFileSystem` | - | `60b6710` | 2016-06-09 | 9 | [Link](https://github.com/marcelogm/ADSFileSystem) |
| libs/Bringing-Old-Photos-Back-to-Life | `libs/Bringing-Old-Photos-Back-to-Life` | v1.0 | `33875ec` | 2022-07-23 | 108 | [Link](https://github.com/microsoft/Bringing-Old-Photos-Back-to-Life) |
| libs/BLAKE3 | `libs/BLAKE3` | 1.8.4 | `15e83a5` | 2026-04-13 | 769 | [Link](https://github.com/BLAKE3-team/BLAKE3.git) |
| libs/btk | `libs/btk` | cs-2.1.0 | `4d1bccde9` | 2026-04-15 | 3600 | [Link](https://github.com/robertpelloni/btk.git) |
| libs/bobui | `libs/bobui` | - | `677b0f352a` | 2026-04-15 | 77856 | [Link](https://github.com/robertpelloni/bobui.git) |
| libs/pandoc | `libs/pandoc` | 3.9.0.2 | `5caad90fc` | 2026-04-13 | 18927 | [Link](https://github.com/jgm/pandoc) |
| libs/ghostpdl | `libs/ghostpdl` | ghostpdl-10.05.0-test-base-001 | `872e7b014` | 2026-04-13 | 24670 | [Link](https://github.com/ArtifexSoftware/ghostpdl) |
| libs/LibRaw | `libs/LibRaw` | 0.22.0 | `f74ddd99` | 2026-04-07 | 2868 | [Link](https://github.com/LibRaw/LibRaw) |
| libs/libde265 | `libs/libde265` | v1.0.18 | `2032bd83` | 2026-04-12 | 2449 | [Link](https://github.com/strukturag/libde265) |
| libs/poppler | `libs/poppler` | poppler-26.04.0 | `e092d116` | 2026-04-11 | 8716 | [Link](https://gitlab.freedesktop.org/poppler/poppler) |
| libs/calibre | `libs/calibre` | v9.7.0 | `a92353468b` | 2026-04-14 | 53347 | [Link](https://github.com/kovidgoyal/calibre) |
| libs/wkhtmltopdf | `libs/wkhtmltopdf` | 0.12.6 | `024b2b2` | 2022-06-29 | 1175 | [Link](https://github.com/wkhtmltopdf/wkhtmltopdf) |
| libs/Magick.NET | `libs/Magick.NET` | 14.12.0 | `3af7e57da` | 2026-04-14 | 6137 | [Link](https://github.com/dlemstra/Magick.NET) |
| libs/ripgrep | `libs/ripgrep` | ignore-0.4.25 | `4519153` | 2026-02-27 | 2209 | [Link](https://github.com/BurntSushi/ripgrep) |
| libs/the_silver_searcher | `libs/the_silver_searcher` | 2.2.0 | `a61f178` | 2020-12-16 | 2057 | [Link](https://github.com/ggreer/the_silver_searcher) |
| libs/pcre2 | `libs/pcre2` | pcre2-10.47 | `ac0eb712` | 2026-04-03 | 2237 | [Link](https://github.com/PCRE2Project/pcre2) |
| libs/re2 | `libs/re2` | 2025-11-05 | `972a15c` | 2026-01-22 | 1165 | [Link](https://github.com/google/re2) |
| libs/rename-utils | `libs/rename-utils` | v2.1 | `4cffb0c0` | 2026-02-20 | 3211 | [Link](https://github.com/nih-at/ckmame) |
| libs/sigil | `libs/sigil` | 2.7.6 | `d75d3084b` | 2026-04-12 | 7248 | [Link](https://github.com/Sigil-Ebook/Sigil) |
| libs/libarchive | `libs/libarchive` | v3.7.5 | `6d9dddba` | 2026-04-13 | 7147 | [Link](https://github.com/libarchive/libarchive) |
| libs/librsync | `libs/librsync` | v2.3.4 | `271744d` | 2025-08-29 | 1675 | [Link](https://github.com/librsync/librsync) |
| libs/zstd | `libs/zstd` | v1.4.7 | `48c0ed73` | 2026-04-01 | 11463 | [Link](https://github.com/facebook/zstd) |
| libs/lz4 | `libs/lz4` | v1.9.2 | `9da37b2e` | 2026-04-01 | 3798 | [Link](https://github.com/lz4/lz4) |
| libs/brotli | `libs/brotli` | v1.2.0 | `737ae47` | 2026-04-15 | 1497 | [Link](https://github.com/google/brotli) |
| libs/p7zip | `libs/p7zip` | pre-p7zip21.07 | `6819e2d` | 2025-05-20 | 427 | [Link](https://github.com/p7zip-project/p7zip) |
| libs/dtl-diff | `libs/dtl-diff` | v1.21 | `32567bb` | 2024-07-11 | 41 | [Link](https://github.com/cubicdaiya/dtl) |
| libs/cmark | `libs/cmark` | 0.31.2 | `64efa3b` | 2026-03-01 | 2646 | [Link](https://github.com/commonmark/cmark) |
| libs/7zip | `libs/7zip` | v0.6.13.0 | `fea94d3` | 2025-10-01 | 851 | [Link](https://github.com/nicehash/NiceHashQuickMiner) |
| libs/glad | `libs/glad` | v2.0.8 | `cef3f89` | 2026-04-09 | 549 | [Link](https://github.com/Dav1dde/glad) |
| libs/rapidjson | `libs/rapidjson` | v1.1.0 | `24b5e7a8` | 2025-02-05 | 2481 | [Link](https://github.com/Tencent/rapidjson) |
| libs/md4c | `libs/md4c` | release-0.5.2 | `481fbfb` | 2024-02-25 | 681 | [Link](https://github.com/mity/md4c) |
| libs/highlightjs | `libs/highlightjs` | 11.11.1 | `5697ae51` | 2025-07-06 | 6702 | [Link](https://github.com/highlightjs/highlight.js) |
| libs/bobgui | `libs/bobgui` | 4.22.0 | `ce7fcfeb92` | 2026-04-14 | 90542 | [Link](https://github.com/robertpelloni/bobgui) |
| libs/JUCE | `libs/JUCE` | 8.0.12 | `501c07674e` | 2026-01-16 | 15597 | [Link](https://github.com/juce-framework/JUCE.git) |
| ai-file-sorter | `ai-file-sorter` | v1.7.3 | `630b46a` | 2026-03-25 | 638 | [Link](https://github.com/hyperfield/ai-file-sorter) |
| VERT"]
	url = https://github.com/VERT-sh/vert
	path = VERT
[submodule "libs/ultimatepp | `libs/ultimatepp` | v2026.1 | `162be7b33` | 2026-04-16 | 16226 | [Link](https://github.com/ultimatepp/ultimatepp) |
