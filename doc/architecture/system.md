# Ejscript Design Document

**Project**: Ejscript (Ejscript for Bun)
**Version**: 2.0.0
**Last Updated**: 2025-10-27
**Status**: Production Ready - Async I/O Complete

## Contents

1. [Overview](#overview)
2. [v2.0.0 Async I/O Architecture](#v200-async-io-architecture)
3. [Architecture](#architecture)
4. [Core Components](#core-components)
5. [Design Decisions](#design-decisions)
6. [Component Designs](#component-designs)

## Overview

Ejscript is a complete TypeScript implementation of the Ejscript core API for the Bun runtime. It provides 35+ core classes that enable Ejscript applications to run on Bun with minimal code changes.

### Goals

- **API Compatibility**: Maintain full compatibility with native Ejscript API
- **Type Safety**: Leverage TypeScript for compile-time type checking
- **Performance**: Utilize Bun's native APIs for optimal performance
- **Minimal Migration**: Require only import statement changes for migration
- **Modern Async**: Async/await patterns for all I/O operations (v2.0.0)

### Non-Goals

- Operator overloading (not possible in JavaScript)
- Full XML/E4X support (deferred to future versions)

### Achieved in v2.0.0

- ✅ **Complete Async I/O Conversion** (all file/command I/O operations)
- ✅ **100% Test Pass Rate** (32/32 tests, 1402/1402 assertions)
- ✅ Complete API compatibility (99%+)
- ✅ HTTP Async I/O (async methods for all HTTP operations)
- ✅ HTTP Partial URL support (Ejscript-style partial URLs: '4100/path', ':8080/api')
- ✅ Comprehensive test coverage with TestMe framework
- ✅ Production ready stability with zero regressions

## v2.0.0 Async I/O Architecture

### Design Philosophy

**Async-First Design**: All I/O operations use async/await for:
- Non-blocking event loop
- Better concurrency and parallelism
- Improved performance for I/O-heavy workloads
- Consistency with Bun's native async APIs
- Modern JavaScript async/await patterns

### Converted Methods (24 total)

**Path Class (13 methods)**:
- Read operations: `readBytes()`, `readString()`, `readJSON()`, `readLines()`, `readXML()`
- Write operations: `write()`, `append()`
- File operations: `copy()`, `remove()`, `removeAll()`, `truncate()`, `rename()`, `makeDir()`

**File Class (8 methods)**:
- Read operations: `read()`, `readBytes()`, `readString()`, `readLines()`
- Write operations: `write()`, `writeLine()`
- Lifecycle: `open()`, `close()`

**Cmd Class (4 methods)**:
- Read operations: `read()`, `readString()`, `readLines()`
- Static: `Cmd.run()`

**Stream Classes**:
- TextStream: All read/write methods async
- BinaryStream: All read/write methods async

### Breaking Changes

1. **File Constructor**: No longer auto-opens files
   - v1.x: `new File(path, mode)` → auto-opened
   - v2.0: `new File(path)` → must call `await file.open(mode)`

2. **All I/O Returns Promises**: Must use `await`
   - v1.x: `const data = path.readString()`
   - v2.0: `const data = await path.readString()`

3. **Logger Close**: Returns Promise when using file output
   - v1.x: `logger.close()` → sync
   - v2.0: `await logger.close()` → async (for files)

### Implementation Details

**Pending Write Tracking** ([Logger.ts](../../src/core/utilities/Logger.ts)):
```typescript
// Track pending async writes to ensure flush before close
private _pendingWrites: Promise<any>[] = []

write(...data: any[]): number {
    const result = this._outStream.write(data.join(' '))
    if (result instanceof Promise) {
        this._pendingWrites.push(result)
    }
}

close(): void | Promise<void> {
    if (this._pendingWrites.length > 0) {
        return Promise.all(this._pendingWrites).then(/* cleanup */)
    }
}
```

**Worker Exit Code** ([Worker.ts](../../src/core/async/Worker.ts)):
```typescript
// Let worker handle exit message instead of forcing termination
exit(code: number = 0): void {
    this.postMessage({ type: 'exit', code })
    // Removed: this.terminate() - let worker exit gracefully
}
```

### Migration Guide

See [README.md](../../README.md) for complete v1.x → v2.0 migration guide.

## Architecture

### Layered Design

```
┌─────────────────────────────────────────┐
│     Application Layer                   │
│  (User code using Ejscript API)             │
├─────────────────────────────────────────┤
│     Ejscript Core API Layer                 │
│  Path, File, Http, App, Stream, etc.    │
├─────────────────────────────────────────┤
│     Bun Native API Layer                │
│  fs, fetch, spawn, timers               │
├─────────────────────────────────────────┤
│     Operating System                    │
└─────────────────────────────────────────┘
```

### Module Organization

- **core/**: Core classes (App, Path, File, Http, etc.)
- **core/streams/**: Stream infrastructure
- **core/utilities/**: Utility classes (Logger, Cache, Timer, etc.)
- **core/async/**: Concurrency primitives (Emitter, Worker)
- **core/types/**: JavaScript type extensions
- **modules/**: Future extension modules

### Key Patterns

1. **Immutability**: Path objects are immutable once created
2. **Singleton**: App is a singleton managing global application state
3. **Interface-Based**: File implements Stream interface for unified I/O
4. **Event-Driven**: Emitter provides publish-subscribe pattern
5. **Prototype Extension**: Type extensions augment native prototypes

## Core Components

### 1. Path (854 lines)
**Purpose**: Immutable path manipulation and file operations
**Key Features**: 80+ methods, cross-platform, MIME detection, glob patterns
**Dependencies**: Node.js path module, Bun fs module

### 2. File (391 lines)
**Purpose**: Synchronous file I/O with Stream interface
**Key Features**: Multiple open modes, seeking, binary/text operations
**Dependencies**: Bun fs module, Stream interface

### 3. Http (678 lines)
**Purpose**: Full-featured HTTP/HTTPS client
**Key Features**: All HTTP methods, headers, cookies, SSL/TLS, auth, uploads
**Dependencies**: Bun fetch API

### 4. App (464 lines)
**Purpose**: Application singleton managing global state
**Key Features**: Args, env, I/O streams, config loading, module paths
**Dependencies**: System, Config, Path

### 5. Stream Infrastructure
**Purpose**: Unified I/O abstraction
**Components**:
- Stream (interface)
- ByteArray (growable byte buffer)
- TextStream (text I/O wrapper)
- BinaryStream (binary I/O with endian control)

### 6. Type Extensions
**Purpose**: Enhance native JavaScript types
**Types Extended**: String, Array, Object, Date, Number
**Implementation**: Prototype augmentation at module load

## Design Decisions

### ES4 to ES6+ Translation

| ES4 Feature | ES6+ Equivalent | Notes |
|------------|-----------------|-------|
| Classes | ES6 classes | Direct mapping |
| Namespaces | ES6 modules | Nested objects for sub-namespaces |
| Optional typing | TypeScript types | Compile-time only |
| Operator overloading | Not supported | ES limitation |
| Dynamic properties | Getters/setters | Using ES6 get/set |

### Synchronous vs Asynchronous

**Decision**: Implement synchronous I/O first

**Rationale**:
- Matches native Ejscript behavior
- Simpler initial implementation
- Bun's sync APIs are optimized
- Async can be added later without breaking changes

### Native API Selection

| Feature | Native API | Rationale |
|---------|-----------|-----------|
| File I/O | Bun fs module | Optimized for Bun |
| HTTP | fetch() | Standard, well-supported |
| Process | Bun.spawn() | Better than child_process |
| Timers | Bun native timers | High performance |

### Error Handling

- Throw JavaScript Error objects with descriptive messages
- Match native Ejscript error behavior where possible
- Use try-catch in public APIs that might fail
- Return null/undefined for non-critical failures (e.g., Path.accessed)

### Cross-Platform Support

**Strategy**: Use Node.js path module for cross-platform path handling

**Platform-Specific Handling**:
- Path separators (/ vs \)
- Line endings (LF vs CRLF)
- Case sensitivity
- Path formats (Unix vs Windows)

## Component Designs

### Additional Design Documents

- **[api-compatibility.md](components/api-compatibility.md)** - Comprehensive guide to Ejscript type extensions and ES6 compatibility
  - String, Array, Date, Object, Number extensions
  - ES6 equivalents and migration strategies
  - Performance considerations

### Path Design

**Immutability Pattern**:
```typescript
class Path {
    private _path: string  // Immutable after construction

    constructor(pathString: string = '.') {
        this._path = pathString
    }

    // All operations return new Path objects
    join(...others: string[]): Path {
        return new Path(path.join(this._path, ...others))
    }
}
```

**Why Immutable?**
- Prevents bugs from unexpected mutations
- Thread-safe by design
- Matches native Ejscript behavior
- Enables safe sharing across components

### App Singleton Design

**Singleton Pattern**:
```typescript
class AppClass {
    private static instance: AppClass

    private constructor() { /* private */ }

    static getInstance(): AppClass {
        if (!AppClass.instance) {
            AppClass.instance = new AppClass()
        }
        return AppClass.instance
    }
}

export const App = AppClass.getInstance()
```

**Why Singleton?**
- Single source of truth for application state
- Globally accessible args, env, I/O streams
- Matches native Ejscript App behavior

### Stream Interface Design

**Interface-Based Polymorphism**:
```typescript
interface Stream {
    read(count?: number): ByteArray | null
    write(data: ByteArray | string): number
    close(): void
    // ... other methods
}

class File implements Stream {
    // File-specific implementation
}
```

**Why Interface?**
- Enables polymorphic I/O operations
- File, Socket, and custom streams all work the same way
- Simplifies stream wrapping (TextStream, BinaryStream)

### Type Extension Design

**Prototype Augmentation**:
```typescript
// Declare interface extension
declare global {
    interface String {
        toPascal(): string
    }
}

// Implement on prototype
String.prototype.toPascal = function(): string {
    return this.split(/[_-\s]/)
        .map(word => word.charAt(0).toUpperCase() + word.slice(1).toLowerCase())
        .join('')
}
```

**Why Prototype?**
- Works with all string instances automatically
- Matches JavaScript idiom
- No wrapper objects needed

## Performance Considerations

### Optimization Strategies

1. **Lazy Evaluation**: Path operations don't touch filesystem until needed
2. **Native APIs**: Use Bun's optimized native functions
3. **Minimal Allocations**: Reuse buffers where possible (ByteArray)
4. **Efficient Data Structures**: Use Uint8Array for binary data

### Benchmarking Approach

- Focus on common operations: file reads, path joins, HTTP requests
- Compare against native Ejscript where possible
- Measure against Node.js equivalents
- Target: Match or exceed native Ejscript performance

## Security Considerations

### Input Validation

- Path operations validate for path traversal
- HTTP client validates URLs
- File operations check permissions before acting

### Secure Defaults

- HTTP uses HTTPS by default
- File operations respect umask
- Temp files created with secure permissions

## Testing Strategy

### Unit Tests

- Each core class has corresponding test file
- Test both success and error paths
- Cross-platform compatibility tests
- Edge cases (empty strings, null, large files)

### Integration Tests

- End-to-end workflows
- Multiple components working together
- Example applications as integration tests

### Test Coverage Goals

- Core classes: 90%+ coverage
- Utilities: 80%+ coverage
- Type extensions: 100% coverage

## Future Design Considerations

### Async/Await Support

Add async versions of I/O operations:
```typescript
class Path {
    readString(): string         // Existing sync
    readStringAsync(): Promise<string>  // Future async
}
```

### Stream Performance

Consider stream-based file reading for large files:
```typescript
path.createReadStream().pipe(destination)
```

### Module System

Support additional modules as separate packages:
- @ejsx/template - Template engine
- @ejsx/mail - Email support
- @ejsx/zlib - Compression
- @ejsx/tar - Archive operations

## References

- [Ejscript Documentation](https://www.embedthis.com/ejscript/)
- [Bun Documentation](https://bun.sh/docs)
- [TypeScript Handbook](https://www.typescriptlang.org/docs/)
