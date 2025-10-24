# Ejscript Development Procedures

**Project**: Ejscript (Ejscript for Bun)
**Last Updated**: 2025-10-20

## Contents

1. [Development Environment Setup](#development-environment-setup)
2. [Building the Project](#building-the-project)
3. [Testing Procedures](#testing-procedures)
4. [Adding New Features](#adding-new-features)
5. [Code Review Process](#code-review-process)
6. [Release Procedures](#release-procedures)
7. [Troubleshooting](#troubleshooting)

## Development Environment Setup

### Prerequisites

**Required**:
- Bun 1.0 or later
- TypeScript 5.0 or later
- Git

**Recommended**:
- VS Code or similar IDE with TypeScript support
- ESLint/Prettier for code formatting

### Initial Setup

1. **Clone the repository**:
```bash
cd /Users/mob/c/ejsx
```

2. **Install dependencies**:
```bash
bun install
```

3. **Verify installation**:
```bash
# Type check
bun run typecheck

# Run tests
bun test

# Run example
bun examples/basic.ts
```

### IDE Configuration

**VS Code settings** (recommended):
```json
{
  "typescript.tsdk": "node_modules/typescript/lib",
  "typescript.enablePromptUseWorkspaceTsdk": true,
  "editor.formatOnSave": true,
  "editor.codeActionsOnSave": {
    "source.fixAll": true
  }
}
```

## Building the Project

### Development Build

```bash
# Type check only (no output files)
bun run typecheck

# Full build (generates dist/ with .js and .d.ts files)
bun run build
```

### Build Output

The build process:
1. Cleans the `dist/` directory
2. Runs TypeScript compiler
3. Generates JavaScript files in `dist/`
4. Generates type definition files (.d.ts)
5. Generates source maps

### Continuous Development

```bash
# Watch mode - rebuilds on file changes
bun run dev
```

## Testing Procedures

### Running Tests

**IMPORTANT**: This project uses TestMe for all unit tests. Tests use `.tst.ts` extension.

**All tests**:
```bash
tm                    # Run all tests (from test/ directory)
bun test             # Alternative: run via npm script
```

**Specific test file**:
```bash
tm test/path.tst.ts                    # Run specific test
tm test/core/http-partial-urls.tst.ts  # Run specific core test
```

**Watch mode** (not available in TestMe - run tests manually after changes)

### Writing Tests

**IMPORTANT**: All tests must use TestMe framework with `.tst.ts` extension.

**Test file structure**:
```typescript
import { describe, test, expect } from '@embedthis/testme'
import { Path } from '../../src/core/Path'

describe('Path', () => {
    test('should create absolute paths', () => {
        const p = new Path('/tmp/test')
        expect(p.isAbsolute).toBe(true)
        expect(p.name).toBe('/tmp/test')
    })

    test('should handle errors gracefully', () => {
        expect(() => {
            new Path('/nonexistent').readString()
        }).toThrow()
    })
})
```

**TestMe Best Practices**:
- Use `describe()` for test groups
- Use `test()` for individual tests (not `it()`)
- Use `expect()` for assertions with Jest-style matchers
- Test files must have `.tst.ts` extension
- Place tests in `test/` directory
- Use `getpid()` for unique temporary filenames
- Tests must be portable (Windows, macOS, Linux)
- Tests must run in parallel safely

**Note**: While this project currently uses Bun's test runner, the parent project standards require TestMe for unit testing. Future test development should use TestMe with Jest-style expect API.

### Test Organization

- Place tests in `test/` directory
- Mirror source structure: `src/core/Path.ts` → `test/core/path.test.ts`
- Use descriptive test names
- Group related tests with `describe` blocks
- Clean up resources (files, sockets) in `afterEach`

### Test Coverage

**Running coverage** (future):
```bash
bun test --coverage
```

**Coverage goals**:
- Core classes: 90%+
- Utilities: 80%+
- Type extensions: 100%

## Adding New Features

### Procedure for New Core Class

1. **Design phase**:
   - Document API in design doc
   - Review with maintainers
   - Consider backward compatibility

2. **Implementation**:
   ```bash
   # Create class file
   touch src/core/NewClass.ts

   # Create test file
   touch test/core/newclass.test.ts
   ```

3. **Class structure**:
   ```typescript
   /**
    * NewClass - Brief description
    *
    * Detailed description of what this class does
    * @spec ejs
    * @stability evolving
    */
   export class NewClass {
       /**
        * Constructor
        * @param param Description
        */
       constructor(param: string) {
           // Implementation
       }

       /**
        * Method description
        * @returns Return description
        */
       someMethod(): string {
           // Implementation
           return ''
       }
   }
   ```

4. **Export from index**:
   ```typescript
   // Add to src/index.ts
   export { NewClass } from './core/NewClass'
   ```

5. **Write tests**:
   ```typescript
   // test/core/newclass.test.ts
   import { describe, it, expect } from 'bun:test'
   import { NewClass } from '../../src/core/NewClass'

   describe('NewClass', () => {
       it('should work as expected', () => {
           const instance = new NewClass('test')
           expect(instance.someMethod()).toBe('test')
       })
   })
   ```

6. **Verify**:
   ```bash
   bun run typecheck
   bun test test/core/newclass.test.ts
   bun run build
   ```

7. **Document**:
   - Add JSDoc comments
   - Update README.md if user-facing
   - Update DESIGN.md

### Procedure for Type Extension

1. **Create extension file**:
   ```typescript
   // src/core/types/NewTypeExtensions.ts

   declare global {
       interface String {
           newMethod(): string
       }
   }

   String.prototype.newMethod = function(): string {
       return this.toUpperCase()
   }

   export {} // Makes this a module
   ```

2. **Export from index**:
   ```typescript
   // src/index.ts
   export * from './core/types/NewTypeExtensions'
   ```

3. **Write tests**:
   ```typescript
   import { describe, it, expect } from 'bun:test'
   import '../../src/core/types/NewTypeExtensions'

   describe('String extensions', () => {
       it('should have newMethod', () => {
           expect('hello'.newMethod()).toBe('HELLO')
       })
   })
   ```

### Procedure for Utility Function

1. **Add to appropriate utility file** or create new one
2. **Export from module and index**
3. **Write tests**
4. **Document usage**

## Code Review Process

### Before Submitting

**Self-review checklist**:
- [ ] Code compiles without errors: `bun run typecheck`
- [ ] All tests pass: `bun test`
- [ ] New tests written for new functionality
- [ ] JSDoc comments added for public APIs
- [ ] No console.log or debug code
- [ ] Error handling implemented
- [ ] Type safety maintained (no `any` types)

### Code Style Guidelines

**TypeScript**:
```typescript
// Good
export class Path {
    private _path: string

    constructor(pathString: string = '.') {
        this._path = pathString
    }

    get name(): string {
        return this._path
    }
}

// Bad
export class Path {
    path: any // Avoid 'any'

    constructor(p) { // Missing type annotation
        this.path = p
    }
}
```

**Naming conventions**:
- Classes: PascalCase (`Path`, `HttpClient`)
- Methods/functions: camelCase (`readString`, `toAbsolute`)
- Private members: underscore prefix (`_path`, `_buffer`)
- Constants: UPPER_SNAKE_CASE (`MAX_SIZE`, `DEFAULT_MODE`)

**Comments**:
```typescript
// Good - explains WHY
// Use immutable pattern to prevent accidental mutations
private _path: string

// Bad - explains WHAT (obvious from code)
// Store the path
private _path: string
```

### Review Criteria

**Functionality**:
- Does it work as intended?
- Are edge cases handled?
- Is error handling robust?

**API Design**:
- Is the API intuitive?
- Is it consistent with existing APIs?
- Is it documented?

**Code Quality**:
- Is it readable?
- Is it maintainable?
- Are there code smells?

**Testing**:
- Are tests comprehensive?
- Do tests cover edge cases?
- Are tests maintainable?

## Release Procedures

### Version Numbering

Follow **Semantic Versioning** (MAJOR.MINOR.PATCH):
- **PATCH**: Bug fixes (0.1.0 → 0.1.1)
- **MINOR**: New features, backward compatible (0.1.0 → 0.2.0)
- **MAJOR**: Breaking changes (0.1.0 → 1.0.0)

### Pre-Release Checklist

- [ ] All tests passing
- [ ] TypeScript compilation successful
- [ ] Examples running correctly
- [ ] Documentation updated
- [ ] CHANGELOG.md updated
- [ ] Version bumped in package.json

### Release Process

1. **Update version**:
   ```bash
   # Edit package.json
   vim package.json
   # Change version number
   ```

2. **Update CHANGELOG**:
   ```bash
   vim .agent/logs/CHANGELOG.md
   # Add new version section with changes
   ```

3. **Build and test**:
   ```bash
   bun run build
   bun test
   bun examples/basic.ts
   ```

4. **Commit changes**:
   ```bash
   git add package.json .agent/logs/CHANGELOG.md
   git commit -m "Release v0.2.0"
   git tag v0.2.0
   ```

5. **Publish** (if public):
   ```bash
   npm publish
   ```

### Post-Release

- Update documentation site (if any)
- Announce release (GitHub, social media, etc.)
- Monitor for issues
- Plan next release

## Troubleshooting

### TypeScript Compilation Errors

**Error**: "Cannot find module"
```bash
# Solution: Ensure all imports use correct paths
# Check tsconfig.json paths configuration
bun run typecheck
```

**Error**: "Type 'X' is not assignable to type 'Y'"
```typescript
// Solution: Add proper type annotations
// Use type assertions sparingly
const result = someFunction() as ExpectedType
```

### Test Failures

**Tests fail locally**:
```bash
# Clean build
rm -rf dist node_modules
bun install
bun test
```

**Tests pass locally but fail in CI**:
- Check for platform-specific issues (path separators, line endings)
- Check for timing issues (use proper async/await)
- Check for file system race conditions

### Build Issues

**Build fails**:
```bash
# Check TypeScript version
bun --version
tsc --version

# Clean and rebuild
rm -rf dist
bun run build
```

**Out of memory**:
```bash
# Increase Node.js memory (if needed)
NODE_OPTIONS=--max-old-space-size=4096 bun run build
```

### Runtime Issues

**Module not found**:
- Check imports are using correct paths
- Ensure module is exported from index.ts
- Check package.json exports field

**Performance issues**:
- Profile with Bun's built-in profiler
- Check for unnecessary file system operations
- Look for memory leaks (large objects not being GC'd)

### Getting Help

1. **Check documentation**:
   - README.md
   - DESIGN.md
   - This PROCEDURES.md

2. **Search issues**:
   - Check existing GitHub issues
   - Look for similar problems

3. **Ask for help**:
   - Open GitHub issue with:
     - Minimal reproduction
     - Expected vs actual behavior
     - Environment details (Bun version, OS, etc.)

## Common Tasks Quick Reference

```bash
# Setup
bun install

# Development
bun run typecheck        # Type check
bun run dev             # Watch mode
bun test                # Run tests
bun examples/basic.ts   # Run example

# Building
bun run build           # Production build

# Testing
bun test                           # All tests
bun test test/core/path.test.ts   # Specific test
bun test --watch                   # Watch mode

# Maintenance
bun update              # Update dependencies
rm -rf node_modules dist && bun install  # Clean install
```

## References

- [Bun Documentation](https://bun.sh/docs)
- [TypeScript Handbook](https://www.typescriptlang.org/docs/)
- [DESIGN.md](../designs/DESIGN.md) - Architecture details
- [PLAN.md](../plans/PLAN.md) - Project roadmap
