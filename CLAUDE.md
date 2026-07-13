# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a complete TypeScript implementation of the Ejscript core API for the Bun runtime. It provides 35+ core classes enabling Ejscript applications to run on Bun with minimal code changes - only requiring ES6 import statements.

## Build Commands

```bash
# Install dependencies
bun install

# Type check
bun run typecheck

# Build (compile TypeScript to JavaScript with type definitions)
bun run build

# Run all tests
bun test

# Run a specific test file
bun test <path-to-test-file>

# Run examples
bun examples/basic.ts
```

## Architecture

### Core Module Structure

The codebase follows a layered modular design:

```
src/
├── index.ts              # Main exports
├── globals.ts            # Global namespace (ejs)
├── core/
│   ├── App.ts           # Application singleton - central state management
│   ├── Path.ts          # Path operations (854 lines, 80+ methods)
│   ├── File.ts          # Asynchronous file I/O with Stream interface
│   ├── Http.ts          # Full HTTP/HTTPS client (678 lines, 40+ methods)
│   ├── FileSystem.ts    # File system operations
│   ├── Socket.ts        # TCP/UDP networking
│   ├── WebSocket.ts     # WebSocket client
│   ├── System.ts        # System information
│   ├── Config.ts        # Platform configuration
│   ├── streams/         # Stream infrastructure
│   │   ├── Stream.ts    # Base interface
│   │   ├── ByteArray.ts # Growable byte buffer
│   │   ├── TextStream.ts
│   │   └── BinaryStream.ts
│   ├── utilities/       # Utilities (Logger, Cache, Timer, Cmd, etc.)
│   ├── async/          # Concurrency (Emitter, Worker)
│   └── types/          # JavaScript type extensions
```

### Key Architectural Patterns

1. **Immutability**: Path objects are immutable once created
2. **Stream Interface**: File implements the Stream interface for unified I/O
3. **Singleton Pattern**: App is a singleton managing application state
4. **Event-Driven**: Emitter provides event-driven programming support
5. **Bun Native**: Leverages Bun's native APIs (fs, fetch, spawn) for performance

### ES4-to-ES6 Translation Approach

- **Classes** → Direct ES6 class mapping with TypeScript types
- **Namespaces** → ES6 modules with nested objects
- **Optional Typing** → Full TypeScript type safety
- **No Operator Overloading** → Not supported in JavaScript (ES limitation)

## Testing Strategy

### Test Location and Naming
- Tests are located in `test/` directory
- Test files use `.test.ts` extension
- Core class tests are in `test/core/`
- Integration tests are in `test/integration/`

### Running Tests
```bash
# Run all tests
bun test

# Run specific test file
bun test test/core/path.test.ts

# Run tests in watch mode
bun test --watch
```

### Test Structure
Tests use Bun's built-in test runner with `describe`, `it`, and `expect` assertions:

```typescript
import { describe, it, expect } from 'bun:test'
import { Path } from '../src/core/Path'

describe('Path', () => {
    it('should create absolute paths', () => {
        const p = new Path('/tmp/test')
        expect(p.isAbsolute).toBe(true)
    })
})
```

## Development Conventions

### TypeScript Configuration
- Strict type checking enabled
- Target: ESNext (Bun optimized)
- Module resolution: bundler mode
- All source files must have proper type annotations

### Code Style
- Use TypeScript types for all function parameters and return values
- Prefer `const` over `let`, avoid `var`
- Use ES6+ features (arrow functions, destructuring, spread)
- JSDoc comments for public APIs
- Single quotes for strings (except when avoiding escapes)

### File Organization
- One class per file
- File name matches class name
- Export main class as named export
- Group related functionality in subdirectories

## API Compatibility

This implementation maintains strong API compatibility with native Ejscript, but requires some changes for migration:

### Required Changes for Migration

1. **ES6 Imports Required**
   - All classes must be explicitly imported using ES6 import syntax
   - Example: `import { Path, File, Http, Socket } from 'ejscript'`

2. **For-In Loop with Numbers** ⚠️ SYNTAX DIFFERENCE
   - **Ejscript**: `for (let i in N)` iterates from 0 to N-1
   - **JavaScript/TypeScript**: `for (let i in N)` does nothing (N is not iterable)
   - **Migration**: Replace `for (let i in count)` with `for (let i = 0; i < count; i++)`
   - Example:
     ```javascript
     // Ejscript:
     for (let i in 64) { ... }  // Iterates 0-63

     // TypeScript/Bun:
     for (let i = 0; i < 64; i++) { ... }  // Equivalent
     ```

