/**
 * Test streaming POST/PUT operations in Http
 */

import { describe, it, expect, beforeAll, afterAll } from 'testme'
import { Http } from '../src/core/Http'
import { TestServer } from './helpers/test-server'

const TEST_PORT = 4401
let server: TestServer

await describe('Http Streaming', async () => {
    beforeAll(async () => {
        // Start test server using TestServer class
        server = new TestServer({ port: TEST_PORT })
        await server.start()
    })

    afterAll(async () => {
        if (server) {
            await server.stop()
        }
    })

    await describe('ReadableStream as data parameter', async () => {
        it('should accept ReadableStream in post() data parameter', async () => {
            const stream = new ReadableStream({
                start(controller) {
                    controller.enqueue(new TextEncoder().encode('Hello '))
                    controller.enqueue(new TextEncoder().encode('Streaming '))
                    controller.enqueue(new TextEncoder().encode('World'))
                    controller.close()
                }
            })

            const http = new Http()
            http.post(`127.0.0.1:${TEST_PORT}/echo`, stream)
            await http.wait()

            expect(http.status).toBe(200)
            const response = JSON.parse(http.response)
            expect(response.body).toBe('Hello Streaming World')
        })

        it('should accept ReadableStream in put() data parameter', async () => {
            const stream = new ReadableStream({
                start(controller) {
                    controller.enqueue(new TextEncoder().encode('PUT data'))
                    controller.close()
                }
            })

            const http = new Http()
            http.put(`127.0.0.1:${TEST_PORT}/echo`, stream)
            await http.wait()

            expect(http.status).toBe(200)
            const response = JSON.parse(http.response)
            expect(response.body).toBe('PUT data')
        })

        it('should handle large streaming data', async () => {
            // Create a stream that sends 10KB of data
            const chunkSize = 1024
            const numChunks = 10
            const stream = new ReadableStream({
                start(controller) {
                    for (let i = 0; i < numChunks; i++) {
                        const chunk = new Uint8Array(chunkSize).fill(65 + (i % 26)) // A-Z
                        controller.enqueue(chunk)
                    }
                    controller.close()
                }
            })

            const http = new Http()
            http.post(`127.0.0.1:${TEST_PORT}/echo`, stream)
            await http.wait()

            expect(http.status).toBe(200)
            const response = JSON.parse(http.response)
            expect(response.body.length).toBe(chunkSize * numChunks)
        })
    })

    await describe('Incremental write() API', async () => {
        it('should build request body with multiple write() calls', async () => {
            const http = new Http()
            http.method = 'POST'
            http.uri = `127.0.0.1:${TEST_PORT}/echo`

            const bytes1 = http.write('Chunk1 ')
            const bytes2 = http.write('Chunk2 ')
            const bytes3 = http.write('Chunk3')

            expect(bytes1).toBeGreaterThan(0)
            expect(bytes2).toBeGreaterThan(0)
            expect(bytes3).toBeGreaterThan(0)

            http.connect('POST')
            await http.finalize()

            expect(http.status).toBe(200)
            const response = JSON.parse(http.response)
            expect(response.body).toBe('Chunk1 Chunk2 Chunk3')
        })

        it('should write binary data with Uint8Array', async () => {
            const http = new Http()
            http.method = 'POST'
            http.uri = `127.0.0.1:${TEST_PORT}/echo`

            const data1 = new TextEncoder().encode('Binary ')
            const data2 = new TextEncoder().encode('Data')

            const bytes1 = http.write(data1)
            const bytes2 = http.write(data2)

            expect(bytes1).toBe(7)
            expect(bytes2).toBe(4)

            http.connect('POST')
            await http.finalize()

            expect(http.status).toBe(200)
            const response = JSON.parse(http.response)
            expect(response.body).toBe('Binary Data')
        })

        it('should serialize objects as JSON in write()', async () => {
            const http = new Http()
            http.method = 'POST'
            http.uri = `127.0.0.1:${TEST_PORT}/echo`

            const obj = { message: 'Hello', count: 42 }
            const bytes = http.write(obj)

            expect(bytes).toBeGreaterThan(0)

            http.connect('POST')
            await http.finalize()

            expect(http.status).toBe(200)
            const response = JSON.parse(http.response)
            expect(response.body).toBe(JSON.stringify(obj))
        })

        it('should handle mixed data types in write()', async () => {
            const http = new Http()
            http.method = 'POST'
            http.uri = `127.0.0.1:${TEST_PORT}/echo`

            http.write('String ')
            http.write(new TextEncoder().encode('Binary '))
            http.write({ type: 'object' })

            http.connect('POST')
            await http.finalize()

            expect(http.status).toBe(200)
            const response = JSON.parse(http.response)
            expect(response.body).toContain('String ')
            expect(response.body).toContain('Binary ')
            expect(response.body).toContain('{"type":"object"}')
        })

        it('should write large amounts of data incrementally', async () => {
            const http = new Http()
            http.method = 'POST'
            http.uri = `127.0.0.1:${TEST_PORT}/echo`

            let totalBytes = 0
            for (let i = 0; i < 100; i++) {
                const chunk = `Chunk${i} `
                totalBytes += http.write(chunk)
            }

            expect(totalBytes).toBeGreaterThan(0)

            http.connect('POST')
            await http.finalize()

            expect(http.status).toBe(200)
            const response = JSON.parse(http.response)
            expect(response.body).toContain('Chunk0 ')
            expect(response.body).toContain('Chunk99 ')
        })
    })

    await describe('Stream lifecycle', async () => {
        it('should reset streaming state on reset()', async () => {
            const http = new Http()
            http.method = 'POST'
            http.uri = `127.0.0.1:${TEST_PORT}/echo`

            http.write('First request')
            http.connect('POST')
            await http.finalize()

            const firstResponse = JSON.parse(http.response)
            expect(firstResponse.body).toBe('First request')

            // Reset and make new request
            http.reset()
            http.method = 'POST'
            http.uri = `127.0.0.1:${TEST_PORT}/echo`

            http.write('Second request')
            http.connect('POST')
            await http.finalize()

            const secondResponse = JSON.parse(http.response)
            expect(secondResponse.body).toBe('Second request')
        })

        it('should properly close stream on finalize()', async () => {
            const http = new Http()
            http.method = 'POST'
            http.uri = `127.0.0.1:${TEST_PORT}/echo`

            http.write('Data')
            expect(http.finalized).toBe(false)

            http.connect('POST')
            await http.finalize()
            expect(http.finalized).toBe(true)
            expect(http.status).toBe(200)
        })
    })

    await describe('File streaming', async () => {
        it('should stream file content using Bun.file()', async () => {
            // Create a test file
            const testFile = `.test/stream-test-${process.pid}.txt`
            await Bun.write(testFile, 'File content for streaming')

            const file = Bun.file(testFile)
            const stream = file.stream()

            const http = new Http()
            http.post(`127.0.0.1:${TEST_PORT}/echo`, stream)
            await http.wait()

            expect(http.status).toBe(200)
            const response = JSON.parse(http.response)
            expect(response.body).toBe('File content for streaming')

            // Cleanup
            await Bun.write(testFile, '')
            await import('fs').then(fs => fs.promises.unlink(testFile))
        })

        it('should stream large file content', async () => {
            // Create a larger test file (10KB)
            const testFile = `.test/stream-large-${process.pid}.txt`
            const largeContent = 'X'.repeat(10240)
            await Bun.write(testFile, largeContent)

            const file = Bun.file(testFile)
            const stream = file.stream()

            const http = new Http()
            http.post(`127.0.0.1:${TEST_PORT}/echo`, stream)
            await http.wait()

            expect(http.status).toBe(200)
            const response = JSON.parse(http.response)
            expect(response.body.length).toBe(10240)
            expect(response.body).toBe(largeContent)

            // Cleanup
            await import('fs').then(fs => fs.promises.unlink(testFile))
        })
    })

    await describe('Streaming with authentication', async () => {
        it('should stream with basic authentication', async () => {
            const stream = new ReadableStream({
                start(controller) {
                    controller.enqueue(new TextEncoder().encode('Authenticated data'))
                    controller.close()
                }
            })

            const http = new Http()
            http.setCredentials('testuser', 'testpass', 'basic')
            http.post(`127.0.0.1:${TEST_PORT}/auth/basic`, stream)
            await http.wait()

            expect(http.status).toBe(200)
            const response = JSON.parse(http.response)
            expect(response.authenticated).toBe(true)
            expect(response.body).toBe('Authenticated data')
        })

        it('should stream with digest authentication', async () => {
            // Note: Digest auth requires two requests (401 challenge + authenticated request)
            // ReadableStreams can only be consumed once, so we use string data instead
            const http = new Http()
            http.setCredentials('testuser', 'testpass')
            http.post(`127.0.0.1:${TEST_PORT}/auth/digest`, 'Digest auth data')
            await http.wait()

            expect(http.status).toBe(200)
            const response = JSON.parse(http.response)
            expect(response.authenticated).toBe(true)
            expect(response.body).toBe('Digest auth data')
        })
    })

    await describe('Custom ReadableStream creation', async () => {
        it('should handle async data generation in stream', async () => {
            const stream = new ReadableStream({
                async start(controller) {
                    for (let i = 0; i < 5; i++) {
                        await new Promise(resolve => setTimeout(resolve, 10))
                        controller.enqueue(new TextEncoder().encode(`Async${i} `))
                    }
                    controller.close()
                }
            })

            const http = new Http()
            http.post(`127.0.0.1:${TEST_PORT}/echo`, stream)
            await http.wait()

            expect(http.status).toBe(200)
            const response = JSON.parse(http.response)
            expect(response.body).toBe('Async0 Async1 Async2 Async3 Async4 ')
        })
    })
})
