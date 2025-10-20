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
    })

    afterAll(() => {
        server?.stop()
    })

    it('should complete port-only URL: "14100/index.html"', async () => {
        const http = new Http()
        await http.get('14100/index.html')
        expect(http.status).toBe(200)
        expect(http.response).toContain('Path: /index.html')
    })

    it('should complete IP without scheme: "127.0.0.1:14100/test.html"', async () => {
        const http = new Http()
        await http.get('127.0.0.1:14100/test.html')
        expect(http.status).toBe(200)
        expect(http.response).toContain('Path: /test.html')
    })

    it('should complete colon-port URL: ":14100/page.html"', async () => {
        const http = new Http()
        await http.get(':14100/page.html')
        expect(http.status).toBe(200)
        expect(http.response).toContain('Path: /page.html')
    })

    it('should handle localhost without scheme: "localhost:14100/file.html"', async () => {
        const http = new Http()
        await http.get('localhost:14100/file.html')
        expect(http.status).toBe(200)
        expect(http.response).toContain('Path: /file.html')
    })

    it('should not modify complete URLs with http scheme', async () => {
        const http = new Http()
        await http.get('http://127.0.0.1:14100/complete.html')
        expect(http.status).toBe(200)
        expect(http.response).toContain('Path: /complete.html')
    })

    it('should work with POST requests on partial URLs', async () => {
        const http = new Http()
        await http.post('14100/data', 'test data')
        expect(http.status).toBe(200)
        expect(http.response).toContain('Path: /data')
    })
})
