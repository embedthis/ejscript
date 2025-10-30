/**
 * Build script for Ejscript
 *
 * Compiles TypeScript to JavaScript and generates type definitions
 */

import { $ } from 'bun'
import { existsSync, rmSync } from 'fs'

console.log('Building Ejscript...\n')

// Clean dist directory
if (existsSync('dist')) {
    console.log('Cleaning dist directory...')
    rmSync('dist', { recursive: true, force: true })
}

// Run TypeScript compiler
console.log('Compiling TypeScript...')
try {
    await $`bun tsc`
    console.log('✓ TypeScript compilation complete\n')
} catch (error) {
    console.error('✗ TypeScript compilation failed')
    process.exit(1)
}

/*
console.log('Build complete!')
console.log('\nYou can now:')
console.log('  - Run tests: bun test')
console.log('  - Try examples: bun examples/basic.ts')
*/
