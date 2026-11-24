#include "fo/providers/tesseract/tesseract_ocr_provider.hpp"
#include "fo/core/provider.hpp"

#include <cstdlib>
#include <cstring>

#ifdef FO_HAVE_TESSERACT

namespace fo::providers::tesseract {
    struct TesseractOCRProvider::Impl {
        // :: qualification required: unqualified `tesseract` resolves to
        // this very namespace (fo::providers::tesseract), hiding the lib.
        ::tesseract::TessBaseAPI api;
        std::string lang;
        bool ready = false;
    };

    TesseractOCRProvider::TesseractOCRProvider() : impl_(std::make_unique<Impl>()) {}

    TesseractOCRProvider::~TesseractOCRProvider() = default;

    std::optional<std::string> TesseractOCRProvider::unavailable_reason(const std::string& lang) {
        if (impl_->ready && impl_->lang == lang) return std::nullopt;
        impl_->api.End();
        const char* datapath = std::getenv("TESSDATA_PREFIX");
        if (impl_->api.Init(datapath, lang.c_str())) {
            return std::string("Tesseract could not load language '") + lang + "' tessdata" +
                   (datapath ? std::string(" from TESSDATA_PREFIX=") + datapath
                             : std::string(" (TESSDATA_PREFIX is not set; point it at a tessdata directory)"));
        }
        impl_->lang = lang;
        impl_->ready = true;
        return std::nullopt;
    }

    std::optional<core::OCRResult> TesseractOCRProvider::recognize(const std::filesystem::path& image_path, const std::string& lang) {
        // Tesseract needs tessdata (e.g. eng.traineddata). Discovery:
        // TESSDATA_PREFIX env wins; nullptr lets Tesseract use its own
        // compiled-in/search paths (vcpkg installs share/tessdata nearby).
        if (!impl_->ready || impl_->lang != lang) {
            impl_->api.End();
            const char* datapath = std::getenv("TESSDATA_PREFIX");
            if (impl_->api.Init(datapath, lang.c_str())) return std::nullopt;
            impl_->lang = lang;
            impl_->ready = true;
        }
        Pix* image = pixRead(image_path.string().c_str());
        if (!image) return std::nullopt;
        impl_->api.SetImage(image);
        char* utf8 = impl_->api.GetUTF8Text();
        if (!utf8) { pixDestroy(&image); return std::nullopt; }
        core::OCRResult result;
        result.text = utf8;
        result.confidence = impl_->api.MeanTextConf() / 100.0f;
        result.lang = lang;
        delete[] utf8;
        pixDestroy(&image);
        return result;
    }
}

namespace fo::core {
    // Explicit registration (called from register_all_providers): the
    // TU-local static would be linker-dropped from fo_core.lib.
    void register_ocr_tesseract() {
        static Provider<IOCRProvider, fo::providers::tesseract::TesseractOCRProvider> p("tesseract");
        (void)p;
    }
}

#else

namespace fo::core {
    void register_ocr_tesseract() {}
}

#endif
