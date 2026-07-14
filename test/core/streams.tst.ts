/**
 * Stream Tests (ByteArray, TextStream, BinaryStream)
 * Migrated from src/core/test/bytearray/*.tst, textstream/*.tst, binarystream/*.tst
 */

import { test, expect, describe, beforeAll, afterAll } from 'testme'
import { ByteArray } from '../../src/core/streams/ByteArray'
import { TextStream } from '../../src/core/streams/TextStream'
import { BinaryStream, Endian } from '../../src/core/streams/BinaryStream'
import { Path } from '../../src/core/Path'
import { assert, createTestFile, createTestFileSync, cleanupTestFile, randomTestPath } from '../helpers'

await describe('ByteArray', async () => {
  await describe('Construction', () => {
    test('creates ByteArray with size', () => {
      const b = new ByteArray(200)
      expect(b).not.toBeNull()
      expect(b.size).toBe(200)
      expect(b.readPosition).toBe(0)
      expect(b.writePosition).toBe(0)
      expect(b.length).toBe(0)
      expect(b.room).toBe(200)
    })

    test('creates empty ByteArray', () => {
      const b = new ByteArray()
      expect(b).not.toBeNull()
      expect(b.length).toBe(0)
    })
  })

  await describe('Basic Read/Write', () => {
    test('writeByte and readByte', () => {
      const b = new ByteArray(200)
      b.writeByte(110)
      b.writeByte(111)
      b.writeByte(112)

      expect(b.toString()).toBe('nop')
      expect(b.readByte()).toBe(110)
      expect(b.readPosition).toBe(1)
      expect(b.toString()).toBe('op')
    })

    test('write string', () => {
      const b = new ByteArray(200)
      b.writeData('Sunny')
      expect(b.readPosition).toBe(0)
      expect(b.writePosition).toBe(5)

      b.writeData(' Day')
      expect(b.writePosition).toBe(9)

      const s = b.readString()
      expect(s).toBe('Sunny Day')
      expect(b.readPosition).toBe(b.writePosition)
    })

    test('write and read with toString', () => {
      const b = new ByteArray(200)
      b.writeData('Hello World')
      expect(b.toString()).toBe('Hello World')
    })
  })

  await describe('Indexing', () => {
    test('index access reads bytes', () => {
      const b = new ByteArray(200)
      b.writeData('Sunny')
      expect(b[0]).toBe(83) // 'S'
      expect(b[1]).toBe(117) // 'u'
      expect(b[2]).toBe(110) // 'n'
    })

    test('index access after read', () => {
      const b = new ByteArray(200)
      b.writeData('Sunny Day')
      expect(b[2]).toBe(110) // 'n'
    })

    test('low level access after consuming', () => {
      const b = new ByteArray(200)
      b.writeByte(110)
      b.writeByte(111)
      b.writeByte(112)
      b.readByte()
      expect(b.readPosition).toBe(1)
      expect(b[0]).toBe(110) // Can still access via index
      expect(String.fromCharCode(b[0])).toBe('n')
    })
  })

  await describe('Iteration', () => {
    test('iterates over available data', () => {
      const b = new ByteArray(200)
      b.writeData('Sunny')
      let count = 0
      for (const byte of b) {
        count++
        expect(typeof byte).toBe('number')
      }
      expect(count).toBe(5)
    })

    test('iterates after partial read', () => {
      const b = new ByteArray(200)
      b.writeData('Hello World')
      b.readByte() // Consume 'H'

      let s = ''
      for (const byte of b) {
        s += String.fromCharCode(byte)
      }
      expect(s).toBe('ello World')
    })
  })

  await describe('Flush', () => {
    test('flush resets positions', () => {
      const b = new ByteArray(200)
      expect(b.size).toBe(200)
      expect(b.length).toBe(0)

      b.flush()
      expect(b.length).toBe(0)

      b.writeData('Hello')
      expect(b.length).toBe(5)

      b.flush()
      expect(b.length).toBe(0)
      expect(b.size).toBe(200)
    })
  })

  await describe('Read Position', () => {
    test('readPosition can be moved', () => {
      const b = new ByteArray(200)
      b.writeData('Sunny Day')
      b.readPosition += 6
      expect(b.readString()).toBe('Day')
    })
  })

  await describe('Read Method', () => {
    test('read into another ByteArray', () => {
      const b = new ByteArray(200)
      b.writeByte(1)
      b.writeByte(2)
      b.writeByte(3)

      const check = new ByteArray(10)
      b.read(check, 0, 3)
      expect(check[0]).toBe(1)
      expect(check[1]).toBe(2)
      expect(check[2]).toBe(3)
    })

    test('read large amount of data', () => {
      const size = 4096
      const b = new ByteArray(size) // Allocate sufficient initial size

      for (let i = 0; i < size; i++) {
        b.writeByte(i % 256)
      }

      const check = new ByteArray(size, false)
      b.read(check)
      expect(check.length).toBe(size)

      for (let i = 0; i < 200; i++) {
        expect(check[i]).toBe(i % 256)
        expect(check.readByte()).toBe(i % 256)
      }
    })
  })

  await describe('Primitive Types', () => {
    test('read and write primitives', () => {
      const b = new ByteArray()
      b.writeByte(1)
      b.writeShort(2)
      b.writeInteger32(3)
      b.writeLong(4n)
      b.writeDouble(5.1234)

      expect(b.readByte()).toBe(1)
      expect(b.readShort()).toBe(2)
      expect(b.readInteger32()).toBe(3)
      expect(b.readLong()).toBe(4n)
      expect(b.readDouble()).toBeCloseTo(5.1234, 4)
    })
  })

  await describe('Events', () => {
    test('readable event fires on flush', () => {
      const b = new ByteArray(200)
      let saveData: string | null = null

      b.on('readable', (event: string, ba: ByteArray) => {
        saveData = ba.readString()
      })

      b.writeData('Hello World')
      b.flush()
      expect(saveData).toBe('Hello World')
    })

    test('writable event fires immediately', () => {
      const b = new ByteArray(200)
      b.on('writable', (event: string, ba: ByteArray) => {
        ba.writeData('Some Data')
      })

      expect(b.readString()).toBe('Some Data')
    })
  })

  await describe('Copy Operations', () => {
    test('copyIn copies from another ByteArray', () => {
      const source = new ByteArray(200)
      source.writeData('Hello')

      const dest = new ByteArray(200)
      const count = dest.copyIn(0, source)

      expect(count).toBeGreaterThan(0)
      expect(String.fromCharCode(dest[0], dest[1], dest[2])).toBe('Hel')

      dest.writePosition = source.length
      expect(dest.toString()).toBe('Hello')
    })

    test('copyOut copies to another ByteArray', () => {
      const source = new ByteArray(200)
      source.writeData('Hello')

      const dest = new ByteArray(1024)
      source.copyOut(0, dest, 0, 4)

      expect(String.fromCharCode(dest[0], dest[1], dest[2])).toBe('Hel')
      dest.writePosition = 4
      expect(dest.toString()).toBe('Hell')
    })
  })

  await describe('New API Methods (Phase 5)', () => {
    test('input/output aliases for read/write positions', () => {
      const b = new ByteArray(200)
      b.writeData('Hello World')

      expect(b.output).toBe(11)
      expect(b.input).toBe(0)

      b.input = 6
      expect(b.readPosition).toBe(6)
      expect(b.readString()).toBe('World')

      b.output = 5
      expect(b.writePosition).toBe(5)
    })

    test('roomLeft getter', () => {
      const b = new ByteArray(100)
      expect(b.roomLeft).toBe(100)

      b.writeData('Hello')
      expect(b.roomLeft).toBe(95)
      expect(b.roomLeft).toBe(b.room)
    })

    test('resizable getter/setter', () => {
      const b = new ByteArray(50, true)
      expect(b.resizable).toBe(true)

      b.resizable = false
      expect(b.resizable).toBe(false)
    })

    test('encoding getter/setter', () => {
      const b = new ByteArray(100)
      expect(b.encoding).toBe('utf-8')

      b.encoding = 'utf-16'
      expect(b.encoding).toBe('utf-16')
    })

    test('writeString writes string data', () => {
      const b = new ByteArray(200)
      const written = b.writeString('Hello World')

      expect(written).toBe(11)
      expect(b.writePosition).toBe(11)
      expect(b.toString()).toBe('Hello World')
    })

    test('readString with encoding parameter', () => {
      const b = new ByteArray(200)
      b.writeData('Hello')

      const str = b.readString(-1, 'utf-8')
      expect(str).toBe('Hello')
    })

    test('compress creates compressed ByteArray', () => {
      const b = new ByteArray(1000)
      b.writeData('Hello World! '.repeat(10))

      const compressed = b.compress()
      expect(compressed).not.toBeNull()
      expect(compressed.length).toBeGreaterThan(0)
      expect(compressed.length).toBeLessThan(b.length)
    })

    test('uncompress decompresses data', () => {
      const original = new ByteArray(1000)
      const testData = 'The quick brown fox jumps over the lazy dog. '.repeat(5)
      original.writeData(testData)

      const compressed = original.compress()
      const uncompressed = compressed.uncompress()

      expect(uncompressed.toString()).toBe(testData)
      expect(uncompressed.length).toBe(original.length)
    })

    test('compress/uncompress round-trip preserves data', () => {
      const b = new ByteArray(2000) // Increased size to accommodate test data
      const testData = 'Lorem ipsum dolor sit amet, consectetur adipiscing elit. '.repeat(20)
      b.writeData(testData)

      const originalLength = b.length
      const compressed = b.compress()
      const uncompressed = compressed.uncompress()

      expect(uncompressed.toString()).toBe(testData)
      expect(uncompressed.length).toBe(originalLength)

      // Compression should reduce size significantly for repetitive data
      expect(compressed.length).toBeLessThan(originalLength * 0.5)
    })
  })
})

