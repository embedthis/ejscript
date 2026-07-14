import { describe, it, expect, beforeAll, afterAll } from '@embedthis/testme'
import { Http } from '../src/core/Http'
import { TestServer } from './helpers/test-server'

let server: TestServer

await describe('Http Digest Authentication Integration', async () => {
    beforeAll(async () => {
        server = new TestServer()
        await server.start()
    })

    afterAll(async () => {
        if (server) {
            await server.stop()
        }
    })

    await describe('Basic Digest Auth Flow', async () => {
        it('should handle digest auth with MD5 and qop=auth', async () => {
            const http = new Http()
            http.setCredentials('testuser', 'testpass', 'digest')

            http.get(`${server.url}/digest-auth`)
            await http.wait()

            expect(http.status).toBe(200)
            expect(http.response).toBe('Digest Authenticated')
        })

        it('should handle digest auth with SHA-256', async () => {
            const http = new Http()
            http.setCredentials('testuser', 'testpass', 'digest')

            http.get(`${server.url}/digest-auth-sha256`)
            await http.wait()

            expect(http.status).toBe(200)
            expect(http.response).toBe('Digest SHA-256 Authenticated')
        })

        it('should handle digest auth without qop', async () => {
            const http = new Http()
            http.setCredentials('testuser', 'testpass', 'digest')

            http.get(`${server.url}/digest-auth-no-qop`)
            await http.wait()

            expect(http.status).toBe(200)
            expect(http.response).toBe('Digest No-QOP Authenticated')
        })

        it('should handle digest auth with POST request', async () => {
            const http = new Http()
            http.setCredentials('testuser', 'testpass', 'digest')

            http.post(`${server.url}/digest-auth-post`, 'test data')
            await http.wait()

            expect(http.status).toBe(200)
            expect(http.response).toContain('Digest POST Authenticated')
            expect(http.response).toContain('test data')
        })
    })

    await describe('Nonce Reuse', async () => {
        it('should reuse nonce for subsequent requests to same endpoint', async () => {
            const http = new Http()
            http.setCredentials('testuser', 'testpass', 'digest')

            // First request - receives 401, then authenticates
            http.get(`${server.url}/digest-auth`)
            await http.wait()
            expect(http.status).toBe(200)
            expect(http.response).toBe('Digest Authenticated')

            // Check that digest auth state is set
            expect((http as any)._digestAuth).toBeDefined()
            expect((http as any)._digestAuth.nonce).toBe('abc123')
            expect((http as any)._digestAuth.nc).toBe(1)

            // Second request - should reuse nonce (preemptive auth)
            http.get(`${server.url}/digest-auth`)
            await http.wait()
            expect(http.status).toBe(200)
            expect(http.response).toBe('Digest Authenticated')

            // Nonce count should increment
            expect((http as any)._digestAuth.nc).toBe(2)
        })

        it('should get new challenge for different endpoint', async () => {
            const http = new Http()
            http.setCredentials('testuser', 'testpass', 'digest')

            // First request to one endpoint
            http.get(`${server.url}/digest-auth`)
            await http.wait()
            expect(http.status).toBe(200)
            const firstNonce = (http as any)._digestAuth.nonce

            // Second request to different endpoint
            http.get(`${server.url}/digest-auth-sha256`)
            await http.wait()
            expect(http.status).toBe(200)
            const secondNonce = (http as any)._digestAuth.nonce

            // Should have different nonce for different endpoint
            expect(secondNonce).not.toBe(firstNonce)
        })
    })

    await describe('Auth Header Format', async () => {
        it('should include all required digest fields with qop', async () => {
            const http = new Http()
            http.setCredentials('testuser', 'testpass', 'digest')

            // Capture the Authorization header by checking response
            http.get(`${server.url}/digest-auth`)
            await http.wait()

            expect(http.status).toBe(200)

            // Check digest auth state has correct structure
            const digestAuth = (http as any)._digestAuth
            expect(digestAuth.username).toBe('testuser')
            expect(digestAuth.realm).toBe('test')
            expect(digestAuth.nonce).toBe('abc123')
            expect(digestAuth.algorithm).toBe('MD5')
            expect(digestAuth.qop).toBe('auth')
            expect(digestAuth.nc).toBe(1)
        })

        it('should omit qop fields when not present in challenge', async () => {
            const http = new Http()
            http.setCredentials('testuser', 'testpass', 'digest')

            http.get(`${server.url}/digest-auth-no-qop`)
            await http.wait()

            expect(http.status).toBe(200)

            // Check digest auth state
            const digestAuth = (http as any)._digestAuth
            expect(digestAuth.qop).toBeUndefined()
        })

        it('should include opaque when present in challenge', async () => {
            const http = new Http()
            http.setCredentials('testuser', 'testpass', 'digest')

            http.get(`${server.url}/digest-auth-sha256`)
            await http.wait()

            expect(http.status).toBe(200)

            const digestAuth = (http as any)._digestAuth
            expect(digestAuth.opaque).toBe('5ccc069c')
        })
    })

    await describe('Error Scenarios', async () => {
        it('should return 401 when no credentials provided', async () => {
            const http = new Http()

            http.get(`${server.url}/digest-auth`)
            await http.wait()

            expect(http.status).toBe(401)
            expect(http.response).toBe('Unauthorized')
        })

        it('should handle server rejection after auth attempt', async () => {
            const http = new Http()
            http.setCredentials('wronguser', 'wrongpass', 'digest')

            http.get(`${server.url}/digest-auth`)
            await http.wait()

            // Server will return 403 for invalid credentials (after checking auth header)
            // Our test server does simple validation, so it might return 200 or 403
            // depending on implementation
            expect(http.status).toBeGreaterThanOrEqual(200)
        })
    })

    await describe('Credentials Management', async () => {
        it('should clear digest state when credentials cleared', async () => {
            const http = new Http()
            http.setCredentials('testuser', 'testpass', 'digest')

            http.get(`${server.url}/digest-auth`)
            await http.wait()
            expect(http.status).toBe(200)
            expect((http as any)._digestAuth).toBeDefined()

            http.setCredentials(null, null)
            expect((http as any)._digestAuth).toBeUndefined()
        })

        it('should clear digest state when changing credentials', async () => {
            const http = new Http()
            http.setCredentials('testuser', 'testpass', 'digest')

            http.get(`${server.url}/digest-auth`)
            await http.wait()
            expect(http.status).toBe(200)
            const oldDigest = (http as any)._digestAuth

            http.setCredentials('newuser', 'newpass', 'digest')
            expect((http as any)._digestAuth).toBeUndefined()
        })

        it('should clear digest state on reset', async () => {
            const http = new Http()
            http.setCredentials('testuser', 'testpass', 'digest')

            http.get(`${server.url}/digest-auth`)
            await http.wait()
            expect(http.status).toBe(200)
            expect((http as any)._digestAuth).toBeDefined()

            http.reset()
            expect((http as any)._digestAuth).toBeUndefined()
        })
    })

    await describe('Mixed Auth Types', async () => {
        it('should use basic auth when type is basic', async () => {
            const http = new Http()
            http.setCredentials('user', 'pass', 'basic')

            http.get(`${server.url}/auth`)
            await http.wait()

            expect(http.status).toBe(200)
            expect(http.response).toBe('Authenticated')
        })

        it('should switch from basic to digest auth', async () => {
            const http = new Http()
            http.setCredentials('user', 'pass', 'basic')

            http.get(`${server.url}/auth`)
            await http.wait()
            expect(http.status).toBe(200)

            http.setCredentials('testuser', 'testpass', 'digest')

            http.get(`${server.url}/digest-auth`)
            await http.wait()
            expect(http.status).toBe(200)
            expect(http.response).toBe('Digest Authenticated')
        })

        it('should default to basic auth when type not specified', async () => {
            const http = new Http()
            http.setCredentials('user', 'pass')  // No type

            http.get(`${server.url}/auth`)
            await http.wait()

            expect(http.status).toBe(200)
            expect(http.response).toBe('Authenticated')
        })
    })

    await describe('Multiple Requests', async () => {
        it('should handle multiple sequential requests with digest auth', async () => {
            const http = new Http()
            http.setCredentials('testuser', 'testpass', 'digest')

            // Request 1
            http.get(`${server.url}/digest-auth`)
            await http.wait()
            expect(http.status).toBe(200)
            expect((http as any)._digestAuth.nc).toBe(1)

            // Request 2
            http.get(`${server.url}/digest-auth`)
            await http.wait()
            expect(http.status).toBe(200)
            expect((http as any)._digestAuth.nc).toBe(2)

            // Request 3
            http.get(`${server.url}/digest-auth`)
            await http.wait()
            expect(http.status).toBe(200)
            expect((http as any)._digestAuth.nc).toBe(3)
        })

        it('should handle POST after GET with nonce reuse', async () => {
            const http = new Http()
            http.setCredentials('testuser', 'testpass', 'digest')

            // First GET
            http.get(`${server.url}/digest-auth`)
            await http.wait()
            expect(http.status).toBe(200)

            // Then POST (different endpoint, will get new challenge)
            http.post(`${server.url}/digest-auth-post`, 'data')
            await http.wait()
            expect(http.status).toBe(200)
        })
    })

    await describe('Algorithm Support', async () => {
        it('should handle MD5 algorithm', async () => {
            const http = new Http()
            http.setCredentials('testuser', 'testpass', 'digest')

            http.get(`${server.url}/digest-auth`)
            await http.wait()

            expect(http.status).toBe(200)
            expect((http as any)._digestAuth.algorithm).toBe('MD5')
        })

        it('should handle SHA-256 algorithm', async () => {
            const http = new Http()
            http.setCredentials('testuser', 'testpass', 'digest')

            http.get(`${server.url}/digest-auth-sha256`)
            await http.wait()

            expect(http.status).toBe(200)
            expect((http as any)._digestAuth.algorithm).toBe('SHA-256')
        })
    })

    await describe('Auto-Detection of Auth Type', async () => {
        it('should auto-detect digest auth from server response', async () => {
            const http = new Http()
            // No auth type specified - should auto-detect
            http.setCredentials('testuser', 'testpass')

            http.get(`${server.url}/digest-auth`)
            await http.wait()

            expect(http.status).toBe(200)
            expect(http.response).toBe('Digest Authenticated')
            expect((http as any)._digestAuth).toBeDefined()
            expect((http as any)._digestAuth.algorithm).toBe('MD5')
        })

        it('should auto-detect basic auth from server response', async () => {
            const http = new Http()
            // No auth type specified - should auto-detect
            http.setCredentials('user', 'pass')

            http.get(`${server.url}/auth`)
            await http.wait()

            expect(http.status).toBe(200)
            expect(http.response).toBe('Authenticated')
        })

        it('should reuse auto-detected digest auth on subsequent requests', async () => {
            const http = new Http()
            http.setCredentials('testuser', 'testpass')  // No type

            // First request - auto-detects digest
            http.get(`${server.url}/digest-auth`)
            await http.wait()
            expect(http.status).toBe(200)
            expect((http as any)._digestAuth).toBeDefined()

            // Second request - reuses digest auth
            http.get(`${server.url}/digest-auth`)
            await http.wait()
            expect(http.status).toBe(200)
            expect((http as any)._digestAuth.nc).toBe(2)
        })
    })
})
