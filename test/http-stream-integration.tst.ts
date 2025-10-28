/**
 * Integration tests for HTTP streaming with a real server
 */

import { describe, it, expect, beforeAll, afterAll } from 'testme'
import { Http } from '../src/core/Http'

const TEST_PORT = 4450  // Use unique port to avoid conflicts
let server: ReturnType<typeof Bun.serve>

await describe('Http Streaming Integration', async () => {
    beforeAll(async () => {
        // Create a simple test server
        try {
            server = Bun.serve({
                port: TEST_PORT,
                fetch: async (req: Request) => {
                    const url = new URL(req.url)

                    if (url.pathname === '/echo') {
                        // Echo back the request body
                        const body = await req.text()
                        return new Response(JSON.stringify({
                            method: req.method,
                            body: body,
                            contentType: req.headers.get('content-type')
                        }), {
                            headers: { 'Content-Type': 'application/json' }
                        })
                    }

                    return new Response('Not Found', { status: 404 })
                }
            })
            // Give server time to start
            await new Promise(resolve => setTimeout(resolve, 200))
        } catch (error) {
            console.error('Server start error:', error)
            throw error
        }
    })

    afterAll(() => {
        if (server) {
            server.stop()
        }
    })

    await describe('ReadableStream as data parameter', async () => {
        it('should POST with ReadableStream', async () => {
            const stream = new ReadableStream({
                start(controller) {
                    controller.enqueue(new TextEncoder().encode('Hello '))
                    controller.enqueue(new TextEncoder().encode('Streaming'))
                    controller.close()
                }
            })

            const http = new Http()
            http.post(`127.0.0.1:${TEST_PORT}/echo`, stream)
            await http.finalize()

            expect(http.status).toBe(200)
            const response = JSON.parse(http.response)
            expect(response.body).toBe('Hello Streaming')
            expect(response.method).toBe('POST')
        })

        it('should PUT with ReadableStream', async () => {
            const stream = new ReadableStream({
                start(controller) {
                    controller.enqueue(new TextEncoder().encode('PUT data'))
                    controller.close()
                }
            })

            const http = new Http()
            http.put(`127.0.0.1:${TEST_PORT}/echo`, stream)
            await http.finalize()

            expect(http.status).toBe(200)
            const response = JSON.parse(http.response)
            expect(response.body).toBe('PUT data')
            expect(response.method).toBe('PUT')
        })

        it('should handle large streaming data', async () => {
            const chunkSize = 1024
            const numChunks = 10

            const stream = new ReadableStream({
                start(controller) {
                    for (let i = 0; i < numChunks; i++) {
                        const chunk = new Uint8Array(chunkSize).fill(65 + (i % 26))
                        controller.enqueue(chunk)
                    }
                    controller.close()
                }
            })

            const http = new Http()
            http.post(`127.0.0.1:${TEST_PORT}/echo`, stream)
            await http.finalize()

            expect(http.status).toBe(200)
            const response = JSON.parse(http.response)
            expect(response.body.length).toBe(chunkSize * numChunks)
        })
    })

    await describe('Incremental write() API', async () => {
        it('should build request with multiple write() calls', async () => {
            const http = new Http()
            http.connect('POST', `127.0.0.1:${TEST_PORT}/echo`)

            http.write('Part1 ')
            http.write('Part2 ')
            http.write('Part3')

            await http.finalize()

            expect(http.status).toBe(200)
            const response = JSON.parse(http.response)
            expect(response.body).toBe('Part1 Part2 Part3')
        })

        it('should write binary data', async () => {
            const http = new Http()
            http.connect('POST', `127.0.0.1:${TEST_PORT}/echo`)

            const data1 = new TextEncoder().encode('Binary ')
            const data2 = new TextEncoder().encode('Content')

            http.write(data1)
            http.write(data2)

            await http.finalize()

            expect(http.status).toBe(200)
            const response = JSON.parse(http.response)
            expect(response.body).toBe('Binary Content')
        })

        it('should handle large incremental writes', async () => {
            const http = new Http()
            http.connect('POST', `127.0.0.1:${TEST_PORT}/echo`)

            for (let i = 0; i < 100; i++) {
                http.write(`Chunk${i} `)
            }

            await http.finalize()

            expect(http.status).toBe(200)
            const response = JSON.parse(http.response)
            expect(response.body).toContain('Chunk0 ')
            expect(response.body).toContain('Chunk99 ')
        })
    })

    await describe('File streaming', async () => {
        it('should stream file content', async () => {
            const testFile = `.test/stream-integration-${process.pid}.txt`
            await Bun.write(testFile, 'File content to stream')

            const file = Bun.file(testFile)
            const stream = file.stream()

            const http = new Http()
            http.post(`127.0.0.1:${TEST_PORT}/echo`, stream)
            await http.finalize()

            expect(http.status).toBe(200)
            const response = JSON.parse(http.response)
            expect(response.body).toBe('File content to stream')

            // Cleanup
            await import('fs').then(fs => fs.promises.unlink(testFile).catch(() => {}))
        })

        it('should stream large file', async () => {
            const testFile = `.test/stream-large-${process.pid}.txt`
            const largeContent = 'X'.repeat(5000)
            await Bun.write(testFile, largeContent)

            const file = Bun.file(testFile)
            const stream = file.stream()

            const http = new Http()
            http.post(`127.0.0.1:${TEST_PORT}/echo`, stream)
            await http.finalize()

            expect(http.status).toBe(200)
            const response = JSON.parse(http.response)
            expect(response.body.length).toBe(5000)

            // Cleanup
            await import('fs').then(fs => fs.promises.unlink(testFile).catch(() => {}))
        })
    })

    await describe('Async stream generation', async () => {
        it('should handle async data in ReadableStream', async () => {
            const stream = new ReadableStream({
                async start(controller) {
                    for (let i = 0; i < 5; i++) {
                        await new Promise(resolve => setTimeout(resolve, 5))
                        controller.enqueue(new TextEncoder().encode(`Async${i} `))
                    }
                    controller.close()
                }
            })

            const http = new Http()
            http.post(`127.0.0.1:${TEST_PORT}/echo`, stream)
            await http.finalize()

            expect(http.status).toBe(200)
            const response = JSON.parse(http.response)
            expect(response.body).toBe('Async0 Async1 Async2 Async3 Async4 ')
        })
    })

    await describe('Stream reuse', async () => {
        it('should allow multiple requests after reset()', async () => {
            const http = new Http()

            // First request
            http.connect('POST', `127.0.0.1:${TEST_PORT}/echo`)
            http.write('First')
            await http.finalize()

            const first = JSON.parse(http.response)
            expect(first.body).toBe('First')

            // Reset and second request
            http.reset()
            http.connect('POST', `127.0.0.1:${TEST_PORT}/echo`)
            http.write('Second')
            await http.finalize()

            const second = JSON.parse(http.response)
            expect(second.body).toBe('Second')
        })
    })
})
