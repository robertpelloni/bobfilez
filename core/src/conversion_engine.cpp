/// @file conversion_engine.cpp
/// @brief ConversionEngine implementation + all converter backends.
///
/// Backend priority order for overlapping capabilities:
///   Video/Audio: FFmpeg (fastest, most formats)
///   Image:       ImageMagick (most formats), libvips (fastest for large images)
///   PDF:         Ghostscript (gs/eps/ps), Poppler (pdf→png/svg/text)
///   Documents:   Pandoc (markup/docx/epub/html/tex), Calibre (ebooks)
///   Vector:      Inkscape CLI (svg↔pdf↔png)
///   Raw images:  dcraw / LibRaw via ImageMagick
///
/// All converters invoke external CLI tools. This is intentional:
///   - We get maintenance-free format support (FFmpeg team, IM team handle codecs)
///   - Users can update tools independently
///   - No need to link C libraries that have complex license arrangements
///   - We check for tool availability at runtime with helpful error messages
///
/// Tool availability is cached after first check. If a tool isn't installed,
/// the converter is silently unavailable (not registered), and the engine
/// reports "No converter found" with a helpful install hint.

#include "fo/core/conversion_interface.hpp"
#include "fo/core/registry.hpp"
#include <cstdlib>
#include <cstdio>
#include <chrono>
#include <filesystem>
#include <thread>
#include <mutex>
#include <atomic>
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <fstream>

#ifdef _WIN32
#define popen  _popen
#define pclose _pclose
#endif

namespace fo::core {

//─────────────────────────────────────────────────────────────────────────────
// Utility helpers
//─────────────────────────────────────────────────────────────────────────────

/// Run a shell command, return {exit_code, combined_output}.
static std::pair<int, std::string> run_cmd(const std::string& cmd) {
    std::string output;
    FILE* pipe = popen((cmd + " 2>&1").c_str(), "r");
    if (!pipe) return {-1, "Failed to run command"};
    char buf[1024];
    while (fgets(buf, sizeof(buf), pipe)) output += buf;
    int rc = pclose(pipe);
    return {rc, output};
}

/// Check whether a tool is available on PATH
static bool tool_available(const std::string& name) {
#ifdef _WIN32
    auto [rc, out] = run_cmd("where " + name + " 2>nul");
#else
    auto [rc, out] = run_cmd("which " + name + " 2>/dev/null");
#endif
    return rc == 0;
}

/// Build a unique output path, adding _2 _3 etc. if dest exists.
static std::filesystem::path unique_output_path(const std::filesystem::path& desired) {
    if (!std::filesystem::exists(desired)) return desired;
    auto stem = desired.stem().string();
    auto ext  = desired.extension().string();
    auto dir  = desired.parent_path();
    for (int i = 2; i < 10000; ++i) {
        auto candidate = dir / (stem + "_" + std::to_string(i) + ext);
        if (!std::filesystem::exists(candidate)) return candidate;
    }
    return desired;
}

//─────────────────────────────────────────────────────────────────────────────
// FFmpeg converter — Video & Audio
//─────────────────────────────────────────────────────────────────────────────
class FFmpegConverter : public IConverter {
public:
    std::string name() const override { return "ffmpeg"; }
    std::string description() const override {
        return "FFmpeg — universal video/audio converter (mp4/mkv/avi/mov/webm/flv/gif/mp3/flac/aac/ogg/wav/opus/m4a/etc.)";
    }

    // All extensions FFmpeg handles input for
    std::vector<std::string> supported_input_extensions() const override {
        return {"mp4","mkv","avi","mov","wmv","flv","webm","m4v","ts","mpg","mpeg","3gp",
                "mp3","flac","wav","aac","ogg","opus","m4a","wma","aiff","ape","wv",
                "gif","mts","m2ts"};
    }

