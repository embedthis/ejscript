# Systematic API Audit Plan

**Goal**: Ensure 100% API compatibility at method/property level with original Ejscript
**Source**: `/Users/mob/dev2/ejscript/src/`
**Status**: IN PROGRESS

## Audit Checklist

### Core Classes

- [x] ✅ **Emitter** - COMPLETE (added fire, fireThis, hasObservers, getObservers, clearObservers)
- [ ] 🔲 **Http** - IN PROGRESS (missing: addHeader, afetch, publicForm, setCallback, several properties)
- [ ] 🔲 **App**
- [ ] 🔲 **Path**
- [ ] 🔲 **File**

### Streams

- [ ] 🔲 **ByteArray**
- [ ] 🔲 **TextStream**
- [ ] 🔲 **BinaryStream**
- [ ] 🔲 **Stream** (interface)

### Type Extensions

- [ ] 🔲 **String**
- [ ] 🔲 **Array**
- [ ] 🔲 **Date**
- [ ] 🔲 **Number**
- [ ] 🔲 **Object**

### Utilities

- [ ] 🔲 **Uri**
- [ ] 🔲 **Global**
- [ ] 🔲 **Cache**
- [ ] 🔲 **Logger**
- [ ] 🔲 **Timer**
- [ ] 🔲 **Cmd**
- [ ] 🔲 **Inflector**
- [ ] 🔲 **Memory**
- [ ] 🔲 **GC**

### Async

- [ ] 🔲 **Worker**

### Network

- [ ] 🔲 **Socket**
- [ ] 🔲 **WebSocket**

### System

- [ ] 🔲 **Config**
- [ ] 🔲 **System**
- [ ] 🔲 **FileSystem**

## Quick Audit Process

For each class:
1. Extract methods from original: `grep -E "^\s+function " /Users/mob/dev2/ejscript/src/core/ClassName.es`
2. Extract methods from Ejscript: `grep -E "^\s+\w+\(" src/core/ClassName.ts`
3. Compare and identify gaps
4. Add missing methods
5. Add tests for new methods
6. Mark complete

## Priority Order

**Phase 1: Already Tested Classes** (ensure completeness)
1. Emitter ✅
2. Http (partially tested - 105 tests)
3. App (partially tested - 64 tests)
4. Type Extensions (fully tested - 245 tests)
5. Path, File, Streams (fully tested)

**Phase 2: Untested Classes**
6. Uri, Global, Cache, Logger, Timer
7. Socket, WebSocket, Worker
8. Config, System, FileSystem

## Current Status

**Classes Audited**: 1/33 (3%)
**Test Coverage**: 547/568 passing (96%)
**Missing Methods Found**: ~15 in Http alone

## Next Actions

1. Complete Http audit and fixes
2. Audit App
3. Audit type extensions (String, Array, Date, Number, Object)
4. Continue systematically through all classes
