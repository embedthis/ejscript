# Quick Start Guide

## Installation

```bash
cd ejscript-bun
bun install
```

## Running Examples

```bash
# Basic example
bun examples/basic.ts
```

## Using in Your Project

```bash
# From this directory
bun add file:../ejscript-bun
```

Then in your code:

```typescript
import { Path, File, Http, App } from 'ejscript'

// Path operations
const file = new Path('/tmp/test.txt')
file.write('Hello from Ejscript!')
console.log(file.readString())

// HTTP requests
const http = new Http()
http.get('https://api.github.com')
console.log(http.status, http.statusMessage)

// Application info
console.log('Working directory:', App.dir.name)
console.log('Arguments:', App.args)
```

## Running Tests

```bash
bun test
```

## Building

```bash
bun run build
```

## API Documentation

See the original Ejscript files in `src/` for detailed JSDoc comments on each class and method.

Main classes:
- **Path** - File path manipulation
- **File** - File I/O
- **Http** - HTTP client
- **App** - Application singleton
- **System** - System information
- **Uri** - URI parsing
- **Cache** - In-memory caching
- **Logger** - Logging framework

For full documentation, see IMPLEMENTATION.md