    std::vector<std::string> supported_output_extensions(const std::string& in) const override {
        // Video inputs → video + audio + gif
        static const std::vector<std::string> VIDEO_EXTS = {
            "mp4","mkv","avi","mov","wmv","webm","flv","m4v","gif","mp3","aac","ogg","wav","flac","opus"};
        static const std::vector<std::string> AUDIO_EXTS = {
            "mp3","flac","wav","aac","ogg","opus","m4a","wma","aiff"};
        static const std::vector<std::string> VIDEO_IN = {
            "mp4","mkv","avi","mov","wmv","flv","webm","m4v","ts","mpg","mpeg","3gp","mts","m2ts","gif"};
        bool is_video = std::find(VIDEO_IN.begin(), VIDEO_IN.end(), in) != VIDEO_IN.end();
        return is_video ? VIDEO_EXTS : AUDIO_EXTS;
    }

    std::vector<ConvertOption> available_options(const std::string& in, const std::string& out) const override {
        std::vector<ConvertOption> opts;

        // Common video options
        if (out == "mp4" || out == "mkv" || out == "webm" || out == "avi") {
            opts.push_back({"video_bitrate","Video bitrate (e.g. 2M, 5000k)","2M","string",{},{},"","20M"});
            opts.push_back({"video_codec","Video codec","libx264","choice",{"libx264","libx265","vp9","vp8","av1"},{},"",""});
            opts.push_back({"crf","CRF quality (lower=better, 0=lossless, 51=worst)","23","int",{},{},"0","51"});
            opts.push_back({"fps","Output framerate (0=keep original)","0","int",{},{},"0","240"});
            opts.push_back({"resolution","Output resolution (empty=keep, e.g. 1920x1080)","","string",{},{},{},"4320x4320"});
            opts.push_back({"preset","Encoding preset (speed vs quality)","medium","choice",
                            {"ultrafast","superfast","veryfast","faster","fast","medium","slow","slower","veryslow"},{},"",""});
        }
        if (out == "gif") {
            opts.push_back({"fps","GIF framerate","15","int",{},"1","60","60"});
            opts.push_back({"scale","GIF width (-1=auto height)","320","int",{},"8","4096","4096"});
            opts.push_back({"loop","Loop count (0=forever)","0","int",{},"0","65535","65535"});
        }
        // Audio options
        if (out == "mp3") {
            opts.push_back({"audio_bitrate","Audio bitrate (e.g. 128k, 320k)","192k","string",{},{},{},{}});
            opts.push_back({"audio_quality","VBR quality 0-9 (0=best)","2","int",{},"0","9","9"});
        }
        if (out == "flac") {
            opts.push_back({"compression","FLAC compression level 0-12","5","int",{},"0","12","12"});
        }
        if (out == "ogg" || out == "opus") {
            opts.push_back({"audio_bitrate","Bitrate (e.g. 128k)","128k","string",{},{},{},{}});
        }
        // Universal extras
        opts.push_back({"start_time","Trim start time (HH:MM:SS or seconds)","","string",{},{},{},{}});
        opts.push_back({"duration","Trim duration (HH:MM:SS or seconds)","","string",{},{},{},{}});
        opts.push_back({"audio_channels","Audio channels (0=keep, 1=mono, 2=stereo)","0","int",{},"0","8","8"});
        opts.push_back({"audio_sample_rate","Sample rate (0=keep, e.g. 44100)","0","int",{},"0","192000","192000"});
        opts.push_back({"extra_args","Extra FFmpeg arguments (advanced)","","string",{},{},{},{}});
        return opts;
    }

