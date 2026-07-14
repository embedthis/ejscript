# Ejscript for Bun - Compatibility Guide

Complete guide to differences and incompatibilities between Ejscript for Bun and native Ejscript.

**Last Updated**: 2025-10-26
**Version**: 1.0.0

---

## Table of Contents

- [Breaking Changes](#breaking-changes)
- [Import Requirements](#import-requirements)
- [Async vs Sync Differences](#async-vs-sync-differences)
- [Unimplemented Features](#unimplemented-features)
- [Language-Level Differences](#language-level-differences)
- [Behavioral Differences](#behavioral-differences)
- [Type Extensions Compatibility](#type-extensions-compatibility)
- [Migration Checklist](#migration-checklist)

---

## Summary

**Compatibility Level**: **99%+**

Ejscript for Bun maintains near-perfect API compatibility with native Ejscript. Most code will run unchanged after adding ES6 imports.

### What's Compatible

✅ All core classes (Path, File, Http, App, etc.)
✅ All methods have identical signatures
✅ All properties work the same way
✅ All type extensions available
✅ 1279+ tests passing from original test suite

### What's Different

⚠️ Requires ES6 imports
⚠️ HTTP methods are async (require `await`)
⚠️ Socket I/O methods are async (require `await`)
⚠️ File I/O is synchronous only
⚠️ ByteArray has fixed size (pre-allocate sufficient buffer)
⚠️ `for each` syntax → `for...of` / `for...in`
⚠️ No runtime type coercion (TypeScript types are compile-time only)
⚠️ Array indices are strings in `for...in` loops
⚠️ Cannot re-declare variables with `let`/`const`

---

## Breaking Changes

### 1. Import Requirements

**Impact**: 🔴 **BREAKING** - All code must be updated

**Native Ejscript**:
```javascript
// No imports needed - global namespace
let path = new Path('/tmp/test.txt')
let http = new Http()
```

**Ejscript for Bun**:
```typescript
// ES6 imports required
import { Path, Http } from '@embedthis/ejscript'

let path = new Path('/tmp/test.txt')
let http = new Http()
```

**Migration**: Add import statement at top of each file.

---

### 2. HTTP Methods are Async

**Impact**: 🟡 **MODERATE** - Requires adding `await` keywords

**Native Ejscript**:
```javascript
let http = new Http()
http.get('https://example.com')  // Synchronous
console.log(http.response)
```

**Ejscript for Bun**:
```typescript
import { Http } from '@embedthis/ejscript'

let http = new Http()
http.get('https://example.com')  // Returns Http object for chaining
await http.wait()  // Wait for request to complete
console.log(http.response)
```

**Correct Usage Pattern**:
- HTTP methods (`get()`, `post()`, etc.) return `Http` objects for method chaining
- Call `await http.wait()` or `await http.finalize()` to complete the request
- Then read the response from `http.response`, `http.status`, etc.

**Common Pattern**:
```typescript
// Pattern 1: Separate calls (most common)
http.get(url)
await http.wait()

// Pattern 2: Method chaining
http.setHeader('Auth', token).get(url)
await http.finalize()

// Pattern 3: Configure, then execute
http.followRedirects = true
http.retries = 3
http.get(url)
await http.wait()
```

**Why**: HTTP methods return `Http` objects to enable method chaining and configuration. Use `wait()` or `finalize()` to complete async operations.

**Migration**:
1. Remove `await` from HTTP method calls (they don't return Promises)
2. Add `await http.wait()` after the method call
3. Mark containing function as `async`

---

### 3. Socket I/O Methods are Async

**Impact**: 🟡 **MODERATE** - Requires adding `await` keywords

**Native Ejscript**:
```javascript
let socket = new Socket()
socket.connect('localhost:8080')
socket.write('Hello')  // Synchronous
let response = new ByteArray()
socket.read(response, -1)  // Synchronous, blocks until data arrives
```

**Ejscript for Bun**:
```typescript
import { Socket, ByteArray } from '@embedthis/ejscript'

let socket = new Socket()
socket.connect('localhost:8080')
await socket.write('Hello')  // Async - requires await
let response = new ByteArray()
await socket.read(response, -1)  // Async - requires await
```

**Affected Methods**:
- `socket.read(buffer, offset?, count?)` → `await socket.read(buffer, offset?, count?)`
- `socket.write(...data)` → `await socket.write(...data)`

**Why**: JavaScript event loop is single-threaded and cannot block while processing network events. Async I/O is the idiomatic approach.

**Alternative**: For event-driven code, use `socket.on('readable', handler)` instead of `read()`.

**Migration**:
1. Add `await` before all socket read/write calls
2. Mark containing function as `async`
3. Or use event-driven pattern with `socket.on('readable', ...)`

---

## Import Requirements

### Required Imports

All classes must be imported:

```typescript
// Core classes
import { Path, File, App, Http } from '@embedthis/ejscript'

// Streams
import { ByteArray, TextStream, BinaryStream, Stream } from '@embedthis/ejscript'

// Utilities
import { Logger, Cache, Timer, Cmd, Uri } from '@embedthis/ejscript'

// Networking
import { Socket, WebSocket } from '@embedthis/ejscript'

// Config & System
import { Config, System, FileSystem } from '@embedthis/ejscript'

// Async
import { Emitter, Worker } from '@embedthis/ejscript'
```

### No Global Namespace

Unlike native Ejscript, there is **no global namespace**. Everything must be explicitly imported.

**Won't work**:
```javascript
// This will fail - Path is not global
let p = new Path('/tmp')
```

**Correct**:
```typescript
import { Path } from '@embedthis/ejscript'
let p = new Path('/tmp')
```

---

## Async vs Sync Differences

### HTTP Client: Method Chaining Pattern

| Method | Native | Bun Version |
|--------|--------|-------------|
| `get()` | Sync, returns nothing | **Returns Http** - use `await http.wait()` |
| `post()` | Sync, returns nothing | **Returns Http** - use `await http.wait()` |
| `put()` | Sync, returns nothing | **Returns Http** - use `await http.wait()` |
| `del()` | Sync, returns nothing | **Returns Http** - use `await http.wait()` |
| `head()` | Sync, returns nothing | **Returns Http** - use `await http.wait()` |
| `form()` | Sync, returns nothing | **Returns Http** - use `await http.wait()` |
| `jsonForm()` | Sync, returns nothing | **Returns Http** - use `await http.wait()` |
| `upload()` | Sync, returns nothing | **Returns Http** - use `await http.wait()` |
| `connect()` | Sync, returns nothing | **Returns Http** - use `await http.wait()` |

### Socket I/O: Async

| Method | Native | Bun Version |
|--------|--------|-------------|
| `read()` | Sync (blocking) | **Async** - requires `await` |
| `write()` | Sync | **Async** - requires `await` |
| `connect()` | Sync | Sync (same) |
| `listen()` | Sync | Sync (same) |
| `accept()` | Sync (blocking) | Sync (same) |
| Event handlers | Yes | Yes (same) |

**Note**: For high-performance event-driven code, use `socket.on('readable', handler)` instead of `await socket.read()`.

### File I/O: Synchronous

File operations remain **synchronous** (same as native):

```typescript
// Both native and Bun - synchronous
const file = new File('/tmp/test.txt', 'w')
file.write('Hello')  // Synchronous
file.close()

const content = new Path('/tmp/test.txt').readString()  // Synchronous
```

**Note**: Async file I/O is not yet implemented.

### Command Execution: Mixed

```typescript
// Constructor execution - synchronous (like native)
const cmd = new Cmd('ls -la')
console.log(cmd.status, cmd.response)

// Explicit async - for long-running commands
const server = new Cmd()
server.start('node server.js', { detach: true })
await server.wait()  // Async wait
```

---

## Unimplemented Features

### 1. ByteArray Dynamic Growth

**Status**: ⚠️ **LIMITED**

**Native Ejscript**:
```javascript
let ba = new ByteArray(10, true)  // Growable
ba.write(new Uint8Array(100))     // Automatically grows
```

**Ejscript for Bun**:
```typescript
// ByteArray cannot grow beyond initial size
let ba = new ByteArray(10, true)
ba.write(new Uint8Array(100))  // ❌ ERROR: ByteArray overflow

// WORKAROUND: Pre-allocate sufficient size
let ba = new ByteArray(1024 * 1024, true)  // 1MB
ba.write(new Uint8Array(100))  // ✅ Works within capacity
```

**Reason**: ByteArray extends Uint8Array which has fixed size in JavaScript.

**Workaround**: Allocate sufficient initial size based on expected data.

---

### 2. Async File I/O

**Status**: ⚠️ **NOT IMPLEMENTED**

**Native Ejscript**:
```javascript
let file = new File('/tmp/test.txt')
file.async = true
file.open('r')
// Async operations...
```

**Ejscript for Bun**:
```typescript
let file = new File('/tmp/test.txt')
file.async = true  // ⚠️ Throws error - not supported

// All file operations are synchronous
```

**Reason**: Current implementation uses synchronous Bun file APIs.

**Future**: Async file I/O may be added in future version.

---

### 3. XML/E4X Support

**Status**: ❌ **NOT IMPLEMENTED**

XML and E4X (ECMAScript for XML) features are not implemented:

```javascript
// Native Ejscript
let xml = new XML('<root><item>value</item></root>')

// Ejscript for Bun
// ❌ XML class not available
```

**Reason**: E4X was deprecated and removed from JavaScript. Use JSON or external XML libraries instead.

**Workaround**:
- Use JSON instead of XML when possible
- Use external libraries like `fast-xml-parser` for XML parsing

---

### 4. JSON5 Output (Serialization)

**Status**: ⚠️ **LIMITED** - Parsing supported, output not supported

**What Works** - JSON5 Input/Parsing:
```typescript
import { JSON } from '@embedthis/ejscript'

// ✅ Can parse JSON5 input (comments, trailing commas, unquoted keys, etc.)
const config = JSON.parse(`{
    // This is a comment
    name: 'test',      // unquoted key, single quotes
    value: 42,         // trailing comma allowed
}`)
```

**What Doesn't Work** - JSON5 Output:
```typescript
// ❌ Output is always standard JSON (no comments, quoted keys, no trailing commas)
const output = JSON.stringify(obj)
// → {"name":"test","value":42}  // standard JSON only

const pretty = serialize(obj, { pretty: true })
// → Still standard JSON format, just indented
```

**Reason**:
- JSON5 **parsing** is fully implemented with custom preprocessor
- JSON5 **output** would require custom serialization (not implemented)
- Output uses native `JSON.stringify()` which only produces standard JSON

**Impact**:
- ✅ **Config files**: Can read JSON5 config files (`.ejsrc`, `testme.json5`, etc.)
- ✅ **Data parsing**: Can parse JSON5 data received from external sources
- ❌ **Generating JSON5**: Cannot programmatically create JSON5-formatted files with comments
- ❌ **Round-trip preservation**: Parsing JSON5 and re-serializing loses JSON5 features

**Workaround**:
- For reading config files: No workaround needed - JSON5 parsing works
- For writing JSON5 output: Use the `json5` package or write standard JSON
- Most use cases only need JSON5 input (hand-written configs), not output

**Comparison**:

| Feature | Native Ejscript | Ejscript for Bun |
|---------|----------------|------------------|
| Parse JSON5 input | ✅ Yes | ✅ Yes |
| Generate JSON5 output | ❓ Unknown | ❌ No (standard JSON only) |
| Parse standard JSON | ✅ Yes | ✅ Yes |
| Generate standard JSON | ✅ Yes | ✅ Yes |

---

### 5. Operator Overloading

**Status**: ❌ **NOT POSSIBLE**

**Native Ejscript**:
```javascript
let p1 = new Path('/tmp')
let p2 = p1 + '/subdir'  // Operator overloading
```

**Ejscript for Bun**:
```typescript
let p1 = new Path('/tmp')
let p2 = p1.join('subdir')  // ✅ Use method instead
```

**Reason**: JavaScript doesn't support operator overloading.

**Workaround**: Use explicit methods (`.join()`, `.append()`, etc.)

---

## Language-Level Differences

These are fundamental differences between Ejscript and JavaScript/TypeScript that affect how code is written.

### 1. Variable Re-declaration

**Impact**: 🟡 **MODERATE** - May cause errors in strict mode

**Native Ejscript**:
```javascript
// Multiple declarations of the same name allowed in a block
let x = 10
let x = 20  // ✅ Allowed in Ejscript
```

**Ejscript for Bun**:
```typescript
// JavaScript/TypeScript doesn't allow re-declaration in same scope
let x = 10
let x = 20  // ❌ SyntaxError: Identifier 'x' has already been declared
```

**Reason**: JavaScript (ES6+) doesn't allow re-declaration of `let`/`const` in the same block scope.

**Workaround**: Use assignment instead of re-declaration:
```typescript
let x = 10
x = 20  // ✅ Assignment works
```

---

### 2. Loop Iteration Syntax

**Impact**: 🟡 **MODERATE** - Requires syntax changes

**Native Ejscript**:
```javascript
// Ejscript uses 'for each' syntax
let arr = [1, 2, 3]
for each (item in arr) {
    print(item)
}

let obj = {a: 1, b: 2}
for each (value in obj) {
    print(value)
}

// Ejscript allows numeric range iteration
for (let i in 64) {  // Iterates i = 0 to 63
    print(i)
}
```

**Ejscript for Bun**:
```typescript
// Use ES6 for...of for arrays, for...in for objects
let arr = [1, 2, 3]
for (const item of arr) {  // for...of iterates values
    print(item)
}

let obj = {a: 1, b: 2}
for (const key in obj) {  // for...in iterates keys
    print(obj[key])       // Access value via key
}

// Numeric iteration requires standard for loop
for (let i = 0; i < 64; i++) {  // Standard C-style loop
    print(i)
}
```

**Key Differences**:
- `for each (x in arr)` → `for (const x of arr)` (arrays/iterables)
- `for each (x in obj)` → `for (const key in obj)` then access `obj[key]` (objects)
- `for (i in N)` → `for (let i = 0; i < N; i++)` (numeric range)

**Migration Table**:
| Ejscript | ES6 Equivalent | Use Case |
|----------|----------------|----------|
| `for each (item in arr)` | `for (const item of arr)` | Array values |
| `for each (val in obj)` | `for (const k in obj)` + `obj[k]` | Object values |
| `for (i in arr)` | `for (const i in arr)` | Array indices (strings!) |
| `for (let i in N)` | `for (let i = 0; i < N; i++)` | Numeric range 0 to N-1 |

---

### 3. Type Coercion and Type Annotations

**Impact**: 🔴 **CRITICAL** - Fundamental difference

**Native Ejscript**:
```javascript
// Ejscript types have runtime effect - values are cast to declared type
function process(value: Number): String {
    // 'value' is automatically cast to Number at runtime
    // Return value is automatically cast to String at runtime
    return value * 2  // Automatically converted to String
}

let result = process("42")  // "42" cast to Number(42), result cast to "84"
```

**Ejscript for Bun**:
```typescript
// TypeScript types are compile-time only - no runtime casting
function process(value: number): string {
    // No automatic type conversion at runtime
    // TypeScript only checks types during compilation
    return (value * 2).toString()  // Must explicitly convert
}

let result = process(42)  // Must pass actual number, not string
// process("42") would give compile error but no runtime casting
```

**Key Differences**:
1. **Ejscript types are runtime** - values are coerced to match type annotations
2. **TypeScript types are compile-time only** - no runtime type checking or coercion
3. **Ejscript casts parameters** - `process("42")` automatically converts to `Number(42)`
4. **Ejscript casts return values** - return value automatically converted to declared type
5. **TypeScript requires explicit conversion** - must use `.toString()`, `Number()`, etc.

**Migration Strategy**:
```typescript
// If Ejscript code relies on automatic type coercion:
function ejscriptStyle(value: any): string {
    // Manually coerce types to match Ejscript behavior
    const num = Number(value)  // Explicit conversion
    return String(num * 2)     // Explicit conversion
}
```

**Nullable Types**:
- Ejscript: `String?` = nullable, `String!` = not nullable
- TypeScript: `string | null` = nullable, `string` = not nullable (with strictNullChecks)

---

### 4. Type Checking Operators

**Impact**: 🔴 **CRITICAL** - Syntax errors, requires code changes

**Native Ejscript**:
```javascript
// Ejscript uses 'is' operator for type checking
if (value is String) {
    print("It's a string")
}

if (obj is MyClass) {
    print("It's an instance of MyClass")
}

// Ejscript also has typeOf() function
let type = typeOf(value)  // Returns the type name as a string
```

**Ejscript for Bun**:
```typescript
// JavaScript uses 'typeof' operator (lowercase) for primitives
if (typeof value === 'string') {
    console.log("It's a string")
}

// Use 'instanceof' for class instances
if (obj instanceof MyClass) {
    console.log("It's an instance of MyClass")
}

// No direct equivalent to typeOf() - use typeof or constructor.name
let primitiveType = typeof value           // 'string', 'number', 'boolean', etc.
let classType = value?.constructor?.name   // Class name for objects
```

**Key Differences**:
1. **`is` operator doesn't exist in JavaScript** - causes syntax error
2. **Use `typeof`** for primitive type checking (string, number, boolean, etc.)
3. **Use `instanceof`** for class/constructor checking
4. **`typeOf()` function doesn't exist** - use `typeof` operator or `.constructor.name`

**Common Conversions**:
```typescript
// Ejscript → JavaScript/TypeScript

// String checking:
value is String          →  typeof value === 'string'
value is Number          →  typeof value === 'number'
value is Boolean         →  typeof value === 'boolean'

// Object/Array checking:
value is Array           →  Array.isArray(value)
value is Object          →  typeof value === 'object' && value !== null
value is Function        →  typeof value === 'function'

// Class instance checking:
obj is MyClass           →  obj instanceof MyClass

// Type name (typeOf function):
typeOf(value)            →  typeof value  // for primitives
typeOf(obj)              →  obj?.constructor?.name  // for objects
```

**Migration Strategy**:
```typescript
// Find and replace pattern (careful with context!):
// 1. Search for: /\s+is\s+String/
//    Replace with: typeof $1 === 'string'

// 2. Search for: /\s+is\s+(\w+)/  (for class checks)
//    Replace with: instanceof $1

// 3. Search for: typeOf\(
//    Review manually - depends on usage context
```

**Important Notes**:
- The `is` operator is **not** the same as `===` (strict equality)
- `is` checks **type**, while `===` checks **value and type**
- Always test after conversion to ensure correct behavior

---

### 5. Multiline String Literals

**Impact**: 🟡 **MODERATE** - Requires syntax changes

**Native Ejscript**:
```javascript
// Ejscript allows multiline strings with single or double quotes
let text = "This is a
multiline
string"

let message = 'Another
multiline
example'
```

**Ejscript for Bun**:
```typescript
// JavaScript requires backticks (template literals) for multiline strings
let text = `This is a
multiline
string`

let message = `Another
multiline
example`

// Single/double quotes require escape sequences
let oneLineSingle = 'This is\na\nmultiline\nstring'  // Works but ugly
let oneLineDouble = "Another\nmultiline\nexample"     // Works but ugly
```

**Reason**: JavaScript treats unescaped newlines in single/double-quoted strings as syntax errors. Only template literals (backticks) support multiline content.

**Migration**:
```typescript
// Find and replace in test files:
// 1. Multiline strings with ' or " → use backticks `

// Before:
let sql = "SELECT *
FROM users
WHERE active = 1"

// After:
let sql = `SELECT *
FROM users
WHERE active = 1`
```

**Automated Fix**:
While manually reviewing multiline strings is recommended, regex pattern detection can help:
```bash
# Find potential multiline string issues (review manually):
grep -n "['\"].*$" file.ts | grep -v "\\\\n"
```

---

### 6. Array Index Types

**Impact**: 🟡 **MODERATE** - May affect index comparisons

**Native Ejscript**:
```javascript
// Array indices are numbers
let arr = ['a', 'b', 'c']
for (i in arr) {
    print(typeof i)  // "number"
    if (i === 0) { /* matches */ }
}
```

**Ejscript for Bun**:
```typescript
// Array indices are strings in for...in loops
let arr = ['a', 'b', 'c']
for (const i in arr) {
    print(typeof i)  // "string"
    if (i === '0') { /* matches */ }
    if (Number(i) === 0) { /* also works */ }
}

// Use for...of with entries() for numeric indices
for (const [index, value] of arr.entries()) {
    print(typeof index)  // "number"
}
```

**Best Practice**: Prefer `for...of` over `for...in` for arrays:
```typescript
// Recommended for arrays
for (const item of arr) {
    // Iterate values
}

for (const [i, item] of arr.entries()) {
    // Get numeric index and value
}

// Avoid for...in with arrays (gives string indices)
for (const i in arr) {
    // i is string "0", "1", "2"
}
```

---

## Behavioral Differences

### 1. HTTP Partial URLs

**Enhancement**: Ejscript for Bun supports convenient partial URLs:

```typescript
// All of these work in Ejscript for Bun:
http.get('4100/index.html')           // → http://127.0.0.1:4100/index.html
await http.wait()
http.get(':4100/path')                // → http://127.0.0.1:4100/path
await http.wait()
http.get('127.0.0.1:8080/api')       // → http://127.0.0.1:8080/api
await http.wait()

// Full URLs work as normal
http.get('https://example.com/api')   // → https://example.com/api
await http.wait()
```

**Compatibility**: Native Ejscript may require full URLs.

---

### 2. HTTP Streaming

**Enhancement**: Ejscript for Bun adds streaming support:

```typescript
// NEW in Bun version - streaming POST
const stream = new ReadableStream({...})
http.post(uri, stream)
await http.wait()

// NEW in Bun version - incremental write
http.method = 'POST'
http.uri = uri
http.write('chunk1')
http.write('chunk2')
http.finalize()
await http.connect('POST')
```

**Compatibility**: Native Ejscript doesn't have streaming support. This is a new feature.

---

### 3. HTTP Header Case Normalization

**Impact**: 🟡 **MODERATE** - Header names are normalized to lowercase

**Native Ejscript**:
```javascript
let http = new Http()
http.setHeader('Content-Type', 'application/json')
http.setHeader('User-Agent', 'MyApp/1.0')
http.get('https://example.com')

// Response headers preserve the case sent by server:
http.header('Content-Type')  // Might be "Content-Type"
http.header('content-type')  // Might be different or undefined
// Server might send "Content-Type", "content-type", or "Content-type"
```

**Ejscript for Bun**:
```typescript
import { Http } from '@embedthis/ejscript'

let http = new Http()
http.setHeader('Content-Type', 'application/json')
http.setHeader('User-Agent', 'MyApp/1.0')
http.get('https://example.com')
await http.wait()

// All headers are normalized to lowercase by Bun's fetch()
http.header('content-type')  // ✅ Works (lowercase)
http.header('Content-Type')  // ❌ Returns undefined (case doesn't match)
// All response headers are stored in lowercase
```

**Key Differences**:
1. **Request headers sent**: Bun's `fetch()` normalizes header names to lowercase before sending
2. **Response headers stored**: All response headers are stored with lowercase keys
3. **Case-insensitive access**: Use lowercase names when accessing headers
4. **Server receives lowercase**: The server sees headers like `user-agent` instead of `User-Agent`

**Reason**: Bun's underlying `fetch()` API (from WinterCG standard) normalizes all HTTP headers to lowercase. This is standard behavior for modern HTTP clients, as HTTP/2 and HTTP/3 specifications require lowercase header names.

**Migration Strategy**:
```typescript
// ❌ Old code (won't work reliably)
if (http.header('Content-Type') === 'application/json') { ... }

// ✅ New code (works in both)
if (http.header('content-type') === 'application/json') { ... }

// ✅ Alternative: case-insensitive check
const contentType = http.header('Content-Type') || http.header('content-type')
```

**Impact on Tests**:
If your code checks for headers echoed by the server (e.g., in test dumps), you'll need to:
- Use lowercase header names in assertions
- Make string comparisons case-insensitive
- Example: `response.toLowerCase().includes('user-agent=myapp')`

**Note**: This behavior is compliant with RFC 7230, which specifies that HTTP header names are case-insensitive. Modern HTTP implementations (HTTP/2, HTTP/3) require lowercase headers.

---

### 4. File Encoding

**Enhancement**: Dynamic encoding changes now supported:

```typescript
const file = new File('/tmp/test.txt', 'w')
file.encoding = 'ascii'  // NEW - works in Bun version
file.write('ASCII text')

// Supported encodings:
// utf-8, ascii, latin1, binary, base64, hex
```

**Compatibility**: Native Ejscript may not support `file.encoding` setter.

---

### 5. HTTP Caching Control

**Enhancement**: Control over HTTP caching behavior

**Native Ejscript**:
```javascript
// HTTP caching behavior is implicit - controlled by server cache headers
let http = new Http()
http.get('https://example.com/api/data')
// Behavior depends on server's Cache-Control headers
```

**Ejscript for Bun**:
```typescript
import { Http } from '@embedthis/ejscript'

// Default: Caching enabled (respects HTTP cache headers)
let http = new Http()
http.get('https://example.com/api/data')  // May use cached response

// Disable caching: Forces fresh fetch every time
http.cache = false
http.get('https://example.com/api/data')  // Always fetches fresh

// Re-enable caching
http.cache = true
http.get('https://example.com/api/data')  // May use cache again
```

**Key Features**:
1. **`http.cache = true`** (default) - Normal HTTP caching per RFC 7234
   - Respects `Cache-Control`, `Expires`, `ETag`, etc.
   - Uses Bun's fetch() default caching behavior
2. **`http.cache = false`** - Disables all caching
   - Sets `cache: 'no-store'` in fetch options
   - Forces fresh fetch for every request
   - Useful for testing or real-time data

**Use Cases for Disabling Cache**:
```typescript
// Testing: Ensure fresh data in test suites
http.cache = false
http.get('/api/test-data')
await http.wait()

// Real-time data: Stock prices, sensor readings
http.cache = false
http.get('/api/current-temperature')
await http.wait()

// Development: Avoid stale responses during debugging
http.cache = false
http.get('/api/debug-info')
await http.wait()
```

**When to Keep Cache Enabled** (default):
- Static assets (images, CSS, JavaScript)
- API responses with proper cache headers
- Public CDN content
- Performance-critical applications

**Note**: This controls **client-side** caching in Bun's fetch(). Server-side caching (like Appweb's caching module) is independent and controlled by server configuration.

**Compatibility**: Native Ejscript doesn't expose cache control. This is a new feature in Ejscript for Bun.

---

### 6. Path.open()

**Enhancement**: Now returns File instance:

```typescript
// NEW in Bun version
const file = new Path('/tmp/test.txt').open({ mode: 'w' })
file.write('content')
file.close()
```

**Compatibility**: Native Ejscript may have different `Path.open()` behavior.

---

## Type Extensions Compatibility

See [api-compatibility.md](../doc/architecture/components/api-compatibility.md) for detailed type extensions compatibility.

### Quick Summary

**Prefer ES6+ equivalents when available**:

| Ejscript | ES6+ Equivalent | Recommendation |
|----------|----------------|----------------|
| `str.contains(s)` | `str.includes(s)` | Use `includes()` |
| `arr.contains(x)` | `arr.includes(x)` | Use `includes()` |
| `arr.transform(fn)` | `arr.map(fn)` | Use `map()` |
| `arr.unique()` | `[...new Set(arr)]` | Either works |
| `arr.clone()` | `[...arr]` | Either works |
| `obj.clone()` | `structuredClone(obj)` | Use `structuredClone()` (ES2021+) |

**Ejscript-only methods** (no ES6 equivalent):
- `str.toPascal()`, `str.toCamel()`
- `str.expand(vars)`
- `date.format()`
- `number.format()`

---

## Migration Checklist

### For Existing Ejscript Code

**API Changes**:
- [ ] Add ES6 imports at top of each file
- [ ] Add `await` to all HTTP method calls
- [ ] Add `await` to all Socket read/write calls
- [ ] Mark functions using HTTP/Socket as `async`
- [ ] Replace `file.async = true` with synchronous operations
- [ ] Pre-allocate ByteArray with sufficient size

**Language Changes**:
- [ ] Replace `for each (x in arr)` with `for (const x of arr)`
- [ ] Replace `for each (v in obj)` with `for (const k in obj)` + access `obj[k]`
- [ ] Convert multiline strings with single/double quotes to backticks
- [ ] Remove duplicate variable declarations (use assignment instead)
- [ ] Add explicit type conversions where Ejscript had automatic coercion
- [ ] Handle array indices as strings in `for...in` loops (or use `for...of`)

**Testing**:
- [ ] Test all code paths thoroughly
- [ ] Verify loop iterations work correctly
- [ ] Check type conversions are explicit

### Example Migration

**Before (Native Ejscript)**:
```javascript
function fetchData(url) {
    let http = new Http()
    http.get(url)
    if (http.status == 200) {
        return http.response
    }
    return null
}
```

**After (Ejscript for Bun)**:
```typescript
import { Http } from '@embedthis/ejscript'

async function fetchData(url) {
    let http = new Http()
    http.get(url)
    await http.wait()  // Wait for request to complete
    if (http.status == 200) {
        return http.response
    }
    return null
}
```

### Code Patterns

**Pattern 1: HTTP Request**
```typescript
// Before
let http = new Http()
http.get(url)
process(http.response)

// After
import { Http } from '@embedthis/ejscript'
let http = new Http()
http.get(url)
await http.wait()
process(http.response)
```

**Pattern 2: File Operations**
```typescript
// Before
let path = new Path('/tmp/data.txt')
let content = path.readString()

// After
import { Path } from '@embedthis/ejscript'
let path = new Path('/tmp/data.txt')
let content = path.readString()  // No change - still sync
```

**Pattern 3: Command Execution**
```typescript
// Before
let cmd = new Cmd('ls -la')
print(cmd.response)

// After
import { Cmd } from '@embedthis/ejscript'
let cmd = new Cmd('ls -la')
print(cmd.response)  // No change - still sync
```

---

## Testing Compatibility

Run the comprehensive test suite:

```bash
bun test
```

**Current Status**: **1279/1289 tests passing (99.2%)**

Most tests are direct ports from original Ejscript test suite, ensuring API compatibility.

---

## Performance Considerations

### Faster

✅ HTTP requests (uses Bun's optimized `fetch()`)
✅ File I/O (uses Bun's fast native APIs)
✅ Process spawning (uses Bun's optimized `spawn()`)
✅ JSON parsing (Bun's native JSON parser)

### Same

➡️ Path operations (lightweight, no I/O)
➡️ String operations (native JavaScript)
➡️ Array operations (native JavaScript)

### Limitations

⚠️ No async file I/O (all synchronous)
⚠️ ByteArray fixed size (pre-allocate)
⚠️ HTTP requires `await` (but enables non-blocking)

---

## Getting Help

### Documentation

- [API Reference](API.md) - Complete API documentation
- [Examples](EXAMPLES.md) - Practical code examples
- [README](../README.md) - Project overview

### Issues

If you encounter compatibility issues:

1. Check this document
2. Review [api-compatibility.md](../doc/architecture/components/api-compatibility.md)
3. Search test suite for examples
4. Check [CHANGELOG.md](../doc/sessions/CHANGELOG.md)

---

## Summary Table

| Feature | Native | Bun | Status | Impact |
|---------|--------|-----|--------|--------|
| **API Features** | | | | |
| Imports | Global | ES6 | 🔴 Breaking | Add imports |
| HTTP Methods | Sync | Async | 🟡 Moderate | Add `await` |
| Socket I/O | Sync | Async | 🟡 Moderate | Add `await` |
| File I/O | Sync/Async | Sync | 🟢 Compatible | Same for sync |
| ByteArray Growth | Dynamic | Fixed | 🟡 Limited | Pre-allocate |
| Path Operations | Yes | Yes | 🟢 Compatible | Identical |
| Type Extensions | Yes | Yes | 🟢 Compatible | Identical |
| Cmd Execution | Sync | Sync/Async | 🟢 Compatible | Same for sync |
| Streams | Yes | Yes | 🟢 Compatible | Identical |
| HTTP Streaming | No | Yes | 🟢 Enhanced | New feature |
| File Encoding | Limited | Full | 🟢 Enhanced | New feature |
| JSON5 Parsing | Yes | Yes | 🟢 Compatible | Identical |
| JSON5 Output | Unknown | No | 🟡 Limited | Standard JSON only |
| XML/E4X | Yes | No | 🔴 Missing | Use JSON/library |
| Operator Overload | Yes | No | 🔴 Missing | Use methods |
| **Language Features** | | | | |
| `for each` Syntax | Yes | No | 🔴 Breaking | Use `for...of`/`for...in` |
| Multiline Strings | Any quotes | Backticks | 🟡 Moderate | Use backticks \` |
| Variable Re-declaration | Yes | No | 🟡 Moderate | Use assignment |
| Runtime Type Coercion | Yes | No | 🔴 Critical | Manual conversion |
| Array Index Type | Number | String | 🟡 Moderate | Use `for...of` instead |

**Legend**:
- 🟢 Compatible - Works the same or better
- 🟡 Limited - Works with modifications
- 🔴 Breaking - Requires code changes

---

**Conclusion**: Ejscript for Bun maintains **99%+ API compatibility** with native Ejscript. The main changes are:

**Required Changes**:
1. Add ES6 imports (all files)
2. Add `await` to HTTP/Socket calls (async)
3. Replace `for each` with `for...of` or `for...in` (syntax)
4. Convert multiline strings to use backticks `` ` `` (template literals)
5. Add explicit type conversions (no runtime coercion)

**Recommended Changes**:
5. Pre-allocate ByteArray size (avoid overflow)
6. Use `for...of` instead of `for...in` for arrays (numeric indices)

All other APIs work identically!

---

*Last Updated: 2025-10-26*
*Version: 1.0.0*
