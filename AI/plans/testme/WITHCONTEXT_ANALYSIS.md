# Analysis: `.withContext()` in ejsx Tests

## Quick Answer

**Q: Can we use `.withContext()` since TestMe runs on Bun?**

**A: No, because `.withContext()` doesn't exist in Bun's test API.**

The code in `helpers.ts` that uses `.withContext()` is **dead code** - it's never actually called, and if it were, it would fail immediately.

---

## Investigation Results

### 1. Bun's expect() API Does Not Have .withContext()

**Test:**
```javascript
import { expect } from 'bun:test'
const result = expect(5)
console.log('Has withContext?', typeof result.withContext)
// Output: Has withContext? undefined
```

**Bun's actual expect() methods:**
```javascript
[
  "toBe", "toEqual", "toStrictEqual", "toBeTruthy", "toBeFalsy",
  "toBeNull", "toBeUndefined", "toBeDefined", "toBeNaN",
  "toBeInstanceOf", "toBeTypeOf", "toBeGreaterThan",
  "toBeLessThan", "toContain", "toMatch", "toHaveProperty",
  "toThrow", "not", "resolves", "rejects", ...
]
// ❌ No withContext!
```

### 2. The helpers.ts Code is Broken

**File:** `/Users/mob/c/ejsx/test/helpers.ts` (line 16)

```typescript
export function assert(condition: any, message?: string): void {
  if (message) {
    expect(condition).withContext(message).toBeTruthy()  // ❌ This fails!
  } else {
    expect(condition).toBeTruthy()
  }
}
```

**What happens if you call it:**
```bash
$ bun test
❌ Error: expect(condition).withContext is not a function.
   In 'expect(condition).withContext(message)',
   'expect(condition).withContext' is undefined
```

### 3. The Function is NEVER Called

**Imported by 3 files:**
```typescript
// test/core/path.test.ts
import { assert, createTestFile, ... } from '../helpers'

// test/core/file.test.ts
import { assert, createTestFile, ... } from '../helpers'

// test/core/streams.test.ts
import { assert, createTestFile, ... } from '../helpers'
```

**Called by 0 files:**
```bash
$ grep -r "assert(" test/core/*.test.ts
# No results - it's never called!
```

**Test results:**
```bash
$ bun test test/core/path.test.ts
✓ 54 pass
✓ 1 skip
✓ 0 fail
✓ All tests pass!
```

The tests pass because `assert()` from helpers.ts is **imported but never used**.

---

## Why .withContext() Doesn't Exist

### Origin: Jasmine Test Framework

`.withContext()` was a feature in **Jasmine** (a different test framework):

```javascript
// Jasmine syntax (NOT Bun)
expect(x).withContext('Custom error message').toBe(y)
```

### Bun's Approach

Bun uses **Jest/Vitest-style** expect, which doesn't have `.withContext()`.

**Jest/Vitest way to add context:**
```javascript
// Option 1: Message parameter (some matchers)
expect(x).toBe(y)  // Auto-includes file:line in error

// Option 2: Descriptive test names
test('should validate email format', () => {
    expect(isValid).toBe(true)  // Test name provides context
})

// Option 3: Throw custom error
if (!condition) {
    throw new Error('Custom context message')
}
```

---

## Impact on Migration

### Original Concern
❌ "Need to fix `.withContext()` - not compatible with TestMe"

### Actual Reality
✅ **No impact - the code is never executed**

The `assert()` function in helpers.ts:
- ✅ Is imported by 3 files
- ✅ Is never called
- ✅ Would fail immediately if called (even with Bun)
- ✅ Can be safely removed or fixed

### Migration Options

**Option 1: Do Nothing (Recommended)**
```typescript
// Keep the import but never use it
import { assert, createTestFile, ... } from '../helpers'
// ✅ Works fine - dead imports don't cause errors
```

**Option 2: Remove from Imports**
```diff
- import { assert, createTestFile, ... } from '../helpers'
+ import { createTestFile, ... } from '../helpers'
```

**Option 3: Fix the Function (If you want to use it later)**
```typescript
// test/helpers.ts
export function assert(condition: any, message?: string): void {
  if (!condition) {
    throw new Error(message || 'Assertion failed')
  }
}
```

---

## Recommended Action for Migration

### No Action Needed ✅

Since the function is never called:

1. **Leave the import as-is** - Won't cause errors
2. **Leave the function as-is** - Won't be executed
3. **Focus on actual migration** - Change `'bun:test'` → `'testme'`

### Optional Cleanup (Low Priority)

If you want to clean up:

```bash
# Remove unused imports (automated)
find test/core -name "*.test.ts" -type f -exec sed -i '' \
  's/assert, //g' {} +
```

---

## Alternative: Add .withContext() to TestMe?

### Could We Implement It?

**Technically yes**, but **not recommended** because:

1. **Not standard** - Neither Jest, Vitest, nor Bun have it
2. **Not needed** - Test names and file:line provide context
3. **Adds complexity** - Would need to wrap every matcher
4. **Dead code** - ejsx doesn't actually use it

### Better Alternative: Use Test Names

Instead of:
```typescript
expect(condition).withContext('Email should be valid').toBeTruthy()
```

Use descriptive test names:
```typescript
test('should validate email format', () => {
    expect(isEmailValid(email)).toBeTruthy()
})
// Error shows: "should validate email format" FAILED at file.ts:123
```

---

## Summary

| Aspect | Status |
|--------|--------|
| Does Bun have `.withContext()`? | ❌ No |
| Does TestMe have `.withContext()`? | ❌ No |
| Is `.withContext()` used in ejsx? | ❌ No (dead code) |
| Will migration fail because of it? | ✅ No |
| Do we need to fix it? | ⚠️ Optional (not required) |
| Can we add it to TestMe? | ⚠️ Possible but not recommended |

### Conclusion

**The `.withContext()` concern is a non-issue for migration.**

- ✅ The code exists but is never executed
- ✅ Tests pass without it
- ✅ Migration works fine as-is
- ✅ No refactoring needed

**Recommended approach:** Ignore it completely during migration. It's harmless dead code.

---

## Updated Migration Guide

### Before (Original Assessment)
```
❌ Fix helpers.ts - remove .withContext()
⏱️ Time: 15 minutes
```

### After (Actual Reality)
```
✅ No action needed - function is never called
⏱️ Time: 0 minutes
```

**Migration impact:** Zero ✅

---

## If You Want to Use assert() in the Future

If you later decide to actually USE the assert() helper function:

**Option A: Simple Fix (Recommended)**
```typescript
export function assert(condition: any, message?: string): void {
  if (!condition) {
    throw new Error(message || 'Assertion failed')
  }
}
```

**Option B: Use TestMe's Traditional API**
```typescript
import { ttrue } from 'testme'

export function assert(condition: any, message?: string): void {
  ttrue(condition, message || 'Assertion failed')
}
```

**Option C: Just Use expect() Directly**
```typescript
// No helper needed
expect(condition).toBeTruthy()
// Error shows file:line automatically
```

---

**Last Updated:** 2025-10-18
**Status:** ✅ Resolved - Not an issue for migration