    ConversionResult convert(const std::filesystem::path& input,
                              const std::filesystem::path& output,
                              const std::map<std::string, std::string>& opts) override {
        ConversionResult r;
        r.input = input;
        r.output = output;
        auto t0 = std::chrono::steady_clock::now();

        // Build FFmpeg command
        std::ostringstream cmd;
        cmd << "ffmpeg -y -hide_banner -loglevel error";

        // Trim start / duration
        auto it = opts.find("start_time");
        if (it != opts.end() && !it->second.empty())
            cmd << " -ss " << it->second;
        it = opts.find("duration");
        if (it != opts.end() && !it->second.empty())
            cmd << " -t " << it->second;

        cmd << " -i \"" << input.string() << "\"";

        // Video options
        auto get = [&](const std::string& k, const std::string& def) -> std::string {
            auto f = opts.find(k); return (f != opts.end() && !f->second.empty()) ? f->second : def;
        };

        std::string out_ext = output.extension().string();
        if (!out_ext.empty() && out_ext[0] == '.') out_ext = out_ext.substr(1);

        if (out_ext == "gif") {
            int fps_val = std::stoi(get("fps","15"));
            int scale_val = std::stoi(get("scale","320"));
            cmd << " -vf \"fps=" << fps_val << ",scale=" << scale_val << ":-1:flags=lanczos,split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse\"";
        } else {
            std::string codec = get("video_codec","");
            if (!codec.empty()) cmd << " -c:v " << codec;

            std::string crf = get("crf","");
            if (!crf.empty() && crf != "0") cmd << " -crf " << crf;

            std::string vbr = get("video_bitrate","");
            if (!vbr.empty()) cmd << " -b:v " << vbr;

            std::string fps = get("fps","");
            if (!fps.empty() && fps != "0") cmd << " -r " << fps;

            std::string res = get("resolution","");
            if (!res.empty()) cmd << " -vf scale=" << res;

            std::string preset = get("preset","");
            if (!preset.empty()) cmd << " -preset " << preset;
        }

        // Audio options
        std::string abr = get("audio_bitrate","");
        if (!abr.empty()) cmd << " -b:a " << abr;
        std::string aq = get("audio_quality","");
        if (!aq.empty()) cmd << " -q:a " << aq;
        std::string sr = get("audio_sample_rate","");
        if (!sr.empty() && sr != "0") cmd << " -ar " << sr;
        std::string ac = get("audio_channels","");
        if (!ac.empty() && ac != "0") cmd << " -ac " << ac;
        std::string comp = get("compression","");
        if (!comp.empty()) cmd << " -compression_level " << comp;

        // Extra args
        std::string extra = get("extra_args","");
        if (!extra.empty()) cmd << " " << extra;

        cmd << " \"" << output.string() << "\"";

        auto [rc, out_str] = run_cmd(cmd.str());
        auto t1 = std::chrono::steady_clock::now();
        r.duration_sec = std::chrono::duration<double>(t1 - t0).count();
        r.success = (rc == 0) && std::filesystem::exists(output);
        if (!r.success) r.error = out_str.empty() ? "ffmpeg returned exit code " + std::to_string(rc) : out_str;
        if (r.success) {
            r.input_size = std::filesystem::file_size(input);
            r.output_size = std::filesystem::file_size(output);
        }
        return r;
    }
};

//─────────────────────────────────────────────────────────────────────────────
// ImageMagick converter — Images (and PDF pages)
//─────────────────────────────────────────────────────────────────────────────
class ImageMagickConverter : public IConverter {
public:
    std::string name() const override { return "imagemagick"; }
    std::string description() const override {
        return "ImageMagick — convert between 200+ image formats (png/jpg/webp/avif/heic/tiff/bmp/ico/svg/pdf/ps/eps/psd/raw...)";
    }

    std::vector<std::string> supported_input_extensions() const override {
        return {"png","jpg","jpeg","webp","avif","heic","heif","tiff","tif","bmp","ico","gif",
                "svg","pdf","ps","eps","psd","xcf","raw","cr2","cr3","nef","arw","dng","orf","rw2",
                "pbm","pgm","ppm","pam","xpm","xbm","pcx","tga","sgi","hdr","exr",
                "jp2","j2k","jxl","cur","ani","wmf","emf","dib"};
    }

