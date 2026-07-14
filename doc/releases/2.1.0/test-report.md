# Test Report — 2.1.0

Product: Ejscript for Bun (`@embedthis/ejscript`)
Version: 2.1.0
Release Date: 2026-07-14
Release Commit: `adb7533`
Status: Approved

## 1. Summary

This report records the test evidence for the 2.1.0 release. The suite is the authority for release
authorization: `make promote` (the only publishing path) depends on the `test` target, so the full
TestMe suite must pass before the tarball can be uploaded to npm.

### Overall Result: PASS

| Category | Total | Passed | Failed | Skipped | Pass Rate |
|----------|-------|--------|--------|---------|-----------|
| Test files (TestMe) | 33 | 33 | 0 | 0 | 100% |
| Assertions | 1430 | 1430 | 0 | 0 | 100% |

The project does not separate unit from integration suites; the 33 TestMe files span both. HTTP,
WebSocket, and Socket suites exercise real listeners on loopback and are integration-level.

Counts are those recorded at release commit `adb7533` (corrected in `7d3e473`) and enforced by the CI
matrix below. They were not independently re-measured when this report was written; CI and the
`make promote` gate are the evidence of record.

> **Why the count changed from 2.0.0.** The 2.0.0 milestone reported 1442 assertions. That figure was
> inflated: un-awaited nested `describe()` blocks were silently dropped and reported as passing without
> executing (fixed in `b30edb8`). 1430/1430 is the first *trustworthy* total for this project.

## 2. Test Environment

| Property | Value |
|----------|-------|
| Test framework | TestMe (`tm`) 0.8.x, `.tst.ts` files |
| Runner invocation | `make test` (runs `bin/prep-test`, then `tm`) |
| CI/CD platform | GitHub Actions (`.github/workflows/ci.yml`) |
| Runtime | Bun (latest), minimum supported Bun >= 1.0.0 |
| Build configuration | Release (`bun run build`) |

## 3. Platform Verification

Verified by the CI matrix (`os: [ubuntu-latest, macos-latest, windows-latest]`, `fail-fast: false`).

| Platform | Tier | Build | Tests | Status |
|----------|------|-------|-------|--------|
| Linux (ubuntu-latest) | 1 | Pass | Pass | Pass |
| macOS (macos-latest) | 1 | Pass | Pass | Pass |
| Windows (windows-latest) | 1 | Pass | Pass | Pass |

Windows parity was a specific focus of this release; see the Fixed section of
[release-notes.md](release-notes.md).

## 4. Security Testing

This is a pure-TypeScript library with **zero runtime dependencies**, executing on the Bun runtime. It
performs no memory management of its own and ships no native code, which bounds the applicable test
categories.

| Activity | Status | Notes |
|----------|--------|-------|
| Dependency vulnerability scan | Pass | Zero runtime dependencies — no third-party code is distributed in the tarball. See [vulnerability-disposition.md](vulnerability-disposition.md). |
| Type-level static analysis | Pass | `tsc` strict mode, zero errors (`bun run typecheck`, gated by `prepublishOnly`). |
| Dedicated SAST tool | **Not performed** | No SAST tooling is configured for this project. See Gaps. |
| Dynamic analysis | **Not performed** | No DAST tooling is configured. See Gaps. |
| Fuzz testing | **Not performed** | No fuzz harness exists. See Gaps. |
| Leak testing | Not applicable | Garbage-collected runtime; no manual allocation. |

## 5. Code Coverage

**Not measured.** No coverage tooling is configured for this project, so no line, branch, or function
coverage figures can be stated for 2.1.0. See Gaps.

## 6. Exit Criteria Verification

| Criterion | Status | Evidence |
|-----------|--------|---------|
| All tests passing | Pass | 33/33 files, 1430/1430 assertions; CI matrix green |
| Passing suite gates the upload | Pass | `make promote` depends on `test` |
| Typecheck clean | Pass | `bun run typecheck`, enforced by `prepublishOnly` |
| Build without errors | Pass | `bun run build` |
| Cross-platform verification | Pass | CI matrix: Linux, macOS, Windows |
| No known exploitable vulnerabilities | Pass | [vulnerability-disposition.md](vulnerability-disposition.md) |
| No third-party runtime code shipped | Pass | Zero runtime dependencies; `files: [dist, src, README.md, LICENSE.md]` |
| Artifact integrity recorded | Pass | [SHA256SUMS](SHA256SUMS) |
| Code coverage meets target | **Not met** | No coverage tooling configured; no target defined |
| SAST / fuzz clean | **Not met** | Not performed |

## 7. Gaps and Deferrals

These are recorded rather than waived. None is considered release-blocking for a zero-dependency,
memory-safe library, but each is a real gap against the standard release checklist.

| Gap | Severity | Justification for Deferral |
|-----|----------|---------------------------|
| No code coverage measurement | Medium | Suite covers all 35+ core classes by construction, but the actual percentage is unknown. Recommend adding `bun test --coverage` reporting and setting a target before the next minor release. |
| No SAST tooling | Low | `tsc` strict mode catches the type-level defect classes that dominate this codebase; no native memory unsafety is possible. |
| No fuzz harness | Low | The parsing surface (`Uri`, `JSON`, `Path`, HTTP header handling) would benefit from fuzzing. Candidate for a future release. |
| No performance benchmarks | Low | No performance requirements are currently specified for the library. |

## 8. Known Issues

Carried into the release; see [release-notes.md](release-notes.md) for the full list. None affects the
correctness of the shipped API surface.

| Issue | Severity | Justification for Deferral |
|-------|----------|---------------------------|
| No synchronous I/O variants | Low | Deliberate design; callers use `fs.*Sync` directly if needed. |
| Advanced glob patterns incomplete | Low | Common patterns work; documented limitation. |
| XML/E4X not implemented | Low | Optional legacy feature; no demand. |
| `Worker` is a basic implementation | Low | Sufficient for current use cases; documented. |

---

*This test report is part of the release conformity dossier for 2.1.0.*
