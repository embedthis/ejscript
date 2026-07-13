# API Audit: Ejscript vs Original Ejscript

**Project**: Ejscript (Ejscript for Bun)
**Audit Date**: 2025-10-17
**Auditor**: Claude Code
**Source**: `/Users/mob/dev2/ejscript`

## Purpose

This document audits the Ejscript implementation against the original Ejscript source code to identify:
- ✅ Complete API compatibility
- ⚠️ Partial compatibility (similar but different)
- ❌ Missing methods/properties
- 🆕 New methods not in original

## Status Legend

- ✅ **COMPLETE** - Fully implemented and compatible
- ⚠️ **PARTIAL** - Implemented but with differences
- ❌ **MISSING** - Not implemented
- 🆕 **ADDED** - New feature not in original Ejscript
- 🔍 **NEEDS AUDIT** - Not yet audited

## Emitter Class

**Source Files**:
- Original: `/Users/mob/dev2/ejscript/src/core/Emitter.es`
- Ejscript: `src/core/async/Emitter.ts`
- Tests: `test/emitter.test.ts` (92 tests)

### API Comparison

| Method/Property | Original | Ejscript | Status | Notes |
|----------------|----------|------|--------|-------|
| `on(name, callback)` | ✅ | ✅ | ✅ COMPLETE | Both support string/array for name |
| `off(name, callback)` | ✅ | ✅ | ✅ COMPLETE | Both support string/array |
| `fire(name, ...args)` | ✅ | ❌ | ⚠️ **RENAMED** | Ejscript uses `emit()` instead (Node.js convention) |
| `emit(name, ...args)` | ❌ | ✅ | 🆕 ADDED | Replaces `fire()` |
| `fireThis(name, thisObj, ...args)` | ✅ | ❌ | ❌ **MISSING** | Allows explicit `this` binding |
| `clearObservers(name?)` | ✅ | ❌ | ⚠️ **RENAMED** | Ejscript uses `removeAllListeners()` |
| `removeAllListeners(name?)` | ❌ | ✅ | 🆕 ADDED | Replaces `clearObservers()` |
| `hasObservers()` | ✅ | ❌ | ❌ **MISSING** | Returns boolean if any observers exist |
| `getObservers(name)` | ✅ | ❌ | ❌ **MISSING** | Returns cloned array of observers |
| `delayedFire(name, delay, ...args)` | ⚠️ | ❌ | ❌ **MISSING** | Marked TODO in original |
| `once(name, callback)` | ❌ | ✅ | 🆕 ADDED | One-time listener (common Node.js pattern) |
| `listenerCount(name)` | ❌ | ✅ | 🆕 ADDED | Count listeners for event |
| `eventNames()` | ❌ | ✅ | 🆕 ADDED | Get all event names |

### Critical Differences

#### 1. Callback Signature **INCOMPATIBLE**

**Original Ejscript**:
```javascript
events.on("test", function(eventName, arg1, arg2) {
    // eventName is "test", args follow
})
events.fire("test", "value1", "value2")
```

**Ejscript**:
```typescript
emitter.on("test", (arg1, arg2) => {
    // No event name passed!
})
emitter.emit("test", "value1", "value2")
```

**Impact**: ❌ **CRITICAL INCOMPATIBILITY** - All callbacks expect different arguments!

#### 2. Re-entrancy Protection **MISSING**

**Original**: Has `active`/`pending` mechanism to handle recursive fires
**Ejscript**: Uses array copy but doesn't handle re-entrancy properly

**Impact**: ⚠️ Medium - Can cause issues with nested event emissions

#### 3. Meta-events **MISSING**

**Original**: Fires "on" and "off" events when observers added/removed
**Ejscript**: No meta-events

**Impact**: 🟡 Low - Nice-to-have feature for debugging

#### 4. Method Names **INCOMPATIBLE**

**Original**: `fire()`, `clearObservers()`
**Ejscript**: `emit()`, `removeAllListeners()`

