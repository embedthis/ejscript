# Application Migration Guide: Native Ejscript → Ejscript for Bun

**Migrating**: Ejscript applications from native runtime to Bun
**Last Updated**: 2025-10-27

---

## Overview

This guide helps you migrate Ejscript applications written for the native runtime to run on Bun using Ejscript for Bun (TypeScript port).

**What's Changing**:
1. **Runtime**: Native Ejscript → Bun JavaScript runtime
2. **API Version**: Ejscript v1.x (sync) → v2.0.0 (async)
3. **Imports**: Global namespace → ES6 imports required
4. **Language**: Ejscript syntax → JavaScript/TypeScript ES6+

---

## Migration Complexity

**Estimated Effort**: Varies by application size
- Small scripts (< 500 lines): 1-2 hours
- Medium apps (500-2000 lines): 4-8 hours
- Large apps (2000+ lines): 2-4 days

**With automation tools**: Can reduce effort by 50-70%

---

## Quick Start: 3-Step Migration

### Step 1: Add ES6 Imports

**BEFORE (Native Ejscript)**:
```javascript
// No imports - everything is global
let path = new Path('/tmp/file.txt')
let http = new Http()
```

**AFTER (Ejscript for Bun)**:
```typescript
import { Path, Http } from 'ejscript'

let path = new Path('/tmp/file.txt')
let http = new Http()
```

**All Classes Need Imports**:
```typescript
// Core classes
import { Path, File, App, Http, Socket } from 'ejscript'

// Streams
import { ByteArray, TextStream, BinaryStream } from 'ejscript'

// Utilities
import { Logger, Cache, Timer, Cmd, Uri } from 'ejscript'
```

### Step 2: Update to Async Patterns (v2.0.0)

#### HTTP Methods

**BEFORE (v1.x - Synchronous)**:
```javascript
let http = new Http()
await http.get('https://example.com')  // v1.x: immediate status
console.log(http.status)  // Available right away
```

**AFTER (v2.0.0 - Async)**:
```typescript
import { Http } from 'ejscript'

let http = new Http()
http.get('https://example.com')       // Start request
await http.finalize()                  // Wait for completion
console.log(http.status)               // Now available
```

**All HTTP Methods Affected**:
- `http.get()`, `http.post()`, `http.put()`, `http.del()`, `http.head()`
- `http.form()`, `http.connect()`, `http.upload()`
- All need `await http.finalize()` after calling

#### Socket Operations

**BEFORE (v1.x)**:
```javascript
let socket = new Socket()
socket.connect('localhost:8080')
socket.write('Hello')                // Synchronous
let buf = new ByteArray()
socket.read(buf, -1)                 // Synchronous (blocks)
```

**AFTER (v2.0.0)**:
```typescript
import { Socket, ByteArray } from 'ejscript'

let socket = new Socket()
socket.connect('localhost:8080')
await socket.write('Hello')          // Async
let buf = new ByteArray()
await socket.read(buf, -1)           // Async
```

#### File Operations

**BEFORE (v1.x)**:
```javascript
// File auto-opens
let file = new File('/tmp/test.txt', 'r')
let content = file.readString()      // Synchronous
file.close()                         // Synchronous
```

**AFTER (v2.0.0)**:
```typescript
import { File } from 'ejscript'

// Must explicitly open
let file = new File('/tmp/test.txt', 'r')
await file.open()
let content = await file.readString()  // Async
await file.close()                     // Async
```

#### Path Methods

**BEFORE (v1.x)**:
```javascript
let path = new Path('/tmp/data.json')
let data = path.readString()         // Synchronous
path.remove()                        // Synchronous
```

**AFTER (v2.0.0)**:
```typescript
import { Path } from 'ejscript'

let path = new Path('/tmp/data.json')
let data = await path.readString()   // Async
await path.remove()                  // Async
```

### Step 3: Update Language Syntax

#### `for each` Loops