2. **File I/O Methods are Async (v2.0.0+)** ⚠️ BREAKING CHANGE
   - `File.open(options?)` → Returns `Promise<File>` (must await)
   - `File.close()` → Returns `Promise<void>`
   - `File.read(buffer, offset?, count?)` → Returns `Promise<number | null>`
   - `File.write(...items)` → Returns `Promise<number>`
   - `File.readBytes(count?)` → Returns `Promise<ByteArray | null>`
   - `File.readString(count?)` → Returns `Promise<string | null>`
   - `File.readLines()` → Returns `Promise<string[] | null>`
   - `File.writeLine(...items)` → Returns `Promise<number>`
   - **File constructor NO LONGER auto-opens** - must call `await file.open()` explicitly
   - **Path.open(options?)** → Returns `Promise<File>` (handles opening automatically)
   - **Path.openTextStream(mode?)** → Returns `Promise<TextStream>`
   - **Path.openBinaryStream(mode?)** → Returns `Promise<BinaryStream>`

3. **Stream Methods are Async (v2.0.0+)** ⚠️ BREAKING CHANGE
   - `TextStream.read()`, `readLine()`, `readLines()`, `readString()` → All return Promises
   - `TextStream.write()`, `writeLine()` → Return Promises
   - `TextStream.close()` → Returns `Promise<void>`
   - `BinaryStream.read*()` methods → All return Promises
   - `BinaryStream.write*()` methods → All return Promises
   - `BinaryStream.close()` → Returns `Promise<void>`

4. **Socket I/O Methods are Async**
   - `Socket.read(buffer, offset?, count?)` → Returns `Promise<number | null>`
   - `Socket.write(...data)` → Returns `Promise<number>`
   - Must use `await` when calling these methods
   - Tests must be written with async functions or use `.then()` chains

5. **HTTP Methods are Async**
   - `Http.finalize()` → Returns `Promise<void>` (must await before reading response)
   - `Http.wait(timeout?)` → Returns `Promise<boolean>`
   - `Http.fetch(uri, method?, ...data)` → Returns `Promise<string>` (static method)

6. **Event-Driven Alternative Available**
   - For high-performance code, use event-driven pattern: `socket.on('readable', handler)`
   - Avoids async overhead for streaming scenarios

7. **Type Annotations**
   - TypeScript is recommended but not required
   - Type definitions are provided for better IDE support

**Migration Example (v1.x to v2.0.0):**

```typescript
// v1.x - Synchronous (NO LONGER WORKS in v2.0.0)
import { File } from 'ejscript'
const file = new File('/tmp/test.txt', 'r')  // Auto-opened
const content = file.readString()
file.close()

// v2.0.0 - Asynchronous (REQUIRED)
import { File } from 'ejscript'
const file = new File('/tmp/test.txt')
await file.open('r')  // Must explicitly open
const content = await file.readString()
await file.close()

// v2.0.0 - Simplified with Path.open() (RECOMMENDED)
import { Path } from 'ejscript'
const file = await new Path('/tmp/test.txt').open('r')
const content = await file.readString()
await file.close()
```

## Common Development Tasks

### Adding a New Core Class
1. Create file in `src/core/` or appropriate subdirectory
2. Implement class with full TypeScript types
3. Add JSDoc comments for all public methods
4. Export from `src/index.ts`
5. Create corresponding test file in `test/core/`
6. Run `bun test` to verify

### Extending Type Extensions
Type extensions are in `src/core/types/`. They augment native JavaScript types:
- Declare interface extensions matching native types
- Implement as prototype methods at module load
- Export extensions from `src/index.ts`

### Working with Bun Native APIs
- Use `import * as fs from 'fs'` for file operations
- Use `fetch()` for HTTP requests
- Use `Bun.spawn()` for process execution
- Reference Bun documentation for native API details

## Key Implementation Details

### Path Class
- Immutable path objects with 80+ methods
- Cross-platform (Windows/Unix) path handling
- File operations: copy, move, remove, rename
- MIME type detection built-in
- Glob pattern support for file iteration

### HTTP Class
- Built on Bun's `fetch()` API
- Supports all HTTP methods (GET, POST, PUT, DELETE, etc.)
- Full header/cookie management
- SSL/TLS support
- Authentication (Basic, Digest)
- File upload with multipart/form-data
- Automatic redirect handling

### App Singleton
- Manages application-wide state
- Provides access to: args, env, stdin, stdout, stderr
- Working directory control
- Config file loading (.ejsrc format)
- Event loop integration

### Stream System
- Stream interface for unified I/O
- ByteArray: growable byte buffer
- TextStream: text I/O wrapper
- BinaryStream: binary I/O with endian control
- File implements Stream interface

## Documentation

### Root Level
- [README.md](README.md) - Project overview and quick start
- [docs/](docs/) - User-facing guides and compatibility reference
- JSDoc comments in source files provide API documentation

