# Submodule Dashboard

**Last Updated:** 2026-04-01 21:07:41

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
| libs/fstlib | `libs/fstlib` | v0.1.1 | `cb252c7` | 2026-03-01 | 185 | [Link](https://github.com/fstpackage/fstlib) |
| libs/hash-library | `libs/hash-library` | hash_library_v8 | `1f94d39` | 2026-02-26 | 13 | [Link](https://github.com/stbrumme/hash-library) |
| libs/libheif | `libs/libheif` | v1.21.2 | `4bd2148b` | 2026-04-01 | 5213 | [Link](https://github.com/strukturag/libheif) |
| libs/heif | `libs/heif` | v3.7.1 | `b3a8a98` | 2026-03-01 | 62 | [Link](https://github.com/nokiatech/heif) |
| libs/imageinfo | `libs/imageinfo` | - | `0a6e4be` | 2026-03-19 | 45 | [Link](https://github.com/xiaozhuai/imageinfo) |
| libs/image_info | `libs/image_info` | v1.2.2 | `f0ab154` | 2026-02-26 | 33 | [Link](https://github.com/gottfrois/image_info) |
| libs/Windows | `libs/Windows` | build-binaries-2025-08-30 | `5e6884e` | 2026-02-26 | 5 | [Link](https://github.com/ImageMagick/Windows) |
| libs/Dependencies | `libs/Dependencies` | 2026.03.22.1904 | `df235ed` | 2026-04-01 | 179 | [Link](https://github.com/ImageMagick/Dependencies) |
| libs/ImageMagick | `libs/ImageMagick` | 7.1.2-18 | `b5f32dfb4` | 2026-04-01 | 23874 | [Link](https://github.com/ImageMagick/ImageMagick) |
| libs/gdk-pixbuf | `libs/gdk-pixbuf` | - | `f81fccb` | 2026-02-26 | 99 | [Link](https://github.com/ImageMagick/gdk-pixbuf) |
| libs/Imath | `libs/Imath` | v3.2.0 | `cd54202` | 2026-04-01 | 711 | [Link](https://github.com/AcademySoftwareFoundation/Imath) |
| libs/openexr | `libs/openexr` | v3.4-alpha | `36f035a1` | 2026-04-01 | 2897 | [Link](https://github.com/AcademySoftwareFoundation/openexr) |
| libs/OpenImageIO | `libs/OpenImageIO` | v3.2.0.0-dev | `f709b430d` | 2026-04-01 | 6516 | [Link](https://github.com/AcademySoftwareFoundation/OpenImageIO) |
| libs/OpenColorIO | `libs/OpenColorIO` | v2.1.0 | `8a3a37e2` | 2026-04-01 | 1597 | [Link](https://github.com/AcademySoftwareFoundation/OpenColorIO) |
| libs/OpenTimelineIO | `libs/OpenTimelineIO` | v0.18.1 | `a5558ef` | 2026-04-01 | 857 | [Link](https://github.com/AcademySoftwareFoundation/OpenTimelineIO) |
| libs/OpenCue | `libs/OpenCue` | v1.13.8 | `73a449c3` | 2026-04-01 | 1305 | [Link](https://github.com/AcademySoftwareFoundation/OpenCue) |
| libs/OpenRV | `libs/OpenRV` | v3.1.0 | `48bc17ca` | 2026-04-01 | 633 | [Link](https://github.com/AcademySoftwareFoundation/OpenRV) |
| libs/openfx | `libs/openfx` | OFX_Release_1.5.1 | `651b62c` | 2026-03-05 | 805 | [Link](https://github.com/AcademySoftwareFoundation/openfx) |
| libs/openapv | `libs/openapv` | v0.2.1.2 | `949ca81` | 2026-03-21 | 228 | [Link](https://github.com/AcademySoftwareFoundation/openapv) |
| libs/libxml2 | `libs/libxml2` | v2.15.0 | `89f45c54` | 2026-04-01 | 7781 | [Link](https://github.com/GNOME/libxml2) |
| libs/libxmlplusplus | `libs/libxmlplusplus` | 5.6.0 | `cd5169b` | 2026-02-26 | 565 | [Link](https://github.com/libxmlplusplus/libxmlplusplus) |
| libs/json-c | `libs/json-c` | - | `17a5409` | 2026-03-01 | 1426 | [Link](https://github.com/json-c/json-c) |
| libs/SysmonForLinux | `libs/SysmonForLinux` | 1.5.1.0 | `77c68da` | 2026-04-01 | 140 | [Link](https://github.com/microsoft/SysmonForLinux) |
| libs/fast-lzma2 | `libs/fast-lzma2` | v1.0.1 | `1878ae7` | 2026-03-01 | 248 | [Link](https://github.com/conor42/fast-lzma2) |
| libs/pxz | `libs/pxz` | - | `c692b16` | 2026-03-17 | 90 | [Link](https://github.com/jnovy/pxz) |
| libs/openh264 | `libs/openh264` | v2.6.0 | `5d98ed16` | 2026-03-01 | 5267 | [Link](https://github.com/cisco/openh264) |
| libs/libffi | `libs/libffi` | v3.5.2 | `1ce028a` | 2026-04-01 | 1931 | [Link](https://github.com/libffi/libffi) |
| libs/dragonffi | `libs/dragonffi` | dffi-0.9.4 | `26611fc` | 2026-03-01 | 169 | [Link](https://github.com/aguinet/dragonffi) |
| libs/WizardsToolkit | `libs/WizardsToolkit` | - | `abe6322` | 2026-03-01 | 1001 | [Link](https://github.com/ImageMagick/WizardsToolkit) |
| libs/openjpeg | `libs/openjpeg` | v2.5.4 | `3865c45e` | 2026-03-01 | 3155 | [Link](https://github.com/uclouvain/openjpeg) |
| libs/libzip | `libs/libzip` | v1.11.4 | `3ffd5b0b` | 2026-03-21 | 3686 | [Link](https://github.com/nih-at/libzip) |
| libs/libiconv | `libs/libiconv` | libiconv-1.17-1 | `5cf0a97` | 2026-03-01 | 47 | [Link](https://github.com/winlibs/libiconv) |
| libs/ckmame | `libs/ckmame` | v2.1 | `cf1cfd36` | 2026-03-01 | 3214 | [Link](https://github.com/nih-at/ckmame) |
| libs/nihtest | `libs/nihtest` | v1.10.1 | `414a37f` | 2026-03-01 | 152 | [Link](https://github.com/nih-at/nihtest) |
| libs/ziptools | `libs/ziptools` | - | `0fb7fa5` | 2026-03-01 | 16 | [Link](https://github.com/nih-at/ziptools) |
| libs/nihtest-cpp | `libs/nihtest-cpp` | - | `de5530e` | 2026-03-01 | 208 | [Link](https://github.com/nih-at/nihtest-cpp) |
| libs/openssl | `libs/openssl` | openssl-1.1.1t | `be4b63d4` | 2026-03-01 | 50 | [Link](https://github.com/winlibs/openssl) |
| libs/sqlite3 | `libs/sqlite3` | sqlite3-3.51.3 | `ad69ad9` | 2026-04-01 | 29 | [Link](https://github.com/winlibs/sqlite3) |
| libs/postgresql | `libs/postgresql` | libpq-16.12 | `9d63b2e1` | 2026-03-01 | 61 | [Link](https://github.com/winlibs/postgresql) |
| libs/cURL | `libs/cURL` | libcurl-8.19.0-1 | `fb579fa30` | 2026-04-01 | 119 | [Link](https://github.com/winlibs/cURL) |
| libs/libtidy | `libs/libtidy` | libtidy-5.6.0-1 | `93bb0ff` | 2026-03-01 | 17 | [Link](https://github.com/winlibs/libtidy) |
| libs/libjpeg | `libs/libjpeg` | libjpeg-turbo-3.0.3 | `7e5c924` | 2026-03-02 | 23 | [Link](https://github.com/winlibs/libjpeg) |
| libs/glib | `libs/glib` | glib-2.53.3-1 | `6196ac0` | 2026-03-01 | 24 | [Link](https://github.com/winlibs/glib) |
| libs/enchant | `libs/enchant` | enchant-2.2.8 | `b0458c2` | 2026-02-26 | 19 | [Link](https://github.com/winlibs/enchant) |
| libs/net-snmp | `libs/net-snmp` | net-snmp-5.9.4 | `a6038c1` | 2026-03-01 | 21 | [Link](https://github.com/winlibs/net-snmp) |
| libs/openldap | `libs/openldap` | openldap-2.4.47-1 | `d42ab66` | 2026-03-01 | 47 | [Link](https://github.com/winlibs/openldap) |
| libs/nghttp2 | `libs/nghttp2` | nghttp2-1.68.1 | `1f54601` | 2026-04-01 | 29 | [Link](https://github.com/winlibs/nghttp2) |
| libs/libxpm | `libs/libxpm` | libxpm-3.5.18 | `09090ab` | 2026-04-01 | 23 | [Link](https://github.com/winlibs/libxpm) |
| libs/cyrus-sasl | `libs/cyrus-sasl` | libsasl-2.1.27-3 | `a57e161` | 2026-02-26 | 11 | [Link](https://github.com/winlibs/cyrus-sasl) |
| libs/icu4c | `libs/icu4c` | icu4c-78.2 | `2cd68e36` | 2026-03-01 | 44 | [Link](https://github.com/winlibs/icu4c) |
| libs/mpir | `libs/mpir` | mpir-3.0.0-1 | `df470da` | 2026-02-26 | 20 | [Link](https://github.com/winlibs/mpir) |
| libs/gettext | `libs/gettext` | libintl-0.18.3-9 | `68e963a` | 2026-03-01 | 20 | [Link](https://github.com/winlibs/gettext) |
| libs/libxslt | `libs/libxslt` | libxslt-1.1.39 | `0156a5f` | 2026-03-01 | 33 | [Link](https://github.com/winlibs/libxslt) |
| libs/libwebp | `libs/libwebp` | libwebp-1.6.0 | `ac9c32e` | 2026-04-01 | 19 | [Link](https://github.com/winlibs/libwebp) |
| libs/freetype | `libs/freetype` | freetype-2.14.3 | `90983cd` | 2026-04-01 | 34 | [Link](https://github.com/winlibs/freetype) |
| libs/libavif | `libs/libavif` | libavif-1.4.1 | `b74ff95` | 2026-04-01 | 15 | [Link](https://github.com/winlibs/libavif) |
| libs/libssh2 | `libs/libssh2` | libssh2-1.11.1 | `3d45050` | 2026-03-01 | 35 | [Link](https://github.com/winlibs/libssh2) |
| libs/libpng | `libs/libpng` | libpng-1.6.56 | `93126fc` | 2026-04-01 | 33 | [Link](https://github.com/winlibs/libpng) |
| libs/libsodium | `libs/libsodium` | libsodium-1.0.21 | `7e572a6` | 2026-02-26 | 14 | [Link](https://github.com/winlibs/libsodium) |
| libs/qdbm | `libs/qdbm` | libqdbm-1.8.78 | `1a556e3` | 2026-03-01 | 5 | [Link](https://github.com/winlibs/qdbm) |
| libs/oniguruma | `libs/oniguruma` | libonig-6.9.9 | `63bbbac` | 2026-03-01 | 14 | [Link](https://github.com/winlibs/oniguruma) |
| libs/argon2 | `libs/argon2` | libargon2-20190702 | `1ce5c81` | 2026-03-01 | 12 | [Link](https://github.com/winlibs/argon2) |
| libs/lmdb | `libs/lmdb` | liblmdb-0.9.33 | `6354a6a` | 2026-03-01 | 11 | [Link](https://github.com/winlibs/lmdb) |
| libs/libbzip2 | `libs/libbzip2` | libbzip2-1.0.8-1 | `50fc1d6` | 2026-03-01 | 11 | [Link](https://github.com/winlibs/libbzip2) |
| libs/zlib | `libs/zlib` | zlib-1.3.2 | `3ffe779` | 2026-03-05 | 27 | [Link](https://github.com/winlibs/zlib) |
| libs/ssdeep | `libs/ssdeep` | ssdeep-2.13 | `38274ba` | 2026-02-26 | 8 | [Link](https://github.com/winlibs/ssdeep) |
| libs/wineditline | `libs/wineditline` | WinEditLine-2.206 | `67d1e09` | 2026-03-01 | 21 | [Link](https://github.com/winlibs/wineditline) |
| libs/pthreads | `libs/pthreads` | pthreads-3.0.0 | `3015396` | 2026-03-01 | 5 | [Link](https://github.com/winlibs/pthreads) |
| libs/imap | `libs/imap` | imap-2007f-1 | `8b21912` | 2026-03-01 | 7 | [Link](https://github.com/winlibs/imap) |
| libs/pslib | `libs/pslib` | pslib-0.4.6 | `34ce98a` | 2026-03-01 | 4 | [Link](https://github.com/winlibs/pslib) |
| libs/libmcrypt | `libs/libmcrypt` | libmcrypt-2.5.8-3.4 | `c14d309` | 2026-03-01 | 7 | [Link](https://github.com/winlibs/libmcrypt) |
| libs/dirent | `libs/dirent` | dirent-20080629 | `1d454a8` | 2026-03-01 | 9 | [Link](https://github.com/winlibs/dirent) |
| libs/c-ares | `libs/c-ares` | - | `6298122` | 2026-02-26 | 3 | [Link](https://github.com/winlibs/c-ares) |
| libs/httpd | `libs/httpd` | - | `4c306aef39` | 2026-04-01 | 34338 | [Link](https://github.com/apache/httpd) |
| libs/fribidi | `libs/fribidi` | - | `09fa0da` | 2026-02-26 | 4 | [Link](https://github.com/winlibs/fribidi) |
| libs/libvpx | `libs/libvpx` | libvpx-1.1.0 | `33f47c8` | 2026-03-01 | 5 | [Link](https://github.com/winlibs/libvpx) |
| libs/libvbucket | `libs/libvbucket` | 2.1.1r | `8d15749` | 2026-03-01 | 129 | [Link](https://github.com/membase/libvbucket) |
| libs/FFmpeg | `libs/FFmpeg` | n8.2-dev | `81f74fcacf` | 2026-04-01 | 123815 | [Link](https://github.com/FFmpeg/FFmpeg) |
| libs/hashcat | `libs/hashcat` | v7.1.2 | `997d0524a` | 2026-03-02 | 11059 | [Link](https://github.com/hashcat/hashcat) |
| libs/radare2 | `libs/radare2` | 6.1.2 | `ad3cafe2ef` | 2026-04-01 | 35268 | [Link](https://github.com/radareorg/radare2) |
| libs/C | `libs/C` | - | `72a019b1` | 2026-03-02 | 1540 | [Link](https://github.com/TheAlgorithms/C) |
| libs/vlc | `libs/vlc` | 4.0.0-dev | `896e658473` | 2026-04-01 | 109703 | [Link](https://github.com/videolan/vlc) |
| libs/sumatrapdf | `libs/sumatrapdf` | 3.2 | `97c1d9141` | 2026-03-11 | 16597 | [Link](https://github.com/sumatrapdfreader/sumatrapdf) |
| libs/mpv | `libs/mpv` | v0.41.0 | `de84642ae4` | 2026-04-01 | 54877 | [Link](https://github.com/mpv-player/mpv) |
| libs/sqlite | `libs/sqlite` | version-3.52.0 | `e4b9a19b95` | 2026-04-01 | 31596 | [Link](https://github.com/sqlite/sqlite) |
| libs/tinyxml2 | `libs/tinyxml2` | 11.0.0 | `f41a497` | 2026-03-11 | 1286 | [Link](https://github.com/leethomason/tinyxml2) |
| libs/libvips | `libs/libvips` | v8.18.1 | `cb3a9f108` | 2026-04-01 | 9255 | [Link](https://github.com/libvips/libvips) |
| libs/opencv | `libs/opencv` | 4.13.0 | `ab5eea4304` | 2026-04-01 | 36080 | [Link](https://github.com/opencv/opencv) |
| libs/MediaInfoLib | `libs/MediaInfoLib` | v26.01 | `87b804751` | 2026-03-17 | 9786 | [Link](https://github.com/MediaArea/MediaInfoLib) |
| libs/MediaInfo | `libs/MediaInfo` | v26.01 | `aa7008cb3` | 2026-04-01 | 2800 | [Link](https://github.com/MediaArea/MediaInfo) |
| libs/mm_file | `libs/mm_file` | v1.0.0 | `e57b1b0` | 2026-03-01 | 18 | [Link](https://github.com/jermp/mm_file) |
| libs/TinyEXIF | `libs/TinyEXIF` | 1.0.4 | `46bc5a0` | 2026-03-19 | 44 | [Link](https://github.com/cdcseacave/TinyEXIF) |
| libs/libjpeg-turbo | `libs/libjpeg-turbo` | 3.1.3 | `647dd329` | 2026-03-21 | 2863 | [Link](https://github.com/libjpeg-turbo/libjpeg-turbo) |
| libs/jhead | `libs/jhead` | 3.08 | `44f1f23` | 2026-04-01 | 549 | [Link](https://github.com/Matthias-Wandel/jhead) |
| libs/libexif | `libs/libexif` | libexif-0_6_25-release | `d8de7d4` | 2026-04-01 | 1507 | [Link](https://github.com/libexif/libexif) |
| libs/getopt-win32 | `libs/getopt-win32` | - | `bc89dc4` | 2026-02-26 | 5 | [Link](https://github.com/nanoporetech/getopt-win32) |
| libs/raylib | `libs/raylib` | 5.5 | `1df2b2206` | 2026-04-01 | 9721 | [Link](https://github.com/raysan5/raylib) |
| libs/lvgl | `libs/lvgl` | v9.5.0 | `9f91b9fd9` | 2026-04-01 | 12433 | [Link](https://github.com/lvgl/lvgl) |
| libs/getopt-win | `libs/getopt-win` | v2.42.0 | `ac1b9ea` | 2026-03-01 | 36 | [Link](https://github.com/ludvikjerabek/getopt-win) |
| libs/libunistd | `libs/libunistd` | v1.4 | `bd579ab` | 2026-03-01 | 554 | [Link](https://github.com/robinrowe/libunistd) |
| libs/ShazamAPI | `libs/ShazamAPI` | - | `096d5a2` | 2026-03-01 | 33 | [Link](https://github.com/Numenorean/ShazamAPI) |
| libs/seek-tune | `libs/seek-tune` | - | `5374d39` | 2026-03-01 | 340 | [Link](https://github.com/cgzirim/seek-tune) |
| libs/ShazamIO | `libs/ShazamIO` | 0.8.1 | `11739c8` | 2026-03-01 | 318 | [Link](https://github.com/shazamio/ShazamIO) |
| libs/audio-recognizer | `libs/audio-recognizer` | - | `3dcaaee` | 2026-02-26 | 12 | [Link](https://github.com/wsieroci/audio-recognizer) |
| libs/pHash | `libs/pHash` | - | `2f69861` | 2026-02-26 | 57 | [Link](https://github.com/nihas101/pHash) |
| libs/image-hash | `libs/image-hash` | - | `d426783` | 2026-03-01 | 215 | [Link](https://github.com/ytetsuro/image-hash) |
| libs/imghash-viewer | `libs/imghash-viewer` | v0.1.0 | `1154eb2` | 2026-03-01 | 31 | [Link](https://github.com/YannickAlex07/imghash-viewer) |
| libs/imagehash | `libs/imagehash` | v1.0.7 | `5063b18` | 2026-03-01 | 46 | [Link](https://github.com/vitali-fedulov/imagehash) |
| libs/securecopy | `libs/securecopy` | v9.7.0 | `d4cb8e4` | 2026-02-26 | 30 | [Link](https://github.com/hansij66/securecopy) |
| libs/xattrs | `libs/xattrs` | - | `b60f534` | 2026-02-26 | 46 | [Link](https://github.com/hrbrmstr/xattrs) |
| libs/fuse_xattrs | `libs/fuse_xattrs` | v0.3 | `acf60d4` | 2026-03-01 | 39 | [Link](https://github.com/fbarriga/fuse_xattrs) |
| libs/xattrlib | `libs/xattrlib` | - | `85522f2` | 2026-02-26 | 7 | [Link](https://github.com/amdf/xattrlib) |
| libs/RenStrm | `libs/RenStrm` | 1.0 | `f195bb9` | 2026-02-26 | 9 | [Link](https://github.com/hernandp/RenStrm) |
| libs/metastore | `libs/metastore` | v1.1.2 | `9b78b5d` | 2026-03-01 | 177 | [Link](https://github.com/przemoc/metastore) |
| libs/lsads | `libs/lsads` | 2013-12-29_release | `b349877` | 2026-02-26 | 31 | [Link](https://github.com/assarbad/lsads) |
| libs/Powershell-ADS | `libs/Powershell-ADS` | - | `13a7507` | 2026-02-26 | 5 | [Link](https://github.com/phlashko/Powershell-ADS) |
| libs/SharpADS | `libs/SharpADS` | v1.0 | `8802fc6` | 2026-02-26 | 14 | [Link](https://github.com/ricardojoserf/SharpADS) |
| libs/ADSIdentifier | `libs/ADSIdentifier` | - | `7074e07` | 2026-02-26 | 24 | [Link](https://github.com/HannahVernon/ADSIdentifier) |
| libs/ads | `libs/ads` | - | `812a05d` | 2026-03-01 | 6 | [Link](https://github.com/katahiromz/ads) |
| libs/AlternateDataStreams | `libs/AlternateDataStreams` | - | `9eb3f30` | 2026-02-26 | 9 | [Link](https://github.com/narasimha14/AlternateDataStreams) |
| libs/DataStreamBrowser | `libs/DataStreamBrowser` | - | `70235cc` | 2026-03-01 | 4 | [Link](https://github.com/Thibstars/DataStreamBrowser) |
| libs/libevent | `libs/libevent` | release-2.2.1-alpha | `a73b4ea3` | 2026-04-01 | 5180 | [Link](https://github.com/libevent/libevent) |
| libs/xxHash | `libs/xxHash` | v0.7.4 | `0b8f6b3` | 2026-03-01 | 2072 | [Link](https://github.com/Cyan4973/xxHash) |
| libs/libgit2 | `libs/libgit2` | v1.9.0 | `6845bded0` | 2026-04-01 | 16148 | [Link](https://github.com/libgit2/libgit2) |
| libs/dokany | `libs/dokany` | v2.3.1.1000 | `247fa21` | 2026-03-23 | 1768 | [Link](https://github.com/dokan-dev/dokany) |
| libs/pngquant | `libs/pngquant` | 3.0.3 | `e675213` | 2026-04-01 | 1216 | [Link](https://github.com/kornelski/pngquant) |
| libs/dunst | `libs/dunst` | v1.13.2 | `fc2653a` | 2026-04-01 | 2637 | [Link](https://github.com/dunst-project/dunst) |
| libs/audiocraft | `libs/audiocraft` | v1.3.0 | `c21ee02` | 2026-03-01 | 256 | [Link](https://github.com/facebookresearch/audiocraft) |
| libs/hashingImage | `libs/hashingImage` | - | `ac344e2` | 2026-03-01 | 5 | [Link](https://github.com/goongong/hashingImage) |
| libs/util-linux | `libs/util-linux` | v2.43-devel | `c5762dce8` | 2026-04-01 | 22111 | [Link](https://github.com/util-linux/util-linux) |
| libs/libimghash | `libs/libimghash` | - | `ff704c7` | 2026-03-01 | 43 | [Link](https://github.com/shuttie/libimghash) |
| libs/perceptual-dct-hash | `libs/perceptual-dct-hash` | - | `c533281` | 2026-03-01 | 31 | [Link](https://github.com/alangshur/perceptual-dct-hash) |
| libs/pHash.c | `libs/pHash.c` | v1.1.0 | `801f0f2` | 2026-03-02 | 35 | [Link](https://github.com/folksable/pHash.c) |
| libs/tinyphash | `libs/tinyphash` | - | `ccab513` | 2026-02-26 | 16 | [Link](https://github.com/Wassasin/tinyphash) |
| libs/ADSman | `libs/ADSman` | - | `3ab428a` | 2026-03-01 | 5 | [Link](https://github.com/8gudbits/ADSman) |
| libs/ADSFileSystem | `libs/ADSFileSystem` | - | `5ab73c6` | 2026-02-26 | 11 | [Link](https://github.com/marcelogm/ADSFileSystem) |
| libs/Bringing-Old-Photos-Back-to-Life | `libs/Bringing-Old-Photos-Back-to-Life` | v1.0 | `2093171` | 2026-02-26 | 110 | [Link](https://github.com/microsoft/Bringing-Old-Photos-Back-to-Life) |
| ai-file-sorter | `ai-file-sorter` | v1.6.0 | `3fec960` | 2026-04-01 | 588 | [Link](https://github.com/hyperfield/ai-file-sorter) |
| VERT | `VERT` | - | `990f3f5` | 2026-04-01 | 746 | [Link](https://github.com/VERT-sh/VERT) |
| libs/BLAKE3 | `libs/BLAKE3` | 1.8.4 | `175f664` | 2026-04-01 | 775 | [Link](https://github.com/BLAKE3-team/BLAKE3.git) |
| libs/bobui | `libs/bobui` | - | `06af949231` | 2026-03-01 | 77649 | [Link](https://github.com/robertpelloni/bobui) |