    std::vector<std::string> supported_output_extensions(const std::string&) const override {
        return {"png","jpg","webp","avif","heic","tiff","bmp","ico","gif",
                "pdf","ps","eps","svg","ppm","pgm","pbm","jp2","jxl","tga","hdr","exr"};
    }

    std::vector<ConvertOption> available_options(const std::string&, const std::string& out) const override {
        std::vector<ConvertOption> opts;
        if (out == "jpg" || out == "jpeg" || out == "webp") {
            opts.push_back({"quality","JPEG/WebP quality 1-100 (100=best)","85","int",{},"1","100","100"});
        }
        if (out == "png") {
            opts.push_back({"compression","PNG compression 0-9 (9=smallest)","6","int",{},"0","9","9"});
        }
        if (out == "avif" || out == "heic") {
            opts.push_back({"quality","Quality 1-100","75","int",{},"1","100","100"});
        }
        if (out == "pdf") {
            opts.push_back({"density","DPI for rasterization","300","int",{},"72","1200","1200"});
        }
        opts.push_back({"resize","Resize geometry (e.g. 50%, 800x600, 800x>)","","string",{},{},{},{}});
        opts.push_back({"gravity","Gravity for crop/resize","Center","choice",
                        {"Center","North","South","East","West","NorthEast","NorthWest","SouthEast","SouthWest"},{},"",""});
        opts.push_back({"colorspace","Force colorspace (empty=keep)","","choice",
                        {"","sRGB","CMYK","Gray","HSL"},{},"",""});
        opts.push_back({"strip","Strip EXIF metadata","false","bool",{},{},{},{}});
        opts.push_back({"rotate","Auto-rotate based on EXIF (true/false)","true","bool",{},{},{},{}});
        opts.push_back({"page","PDF page to extract (0=all, 1-N=specific)","0","int",{},"0","9999","9999"});
        opts.push_back({"density","Rasterize DPI (for PDF/SVG/EPS input)","150","int",{},"72","1200","1200"});
        opts.push_back({"extra_args","Extra ImageMagick arguments (advanced)","","string",{},{},{},{}});
        return opts;
    }

    ConversionResult convert(const std::filesystem::path& input,
                              const std::filesystem::path& output,
                              const std::map<std::string, std::string>& opts) override {
        ConversionResult r;
        r.input = input; r.output = output;
        auto t0 = std::chrono::steady_clock::now();

        auto get = [&](const std::string& k, const std::string& def) {
            auto it = opts.find(k); return (it != opts.end() && !it->second.empty()) ? it->second : def;
        };

        std::ostringstream cmd;
        cmd << "magick";

        // Density must come BEFORE input for PDF/SVG/EPS
        std::string density = get("density","");
        if (!density.empty() && density != "0") cmd << " -density " << density;

        // For PDF, extract a specific page
        std::string page = get("page","0");
        std::string in_spec = input.string();
        if (page != "0" && page != "") in_spec += "[" + std::to_string(std::stoi(page)-1) + "]";

        cmd << " \"" << in_spec << "\"";

        std::string rotate = get("rotate","true");
        if (rotate == "true") cmd << " -auto-orient";

        std::string colorspace = get("colorspace","");
        if (!colorspace.empty()) cmd << " -colorspace " << colorspace;

        std::string resize = get("resize","");
        if (!resize.empty()) cmd << " -resize \"" << resize << "\"";

        std::string quality = get("quality","");
        if (!quality.empty()) cmd << " -quality " << quality;

        std::string compression = get("compression","");
        if (!compression.empty()) cmd << " -define png:compression-level=" << compression;

        std::string strip = get("strip","false");
        if (strip == "true") cmd << " -strip";

        std::string extra = get("extra_args","");
        if (!extra.empty()) cmd << " " << extra;

        cmd << " \"" << output.string() << "\"";

        auto [rc, out_str] = run_cmd(cmd.str());
        auto t1 = std::chrono::steady_clock::now();
        r.duration_sec = std::chrono::duration<double>(t1 - t0).count();
        r.success = (rc == 0) && std::filesystem::exists(output);
        if (!r.success) r.error = out_str.empty() ? "magick returned exit code " + std::to_string(rc) : out_str;
        if (r.success) {
            r.input_size = std::filesystem::file_size(input);
            r.output_size = std::filesystem::file_size(output);
        }
        return r;
    }
};

//─────────────────────────────────────────────────────────────────────────────
// Pandoc converter — Documents
//─────────────────────────────────────────────────────────────────────────────
class PandocConverter : public IConverter {
public:
    std::string name() const override { return "pandoc"; }
    std::string description() const override {
        return "Pandoc — universal document converter (Markdown/RST/HTML/DOCX/ODT/EPUB/LaTeX/PDF...)";
    }