**BEFORE**:
```javascript
let arr = [1, 2, 3]
for each (item in arr) {
    print(item)
}

let obj = {a: 1, b: 2}
for each (value in obj) {
    print(value)
}
```

**AFTER**:
```typescript
let arr = [1, 2, 3]
for (const item of arr) {          // for...of for arrays
    print(item)
}

let obj = {a: 1, b: 2}
for (const key in obj) {           // for...in for objects
    print(obj[key])                // Access value via key
}
```

#### Multiline Strings

**BEFORE**:
```javascript
let html = "<html>
<body>
  <h1>Title</h1>
</body>
</html>"
```

**AFTER**:
```typescript
let html = `<html>
<body>
  <h1>Title</h1>
</body>
</html>`  // Use backticks for multiline
```

#### Variable Re-declaration

**BEFORE**:
```javascript
let x = 10
let x = 20  // Allowed in Ejscript
```

**AFTER**:
```typescript
let x = 10
x = 20      // Use assignment, not re-declaration
```

---

## Complete Migration Checklist

### API Changes
- [ ] Add ES6 imports to all files
- [ ] Add `await` to all HTTP method calls
- [ ] Add `await http.finalize()` after HTTP methods
- [ ] Add `await` to Socket read/write operations
- [ ] Add `await` to File open/read/write/close operations
- [ ] Add `await` to Path async methods (readString, remove, etc.)
- [ ] Mark functions using async I/O as `async`

### Language Changes
- [ ] Replace `for each (x in arr)` with `for (const x of arr)`
- [ ] Replace `for each (v in obj)` with `for (const k in obj)` + access `obj[k]`
- [ ] Convert multiline strings to use backticks `` ` ``
- [ ] Remove duplicate variable declarations (use assignment)
- [ ] Add explicit type conversions where needed

### Testing
- [ ] Test all code paths thoroughly
- [ ] Verify all async operations complete
- [ ] Check error handling works correctly
- [ ] Performance test if needed

---

## Common Patterns & Examples

### Pattern 1: Simple HTTP Request

**BEFORE**:
```javascript
function fetchData(url) {
    let http = new Http()
    await http.get(url)
    return http.status == 200 ? http.response : null
}
```

**AFTER**:
```typescript
import { Http } from 'ejscript'

async function fetchData(url: string): Promise<string | null> {
    let http = new Http()
    http.get(url)
    await http.finalize()
    return http.status == 200 ? http.response : null
}
```

### Pattern 2: File Processing

**BEFORE**:
```javascript
function processFile(filename) {
    let path = new Path(filename)
    let content = path.readString()
    // Process content...
    path.write(processedContent)
}
```

**AFTER**:
```typescript
import { Path } from 'ejscript'

async function processFile(filename: string): Promise<void> {
    let path = new Path(filename)
    let content = await path.readString()
    // Process content...
    await path.write(processedContent)
}
```

### Pattern 3: Socket Server

**BEFORE**:
```javascript
let server = new Socket()
server.listen(8080)
while (true) {
    let client = server.accept()
    let data = new ByteArray()
    client.read(data, -1)
    client.write("Response")
    client.close()
}
```

**AFTER**:
```typescript
import { Socket, ByteArray } from 'ejscript'