await describe('TextStream', async () => {
  await describe('Construction', () => {
    test('creates TextStream from ByteArray', () => {
      const b = new ByteArray(1000)
      const t = new TextStream(b)
      expect(t).not.toBeNull()
    })
  })

  await describe('Write Operations', () => {
    test('writeLine writes with newlines', async () => {
      const b = new ByteArray(1000)
      const t = new TextStream(b)

      await t.writeLine('line 1', 'line 2', 'line 3')

      const result = b.toString()
      // Accept either \r\n or \n
      expect(
        result === 'line 1\r\nline 2\r\nline 3\r\n' || result === 'line 1\nline 2\nline 3\n'
      ).toBe(true)
    })

    test('write string without newline', async () => {
      const b = new ByteArray(1000)
      const t = new TextStream(b)

      await t.write('Hello', ' ', 'World')
      expect(b.toString()).toBe('Hello World')
    })
  })

  await describe('Read Operations', () => {
    test('read from TextStream', async () => {
      const b = new ByteArray(1000)
      const t = new TextStream(b)

      b.writeData('Line one\nLine 2\nLine 3')
      const dest = new ByteArray(1000)
      const count = await t.read(dest)

      expect(count).toBe(22)
      expect(dest.readString()).toBe('Line one\nLine 2\nLine 3')
    })

    test('readLine reads single line', async () => {
      const b = new ByteArray(1000)
      const t = new TextStream(b)

      b.writeData('Line 1\nLine 2\nLine 3')
      expect(await t.readLine()).toBe('Line 1')
      expect(await t.readLine()).toBe('Line 2')
      expect(await t.readLine()).toBe('Line 3')
    })

    test('readLines reads all lines', async () => {
      const b = new ByteArray(1000)
      const t = new TextStream(b)

      b.writeData('Line one\nLine two\nLine three')
      const lines = await t.readLines()

      expect(lines).toHaveLength(3)
      expect(lines[0]).toBe('Line one')
      expect(lines[1]).toBe('Line two')
      expect(lines[2]).toBe('Line three')
    })
  })

  await describe('File Integration', () => {
    test('openTextStream for writing', async () => {
      const file = randomTestPath('textstream', '.tmp')
      const s = await file.openTextStream('wt')

      await s.writeLine('Hello')
      await s.writeLine('World')
      await s.close()

      expect(file.exists).toBe(true)

      cleanupTestFile(file)
    })

    test('openTextStream for reading', async () => {
      const file = await createTestFile(randomTestPath('textstream-read', '.tmp').name, 'Hello\nWorld')
      const s = await file.openTextStream('rt')

      expect(await s.readLine()).toBe('Hello')
      expect(await s.readLine()).toBe('World')

      await s.close()
      cleanupTestFile(file)
    })

    test('round-trip write and read', async () => {
      const file = randomTestPath('textstream-roundtrip', '.tmp')

      const ws = await file.openTextStream('wt')
      await ws.writeLine('First line')
      await ws.writeLine('Second line')
      await ws.close()

      const rs = await file.openTextStream('rt')
      expect(await rs.readLine()).toBe('First line')
      expect(await rs.readLine()).toBe('Second line')
      await rs.close()

      cleanupTestFile(file)
    })
  })
})

