# Submodule Dashboard

**Last Updated:** 2026-11-24 05:21:37

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
| libs/fstlib | `libs/fstlib` | - | `3a698e1f3` | 2026-11-24 | 138 | [Link](https://github.com/fstpackage/fstlib) |
| libs/hash-library | `libs/hash-library` | hash_library_v8 | `d389d18` | 2021-09-29 | 11 | [Link](https://github.com/stbrumme/hash-library) |
| libs/libheif | `libs/libheif` | - | `3a698e1f3` | 2026-11-24 | 138 | [Link](https://github.com/strukturag/libheif) |
| libs/heif | `libs/heif` | - | `3a698e1f3` | 2026-11-24 | 138 | [Link](https://github.com/nokiatech/heif) |
| libs/imageinfo | `libs/imageinfo` | - | `3a698e1f3` | 2026-11-24 | 138 | [Link](https://github.com/xiaozhuai/imageinfo) |
| libs/image_info | `libs/image_info` | - | `3a698e1f3` | 2026-11-24 | 138 | [Link](https://github.com/gottfrois/image_info) |
| libs/Windows | `libs/Windows` | - | `3a698e1f3` | 2026-11-24 | 138 | [Link](https://github.com/ImageMagick/Windows) |
| libs/Dependencies | `libs/Dependencies` | 2026.07.03.1508 | `2cc16a3` | 2026-07-17 | 237 | [Link](https://github.com/ImageMagick/Dependencies) |
| libs/ImageMagick | `libs/ImageMagick` | 7.1.2-27 | `3105066a1` | 2026-07-17 | 24411 | [Link](https://github.com/ImageMagick/ImageMagick) |
| libs/gdk-pixbuf | `libs/gdk-pixbuf` | - | `1284323` | 2026-07-11 | 114 | [Link](https://github.com/ImageMagick/gdk-pixbuf) |
| libs/Imath | `libs/Imath` | v3.2.2-rc2 | `1163027` | 2026-07-11 | 813 | [Link](https://github.com/AcademySoftwareFoundation/Imath) |
| libs/openexr | `libs/openexr` | v3.4-alpha | `2fea5747` | 2026-07-17 | 3310 | [Link](https://github.com/AcademySoftwareFoundation/openexr) |
| libs/OpenImageIO | `libs/OpenImageIO` | v3.2.0.2-dev | `f28a67deb` | 2026-07-17 | 6997 | [Link](https://github.com/AcademySoftwareFoundation/OpenImageIO) |
| libs/OpenColorIO | `libs/OpenColorIO` | v2.5.2 | `6cad7bf8` | 2026-07-17 | 1755 | [Link](https://github.com/AcademySoftwareFoundation/OpenColorIO) |
| libs/OpenTimelineIO | `libs/OpenTimelineIO` | v0.18.1 | `9665dca` | 2026-07-17 | 935 | [Link](https://github.com/AcademySoftwareFoundation/OpenTimelineIO) |
| libs/OpenCue | `libs/OpenCue` | v1.19.0 | `3f46372a` | 2026-07-11 | 1652 | [Link](https://github.com/AcademySoftwareFoundation/OpenCue) |
| libs/OpenRV | `libs/OpenRV` | v4.0.0 | `e2f29e5a` | 2026-07-17 | 755 | [Link](https://github.com/AcademySoftwareFoundation/OpenRV) |
| libs/openfx | `libs/openfx` | OFX_Release_1.5.1 | `ae250d35` | 2026-07-17 | 862 | [Link](https://github.com/AcademySoftwareFoundation/openfx) |
| libs/openapv | `libs/openapv` | v0.3.0.0 | `f05e6fd` | 2026-07-11 | 684 | [Link](https://github.com/AcademySoftwareFoundation/openapv) |
| libs/libxml2 | `libs/libxml2` | v2.15.3 | `69aa9a10` | 2026-07-11 | 7976 | [Link](https://github.com/GNOME/libxml2) |
| libs/libxmlplusplus | `libs/libxmlplusplus` | 5.6.1 | `252dd14` | 2026-07-17 | 622 | [Link](https://github.com/libxmlplusplus/libxmlplusplus) |
| libs/json-c | `libs/json-c` | json-c-0.19-20260627 | `e7c8935` | 2026-07-11 | 1501 | [Link](https://github.com/json-c/json-c) |
| libs/SysmonForLinux | `libs/SysmonForLinux` | 1.5.2 | `bfa131b` | 2026-07-17 | 173 | [Link](https://github.com/microsoft/SysmonForLinux) |
| libs/fast-lzma2 | `libs/fast-lzma2` | v1.0.1 | `3daee62` | 2026-07-11 | 285 | [Link](https://github.com/conor42/fast-lzma2) |
| libs/pxz | `libs/pxz` | - | `61913d4` | 2026-07-11 | 90 | [Link](https://github.com/jnovy/pxz) |
| libs/openh264 | `libs/openh264` | v2.6.0 | `a4f25f8c` | 2026-07-17 | 5329 | [Link](https://github.com/cisco/openh264) |
| libs/libffi | `libs/libffi` | v3.7.1 | `a48411da` | 2026-07-17 | 2086 | [Link](https://github.com/libffi/libffi) |
| libs/dragonffi | `libs/dragonffi` | dffi-0.9.4 | `93c210b` | 2026-07-17 | 292 | [Link](https://github.com/aguinet/dragonffi) |
| libs/WizardsToolkit | `libs/WizardsToolkit` | - | `3fa1d2b` | 2026-07-17 | 1004 | [Link](https://github.com/ImageMagick/WizardsToolkit) |
| libs/openjpeg | `libs/openjpeg` | v2.5.4 | `4f1eeb45` | 2026-07-17 | 3186 | [Link](https://github.com/uclouvain/openjpeg) |
| libs/libzip | `libs/libzip` | v1.11.4 | `4f4ddaba` | 2026-07-17 | 3739 | [Link](https://github.com/nih-at/libzip) |
| libs/libiconv | `libs/libiconv` | libiconv-1.19-1 | `713968a` | 2026-07-11 | 56 | [Link](https://github.com/winlibs/libiconv) |
| libs/ckmame | `libs/ckmame` | v2.1 | `ba693417` | 2026-07-17 | 3256 | [Link](https://github.com/nih-at/ckmame) |
| libs/nihtest | `libs/nihtest` | v1.11.1 | `ab8f3ab` | 2026-07-11 | 157 | [Link](https://github.com/nih-at/nihtest) |
| libs/ziptools | `libs/ziptools` | - | `a2c9d23` | 2026-07-11 | 18 | [Link](https://github.com/nih-at/ziptools) |
| libs/nihtest-cpp | `libs/nihtest-cpp` | - | `78d7c69` | 2026-07-11 | 210 | [Link](https://github.com/nih-at/nihtest-cpp) |
| libs/openssl | `libs/openssl` | openssl-4.0.0 | `cfed08db` | 2026-07-17 | 88 | [Link](https://github.com/winlibs/openssl) |
| libs/sqlite3 | `libs/sqlite3` | sqlite3-3.53.2 | `698fc5a` | 2026-07-11 | 30 | [Link](https://github.com/winlibs/sqlite3) |
| libs/postgresql | `libs/postgresql` | libpq-16.13 | `6b8c1b87c` | 2026-07-11 | 72 | [Link](https://github.com/winlibs/postgresql) |
| libs/cURL | `libs/cURL` | libcurl-8.21.0-2 | `cda84dbe2` | 2026-07-11 | 132 | [Link](https://github.com/winlibs/cURL) |
| libs/libtidy | `libs/libtidy` | libtidy-5.8.0 | `cf7f5dc` | 2026-07-17 | 24 | [Link](https://github.com/winlibs/libtidy) |
| libs/libjpeg | `libs/libjpeg` | libjpeg-turbo-3.1.4.1 | `a63ceb5` | 2026-07-11 | 31 | [Link](https://github.com/winlibs/libjpeg) |
| libs/glib | `libs/glib` | glib-2.88.1-1 | `21e31f0` | 2026-07-11 | 53 | [Link](https://github.com/winlibs/glib) |
| libs/enchant | `libs/enchant` | libenchant2-2.8.16-1 | `b4553b9` | 2026-07-11 | 27 | [Link](https://github.com/winlibs/enchant) |
| libs/net-snmp | `libs/net-snmp` | net-snmp-5.9.5.2 | `a44a7bc` | 2026-07-17 | 26 | [Link](https://github.com/winlibs/net-snmp) |
| libs/openldap | `libs/openldap` | openldap-2.6.13-3 | `5bd8eda` | 2026-07-11 | 81 | [Link](https://github.com/winlibs/openldap) |
| libs/nghttp2 | `libs/nghttp2` | nghttp2-1.69.0-1 | `12405a7` | 2026-07-11 | 36 | [Link](https://github.com/winlibs/nghttp2) |
| libs/libxpm | `libs/libxpm` | libxpm-3.5.19 | `a66144c` | 2026-07-11 | 25 | [Link](https://github.com/winlibs/libxpm) |
| libs/cyrus-sasl | `libs/cyrus-sasl` | libsasl-2.1.28-2 | `4e17287` | 2026-07-11 | 22 | [Link](https://github.com/winlibs/cyrus-sasl) |
| libs/icu4c | `libs/icu4c` | icu4c-78.2 | `f3777e96` | 2026-07-11 | 52 | [Link](https://github.com/winlibs/icu4c) |
| libs/mpir | `libs/mpir` | mpir-3.0.0-2 | `e712973` | 2026-07-11 | 22 | [Link](https://github.com/winlibs/mpir) |
| libs/gettext | `libs/gettext` | gettext-1.0 | `4a2685f` | 2026-07-11 | 22 | [Link](https://github.com/winlibs/gettext) |
| libs/libxslt | `libs/libxslt` | libxslt-1.1.43-2 | `00f29b3` | 2026-07-11 | 43 | [Link](https://github.com/winlibs/libxslt) |
| libs/libwebp | `libs/libwebp` | libwebp-1.6.0 | `cca7c0d` | 2026-07-11 | 20 | [Link](https://github.com/winlibs/libwebp) |
| libs/freetype | `libs/freetype` | freetype-2.14.3 | `fc0f4cf` | 2026-07-11 | 35 | [Link](https://github.com/winlibs/freetype) |
| libs/libavif | `libs/libavif` | libavif-1.4.2 | `700438b` | 2026-07-11 | 18 | [Link](https://github.com/winlibs/libavif) |
| libs/libssh2 | `libs/libssh2` | libssh2-1.11.1-5 | `72c9ef3` | 2026-07-11 | 47 | [Link](https://github.com/winlibs/libssh2) |
| libs/libpng | `libs/libpng` | libpng-1.6.58 | `ea87c41` | 2026-07-17 | 41 | [Link](https://github.com/winlibs/libpng) |
| libs/libsodium | `libs/libsodium` | libsodium-1.0.22 | `3766aae` | 2026-07-11 | 15 | [Link](https://github.com/winlibs/libsodium) |
| libs/qdbm | `libs/qdbm` | libqdbm-1.8.78 | `a81e590` | 2026-07-11 | 6 | [Link](https://github.com/winlibs/qdbm) |
| libs/oniguruma | `libs/oniguruma` | libonig-6.9.10 | `9ed516e` | 2026-07-11 | 17 | [Link](https://github.com/winlibs/oniguruma) |
| libs/argon2 | `libs/argon2` | libargon2-20190702 | `96fd458` | 2026-07-11 | 13 | [Link](https://github.com/winlibs/argon2) |
| libs/lmdb | `libs/lmdb` | liblmdb-0.9.35 | `ebd1ff9` | 2026-07-11 | 14 | [Link](https://github.com/winlibs/lmdb) |
| libs/libbzip2 | `libs/libbzip2` | libbzip2-1.0.8-1 | `8a2c3e3` | 2026-07-11 | 12 | [Link](https://github.com/winlibs/libbzip2) |
| libs/zlib | `libs/zlib` | zlib-1.3.2 | `2687a49` | 2026-07-11 | 28 | [Link](https://github.com/winlibs/zlib) |
| libs/ssdeep | `libs/ssdeep` | libfuzzy-2.14.1 | `5e83651` | 2026-07-11 | 12 | [Link](https://github.com/winlibs/ssdeep) |
| libs/wineditline | `libs/wineditline` | WinEditLine-2.208 | `e1a03f7` | 2026-07-17 | 26 | [Link](https://github.com/winlibs/wineditline) |
| libs/pthreads | `libs/pthreads` | pthreads-3.0.0 | `8a11cb9` | 2026-07-11 | 6 | [Link](https://github.com/winlibs/pthreads) |
| libs/imap | `libs/imap` | imap-2007f-1 | `1073dc7` | 2026-07-11 | 8 | [Link](https://github.com/winlibs/imap) |
| libs/pslib | `libs/pslib` | pslib-0.4.6 | `09bbd0d` | 2026-07-11 | 6 | [Link](https://github.com/winlibs/pslib) |
| libs/libmcrypt | `libs/libmcrypt` | libmcrypt-2.5.8-3.4 | `9b37908` | 2026-07-11 | 8 | [Link](https://github.com/winlibs/libmcrypt) |
| libs/dirent | `libs/dirent` | dirent-20080629 | `01abed1` | 2026-07-11 | 10 | [Link](https://github.com/winlibs/dirent) |
| libs/c-ares | `libs/c-ares` | - | `5cd2363` | 2026-07-11 | 4 | [Link](https://github.com/winlibs/c-ares) |
| libs/httpd | `libs/httpd` | - | `9142092dd0` | 2026-07-11 | 39518 | [Link](https://github.com/apache/httpd) |
| libs/fribidi | `libs/fribidi` | - | `fdaeffd` | 2026-07-11 | 5 | [Link](https://github.com/winlibs/fribidi) |
| libs/libvpx | `libs/libvpx` | libvpx-1.1.0 | `cecbf61` | 2026-07-11 | 7 | [Link](https://github.com/winlibs/libvpx) |
| libs/libvbucket | `libs/libvbucket` | 2.1.1r | `9975a33` | 2026-07-11 | 131 | [Link](https://github.com/membase/libvbucket) |
| libs/FFmpeg | `libs/FFmpeg` | n9.1-dev | `d5a121dadb` | 2026-07-17 | 126220 | [Link](https://github.com/FFmpeg/FFmpeg) |
| libs/hashcat | `libs/hashcat` | v7.1.2 | `3582662b6` | 2026-07-11 | 11062 | [Link](https://github.com/hashcat/hashcat) |
| libs/radare2 | `libs/radare2` | 6.1.8 | `c5be9edb81` | 2026-07-17 | 36273 | [Link](https://github.com/radareorg/radare2) |
| libs/C | `libs/C` | - | `38bbf87b` | 2026-07-11 | 1575 | [Link](https://github.com/TheAlgorithms/C) |
| libs/vlc | `libs/vlc` | 3.0.23-2 | `b273b9d663` | 2026-07-17 | 118967 | [Link](https://github.com/videolan/vlc) |
| libs/sumatrapdf | `libs/sumatrapdf` | 3.2 | `2b83be83b` | 2026-07-17 | 19132 | [Link](https://github.com/sumatrapdfreader/sumatrapdf) |
| libs/mpv | `libs/mpv` | git-release | `610755ef7c` | 2026-07-11 | 55366 | [Link](https://github.com/mpv-player/mpv) |
| libs/sqlite | `libs/sqlite` | version-3.53.0 | `a2a36806ee` | 2026-07-17 | 32139 | [Link](https://github.com/sqlite/sqlite) |
| libs/tinyxml2 | `libs/tinyxml2` | 11.0.0 | `5ed6c68` | 2026-07-11 | 1327 | [Link](https://github.com/leethomason/tinyxml2) |
| libs/libvips | `libs/libvips` | v8.18.4 | `814806ace` | 2026-07-17 | 9538 | [Link](https://github.com/libvips/libvips) |
| libs/opencv | `libs/opencv` | 4.13.0 | `089055e6cc` | 2026-07-11 | 36450 | [Link](https://github.com/opencv/opencv) |
| libs/MediaInfoLib | `libs/MediaInfoLib` | v26.05 | `3c434659c` | 2026-07-11 | 9945 | [Link](https://github.com/MediaArea/MediaInfoLib) |
| libs/MediaInfo | `libs/MediaInfo` | v26.05 | `948e91657` | 2026-07-11 | 2886 | [Link](https://github.com/MediaArea/MediaInfo) |
| libs/mm_file | `libs/mm_file` | v1.0.0 | `def5082` | 2026-07-11 | 19 | [Link](https://github.com/jermp/mm_file) |
| libs/TinyEXIF | `libs/TinyEXIF` | 1.0.4 | `434bd4c` | 2026-07-11 | 45 | [Link](https://github.com/cdcseacave/TinyEXIF) |
| libs/libjpeg-turbo | `libs/libjpeg-turbo` | 3.2.0 | `3fd7778f` | 2026-07-17 | 3030 | [Link](https://github.com/libjpeg-turbo/libjpeg-turbo) |
| libs/jhead | `libs/jhead` | 3.08 | `0ae9a1c` | 2026-07-11 | 552 | [Link](https://github.com/Matthias-Wandel/jhead) |
| libs/libexif | `libs/libexif` | libexif-0_6_26-release | `acbfe61` | 2026-07-17 | 1542 | [Link](https://github.com/libexif/libexif) |
| libs/getopt-win32 | `libs/getopt-win32` | - | `815ba4a` | 2026-07-11 | 10 | [Link](https://github.com/nanoporetech/getopt-win32) |
| libs/raylib | `libs/raylib` | 6.0 | `75e18dcc6` | 2026-07-17 | 10017 | [Link](https://github.com/raysan5/raylib) |
| libs/lvgl | `libs/lvgl` | v9.5.0 | `490eaa27b` | 2026-07-17 | 12705 | [Link](https://github.com/lvgl/lvgl) |
| libs/getopt-win | `libs/getopt-win` | v2.42.0 | `b5e50e5` | 2026-07-11 | 37 | [Link](https://github.com/ludvikjerabek/getopt-win) |
| libs/libunistd | `libs/libunistd` | v1.4 | `b808590` | 2026-07-11 | 555 | [Link](https://github.com/robinrowe/libunistd) |
| libs/ShazamAPI | `libs/ShazamAPI` | - | `a49d130` | 2026-07-11 | 36 | [Link](https://github.com/Numenorean/ShazamAPI) |
| libs/seek-tune | `libs/seek-tune` | - | `92167a4` | 2026-07-11 | 341 | [Link](https://github.com/cgzirim/seek-tune) |
| libs/ShazamIO | `libs/ShazamIO` | 0.8.1 | `2eb12b2` | 2026-07-11 | 325 | [Link](https://github.com/shazamio/ShazamIO) |
| libs/audio-recognizer | `libs/audio-recognizer` | - | `291d1fc` | 2026-07-11 | 13 | [Link](https://github.com/wsieroci/audio-recognizer) |
| libs/pHash | `libs/pHash` | - | `f591ce5` | 2026-07-11 | 58 | [Link](https://github.com/nihas101/pHash) |
| libs/image-hash | `libs/image-hash` | v0.0.7 | `bb73fb593` | 2026-07-11 | 265 | [Link](https://github.com/ytetsuro/image-hash) |
| libs/imghash-viewer | `libs/imghash-viewer` | v0.1.0 | `8c0966c` | 2026-07-11 | 32 | [Link](https://github.com/YannickAlex07/imghash-viewer) |
| libs/imagehash | `libs/imagehash` | v1.0.7 | `546bf7b` | 2026-07-11 | 47 | [Link](https://github.com/vitali-fedulov/imagehash) |
| libs/securecopy | `libs/securecopy` | v9.7.0 | `aa76ba9` | 2026-07-11 | 31 | [Link](https://github.com/hansij66/securecopy) |
| libs/xattrs | `libs/xattrs` | - | `ce73f4c` | 2026-07-11 | 47 | [Link](https://github.com/hrbrmstr/xattrs) |
| libs/fuse_xattrs | `libs/fuse_xattrs` | v0.3 | `c58b455` | 2026-07-11 | 40 | [Link](https://github.com/fbarriga/fuse_xattrs) |
| libs/xattrlib | `libs/xattrlib` | - | `f75fe22` | 2026-07-11 | 8 | [Link](https://github.com/amdf/xattrlib) |
| libs/RenStrm | `libs/RenStrm` | 1.0 | `9c8bbe9` | 2026-07-11 | 10 | [Link](https://github.com/hernandp/RenStrm) |
| libs/metastore | `libs/metastore` | v1.1.2 | `8a31742` | 2026-07-11 | 178 | [Link](https://github.com/przemoc/metastore) |
| libs/lsads | `libs/lsads` | 2013-12-29_release | `78aca62` | 2026-07-11 | 32 | [Link](https://github.com/assarbad/lsads) |
| libs/Powershell-ADS | `libs/Powershell-ADS` | - | `780a069` | 2026-07-11 | 6 | [Link](https://github.com/phlashko/Powershell-ADS) |
| libs/SharpADS | `libs/SharpADS` | v1.0 | `f5b8fdc` | 2026-07-11 | 15 | [Link](https://github.com/ricardojoserf/SharpADS) |
| libs/ADSIdentifier | `libs/ADSIdentifier` | - | `7ce7348` | 2026-07-11 | 25 | [Link](https://github.com/HannahVernon/ADSIdentifier) |
| libs/ads | `libs/ads` | - | `6dbac68` | 2026-07-11 | 7 | [Link](https://github.com/katahiromz/ads) |
| libs/AlternateDataStreams | `libs/AlternateDataStreams` | - | `f806cb9` | 2026-07-11 | 10 | [Link](https://github.com/narasimha14/AlternateDataStreams) |
| libs/DataStreamBrowser | `libs/DataStreamBrowser` | - | `8aeb446` | 2026-07-11 | 5 | [Link](https://github.com/robertpelloni/DataStreamBrowser) |
| libs/libevent | `libs/libevent` | release-2.2.2-alpha | `ffe1b42d` | 2026-07-17 | 5269 | [Link](https://github.com/libevent/libevent) |
| libs/xxHash | `libs/xxHash` | v0.7.4 | `ee252bc` | 2026-06-29 | 2074 | [Link](https://github.com/Cyan4973/xxHash) |
| libs/libgit2 | `libs/libgit2` | v1.9.0 | `7b30d1bd6` | 2026-07-17 | 16343 | [Link](https://github.com/libgit2/libgit2) |
| libs/dokany | `libs/dokany` | v2.3.1.1000 | `13cb020` | 2026-07-17 | 1837 | [Link](https://github.com/dokan-dev/dokany) |
| libs/pngquant | `libs/pngquant` | 3.0.3 | `1331cfe` | 2026-07-17 | 1229 | [Link](https://github.com/kornelski/pngquant) |
| libs/dunst | `libs/dunst` | v1.13.2 | `de7325b` | 2026-07-11 | 2650 | [Link](https://github.com/dunst-project/dunst) |
| libs/audiocraft | `libs/audiocraft` | v1.3.0 | `eb623ab` | 2026-07-11 | 325 | [Link](https://github.com/facebookresearch/audiocraft) |
| libs/hashingImage | `libs/hashingImage` | - | `a30f691` | 2026-07-11 | 7 | [Link](https://github.com/goongong/hashingImage) |
| libs/util-linux | `libs/util-linux` | v2.42.2 | `918d38d94` | 2026-07-11 | 23290 | [Link](https://github.com/util-linux/util-linux) |
| libs/libimghash | `libs/libimghash` | - | `da4939c` | 2026-07-11 | 45 | [Link](https://github.com/shuttie/libimghash) |
| libs/perceptual-dct-hash | `libs/perceptual-dct-hash` | - | `b614f51` | 2026-07-11 | 32 | [Link](https://github.com/alangshur/perceptual-dct-hash) |
| libs/pHash.c | `libs/pHash.c` | v1.1.0 | `6d4d363` | 2026-07-11 | 37 | [Link](https://github.com/folksable/pHash.c) |
| libs/tinyphash | `libs/tinyphash` | - | `16961e5` | 2026-07-11 | 17 | [Link](https://github.com/Wassasin/tinyphash) |
| libs/ADSman | `libs/ADSman` | - | `3764c25` | 2026-07-11 | 6 | [Link](https://github.com/8gudbits/ADSman) |
| libs/ADSFileSystem | `libs/ADSFileSystem` | - | `920b7b6` | 2026-07-11 | 12 | [Link](https://github.com/robertpelloni/ADSFileSystem) |
| libs/Bringing-Old-Photos-Back-to-Life | `libs/Bringing-Old-Photos-Back-to-Life` | v1.0 | `49c8ca5` | 2026-07-11 | 111 | [Link](https://github.com/microsoft/Bringing-Old-Photos-Back-to-Life) |
| libs/BLAKE3 | `libs/BLAKE3` | 1.8.5 | `8aa5145` | 2026-05-21 | 777 | [Link](https://github.com/BLAKE3-team/BLAKE3.git) |
| libs/btk | `libs/btk` | cs-2.1.0 | `d2c7d7071` | 2026-07-11 | 3654 | [Link](https://github.com/robertpelloni/bcs.git) |
| libs/bobui | `libs/bobui` | - | `861c53858d` | 2026-07-17 | 77988 | [Link](https://github.com/robertpelloni/bqt.git) |
| libs/pandoc | `libs/pandoc` | 3.10 | `fc4098281` | 2026-07-17 | 206 | [Link](https://github.com/jgm/pandoc) |
| libs/ghostpdl | `libs/ghostpdl` | - | `c455da86a` | 2026-07-17 | 163 | [Link](https://github.com/ArtifexSoftware/ghostpdl) |
| libs/LibRaw | `libs/LibRaw` | 0.22.0 | `284871d8` | 2026-07-17 | 2896 | [Link](https://github.com/LibRaw/LibRaw) |
| libs/libde265 | `libs/libde265` | v1.1.1 | `f063197` | 2026-07-11 | 74 | [Link](https://github.com/strukturag/libde265) |
| libs/poppler | `libs/poppler` | poppler-26.07.0 | `a342802b7` | 2026-07-17 | 191 | [Link](https://gitlab.freedesktop.org/poppler/poppler) |
| libs/calibre | `libs/calibre` | v9.11.0 | `0f8a585b79` | 2026-07-17 | 54006 | [Link](https://github.com/kovidgoyal/calibre) |
| libs/wkhtmltopdf | `libs/wkhtmltopdf` | - | `6819e62` | 2026-07-17 | 6 | [Link](https://github.com/wkhtmltopdf/wkhtmltopdf) |
| libs/Magick.NET | `libs/Magick.NET` | 14.15.0 | `5acd87d83` | 2026-07-17 | 149 | [Link](https://github.com/dlemstra/Magick.NET) |
| libs/ripgrep | `libs/ripgrep` | ignore-0.4.30 | `e8c8419` | 2026-07-17 | 64 | [Link](https://github.com/BurntSushi/ripgrep) |
| libs/the_silver_searcher | `libs/the_silver_searcher` | - | `7f2c6ce` | 2026-07-11 | 2 | [Link](https://github.com/ggreer/the_silver_searcher) |
| libs/pcre2 | `libs/pcre2` | - | `7d389886` | 2026-07-17 | 16 | [Link](https://github.com/PCRE2Project/pcre2) |
| libs/re2 | `libs/re2` | - | `2b3e785` | 2026-07-11 | 2 | [Link](https://github.com/google/re2) |
| libs/rename-utils | `libs/rename-utils` | - | `e518c048` | 2026-07-17 | 42 | [Link](https://github.com/nih-at/ckmame) |
| libs/sigil | `libs/sigil` | 2.8.0 | `7b3321322` | 2026-07-17 | 111 | [Link](https://github.com/Sigil-Ebook/Sigil) |
| libs/libarchive | `libs/libarchive` | v3.7.5 | `28c3a52a` | 2026-07-17 | 7983 | [Link](https://github.com/libarchive/libarchive) |
| libs/librsync | `libs/librsync` | - | `59e3308` | 2026-07-11 | 2 | [Link](https://github.com/librsync/librsync) |
| libs/zstd | `libs/zstd` | - | `eb9339be` | 2026-07-11 | 14 | [Link](https://github.com/facebook/zstd) |
| libs/lz4 | `libs/lz4` | v1.9.2 | `0d04f459` | 2026-07-11 | 3825 | [Link](https://github.com/lz4/lz4) |
| libs/brotli | `libs/brotli` | v1.2.0 | `2e4b72e` | 2026-07-17 | 1497 | [Link](https://github.com/google/brotli) |
| libs/p7zip | `libs/p7zip` | - | `f860145` | 2026-07-11 | 2 | [Link](https://github.com/p7zip-project/p7zip) |
| libs/dtl-diff | `libs/dtl-diff` | v1.21 | `e70e736` | 2026-07-11 | 2 | [Link](https://github.com/cubicdaiya/dtl) |
| libs/cmark | `libs/cmark` | - | `969b278` | 2026-07-11 | 20 | [Link](https://github.com/commonmark/cmark) |
| libs/7zip | `libs/7zip` | - | `924831a` | 2026-07-11 | 2 | [Link](https://github.com/nicehash/NiceHashQuickMiner) |
| libs/glad | `libs/glad` | - | `5a45e47` | 2026-07-11 | 4 | [Link](https://github.com/Dav1dde/glad) |
| libs/rapidjson | `libs/rapidjson` | - | `9e9419a` | 2026-07-17 | 7 | [Link](https://github.com/Tencent/rapidjson) |
| libs/md4c | `libs/md4c` | release-0.5.3 | `0347a40` | 2026-07-11 | 118 | [Link](https://github.com/mity/md4c) |
| libs/highlightjs | `libs/highlightjs` | 11.11.2 | `a2212676` | 2026-07-17 | 55 | [Link](https://github.com/highlightjs/highlight.js) |
| libs/bobgui | `libs/bobgui` | 4.22.0 | `955b67e812` | 2026-07-17 | 93666 | [Link](https://github.com/robertpelloni/bgtk) |
| libs/JUCE | `libs/JUCE` | 8.0.13 | `762a87fc30` | 2026-07-10 | 16075 | [Link](https://github.com/juce-framework/JUCE.git) |
| ai-file-sorter | `ai-file-sorter` | v1.8.0 | `b3f5aad` | 2026-07-17 | 840 | [Link](https://github.com/hyperfield/ai-file-sorter) |
| VERT"]
	url = https://github.com/VERT-sh/vert
	path = VERT
[submodule "libs/ultimatepp | `libs/ultimatepp` | v2026.1 | `9105a8650` | 2026-07-17 | 16343 | [Link](https://github.com/ultimatepp/ultimatepp) |
