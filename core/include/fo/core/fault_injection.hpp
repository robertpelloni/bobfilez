#pragma once

#include <cstdlib>
#include <string>

namespace fo::core {

// Deterministic crash checkpoints for recovery testing (roast
// rounds 6/8). When FO_FAULT_AT names this exact point, the
// process _Exit(70)s on the spot: no destructors, no rollback —
// a hard kill with already-committed journal rows flushed.
// Production runs never set the variable, and the getenv probe
// is one comparison per checkpoint, so the hook costs nothing
// when disarmed. Tests enumerate the points; recovery after
// each is reconciled from journal state (see
// OperationRepository::reconcile_pending).
inline void fault_checkpoint(const char* point) {
    static const std::string target = [] {
#ifdef _WIN32
        char* buf = nullptr;
        size_t len = 0;
        if (_dupenv_s(&buf, &len, "FO_FAULT_AT") != 0 || !buf) return std::string{};
        std::string v(buf);
        free(buf);
        return v;
#else
        const char* v = std::getenv("FO_FAULT_AT");
        return v ? std::string(v) : std::string{};
#endif
    }();
    if (!target.empty() && target == point) {
        std::_Exit(70);
    }
}

} // namespace fo::core