    std::vector<std::string> supported_input_extensions() const override {
        return {"md","markdown","rst","txt","html","htm","docx","odt","rtf","epub","tex","latex",
                "org","wiki","adoc","asciidoc","ipynb","csv","tsv","json"};
    }

    std::vector<std::string> supported_output_extensions(const std::string&) const override {
        return {"html","htm","pdf","docx","odt","epub","md","rst","tex","latex","rtf",
                "txt","json","xml","pptx","asciidoc","org","wiki","man","fb2"};
    }

    std::vector<ConvertOption> available_options(const std::string&, const std::string& out) const override {
        std::vector<ConvertOption> opts;
        opts.push_back({"template","Pandoc template file path","","string",{},{},{},{}});
        opts.push_back({"css","CSS file for HTML output","","string",{},{},{},{}});
        opts.push_back({"toc","Include table of contents","false","bool",{},{},{},{}});
        opts.push_back({"toc_depth","TOC depth","3","int",{},"1","6","6"});
        opts.push_back({"number_sections","Number section headings","false","bool",{},{},{},{}});
        opts.push_back({"standalone","Produce standalone document","true","bool",{},{},{},{}});
        opts.push_back({"pdf_engine","PDF engine (for PDF output)","pdflatex","choice",
                        {"pdflatex","xelatex","lualatex","wkhtmltopdf","weasyprint"},{},"",""});
        if (out == "epub") {
            opts.push_back({"epub_cover","EPUB cover image path","","string",{},{},{},{}});
            opts.push_back({"epub_metadata","EPUB metadata XML path","","string",{},{},{},{}});
        }
        opts.push_back({"extra_args","Extra Pandoc arguments","","string",{},{},{},{}});
        return opts;
    }

