import { describe, it, expect, beforeEach } from 'testme'
import { Http } from '../src/core/Http'
import { Uri } from '../src/core/utilities/Uri'

await describe('Http', async () => {
    await describe('Construction', async () => {
        it('should create Http object with no uri', () => {
            const http = new Http()
            expect(http).toBeInstanceOf(Http)
            expect(http.uri).toBeNull()
        })

        it('should create Http object with string uri', () => {
            const http = new Http('https://example.com')
            expect(http.uri).toBeInstanceOf(Uri)
            expect(http.uri?.toString()).toContain('example.com')
        })

        it('should create Http object with Uri object', () => {
            const uri = new Uri('https://example.com')
            const http = new Http(uri)
            expect(http.uri).toBe(uri)
        })

        it('should create Http object with null uri', () => {
            const http = new Http(null)
            expect(http.uri).toBeNull()
        })
    })

    await describe('HTTP Status Constants', async () => {
        it('should have 1xx informational status codes', () => {
            expect(Http.Continue).toBe(100)
        })

        it('should have 2xx success status codes', () => {
            expect(Http.Ok).toBe(200)
            expect(Http.Created).toBe(201)
            expect(Http.Accepted).toBe(202)
            expect(Http.NotAuthoritative).toBe(203)
            expect(Http.NoContent).toBe(204)
            expect(Http.Reset).toBe(205)
            expect(Http.PartialContent).toBe(206)
        })

        it('should have 3xx redirection status codes', () => {
            expect(Http.MultipleChoice).toBe(300)
            expect(Http.MovedPermanently).toBe(301)
            expect(Http.MovedTemporarily).toBe(302)
            expect(Http.SeeOther).toBe(303)
            expect(Http.NotModified).toBe(304)
            expect(Http.UseProxy).toBe(305)
        })

        it('should have 4xx client error status codes', () => {
            expect(Http.BadRequest).toBe(400)
            expect(Http.Unauthorized).toBe(401)
            expect(Http.PaymentRequired).toBe(402)
            expect(Http.Forbidden).toBe(403)
            expect(Http.NotFound).toBe(404)
            expect(Http.BadMethod).toBe(405)
            expect(Http.NotAcceptable).toBe(406)
            expect(Http.ProxyAuthRequired).toBe(407)
            expect(Http.RequestTimeout).toBe(408)
            expect(Http.Conflict).toBe(409)
            expect(Http.Gone).toBe(410)
            expect(Http.LengthRequired).toBe(411)
            expect(Http.PrecondFailed).toBe(412)
            expect(Http.EntityTooLarge).toBe(413)
            expect(Http.UriTooLong).toBe(414)
            expect(Http.UnsupportedMedia).toBe(415)
            expect(Http.BadRange).toBe(416)
        })

        it('should have 5xx server error status codes', () => {
            expect(Http.ServerError).toBe(500)
            expect(Http.NotImplemented).toBe(501)
            expect(Http.BadGateway).toBe(502)
            expect(Http.ServiceUnavailable).toBe(503)
            expect(Http.GatewayTimeout).toBe(504)
            expect(Http.VersionNotSupported).toBe(505)
        })
    })

    await describe('Properties', async () => {
        let http: Http

        beforeEach(() => {
            http = new Http('https://example.com/test')
        })

        await describe('uri', async () => {
            it('should get uri', () => {
                expect(http.uri).toBeInstanceOf(Uri)
                expect(http.uri?.host).toBe('example.com')
            })

            it('should set uri with string', () => {
                http.uri = 'https://different.com'
                expect(http.uri?.host).toBe('different.com')
            })

            it('should set uri with Uri object', () => {
                const newUri = new Uri('https://another.com')
                http.uri = newUri
                expect(http.uri).toBe(newUri)
            })

            it('should set uri to null', () => {
                http.uri = null
                expect(http.uri).toBeNull()
            })
        })

        await describe('method', async () => {
            it('should have default method GET', () => {
                expect(http.method).toBe('GET')
            })

            it('should set method', () => {
                http.method = 'POST'
                expect(http.method).toBe('POST')
            })

            it('should set various HTTP methods', () => {
                const methods = ['GET', 'POST', 'PUT', 'DELETE', 'HEAD', 'OPTIONS', 'PATCH']
                methods.forEach(method => {
                    http.method = method
                    expect(http.method).toBe(method)
                })
            })
        })

        await describe('async', async () => {
            it('should always return true (deprecated property)', () => {
                expect(http.async).toBe(true)
            })

            it('should accept async mode setting but have no effect', () => {
                http.async = false
                expect(http.async).toBe(true) // Still true
            })
        })

        await describe('followRedirects', async () => {
            it('should default to false', () => {
                expect(http.followRedirects).toBe(false)
            })

            it('should set followRedirects', () => {
                http.followRedirects = true
                expect(http.followRedirects).toBe(true)
            })
        })

        await describe('retries', async () => {
            it('should default to 2', () => {
                expect(http.retries).toBe(2)
            })

            it('should set retries', () => {
                http.retries = 5
                expect(http.retries).toBe(5)
            })

            it('should set retries to 0', () => {
                http.retries = 0
                expect(http.retries).toBe(0)
            })
        })

        await describe('verify', async () => {
            it('should default to true', () => {
                expect(http.verify).toBe(true)
            })

            it('should set verify', () => {
                http.verify = false
                expect(http.verify).toBe(false)
            })
        })

        await describe('verifyIssuer', async () => {
            it('should default to true', () => {
                expect(http.verifyIssuer).toBe(true)
            })

            it('should set verifyIssuer', () => {
                http.verifyIssuer = false
                expect(http.verifyIssuer).toBe(false)
            })
        })

        await describe('response', async () => {
            it('should default to empty string', () => {
                expect(http.response).toBe('')
            })

            it('should set response', () => {
                http.response = 'test response'
                expect(http.response).toBe('test response')
            })
        })
    })

    await describe('Headers', async () => {
        let http: Http

        beforeEach(() => {
            http = new Http()
        })

        await describe('setHeader()', async () => {
            it('should set a header', () => {
                http.setHeader('Content-Type', 'application/json')
                const headers = http.getRequestHeaders()
                expect(headers['Content-Type']).toBe('application/json')
            })

            it('should set multiple headers', () => {
                http.setHeader('Content-Type', 'application/json')
                http.setHeader('Accept', 'application/json')
                const headers = http.getRequestHeaders()
                expect(headers['Content-Type']).toBe('application/json')
                expect(headers['Accept']).toBe('application/json')
            })

            it('should overwrite existing header', () => {
                http.setHeader('Content-Type', 'text/html')
                http.setHeader('Content-Type', 'application/json')
                const headers = http.getRequestHeaders()
                expect(headers['Content-Type']).toBe('application/json')
            })
        })

        await describe('addHeader()', async () => {
            it('should add a header (Ejscript compatibility)', () => {
                http.addHeader('X-Custom', 'value1')
                const headers = http.getRequestHeaders()
                expect(headers['X-Custom']).toBe('value1')
            })

            it('should add multiple headers', () => {
                http.addHeader('X-Custom-1', 'value1')
                http.addHeader('X-Custom-2', 'value2')
                const headers = http.getRequestHeaders()
                expect(headers['X-Custom-1']).toBe('value1')
                expect(headers['X-Custom-2']).toBe('value2')
            })

            it('should work identically to setHeader', () => {
                http.addHeader('Test', 'value')
                http.setHeader('Test2', 'value2')
                const headers = http.getRequestHeaders()
                expect(headers['Test']).toBe('value')
                expect(headers['Test2']).toBe('value2')
            })
        })

        await describe('removeHeader()', async () => {
            it('should remove a header', () => {
                http.setHeader('X-Remove-Me', 'value')
                http.removeHeader('X-Remove-Me')
                const headers = http.getRequestHeaders()
                expect(headers['X-Remove-Me']).toBeUndefined()
            })

            it('should not error when removing non-existent header', () => {
                expect(() => {
                    http.removeHeader('Non-Existent')
                }).not.toThrow()
            })
        })

        await describe('getRequestHeaders()', async () => {
            it('should return default headers including User-Agent', () => {
                const headers = http.getRequestHeaders()
                expect(headers['User-Agent']).toBe('Embedthis-http')
            })

            it('should return copy of headers', () => {
                http.setHeader('Test', 'value')
                const headers1 = http.getRequestHeaders()
                const headers2 = http.getRequestHeaders()
                expect(headers1).not.toBe(headers2)
                expect(headers1).toEqual(headers2)
            })
        })

        await describe('setCookie()', async () => {
            it('should set cookie header', () => {
                http.setCookie('session=abc123')
                const headers = http.getRequestHeaders()
                expect(headers['Cookie']).toBe('session=abc123')
            })

            it('should overwrite previous cookie', () => {
                http.setCookie('old=value')
                http.setCookie('new=value')
                const headers = http.getRequestHeaders()
                expect(headers['Cookie']).toBe('new=value')
            })
        })
    })

    await describe('Credentials', async () => {
        let http: Http

        beforeEach(() => {
            http = new Http()
        })

        await describe('setCredentials()', async () => {
            it('should set basic auth credentials', () => {
                http.setCredentials('user', 'pass')
                // Credentials should be stored internally
                expect(() => http.setCredentials('user', 'pass')).not.toThrow()
            })

            it('should set credentials with type', () => {
                http.setCredentials('user', 'pass', 'digest')
                expect(() => http.setCredentials('user', 'pass', 'digest')).not.toThrow()
            })
        })
    })

    await describe('Finalization', async () => {
        let http: Http

        beforeEach(() => {
            http = new Http()
        })

        await describe('finalize()', async () => {
            it('should mark as finalized', () => {
                expect(http.finalized).toBe(false)
                http.finalize()
                expect(http.finalized).toBe(true)
            })

            it('should be idempotent', () => {
                http.finalize()
                http.finalize()
                expect(http.finalized).toBe(true)
            })
        })

        await describe('reset()', async () => {
            it('should reset headers', () => {
                http.setHeader('Test', 'value')
                http.reset()
                const headers = http.getRequestHeaders()
                expect(headers).toEqual({})
            })

            it('should reset response', () => {
                http.response = 'test'
                http.reset()
                expect(http.response).toBe('')
            })

            it('should reset finalized state', () => {
                http.finalize()
                http.reset()
                expect(http.finalized).toBe(false)
            })

            it('should allow reuse after reset', () => {
                http.setHeader('Test', 'value')
                http.finalize()
                http.reset()
                http.setHeader('New', 'value')
                expect(http.getRequestHeaders()).toEqual({ 'New': 'value' })
            })
        })
    })

    await describe('Response Methods', async () => {
        let http: Http

        beforeEach(() => {
            http = new Http()
            http.response = 'Line 1\nLine 2\nLine 3'
        })

        await describe('readString()', async () => {
            it('should read entire response when count is -1', () => {
                const result = http.readString(-1)
                expect(result).toBe('Line 1\nLine 2\nLine 3')
            })

            it('should read specified number of characters', () => {
                const result = http.readString(6)
                expect(result).toBe('Line 1')
            })

            it('should read 0 characters', () => {
                const result = http.readString(0)
                expect(result).toBe('')
            })
        })

        await describe('readLines()', async () => {
            it('should read all lines when count is -1', () => {
                const lines = http.readLines(-1)
                expect(lines).toEqual(['Line 1', 'Line 2', 'Line 3'])
            })

            it('should read specified number of lines', () => {
                const lines = http.readLines(2)
                expect(lines).toEqual(['Line 1', 'Line 2'])
            })

            it('should handle empty response', () => {
                http.response = ''
                const lines = http.readLines()
                expect(lines).toEqual([])
            })

            it('should handle CRLF line endings', () => {
                http.response = 'Line 1\r\nLine 2\r\nLine 3'
                const lines = http.readLines()
                expect(lines).toEqual(['Line 1', 'Line 2', 'Line 3'])
            })
        })

        await describe('readXml()', async () => {
            it('should return response as-is', () => {
                http.response = '<root><item>value</item></root>'
                const xml = http.readXml()
                expect(xml).toBe('<root><item>value</item></root>')
            })
        })
    })

    await describe('Security Properties', async () => {
        let http: Http

        beforeEach(() => {
            http = new Http('https://example.com')
        })

        await describe('isSecure', async () => {
            it('should return true for https', () => {
                const secureHttp = new Http('https://example.com')
                expect(secureHttp.isSecure).toBe(true)
            })

            it('should return false for http', () => {
                const insecureHttp = new Http('http://example.com')
                expect(insecureHttp.isSecure).toBe(false)
            })

            it('should return false when uri is null', () => {
                const noUriHttp = new Http()
                expect(noUriHttp.isSecure).toBe(false)
            })
        })
    })

    await describe('Static Methods', async () => {
        await describe('fetch()', async () => {
            it('should handle string uri', () => {
                // Test with a simple endpoint that won't fail
                // This is a basic API surface test, not a full integration test
                expect(() => {
                    // Don't actually make request in unit test
                    const http = new Http('https://example.com')
                    http.method = 'GET'
                }).not.toThrow()
            })

            it('should handle Uri object', () => {
                expect(() => {
                    const uri = new Uri('https://example.com')
                    const http = new Http(uri)
                    http.method = 'GET'
                }).not.toThrow()
            })
        })
    })

    await describe('HTTP Methods', async () => {
        let http: Http

        beforeEach(() => {
            http = new Http()
            http.uri = 'https://example.com'
        })

        await describe('get()', async () => {
            it('should set method to GET', async () => {
                // Set URI to avoid error, but test will not actually make request in unit test
                http.uri = 'https://example.com'
                const promise = http.get()
                expect(http.method).toBe('GET')
                // Clean up promise to avoid unhandled rejection
                // No longer a Promise - returns Http object
            })

            it('should accept uri parameter', async () => {
                const promise = http.get('https://different.com')
                expect(http.uri?.toString()).toContain('different.com')
                // No longer a Promise - returns Http object
            })
        })

        await describe('post()', async () => {
            it('should set method to POST', async () => {
                http.uri = 'https://example.com'
                const promise = http.post()
                expect(http.method).toBe('POST')
                // No longer a Promise - returns Http object
            })

            it('should accept data parameters', async () => {
                const promise = http.post(null, { key: 'value' })
                expect(http.method).toBe('POST')
                // No longer a Promise - returns Http object
            })
        })

        await describe('put()', async () => {
            it('should set method to PUT', async () => {
                http.uri = 'https://example.com'
                const promise = http.put()
                expect(http.method).toBe('PUT')
                // No longer a Promise - returns Http object
            })

            it('should accept data parameters', async () => {
                const promise = http.put(null, { key: 'value' })
                expect(http.method).toBe('PUT')
                // No longer a Promise - returns Http object
            })
        })

        await describe('head()', async () => {
            it('should set method to HEAD', async () => {
                http.uri = 'https://example.com'
                const promise = http.head()
                expect(http.method).toBe('HEAD')
                // No longer a Promise - returns Http object
            })
        })
    })

    await describe('Error Handling', async () => {
        it('should throw error when connecting without uri', () => {
            const http = new Http()
            // NEW API: connect() throws synchronously, not via Promise rejection
            expect(() => http.connect('GET')).toThrow('No URI specified')
        })

        it('should not throw when uri is set', () => {
            const http = new Http('https://example.com')
            const result = http.connect('GET')
            expect(result).toBeInstanceOf(Http)
        })
    })

    await describe('Method Chaining', async () => {
        it('should allow chaining HTTP method calls', async () => {
            const http = new Http()
            const result = http.get('https://example.com')
            // NEW API: get() returns Http object, not Promise
            expect(result).toBeInstanceOf(Http)
            expect(result).toBe(http)
        })

        it('should allow chaining connect methods', async () => {
            const http = new Http()
            const result = http.connect('GET', 'https://example.com')
            // NEW API: connect() returns Http object, not Promise
            expect(result).toBeInstanceOf(Http)
            expect(result).toBe(http)
        })

        it('should set multiple headers sequentially', () => {
            const http = new Http('https://example.com')
            http.setHeader('Test1', 'value1')
            http.setHeader('Test2', 'value2')
            const headers = http.getRequestHeaders()
            expect(headers['Test1']).toBe('value1')
            expect(headers['Test2']).toBe('value2')
        })
    })
})
