# MAP.md — Project Navigation

**Project**: Ejscript for Bun (`ejscript`)
**Version**: 2.0.0
**Status**: Production ready — async I/O complete
**Last Updated**: 2026-07-14

---

## System Summary

A complete TypeScript implementation of the Ejscript core API for the Bun runtime. It provides
35+ core classes so that Ejscript applications can run on Bun with minimal code changes — in most
cases only the addition of ES6 `import` statements.

As of v2.0.0 all file, stream, and command I/O is asynchronous. The `File` constructor no longer
auto-opens; callers must `await file.open(mode)`. See
[architecture/system.md](architecture/system.md) for the async architecture and the full list of
converted methods, and the root [README.md](../README.md) for the v1.x → v2.0.0 migration guide.

**Key characteristics**

| Aspect | Detail |
|--------|--------|
| Runtime | Bun 1.0+ (zero runtime dependencies) |
| Language | TypeScript 5.0+, strict mode |
| API compatibility | 99%+ with native Ejscript core |
| Tests | TestMe framework (`.tst.ts`), run with `tm` |
| Test status | 33/33 tests, 1468/1468 assertions passing |

---

## Feature Index

No feature directories exist yet — this project predates the feature-centric documentation layout,
and its work is recorded as a roadmap plus session history rather than as discrete features. New
work should be captured as a feature under `doc/features/<kebab-case>/`.

See [features/INDEX.md](features/INDEX.md).

---

## Key Documents

| Document | Path | Description |
|----------|------|-------------|
| Product overview | [overview/product.md](overview/product.md) | What the system is and does |
| Roadmap | [overview/roadmap.md](overview/roadmap.md) | Phases, future work, maintenance and versioning policy |
| System architecture | [architecture/system.md](architecture/system.md) | Overall design, async I/O architecture, core components |
| API compatibility | [architecture/components/api-compatibility.md](architecture/components/api-compatibility.md) | Compatibility analysis against native Ejscript |
| Ejscript type system | [architecture/components/ejscript-type-system.md](architecture/components/ejscript-type-system.md) | Type system semantics and translation |
| Development procedures | [operations/PROCEDURES.md](operations/PROCEDURES.md) | Environment setup, build, test, review, release |
| Test coverage analysis | [engineering/coverage-analysis-2025-10-27.md](engineering/coverage-analysis-2025-10-27.md) | v2.0.0 coverage analysis and recommendations |
| Feature index | [features/INDEX.md](features/INDEX.md) | All features and their status |
| Changelog | [sessions/CHANGELOG.md](sessions/CHANGELOG.md) | Change history |
| External references | [references/REFERENCES.md](references/REFERENCES.md) | Links and resources |

---

## Navigation

- **New to the project?** Start with [overview/product.md](overview/product.md), then
  [architecture/system.md](architecture/system.md).
- **Looking for a feature?** Check [features/INDEX.md](features/INDEX.md).
- **Filing a bug?** Use the `tickets` CLI; ticket files live in `issues/tickets/NNNN.md`.
- **Architecture decisions?** See [architecture/](architecture/).
- **Planning future work?** See [overview/roadmap.md](overview/roadmap.md).
- **Setting up or releasing?** See [operations/PROCEDURES.md](operations/PROCEDURES.md).
- **What changed recently?** See [sessions/CHANGELOG.md](sessions/CHANGELOG.md). Historical
  per-session logs are archived under [archive/sessions/](archive/sessions/).
- **Historical documents?** See [archive/](archive/).

---

## Quick Commands

```bash
bun install          # Install dependencies
bun run typecheck    # Type check
bun run build        # Build to dist/
tm                   # Run all tests (TestMe, from test/)
tm path.tst.ts       # Run a single test
bun docs/examples/basic.ts
```
