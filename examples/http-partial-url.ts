/**
 * Example: Using partial URLs with Http class
 *
 * Demonstrates how Ejscript-style partial URLs are automatically
 * completed before making HTTP requests.
 */
import { Http } from '../src/core/Http'

// Start a test server
const server = Bun.serve({
    port: 8080,
    fetch(req) {
        const url = new URL(req.url)
        return new Response(`Request received at: ${url.pathname}`, {
            headers: { 'Content-Type': 'text/plain' }
        })
    }
})

console.log('Test server started on port 8080\n')

async function demonstratePartialUrls() {
    try {
        // Example 1: Port-only URL
        console.log('1. Port-only URL: "8080/index.html"')
        let http = new Http()
        await http.get('8080/index.html')
        console.log(`   Status: ${http.status}`)
        console.log(`   Response: ${http.response}\n`)

        // Example 2: IP without scheme
        console.log('2. IP without scheme: "127.0.0.1:8080/test"')
        http = new Http()
        await http.get('127.0.0.1:8080/test')
        console.log(`   Status: ${http.status}`)
        console.log(`   Response: ${http.response}\n`)

        // Example 3: Colon-port format
        console.log('3. Colon-port format: ":8080/api/data"')
        http = new Http()
        await http.get(':8080/api/data')
        console.log(`   Status: ${http.status}`)
        console.log(`   Response: ${http.response}\n`)

        // Example 4: Localhost without scheme
        console.log('4. Localhost without scheme: "localhost:8080/page"')
        http = new Http()
        await http.get('localhost:8080/page')
        console.log(`   Status: ${http.status}`)
        console.log(`   Response: ${http.response}\n`)

        // Example 5: Complete URL (unchanged)
        console.log('5. Complete URL: "http://127.0.0.1:8080/complete"')
        http = new Http()
        await http.get('http://127.0.0.1:8080/complete')
        console.log(`   Status: ${http.status}`)
        console.log(`   Response: ${http.response}\n`)

    } catch (error) {
        console.error('Error:', error)
    } finally {
        server.stop()
        console.log('Server stopped')
    }
}

demonstratePartialUrls()
