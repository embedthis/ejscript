/**
 * Test newly implemented methods
 */

import { describe, it, expect } from 'testme'
import { Path } from '../src/core/Path'
import { File } from '../src/core/File'
import { ByteArray } from '../src/core/streams/ByteArray'
import * as fs from 'fs'

await describe('Newly Implemented Methods', async () => {
    await describe('Path.open()', async () => {
        it('should return a File object', async () => {
            const testFile = `.test/path-open-${process.pid}.txt`
            const path = new Path(testFile)

            const file = await path.open({ mode: 'w' })

            expect(file).toBeInstanceOf(File)
            expect(file.canWrite).toBe(true)

            await file.write('Test content')
            await file.close()

            // Cleanup
            fs.unlinkSync(testFile)
        })

        it('should create file for writing', async () => {
            const testFile = `.test/path-open-write-${process.pid}.txt`
            const path = new Path(testFile)

            const file = await path.open({ mode: 'w' })
            await file.write('Hello from Path.open()')
            await file.close()

            const content = await Bun.file(testFile).text()
            expect(content).toBe('Hello from Path.open()')

            // Cleanup
            fs.unlinkSync(testFile)
        })

        it('should open file for reading', async () => {
            const testFile = `.test/path-open-read-${process.pid}.txt`
            await Bun.write(testFile, 'Read this content')

            const path = new Path(testFile)
            const file = await path.open({ mode: 'r' })

            expect(file.canRead).toBe(true)
            const content = await file.readString()
            expect(content).toBe('Read this content')

            await file.close()

            // Cleanup
            fs.unlinkSync(testFile)
        })
    })

    await describe('File.encoding setter', async () => {
        it('should set and get encoding', () => {
            const file = new File('.test/dummy.txt')

            expect(file.encoding).toBe('utf-8')

            file.encoding = 'ascii'
            expect(file.encoding).toBe('ascii')

            file.encoding = 'latin1'
            expect(file.encoding).toBe('latin1')
        })

        it('should normalize UTF-8 encoding names', () => {
            const file = new File('.test/dummy.txt')

            file.encoding = 'utf8'
            expect(file.encoding).toBe('utf-8')

            file.encoding = 'UTF-8'
            expect(file.encoding).toBe('utf-8')
        })

        it('should reject unsupported encodings', () => {
            const file = new File('.test/dummy.txt')

            expect(() => {
                file.encoding = 'invalid-encoding'
            }).toThrow('Unsupported encoding')
        })

        it('should write with ASCII encoding', async () => {
            const testFile = `.test/encoding-ascii-${process.pid}.txt`
            const file = new File(testFile, { mode: 'w' })
            await file.open()

            file.encoding = 'ascii'
            await file.write('ASCII text')
            await file.close()

            const content = await Bun.file(testFile).text()
            expect(content).toBe('ASCII text')

            // Cleanup
            fs.unlinkSync(testFile)
        })

        it('should read with specified encoding', async () => {
            const testFile = `.test/encoding-read-${process.pid}.txt`
            await Bun.write(testFile, 'Test content')

            const file = new File(testFile, { mode: 'r' })
            await file.open()
            file.encoding = 'utf-8'

            const content = await file.readString()
            expect(content).toBe('Test content')

            await file.close()

            // Cleanup
            fs.unlinkSync(testFile)
        })
    })

    await describe('ByteArray with sufficient initial size', async () => {
        it('should work within allocated size', () => {
            const ba = new ByteArray(100, true) // Allocate enough space

            // Write 50 bytes (within capacity)
            const data = new Uint8Array(50).fill(42)
            ba.write(data)

            expect((ba as any)._writePosition).toBe(50)
            expect((ba as any)._size).toBe(100)

            // Verify data was written
            for (let i = 0; i < 50; i++) {
                expect(ba[i]).toBe(42)
            }
        })

        it('should throw clear error when exceeding capacity', () => {
            const ba = new ByteArray(10, true)

            expect(() => {
                ba.write(new Uint8Array(20).fill(1))
            }).toThrow('ByteArray overflow')
        })

        it('should throw error if not growable', () => {
            const ba = new ByteArray(10, false) // Not growable

            expect(() => {
                ba.write(new Uint8Array(20).fill(1))
            }).toThrow('ByteArray is not growable')
        })

        it('should handle large pre-allocated buffer', () => {
            const ba = new ByteArray(100000, true)

            // Write 50KB of data
            const largeData = new Uint8Array(50000).fill(65)
            ba.write(largeData)

            expect((ba as any)._writePosition).toBe(50000)
            expect((ba as any)._size).toBe(100000)

            // Verify toString works on large buffer
            const str = ba.toString()
            expect(str.length).toBe(50000)
            expect(str[0]).toBe('A') // 65 is 'A'
        })

        it('should handle string writes within capacity', () => {
            const ba = new ByteArray(100, true)

            ba.write('Hello World!')

            expect(ba.toString()).toBe('Hello World!')
        })

        it('should handle mixed data types within capacity', () => {
            const ba = new ByteArray(100, true)

            ba.write('String ')
            ba.write(new Uint8Array([65, 66, 67])) // 'ABC'
            ba.write(' End')

            const str = ba.toString()
            expect(str).toBe('String ABC End')
        })

        it('should provide helpful error message on overflow', () => {
            const ba = new ByteArray(10, true)

            try {
                ba.write(new Uint8Array(20).fill(1))
                expect(false).toBe(true) // Should not reach here
            } catch (error: any) {
                expect(error.message).toContain('ByteArray overflow')
                expect(error.message).toContain('need 20 bytes')
                expect(error.message).toContain('only 10 available')
            }
        })
    })

    await describe('Integration: All three features together', async () => {
        it('should use Path.open() with File.encoding on large content', async () => {
            const testFile = `.test/integration-${process.pid}.txt`
            const path = new Path(testFile)

            // Write with File opened via Path.open()
            const writeFile = await path.open({ mode: 'w' })
            writeFile.encoding = 'utf-8'

            // Write large content
            for (let i = 0; i < 100; i++) {
                await writeFile.write(`Line ${i}\n`)
            }
            await writeFile.close()

            // Read back
            const readFile = await path.open({ mode: 'r' })
            readFile.encoding = 'utf-8'
            const content = await readFile.readString()
            await readFile.close()

            expect(content).toContain('Line 0')
            expect(content).toContain('Line 99')

            // Cleanup
            fs.unlinkSync(testFile)
        })
    })
})