**Impact**: ⚠️ Medium - Breaking API change, but documented

### Test Coverage Assessment

**Current Tests**: 92 tests covering:
- ✅ on(), off(), emit() thoroughly tested
- ✅ once() well tested (not in original)
- ✅ listenerCount(), eventNames() tested (not in original)
- ❌ fireThis() not tested (not implemented)
- ❌ hasObservers() not tested (not implemented)
- ❌ getObservers() not tested (not implemented)
- ❌ Callback signature incompatibility not documented in tests
- ❌ Re-entrancy not tested

**Grade**: C+ (Good coverage of what's implemented, but missing critical original features)

### Recommendations

1. **CRITICAL**: Document callback signature incompatibility prominently
2. **HIGH**: Add `fire()` as alias for `emit()` for compatibility
3. **HIGH**: Implement `fireThis()` for explicit this binding
4. **MEDIUM**: Add `hasObservers()` method
5. **MEDIUM**: Add `getObservers()` method
6. **MEDIUM**: Add `clearObservers()` as alias for `removeAllListeners()`
7. **LOW**: Implement meta-events ("on", "off")
8. **LOW**: Add re-entrancy protection

## Http Class

**Status**: 🔍 NEEDS AUDIT

**Source Files**:
- Original: `/Users/mob/dev2/ejscript/src/core/Http.es`
- Ejscript: `src/core/Http.ts`
- Tests: `test/http.test.ts` (105 tests)

**Action**: Need to audit against original Ejscript Http API

## App Class

**Status**: 🔍 NEEDS AUDIT

**Source Files**:
- Original: `/Users/mob/dev2/ejscript/src/core/App.es`
- Ejscript: `src/core/App.ts`
- Tests: `test/app.test.ts` (64 tests)

**Action**: Need to audit against original Ejscript App API

## Type Extensions

**Status**: 🔍 NEEDS AUDIT

**Source Files**:
- Original: `/Users/mob/dev2/ejscript/src/core/*.es` (String, Array, Date, Number, Object)
- Ejscript: `src/core/types/*Extensions.ts`
- Tests: 245 tests across 4 files

**Action**: Need to audit each extension against original

## Summary

### Overall Assessment

**Files Audited**: 1 of 12 tested classes (8%)
**Compatibility Issues Found**: CRITICAL (Emitter callback signature)

### Critical Findings

1. **Emitter Callback Signature** ❌ CRITICAL
   - Original passes event name as first argument
   - Ejscript does not pass event name
   - **All Emitter users will break on migration**

2. **Method Name Changes** ⚠️ MEDIUM
   - `fire()` → `emit()`
   - `clearObservers()` → `removeAllListeners()`
   - Need aliases or migration guide

3. **Missing Methods** ⚠️ MEDIUM
   - `fireThis()`, `hasObservers()`, `getObservers()`
   - May break code depending on these

### Next Steps

1. ✅ Complete audit of Emitter (DONE)
2. 🔲 Audit Http class
3. 🔲 Audit App class
4. 🔲 Audit all type extensions
5. 🔲 Create comprehensive compatibility matrix
6. 🔲 Update tests to verify original API compatibility
7. 🔲 Document all breaking changes
8. 🔲 Provide migration guide

### Test Coverage Reality Check

**What We Tested**: Our implementations (possibly incorrect)
**What We Should Test**: Original Ejscript API compatibility

**Current State**: 528 tests passing, but testing wrong API!

**Required**: Audit-driven test development
1. Read original Ejscript source
2. Document all methods/properties
3. Verify Ejscript implementation matches
4. Write tests for original API
5. Fix incompatibilities

## Conclusion

The test suite is comprehensive for what's implemented, but there are **critical API incompatibilities** with original Ejscript that were not caught because we didn't audit against the source.

**Priority**: HALT further test development until audit is complete.

**Recommendation**: Systematic audit of all classes against `/Users/mob/dev2/ejscript/src/` before claiming production readiness.
