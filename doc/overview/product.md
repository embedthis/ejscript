# Product Overview

**Project**: Ejscript for Bun (`ejscript`)
**Version**: 2.0.0
**Status**: Production ready

---

## What It Is

`ejscript` is a complete TypeScript implementation of the Ejscript core API targeting the Bun
runtime. It reimplements 35+ core Ejscript classes — `Path`, `File`, `Http`, `Socket`,
`WebSocket`, `ByteArray`, `App`, `Logger`, `Cmd`, `Timer`, `Uri` and others — on top of Bun's
native APIs, so that existing Ejscript applications can run on a modern JavaScript runtime.

Migration is intended to be near-mechanical: in most cases the only source change required is the
addition of ES6 `import` statements. A small number of genuine language differences (for example
Ejscript's `for (let i in N)` numeric iteration) must be rewritten; these are enumerated in the
root [CLAUDE.md](../../CLAUDE.md) and [README.md](../../README.md).

## Goals

- **API compatibility** — preserve the native Ejscript core API surface (99%+ achieved).
- **Type safety** — full TypeScript types for every public API.
- **Performance** — use Bun native APIs (`fs.promises`, `fetch`, `Bun.spawn`) directly.
- **Minimal migration cost** — import statements, not rewrites.
- **Modern async** — async/await across the whole I/O stack (v2.0.0).

## Non-Goals

- **Operator overloading** — not expressible in JavaScript.
- **Full XML/E4X support** — deferred; may be added as an optional module.
- **Synchronous I/O variants** — v2.0.0 is async-only; callers needing sync behaviour use
  Node/Bun primitives such as `fs.readFileSync` directly.

## Users

Developers maintaining Ejscript applications who want to run them on Bun, and developers who want
the Ejscript core API (particularly its `Path` and `Http` ergonomics) available in a TypeScript
codebase.

## Scope

The package delivers the Ejscript **core** library only. Optional Ejscript modules (template
engine, mail, zlib, tar, unix utilities) are out of scope for this package and are tracked as
potential separate packages in [roadmap.md](roadmap.md).

## Current State

| Aspect | Status |
|--------|--------|
| Core classes | 35+ implemented |
| API compatibility | 99%+ |
| Async I/O conversion | Complete (v2.0.0) |
| Tests | 33/33 passing, 1468/1468 assertions (TestMe) |
| Runtime dependencies | None (Bun native only) |
| Untested legacy classes | GC, Memory, MprLog, Inflector (optional/legacy) |

## Key Breaking Change (v2.0.0)

All file, stream, and command I/O is asynchronous, and the `File` constructor no longer auto-opens:

```typescript
// v1.x — synchronous (no longer works)
const file = new File('/tmp/test.txt', 'r')
const content = file.readString()

// v2.0.0 — asynchronous
const file = await new Path('/tmp/test.txt').open('r')
const content = await file.readString()
await file.close()
```

See [../architecture/system.md](../architecture/system.md) for the async architecture and the
full list of converted methods.

## Related Documents

- [roadmap.md](roadmap.md) — planned work, phases, and maintenance policy
- [../architecture/system.md](../architecture/system.md) — system design
- [../architecture/components/api-compatibility.md](../architecture/components/api-compatibility.md) — compatibility detail
- [../MAP.md](../MAP.md) — documentation entry point
