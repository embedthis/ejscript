# Migration to Ejscript for Bun

This directory contains migration guides for two scenarios:

---

## Use Case 1: Migrating Ejscript Applications

**You have**: An application written in native Ejscript
**You want**: Run it on Bun using Ejscript for Bun (TypeScript port)
**Read**: [APP_MIGRATION.md](./APP_MIGRATION.md)

**Key Changes**:
- Add ES6 imports
- Update to async I/O patterns (v2.0.0)
- Language syntax changes (`for each` → `for...of`, multiline strings, etc.)

**Estimated Effort**: Depends on app size, ~1-2 hours per 1000 lines with tools

---

## Use Case 2: Migrating Ejscript Tests

**You have**: Test suite written for native Ejscript using legacy TestMe
**You want**: Run tests on new TestMe with Bun + Ejscript for Bun
**Read**: [TEST_MIGRATION.md](./TEST_MIGRATION.md)

**Key Changes**:
- Add ES6 imports (TestMe + Ejscript)
- Update test assertions to use new TestMe
- Update to async I/O patterns (v2.0.0)
- Language syntax changes

**Estimated Effort**: ~1-2 hours per 20 test files with automation

---

## Quick Decision

**Which guide do I need?**

```
What are you migrating?
│
├─ Application code (*.es, *.js, *.ts)
│  └─> Use APP_MIGRATION.md
│
└─ Test suite (*.tst, *.tst.ts)
   └─> Use TEST_MIGRATION.md
```

**Both?** Read both guides - they share common patterns but have test-specific guidance.

---

## Additional Resources

- [../COMPATIBILITY.md](../COMPATIBILITY.md) - Complete API compatibility reference
- [../README.md](../README.md) - Project overview
- [../QUICK_START.md](../QUICK_START.md) - Getting started with Ejscript for Bun

---

*Last Updated: 2025-10-27*
