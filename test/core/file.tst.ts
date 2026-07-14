/**
 * File Tests
 * Migrated from src/core/test/file/*.tst
 */

import { test, expect, describe, beforeAll, afterAll } from '@embedthis/testme'
import { File } from '../../src/core/File'
import { Path } from '../../src/core/Path'
import { ByteArray } from '../../src/core/streams/ByteArray'
import { assert, createTestFile, createTestFileSync, cleanupTestFile, randomTestPath } from '../helpers'
import { TestConfig } from '../config'

await describe('File', async () => {
  let testFile: Path
  const TestLength = 620 // Actual size of file.dat fixture

  beforeAll(() => {
    // Create test fixture
    testFile = TestConfig.files.testDat
  })

  await describe('File Open and Properties', () => {
    test('open file for reading', async () => {
      const f = new File(testFile.name)
      await f.open('r')

      expect(f.isOpen).toBe(true)
      expect(f.canRead).toBe(true)
      expect(f.canWrite).toBe(false)
      expect(f.options.mode).toBe('r')

      await f.close()
      expect(f.isOpen).toBe(false)
    })

    test('open file for writing', async () => {
      const file = randomTestPath('write-test', '.txt')
      const f = new File(file.name)
      await f.open('w')

      expect(f.isOpen).toBe(true)
      expect(f.canWrite).toBe(true)

      await f.write('test content')
      await f.close()

      expect(file.exists).toBe(true)
      cleanupTestFile(file)
    })

    test('open file for append', async () => {
      const file = randomTestPath('append-test', '.txt')
      const f = new File(file.name)
      await f.open('w')
      await f.write('initial')
      await f.close()

      await f.open('a')
      expect(f.isOpen).toBe(true)
      await f.write(' appended')
      await f.close()

      const fs = require('fs')
      const content = fs.readFileSync(file.name, 'utf-8')
      expect(content).toBe('initial appended')

      cleanupTestFile(file)
    })

    test('open with options object', async () => {
      const file = randomTestPath('options-test', '.txt')
      const f = new File(file.name, { mode: 'w', permissions: 0o644 })
      await f.open()

      expect(f.isOpen).toBe(true)
      expect(f.options.mode).toBe('w')
      expect(f.options.permissions).toBe(0o644)

      await f.close()
      cleanupTestFile(file)
    })

    test('file path property', async () => {
      const file = randomTestPath('path-test', '.txt')
      const f = new File(file.name, { mode: 'w' })
      await f.open()
      await f.write('test')
      await f.close()

      expect(f.path.exists).toBe(true)
      expect(f.path.name).toBe(file.name)

      cleanupTestFile(file)
    })

    test('size returns file size', async () => {
      const f = new File(testFile.name)
      await f.open('r')

      expect(f.size).toBe(TestLength)

      await f.close()
    })
  })

  await describe('File Reading', () => {
    test('readBytes reads data', async () => {
      const f = new File(testFile.name)
      await f.open('r')

      const bytes = await f.readBytes(1024)
      expect(bytes).toBeInstanceOf(Uint8Array)
      // Should read TestLength bytes even though buffer is 1024
      expect(bytes!.length).toBe(TestLength)

      await f.close()
    })

    test('read with ByteArray buffer', async () => {
      const f = new File(testFile.name)
      await f.open('r')

      const buffer = new ByteArray(1024)
      const bytesRead = await f.read(buffer)

      expect(bytesRead).toBe(TestLength)

      await f.close()
    })

    test('readString reads text content', async () => {
      const file = createTestFileSync(randomTestPath('readstring-test', '.txt').name, 'Hello World')
      const f = new File(file.name)
      await f.open('r')

      const content = await f.readString()
      expect(content).toBe('Hello World')

      await f.close()
      cleanupTestFile(file)
    })

    test('readString with count limits bytes', async () => {
      const file = createTestFileSync(randomTestPath('readcount-test', '.txt').name, 'Hello World')
      const f = new File(file.name)
      await f.open('r')

      const content = await f.readString(5)
      expect(content).toBe('Hello')

      await f.close()
      cleanupTestFile(file)
    })

    test('readLines reads file as lines', async () => {
      const file = createTestFileSync(randomTestPath('readlines-test', '.txt').name, 'Line 1\nLine 2\nLine 3')
      const f = new File(file.name)
      await f.open('r')

      const lines = await f.readLines()
      expect(lines).toHaveLength(3)
      expect(lines![0]).toBe('Line 1')
      expect(lines![1]).toBe('Line 2')
      expect(lines![2]).toBe('Line 3')

      await f.close()
      cleanupTestFile(file)
    })

    test('index access reads bytes', async () => {
      const file = randomTestPath('index-test', '.dat')
      const f = new File(file.name, { mode: 'w' })
      await f.open()

      // Write 256 bytes
      for (let i = 0; i < 256; i++) {
        f[i] = i
      }
      await f.close()

      // Read back using index access
      await f.open('r')
      for (let i = 0; i < 256; i++) {
        expect(f[i]).toBe(i)
      }
      await f.close()

      cleanupTestFile(file)
    })

    test('iterator yields positions', async () => {
      const file = createTestFileSync(randomTestPath('iter-test', '.dat').name, '0123456789')
      const f = new File(file.name)
      await f.open('r')

      // Iterator yields file positions
      let count = 0
      for (const pos of f) {
        expect(typeof pos).toBe('number')
        expect(pos).toBe(count)
        count++
      }
      expect(count).toBe(file.size)
      await f.close()

      cleanupTestFile(file)
    })
  })

  await describe('File Writing', () => {
    test('write string to file', async () => {
      const file = randomTestPath('write-test', '.txt')
      const f = new File(file.name, { mode: 'w' })
      await f.open()

      const bytesWritten = await f.write('0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789\r\n')
      await f.close()

      expect(f.size).toBe(102)
      expect(file.size).toBe(102)

      cleanupTestFile(file)
    })

    test('write ByteArray to file', async () => {
      const file = randomTestPath('writebytes-test', '.dat')
      const f = new File(file.name, { mode: 'w' })
      await f.open()

      const data = ByteArray.fromData('Binary data content')
      await f.write(data)
      await f.close()

      expect(file.exists).toBe(true)
      expect(file.size).toBeGreaterThan(0)

      cleanupTestFile(file)
    })

    test('write multiple arguments', async () => {
      const file = randomTestPath('writemulti-test', '.txt')
      const f = new File(file.name, { mode: 'w' })
      await f.open()

      await f.write('Hello', ' ', 'World')
      await f.close()

      const fs = require('fs')
      const content = fs.readFileSync(file.name, 'utf-8')
      expect(content).toBe('Hello World')

      cleanupTestFile(file)
    })

    test('writeLine writes with newline', async () => {
      const file = randomTestPath('writeline-test', '.txt')
      const f = new File(file.name, { mode: 'w' })
      await f.open()

      await f.writeLine('Line 1')
      await f.writeLine('Line 2')
      await f.close()

      const fs = require('fs')
      const content = fs.readFileSync(file.name, 'utf-8')
      const lines = content.split(/\r?\n/).filter((line: string) => line.length > 0)
      expect(lines).toHaveLength(2)
      expect(lines[0]).toBe('Line 1')
      expect(lines[1]).toBe('Line 2')

      cleanupTestFile(file)
    })

    test('copy file content with buffer', async () => {
      const srcFile = randomTestPath('copy-src', '.txt')
      const destFile = randomTestPath('copy-dest', '.txt')

      // Create source file with content
      const src = new File(srcFile.name, { mode: 'w' })
      await src.open()
      for (let i = 0; i < 1024; i++) {
        await src.write(`${i} The lazy brown fox jump over the quick dog\n`)
      }
      await src.close()

      // Copy using readBytes which handles buffer sizing correctly
      await src.open('r')
      const dest = new File(destFile.name, { mode: 'w' })
      await dest.open()

      let chunk: ByteArray | null
      while ((chunk = await src.readBytes(4096)) !== null) {
        await dest.write(chunk)
      }

      await src.close()
      await dest.close()

      // Verify sizes match
      expect(srcFile.size).toBe(destFile.size)

      cleanupTestFile(srcFile)
      cleanupTestFile(destFile)
    })
  })

  await describe('File Position', () => {
    test('position gets current position', async () => {
      const f = new File(testFile.name)
      await f.open('r')

      expect(f.position).toBe(0)

      await f.readBytes(100)
      expect(f.position).toBe(100)

      await f.close()
    })

    test('position sets file position', async () => {
      const f = new File(testFile.name)
      await f.open('r')

      f.position = 0
      const first = await f.readBytes(100)

      f.position = 0
      const second = await f.readBytes(100)

      expect(first).toEqual(second)

      await f.close()
    })

    test('seek moves position', async () => {
      const file = createTestFileSync(randomTestPath('seek-test', '.txt').name, '0123456789')
      const f = new File(file.name)
      await f.open('r')

      f.seek(5)
      expect(f.position).toBe(5)

      const content = await f.readString(5)
      expect(content).toBe('56789')

      await f.close()
      cleanupTestFile(file)
    })
  })

  await describe('File Truncate', () => {
    test('truncate reduces file size', async () => {
      const file = randomTestPath('truncate-test', '.txt')
      await testFile.copy(file)

      expect(file.size).toBe(TestLength)

      const f = new File(file.name)
      f.truncate(5)

      expect(f.size).toBe(5)
      expect(file.size).toBe(5)

      cleanupTestFile(file)
    })

    test('truncate on open file', async () => {
      const file = createTestFileSync(randomTestPath('trunc-open-test', '.txt').name, 'Hello World')
      const f = new File(file.name)
      await f.open('w')

      f.truncate(5)
      await f.close()

      expect(file.size).toBe(5)

      cleanupTestFile(file)
    })
  })

  await describe('File Remove', () => {
    test('remove deletes file', async () => {
      const file = createTestFileSync(randomTestPath('remove-test', '.txt').name, 'test content')
      const f = new File(file.name)
      await f.open('r')
      await f.close()

      expect(file.exists).toBe(true)

      await f.remove()
      expect(file.exists).toBe(false)
    })

    test('remove through path property', async () => {
      const file = createTestFileSync(randomTestPath('remove-path-test', '.txt').name, 'test')
      const f = new File(file.name)

      expect(f.path.exists).toBe(true)
      await f.remove()
      expect(f.path.exists).toBe(false)
    })
  })

  await describe('File Options', () => {
    test('options.mode reflects open mode', async () => {
      const file = randomTestPath('mode-test', '.txt')

      const f1 = new File(file.name, { mode: 'w' })
      await f1.open()
      expect(f1.options.mode).toBe('w')
      await f1.close()

      const f2 = new File(file.name, { mode: 'r' })
      await f2.open()
      expect(f2.options.mode).toBe('r')
      await f2.close()

      const f3 = new File(file.name, { mode: 'a' })
      await f3.open()
      expect(f3.options.mode).toBe('a')
      await f3.close()

      cleanupTestFile(file)
    })

    test('options.permissions sets file permissions', async () => {
      const file = randomTestPath('perms-test', '.txt')
      const f = new File(file.name, { mode: 'w', permissions: 0o644 })
      await f.open()

      await f.write('test')
      await f.close()

      expect(f.options.permissions).toBe(0o644)

      cleanupTestFile(file)
    })
  })

  await describe('File Flush', () => {
    test('flush writes buffered data', async () => {
      const file = randomTestPath('flush-test', '.txt')
      const f = new File(file.name, { mode: 'w' })
      await f.open()

      await f.write('buffered content')
      f.flush()

      // File should have content even before close
      expect(file.size).toBeGreaterThan(0)

      await f.close()
      cleanupTestFile(file)
    })
  })

  await describe('File Close', () => {
    test('close closes file', async () => {
      const file = randomTestPath('close-test', '.txt')
      const f = new File(file.name, { mode: 'w' })
      await f.open()

      expect(f.isOpen).toBe(true)

      await f.close()
      expect(f.isOpen).toBe(false)

      cleanupTestFile(file)
    })

    test('close is idempotent', async () => {
      const file = randomTestPath('idempotent-test', '.txt')
      const f = new File(file.name, { mode: 'w' })
      await f.open()

      await f.close()
      expect(f.isOpen).toBe(false)

      // Should not throw
      await f.close()
      expect(f.isOpen).toBe(false)

      cleanupTestFile(file)
    })
  })
})
