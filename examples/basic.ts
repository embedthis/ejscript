/**
 * Basic Ejscript example
 *
 * Demonstrates core functionality
 */

import { Path, File, App, Http, System } from '../src/index'

console.log('=== Ejscript for Bun - Basic Example ===\n')

// Application info
console.log('1. Application Information:')
console.log('   App name:', App.name)
console.log('   Working directory:', App.dir.name)
console.log('   Home directory:', App.home.name)
console.log('   System hostname:', System.hostname)
console.log()

// Path operations
console.log('2. Path Operations:')
const testPath = new Path('/tmp/ejscript-test')
console.log('   Created path:', testPath.name)
console.log('   Absolute:', testPath.absolute.name)
console.log('   Parent:', testPath.parent.name)
console.log('   Basename:', testPath.basename.name)
console.log('   Is absolute:', testPath.isAbsolute)
console.log()

// File operations
console.log('3. File Operations:')
const testFile = new Path('/tmp/ejscript-test.txt')
testFile.write('Hello from Ejscript on Bun!\n')
testFile.append('This is appended text.\n')
const content = testFile.readString()
console.log('   File content:')
console.log('  ', content?.split('\n').join('\n   '))
console.log('   File size:', testFile.size, 'bytes')
console.log('   File exists:', testFile.exists)
testFile.remove()
console.log('   File removed:', !testFile.exists)
console.log()

// String extensions
console.log('4. String Extensions:')
const str = 'hello_world'
console.log('   Original:', str)
console.log('   toPascal():', str.toPascal())
console.log('   toCamel():', str.toCamel())
console.log('   contains("world"):', str.contains('world'))
console.log()

// Array extensions
console.log('5. Array Extensions:')
const arr = [1, 2, 3, 2, 4, 3, 5]
console.log('   Original:', arr)
console.log('   unique():', arr.unique())
console.log('   contains(3):', arr.contains(3))
console.log()

// HTTP client (example - commented out to avoid network dependency)
console.log('6. HTTP Client (example):')
console.log('   // const http = new Http()')
console.log('   // const response = http.get("https://api.example.com/data")')
console.log('   // console.log(response.status)')
console.log()

console.log('=== Example Complete ===')
