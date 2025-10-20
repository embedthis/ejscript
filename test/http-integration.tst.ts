import { describe, it, expect, beforeAll, afterAll } from 'testme'
import { Http } from '../src/core/Http'
import { Uri } from '../src/core/utilities/Uri'
import { TestServer } from './helpers/test-server'
import { ByteArray } from '../src/core/streams/ByteArray'

await describe('Http Integration Tests', async () => {
    let server: TestServer
    let baseUrl: string

    beforeAll(async () => {
        server = new TestServer({ port: 0 }) // Auto-assign port
        await server.start()
        baseUrl = server.url
    })

    afterAll(async () => {
        await server.stop()
    })

    await describe('GET Requests', async () => {
        it('performs simple GET request', async () => {
            const http = new Http()
            await http.get(`${baseUrl}/index.html`)

            expect(http.status).toBe(200)
            expect(http.response).toContain('Hello /index.html')
            expect(http.response).toContain('<html>')
            http.close()
        })

        it('gets text response', async () => {
            const http = new Http()
            await http.get(`${baseUrl}/text`)

            expect(http.status).toBe(200)
            expect(http.response).toBe('Plain text response')
            http.close()
        })

        it('gets JSON response', async () => {
            const http = new Http()
            await http.get(`${baseUrl}/json`)

            expect(http.status).toBe(200)
            const data = JSON.parse(http.response)
            expect(data.message).toBe('Hello JSON')
            expect(typeof data.timestamp).toBe('number')
            http.close()
        })

        it('handles query parameters', async () => {
            const http = new Http()
            await http.get(`${baseUrl}/query?foo=bar&num=42`)

            expect(http.status).toBe(200)
            const data = JSON.parse(http.response)
            expect(data.foo).toBe('bar')
            expect(data.num).toBe('42')
            http.close()
        })

        it('reads response in chunks', async () => {
            const http = new Http()
            await http.get(`${baseUrl}/index.html`)

            expect(http.status).toBe(200)
            const chunk1 = http.readString(6)
            const chunk2 = http.readString(6)

            expect(chunk1).toBe('<html>')
            expect(chunk2).toBe('<head>')
            http.close()
        })

        it('handles large responses', async () => {
            const http = new Http()
            await http.get(`${baseUrl}/large`)

            expect(http.status).toBe(200)
            expect(http.response.length).toBeGreaterThan(100000)
            http.close()
        })
    })

    await describe('POST Requests', async () => {
        it('posts data to server', async () => {
            const http = new Http()
            await http.post(`${baseUrl}/echo`, 'Test data')

            expect(http.status).toBe(200)
            expect(http.response).toBe('Test data')
            http.close()
        })

        it('posts JSON data', async () => {
            const http = new Http()
            const data = { name: 'test', value: 123 }
            http.contentType = 'application/json'
            await http.post(`${baseUrl}/json`, JSON.stringify(data))

            expect(http.status).toBe(200)
            const response = JSON.parse(http.response)
            expect(response.received.name).toBe('test')
            expect(response.received.value).toBe(123)
            http.close()
        })

        it('posts form data', async () => {
            const http = new Http()
            await http.form(`${baseUrl}/form`, { field1: 'value1', field2: 'value2' })

            expect(http.status).toBe(200)
            const response = JSON.parse(http.response)
            expect(response.field1).toBe('value1')
            expect(response.field2).toBe('value2')
            http.close()
        })

        it('posts ByteArray data', async () => {
            const http = new Http()
            const buffer = new ByteArray(100)
            buffer.write('Binary data test')

            await http.post(`${baseUrl}/echo`, buffer)
            expect(http.status).toBe(200)
            expect(http.response).toContain('Binary data test')
            http.close()
        })
    })

    await describe('PUT Requests', async () => {
        it('sends PUT request', async () => {
            const http = new Http()
            await http.put(`${baseUrl}/resource`, 'Updated content')

            expect(http.status).toBe(200)
            expect(http.response).toBe('Resource updated')
            http.close()
        })
    })

    await describe('DELETE Requests', async () => {
        it('sends DELETE request', async () => {
            const http = new Http()
            await http.del(`${baseUrl}/resource`)

            expect(http.status).toBe(200)
            expect(http.response).toBe('Resource deleted')
            http.close()
        })
    })

    await describe('HTTP Headers', async () => {
        it('sends custom headers', async () => {
            const http = new Http()
            http.setHeader('X-Custom-Header', 'TestValue')
            await http.get(`${baseUrl}/headers`)

            expect(http.status).toBe(200)
            const headers = JSON.parse(http.response)
            expect(headers['x-custom-header']).toBe('TestValue')
            http.close()
        })

        it('reads response headers', async () => {
            const http = new Http()
            await http.get(`${baseUrl}/index.html`)

            expect(http.status).toBe(200)
            expect(http.contentType).toContain('text/html')

            const contentType = http.header('content-type')
            expect(contentType).toContain('text/html')
            http.close()
        })

        it('handles case-insensitive header names', async () => {
            const http = new Http()
            await http.get(`${baseUrl}/index.html`)

            const lower = http.header('content-type')
            const upper = http.header('Content-Type')
            const mixed = http.header('Content-Type')

            expect(lower).toBe(upper)
            expect(lower).toBe(mixed)
            http.close()
        })
    })

    await describe('HTTP Status Codes', async () => {
        it('handles 200 OK', async () => {
            const http = new Http()
            await http.get(`${baseUrl}/status/200`)
            expect(http.status).toBe(200)
            http.close()
        })

        it('handles 404 Not Found', async () => {
            const http = new Http()
            await http.get(`${baseUrl}/nonexistent`)
            expect(http.status).toBe(404)
            http.close()
        })

        it('handles 201 Created', async () => {
            const http = new Http()
            await http.get(`${baseUrl}/status/201`)
            expect(http.status).toBe(201)
            http.close()
        })

        it('handles 400 Bad Request', async () => {
            const http = new Http()
            await http.get(`${baseUrl}/status/400`)
            expect(http.status).toBe(400)
            http.close()
        })

        it('handles 500 Internal Server Error', async () => {
            const http = new Http()
            await http.get(`${baseUrl}/status/500`)
            expect(http.status).toBe(500)
            http.close()
        })
    })

    await describe('Redirects', async () => {
        it('follows redirects when enabled', async () => {
            const http = new Http()
            http.followRedirects = true
            await http.get(`${baseUrl}/redirect`)

            expect(http.status).toBe(200)
            expect(http.response).toContain('Redirected successfully')
            http.close()
        })

        it('does not follow redirects by default', async () => {
            const http = new Http()
            await http.get(`${baseUrl}/redirect`)

            expect(http.status).toBe(302)
            http.close()
        })
    })

    await describe('Cookies', async () => {
        it('receives cookies from server', async () => {
            const http = new Http()
            await http.get(`${baseUrl}/cookie`)

            expect(http.status).toBe(200)
            const setCookie = http.header('set-cookie')
            expect(setCookie).toContain('testcookie=testvalue')
            http.close()
        })
    })

    await describe('Authentication', async () => {
        it('sends basic authentication', async () => {
            const http = new Http()
            http.setCredentials('user', 'pass')
            await http.get(`${baseUrl}/auth`)

            expect(http.status).toBe(200)
            expect(http.response).toBe('Authenticated')
            http.close()
        })

        it('handles authentication failure', async () => {
            const http = new Http()
            http.setCredentials('wrong', 'credentials')
            await http.get(`${baseUrl}/auth`)

            expect(http.status).toBe(403)
            http.close()
        })

        it('returns 401 without credentials', async () => {
            const http = new Http()
            await http.get(`${baseUrl}/auth`)

            expect(http.status).toBe(401)
            http.close()
        })
    })

    await describe('Connection Management', async () => {
        it('reuses Http object for multiple requests', async () => {
            const http = new Http()

            await http.get(`${baseUrl}/index.html`)
            expect(http.status).toBe(200)

            await http.get(`${baseUrl}/text`)
            expect(http.status).toBe(200)

            await http.get(`${baseUrl}/json`)
            expect(http.status).toBe(200)

            http.close()
        })

        it('sets uri property', async () => {
            const http = new Http()
            http.uri = `${baseUrl}/index.html`
            await http.get()

            expect(http.status).toBe(200)
            expect(http.uri?.toString()).toContain('/index.html')
            http.close()
        })

        it('uses Uri object', async () => {
            const uri = new Uri(`${baseUrl}/index.html`)
            const http = new Http(uri)
            await http.get()

            expect(http.status).toBe(200)
            http.close()
        })
    })

    await describe('Method Variations', async () => {
        it('uses connectAsync() with GET', async () => {
            const http = new Http()
            const result = await http.connect('GET', `${baseUrl}/index.html`)

            expect(result).toBe(http)
            expect(http.status).toBe(200)
            http.close()
        })

        it('uses connectAsync() with POST', async () => {
            const http = new Http()
            await http.connect('POST', `${baseUrl}/echo`, 'Test data')

            expect(http.status).toBe(200)
            expect(http.response).toBe('Test data')
            http.close()
        })

        it('uses connectAsync() with PUT', async () => {
            const http = new Http()
            await http.connect('PUT', `${baseUrl}/resource`, 'Updated')

            expect(http.status).toBe(200)
            http.close()
        })

        it('uses connectAsync() with DELETE', async () => {
            const http = new Http()
            await http.connect('DELETE', `${baseUrl}/resource`)

            expect(http.status).toBe(200)
            http.close()
        })
    })

    await describe('Response Reading', async () => {
        it('reads response as string', async () => {
            const http = new Http()
            await http.get(`${baseUrl}/text`)

            const response = http.readString()
            expect(response).toBe('Plain text response')
            http.close()
        })

        it('reads response in chunks with readString()', async () => {
            const http = new Http()
            await http.get(`${baseUrl}/text`)

            const chunk1 = http.readString(5)
            const chunk2 = http.readString(5)

            expect(chunk1).toBe('Plain')
            expect(chunk2).toBe(' text')
            http.close()
        })

        it('gets full response property', async () => {
            const http = new Http()
            await http.get(`${baseUrl}/text`)

            expect(http.response).toBe('Plain text response')
            expect(http.response.length).toBe(19)
            http.close()
        })

        it('checks available property', async () => {
            const http = new Http()
            await http.get(`${baseUrl}/text`)

            expect(http.available).toBeGreaterThan(0)
            http.readString(5)
            expect(http.available).toBeLessThan(19)
            http.close()
        })
    })

    await describe('Content Types', async () => {
        it('sets content-type for POST', async () => {
            const http = new Http()
            http.contentType = 'application/json'
            await http.post(`${baseUrl}/echo`, '{"test": true}')

            expect(http.status).toBe(200)
            http.close()
        })

        it('detects JSON content type', async () => {
            const http = new Http()
            await http.get(`${baseUrl}/json`)

            expect(http.contentType).toContain('application/json')
            http.close()
        })

        it('detects HTML content type', async () => {
            const http = new Http()
            await http.get(`${baseUrl}/index.html`)

            expect(http.contentType).toContain('text/html')
            http.close()
        })
    })

    await describe('Error Handling', async () => {
        it('handles connection to invalid host', async () => {
            const http = new Http()
            http.setLimits({ requestTimeout: 2 })  // 2 second timeout
            try {
                await http.get('http://invalid-host-that-does-not-exist.local/')
                // If we get here, request failed but didn't throw
                expect(http.status).not.toBe(200)
            } catch (e) {
                // Connection error expected (timeout or DNS failure)
                expect(e).toBeTruthy()
            }
            http.close()
        })
    })

    await describe('Protocol Detection', async () => {
        it('detects non-secure connection', async () => {
            const http = new Http()
            await http.get(`${baseUrl}/index.html`)

            expect(http.isSecure).toBe(false)
            http.close()
        })
    })
})
