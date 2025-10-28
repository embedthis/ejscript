# Ejscript API Reference

Complete API documentation for Ejscript for Bun - A TypeScript implementation of the Ejscript core API.

## Table of Contents

- [Core Classes](#core-classes)
  - [Path](#path) - Path manipulation and file operations
  - [File](#file) - File I/O with Stream interface
  - [App](#app) - Application singleton
  - [Http](#http) - HTTP/HTTPS client
- [Streams](#streams)
  - [Stream](#stream) - Base stream interface
  - [ByteArray](#bytearray) - Growable byte buffer
  - [TextStream](#textstream) - Text I/O wrapper
  - [BinaryStream](#binarystream) - Binary I/O wrapper
- [Utilities](#utilities)
  - [Logger](#logger) - Multi-level logging
  - [Cache](#cache) - In-memory caching with TTL
  - [Timer](#timer) - Timers with callbacks
  - [Cmd](#cmd) - Command execution
  - [Uri](#uri) - URI parsing and manipulation
- [Networking](#networking)
  - [Socket](#socket) - TCP/UDP sockets
  - [WebSocket](#websocket) - WebSocket client
- [Type Extensions](#type-extensions)
  - [String Extensions](#string-extensions)
  - [Array Extensions](#array-extensions)
  - [Object Extensions](#object-extensions)
  - [Date Extensions](#date-extensions)

---

## Core Classes

### Path

Path manipulation and file system operations. Immutable path objects with 80+ methods.

#### Constructor

```typescript
new Path(path: string | Path)
```

**Examples:**
```typescript
const p = new Path('/tmp/test.txt')
const rel = new Path('docs/readme.md')
const home = new Path('~/documents')
```

#### Properties

| Property | Type | Description |
|----------|------|-------------|
| `name` | `string` | Complete pathname |
| `absolute` | `Path` | Absolute path |
| `basename` | `string` | Filename without directory |
| `dirname` | `string` | Parent directory path |
| `extension` | `string` | File extension (e.g., '.txt') |
| `isAbsolute` | `boolean` | True if path is absolute |
| `isDir` | `boolean` | True if path is a directory |
| `isLink` | `boolean` | True if path is a symbolic link |
| `length` | `number` | File size in bytes |
| `mimeType` | `string` | MIME type based on extension |
| `portable` | `string` | Path with forward slashes |

#### File Operations

```typescript
// Reading
readString(): string | null          // Read entire file as string
readLines(): string[]                // Read file as array of lines
readJSON(): any                      // Parse file as JSON

// Writing
write(...data: any[]): number        // Write data to file
append(...data: any[]): number       // Append data to file
writeLines(lines: string[]): void    // Write array as lines

// File Management
copy(destination: Path): void        // Copy file
move(destination: Path): void        // Move/rename file
remove(): boolean                    // Delete file
rename(newName: string): Path        // Rename file, return new Path
```

#### Directory Operations

```typescript
makeDir(): void                      // Create directory
files(pattern?: string): Path[]      // List files in directory
find(glob: string): Path[]           // Find files matching glob pattern
```

#### Path Manipulation

```typescript
join(...paths: string[]): Path       // Join path components
joinExt(ext: string): Path          // Add extension
replaceExt(ext: string): Path       // Replace extension
trimExt(): Path                     // Remove extension
relative(to: Path): Path            // Get relative path
normalize(): Path                   // Normalize path separators
```

#### File Information

```typescript
exists: boolean                      // File exists
created: Date | null                // Creation time
modified: Date | null               // Last modified time
permissions: number                 // File permissions (Unix)
size: number                        // File size in bytes
```

#### Examples

```typescript
// File operations
const file = new Path('/tmp/data.txt')
file.write('Hello World')
const content = file.readString()

// Directory operations
const dir = new Path('/tmp/mydir')
dir.makeDir()
const files = dir.files('*.txt')

// Path manipulation
const p = new Path('/usr/local/bin/node')
p.basename        // 'node'
p.dirname         // '/usr/local/bin'
p.extension       // ''
p.trimExt()       // Path('/usr/local/bin/node')
```

---

### File

File I/O with Stream interface. Provides synchronous file operations.

#### Constructor

```typescript
new File(path: string | Path, options?: FileOptions | string)
```

**Options:**
- `mode`: 'r' (read), 'w' (write), 'a' (append), 'r+', 'w+', 'a+'
- `permissions`: Unix file permissions (e.g., 0o644)

**Examples:**
```typescript
const file = new File('/tmp/test.txt', { mode: 'w' })
const readFile = new File('/tmp/data.txt', 'r')
```

#### Properties

| Property | Type | Description |
|----------|------|-------------|
| `encoding` | `string` | Text encoding (utf-8, ascii, latin1, etc.) |
| `canRead` | `boolean` | File is readable |
| `canWrite` | `boolean` | File is writable |
| `position` | `number` | Current file position |

#### Methods

```typescript
// Opening/Closing
open(options?: FileOptions | string): void
close(): void

// Reading
read(buffer: Uint8Array, offset?: number, count?: number): number
readString(count?: number): string | null
readLines(): string[] | null
readBytes(count?: number): Uint8Array | null

// Writing
write(...items: any[]): number       // Write data (strings, bytes, objects)
writeByte(value: number): void      // Write single byte

// Position
seek(position: number, whence?: number): number
tell(): number

// Encoding
encoding: string                     // Get/set encoding (utf-8, ascii, etc.)
```

#### Stream Interface

File implements the Stream interface:

```typescript
flush(direction?: number): void
available: number                    // Bytes available to read
```

#### Examples

```typescript
// Write text file
const file = new File('/tmp/output.txt', 'w')
file.write('Line 1\n')
file.write('Line 2\n')
file.close()

// Read with encoding
const file = new File('/tmp/data.txt', 'r')
file.encoding = 'utf-8'
const content = file.readString()
file.close()

// Binary read/write
const binFile = new File('/tmp/data.bin', 'w+')
binFile.write(new Uint8Array([0x01, 0x02, 0x03]))
binFile.seek(0)
const bytes = binFile.readBytes(3)
binFile.close()
```

---

### App

Application singleton providing access to application-wide state.

#### Properties

| Property | Type | Description |
|----------|------|-------------|
| `args` | `string[]` | Command line arguments |
| `dir` | `Path` | Current working directory |
| `env` | `Object` | Environment variables |
| `exeDir` | `Path` | Executable directory |
| `exePath` | `Path` | Executable path |
| `pid` | `number` | Process ID |
| `stdin` | `Stream` | Standard input stream |
| `stdout` | `Stream` | Standard output stream |
| `stderr` | `Stream` | Standard error stream |

#### Methods

```typescript
chdir(path: Path | string): void     // Change working directory
exit(status: number): void           // Exit process
getenv(name: string): string | undefined
putenv(name: string, value: string): void
sleep(ms: number): void             // Sleep milliseconds
```

#### Examples

```typescript
import { App } from 'ejscript'

// Access arguments
console.log('Args:', App.args)

// Environment variables
const home = App.getenv('HOME')
App.putenv('MY_VAR', 'value')

// Working directory
console.log('CWD:', App.dir.name)
App.chdir('/tmp')

// Process info
console.log('PID:', App.pid)
console.log('Exe:', App.exePath.name)
```

---

### Http

Full-featured HTTP/HTTPS client with streaming support, authentication, and more.

#### ⚠️ Important: Method Chaining Pattern

HTTP methods (`get()`, `post()`, etc.) **return `Http` objects for method chaining**, not Promises. To wait for requests to complete, call `await http.wait()` or `await http.finalize()` after the method call.

**Correct Usage:**
```typescript
// Pattern 1: Call method, then wait
http.get(url)
await http.wait()

// Pattern 2: Method chaining
http.setHeader('Auth', token).get(url)
await http.finalize()

// Pattern 3: Configure, call, wait
http.followRedirects = true
http.get(url)
await http.wait()
```

**Incorrect Usage:**
```typescript
await http.get(url)  // ❌ WRONG - get() returns Http, not Promise
```

#### Constructor

```typescript
new Http(uri?: Uri | string)
```

#### Properties

| Property | Type | Description |
|----------|------|-------------|
| `uri` | `Uri \| null` | Request URI |
| `method` | `string` | HTTP method (GET, POST, etc.) |
| `status` | `number \| null` | Response status code |
| `statusMessage` | `string` | Response status message |
| `response` | `string` | Response body |
| `contentType` | `string` | Response content type |
| `contentLength` | `number` | Response content length |
| `headers` | `Object` | Response headers |
| `followRedirects` | `boolean` | Auto-follow redirects |
| `verify` | `boolean` | Verify SSL certificates |

#### HTTP Methods

All methods return `Http` objects for chaining. Use `await http.wait()` to complete.

```typescript
get(uri?: Uri | string, ...data: any[]): Http
post(uri?: Uri | string, ...data: any[]): Http
put(uri?: Uri | string, ...data: any[]): Http
del(uri?: Uri | string): Http
head(uri?: Uri | string): Http
```

#### Specialized Methods

```typescript
// Form data
form(uri: Uri | string, data: Record<string, any>): Http
jsonForm(uri: Uri | string, ...data: any[]): Http

// File upload
upload(uri: string | Uri, files: any, fields?: Record<string, any>): Http

// Static fetch (returns Promise directly - no need for wait())
static async fetch(uri: Uri | string, method?: string, ...data: any[]): Promise<string>
```

#### Async Completion Methods

```typescript
// Wait for request to complete
async wait(timeout?: number): Promise<boolean>

// Finalize request (completes any pending writes)
async finalize(): Promise<void>
```

#### Headers

```typescript
setHeader(key: string, value: string | number): void
getRequestHeaders(): Record<string, string>
header(key: string): string | null     // Get response header
```

#### Authentication

```typescript
setCredentials(username: string, password: string, type?: string): void
```

**Auth types:**
- `'basic'` - HTTP Basic authentication
- `'digest'` - HTTP Digest authentication (RFC 2617/7616)
- `undefined` - Auto-detect (server determines via 401 response)

#### Streaming Support

```typescript
// Stream with ReadableStream
const stream = new ReadableStream({...})
http.post(uri, stream)
await http.wait()

// Incremental write
http.method = 'POST'
http.uri = uri
http.write('chunk1')
http.write('chunk2')
await http.finalize()

// File streaming
http.post(uri, Bun.file('/path/to/file').stream())
await http.wait()
```

#### Response Reading

```typescript
readString(count?: number): string | null
readLines(count?: number): string[] | null
```

#### Examples

```typescript
import { Http } from 'ejscript'

// Simple GET (method chaining pattern)
const http = new Http()
http.get('https://api.github.com/users/octocat')
await http.wait()  // Wait for request to complete
console.log(http.status, http.response)

// POST JSON
http.jsonForm('https://api.example.com/data', { key: 'value' })
await http.wait()

// Authentication (auto-detect)
http.setCredentials('user', 'pass')
http.get('https://example.com/protected')
await http.wait()

// Streaming upload
const stream = Bun.file('large-file.bin').stream()
http.post('https://example.com/upload', stream)
await http.wait()

// Custom headers with method chaining
http.setHeader('X-API-Key', 'secret')
http.setHeader('Content-Type', 'application/json')
http.post('https://api.example.com', JSON.stringify(data))
await http.wait()
```

#### HTTP Status Constants

```typescript
Http.Ok                 // 200
Http.Created            // 201
Http.NoContent          // 204
Http.MovedPermanently   // 301
Http.NotModified        // 304
Http.BadRequest         // 400
Http.Unauthorized       // 401
Http.Forbidden          // 403
Http.NotFound           // 404
Http.ServerError        // 500
// ... and more
```

---

## Streams

### Stream

Base interface for all stream types.

```typescript
interface Stream {
    available: number
    close(): void
    flush(direction?: number): void
    read(buffer: Uint8Array, offset?: number, count?: number): number | null
    write(...data: any[]): number
}
```

**Constants:**
```typescript
Stream.READ = 1
Stream.WRITE = 2
Stream.BOTH = 3
```

---

### ByteArray

Growable byte buffer for binary data. Extends Uint8Array.

#### Constructor

```typescript
new ByteArray(size?: number, growable?: boolean)
```

**Note:** ByteArray extends Uint8Array which has fixed size. Allocate sufficient initial size.

#### Properties

| Property | Type | Description |
|----------|------|-------------|
| `length` | `number` | Current buffer size |
| `available` | `number` | Bytes available to read |
| `readPosition` | `number` | Current read position |
| `writePosition` | `number` | Current write position |
| `room` | `number` | Space available for writing |

#### Reading Methods

```typescript
read(dest: ByteArray, offset?: number, count?: number): number
readByte(): number
readShort(): number
readInteger32(): number
readLong(): bigint
readDouble(): number
readString(count?: number): string
```

#### Writing Methods

```typescript
write(data: Uint8Array | string): number
writeByte(value: number): void
writeShort(value: number): void
writeInteger32(value: number): void
writeLong(value: bigint): void
writeDouble(value: number): void
writeString(str: string, encoding?: string): number
```

#### Utility Methods

```typescript
compact(): void                      // Shift data to beginning
flush(): void                        // Reset and clear
toString(): string                   // Convert to string
toArray(): Uint8Array               // Convert to Uint8Array
compress(): ByteArray               // Gzip compress
uncompress(): ByteArray             // Gzip uncompress
```

#### Examples

```typescript
import { ByteArray } from 'ejscript'

// Create and write
const ba = new ByteArray(1024)
ba.write('Hello ')
ba.write(new Uint8Array([87, 111, 114, 108, 100]))  // 'World'
console.log(ba.toString())  // 'Hello World'

// Binary data
const bin = new ByteArray(1024)
bin.writeInteger32(42)
bin.writeDouble(3.14159)
bin.readPosition = 0
const num = bin.readInteger32()  // 42
const pi = bin.readDouble()       // 3.14159

// Compression
const compressed = ba.compress()
const original = compressed.uncompress()
```

---

### TextStream

Text stream wrapper providing line-oriented I/O.

#### Constructor

```typescript
new TextStream(stream: Stream)
```

#### Methods

```typescript
readLine(): string | null            // Read single line
readLines(): string[]                // Read all lines
readString(count?: number): string | null
write(...data: any[]): number
```

#### Examples

```typescript
import { TextStream, File } from 'ejscript'

const file = new File('/tmp/data.txt', 'r')
const stream = new TextStream(file)

const line = stream.readLine()
const allLines = stream.readLines()
file.close()
```

---

### BinaryStream

Binary stream wrapper with endianness control.

#### Constructor

```typescript
new BinaryStream(stream: Stream, mode?: number)
```

**Modes:**
- `BinaryStream.LITTLE_ENDIAN` (default)
- `BinaryStream.BIG_ENDIAN`

#### Methods

```typescript
readInteger(): number
readLong(): bigint
readDouble(): number
writeInteger(value: number): void
writeLong(value: bigint): void
writeDouble(value: number): void
```

---

## Utilities

### Logger

Multi-level logging system.

#### Constructor

```typescript
new Logger(name?: string)
```

#### Properties

```typescript
level: number                        // Current log level
```

**Levels:**
```typescript
Logger.OFF = 0
Logger.ERROR = 1
Logger.WARN = 2
Logger.INFO = 3
Logger.DEBUG = 4
Logger.TRACE = 5
```

#### Methods

```typescript
error(message: string, ...args: any[]): void
warn(message: string, ...args: any[]): void
info(message: string, ...args: any[]): void
debug(message: string, ...args: any[]): void
trace(message: string, ...args: any[]): void
```

#### Examples

```typescript
import { Logger } from 'ejscript'

const log = new Logger('MyApp')
log.level = Logger.DEBUG

log.info('Application started')
log.debug('Processing data:', data)
log.error('Error occurred:', error)
```

---

### Cache

In-memory caching with TTL support.

#### Constructor

```typescript
new Cache(options?: { lifespan?: number, resolution?: number })
```

**Options:**
- `lifespan`: Entry lifetime in milliseconds
- `resolution`: Cleanup interval in milliseconds

#### Methods

```typescript
read(key: string): any               // Get cached value
write(key: string, value: any, lifespan?: number): void
remove(key: string): void
expire(key: string): void            // Mark as expired
```

#### Examples

```typescript
import { Cache } from 'ejscript'

const cache = new Cache({ lifespan: 60000 })  // 60 second TTL

cache.write('user:123', userData)
const user = cache.read('user:123')
cache.remove('user:123')
```

---

### Timer

Timers with callback support.

#### Constructor

```typescript
new Timer(period: number, callback?: Function, ...args: any[])
```

#### Methods

```typescript
start(): void                        // Start/restart timer
stop(): void                        // Stop timer
restart(period?: number): void       // Restart with new period
```

#### Examples

```typescript
import { Timer } from 'ejscript'

// One-shot timer
const timer = new Timer(1000, () => {
    console.log('Timer fired!')
})

// Periodic timer
const periodic = new Timer(5000, (arg) => {
    console.log('Periodic:', arg)
}, 'data')
periodic.start()

// Stop timer
periodic.stop()
```

---

### Cmd

Command execution with output capture.

#### Constructor

```typescript
new Cmd(command?: string | string[], options?: CmdOptions)
```

**Options:**
- `detach`: Run detached
- `dir`: Working directory
- `exceptions`: Throw on non-zero exit
- `timeout`: Execution timeout (ms)

#### Properties

```typescript
status: number | null                // Exit status
response: string | null              // stdout
error: string | null                 // stderr
pid: number | null                   // Process ID
```

#### Methods

```typescript
start(command: string | string[], options?: CmdOptions): void
wait(): Promise<number>              // Wait for completion
finalize(): void                     // Send EOF to stdin
kill(signal?: number): void          // Kill process
```

#### Static Methods

```typescript
static locate(program: string): Path  // Find program in PATH
static run(command: string | string[], input?: string): string
static sh(script: string): string     // Run shell script
```

#### Examples

```typescript
import { Cmd } from 'ejscript'

// Simple command
const cmd = new Cmd('ls -la')
console.log(cmd.status, cmd.response)

// With options
const git = new Cmd('git status', { dir: '/tmp/myrepo' })

// Locate program
const node = Cmd.locate('node')
console.log(node.name)

// Run shell script
const output = Cmd.sh('echo "Hello" | tr a-z A-Z')
```

---

### Uri

URI parsing and manipulation.

#### Constructor

```typescript
new Uri(uri: string)
```

#### Properties

```typescript
scheme: string                       // e.g., 'https'
host: string                        // Hostname
port: number                        // Port number
path: string                        // Path component
query: string                       // Query string
fragment: string                    // Fragment (#hash)
```

#### Static Methods

```typescript
static decode(str: string): string   // URL decode
static encode(str: string): string   // URL encode
static encodeQuery(params: Record<string, any>): string
static decodeQuery(query: string): Record<string, string>
```

#### Examples

```typescript
import { Uri } from 'ejscript'

const uri = new Uri('https://example.com:8080/path?key=value#section')
console.log(uri.scheme)    // 'https'
console.log(uri.host)      // 'example.com'
console.log(uri.port)      // 8080
console.log(uri.path)      // '/path'

// Query encoding
const query = Uri.encodeQuery({ name: 'John Doe', age: 30 })
// 'name=John%20Doe&age=30'
```

---

## Networking

### Socket

TCP/UDP socket support.

#### Constructor

```typescript
new Socket()
```

#### Methods

```typescript
connect(host: string, port: number): void
listen(host: string, port: number, callback: Function): void
close(): void
read(buffer: Uint8Array, offset?: number, count?: number): number
write(...data: any[]): number
```

---

### WebSocket

WebSocket client.

#### Constructor

```typescript
new WebSocket(uri: string)
```

#### Methods

```typescript
on(event: string, handler: Function): void
send(data: any): void
close(code?: number, reason?: string): void
```

**Events:** `open`, `message`, `error`, `close`

#### Examples

```typescript
import { WebSocket } from 'ejscript'

const ws = new WebSocket('wss://echo.websocket.org')

ws.on('open', () => {
    console.log('Connected')
    ws.send('Hello WebSocket')
})

ws.on('message', (data) => {
    console.log('Received:', data)
})
```

---

## Type Extensions

### String Extensions

Extended methods on String.prototype:

```typescript
// Case conversion
toPascal(): string                   // 'hello_world' -> 'HelloWorld'
toCamel(): string                    // 'hello_world' -> 'helloWorld'
toTitle(): string                    // 'hello world' -> 'Hello World'

// Trimming
trim(): string
trimStart(): string
trimEnd(): string

// Path operations
expand(): string                     // Expand ~ and environment variables
portable(): string                   // Convert to forward slashes

// Utilities
contains(pattern: string): boolean
startsWith(prefix: string): boolean
endsWith(suffix: string): boolean
```

#### Examples

```typescript
'hello_world'.toPascal()             // 'HelloWorld'
'hello-world'.toCamel()              // 'helloWorld'
'~/documents'.expand()               // '/Users/user/documents'
```

---

### Array Extensions

Extended methods on Array.prototype:

```typescript
contains(value: any): boolean        // Check if array contains value
unique(): any[]                      // Remove duplicates
clone(): any[]                       // Shallow copy
compact(): any[]                     // Remove null/undefined
remove(value: any): any[]           // Remove value from array
append(...items: any[]): any[]      // Append items
clear(): any[]                      // Remove all elements
```

#### Examples

```typescript
[1, 2, 3].contains(2)                // true
[1, 2, 2, 3].unique()               // [1, 2, 3]
[1, null, 2, undefined].compact()    // [1, 2]
[1, 2, 3].remove(2)                  // [1, 3]
```

---

### Object Extensions

Extended methods on Object:

```typescript
Object.clone(obj: any): any          // Deep clone
Object.getType(obj: any): string     // Get type name
Object.blend(dest: any, src: any): any  // Merge objects
```

#### Examples

```typescript
const clone = Object.clone({ a: 1, b: { c: 2 } })
Object.getType([1, 2, 3])            // 'Array'
Object.blend({ a: 1 }, { b: 2 })     // { a: 1, b: 2 }
```

---

### Date Extensions

Extended methods on Date.prototype:

```typescript
format(fmt?: string): string         // Format date
elapsed(): number                    // Milliseconds since epoch
future(delay: number): Date         // Date in future
past(delay: number): Date           // Date in past
```

#### Examples

```typescript
new Date().format()                  // '2025-10-26'
new Date().format('yyyy-MM-dd HH:mm:ss')
new Date().elapsed()                 // Time since epoch
new Date().future(60000)             // 1 minute from now
```

---

## Quick Reference

### Common Patterns

#### File I/O

```typescript
// Read text file
const content = new Path('/tmp/file.txt').readString()

// Write text file
new Path('/tmp/output.txt').write('Hello World')

// Read JSON
const data = new Path('/tmp/data.json').readJSON()

// Write JSON
new Path('/tmp/data.json').write(JSON.stringify(obj, null, 2))
```

#### HTTP Requests

```typescript
// GET request
const http = new Http()
http.get('https://api.example.com/data')
await http.wait()
const data = JSON.parse(http.response)

// POST JSON
http.jsonForm('https://api.example.com', { key: 'value' })
await http.wait()

// Upload file
http.upload('https://api.example.com/upload', {
    file: '/tmp/upload.pdf'
}, { description: 'My file' })
await http.wait()
```

#### Command Execution

```typescript
// Simple command
const output = Cmd.run('ls -la')

// With error handling
const cmd = new Cmd('git status')
if (cmd.status === 0) {
    console.log(cmd.response)
} else {
    console.error(cmd.error)
}
```

#### Logging

```typescript
const log = new Logger('MyApp')
log.level = Logger.INFO
log.info('Starting application')
log.error('Error occurred:', error)
```

---

## Migration from Native Ejscript

The only change needed is adding imports:

**Before (native Ejscript):**
```javascript
let path = new Path('/tmp/test.txt')
```

**After (Ejscript for Bun):**
```typescript
import { Path } from 'ejscript'
let path = new Path('/tmp/test.txt')
```

All APIs remain compatible!

---

## Performance Notes

- File operations use Bun's native synchronous APIs (fast)
- HTTP uses fetch() (optimized in Bun)
- Path operations are lightweight (no filesystem access unless needed)
- ByteArray uses JavaScript Uint8Array for efficient binary data
- Timer uses Bun's native timer implementation

---

## Limitations

1. **File I/O**: Currently synchronous only
2. **ByteArray**: Fixed size after construction (allocate sufficient size)
3. **Worker Threads**: Basic implementation

---

## See Also

- [README.md](../README.md) - Project overview
- [QUICK_START.md](../QUICK_START.md) - Getting started guide
- [Examples](../examples/) - Example code
- [Tests](../test/) - Comprehensive test suite

---

*Generated: 2025-10-26*
*Version: 1.0.0*
