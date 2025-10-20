/**
 * File Tests
 * Migrated from src/core/test/file/*.tst
 */

import { test, expect, describe, beforeAll, afterAll } from 'testme'
import { File } from '../../src/core/File'
import { Path } from '../../src/core/Path'
import { ByteArray } from '../../src/core/streams/ByteArray'
import { assert, createTestFile, cleanupTestFile, randomTestPath } from '../helpers'
import { TestConfig } from '../config'

await describe('File', async () => {
  let testFile: Path
  const TestLength = 620 // Actual size of file.dat fixture

  beforeAll(() => {
    // Create test fixture
    testFile = TestConfig.files.testDat
  })

  describe('File Open and Properties', () => {
    test('open file for reading', () => {
      const f = new File(testFile.name)
      f.open('r')

      expect(f.isOpen).toBe(true)
      expect(f.canRead).toBe(true)
      expect(f.canWrite).toBe(false)
      expect(f.options.mode).toBe('r')

      f.close()
      expect(f.isOpen).toBe(false)
    })

    test('open file for writing', () => {
      const file = randomTestPath('write-test', '.txt')
      const f = new File(file.name)
      f.open('w')

      expect(f.isOpen).toBe(true)
      expect(f.canWrite).toBe(true)

      f.write('test content')
      f.close()

      expect(file.exists).toBe(true)
      cleanupTestFile(file)
    })

    test('open file for append', () => {
      const file = randomTestPath('append-test', '.txt')
      const f = new File(file.name)
      f.open('w')
      f.write('initial')
      f.close()

      f.open('a')
      expect(f.isOpen).toBe(true)
      f.write(' appended')
      f.close()

      const content = file.readString()
      expect(content).toBe('initial appended')

      cleanupTestFile(file)
    })

    test('open with options object', () => {
      const file = randomTestPath('options-test', '.txt')
      const f = new File(file.name, { mode: 'w', permissions: 0o644 })

      expect(f.isOpen).toBe(true)
      expect(f.options.mode).toBe('w')
      expect(f.options.permissions).toBe(0o644)

      f.close()
      cleanupTestFile(file)
    })

    test('file path property', () => {
      const file = randomTestPath('path-test', '.txt')
      const f = new File(file.name, { mode: 'w' })
      f.write('test')
      f.close()

      expect(f.path.exists).toBe(true)
      expect(f.path.name).toBe(file.name)

      cleanupTestFile(file)
    })

    test('size returns file size', () => {
      const f = new File(testFile.name)
      f.open('r')

      expect(f.size).toBe(TestLength)

      f.close()
    })
  })

  describe('File Reading', () => {
    test('readBytes reads data', () => {
      const f = new File(testFile.name)
      f.open('r')

      const bytes = f.readBytes(1024)
      expect(bytes).toBeInstanceOf(Uint8Array)
      // Should read TestLength bytes even though buffer is 1024
      expect(bytes!.length).toBe(TestLength)

      f.close()
    })

    test('read with ByteArray buffer', () => {
      const f = new File(testFile.name)
      f.open('r')

      const buffer = new ByteArray(1024)
      const bytesRead = f.read(buffer)

      expect(bytesRead).toBe(TestLength)

      f.close()
    })

    test('readString reads text content', () => {
      const file = createTestFile('/tmp/readstring-test.txt', 'Hello World')
      const f = new File(file.name)
      f.open('r')

      const content = f.readString()
      expect(content).toBe('Hello World')

      f.close()
      cleanupTestFile(file)
    })

    test('readString with count limits bytes', () => {
      const file = createTestFile('/tmp/readcount-test.txt', 'Hello World')
      const f = new File(file.name)
      f.open('r')

      const content = f.readString(5)
      expect(content).toBe('Hello')

      f.close()
      cleanupTestFile(file)
    })

    test('readLines reads file as lines', () => {
      const file = createTestFile('/tmp/readlines-test.txt', 'Line 1\nLine 2\nLine 3')
      const f = new File(file.name)
      f.open('r')

      const lines = f.readLines()
      expect(lines).toHaveLength(3)
      expect(lines![0]).toBe('Line 1')
      expect(lines![1]).toBe('Line 2')
      expect(lines![2]).toBe('Line 3')

      f.close()
      cleanupTestFile(file)
    })

    test('index access reads bytes', () => {
      const file = randomTestPath('index-test', '.dat')
      const f = new File(file.name, { mode: 'w' })

      // Write 256 bytes
      for (let i = 0; i < 256; i++) {
        f[i] = i
      }
      f.close()

      // Read back using index access
      f.open('r')
      for (let i = 0; i < 256; i++) {
        expect(f[i]).toBe(i)
      }
      f.close()

      cleanupTestFile(file)
    })

    test('iterator yields positions', () => {
      const file = createTestFile('/tmp/iter-test.dat', '0123456789')
      const f = new File(file.name)
      f.open('r')

      // Iterator yields file positions
      let count = 0
      for (const pos of f) {
        expect(typeof pos).toBe('number')
        expect(pos).toBe(count)
        count++
      }
      expect(count).toBe(file.size)
      f.close()

      cleanupTestFile(file)
    })
  })

  describe('File Writing', () => {
    test('write string to file', () => {
      const file = randomTestPath('write-test', '.txt')
      const f = new File(file.name, { mode: 'w' })

      const bytesWritten = f.write('0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789\r\n')
      f.close()

      expect(f.size).toBe(102)
      expect(file.size).toBe(102)

      cleanupTestFile(file)
    })

    test('write ByteArray to file', () => {
      const file = randomTestPath('writebytes-test', '.dat')
      const f = new File(file.name, { mode: 'w' })

      const data = ByteArray.fromData('Binary data content')
      f.write(data)
      f.close()

      expect(file.exists).toBe(true)
      expect(file.size).toBeGreaterThan(0)

      cleanupTestFile(file)
    })

    test('write multiple arguments', () => {
      const file = randomTestPath('writemulti-test', '.txt')
      const f = new File(file.name, { mode: 'w' })

      f.write('Hello', ' ', 'World')
      f.close()

      const content = file.readString()
      expect(content).toBe('Hello World')

      cleanupTestFile(file)
    })

    test('writeLine writes with newline', () => {
      const file = randomTestPath('writeline-test', '.txt')
      const f = new File(file.name, { mode: 'w' })

      f.writeLine('Line 1')
      f.writeLine('Line 2')
      f.close()

      const lines = file.readLines()
      expect(lines).toHaveLength(2)
      expect(lines![0]).toBe('Line 1')
      expect(lines![1]).toBe('Line 2')

      cleanupTestFile(file)
    })

    test('copy file content with buffer', () => {
      const srcFile = randomTestPath('copy-src', '.txt')
      const destFile = randomTestPath('copy-dest', '.txt')

      // Create source file with content
      const src = new File(srcFile.name, { mode: 'w' })
      for (let i = 0; i < 1024; i++) {
        src.write(`${i} The lazy brown fox jump over the quick dog\n`)
      }
      src.close()

      // Copy using readBytes which handles buffer sizing correctly
      src.open('r')
      const dest = new File(destFile.name, { mode: 'w' })

      let chunk: ByteArray | null
      while ((chunk = src.readBytes(4096)) !== null) {
        dest.write(chunk)
      }

      src.close()
      dest.close()

      // Verify sizes match
      expect(srcFile.size).toBe(destFile.size)

      cleanupTestFile(srcFile)
      cleanupTestFile(destFile)
    })
  })

  describe('File Position', () => {
    test('position gets current position', () => {
      const f = new File(testFile.name)
      f.open('r')

      expect(f.position).toBe(0)

      f.readBytes(100)
      expect(f.position).toBe(100)

      f.close()
    })

    test('position sets file position', () => {
      const f = new File(testFile.name)
      f.open('r')

      f.position = 0
      const first = f.readBytes(100)

      f.position = 0
      const second = f.readBytes(100)

      expect(first).toEqual(second)

      f.close()
    })

    test('seek moves position', () => {
      const file = createTestFile('/tmp/seek-test.txt', '0123456789')
      const f = new File(file.name)
      f.open('r')

      f.seek(5)
      expect(f.position).toBe(5)

      const content = f.readString(5)
      expect(content).toBe('56789')

      f.close()
      cleanupTestFile(file)
    })
  })

  describe('File Truncate', () => {
    test('truncate reduces file size', () => {
      const file = randomTestPath('truncate-test', '.txt')
      testFile.copy(file)

      expect(file.size).toBe(TestLength)

      const f = new File(file.name)
      f.truncate(5)

      expect(f.size).toBe(5)
      expect(file.size).toBe(5)

      cleanupTestFile(file)
    })

    test('truncate on open file', () => {
      const file = createTestFile('/tmp/trunc-open-test.txt', 'Hello World')
      const f = new File(file.name)
      f.open('w')

      f.truncate(5)
      f.close()

      expect(file.size).toBe(5)

      cleanupTestFile(file)
    })
  })

  describe('File Remove', () => {
    test('remove deletes file', () => {
      const file = createTestFile('/tmp/remove-test.txt', 'test content')
      const f = new File(file.name)
      f.open('r')
      f.close()

      expect(file.exists).toBe(true)

      f.remove()
      expect(file.exists).toBe(false)
    })

    test('remove through path property', () => {
      const file = createTestFile('/tmp/remove-path-test.txt', 'test')
      const f = new File(file.name)

      expect(f.path.exists).toBe(true)
      f.remove()
      expect(f.path.exists).toBe(false)
    })
  })

  describe('File Options', () => {
    test('options.mode reflects open mode', () => {
      const file = randomTestPath('mode-test', '.txt')

      const f1 = new File(file.name, { mode: 'w' })
      expect(f1.options.mode).toBe('w')
      f1.close()

      const f2 = new File(file.name, { mode: 'r' })
      expect(f2.options.mode).toBe('r')
      f2.close()

      const f3 = new File(file.name, { mode: 'a' })
      expect(f3.options.mode).toBe('a')
      f3.close()

      cleanupTestFile(file)
    })

    test('options.permissions sets file permissions', () => {
      const file = randomTestPath('perms-test', '.txt')
      const f = new File(file.name, { mode: 'w', permissions: 0o644 })

      f.write('test')
      f.close()

      expect(f.options.permissions).toBe(0o644)

      cleanupTestFile(file)
    })
  })

  describe('File Flush', () => {
    test('flush writes buffered data', () => {
      const file = randomTestPath('flush-test', '.txt')
      const f = new File(file.name, { mode: 'w' })

      f.write('buffered content')
      f.flush()

      // File should have content even before close
      expect(file.size).toBeGreaterThan(0)

      f.close()
      cleanupTestFile(file)
    })
  })

  describe('File Close', () => {
    test('close closes file', () => {
      const file = randomTestPath('close-test', '.txt')
      const f = new File(file.name, { mode: 'w' })

      expect(f.isOpen).toBe(true)

      f.close()
      expect(f.isOpen).toBe(false)

      cleanupTestFile(file)
    })

    test('close is idempotent', () => {
      const file = randomTestPath('idempotent-test', '.txt')
      const f = new File(file.name, { mode: 'w' })

      f.close()
      expect(f.isOpen).toBe(false)

      // Should not throw
      f.close()
      expect(f.isOpen).toBe(false)

      cleanupTestFile(file)
    })
  })
})
