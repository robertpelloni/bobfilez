# bobfilez

> **Status: ALPHA — version truth is `VERSION.md` (not pinned here; it changes too often to hardcode)**
> **Scope cut, Nov 19, 2026:** bobfilez is a **file organizer**. That is the product.
> It is not an operating system, not a Windows replacement, and not 39
> professional applications in one. The "Grand Architecture" / OmniShell
> vision in `ARCHITECTURE.md` is aspirational and parked — see below.

## What actually works (verified Nov 17, 2026)

The C++20 core, driven by the CLI and the web dashboard:

- File hashing and deduplication
- Integrity scrub — baselines persist in SQLite (`file_hashes`); later runs
  report corrupted files by name (fixed v6.5.22; before that, scrub was a
  no-op that always reported `corrupted: 0`)
- Similar-image search via perceptual hashing (fixed v6.5.20; before that,
  it could never match)
- Video hashing (FFmpeg provider) and audio fingerprinting (Chromaprint)
- OCR, file scanning, batch rename (fixed v6.5.13; the parser previously
  rejected rename's own options)
- Encrypted vault (init / lock / unlock / list round-trip tested)
- Hash-chained forensic audit ledger (`fo_cli audit`)
- Web dashboard — browser-verified end-to-end Nov 17, 2026

OmniOracle is a **scaffold, not a working RAG assistant**: as of v6.5.21 it
says so plainly instead of fabricating answers with invented citations.
No model or index is wired up yet.

## Tests (Nov 17, 2026)

- Lean ctest: **28/28**
- JUCE-tree ctest: **25/25**
- Dashboard wiring: **18/18**

## What is NOT built / parked

Everything in `ARCHITECTURE.md` beyond the file-organizer core, including:
OmniShell (Windows 11 replacement), OmniVision (NLE), OmniAudio (DAW),
Photo Develop / Photo Library, OmniSec, OmniCrypt, OmniGit, OmniCluster,
Data Swarm (P2P), Cloud panels (S3 / GDrive / Azure), OmniVerse (3D),
OmniFlow, OmniTimeMachine, gamification (XP / achievements / quests),
and the rest of the 39-panel parity list. Those panels are scaffolds,
stubs, or documents — not shipped, working equivalents of DaVinci
Resolve, Ableton Live, Lightroom, IDA Pro, VeraCrypt, or any other
named product.

There is no release, no installer, and no user base yet. Backwards
compatibility is not guaranteed.

---

🛠️ ALPHA SOFTWARE UNDER CONSTRUCTION — Use at your own risk. Backwards compatibility not guaranteed.

```text
╔══════════════════════════════════════════════════════════════════════════════╗
║                                                                              ║
║                     ██╗   ██╗███╗   ██╗██████╗ ███████╗██████╗              ║
║                     ██║   ██║████╗  ██║██╔══██╗██╔════╝██╔══██╗             ║
║                     ██║   ██║██╔██╗ ██║██║  ██║█████╗  ██████╔╝             ║
║                     ██║   ██║██║╚██╗██║██║  ██║██╔══╝  ██╔══██╗             ║
║                     ╚██████╔╝██║ ╚████║██████╔╝███████╗██║  ██║             ║
║                      ╚═════╝ ╚═╝  ╚═══╝╚═════╝ ╚══════╝╚═╝  ╚═╝             ║
║                                                                              ║
║                     ██████╗ ██████╗ ███╗   ██╗███████╗████████╗██████╗      ║
║                    ██╔════╝██╔═══██╗████╗  ██║██╔════╝╚══██╔══╝██╔══██╗     ║
║                    ██║     ██║   ██║██╔██╗ ██║███████╗   ██║   ██████╔╝     ║
║                    ██║     ██║   ██║██║╚██╗██║╚════██║   ██║   ██╔══██╗     ║
║                    ╚██████╗╚██████╔╝██║ ╚████║███████║   ██║   ██║  ██║     ║
║                     ╚═════╝ ╚═════╝ ╚═╝  ╚═══╝╚══════╝   ╚═╝   ╚═╝  ╚═╝     ║
║                                                                              ║
║                     █████╗ ██╗     ██████╗ ██╗  ██╗ █████╗                  ║
║                    ██╔══██╗██║     ██╔══██╗██║  ██║██╔══██╗                 ║
║                    ███████║██║     ██████╔╝███████║███████║                 ║
║                    ██╔══██║██║     ██╔═══╝ ██╔══██║██╔══██║                 ║
║                    ██║  ██║███████╗██║     ██║  ██║██║  ██║                 ║
║                    ╚═╝  ╚═╝╚══════╝╚═╝     ╚═╝  ╚═╝╚═╝  ╚═╝                 ║
║                                                                              ║
║                    ╔══════════════════════════════════════╗                  ║
║                    ║     ⚠️  ALPHA SOFTWARE  ⚠️           ║                  ║
║                    ║  EXPECT BREAKING CHANGES & BUGS     ║                  ║
║                    ║  NOT READY FOR PRODUCTION USE       ║                  ║
║                    ╚══════════════════════════════════════╝                  ║
║                                                                              ║
╚══════════════════════════════════════════════════════════════════════════════╝
```