await describe('BinaryStream', async () => {
  await describe('Construction', () => {
    test('creates BinaryStream from ByteArray', () => {
      const ba = new ByteArray()
      const s = new BinaryStream(ba)
      expect(s).not.toBeNull()
    })
  })

  await describe('Integer Operations', () => {
    test('writeInteger32 and readInteger32', async () => {
      const ba = new ByteArray()
      const s = new BinaryStream(ba)

      await s.writeInteger32(4000)
      expect(ba.length).toBe(4)

      const v = await s.readInteger32()
      expect(v).toBe(4000)
    })

    test('write and read multiple integers', async () => {
      const ba = new ByteArray()
      const s = new BinaryStream(ba)

      for (let i = 0; i < 100; i++) {
        await s.writeInteger32(i)
      }

      for (let i = 0; i < 100; i++) {
        expect(await s.readInteger32()).toBe(i)
      }
    })
  })

  await describe('Endianness', () => {
    test('BigEndian encoding', async () => {
      const ba = new ByteArray()
      const s = new BinaryStream(ba)

      s.endian = BinaryStream.BigEndian
      expect(s.endian).toBe(Endian.BigEndian)

      await s.writeInteger32(0x12345678)

      // Check byte order for big endian
      expect(ba[0]).toBe(0x12)
      expect(ba[1]).toBe(0x34)
      expect(ba[2]).toBe(0x56)
      expect(ba[3]).toBe(0x78)
    })

    test('LittleEndian encoding', async () => {
      const ba = new ByteArray()
      const s = new BinaryStream(ba)

      s.endian = BinaryStream.LittleEndian
      expect(s.endian).toBe(Endian.LittleEndian)

      await s.writeInteger32(0x12345678)

      // Check byte order for little endian
      expect(ba[0]).toBe(0x78)
      expect(ba[1]).toBe(0x56)
      expect(ba[2]).toBe(0x34)
      expect(ba[3]).toBe(0x12)
    })
  })

  await describe('Mixed Types', () => {
    test('read and write various types', async () => {
      const ba = new ByteArray()
      const s = new BinaryStream(ba)

      await s.writeByte(42)
      await s.writeShort(1000)
      await s.writeInteger32(100000)
      await s.writeDouble(3.14159)

      expect(await s.readByte()).toBe(42)
      expect(await s.readShort()).toBe(1000)
      expect(await s.readInteger32()).toBe(100000)
      expect(await s.readDouble()).toBeCloseTo(3.14159, 5)
    })
  })

  await describe('String Operations', () => {
    test('readString reads specified bytes', async () => {
      const ba = new ByteArray()
      ba.writeData('1234567890')

      const s = new BinaryStream(ba)
      expect(await s.readString(5)).toBe('12345')
      expect(await s.readString(5)).toBe('67890')
    })

    test('write and read strings', async () => {
      const ba = new ByteArray()
      const s = new BinaryStream(ba)

      await s.write('Hello world')
      expect(await s.readString(11)).toBe('Hello world')
    })
  })

  await describe('File Integration', () => {
    test('openBinaryStream for writing', async () => {
      const file = randomTestPath('binarystream', '.dat')
      const bs = await file.openBinaryStream('w')

      await bs.write('Hello world')
      bs.flush()
      await bs.close()

      expect(file.size).toBe(11)

      cleanupTestFile(file)
    })

    test('openBinaryStream for reading', async () => {
      const file = createTestFileSync(randomTestPath('binarystream-read', '.dat').name, '1234567890 test data')
      const bs = await file.openBinaryStream('r')

      expect(await bs.readString(5)).toBe('12345')
      expect(String.fromCharCode((await bs.readByte())!)).toBe('6')
      expect(String.fromCharCode((await bs.readByte())!)).toBe('7')
      expect(String.fromCharCode((await bs.readByte())!)).toBe('8')

      await bs.close()
      cleanupTestFile(file)
    })
  })
})
