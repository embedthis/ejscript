# ejsx → TestMe Quick Migration Checklist

**Time Required:** 2-4 hours
**Difficulty:** ⭐ Easy (99% automated)

---

## ✅ Pre-Migration Checklist

- [ ] Read [TESTME_MIGRATION_GUIDE.md](./TESTME_MIGRATION_GUIDE.md) (comprehensive guide)
- [ ] Read [MIGRATION_SUMMARY.md](./MIGRATION_SUMMARY.md) (impact analysis)
- [ ] Backup current tests: `cp -r test test.backup`
- [ ] Ensure all current tests pass with Bun: `bun test`

---

## 🚀 Quick Migration (Copy-Paste Commands)

### 1. Install TestMe (30 seconds)

```bash
cd /Users/mob/c/ejsx
bun add -d @embedthis/testme
```

### 2. Update All Import Statements (1 minute)

```bash
# Automated replacement
find test -name "*.test.ts" -type f -exec sed -i '' \
  "s/from 'bun:test'/from 'testme'/g" {} +

# Verify changes
grep -r "from 'testme'" test --include="*.test.ts" | wc -l
# Expected: 25 files
```

### 3. Create TestMe Configuration (30 seconds)

```bash
cat > test/testme.json5 << 'EOF'
{
    patterns: {
        include: ['**/*.test.ts'],
        exclude: ['**/node_modules/**', '**/dist/**', '**/.testme/**']
    },
    execution: {
        timeout: 30,
        parallel: true,
        workers: 4
    },
    output: {
        verbose: false,
        format: 'simple',
        colors: true
    }
}
EOF
```

### 4. Fix Helper Function (2 minutes)

Edit `test/helpers.ts` and change line 16:

**Before:**
```typescript
export function assert(condition: boolean, message?: string): void {
    expect(condition).withContext(message || 'Assertion failed').toBeTruthy()
}
```

**After:**
```typescript
export function assert(condition: boolean, message?: string): void {
    if (!condition) {
        throw new Error(message || 'Assertion failed')
    }
}
```

### 5. Update package.json (30 seconds)

```bash
# Option 1: Manual edit
# Change "test": "bun test" to "test": "tm"

# Option 2: Use sed
sed -i '' 's/"test": "bun test"/"test": "tm"/' package.json
```

### 6. Update .gitignore (10 seconds)

```bash
echo -e "\n# TestMe artifacts\n.testme/" >> .gitignore
```

### 7. Run Tests (30 seconds)

```bash
npm test
# or
tm
```

---

## ✅ Verification Checklist

After migration, verify:

- [ ] All 25 test files run without errors
- [ ] Import statements changed: `grep "from 'testme'" test/**/*.test.ts`
- [ ] No `.withContext()` errors
- [ ] beforeAll/afterAll hooks execute correctly
- [ ] test.skip() and test.skipIf() work
- [ ] All expect() assertions pass
- [ ] Test output is clean and readable
- [ ] Execution time is reasonable (<10 seconds)

---

## 🎯 Expected Results

### Import Verification
```bash
$ grep -r "from 'testme'" test --include="*.test.ts" | wc -l
25
```

### Test Execution
```bash
$ npm test
✓ Passed: 100+
✗ Failed: 0
Total: 100+
Duration: ~5s
Result: PASSED
```

---

## 🆘 Troubleshooting

### Error: "Cannot find module 'testme'"
```bash
bun add -d @embedthis/testme
```

### Error: "withContext is not a function"
Edit `test/helpers.ts` - see step 4 above

### Error: Tests run but fail unexpectedly
```bash
# Check import statements
grep "bun:test" test/**/*.test.ts
# Should return nothing

# Run with verbose output
tm --verbose
```

### Error: "describe is not defined"
Ensure top-level describe is awaited:
```typescript
await describe('Test', async () => { ... })
```

---

## 🔄 Rollback (if needed)

```bash
rm -rf test
mv test.backup test
bun remove @embedthis/testme
git checkout package.json .gitignore
bun test  # Back to Bun
```

---

## 📊 Progress Tracker

Track your migration:

```
Phase 1: Setup
[ ] Install TestMe
[ ] Backup tests

Phase 2: Code Changes
[ ] Update imports (25 files)
[ ] Fix helpers.ts (1 file)
[ ] Create testme.json5 (1 file)
[ ] Update package.json (1 file)
[ ] Update .gitignore (1 file)

Phase 3: Testing
[ ] Run test suite
[ ] Verify all pass
[ ] Check for errors

Phase 4: Cleanup
[ ] Commit changes
[ ] Remove backup
[ ] Update team

Status: ___% Complete
Time Spent: ___ hours
```

---

## 📝 One-Liner Migration Script

For the brave, here's a one-liner that does everything:

```bash
bun add -d @embedthis/testme && \
find test -name "*.test.ts" -type f -exec sed -i '' "s/from 'bun:test'/from 'testme'/g" {} + && \
cat > test/testme.json5 << 'EOF'
{
    patterns: { include: ['**/*.test.ts'], exclude: ['**/node_modules/**', '**/.testme/**'] },
    execution: { timeout: 30, parallel: true, workers: 4 }
}
EOF
echo -e "\n.testme/" >> .gitignore && \
echo "✅ Migration complete! Run: npm test"
```

**Note:** Still need to manually fix `test/helpers.ts` (step 4)

---

## 🎓 Key Differences: Bun vs TestMe

| Feature | Bun Command | TestMe Command |
|---------|-------------|----------------|
| Run all tests | `bun test` | `tm` or `npm test` |
| Run specific file | `bun test path.test.ts` | `tm path.test.ts` |
| Run by pattern | N/A | `tm path` |
| Verbose output | `bun test --verbose` | `tm --verbose` |
| List tests | N/A | `tm --list` |
| Clean artifacts | N/A | `tm --clean` |

---

## ✨ What Works Identically

**No changes needed for:**
- ✅ All `describe()` blocks
- ✅ All `test()` and `it()` calls
- ✅ All `beforeAll()` and `afterAll()` hooks
- ✅ All `beforeEach()` and `afterEach()` hooks
- ✅ All `test.skip()` and `test.skipIf()` calls
- ✅ All `expect()` matchers (toBe, toEqual, toContain, etc.)
- ✅ All async test functions
- ✅ All test helper functions (except .withContext())
- ✅ All nested describe blocks
- ✅ All platform-specific tests

**Bottom line:** 99% of your code works without modification ✅

---

## 📞 Support

**Documentation:**
- Full Guide: [TESTME_MIGRATION_GUIDE.md](./TESTME_MIGRATION_GUIDE.md)
- Summary: [MIGRATION_SUMMARY.md](./MIGRATION_SUMMARY.md)
- TestMe Docs: [/Users/mob/c/testme/README-JS.md](/Users/mob/c/testme/README-JS.md)

**Need Help?**
- Run tests with verbose: `tm --verbose`
- Check TestMe issues: `https://github.com/anthropics/testme/issues`

---

**Last Updated:** 2025-10-18
**TestMe Version:** Latest (with beforeAll/afterAll support)
**ejsx Version:** 1.0.0
