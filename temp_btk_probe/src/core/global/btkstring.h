/***********************************************************************
*
* BTK compatibility aliases for the CsString annex library.
* These aliases provide an incremental migration path from Cs* naming
* toward BTK-branded public API usage without changing ABI.
*
***********************************************************************/

#ifndef BTK_STRING_COMPAT_H
#define BTK_STRING_COMPAT_H

#include <cs_char.h>
#include <cs_encoding.h>
#include <cs_string.h>
#include <cs_string_iterator.h>
#include <cs_string_view.h>

namespace BtkText = CsString;

using BtkChar            = CsString::CsChar;
using BtkStringUtf8      = CsString::CsString_utf8;
using BtkStringUtf16     = CsString::CsString_utf16;
using BtkStringView      = CsString::CsStringView;
using BtkStringViewUtf8  = CsString::CsStringView_utf8;
using BtkStringViewUtf16 = CsString::CsStringView_utf16;
using BtkString          = CsString::CsString;

template <typename E, typename A>
using BtkBasicString = CsString::CsBasicString<E, A>;

template <typename S>
using BtkBasicStringView = CsString::CsBasicStringView<S>;

template <typename E, typename A>
using BtkStringIterator = CsString::CsStringIterator<E, A>;

template <typename T>
using BtkStringReverseIterator = CsString::CsStringReverseIterator<T>;

#endif
