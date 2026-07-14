# Ejscript for Bun

A Bun port of the [Ejscript](https://github.com/embedthis/ejscript-native) library.

Ejscript was an ES4-era JavaScript environment with its own native compiler and virtual machine.
This package carries its core API forward onto [Bun](https://bun.sh): the same 35+ classes — `Path`,
`File`, `Http`, `App`, `Socket`, and the rest — reimplemented in TypeScript on top of Bun's native
APIs, with async I/O throughout. There is no custom VM here; your code runs on Bun.

The original native C implementation lives at
[embedthis/ejscript-native](https://github.com/embedthis/ejscript-native) and is now an archived
educational project.

**Status**: Production ready (2.x) — 35+ core classes, 99%+ API compatibility, 33/33 tests passing
(1468 assertions), zero runtime dependencies.

Existing Ejscript code usually needs only ES6 `import` statements and `await` on I/O to run here.
See [Migration](#migration) below.

## Features

- **Async I/O throughout** — `File`, `Path`, `TextStream`, `BinaryStream`, and `Cmd` all use async/await
- **Complete core API** — Path, File, Http, App, Emitter, Socket, WebSocket, Worker, and the stream classes
- **Type extensions** — String, Array, Date, Number, and Object extensions, Ejscript compatible
- **Full TypeScript support** — strict typing across every public API
- **Bun optimized** — built on Bun native APIs (`fs.promises`, `fetch`, `Bun.spawn`)
- **Zero runtime dependencies**

## Installation

```bash
bun add @embedthis/ejscript
```

For local development against a checkout:

```bash
bun run build && bun link          # in this directory
bun link @embedthis/ejscript       # in your project
```

See [docs/LINKING.md](docs/LINKING.md) for details.

## Quick Start

```typescript
import { Path, File, Http, App } from '@embedthis/ejscript'

// Path operations (async)
const path = new Path('/tmp/test.txt')
await path.write('Hello from Ejscript on Bun!')
console.log(await path.readString())

// File handles, when you need more control
const file = await new Path('/tmp/test.txt').open('r')
console.log(await file.readString())
await file.close()

// HTTP — issue the request, then await completion
const http = new Http()
http.get('https://api.github.com')
await http.wait()
console.log(http.status, http.response)

// Application info
console.log('Working directory:', App.dir.name)
console.log('Arguments:', App.args)
```

Run the examples:

```bash
bun docs/examples/basic.ts
bun docs/examples/http-streaming.ts
```

## Core APIs

**File system and I/O** — `Path` (80+ methods), `File`, `FileSystem`, `ByteArray`, `TextStream`,
`BinaryStream`

**Application** — `App` (args, env, standard I/O), `Config`, `System`, `Args`

**Networking**
- `Http` — full HTTP/HTTPS client (40+ methods): all methods, SSL/TLS, cookies, file upload
  - Partial URLs, Ejscript style: `'4100/path'` → `'http://127.0.0.1:4100/path'`
  - Basic and Digest auth (RFC 2617/7616), auto-detected — just call `setCredentials()`
  - Streaming uploads: pass a `ReadableStream` to `post()`, or write incrementally then `finalize()`
- `Socket` — TCP/UDP; `WebSocket` — WebSocket client; `Uri` — URI parsing

**Utilities** — `Logger`, `Cache`, `Timer`, `Cmd`, `Memory`, `GC`, `Inflector`

**Concurrency** — `Emitter` (events), `Worker` (worker threads)

**Type extensions** — `String` (toPascal, toCamel, expand…), `Array` (unique, contains, clone…),
`Object` (blend, clone, getType…), `Date` (format, elapsed, future…), `Number` (format)

## Migration

### From v1.x

v2.0.0 made all file I/O asynchronous, and the `File` constructor no longer auto-opens:

```typescript
// v1.x — no longer works
const file = new File('/tmp/test.txt', 'r')   // auto-opened
const content = file.readString()
file.close()

// v2.x
const file = await new Path('/tmp/test.txt').open('r')
const content = await file.readString()
await file.close()
```

Add `await` to every `File`/`Path`/stream read, write, open, and close call, and make the enclosing
functions `async`. The same applies to `openTextStream()` and `openBinaryStream()`.

### From native Ejscript

```typescript
import { Path, Http } from '@embedthis/ejscript'   // imports are required

const path = new Path('/tmp/test.txt')
await path.write('data')                   // I/O is async

const http = new Http()
http.get('https://api.example.com')
await http.wait()                          // wait for completion
print(http.response)
```

Sockets, WebSockets, and the type extensions are unchanged. One syntax difference to watch: Ejscript's
`for (let i in 10)` iterates 0-9, which JavaScript does not do — use a counted `for` loop.

Full guides: [docs/COMPATIBILITY.md](docs/COMPATIBILITY.md) for the API reference,
[docs/migration/APP_MIGRATION.md](docs/migration/APP_MIGRATION.md) for applications, and
[docs/migration/TEST_MIGRATION.md](docs/migration/TEST_MIGRATION.md) for test suites.

## Project Structure

```
ejscript/
├── src/
│   ├── core/          # Core classes (Path, File, Http, etc.)
│   ├── streams/       # Stream infrastructure
│   ├── utilities/     # Utility classes
│   ├── async/         # Async/concurrency
│   └── types/         # Type extensions
├── test/              # Unit tests (TestMe, *.tst.ts)
├── doc/               # Project documentation (architecture, roadmap, procedures)
└── docs/              # User-facing guides (API, migration)
    └── examples/      # Example code
```

## Testing

Tests use the [TestMe](https://github.com/embedthis/testme) framework:

```bash
make test              # Full suite
cd test && tm          # Same, directly
cd test && tm path     # A single test
```

Current: 33/33 tests passing, 1468/1468 assertions. See
[doc/engineering/coverage-analysis-2025-10-27.md](doc/engineering/coverage-analysis-2025-10-27.md)
for coverage analysis.

## Documentation

**Users** — [docs/API.md](docs/API.md) (complete API reference),
[docs/EXAMPLES.md](docs/EXAMPLES.md) (common patterns),
[docs/COMPATIBILITY.md](docs/COMPATIBILITY.md) (compatibility guide)

**Developers** — [doc/MAP.md](doc/MAP.md) is the entry point, covering
[architecture](doc/architecture/system.md), [roadmap](doc/overview/roadmap.md),
[procedures](doc/operations/PROCEDURES.md), and the
[changelog](doc/sessions/CHANGELOG.md). [CLAUDE.md](CLAUDE.md) holds AI assistant guidance.

## Compatibility

Bun 1.0+, TypeScript 5.0+, on macOS, Linux, and Windows.

## Credits

Based on Embedthis Ejscript, the original native ES4 implementation, now archived at
[embedthis/ejscript-native](https://github.com/embedthis/ejscript-native).

## License

GPL-2.0-only. See [LICENSE.md](LICENSE.md).
