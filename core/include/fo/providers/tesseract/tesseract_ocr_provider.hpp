#pragma once

#include "fo/core/ocr_interface.hpp"

#ifdef FO_HAVE_TESSERACT
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#endif

namespace fo::providers::tesseract {

    class TesseractOCRProvider : public core::IOCRProvider {
      public:
        TesseractOCRProvider();
        ~TesseractOCRProvider();
        std::optional<core::OCRResult> recognize(const std::filesystem::path& image_path, const std::string& lang = "eng") override;
        std::optional<std::string> unavailable_reason(const std::string& lang = "eng") override;
        std::string name() const override { return "tesseract"; }

      private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };

}
