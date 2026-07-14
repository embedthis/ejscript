#!/usr/bin/env bun
/**
 * Args Class Demo
 *
 * Demonstrates command-line argument parsing with the Args class
 *
 * Usage:
 *   bun docs/examples/args-demo.ts --verbose --port 8080 --mode production file1.txt file2.txt
 *   bun docs/examples/args-demo.ts -v --log stdout:3 --include lib --include src
 *   bun docs/examples/args-demo.ts -?
 */

import { Args } from '../../src/core/Args'

function usage() {
    console.log(`
Usage: args-demo [OPTIONS] [FILES...]

Options:
  -v, --verbose          Enable verbose output
  -q, --quiet            Suppress all output
  --port PORT            Server port number (default: 8080)
  --log PATTERN          Log pattern (e.g., stderr:4)
  --mode MODE            Mode: development, staging, production (default: development)
  --include DIR          Include directory (can be specified multiple times)
  -?, --help             Show this help message

Examples:
  args-demo --verbose --port 3000 file.txt
  args-demo -v --mode production --include lib --include src
  args-demo --log stdout:3 data/*.json
`)
}

// Parse command-line arguments
const args = new Args({
    options: {
        verbose: {
            alias: 'v',
            value: false
        },
        quiet: {
            alias: 'q',
            value: false
        },
        port: {
            range: Number,
            value: 8080
        },
        log: {
            range: /\w+(:\d+)/,
            value: 'stderr:4'
        },
        mode: {
            range: ['development', 'staging', 'production'],
            value: 'development'
        },
        include: {
            range: String,
            separator: Array  // Collect multiple values in array
        }
    },
    usage: usage,
    onerror: 'exit'
})

// Display parsed arguments
console.log('=== Parsed Command-Line Arguments ===\n')
console.log('Program:', args.program.toString())
console.log('\nOptions:')
console.log('  Verbose:', args.options.verbose)
console.log('  Quiet:', args.options.quiet)
console.log('  Port:', args.options.port, `(${typeof args.options.port})`)
console.log('  Log:', args.options.log)
console.log('  Mode:', args.options.mode)
if (args.options.include) {
    console.log('  Include directories:', args.options.include)
}

if (args.rest.length > 0) {
    console.log('\nRemaining arguments (files):')
    for (const file of args.rest) {
        console.log('  -', file)
    }
} else {
    console.log('\nNo files specified.')
}

console.log('\n=== Simulated Application Behavior ===\n')

if (args.options.verbose && !args.options.quiet) {
    console.log('[VERBOSE] Application starting...')
    console.log('[VERBOSE] Port:', args.options.port)
    console.log('[VERBOSE] Mode:', args.options.mode)
    console.log('[VERBOSE] Log level:', args.options.log)
}

if (args.rest.length > 0) {
    console.log('Processing files:')
    for (const file of args.rest) {
        console.log(`  Processing: ${file}`)
    }
} else if (!args.options.quiet) {
    console.log('No files to process.')
}

if (args.options.verbose && !args.options.quiet) {
    console.log('[VERBOSE] Application finished.')
}
