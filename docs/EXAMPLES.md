# Ejscript Examples

Practical examples demonstrating common use cases with Ejscript for Bun.

## Table of Contents

- [File Operations](#file-operations)
- [HTTP Client](#http-client)
- [Command Execution](#command-execution)
- [Data Processing](#data-processing)
- [Logging and Debugging](#logging-and-debugging)
- [Advanced Patterns](#advanced-patterns)

---

## File Operations

### Reading Files

```typescript
import { Path } from 'ejscript'

// Read entire file
const content = new Path('/tmp/data.txt').readString()

// Read lines
const lines = new Path('/tmp/data.txt').readLines()
for (const line of lines) {
    console.log(line)
}

// Read JSON
const data = new Path('/tmp/config.json').readJSON()
console.log(data.setting)

// Read with File class
import { File } from 'ejscript'
const file = new File('/tmp/data.txt', 'r')
const chunk = file.readString(1024)  // Read 1KB
file.close()
```

### Writing Files

```typescript
import { Path } from 'ejscript'

// Write string
new Path('/tmp/output.txt').write('Hello World')

// Append to file
new Path('/tmp/log.txt').append('New log entry\n')

// Write lines
new Path('/tmp/output.txt').writeLines([
    'Line 1',
    'Line 2',
    'Line 3'
])

// Write JSON
const data = { name: 'John', age: 30 }
new Path('/tmp/data.json').write(JSON.stringify(data, null, 2))

// Write with File class
import { File } from 'ejscript'
const file = new File('/tmp/output.txt', 'w')
file.encoding = 'utf-8'
file.write('Line 1\n')
file.write('Line 2\n')
file.close()
```

### File Management

```typescript
import { Path } from 'ejscript'

const source = new Path('/tmp/source.txt')
const dest = new Path('/tmp/backup')

// Copy file
source.copy(dest.join('source.txt'))

// Move/rename
source.move(new Path('/tmp/newname.txt'))

// Delete
new Path('/tmp/old.txt').remove()

// Check existence
if (new Path('/tmp/file.txt').exists) {
    console.log('File exists')
}

// Get file info
const file = new Path('/tmp/data.txt')
console.log('Size:', file.size)
console.log('Modified:', file.modified)
console.log('Created:', file.created)
```

### Directory Operations

```typescript
import { Path } from 'ejscript'

// Create directory
new Path('/tmp/mydir').makeDir()

// List files
const dir = new Path('/tmp/mydir')
const files = dir.files()
for (const file of files) {
    console.log(file.name)
}

// List with pattern
const txtFiles = dir.files('*.txt')

// Find files recursively
const allJs = new Path('/tmp/project').find('**/*.js')

// Iterate files
for (const file of dir.files()) {
    if (file.isDir) {
        console.log('Directory:', file.basename)
    } else {
        console.log('File:', file.basename, file.size, 'bytes')
    }
}
```

### Path Manipulation

```typescript
import { Path } from 'ejscript'

const p = new Path('/usr/local/bin/node')

// Get components
console.log(p.basename)      // 'node'
console.log(p.dirname)       // '/usr/local/bin'
console.log(p.extension)     // ''

// Build paths
const configFile = new Path('/etc').join('app', 'config.json')
// /etc/app/config.json

// Change extension
const backup = new Path('/tmp/data.json').replaceExt('.bak')
// /tmp/data.bak

// Relative path
const rel = new Path('/usr/local/bin').relative(new Path('/usr/local'))
// ../bin
```

---

## HTTP Client

### Basic Requests

```typescript
import { Http } from 'ejscript'

// GET request
const http = new Http()
http.get('https://api.github.com/users/octocat')
await http.wait()  // Wait for request to complete
console.log('Status:', http.status)
console.log('Response:', http.response)

// Parse JSON response
const data = JSON.parse(http.response)
console.log('Name:', data.name)

// POST with data
http.post('https://httpbin.org/post', 'key=value&foo=bar')
await http.wait()

// Static fetch (returns Promise directly)
const response = await Http.fetch('https://example.com')
console.log(response)
```

### JSON API Calls

```typescript
import { Http } from 'ejscript'

// POST JSON data
const http = new Http()
http.jsonForm('https://api.example.com/users', {
    name: 'John Doe',
    email: 'john@example.com',
    age: 30
})
await http.wait()

if (http.status === 201) {
    const user = JSON.parse(http.response)
    console.log('Created user:', user.id)
}

// PUT JSON
http.setHeader('Content-Type', 'application/json')
http.put('https://api.example.com/users/123',
    JSON.stringify({ name: 'Jane Doe' }))
await http.wait()
```

### Authentication

```typescript
import { Http } from 'ejscript'

// Basic auth
const http = new Http()
http.setCredentials('username', 'password', 'basic')
http.get('https://api.example.com/protected')
await http.wait()

// Digest auth (auto-detect)
http.setCredentials('user', 'pass')  // Server will send 401 with WWW-Authenticate
http.get('https://api.example.com/protected')
await http.wait()

// API key header
http.setHeader('Authorization', 'Bearer YOUR_API_KEY')
http.setHeader('X-API-Key', 'secret-key')
http.get('https://api.example.com/data')
await http.wait()
```

### File Upload

```typescript
import { Http } from 'ejscript'

// Upload single file
const http = new Http()
http.upload('https://api.example.com/upload', {
    file: '/tmp/document.pdf'
}, {
    description: 'My document',
    category: 'reports'
})
await http.wait()

// Upload multiple files
http.upload('https://api.example.com/upload', {
    file1: '/tmp/image1.jpg',
    file2: '/tmp/image2.jpg'
})
await http.wait()

// Streaming upload (large files)
const stream = Bun.file('/tmp/large-video.mp4').stream()
http.post('https://api.example.com/upload', stream)
await http.wait()
```

### Advanced HTTP

```typescript
import { Http } from 'ejscript'

const http = new Http()

// Custom headers
http.setHeader('User-Agent', 'MyApp/1.0')
http.setHeader('Accept', 'application/json')
http.setHeader('X-Request-ID', crypto.randomUUID())

// Follow redirects
http.followRedirects = true

// SSL verification
http.verify = true  // Verify SSL certificates

// Set timeout
http.setLimits({ requestTimeout: 30 })  // 30 seconds

// Make request
http.get('https://api.example.com/data')
await http.wait()

// Check response headers
console.log('Content-Type:', http.contentType)
console.log('Content-Length:', http.contentLength)
console.log('Date:', http.date)

// Get specific header
const rateLimit = http.header('X-RateLimit-Remaining')
```

### Streaming POST

```typescript
import { Http } from 'ejscript'

// Stream data incrementally
const http = new Http()
http.method = 'POST'
http.uri = 'https://api.example.com/stream'

http.write('{"events":[')
for (let i = 0; i < 100; i++) {
    if (i > 0) http.write(',')
    http.write(JSON.stringify({ id: i, time: Date.now() }))
}
http.write(']}')
http.finalize()

await http.connect('POST')
console.log('Status:', http.status)
```

---

## Command Execution

### Simple Commands

```typescript
import { Cmd } from 'ejscript'

// Execute and wait
const cmd = new Cmd('ls -la /tmp')
console.log('Exit code:', cmd.status)
console.log('Output:', cmd.response)

// Run shell script
const output = Cmd.sh(`
    echo "Starting..."
    ls -la
    echo "Done"
`)
console.log(output)

// Run with error handling
const git = new Cmd('git status')
if (git.status !== 0) {
    console.error('Git error:', git.error)
} else {
    console.log(git.response)
}
```

### Working with Processes

```typescript
import { Cmd } from 'ejscript'

// Set working directory
const cmd = new Cmd('npm install', { dir: '/tmp/myproject' })

// Detached process
const server = new Cmd()
server.start('node server.js', { detach: true })
console.log('Server PID:', server.pid)

// Wait for completion
await server.wait()

// Kill process
server.kill()
```

### Finding Programs

```typescript
import { Cmd } from 'ejscript'

// Locate program in PATH
const node = Cmd.locate('node')
if (node.exists) {
    console.log('Node found at:', node.name)

    // Use it
    const cmd = new Cmd([node.name, '--version'])
    console.log(cmd.response)
}

// Check multiple programs
const programs = ['git', 'node', 'npm', 'python3']
for (const prog of programs) {
    const path = Cmd.locate(prog)
    console.log(`${prog}: ${path.exists ? path.name : 'not found'}`)
}
```

### Pipelines

```typescript
import { Cmd } from 'ejscript'

// Execute pipeline
const result = Cmd.sh('cat /tmp/data.txt | grep "error" | wc -l')
const errorCount = parseInt(result.trim())
console.log('Error lines:', errorCount)

// Complex pipeline
const output = Cmd.sh(`
    find /tmp -name "*.log" -mtime -1 |
    xargs cat |
    grep "ERROR" |
    sort |
    uniq -c |
    sort -rn
`)
console.log(output)
```

---

## Data Processing

### JSON Processing

```typescript
import { Path } from 'ejscript'

// Load and process JSON
const data = new Path('/tmp/users.json').readJSON()

// Filter and transform
const activeUsers = data.users
    .filter(u => u.active)
    .map(u => ({ id: u.id, name: u.name }))

// Save result
new Path('/tmp/active-users.json')
    .write(JSON.stringify(activeUsers, null, 2))

// Merge JSON files
const config1 = new Path('/etc/app/config.json').readJSON()
const config2 = new Path('/etc/app/override.json').readJSON()
const merged = Object.blend(config1, config2)
new Path('/tmp/merged-config.json')
    .write(JSON.stringify(merged, null, 2))
```

### CSV Processing

```typescript
import { Path } from 'ejscript'

// Read CSV
const lines = new Path('/tmp/data.csv').readLines()
const headers = lines[0].split(',')
const rows = lines.slice(1).map(line => {
    const values = line.split(',')
    const obj = {}
    headers.forEach((h, i) => obj[h.trim()] = values[i].trim())
    return obj
})

// Process data
const filtered = rows.filter(r => parseInt(r.age) > 18)

// Write CSV
const output = [headers.join(',')]
filtered.forEach(row => {
    output.push(headers.map(h => row[h]).join(','))
})
new Path('/tmp/filtered.csv').writeLines(output)
```

### Log File Analysis

```typescript
import { Path } from 'ejscript'

// Analyze log file
const logFile = new Path('/var/log/app.log')
const lines = logFile.readLines()

const errors = lines.filter(line => line.includes('ERROR'))
const warnings = lines.filter(line => line.includes('WARN'))

console.log(`Total lines: ${lines.length}`)
console.log(`Errors: ${errors.length}`)
console.log(`Warnings: ${warnings.length}`)

// Extract timestamps
const timePattern = /(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})/
const firstError = errors[0]
const match = firstError.match(timePattern)
if (match) {
    console.log('First error at:', match[1])
}

// Write error report
new Path('/tmp/error-report.txt').writeLines([
    `Error Report - ${new Date().format()}`,
    `Total Errors: ${errors.length}`,
    '',
    ...errors
])
```

---

## Logging and Debugging

### Basic Logging

```typescript
import { Logger } from 'ejscript'

// Create logger
const log = new Logger('MyApp')
log.level = Logger.INFO

// Log messages
log.info('Application started')
log.debug('Processing data...', { count: 100 })
log.warn('Low memory:', { available: 512 })
log.error('Failed to connect:', error)

// Conditional logging
if (log.level >= Logger.DEBUG) {
    log.debug('Detailed debug info:', complexObject)
}
```

### Application Logging

```typescript
import { Logger, Path } from 'ejscript'

class Application {
    private log: Logger

    constructor() {
        this.log = new Logger('App')
        this.log.level = Logger.INFO
    }

    async start() {
        this.log.info('Starting application...')

        try {
            await this.initialize()
            this.log.info('Application started successfully')
        } catch (error) {
            this.log.error('Failed to start:', error)
            throw error
        }
    }

    private async initialize() {
        this.log.debug('Loading configuration...')
        const config = new Path('/etc/app/config.json').readJSON()
        this.log.debug('Config loaded:', config)

        this.log.info('Initializing services...')
        // ...
    }
}

const app = new Application()
await app.start()
```

### Debug Tracing

```typescript
import { Logger } from 'ejscript'

function processData(data: any[]) {
    const log = new Logger('DataProcessor')
    log.level = Logger.TRACE

    log.trace('processData called with', data.length, 'items')

    const result = []
    for (let i = 0; i < data.length; i++) {
        log.trace('Processing item', i, ':', data[i])
        const processed = transform(data[i])
        result.push(processed)
    }

    log.trace('Returning', result.length, 'results')
    return result
}
```

---

## Advanced Patterns

### Caching

```typescript
import { Cache, Http } from 'ejscript'

// Create cache with 5-minute TTL
const cache = new Cache({ lifespan: 300000 })

async function getUserData(userId: string) {
    // Check cache first
    const cached = cache.read(`user:${userId}`)
    if (cached) {
        console.log('Cache hit')
        return cached
    }

    // Fetch from API
    console.log('Cache miss - fetching...')
    const http = new Http()
    http.get(`https://api.example.com/users/${userId}`)
    await http.wait()
    const userData = JSON.parse(http.response)

    // Store in cache
    cache.write(`user:${userId}`, userData)

    return userData
}

// Use it
const user = await getUserData('123')
console.log(user)
```

### Rate Limiting

```typescript
import { Timer } from 'ejscript'

class RateLimiter {
    private queue: Function[] = []
    private timer: Timer

    constructor(requestsPerSecond: number) {
        const interval = 1000 / requestsPerSecond
        this.timer = new Timer(interval, () => this.processNext())
        this.timer.start()
    }

    async execute<T>(fn: () => Promise<T>): Promise<T> {
        return new Promise((resolve, reject) => {
            this.queue.push(async () => {
                try {
                    resolve(await fn())
                } catch (error) {
                    reject(error)
                }
            })
        })
    }

    private processNext() {
        const fn = this.queue.shift()
        if (fn) fn()
    }
}

// Use rate limiter
const limiter = new RateLimiter(10)  // 10 requests per second

for (let i = 0; i < 100; i++) {
    await limiter.execute(async () => {
        const http = new Http()
        http.get(`https://api.example.com/item/${i}`)
        await http.wait()
        return http.response
    })
}
```

### File Watcher Pattern

```typescript
import { Path, Timer } from 'ejscript'

class FileWatcher {
    private file: Path
    private lastModified: number
    private timer: Timer

    constructor(filePath: string, callback: (content: string) => void) {
        this.file = new Path(filePath)
        this.lastModified = this.file.modified?.getTime() || 0

        this.timer = new Timer(1000, () => {
            const currentMod = this.file.modified?.getTime() || 0
            if (currentMod > this.lastModified) {
                this.lastModified = currentMod
                const content = this.file.readString()
                callback(content)
            }
        })

        this.timer.start()
    }

    stop() {
        this.timer.stop()
    }
}

// Watch config file
const watcher = new FileWatcher('/etc/app/config.json', (content) => {
    console.log('Config changed!')
    const config = JSON.parse(content)
    // Reload configuration
})
```

### Batch Processing

```typescript
import { Path, Logger } from 'ejscript'

async function processBatch(files: Path[], batchSize: number) {
    const log = new Logger('BatchProcessor')
    const total = files.length

    for (let i = 0; i < total; i += batchSize) {
        const batch = files.slice(i, i + batchSize)
        log.info(`Processing batch ${i / batchSize + 1} (${batch.length} files)`)

        // Process batch
        const results = await Promise.all(
            batch.map(async file => {
                try {
                    const content = file.readString()
                    // Process content
                    return { file: file.name, success: true }
                } catch (error) {
                    log.error(`Failed to process ${file.name}:`, error)
                    return { file: file.name, success: false, error }
                }
            })
        )

        const successful = results.filter(r => r.success).length
        log.info(`Batch complete: ${successful}/${batch.length} successful`)
    }
}

// Process files in batches of 10
const dir = new Path('/tmp/data')
const files = dir.files('*.txt')
await processBatch(files, 10)
```

### Configuration Management

```typescript
import { Path, App } from 'ejscript'

class Config {
    private data: any
    private file: Path

    constructor(configFile?: string) {
        // Default config path
        this.file = new Path(configFile || App.dir.join('.config.json'))
        this.load()
    }

    private load() {
        if (this.file.exists) {
            this.data = this.file.readJSON()
        } else {
            this.data = this.getDefaults()
            this.save()
        }
    }

    private getDefaults() {
        return {
            logLevel: 'INFO',
            maxConnections: 100,
            timeout: 30000
        }
    }

    get(key: string, defaultValue?: any): any {
        return this.data[key] !== undefined ? this.data[key] : defaultValue
    }

    set(key: string, value: any) {
        this.data[key] = value
        this.save()
    }

    private save() {
        this.file.write(JSON.stringify(this.data, null, 2))
    }
}

// Use config
const config = new Config()
console.log('Log level:', config.get('logLevel'))
config.set('maxConnections', 200)
```

---

## More Examples

See the [examples/](../examples/) directory for additional code samples:

- `basic.ts` - Core features demonstration
- `http-streaming.ts` - HTTP streaming examples
- `http-digest-auth.ts` - Digest authentication
- And more...

---

*Last updated: 2025-10-26*
