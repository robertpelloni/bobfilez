/***********************************************************************
*
* BTK compatibility aliases for the CsPointer annex library.
* These aliases provide an incremental migration path from Cs* naming
* toward BTK-branded public API usage without changing ABI.
*
***********************************************************************/

#ifndef BTK_POINTER_COMPAT_H
#define BTK_POINTER_COMPAT_H

#include <cs_enable_shared.h>
#include <cs_pointer_traits.h>
#include <cs_shared_array_pointer.h>
#include <cs_shared_pointer.h>
#include <cs_unique_array_pointer.h>
#include <cs_unique_pointer.h>
#include <cs_weak_pointer.h>

namespace BtkPointer = CsPointer;

template <typename T>
using BtkSharedPointer = CsPointer::CsSharedPointer<T>;

template <typename T>
using BtkWeakPointer = CsPointer::CsWeakPointer<T>;

template <typename T, typename Deleter = std::default_delete<T>>
using BtkUniquePointer = CsPointer::CsUniquePointer<T, Deleter>;

template <typename T>
using BtkSharedArrayPointer = CsPointer::CsSharedArrayPointer<T>;

template <typename T, typename Deleter = std::default_delete<CsPointer::cs_add_missing_extent_t<T>>>
using BtkUniqueArrayPointer = CsPointer::CsUniqueArrayPointer<T, Deleter>;

template <typename T>
using BtkEnableSharedFromThis = CsPointer::CsEnableSharedFromThis<T>;

#endif
