/***********************************************************************
*
* BTK compatibility aliases for the CsSignal annex library.
* These aliases provide an incremental migration path from Cs* naming
* toward BTK-branded public API usage without changing ABI.
*
***********************************************************************/

#ifndef BTK_SIGNAL_COMPAT_H
#define BTK_SIGNAL_COMPAT_H

#include <cs_signal.h>
#include <cs_slot.h>

namespace BtkSignal = CsSignal;

using BtkConnectionKind = CsSignal::ConnectionKind;
using BtkDisconnectKind = CsSignal::DisconnectKind;
using BtkSignalBase     = CsSignal::SignalBase;
using BtkSlotBase       = CsSignal::SlotBase;
using BtkPendingSlot    = CsSignal::PendingSlot;

#endif
