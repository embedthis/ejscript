# Ejscript Documentation

Comprehensive documentation for Ejscript for Bun - a complete TypeScript implementation of the Ejscript core API.

## Documentation Index

### Getting Started

- **[Quick Start Guide](../README.md#quick-start)** - Get up and running quickly
- **[Installation](../README.md#installation)** - Installation instructions
- **[Local Development Linking](LINKING.md)** - Using ejscript in other projects during development
- **[Compatibility Guide](COMPATIBILITY.md)** - **Complete differences & incompatibilities** with native Ejscript
- **[Migration Guide](#migration-from-native-ejscript)** - Migrating from native Ejscript

### Core Documentation

- **[API Reference](API.md)** - Complete API documentation for all classes and methods
- **[Examples](EXAMPLES.md)** - Practical code examples and common patterns
- **[Type Extensions](API.md#type-extensions)** - String, Array, Object, Date extensions

### Detailed Guides

#### Core Classes
- [Path](API.md#path) - Path manipulation and file operations (80+ methods)
- [File](API.md#file) - File I/O with Stream interface
- [App](API.md#app) - Application singleton (args, env, I/O)
- [Http](API.md#http) - Full HTTP/HTTPS client with streaming

#### Streams
- [ByteArray](API.md#bytearray) - Growable byte buffer
- [TextStream](API.md#textstream) - Text I/O wrapper
- [BinaryStream](API.md#binarystream) - Binary I/O with endian control
- [Stream](API.md#stream) - Base stream interface

#### Utilities
- [Logger](API.md#logger) - Multi-level logging system
- [Cache](API.md#cache) - In-memory caching with TTL
- [Timer](API.md#timer) - Timers with callbacks
- [Cmd](API.md#cmd) - Command execution
- [Uri](API.md#uri) - URI parsing and manipulation

#### Networking
- [Socket](API.md#socket) - TCP/UDP networking
- [WebSocket](API.md#websocket) - WebSocket client

### Advanced Topics

- **[HTTP Streaming](API.md#streaming-support)** - Stream large uploads/downloads
- **[Authentication](API.md#authentication)** - Basic and Digest auth
- **[Binary Data](API.md#bytearray)** - Working with binary data
- **[Async Operations](API.md#async-operations)** - Async patterns

### Reference

- **[Architecture](../doc/architecture/system.md)** - System architecture and design
- **[Changelog](../doc/sessions/CHANGELOG.md)** - Version history and changes
- **[Project Map](../doc/MAP.md)** - Documentation entry point and current status

---

## Quick Links

### Common Tasks

- [Reading Files](EXAMPLES.md#reading-files)
- [Writing Files](EXAMPLES.md#writing-files)
- [HTTP Requests](EXAMPLES.md#basic-requests)
- [JSON API Calls](EXAMPLES.md#json-api-calls)
- [Command Execution](EXAMPLES.md#simple-commands)
- [Directory Operations](EXAMPLES.md#directory-operations)

### API Quick Reference

```typescript
// Import what you need
import { Path, File, Http, App, Logger } from '@embedthis/ejscript'

// File I/O
const content = new Path('/tmp/file.txt').readString()
new Path('/tmp/output.txt').write('Hello World')

// HTTP request
const http = new Http()
http.get('https://api.example.com/data')
await http.wait()
const data = JSON.parse(http.response)

// Command execution
const cmd = new Cmd('ls -la')
console.log(cmd.response)

// Logging
const log = new Logger('MyApp')
log.info('Application started')
```

---

## Migration from Native Ejscript

Ejscript for Bun maintains **99%+ API compatibility** with native Ejscript. The only required change is adding ES6 import statements.

### Before (Native Ejscript)

```javascript
// No imports needed in native Ejscript
let path = new Path('/tmp/test.txt')
path.write('Hello')

let http = new Http()
http.get('https://example.com')
```

### After (Ejscript for Bun)

```typescript
// Add ES6 imports
import { Path, Http } from '@embedthis/ejscript'

let path = new Path('/tmp/test.txt')
path.write('Hello')

let http = new Http()
http.get('https://example.com')  // Returns Http object
await http.wait()  // Wait for request to complete
```

### Key Differences

1. **Imports Required**
   - Native: No imports, global namespace
   - Bun: ES6 imports required

2. **HTTP Methods Return Http Objects**
   - Native: `http.get()` - synchronous, no return
   - Bun: `http.get()` returns `Http`, then `await http.wait()`

3. **File I/O is Synchronous**
   - Both versions use synchronous file I/O
   - Async file operations are a future enhancement

4. **Type Safety**
   - Native: Dynamic typing
   - Bun: Full TypeScript type safety available

### Breaking Changes

**None!** All APIs work identically once imports are added.

### New Features in Bun Version

1. **HTTP Streaming** - Stream large uploads/downloads
2. **File.encoding** - Dynamic encoding changes
3. **Path.open()** - Open files directly from Path
4. **Better Error Messages** - Clear, actionable errors
5. **TypeScript Support** - Full type safety

---

## Project Structure

```
ejs/
├── src/                    # Source code
│   ├── index.ts           # Main exports
│   ├── globals.ts         # Global namespace
│   ├── core/              # Core classes
│   │   ├── App.ts
│   │   ├── Path.ts
│   │   ├── File.ts
│   │   ├── Http.ts
│   │   ├── streams/       # Stream classes
│   │   ├── utilities/     # Utility classes
│   │   ├── async/         # Async classes
│   │   └── types/         # Type extensions
│   └── ...
├── test/                  # Test suite
├── docs/                  # This documentation
│   ├── README.md         # This file
│   ├── API.md            # API reference
│   ├── EXAMPLES.md       # Code examples
│   └── examples/         # Example code
└── doc/                   # Project documentation
    ├── MAP.md             # Navigation entry point
    ├── overview/          # Product overview and roadmap
    ├── architecture/      # Design docs
    ├── features/          # Feature units
    ├── operations/        # Development procedures
    └── sessions/          # Changelog and session logs
```

---

## Documentation Standards

All public APIs are documented with:

- **JSDoc comments** - In source code
- **Type definitions** - TypeScript types
- **Usage examples** - Code samples
- **Parameter descriptions** - All parameters documented
- **Return value descriptions** - What methods return

### Example Documentation

```typescript
/**
 * Read entire file as string
 * @param count Number of bytes to read (-1 for all)
 * @returns File contents as string, or null on error
 * @example
 * const content = file.readString()
 * const chunk = file.readString(1024)  // Read 1KB
 */
readString(count: number = -1): string | null
```

---

## Contributing Documentation

To improve documentation:

1. Keep examples practical and tested
2. Update API docs when adding features
3. Include code samples for all public methods
4. Document edge cases and limitations
5. Keep changelog up to date

---

## Support and Resources

### Internal Resources

- **Source Code**: `src/` directory
- **Tests**: `test/` directory - comprehensive test coverage
- **Examples**: `docs/examples/` directory - working code samples
- **Design Docs**: `doc/architecture/` - architecture documentation

### External Resources

- **Bun Documentation**: https://bun.sh/docs
- **TypeScript**: https://www.typescriptlang.org/docs
- **Original Ejscript**: Referenced in parent directories

### Getting Help

1. Check [API Reference](API.md) for method documentation
2. Review [Examples](EXAMPLES.md) for usage patterns
3. Search test suite for edge cases
4. Check [Changelog](../doc/sessions/CHANGELOG.md) for recent changes

---

## Version History

- **v1.0.0** (2025-10-26) - Initial release
  - Complete core API implementation
  - 1279/1289 tests passing (99.2%)
  - HTTP streaming support
  - All unimplemented methods completed

See [CHANGELOG.md](../doc/sessions/CHANGELOG.md) for detailed version history.

---

*Last Updated: 2025-10-26*
*Version: 1.0.0*
