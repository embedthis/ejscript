# Async I/O Conversion Plan

**Project**: Ejscript for Bun
**Version Target**: 2.0.0 (Breaking Change)
**Created**: 2025-10-26
**Status**: Approved - Ready for Implementation

## Overview

This document catalogs all methods that will be converted from synchronous to asynchronous operations. This is a **breaking change** that will require application code to use `await` keywords, but provides significant benefits in performance, scalability, and consistency with modern JavaScript patterns.

## Conversion Strategy

**Approach:** Convert existing sync I/O methods to async (return Promises)
**Naming:** Keep method names identical (no `Async` suffix needed)
**Breaking:** YES - Apps must add `await` keywords and make functions `async`
**Version:** Bump to 2.0.0 (major version per semver)

## Classes and Methods to Convert

### 1. Path Class - 12 Methods to Convert

**File**: `src/core/Path.ts` (854 lines)

#### Read Operations (4 methods)

| Method | Current Signature | New Signature | Line |
|--------|------------------|---------------|------|
| `readBytes()` | `readBytes(): Uint8Array \| null` | `async readBytes(): Promise<Uint8Array \| null>` | ~801 |
| `readString()` | `readString(): string \| null` | `async readString(): Promise<string \| null>` | ~813 |
| `readJSON()` | `readJSON(): any` | `async readJSON(): Promise<any>` | ~825 |
| `readLines()` | `readLines(): string[] \| null` | `async readLines(): Promise<string[] \| null>` | ~834 |

**Current Implementation:**
```typescript
readString(): string | null {
    try {
        return fs.readFileSync(this._path, 'utf-8')
    } catch {
        return null
    }
}
```

**New Implementation:**
```typescript
async readString(): Promise<string | null> {
    try {
        const file = Bun.file(this._path)
        return await file.text()
    } catch {
        return null
    }
}
```

#### Write Operations (2 methods)

| Method | Current Signature | New Signature | Line |
|--------|------------------|---------------|------|
| `write()` | `write(...args: any[]): void` | `async write(...args: any[]): Promise<void>` | ~783 |
| `append()` | `append(data: string \| Uint8Array, options?: string): void` | `async append(data: string \| Uint8Array, options?: string): Promise<void>` | ~775 |

**Current Implementation:**
```typescript
write(...args: any[]): void {
    const content = args.map(arg => /* ... */).join('')
    fs.writeFileSync(this._path, content, { mode: 0o644 })
}
```

**New Implementation:**
```typescript
async write(...args: any[]): Promise<void> {
    const content = args.map(arg => /* ... */).join('')
    await Bun.write(this._path, content)
}
```

#### File Management Operations (6 methods)

| Method | Current Signature | New Signature | Line |
|--------|------------------|---------------|------|
| `copy()` | `copy(destination: Path \| string, options?): void` | `async copy(destination: Path \| string, options?): Promise<void>` | ~137 |
| `remove()` | `remove(): boolean` | `async remove(): Promise<boolean>` | ~549 |
| `removeAll()` | `removeAll(): boolean` | `async removeAll(): Promise<boolean>` | ~570 |
| `truncate()` | `truncate(size: number): void` | `async truncate(size: number): Promise<void>` | ~755 |
| `rename()` | `rename(target: Path \| string): void` | `async rename(target: Path \| string): Promise<void>` | ~591 |
| `makeDir()` | `makeDir(options?): boolean` | `async makeDir(options?): Promise<boolean>` | ~402 |

**Implementation Notes:**
- Use `fs.promises.copyFile()` instead of `fs.copyFileSync()`
- Use `fs.promises.unlink()` instead of `fs.unlinkSync()`
- Use `fs.promises.rm()` instead of `fs.rmSync()`
- Use `fs.promises.mkdir()` instead of `fs.mkdirSync()`
- Use `fs.promises.rename()` instead of `fs.renameSync()`
- Use `fs.promises.truncate()` instead of `fs.truncateSync()`

### 2. File Class - 8 Methods to Convert

**File**: `src/core/File.ts` (498 lines)

#### I/O Operations (6 methods)

