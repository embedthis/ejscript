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
│   ├── File.ts          # Synchronous file I/O with Stream interface
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

This implementation maintains full API compatibility with native Ejscript. The only required change for migration is adding imports:

**Before (native Ejscript):**
```javascript
let path = new Path('/tmp/test.txt')
```

**After (Ejscript for Bun):**
```typescript
import { Path } from 'ejscript'
let path = new Path('/tmp/test.txt')
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
- [QUICK_START.md](QUICK_START.md) - Getting started guide
- JSDoc comments in source files provide API documentation

### .agent/ Directory Structure
All comprehensive project documentation is organized under `.agent/`:
- [.agent/designs/DESIGN.md](.agent/designs/DESIGN.md) - Architecture and design decisions
- [.agent/plans/PLAN.md](.agent/plans/PLAN.md) - Project roadmap and future plans
- [.agent/procedures/PROCEDURES.md](.agent/procedures/PROCEDURES.md) - Development procedures
- [.agent/logs/CHANGELOG.md](.agent/logs/CHANGELOG.md) - Complete change history
- [.agent/context/CURRENT.md](.agent/context/CURRENT.md) - Current project state
- [.agent/references/REFERENCES.md](.agent/references/REFERENCES.md) - External resources
- [.agent/README.md](.agent/README.md) - Documentation structure overview

## Performance Considerations

- File operations use Bun's native synchronous APIs (fast)
- HTTP uses fetch() (optimized in Bun)
- Path operations are lightweight (no filesystem access unless needed)
- ByteArray uses JavaScript Uint8Array for efficient binary data
- Timer uses Bun's native timer implementation

## Limitations and Known Issues

1. **Synchronous I/O Only**: File operations are currently synchronous
2. **Glob Patterns**: Basic glob support implemented, advanced patterns may need enhancement
3. **XML/E4X**: Not implemented (optional future enhancement)
4. **Operator Overloading**: Cannot be implemented in JavaScript
5. **Worker Threads**: Basic implementation, may need enhancement for complex use cases

## Future Enhancement Areas

- Async/await versions of file I/O operations
- Full glob pattern matching (*, **, ?, [])
- XML parsing support
- Additional utility modules (@ejscript/template, @ejscript/mail, etc.)
- Performance benchmarking suite

# Important Notes
- Use TestMe for unit tests
- Always read the ../CLAUDE.md for additional guidance and information.
- Ejscript types are not like typescript types that have no runtime impact. Ejscript will case parameters to the required argument type and cast return values to the required function return type if possible.
- In Ejscript, a type defined with a trailing ? means can be nullable. A trailing ! means not nullable.
- In API doc, do not being multi-line comments with "*"
- 