async function runServer(): Promise<void> {
    let server = new Socket()
    server.listen(8080)
    while (true) {
        let client = server.accept()
        let data = new ByteArray()
        await client.read(data, -1)
        await client.write("Response")
        client.close()
    }
}
```

---

## Automated Migration Tools

### Find and Replace Patterns

**1. Add imports to files without them**:
```bash
# Add to top of each .js/.es file:
# import { Path, File, Http, App } from 'ejscript'
```

**2. Convert for each loops**:
```bash
# Find: for each \((.*?) in (.*?)\)
# Replace with for...of for arrays, for...in for objects
```

**3. Convert multiline strings**:
```bash
# Manual review recommended - look for unescaped newlines
# in single or double-quoted strings
```

---

## API Compatibility Reference

### What's Compatible (No Changes)

✅ **Path operations** (non-I/O):
- `path.join()`, `path.basename`, `path.dirname`, `path.extension`
- `path.exists`, `path.isDir`, `path.isFile`, `path.size`
- All properties work the same

✅ **String extensions**:
- `str.contains()`, `str.toPascal()`, `str.toCamel()`, `str.expand()`

✅ **Array extensions**:
- `arr.contains()`, `arr.unique()`, `arr.append()`, `arr.clone()`

✅ **App class**:
- `App.args`, `App.env`, `App.dir`, `App.exePath`
- All work the same

### What Changed (Requires Updates)

⚠️ **HTTP methods**: All are now async, require `finalize()`
⚠️ **Socket I/O**: `read()` and `write()` are async
⚠️ **File I/O**: All methods async, must `open()` explicitly
⚠️ **Path I/O**: `readString()`, `readJSON()`, `write()`, `remove()` are async

### What's Not Supported

❌ **XML/E4X**: Not implemented (use JSON or external XML library)
❌ **Operator overloading**: JavaScript limitation (use methods instead)
❌ **Runtime type coercion**: TypeScript types are compile-time only

See [../COMPATIBILITY.md](../COMPATIBILITY.md) for complete API reference.

---

## Migration Strategy

### Recommended Approach

1. **Start with imports** (1 hour)
   - Add ES6 imports to all files
   - Use automated find/replace where possible

2. **Update async patterns** (2-4 hours)
   - Add `await` to HTTP/Socket/File operations
   - Mark functions as `async`
   - Test incrementally

3. **Fix language syntax** (1-2 hours)
   - Convert `for each` loops
   - Fix multiline strings
   - Remove variable re-declarations

4. **Test thoroughly** (2-4 hours)
   - Run full test suite
   - Fix any runtime errors
   - Performance test critical paths

### Incremental vs Big Bang

**Incremental (Recommended)**:
- Migrate one module at a time
- Test each module before moving on
- Lower risk, easier debugging
- Takes longer but more controlled

**Big Bang**:
- Migrate everything at once
- Test everything together
- Higher risk, harder debugging
- Faster if you have good test coverage

---

## Troubleshooting

### Issue: Import errors

**Symptom**: `Cannot find module 'ejscript'`
**Solution**:
```bash
bun install
# Ensure ejscript is linked/installed
```

### Issue: Status not ready

**Symptom**: `http.status` is undefined or null
**Solution**: Add `await http.finalize()` after HTTP method
```typescript
http.get(url)
await http.finalize()  // ← Add this
console.log(http.status)
```

### Issue: File not opening

**Symptom**: File operations fail
**Solution**: Call `await file.open()` after constructor
```typescript
let file = new File(path, 'r')
await file.open()  // ← Add this
```

### Issue: Syntax errors with loops

**Symptom**: `Unexpected token 'each'`
**Solution**: Convert to ES6 loops
```typescript
// Old: for each (item in arr)
// New: for (const item of arr)
```

---

## Getting Help

### Documentation
- [../COMPATIBILITY.md](../COMPATIBILITY.md) - Complete API compatibility
- [../README.md](../README.md) - Project overview
- [TEST_MIGRATION.md](./TEST_MIGRATION.md) - If you're also migrating tests

### Common Issues
- Check [../COMPATIBILITY.md](../COMPATIBILITY.md) for API differences
- See examples in `examples/` directory
- Review test files in `test/` for patterns

---

## Success Metrics

**You've successfully migrated when**:
- ✅ All imports added
- ✅ All async operations use `await`
- ✅ All tests passing
- ✅ No runtime errors
- ✅ Performance acceptable

**Typical Results**:
- Migration time: 1-4 days for medium apps
- Performance: Same or better (Bun is fast!)
- Maintenance: Easier with TypeScript types
- Compatibility: 99%+ API compatibility

---

*Last Updated: 2025-10-27*
*See [README.md](./README.md) for guide navigation*