| Method | Current Signature | New Signature | Line |
|--------|------------------|---------------|------|
| `read()` | `read(buffer: Uint8Array, offset?: number, count?: number): number \| null` | `async read(buffer: Uint8Array, offset?: number, count?: number): Promise<number \| null>` | ~241 |
| `readBytes()` | `readBytes(count?: number): ByteArray \| null` | `async readBytes(count?: number): Promise<ByteArray \| null>` | ~289 |
| `readString()` | `readString(count?: number): string \| null` | `async readString(count?: number): Promise<string \| null>` | ~316 |
| `readLines()` | `readLines(): string[] \| null` | `async readLines(): Promise<string[] \| null>` | ~332 |
| `write()` | `write(...items: any[]): number` | `async write(...items: any[]): Promise<number>` | ~413 |
| `writeLine()` | `writeLine(...items: any[]): number` | `async writeLine(...items: any[]): Promise<number>` | ~456 |

**Current Implementation:**
```typescript
read(buffer: Uint8Array, offset: number = 0, count: number = -1): number | null {
    // ... validation ...
    const bytesRead = fs.readSync(this._fd, buffer, offset, count, this._position)
    this._position += bytesRead
    return bytesRead
}
```

**New Implementation:**
```typescript
async read(buffer: Uint8Array, offset: number = 0, count: number = -1): Promise<number | null> {
    // ... validation ...
    const fs = await import('fs/promises')
    const handle = await fs.open(this._path.name, this._mode)
    const result = await handle.read(buffer, offset, count, this._position)
    this._position += result.bytesRead
    await handle.close()
    return result.bytesRead
}
```

#### File Management (2 methods)

| Method | Current Signature | New Signature | Line |
|--------|------------------|---------------|------|
| `open()` | `open(options?: FileOptions \| string \| null): File` | `async open(options?: FileOptions \| string \| null): Promise<File>` | ~127 |
| `close()` | `close(): void` | `async close(): Promise<void>` | ~75 |

**Implementation Notes:**
- Use `fs.promises.open()` for file handle
- Store file handle instead of file descriptor
- Use handle methods for async read/write
- Properly close handles in `close()`

### 3. Cmd Class - 4 Methods to Convert

**File**: `src/core/utilities/Cmd.ts` (605 lines)

#### Instance Methods (3 methods)

| Method | Current Signature | New Signature | Line |
|--------|------------------|---------------|------|
| `read()` | `read(buffer: ByteArray, offset?: number, count?: number): number \| null` | `async read(buffer: ByteArray, offset?: number, count?: number): Promise<number \| null>` | ~226 |
| `readString()` | `readString(count?: number): string \| null` | `async readString(count?: number): Promise<string \| null>` | ~252 |
| `readLines()` | `readLines(count?: number): string[] \| null` | `async readLines(count?: number): Promise<string[] \| null>` | ~271 |

**Already Async:**
- `wait(timeout?: number): Promise<boolean>` ✅ (line ~421)

#### Static Methods (1 method)

| Method | Current Signature | New Signature | Line |
|--------|------------------|---------------|------|
| `Cmd.run()` | `static run(command, options?, data?): string \| null` | `static async run(command, options?, data?): Promise<string \| null>` | ~551 |

**Implementation Notes:**
- Cmd already uses async `Bun.spawn()` internally
- The `_readStream()` method is already async (line ~350)
- Need to properly await stream reading in `read()` methods
- `Cmd.sh()` can be updated to use async `run()`

### 4. Methods to KEEP Synchronous

These methods perform no I/O or only fast metadata lookups and should remain sync:

#### Path Class - Sync Methods (Keep As-Is)

**Path Manipulation (No I/O):**
- `join(...others: string[]): Path`
- `joinExt(ext: string): Path`
- `trimExt(): Path`
- `basename`, `dirname`, `extension`, `normalize`, `absolute`, `relative`
- `map()`, `match()`, `portable`, `natural`, `windows`

**Metadata (Fast stat operations):**
- `exists: boolean` - Can stay sync (fast fs.existsSync)
- `isDir: boolean` - Can stay sync (fast statSync)
- `isFile: boolean` - Can stay sync
- `isLink: boolean` - Can stay sync
- `size: number` - Can stay sync (cached or fast stat)
- `perms: number` - Can stay sync
- `modified: Date` - Can stay sync
- `accessed: Date` - Can stay sync
- `attributes: object` - Can stay sync
- `setAttributes()` - Could convert, but low priority

**Utility Methods:**
- `toString()`, `valueOf()`, `iterator`
- `same(other: Path): boolean`
- `childOf(dir: Path): boolean`
- `relativeTo(origin?: Path): Path`

#### File Class - Sync Methods (Keep As-Is)

**Properties:**
- `isOpen: boolean`
- `canRead: boolean`
- `canWrite: boolean`
- `position: number` (get/set)
- `path: Path`
- `size: number` - Uses fstatSync when open (fast)
- `encoding: string` (get/set)
- `options: FileOptions`