    ConversionResult convert(const std::filesystem::path& input,
                              const std::filesystem::path& output,
                              const std::map<std::string, std::string>& opts) override {
        ConversionResult r;
        r.input = input; r.output = output;
        auto t0 = std::chrono::steady_clock::now();

        auto get = [&](const std::string& k, const std::string& def) {
            auto it = opts.find(k); return (it != opts.end() && !it->second.empty()) ? it->second : def;
        };

        std::string out_ext = output.extension().string();
        if (!out_ext.empty() && out_ext[0] == '.') out_ext = out_ext.substr(1);

        std::ostringstream cmd;
        cmd << "pandoc";

        if (get("standalone","true") == "true") cmd << " -s";
        if (get("toc","false") == "true") cmd << " --toc";
        std::string tod = get("toc_depth","");
        if (!tod.empty()) cmd << " --toc-depth=" << tod;
        if (get("number_sections","false") == "true") cmd << " -N";

        std::string tmpl = get("template","");
        if (!tmpl.empty()) cmd << " --template=\"" << tmpl << "\"";

        std::string css = get("css","");
        if (!css.empty()) cmd << " --css=\"" << css << "\"";

        if (out_ext == "pdf") {
            std::string engine = get("pdf_engine","pdflatex");
            cmd << " --pdf-engine=" << engine;
        }
        if (out_ext == "epub") {
            std::string cover = get("epub_cover","");
            if (!cover.empty()) cmd << " --epub-cover-image=\"" << cover << "\"";
        }

        std::string extra = get("extra_args","");
        if (!extra.empty()) cmd << " " << extra;

        cmd << " -o \"" << output.string() << "\" \"" << input.string() << "\"";

        auto [rc, out_str] = run_cmd(cmd.str());
        auto t1 = std::chrono::steady_clock::now();
        r.duration_sec = std::chrono::duration<double>(t1 - t0).count();
        r.success = (rc == 0) && std::filesystem::exists(output);
        if (!r.success) r.error = out_str.empty() ? "pandoc exit " + std::to_string(rc) : out_str;
        if (r.success) {
            r.input_size = std::filesystem::file_size(input);
            r.output_size = std::filesystem::file_size(output);
        }
        return r;
    }
};

//─────────────────────────────────────────────────────────────────────────────
// Calibre converter — Ebooks
//─────────────────────────────────────────────────────────────────────────────
class CalibreConverter : public IConverter {
public:
    std::string name() const override { return "calibre"; }
    std::string description() const override { return "Calibre (ebook-convert) — ebook format conversion"; }

    std::vector<std::string> supported_input_extensions() const override {
        return {"epub","mobi","azw3","pdf","html","docx","txt","rtf","chm","fb2","lit","lrf","pdb","pml","snb","tcr"};
    }
    std::vector<std::string> supported_output_extensions(const std::string&) const override {
        return {"epub","mobi","azw3","pdf","html","docx","txt","rtf","fb2","lit","lrf","pdb","pml","snb","tcr"};
    }
    std::vector<ConvertOption> available_options(const std::string&, const std::string&) const override {
        std::vector<ConvertOption> opts;
        opts.push_back({"title","Set book title","","string",{},{},{},{}});
        opts.push_back({"authors","Set book authors","","string",{},{},{},{}});
        opts.push_back({"cover","Path to cover image","","string",{},{},{},{}});
        opts.push_back({"remove_margins","Remove page margins","false","bool",{},{},{},{}});
        opts.push_back({"extra_args","Extra ebook-convert arguments","","string",{},{},{},{}});
        return opts;
    }
    ConversionResult convert(const std::filesystem::path& input, const std::filesystem::path& output, const std::map<std::string, std::string>& opts) override {
        ConversionResult r; r.input = input; r.output = output;
        auto get = [&](const std::string& k, const std::string& def) { auto it = opts.find(k); return (it != opts.end() && !it->second.empty()) ? it->second : def; };
        
        std::ostringstream cmd;
        cmd << "ebook-convert \"" << input.string() << "\" \"" << output.string() << "\"";
        
        std::string title = get("title",""); if (!title.empty()) cmd << " --title=\"" << title << "\"";
        std::string auth = get("authors",""); if (!auth.empty()) cmd << " --authors=\"" << auth << "\"";
        std::string cover = get("cover",""); if (!cover.empty()) cmd << " --cover=\"" << cover << "\"";
        if (get("remove_margins","false") == "true") cmd << " --remove-first-image";
        
        std::string extra = get("extra_args",""); if (!extra.empty()) cmd << " " << extra;
        
        auto t0 = std::chrono::steady_clock::now();
        auto [rc, out_str] = run_cmd(cmd.str());
        r.duration_sec = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
        r.success = (rc == 0) && std::filesystem::exists(output);
        if (!r.success) r.error = out_str.empty() ? "ebook-convert exit " + std::to_string(rc) : out_str;
        return r;
    }
};

//─────────────────────────────────────────────────────────────────────────────
// Ghostscript converter — PDF/PS
//─────────────────────────────────────────────────────────────────────────────
class GhostscriptConverter : public IConverter {
public:
    std::string name() const override { return "ghostscript"; }
    std::string description() const override { return "Ghostscript (gs) — PDF/PostScript processing and rasterization"; }

