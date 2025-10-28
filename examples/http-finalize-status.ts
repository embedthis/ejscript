#!/usr/bin/env bun

/**
 * Example: Http.finalize() returns status code
 *
 * Demonstrates the convenient new feature where finalize() returns
 * the HTTP status code directly.
 */

import { Http } from '../src/core/Http'

async function main() {
    console.log('Http.finalize() Status Return Example\n')

    // Example 1: Old way (still works)
    console.log('1. Traditional approach (two steps):')
    const http1 = new Http()
    http1.get('https://httpbin.org/status/200')
    await http1.finalize()
    const status1 = http1.status
    console.log(`   Status: ${status1}\n`)

    // Example 2: New way (convenient single-line)
    console.log('2. New convenient approach (one step):')
    const http2 = new Http()
    http2.get('https://httpbin.org/status/201')
    const status2 = await http2.finalize()
    console.log(`   Status: ${status2}\n`)

    // Example 3: Error status codes
    console.log('3. Error status codes:')
    const http3 = new Http()
    http3.get('https://httpbin.org/status/404')
    const status3 = await http3.finalize()
    console.log(`   Status: ${status3}`)
    console.log(`   Success: ${http3.success}`)
    console.log(`   Status Message: ${http3.statusMessage}\n`)

    // Example 4: Use in conditionals
    console.log('4. Direct use in conditionals:')
    const http4 = new Http()
    http4.get('https://httpbin.org/status/500')
    if ((await http4.finalize()) >= 500) {
        console.log('   Server error detected!')
    }

    console.log('\nAll examples completed successfully!')
}

main()