**Methods:**
- `flush()` - Currently no-op for unbuffered I/O
- `seek()` - Just updates position, no I/O
- `remove()` - Delegates to Path.remove() (will become async)

#### Cmd Class - Sync Methods (Keep As-Is)

**Properties:**
- `env: Record<string, string>` (get/set)
- `error: string` (get)
- `errorStream: any` (get)
- `pid: number` (get)
- `response: string` (get - cached)
- `status: number` (get)
- `timeout: number` (get/set)

**Methods:**
- `close()` - Cleanup, no async I/O needed
- `finalize()` - Signal end of input
- `flush()` - No-op
- `stop()` - Kill process (sync signal)
- `write()` - May need to be async for consistency

**Static Methods (Keep Sync):**
- `Cmd.locate(program, search?)` - Path lookup
- `Cmd.daemon(cmdline, options?)` - Fire and forget
- `Cmd.exec(cmdline, options?)` - Edge case, low priority
- `Cmd.kill(pid, signal?)` - Process signal (sync)

## Migration Impact on Tests

### Test Files to Update

All test files using I/O methods must be updated to async:

1. **test/core/path.tst.ts** - All Path I/O tests
2. **test/core/file.tst.ts** - All File I/O tests
3. **test/core/cmd.tst.ts** - All Cmd I/O tests
4. **test/http.tst.ts** - May use Path for temp files
5. **test/integration/*.tst.ts** - Integration tests
6. **examples/*.ts** - All example code

### Test Pattern Changes

**Before (Sync):**
```typescript
import { describe, it, expect } from '@embedthis/testme'
import { Path } from '../src/core/Path'

describe('Path', () => {
    it('should read file', () => {
        const path = new Path('/tmp/test.txt')
        path.write('test content')
        const content = path.readString()
        expect(content).toBe('test content')
        path.remove()
    })
})
```

**After (Async):**
```typescript
import { describe, it, expect } from '@embedthis/testme'
import { Path } from '../src/core/Path'

describe('Path', () => {
    it('should read file', async () => {
        const path = new Path('/tmp/test.txt')
        await path.write('test content')
        const content = await path.readString()
        expect(content).toBe('test content')
        await path.remove()
    })
})
```

### Estimated Test Changes

- **~200-300 test cases** will need `await` keywords added
- **~50-75 `it()` blocks** will need `async` keyword
- **~10-15 files** will need updates

## Implementation Phases

### Phase 1: Path Class (Highest Priority)
**Estimated Effort:** 4-6 hours
1. Convert 12 I/O methods to async
2. Update Path class tests (~50 tests)
3. Verify all tests pass
4. Update examples using Path

**Methods:**
- readBytes(), readString(), readJSON(), readLines()
- write(), append()
- copy(), remove(), removeAll(), truncate(), rename(), makeDir()

### Phase 2: File Class
**Estimated Effort:** 3-5 hours
1. Convert 8 I/O methods to async
2. Update File class tests (~40 tests)
3. Verify all tests pass
4. Update examples using File

**Methods:**
- read(), readBytes(), readString(), readLines()
- write(), writeLine()
- open(), close()

### Phase 3: Cmd Class
**Estimated Effort:** 2-4 hours
1. Convert 4 methods to async
2. Update Cmd class tests (~30 tests)
3. Verify all tests pass
4. Update examples using Cmd

**Methods:**
- read(), readString(), readLines()
- Cmd.run() (static)

### Phase 4: Integration & Documentation
**Estimated Effort:** 2-3 hours
1. Run full test suite (1207+ tests)
2. Update all documentation
3. Update CHANGELOG.md
4. Create migration guide
5. Update examples
6. Bump version to 2.0.0

**Total Estimated Effort:** 11-18 hours

## Documentation Updates Required

### Files to Update

1. **DESIGN.md**
   - Add "Async I/O Architecture" section
   - Document conversion rationale
   - Show async patterns

2. **CLAUDE.md**
   - Update usage examples to async
   - Document breaking changes
   - Add migration notes

3. **CHANGELOG.md**
   - Document breaking changes
   - List all converted methods
   - Provide migration examples

4. **README.md**
   - Update quick start to use async
   - Update all code examples
   - Add async/await section

5. **QUICK_START.md**
   - Update all examples to async
   - Show async patterns

6. **New File: MIGRATION_V2.md**
   - Detailed migration guide
   - Before/after examples
   - Common patterns
   - Breaking changes list

### Documentation Template

```markdown
## Breaking Changes in v2.0.0

All I/O operations are now asynchronous and return Promises.

### Migration Required

**Before (v1.x):**
```typescript
const path = new Path('/tmp/file.txt')
path.write('Hello')
const content = path.readString()
path.remove()
```

**After (v2.0.0):**
```typescript
const path = new Path('/tmp/file.txt')
await path.write('Hello')
const content = await path.readString()
await path.remove()
```

### Affected Methods

**Path class (12 methods):**
- readBytes(), readString(), readJSON(), readLines()
- write(), append()
- copy(), remove(), removeAll(), truncate(), rename(), makeDir()

**File class (8 methods):**
- read(), readBytes(), readString(), readLines()
- write(), writeLine()
- open(), close()

**Cmd class (4 methods):**
- read(), readString(), readLines()
- Cmd.run() (static)
```

## Benefits of Async Conversion

### Performance Benefits
✅ **Non-blocking I/O** - Event loop remains responsive
✅ **Concurrent operations** - Multiple I/O operations can run in parallel
✅ **Better throughput** - Especially for I/O-heavy workloads
✅ **Scalability** - Handles more concurrent operations

### API Consistency
✅ **Matches Socket API** - Socket.read/write already async
✅ **Matches Http API** - Http.finalize/wait already async
✅ **Modern JavaScript** - Follows async/await patterns
✅ **Bun-native** - Uses Bun's optimized async APIs

### Code Quality
✅ **Explicit async flow** - `await` keywords make async operations visible
✅ **Error handling** - Promises provide better error handling
✅ **Composability** - Easier to chain async operations
✅ **Future-proof** - Aligns with JavaScript evolution

### Developer Experience
✅ **Consistent patterns** - All I/O is async
✅ **No mixed sync/async** - Clearer mental model
✅ **Better tooling** - IDEs understand async/await
✅ **Debugging** - Async stack traces in modern runtimes

## Breaking Changes Summary

### What Changes
- ❌ **All I/O method calls** must be `await`ed
- ❌ **Calling functions** must be marked `async`
- ❌ **Return types** wrapped in `Promise<>`
- ❌ **Version bump** to 2.0.0

### What Stays the Same
- ✅ **Method names** - Identical names (no `Async` suffix)
- ✅ **Parameters** - Same parameters
- ✅ **Return values** - Same types (just wrapped in Promise)
- ✅ **Properties** - All properties stay sync
- ✅ **Metadata methods** - Fast operations stay sync

### Migration Effort
- **Low complexity** - Just add `await` keywords
- **High visibility** - TypeScript will catch missing awaits
- **Gradual** - Can update method-by-method
- **Automated** - Could write codemod for conversion

## Version Strategy

### Current State
- Version: 1.0.0
- Status: Production ready
- API: Sync I/O

### After Conversion
- Version: 2.0.0
- Status: Production ready with async I/O
- API: Async I/O (breaking change)

### Semver Compliance
- **Major version bump** (1.x → 2.x) per semantic versioning
- **Breaking change** clearly documented
- **Migration guide** provided
- **Changelog** lists all breaking changes

## Success Criteria

### Implementation
✅ All 24 methods converted to async
✅ All converted methods return Promises
✅ All sync implementations replaced with async

### Testing
✅ All 1207+ existing tests updated and passing
✅ New async-specific tests added
✅ Integration tests verify concurrent operations
✅ No test regressions

### Documentation
✅ All documentation updated with async examples
✅ Migration guide created
✅ CHANGELOG documents breaking changes
✅ Examples updated to async/await

### Quality
✅ TypeScript compilation with no errors
✅ All async operations properly awaited
✅ Error handling preserved
✅ Performance maintained or improved

## Next Steps

1. **Approve this plan** ✅ (Done)
2. **Phase 1: Path Class** - Convert 12 methods + tests
3. **Phase 2: File Class** - Convert 8 methods + tests
4. **Phase 3: Cmd Class** - Convert 4 methods + tests
5. **Phase 4: Integration** - Documentation + version bump

## Summary

**Total methods to convert:** 24 methods
- Path: 12 methods
- File: 8 methods
- Cmd: 4 methods

**Breaking change:** YES - Version 2.0.0
**Migration effort:** Low (add `await` keywords)
**Benefits:** High (performance, consistency, modern patterns)
**Risk:** Low (TypeScript catches missing awaits)

---

**Status:** ✅ Approved - Ready for Implementation
**Next:** Begin Phase 1 - Path Class async conversion