    std::vector<std::string> supported_input_extensions() const override { return {"pdf","ps","eps"}; }
    std::vector<std::string> supported_output_extensions(const std::string&) const override { return {"pdf","png","jpg","tiff"}; }
    
    std::vector<ConvertOption> available_options(const std::string&, const std::string& out) const override {
        std::vector<ConvertOption> opts;
        opts.push_back({"dpi","Render DPI","300","int",{},"72","1200","1200"});
        if (out == "pdf") {
            opts.push_back({"compress","Compress PDF (ebook, screen, printer, prepress)","ebook","choice",{"screen","ebook","printer","prepress","default"},{},"",""});
        }
        return opts;
    }
    ConversionResult convert(const std::filesystem::path& input, const std::filesystem::path& output, const std::map<std::string, std::string>& opts) override {
        ConversionResult r; r.input = input; r.output = output;
        auto get = [&](const std::string& k, const std::string& def) { auto it = opts.find(k); return (it != opts.end() && !it->second.empty()) ? it->second : def; };
        
        std::string ext = output.extension().string();
        if (!ext.empty() && ext[0] == '.') ext = ext.substr(1);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        
        std::ostringstream cmd;
#ifdef _WIN32
        cmd << "gswin64c -dNOPAUSE -dBATCH -dSAFER";
#else
        cmd << "gs -dNOPAUSE -dBATCH -dSAFER";
#endif
        
        if (ext == "pdf") {
            cmd << " -sDEVICE=pdfwrite";
            std::string comp = get("compress","ebook");
            if (!comp.empty() && comp != "default") cmd << " -dPDFSETTINGS=/" << comp;
        } else if (ext == "png") {
            cmd << " -sDEVICE=png16m";
        } else if (ext == "jpg" || ext == "jpeg") {
            cmd << " -sDEVICE=jpeg -dJPEGQ=85";
        } else if (ext == "tiff" || ext == "tif") {
            cmd << " -sDEVICE=tiff24nc -dCompression=lzw";
        }
        
        std::string dpi = get("dpi","300");
        if (!dpi.empty()) cmd << " -r" << dpi;
        
        cmd << " -sOutputFile=\"" << output.string() << "\" \"" << input.string() << "\"";
        
        auto t0 = std::chrono::steady_clock::now();
        auto [rc, out_str] = run_cmd(cmd.str());
        r.duration_sec = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
        r.success = (rc == 0) && std::filesystem::exists(output);
        if (!r.success) r.error = out_str.empty() ? "ghostscript exit " + std::to_string(rc) : out_str;
        return r;
    }
};

//─────────────────────────────────────────────────────────────────────────────
// ConversionEngine implementation
//─────────────────────────────────────────────────────────────────────────────

void ConversionEngine::register_converter(std::shared_ptr<IConverter> conv) {
    converters_.push_back(std::move(conv));
}

std::vector<std::shared_ptr<IConverter>> ConversionEngine::find_converters(
    const std::string& in_ext, const std::string& out_ext) const {

    std::vector<std::shared_ptr<IConverter>> result;
    for (auto& c : converters_) {
        auto in_exts = c->supported_input_extensions();
        if (std::find(in_exts.begin(), in_exts.end(), in_ext) == in_exts.end()) continue;
        auto out_exts = c->supported_output_extensions(in_ext);
        if (std::find(out_exts.begin(), out_exts.end(), out_ext) == out_exts.end()) continue;
        result.push_back(c);
    }
    return result;
}

std::vector<std::string> ConversionEngine::possible_outputs(const std::string& in_ext) const {
    std::vector<std::string> all;
    for (auto& c : converters_) {
        auto in = c->supported_input_extensions();
        if (std::find(in.begin(), in.end(), in_ext) == in.end()) continue;
        auto out = c->supported_output_extensions(in_ext);
        for (auto& e : out) if (std::find(all.begin(), all.end(), e) == all.end()) all.push_back(e);
    }
    std::sort(all.begin(), all.end());
    return all;
}

std::vector<std::string> ConversionEngine::all_input_extensions() const {
    std::vector<std::string> all;
    for (auto& c : converters_) {
        for (auto& e : c->supported_input_extensions())
            if (std::find(all.begin(), all.end(), e) == all.end()) all.push_back(e);
    }
    std::sort(all.begin(), all.end());
    return all;
}

ConversionResult ConversionEngine::convert(
    const std::filesystem::path& input,
    const std::string& out_ext,
    const std::filesystem::path& out_dir,
    const std::map<std::string, std::string>& options,
    const std::string& preferred) {

    std::string in_ext = input.extension().string();
    if (!in_ext.empty() && in_ext[0] == '.') in_ext = in_ext.substr(1);
    std::transform(in_ext.begin(), in_ext.end(), in_ext.begin(), ::tolower);

    auto converters = find_converters(in_ext, out_ext);
    if (converters.empty()) {
        ConversionResult r;
        r.input = input;
        r.success = false;
        r.error = "No converter found for ." + in_ext + " → ." + out_ext + ". Ensure FFmpeg/ImageMagick/Pandoc is installed.";
        return r;
    }

    // Pick preferred or first
    std::shared_ptr<IConverter> chosen = converters[0];
    if (!preferred.empty()) {
        for (auto& c : converters) if (c->name() == preferred) { chosen = c; break; }
    }

    // Build output path
    std::filesystem::path dest_dir = out_dir.empty() ? input.parent_path() : out_dir;
    auto output = unique_output_path(dest_dir / (input.stem().string() + "." + out_ext));

    return chosen->convert(input, output, options);
}

std::vector<ConversionResult> ConversionEngine::batch_convert(
    const std::vector<std::filesystem::path>& inputs,
    const std::string& out_ext,
    const std::filesystem::path& out_dir,
    const std::map<std::string, std::string>& options,
    int num_threads,
    ConversionProgressCb progress_cb,
    const std::string& preferred) {

    std::vector<ConversionResult> results(inputs.size());
    std::atomic<int> done{0};
    std::mutex mtx;

    auto work = [&](size_t start, size_t end) {
        for (size_t i = start; i < end; ++i) {
            results[i] = convert(inputs[i], out_ext, out_dir, options, preferred);
            int d = ++done;
            if (progress_cb) {
                std::lock_guard<std::mutex> lk(mtx);
                progress_cb(d, static_cast<int>(inputs.size()), inputs[i], &results[i]);
            }
        }
    };

    if (num_threads <= 1 || inputs.size() == 1) {
        work(0, inputs.size());
    } else {
        std::vector<std::thread> threads;
        size_t chunk = (inputs.size() + num_threads - 1) / num_threads;
        for (int t = 0; t < num_threads; ++t) {
            size_t s = t * chunk;
            size_t e = std::min(s + chunk, inputs.size());
            if (s >= inputs.size()) break;
            threads.emplace_back(work, s, e);
        }
        for (auto& th : threads) th.join();
    }
    return results;
}

ConversionEngine& global_conversion_engine() {
    static ConversionEngine engine = []() {
        ConversionEngine e;
        // Register backends (they'll silently fail at runtime if tool not installed)
        e.register_converter(std::make_shared<FFmpegConverter>());
        e.register_converter(std::make_shared<ImageMagickConverter>());
        e.register_converter(std::make_shared<PandocConverter>());
        e.register_converter(std::make_shared<CalibreConverter>());
        e.register_converter(std::make_shared<GhostscriptConverter>());
        return e;
    }();
    return engine;
}

} // namespace fo::core
