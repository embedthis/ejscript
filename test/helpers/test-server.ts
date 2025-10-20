/**
 * Simple HTTP test server for integration testing
 * Uses Bun.serve() to create a test server
 */

export interface TestServerOptions {
    port?: number
    host?: string
}

export class TestServer {
    private server?: ReturnType<typeof Bun.serve>
    private _port: number
    private _host: string
    private _running: boolean = false

    constructor(options: TestServerOptions = {}) {
        this._port = options.port || 0 // 0 = auto-assign
        this._host = options.host || 'localhost'
    }

    get port(): number {
        return this._port
    }

    get host(): string {
        return this._host
    }

    get url(): string {
        return `http://${this._host}:${this._port}`
    }

    get running(): boolean {
        return this._running
    }

    /**
     * Start the test server
     */
    async start(): Promise<void> {
        if (this._running) {
            return
        }

        this.server = Bun.serve({
            port: this._port,
            hostname: this._host,

            fetch: async (req: Request): Promise<Response> => {
                const url = new URL(req.url)
                const path = url.pathname
                const method = req.method

                // Route: GET /
                if (method === 'GET' && path === '/') {
                    return new Response('<html><head><title>Test Server</title></head><body>Hello World</body></html>', {
                        headers: { 'Content-Type': 'text/html' },
                    })
                }

                // Route: GET /index.html
                if (method === 'GET' && path === '/index.html') {
                    return new Response('<html><head><title>index.html</title></head><body>Hello /index.html</body></html>', {
                        headers: { 'Content-Type': 'text/html' },
                    })
                }

                // Route: GET /json
                if (method === 'GET' && path === '/json') {
                    return Response.json({ message: 'Hello JSON', timestamp: Date.now() })
                }

                // Route: GET /text
                if (method === 'GET' && path === '/text') {
                    return new Response('Plain text response', {
                        headers: { 'Content-Type': 'text/plain' },
                    })
                }

                // Route: GET /headers - echo request headers
                if (method === 'GET' && path === '/headers') {
                    const headers: Record<string, string> = {}
                    req.headers.forEach((value, key) => {
                        headers[key] = value
                    })
                    return Response.json(headers)
                }

                // Route: GET /query - echo query parameters
                if (method === 'GET' && path === '/query') {
                    const params: Record<string, string> = {}
                    url.searchParams.forEach((value, key) => {
                        params[key] = value
                    })
                    return Response.json(params)
                }

                // Route: POST /echo - echo request body
                if (method === 'POST' && path === '/echo') {
                    const body = await req.text()
                    return new Response(body, {
                        headers: { 'Content-Type': req.headers.get('content-type') || 'text/plain' },
                    })
                }

                // Route: POST /json - accept and echo JSON
                if (method === 'POST' && path === '/json') {
                    const data = await req.json()
                    return Response.json({ received: data })
                }

                // Route: POST /form - handle form data
                if (method === 'POST' && path === '/form') {
                    const formData = await req.formData()
                    const result: Record<string, any> = {}
                    formData.forEach((value, key) => {
                        result[key] = value
                    })
                    return Response.json(result)
                }

                // Route: PUT /resource
                if (method === 'PUT' && path === '/resource') {
                    const body = await req.text()
                    return new Response('Resource updated', {
                        status: 200,
                        headers: { 'Content-Type': 'text/plain' },
                    })
                }

                // Route: DELETE /resource
                if (method === 'DELETE' && path === '/resource') {
                    return new Response('Resource deleted', {
                        status: 200,
                        headers: { 'Content-Type': 'text/plain' },
                    })
                }

                // Route: GET /status/:code - return specific status code
                const statusMatch = path.match(/^\/status\/(\d+)$/)
                if (method === 'GET' && statusMatch) {
                    const code = parseInt(statusMatch[1])
                    return new Response(`Status ${code}`, { status: code })
                }

                // Route: GET /redirect - redirect to /redirected
                if (method === 'GET' && path === '/redirect') {
                    return Response.redirect(`${this.url}/redirected`, 302)
                }

                // Route: GET /redirected
                if (method === 'GET' && path === '/redirected') {
                    return new Response('Redirected successfully')
                }

                // Route: GET /delay/:ms - delay response
                const delayMatch = path.match(/^\/delay\/(\d+)$/)
                if (method === 'GET' && delayMatch) {
                    const ms = parseInt(delayMatch[1])
                    await new Promise(resolve => setTimeout(resolve, ms))
                    return new Response(`Delayed ${ms}ms`)
                }

                // Route: GET /large - return large response
                if (method === 'GET' && path === '/large') {
                    const size = 1024 * 100 // 100KB
                    const data = 'x'.repeat(size)
                    return new Response(data, {
                        headers: { 'Content-Type': 'text/plain' },
                    })
                }

                // Route: GET /chunked - test chunked encoding
                if (method === 'GET' && path === '/chunked') {
                    const chunks = ['chunk1\n', 'chunk2\n', 'chunk3\n']
                    const stream = new ReadableStream({
                        async start(controller) {
                            for (const chunk of chunks) {
                                controller.enqueue(new TextEncoder().encode(chunk))
                                await new Promise(resolve => setTimeout(resolve, 10))
                            }
                            controller.close()
                        },
                    })
                    return new Response(stream, {
                        headers: { 'Content-Type': 'text/plain' },
                    })
                }

                // Route: GET /cookie - set cookie
                if (method === 'GET' && path === '/cookie') {
                    return new Response('Cookie set', {
                        headers: {
                            'Set-Cookie': 'testcookie=testvalue; Path=/; HttpOnly',
                        },
                    })
                }

                // Route: GET /auth - basic auth test
                if (method === 'GET' && path === '/auth') {
                    const auth = req.headers.get('authorization')
                    if (!auth || !auth.startsWith('Basic ')) {
                        return new Response('Unauthorized', {
                            status: 401,
                            headers: { 'WWW-Authenticate': 'Basic realm="Test"' },
                        })
                    }
                    const decoded = atob(auth.substring(6))
                    if (decoded === 'user:pass') {
                        return new Response('Authenticated')
                    }
                    return new Response('Invalid credentials', { status: 403 })
                }

                // 404 Not Found
                return new Response('Not Found', { status: 404 })
            },
        })

        // Get the actual port if auto-assigned
        if (this._port === 0) {
            this._port = this.server.port
        }

        this._running = true

        // Unref the server so it doesn't keep the process alive
        this.server.unref()

        // Wait a bit for server to be ready
        await new Promise(resolve => setTimeout(resolve, 50))
    }

    /**
     * Stop the test server
     */
    async stop(): Promise<void> {
        if (this.server) {
            this.server.stop()
            this._running = false
            await new Promise(resolve => setTimeout(resolve, 50))
        }
    }
}
