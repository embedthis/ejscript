# Async I/O Methods - Quick Reference

**Project**: Ejscript for Bun
**Version Target**: 2.0.0
**Last Updated**: 2025-10-26
**Status**: Analysis Complete - Ready for Implementation

## Quick Reference: Methods to Convert to Async

This document provides a quick reference for all methods that will be converted from synchronous to asynchronous in version 2.0.0.

---

## Path Class (12 methods)

**File**: `src/core/Path.ts`

### Read Operations

```typescript
// Current (v1.x)
readBytes(): Uint8Array | null
readString(): string | null
readJSON(): any
readLines(): string[] | null

// Convert to (v2.0)
async readBytes(): Promise<Uint8Array | null>
async readString(): Promise<string | null>
async readJSON(): Promise<any>
async readLines(): Promise<string[] | null>
```

**Usage Before:**
```typescript
const content = path.readString()
```

**Usage After:**
```typescript
const content = await path.readString()
```

### Write Operations

```typescript
// Current (v1.x)
write(...args: any[]): void
append(data: string | Uint8Array, options?: string): void

// Convert to (v2.0)
async write(...args: any[]): Promise<void>
async append(data: string | Uint8Array, options?: string): Promise<void>
```

**Usage Before:**
```typescript
path.write('Hello World')
path.append('\nMore data')
```

**Usage After:**
```typescript
await path.write('Hello World')
await path.append('\nMore data')
```

### File Management

```typescript
// Current (v1.x)
copy(destination: Path | string, options?): void
remove(): boolean
removeAll(): boolean
truncate(size: number): void
rename(target: Path | string): void
makeDir(options?): boolean

// Convert to (v2.0)
async copy(destination: Path | string, options?): Promise<void>
async remove(): Promise<boolean>
async removeAll(): Promise<boolean>
async truncate(size: number): Promise<void>
async rename(target: Path | string): Promise<void>
async makeDir(options?): Promise<boolean>
```

**Usage Before:**
```typescript
path.copy('/backup/file.txt')
path.remove()
path.makeDir({ permissions: 0o755 })
```

**Usage After:**
```typescript
await path.copy('/backup/file.txt')
await path.remove()
await path.makeDir({ permissions: 0o755 })
```

---

## File Class (8 methods)

**File**: `src/core/File.ts`

### I/O Operations

```typescript
// Current (v1.x)
read(buffer: Uint8Array, offset?: number, count?: number): number | null
readBytes(count?: number): ByteArray | null
readString(count?: number): string | null
readLines(): string[] | null
write(...items: any[]): number
writeLine(...items: any[]): number

// Convert to (v2.0)
async read(buffer: Uint8Array, offset?: number, count?: number): Promise<number | null>
async readBytes(count?: number): Promise<ByteArray | null>
async readString(count?: number): Promise<string | null>
async readLines(): Promise<string[] | null>
async write(...items: any[]): Promise<number>
async writeLine(...items: any[]): Promise<number>
```

**Usage Before:**
```typescript
const file = new File('/tmp/test.txt', 'r')
const content = file.readString()
file.close()
```

**Usage After:**
```typescript
const file = await new File('/tmp/test.txt', 'r').open()
const content = await file.readString()
await file.close()
```

### File Management

```typescript
// Current (v1.x)
open(options?: FileOptions | string | null): File
close(): void

// Convert to (v2.0)
async open(options?: FileOptions | string | null): Promise<File>
async close(): Promise<void>
```

**Usage Before:**
```typescript
const file = new File('/tmp/test.txt')
file.open('w')
file.write('test')
file.close()
```

**Usage After:**
```typescript
const file = new File('/tmp/test.txt')
await file.open('w')
await file.write('test')
await file.close()
```

---

## Cmd Class (4 methods)

**File**: `src/core/utilities/Cmd.ts`

### Instance Methods

```typescript
// Current (v1.x)
read(buffer: ByteArray, offset?: number, count?: number): number | null
readString(count?: number): string | null
readLines(count?: number): string[] | null

// Convert to (v2.0)
async read(buffer: ByteArray, offset?: number, count?: number): Promise<number | null>
async readString(count?: number): Promise<string | null>
async readLines(count?: number): Promise<string[] | null>
```

**Usage Before:**
```typescript
const cmd = new Cmd('ls -la')
const output = cmd.readString()
```

