/**
 * Test partial URL completion in Http class
 */
import { describe, it, expect, beforeAll, afterAll } from 'testme'
import { Http } from '../../src/core/Http'

await describe('Http partial URL completion', async () => {
    // Test server setup
    let server: any
    const TEST_PORT = 14100

    beforeAll(async () => {
        // Start a simple test server
        try {
            server = Bun.serve({
                port: TEST_PORT,
                fetch(req) {
                    const url = new URL(req.url)
                    return new Response(`Path: ${url.pathname}`, {
                        status: 200,
                        headers: { 'Content-Type': 'text/plain' }
                    })
                }
            })

            // Verify server is running by checking if it's listening
            if (!server) {
                throw new Error('Server failed to start')
            }

            // Give server more time to fully initialize
            await new Promise(resolve => setTimeout(resolve, 500))

            // Verify we can connect to the server
            try {
                const testResponse = await fetch(`http://127.0.0.1:${TEST_PORT}/test`)
                if (!testResponse.ok && testResponse.status !== 200) {
                    throw new Error(`Server not responding correctly: ${testResponse.status}`)
                }
            } catch (e: any) {
                server.stop()
                throw new Error(`Server started but not accessible: ${e.message}`)
            }
        } catch (err: any) {
            throw new Error(`Failed to start server on port ${TEST_PORT}: ${err.message}`)
        }
    })

    afterAll(() => {
        server?.stop()
    })

    it('should complete port-only URL: "14100/index.html"', async () => {
        const http = new Http()
        http.get('14100/index.html')
        await http.wait()
        expect(http.status).toBe(200)
        expect(http.response).toContain('Path: /index.html')
    })

    it('should complete IP without scheme: "127.0.0.1:14100/test.html"', async () => {
        const http = new Http()
        http.get('127.0.0.1:14100/test.html')
        await http.wait()
        expect(http.status).toBe(200)
        expect(http.response).toContain('Path: /test.html')
    })

    it('should complete colon-port URL: ":14100/page.html"', async () => {
        const http = new Http()
        http.get(':14100/page.html')
        await http.wait()
        expect(http.status).toBe(200)
        expect(http.response).toContain('Path: /page.html')
    })

    it('should handle localhost without scheme: "localhost:14100/file.html"', async () => {
        const http = new Http()
        http.get('localhost:14100/file.html')
        await http.wait()
        expect(http.status).toBe(200)
        expect(http.response).toContain('Path: /file.html')
    })

    it('should not modify complete URLs with http scheme', async () => {
        const http = new Http()
        http.get('http://127.0.0.1:14100/complete.html')
        await http.wait()
        expect(http.status).toBe(200)
        expect(http.response).toContain('Path: /complete.html')
    })

    it('should work with POST requests on partial URLs', async () => {
        const http = new Http()
        http.post('14100/data', 'test data')
        await http.wait()
        expect(http.status).toBe(200)
        expect(http.response).toContain('Path: /data')
    })
})
