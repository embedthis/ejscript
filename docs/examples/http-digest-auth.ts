/**
 * HTTP Digest Authentication Example
 *
 * Demonstrates transparent digest authentication with the Http client.
 * After calling setCredentials() with type 'digest', the Http client
 * automatically handles 401 challenges and digest response computation.
 */

import { Http } from '../../src/core/Http'

console.log('=== HTTP Digest Authentication Example ===\n')

// Example 1: Basic digest auth (will work with test server)
console.log('Example 1: Digest Authentication with Test Server')
console.log('--------------------------------------------------')

async function testDigestAuth() {
    // Start a simple test server (you'll need to run the test server separately)
    console.log('To run this example, start the test server first:')
    console.log('  bun test/helpers/test-server.ts\n')

    const http = new Http()

    // Set credentials with digest type
    http.setCredentials('testuser', 'testpass', 'digest')

    try {
        // Make request - digest auth happens transparently
        // 1. First request returns 401 with WWW-Authenticate challenge
        // 2. Client parses challenge, computes digest response
        // 3. Client retries with Authorization: Digest header
        // 4. Server validates and returns 200
        await http.get('http://localhost:4100/digest-auth')

        console.log(`Status: ${http.status} ${http.statusMessage}`)
        console.log(`Response: ${http.response}\n`)

        // Subsequent requests reuse the nonce (no 401)
        console.log('Making second request (nonce reuse)...')
        await http.get('http://localhost:4100/digest-auth')
        console.log(`Status: ${http.status} ${http.statusMessage}`)
        console.log(`Response: ${http.response}\n`)
    } catch (error: any) {
        console.log(`Error: ${error.message}`)
        console.log('Make sure test server is running on port 4100\n')
    }
}

// Example 2: SHA-256 digest auth
console.log('Example 2: Digest Authentication with SHA-256')
console.log('-----------------------------------------------')

async function testDigestAuthSHA256() {
    const http = new Http()
    http.setCredentials('testuser', 'testpass', 'digest')

    try {
        // This endpoint uses SHA-256 algorithm
        await http.get('http://localhost:4100/digest-auth-sha256')

        console.log(`Status: ${http.status} ${http.statusMessage}`)
        console.log(`Response: ${http.response}`)

        // Check the algorithm used
        const digestAuth = (http as any)._digestAuth
        console.log(`Algorithm: ${digestAuth?.algorithm}\n`)
    } catch (error: any) {
        console.log(`Error: ${error.message}\n`)
    }
}

// Example 3: Switching between Basic and Digest auth
console.log('Example 3: Switching Between Auth Types')
console.log('---------------------------------------')

async function testAuthSwitching() {
    const http = new Http()

    // Use basic auth first
    console.log('Using Basic authentication...')
    http.setCredentials('user', 'pass', 'basic')

    try {
        await http.get('http://localhost:4100/auth')
        console.log(`Basic auth - Status: ${http.status}`)
    } catch (error: any) {
        console.log(`Basic auth error: ${error.message}`)
    }

    // Switch to digest auth
    console.log('\nSwitching to Digest authentication...')
    http.setCredentials('testuser', 'testpass', 'digest')

    try {
        await http.get('http://localhost:4100/digest-auth')
        console.log(`Digest auth - Status: ${http.status}`)
        console.log(`Response: ${http.response}\n`)
    } catch (error: any) {
        console.log(`Digest auth error: ${error.message}\n`)
    }
}

// Example 4: POST with digest auth
console.log('Example 4: POST Request with Digest Authentication')
console.log('--------------------------------------------------')

async function testDigestPost() {
    const http = new Http()
    http.setCredentials('testuser', 'testpass', 'digest')

    try {
        await http.post('http://localhost:4100/digest-auth-post', 'Hello from digest auth!')

        console.log(`Status: ${http.status} ${http.statusMessage}`)
        console.log(`Response: ${http.response}\n`)
    } catch (error: any) {
        console.log(`Error: ${error.message}\n`)
    }
}

// Run all examples
async function runExamples() {
    await testDigestAuth()
    await testDigestAuthSHA256()
    await testAuthSwitching()
    await testDigestPost()

    console.log('=== Examples Complete ===')
    console.log('\nKey Features:')
    console.log('- Transparent digest authentication (no manual challenge handling)')
    console.log('- Automatic algorithm detection (MD5, SHA-256, SHA-512-256)')
    console.log('- Nonce reuse for better performance')
    console.log('- Support for qop=auth and qop=auth-int')
    console.log('- Works with GET, POST, PUT, DELETE, etc.')
}

// Run examples
runExamples().catch(console.error)