**Usage After:**
```typescript
const cmd = new Cmd('ls -la', { detach: true })
const output = await cmd.readString()
```

### Static Methods

```typescript
// Current (v1.x)
static run(command: string | string[], options?: CmdOptions, data?: any): string | null

// Convert to (v2.0)
static async run(command: string | string[], options?: CmdOptions, data?: any): Promise<string | null>
```

**Usage Before:**
```typescript
const output = Cmd.run('echo hello')
```

**Usage After:**
```typescript
const output = await Cmd.run('echo hello')
```

---

## Methods That Stay Synchronous

### Path Class - Sync Methods (No Changes)

**Path Manipulation (No I/O):**
- `join(...others: string[]): Path`
- `joinExt(ext: string): Path`
- `trimExt(): Path`
- `basename`, `dirname`, `extension`
- `normalize`, `absolute`, `relative`, `portable`, `natural`, `windows`
- `map()`, `match()`, `same()`, `childOf()`, `relativeTo()`

**Metadata (Fast operations):**
- `exists: boolean`
- `isDir: boolean`, `isFile: boolean`, `isLink: boolean`
- `size: number`, `perms: number`
- `modified: Date`, `accessed: Date`
- `attributes: object`, `setAttributes()`

**Utility:**
- `toString()`, `valueOf()`, `[Symbol.iterator]()`

### File Class - Sync Methods (No Changes)

**Properties:**
- `isOpen: boolean`, `canRead: boolean`, `canWrite: boolean`
- `position: number` (get/set)
- `path: Path`, `size: number`
- `encoding: string`, `options: FileOptions`

**Methods:**
- `seek(loc: number, whence?: number): void`
- `flush(dir?: number): void`
- `truncate(value: number): void` - *Could be async, low priority*

### Cmd Class - Sync Methods (No Changes)

**Properties:**
- `env: Record<string, string>`
- `error: string`, `errorStream: any`
- `pid: number`, `response: string`, `status: number`
- `timeout: number`

**Methods:**
- `close(): void`
- `finalize(): void`
- `flush(dir?: number): void`
- `stop(signal?: number): boolean`
- `write(...data: any[]): number` - *May need async*

**Static Methods:**
- `Cmd.locate(program, search?)` - Path lookup
- `Cmd.daemon(cmdline, options?)` - Fire and forget
- `Cmd.kill(pid, signal?)` - Process signal

**Already Async:**
- `wait(timeout?: number): Promise<boolean>` ✅

---

## Migration Patterns

### Pattern 1: Simple Read/Write

**Before:**
```typescript
function processFile(filename: string) {
    const path = new Path(filename)
    const data = path.readString()
    return data.toUpperCase()
}
```

**After:**
```typescript
async function processFile(filename: string) {
    const path = new Path(filename)
    const data = await path.readString()
    return data.toUpperCase()
}
```

### Pattern 2: Chained Operations

**Before:**
```typescript
function backupFile(src: string, dest: string) {
    const path = new Path(src)
    path.copy(dest)
    path.remove()
}
```

**After:**
```typescript
async function backupFile(src: string, dest: string) {
    const path = new Path(src)
    await path.copy(dest)
    await path.remove()
}
```

### Pattern 3: File I/O

**Before:**
```typescript
function appendLog(message: string) {
    const file = new File('/var/log/app.log', 'a')
    file.writeLine(message)
    file.close()
}
```

**After:**
```typescript
async function appendLog(message: string) {
    const file = new File('/var/log/app.log')
    await file.open('a')
    await file.writeLine(message)
    await file.close()
}
```

### Pattern 4: Command Execution

**Before:**
```typescript
function getSystemInfo() {
    const output = Cmd.run('uname -a')
    return output
}
```

**After:**
```typescript
async function getSystemInfo() {
    const output = await Cmd.run('uname -a')
    return output
}
```

### Pattern 5: Concurrent Operations

**Before (Sequential):**
```typescript
function processFiles(files: string[]) {
    for (const file of files) {
        const path = new Path(file)
        const content = path.readString()
        process(content)
    }
}
```

**After (Parallel - Better!):**
```typescript
async function processFiles(files: string[]) {
    await Promise.all(files.map(async (file) => {
        const path = new Path(file)
        const content = await path.readString()
        process(content)
    }))
}
```

