import { describe, it, expect, beforeAll, afterAll } from '@embedthis/testme'
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
            http.get(`${baseUrl}/index.html`)
            await http.wait()

            expect(http.status).toBe(200)
            expect(http.response).toContain('Hello /index.html')
            expect(http.response).toContain('<html>')
            http.close()
        })

        it('gets text response', async () => {
            const http = new Http()
            http.get(`${baseUrl}/text`)
            await http.finalize()

            expect(http.status).toBe(200)
            expect(http.response).toBe('Plain text response')
            http.close()
        })

        it('gets JSON response', async () => {
            const http = new Http()
            http.get(`${baseUrl}/json`)
            await http.finalize()

            expect(http.status).toBe(200)
            const data = JSON.parse(http.response)
            expect(data.message).toBe('Hello JSON')
            expect(typeof data.timestamp).toBe('number')
            http.close()
        })

        it('handles query parameters', async () => {
            const http = new Http()
            http.get(`${baseUrl}/query?foo=bar&num=42`)
            await http.finalize()

            expect(http.status).toBe(200)
            const data = JSON.parse(http.response)
            expect(data.foo).toBe('bar')
            expect(data.num).toBe('42')
            http.close()
        })

        it('reads response in chunks', async () => {
            const http = new Http()
            http.get(`${baseUrl}/index.html`)
            await http.finalize()

            expect(http.status).toBe(200)
            const chunk1 = http.readString(6)
            const chunk2 = http.readString(6)

            expect(chunk1).toBe('<html>')
            expect(chunk2).toBe('<head>')
            http.close()
        })

        it('handles large responses', async () => {
            const http = new Http()
            http.get(`${baseUrl}/large`)
            await http.finalize()

            expect(http.status).toBe(200)
            expect(http.response.length).toBeGreaterThan(100000)
            http.close()
        })
    })

    await describe('POST Requests', async () => {
        it('posts data to server', async () => {
            const http = new Http()
            http.post(`${baseUrl}/echo`, 'Test data')
            await http.finalize()

            expect(http.status).toBe(200)
            const response = JSON.parse(http.response)
            expect(response.body).toBe('Test data')
            http.close()
        })

        it('posts JSON data', async () => {
            const http = new Http()
            const data = { name: 'test', value: 123 }
            http.contentType = 'application/json'
            http.post(`${baseUrl}/json`, JSON.stringify(data))
            await http.finalize()

            expect(http.status).toBe(200)
            const response = JSON.parse(http.response)
            expect(response.received.name).toBe('test')
            expect(response.received.value).toBe(123)
            http.close()
        })

        it('posts form data', async () => {
            const http = new Http()
            http.form(`${baseUrl}/form`, { field1: 'value1', field2: 'value2' })
            await http.finalize()

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

            http.post(`${baseUrl}/echo`, buffer)
            await http.finalize()
            expect(http.status).toBe(200)
            expect(http.response).toContain('Binary data test')
            http.close()
        })
    })

    await describe('PUT Requests', async () => {
        it('sends PUT request', async () => {
            const http = new Http()
            http.put(`${baseUrl}/resource`, 'Updated content')
            await http.finalize()

            expect(http.status).toBe(200)
            expect(http.response).toBe('Resource updated')
            http.close()
        })
    })

    await describe('DELETE Requests', async () => {
        it('sends DELETE request', async () => {
            const http = new Http()
            http.del(`${baseUrl}/resource`)
            await http.finalize()

            expect(http.status).toBe(200)
            expect(http.response).toBe('Resource deleted')
            http.close()
        })
    })

    await describe('HTTP Headers', async () => {
        it('sends custom headers', async () => {
            const http = new Http()
            http.setHeader('X-Custom-Header', 'TestValue')
            http.get(`${baseUrl}/headers`)
            await http.finalize()

            expect(http.status).toBe(200)
            const headers = JSON.parse(http.response)
            expect(headers['x-custom-header']).toBe('TestValue')
            http.close()
        })

        it('reads response headers', async () => {
            const http = new Http()
            http.get(`${baseUrl}/index.html`)
            await http.finalize()

            expect(http.status).toBe(200)
            expect(http.contentType).toContain('text/html')

            const contentType = http.header('content-type')
            expect(contentType).toContain('text/html')
            http.close()
        })

        it('handles case-insensitive header names', async () => {
            const http = new Http()
            http.get(`${baseUrl}/index.html`)
            await http.finalize()

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
            http.get(`${baseUrl}/status/200`)
            await http.finalize()
            expect(http.status).toBe(200)
            http.close()
        })

        it('handles 404 Not Found', async () => {
            const http = new Http()
            http.get(`${baseUrl}/nonexistent`)
            await http.finalize()
            expect(http.status).toBe(404)
            http.close()
        })

        it('handles 201 Created', async () => {
            const http = new Http()
            http.get(`${baseUrl}/status/201`)
            await http.finalize()
            expect(http.status).toBe(201)
            http.close()
        })

        it('handles 400 Bad Request', async () => {
            const http = new Http()
            http.get(`${baseUrl}/status/400`)
            await http.finalize()
            expect(http.status).toBe(400)
            http.close()
        })

        it('handles 500 Internal Server Error', async () => {
            const http = new Http()
            http.get(`${baseUrl}/status/500`)
            await http.finalize()
            expect(http.status).toBe(500)
            http.close()
        })

        it('finalize() returns HTTP status code', async () => {
            const http = new Http()
            http.get(`${baseUrl}/status/200`)
            const status = await http.finalize()
            expect(status).toBe(200)
            expect(http.status).toBe(200)
            http.close()
        })

        it('finalize() returns status for error codes', async () => {
            const http = new Http()
            http.get(`${baseUrl}/nonexistent`)
            const status = await http.finalize()
            expect(status).toBe(404)
            expect(http.status).toBe(404)
            http.close()
        })
    })

    await describe('Redirects', async () => {
        it('follows redirects when enabled', async () => {
            const http = new Http()
            http.followRedirects = true
            http.get(`${baseUrl}/redirect`)
            await http.finalize()

            expect(http.status).toBe(200)
            expect(http.response).toContain('Redirected successfully')
            http.close()
        })

        it('does not follow redirects by default', async () => {
            const http = new Http()
            http.get(`${baseUrl}/redirect`)
            await http.finalize()

            expect(http.status).toBe(302)
            http.close()
        })
    })

    await describe('Cookies', async () => {
        it('receives cookies from server', async () => {
            const http = new Http()
            http.get(`${baseUrl}/cookie`)
            await http.finalize()

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
            http.get(`${baseUrl}/auth`)
            await http.finalize()

            expect(http.status).toBe(200)
            expect(http.response).toBe('Authenticated')
            http.close()
        })

        it('handles authentication failure', async () => {
            const http = new Http()
            http.setCredentials('wrong', 'credentials')
            http.get(`${baseUrl}/auth`)
            await http.finalize()

            expect(http.status).toBe(403)
            http.close()
        })

        it('returns 401 without credentials', async () => {
            const http = new Http()
            http.get(`${baseUrl}/auth`)
            await http.finalize()

            expect(http.status).toBe(401)
            http.close()
        })
    })

    await describe('Connection Management', async () => {
        it('reuses Http object for multiple requests', async () => {
            const http = new Http()

            http.get(`${baseUrl}/index.html`)
            await http.finalize()
            expect(http.status).toBe(200)

            http.get(`${baseUrl}/text`)
            await http.finalize()
            expect(http.status).toBe(200)

            http.get(`${baseUrl}/json`)
            await http.finalize()
            expect(http.status).toBe(200)

            http.close()
        })

        it('sets uri property', async () => {
            const http = new Http()
            http.uri = `${baseUrl}/index.html`
            http.get()
            await http.finalize()

            expect(http.status).toBe(200)
            expect(http.uri?.toString()).toContain('/index.html')
            http.close()
        })

        it('uses Uri object', async () => {
            const uri = new Uri(`${baseUrl}/index.html`)
            const http = new Http(uri)
            http.get()
            await http.finalize()

            expect(http.status).toBe(200)
            http.close()
        })
    })

    await describe('Method Variations', async () => {
        it('uses connectAsync() with GET', async () => {
            const http = new Http()
            const result = http.connect('GET', `${baseUrl}/index.html`)
            await http.wait()

            expect(result).toBe(http)
            expect(http.status).toBe(200)
            http.close()
        })

        it('uses connectAsync() with POST', async () => {
            const http = new Http()
            http.connect('POST', `${baseUrl}/echo`, 'Test data')
            await http.wait()

            expect(http.status).toBe(200)
            const response = JSON.parse(http.response)
            expect(response.body).toBe('Test data')
            http.close()
        })

        it('uses connectAsync() with PUT', async () => {
            const http = new Http()
            http.connect('PUT', `${baseUrl}/resource`, 'Updated')
            await http.wait()

            expect(http.status).toBe(200)
            http.close()
        })

        it('uses connectAsync() with DELETE', async () => {
            const http = new Http()
            http.connect('DELETE', `${baseUrl}/resource`)
            await http.wait()

            expect(http.status).toBe(200)
            http.close()
        })
    })

    await describe('Response Reading', async () => {
        it('reads response as string', async () => {
            const http = new Http()
            http.get(`${baseUrl}/text`)
            await http.finalize()

            const response = http.readString()
            expect(response).toBe('Plain text response')
            http.close()
        })

        it('reads response in chunks with readString()', async () => {
            const http = new Http()
            http.get(`${baseUrl}/text`)
            await http.finalize()

            const chunk1 = http.readString(5)
            const chunk2 = http.readString(5)

            expect(chunk1).toBe('Plain')
            expect(chunk2).toBe(' text')
            http.close()
        })

        it('gets full response property', async () => {
            const http = new Http()
            http.get(`${baseUrl}/text`)
            await http.finalize()

            expect(http.response).toBe('Plain text response')
            expect(http.response.length).toBe(19)
            http.close()
        })

        it('checks available property', async () => {
            const http = new Http()
            http.get(`${baseUrl}/text`)
            await http.finalize()

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
            http.post(`${baseUrl}/echo`, '{"test": true}')
            await http.finalize()

            expect(http.status).toBe(200)
            http.close()
        })

        it('detects JSON content type', async () => {
            const http = new Http()
            http.get(`${baseUrl}/json`)
            await http.finalize()

            expect(http.contentType).toContain('application/json')
            http.close()
        })

        it('detects HTML content type', async () => {
            const http = new Http()
            http.get(`${baseUrl}/index.html`)
            await http.finalize()

            expect(http.contentType).toContain('text/html')
            http.close()
        })
    })

    await describe('Error Handling', async () => {
        it('handles connection to invalid host', async () => {
            const http = new Http()
            http.setLimits({ requestTimeout: 2 })  // 2 second timeout
            try {
                http.get('http://invalid-host-that-does-not-exist.local/')
                await http.finalize()
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
            http.get(`${baseUrl}/index.html`)
            await http.finalize()

            expect(http.isSecure).toBe(false)
            http.close()
        })
    })
})
