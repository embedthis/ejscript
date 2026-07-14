# Local Package Linking Guide

This guide explains how to use the ejscript package in other local projects during development.

## Publishing for Local Development

The package has been published locally using `bun link`. This creates a symbolic link in Bun's global registry that points to this development directory.

### What's Already Done

```bash
# These steps have already been completed:
bun run build      # Built the package
bun link           # Created global symlink
```

The symlink is located at:
```
~/.bun/install/global/node_modules/ejscript -> <your-project-path>/ejs
```

## Using in Other Projects

To use this package in another local project:

### Option 1: Using bun link (Recommended for Development)

In your other project directory:

```bash
cd /path/to/your/project
bun link @embedthis/ejscript
```

Then in your TypeScript/JavaScript files:

```typescript
import { Path, File, Http } from '@embedthis/ejscript'

// Use the classes
const path = new Path('/tmp/test.txt')
const http = new Http()
http.get('4100/index.html')  // Partial URL support!
await http.wait()
```

### Option 2: Using package.json dependency

Add to your `package.json`:

```json
{
  "dependencies": {
    "ejscript": "link:ejscript"
  }
}
```

Then run:
```bash
bun install
```

## Making Changes

When you make changes to this package:

1. **Rebuild** (if you changed TypeScript source):
   ```bash
   cd <your-project-path>/ejs
   bun run build
   ```

2. **No re-linking needed** - The symlink automatically points to the latest code!

3. **In consuming project** - Just restart your development server or re-run your script. The changes are immediately available.

## Testing Your Changes

You can test changes in two ways:

1. **In this package**:
   ```bash
   bun test
   bun docs/examples/basic.ts
   ```

2. **In a consuming project**:
   ```bash
   cd /path/to/your/project
   bun run your-script.ts  # Uses the latest code automatically
   ```

## Unlinking

If you need to unlink the package:

```bash
# In the consuming project
cd /path/to/your/project
bun unlink ejscript

# Or to remove the global link entirely
cd <your-project-path>/ejs
bun unlink
```

## Publishing to npm (Future)

When ready to publish to npm registry:

```bash
# Update version in package.json first
npm version patch  # or minor, or major

# Publish
npm publish
# or
bun publish
```

## Verifying the Link

Check that the symlink exists:
```bash
ls -la ~/.bun/install/global/node_modules/ | grep ejscript
```

Should show:
```
ejscript -> <your-project-path>/ejs
```

## Example Project Setup

Create a new test project:

```bash
mkdir /tmp/test-ejscript
cd /tmp/test-ejscript
bun init -y
bun link @embedthis/ejscript
```

Create `index.ts`:
```typescript
import { Http, Path } from '@embedthis/ejscript'

// Test partial URL completion
const http = new Http()
http.get('4100/test.html')
await http.wait()
console.log('Status:', http.status)

// Test Path operations
const path = new Path('/tmp/test.txt')
console.log('Basename:', path.basename)
```

Run it:
```bash
bun run index.ts
```

## Notes

- The linked package uses the **exact source code** from your local project directory
- Changes to source files are reflected immediately after rebuilding
- Type definitions are automatically updated when you rebuild
- All exports from `src/index.ts` are available
- No need to re-link after changes - the symlink persists

## Current Features

The linked package includes all ejscript features:

- ✅ 35+ core classes
- ✅ Full Path, File, Http API
- ✅ Stream system (ByteArray, TextStream, BinaryStream)
- ✅ Networking (Http, Socket, WebSocket)
- ✅ Utilities (Logger, Timer, Cache, Cmd, Uri)
- ✅ Type extensions (String, Array, Object, Date, Number)
- ✅ Global functions (blend, serialize, clone, md5, etc.)
- ✅ **NEW: Partial URL completion** in Http class

## Support

For issues or questions:
- Check the main [README.md](../README.md)
- Review [CLAUDE.md](../CLAUDE.md) for project details
- Run tests: `bun test`
