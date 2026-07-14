/**
 * HTTP Streaming Example - Connect-Write-Finalize Pattern
 *
 * Demonstrates the new streaming API for HTTP requests.
 * Pattern: connect() -> write() -> finalize() -> access response
 */

import { Http } from '../../src/core/Http'

console.log('=== HTTP Streaming Examples (New Pattern) ===\n')

// Example 1: Basic connect-write-finalize pattern
async function streamWithReadableStream() {
    console.log('1. Basic Connect-Write-Finalize Pattern:')

    const http = new Http()

    // Connect sets up the request
    http.connect('POST', 'https://httpbin.org/post')

    // Write chunks
    http.write('Hello ')
    http.write('from ')
    http.write('Ejscript!')

    // Finalize sends the request and waits for response
    await http.finalize()

    console.log(`Status: ${http.status}`)
    console.log(`Response length: ${http.response.length} bytes\n`)
}

// Example 2: NEW PATTERN - Connect, Write, Finalize
async function streamWithWrite() {
    console.log('2. NEW PATTERN - Connect, Write, Finalize:')

    const http = new Http()

    // Step 1: Connect (non-blocking setup)
    http.connect('POST', 'https://httpbin.org/post')

    // Step 2: Write data incrementally
    const bytes1 = http.write('Part 1: ')
    const bytes2 = http.write('Part 2: ')
    const bytes3 = http.write({ message: 'JSON object' })

    console.log(`Wrote ${bytes1 + bytes2 + bytes3} bytes`)

    // Step 3: Finalize and wait for response
    await http.finalize()

    // Step 4: Access response
    console.log(`Status: ${http.status}`)
    console.log(`Response length: ${http.response.length} bytes\n`)
}

// Example 3: Stream file content
async function streamFile() {
    console.log('3. Streaming file content:')

    // Create a temporary test file
    const testFile = `.test/stream-example-${process.pid}.txt`
    await Bun.write(testFile, 'This is file content being streamed via HTTP POST!')

    const file = Bun.file(testFile)
    const stream = file.stream()

    const http = new Http()
    try {
        http.post('https://httpbin.org/post', stream)
        await http.finalize()  // Must call finalize after post with stream
        console.log(`Status: ${http.status}`)
        console.log(`File streamed successfully\n`)
    } catch (error) {
        console.log(`Error: ${error}\n`)
    } finally {
        // Cleanup
        await import('fs').then(fs => fs.promises.unlink(testFile).catch(() => {}))
    }
}

// Example 4: Using wait() instead of finalize()
async function streamAsyncData() {
    console.log('4. Using wait() instead of finalize():')

    const http = new Http()
    http.connect('POST', 'https://httpbin.org/post')

    for (let i = 0; i < 5; i++) {
        http.write(`Chunk ${i} `)
    }

    // Close stream without awaiting
    http.finalize()

    // Use wait() to wait for completion
    const completed = await http.wait()

    console.log(`Completed: ${completed}`)
    console.log(`Status: ${http.status}`)
    console.log(`Stream sent successfully\n`)
}

// Example 5: Error handling - accessing response before finalize
async function streamLargeData() {
    console.log('5. Error handling - accessing before finalize:')

    const http = new Http()
    http.connect('POST', 'https://httpbin.org/post')
    http.write('some data')

    // Try to access response before finalize - will throw
    try {
        console.log(http.response)
    } catch (error) {
        console.log(`Error (expected): ${(error as Error).message}`)
    }

    // After finalize, response is available
    await http.finalize()
    console.log(`Status: ${http.status}`)
    console.log(`Response is now accessible\n`)
}

// Example 6: Pattern comparison
function showPatternComparison() {
    console.log('6. Pattern Comparison:\n')

    console.log('OLD PATTERN (broken):')
    console.log('  await http.connect("POST", url, data)')
    console.log('  let response = http.response\n')

    console.log('NEW STREAMING PATTERN:')
    console.log('  http.connect("POST", url)   // Non-blocking setup')
    console.log('  http.write(chunk1)          // Accumulate chunks')
    console.log('  http.write(chunk2)')
    console.log('  await http.finalize()       // Send and wait')
    console.log('  let response = http.response // Access result\n')

    console.log('ALTERNATIVE (using wait):')
    console.log('  http.connect("POST", url)')
    console.log('  http.write(data)')
    console.log('  http.finalize()             // Close stream')
    console.log('  await http.wait()           // Wait for completion')
    console.log('  let response = http.response\n')

    console.log('CONVENIENCE (immediate send):')
    console.log('  http.post(url, data)        // Setup with data')
    console.log('  await http.finalize()       // Send and wait')
    console.log('  let response = http.response\n')
}

// Run all examples
async function main() {
    await streamWithReadableStream()
    await streamWithWrite()
    await streamFile()
    await streamAsyncData()
    await streamLargeData()
    showPatternComparison()

    console.log('=== All streaming examples completed ===')
}

main().catch(console.error)