---

## TypeScript Type Changes

### Path Class

```typescript
// Before
declare class Path {
    readString(): string | null
    write(...args: any[]): void
    remove(): boolean
}

// After
declare class Path {
    readString(): Promise<string | null>
    write(...args: any[]): Promise<void>
    remove(): Promise<boolean>
}
```

### File Class

```typescript
// Before
declare class File {
    readString(count?: number): string | null
    write(...items: any[]): number
    open(options?): File
}

// After
declare class File {
    readString(count?: number): Promise<string | null>
    write(...items: any[]): Promise<number>
    open(options?): Promise<File>
}
```

### Cmd Class

```typescript
// Before
declare class Cmd {
    readString(count?: number): string | null
    static run(command, options?, data?): string | null
}

// After
declare class Cmd {
    readString(count?: number): Promise<string | null>
    static run(command, options?, data?): Promise<string | null>
}
```

---

## Common Mistakes to Avoid

### ❌ Mistake 1: Forgetting await

```typescript
// WRONG - Missing await
const content = path.readString()  // Returns Promise, not string!

// CORRECT
const content = await path.readString()
```

### ❌ Mistake 2: Not marking function async

```typescript
// WRONG - Function not async
function readFile(filename: string) {
    const path = new Path(filename)
    return await path.readString()  // Error: await only in async functions
}

// CORRECT
async function readFile(filename: string) {
    const path = new Path(filename)
    return await path.readString()
}
```

### ❌ Mistake 3: Sequential when parallel is better

```typescript
// INEFFICIENT - Sequential I/O
async function readMultipleFiles(files: string[]) {
    const results = []
    for (const file of files) {
        results.push(await new Path(file).readString())
    }
    return results
}

// BETTER - Parallel I/O
async function readMultipleFiles(files: string[]) {
    return Promise.all(files.map(file =>
        new Path(file).readString()
    ))
}
```

### ❌ Mistake 4: Not handling errors

```typescript
// RISKY - No error handling
async function processFile(filename: string) {
    const content = await new Path(filename).readString()
    return content.toUpperCase()  // Fails if file doesn't exist
}

// BETTER - With error handling
async function processFile(filename: string) {
    try {
        const content = await new Path(filename).readString()
        return content ? content.toUpperCase() : ''
    } catch (error) {
        console.error(`Failed to read ${filename}:`, error)
        return ''
    }
}
```

---

## Implementation Checklist

### For Each Method Conversion

- [ ] Update method signature to async
- [ ] Replace sync fs calls with async equivalents
- [ ] Update return type to Promise<T>
- [ ] Add proper error handling
- [ ] Update JSDoc comments
- [ ] Update TypeScript types
- [ ] Update tests to use await
- [ ] Verify error cases still work
- [ ] Test concurrent operations

### For Each Class

- [ ] All I/O methods converted
- [ ] All tests updated and passing
- [ ] Type definitions updated
- [ ] JSDoc comments updated
- [ ] Examples updated
- [ ] Integration tests pass

### Project-Wide

- [ ] All 24 methods converted
- [ ] All 1207+ tests passing
- [ ] All examples updated
- [ ] All documentation updated
- [ ] CHANGELOG.md updated
- [ ] Version bumped to 2.0.0
- [ ] Migration guide created

---

## Summary

**Methods to convert:** 24 total
- **Path:** 12 methods (readBytes, readString, readJSON, readLines, write, append, copy, remove, removeAll, truncate, rename, makeDir)
- **File:** 8 methods (read, readBytes, readString, readLines, write, writeLine, open, close)
- **Cmd:** 4 methods (read, readString, readLines, Cmd.run)

**Methods staying sync:** ~50+ methods (properties, metadata, path manipulation)

**Breaking change:** YES - Requires v2.0.0
**Migration effort:** Low - Add `await` keywords
**Benefits:** Non-blocking I/O, better concurrency, modern patterns

---

**See also:**
- [Full Implementation Plan](ASYNC_CONVERSION_PLAN.md) - Detailed conversion plan
- [Design Document](DESIGN.md) - Overall architecture
- [API Compatibility](API_COMPATIBILITY.md) - API compatibility guide

**Status:** ✅ Analysis Complete - Ready for Implementation