### doc/ Directory Structure
All comprehensive project documentation is organized under `doc/`. Start at
[doc/MAP.md](doc/MAP.md) - the navigation entry point.
- [doc/overview/product.md](doc/overview/product.md) - What the system is and does
- [doc/overview/roadmap.md](doc/overview/roadmap.md) - Project roadmap and future plans
- [doc/architecture/system.md](doc/architecture/system.md) - Architecture and design decisions
- [doc/architecture/components/](doc/architecture/components/) - API compatibility and type system
- [doc/features/INDEX.md](doc/features/INDEX.md) - Feature index
- [doc/operations/PROCEDURES.md](doc/operations/PROCEDURES.md) - Development procedures
- [doc/engineering/](doc/engineering/) - Test coverage analysis
- [doc/sessions/CHANGELOG.md](doc/sessions/CHANGELOG.md) - Complete change history
- [doc/references/REFERENCES.md](doc/references/REFERENCES.md) - External resources
- [doc/archive/](doc/archive/) - Superseded and historical documents

## Performance Considerations

- File operations use Bun's native async APIs via fs.promises (optimized in Bun)
- HTTP uses fetch() (optimized in Bun)
- Path operations are lightweight (no filesystem access unless needed)
- ByteArray uses JavaScript Uint8Array for efficient binary data
- Timer uses Bun's native timer implementation
- Async I/O enables concurrent operations without blocking

## Limitations and Known Issues

1. **All File I/O is Async**: No synchronous variants (use fs.readFileSync if needed for simple cases)
2. **Glob Patterns**: Basic glob support implemented, advanced patterns may need enhancement
3. **XML/E4X**: Not implemented (optional future enhancement)
4. **Operator Overloading**: Cannot be implemented in JavaScript
5. **Worker Threads**: Basic implementation, may need enhancement for complex use cases
6. **Logger File Output**: Minor timing considerations when logging to files due to async writes

## Version History

### v2.0.0 (Current) - Async I/O Conversion
- **BREAKING**: File I/O operations are now asynchronous
- **BREAKING**: File constructor no longer auto-opens files
- **BREAKING**: Stream read/write methods are async
- **BREAKING**: Path.open(), openTextStream(), openBinaryStream() are async
- Improved: Better concurrency support
- Improved: Non-blocking I/O operations
- Fixed: All 1374 test assertions passing

### v1.x - Synchronous I/O
- Original synchronous file operations
- Auto-opening File constructor

## Future Enhancement Areas

- Full glob pattern matching (*, **, ?, [])
- XML parsing support
- Additional utility modules (@ejscript/template, @ejscript/mail, etc.)
- Performance benchmarking suite
- Synchronous convenience methods for simple use cases

# Important Notes
- Use TestMe for unit tests
- Always read the ../CLAUDE.md for additional guidance and information.
- Ejscript types are not like typescript types that have no runtime impact. Ejscript will case parameters to the required argument type and cast return values to the required function return type if possible.
- In Ejscript, a type defined with a trailing ? means can be nullable. A trailing ! means not nullable.
- In API doc, do not being multi-line comments with "*"
- **Cmd class usage:**
  - **String commands** (e.g., `new Cmd("echo hello | grep h")`) are wrapped in a shell (bash/sh/cmd.exe) to support shell features (pipes, redirects, built-ins, wildcards)
  - **Array commands** (e.g., `new Cmd(['git', '--version'])`) execute directly without shell wrapper for better security and performance
  - Array commands must use real executables, not shell built-ins (use `['bash', '--version']` not `['echo', 'test']` on Windows)
  - For multiple arguments, prefer array syntax on Windows to avoid shell parsing issues: `new Cmd([program, arg1, arg2])` instead of `new Cmd(program + ' ' + arg1 + ' ' + arg2)`
-


## Project Documentation

This module maintains structured documentation in the `doc/` directory to assist Claude Code and
developers. It follows the standard layout in `~/dev/doc/DIRECTORIES.md`:

- **doc/MAP.md** - AI navigation entry point; start here
- **doc/overview/** - Product overview and roadmap
- **doc/requirements/** - System, use-case, security, and regulatory requirements
- **doc/architecture/** - System design and component designs
- **doc/features/** - Feature-centric implementation units (see `INDEX.md`)
- **doc/engineering/** - Development standards, tooling, and coverage analysis
- **doc/security/** - Security practices and advisories
- **doc/operations/** - Development, build, test, and release procedures
- **doc/issues/** - Issue tickets (`tickets/NNNN.md`, managed by the `tickets` CLI)
- **doc/compliance/** - CRA and traceability documentation
- **doc/releases/** - Per-release artifacts
- **doc/sessions/** - Changelog and session logs
- **doc/references/** - External documentation and resources
- **doc/archive/** - Superseded and historical documents

Claude Code capabilities (commands, skills, agents, workflows, prompts) live under `.claude/`.

## Additional Resources

- **Parent Project**: See `../CLAUDE.md` for general build commands, testing procedures, and overall EmbedThis architecture
- **API Documentation**: JSDoc comments in `src/`; user-facing guides in `docs/`
- **Project Documentation**: See `doc/` for architecture, roadmap, procedures, and context
