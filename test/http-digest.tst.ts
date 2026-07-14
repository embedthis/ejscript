import { describe, it, expect } from '@embedthis/testme'
import { Http } from '../src/core/Http'

await describe('Http Digest Authentication', async () => {
    await describe('Challenge Parsing', async () => {
        it('should parse valid digest challenge with all fields', () => {
            const http = new Http()
            const challenge = 'Digest realm="test", nonce="abc123", algorithm=SHA-256, qop="auth", opaque="xyz789"'

            // Access private method for testing
            const parsed = (http as any)._parseDigestChallenge(challenge)

            expect(parsed).not.toBeNull()
            expect(parsed.realm).toBe('test')
            expect(parsed.nonce).toBe('abc123')
            expect(parsed.algorithm).toBe('SHA-256')
            expect(parsed.qop).toBe('auth')
            expect(parsed.opaque).toBe('xyz789')
        })

        it('should parse digest challenge with minimal fields', () => {
            const http = new Http()
            const challenge = 'Digest realm="test", nonce="abc123"'

            const parsed = (http as any)._parseDigestChallenge(challenge)

            expect(parsed).not.toBeNull()
            expect(parsed.realm).toBe('test')
            expect(parsed.nonce).toBe('abc123')
            expect(parsed.algorithm).toBe('MD5')  // Default
            expect(parsed.qop).toBeUndefined()
            expect(parsed.opaque).toBeUndefined()
        })

        it('should handle unquoted values', () => {
            const http = new Http()
            const challenge = 'Digest realm=test, nonce=abc123, algorithm=MD5'

            const parsed = (http as any)._parseDigestChallenge(challenge)

            expect(parsed).not.toBeNull()
            expect(parsed.realm).toBe('test')
            expect(parsed.nonce).toBe('abc123')
            expect(parsed.algorithm).toBe('MD5')
        })

        it('should return null for non-Digest header', () => {
            const http = new Http()
            const challenge = 'Basic realm="test"'

            const parsed = (http as any)._parseDigestChallenge(challenge)

            expect(parsed).toBeNull()
        })

        it('should return null when realm is missing', () => {
            const http = new Http()
            const challenge = 'Digest nonce="abc123"'

            const parsed = (http as any)._parseDigestChallenge(challenge)

            expect(parsed).toBeNull()
        })

        it('should return null when nonce is missing', () => {
            const http = new Http()
            const challenge = 'Digest realm="test"'

            const parsed = (http as any)._parseDigestChallenge(challenge)

            expect(parsed).toBeNull()
        })

        it('should parse RFC 2617 example challenge', () => {
            const http = new Http()
            const challenge = 'Digest realm="testrealm@host.com", qop="auth,auth-int", nonce="dcd98b7102dd2f0e8b11d0f600bfb0c093", opaque="5ccc069c403ebaf9f0171e9517f40e41"'

            const parsed = (http as any)._parseDigestChallenge(challenge)

            expect(parsed).not.toBeNull()
            expect(parsed.realm).toBe('testrealm@host.com')
            expect(parsed.nonce).toBe('dcd98b7102dd2f0e8b11d0f600bfb0c093')
            expect(parsed.qop).toBe('auth,auth-int')
            expect(parsed.opaque).toBe('5ccc069c403ebaf9f0171e9517f40e41')
        })
    })

    await describe('Hash Computation', async () => {
        it('should compute MD5 hash correctly', () => {
            const http = new Http()
            const result = (http as any)._digestHash('MD5', 'test')

            expect(result).toBe('098f6bcd4621d373cade4e832627b4f6')
        })

        it('should compute SHA-256 hash correctly', () => {
            const http = new Http()
            const result = (http as any)._digestHash('SHA-256', 'test')

            expect(result).toBe('9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a08')
        })

        it('should handle case-insensitive algorithm names', () => {
            const http = new Http()
            const result1 = (http as any)._digestHash('md5', 'test')
            const result2 = (http as any)._digestHash('MD5', 'test')
            const result3 = (http as any)._digestHash('Md5', 'test')

            expect(result1).toBe(result2)
            expect(result2).toBe(result3)
        })

        it('should compute hash with special characters', () => {
            const http = new Http()
            const result = (http as any)._digestHash('MD5', 'user:realm:pass')

            expect(result).toBe('8493fbc53ba582fb4c044c456bdc40eb')
        })

        it('should compute hash with empty string', () => {
            const http = new Http()
            const result = (http as any)._digestHash('MD5', '')

            expect(result).toBe('d41d8cd98f00b204e9800998ecf8427e')
        })

        it('should default to MD5 for unknown algorithm', () => {
            const http = new Http()
            const result = (http as any)._digestHash('UNKNOWN', 'test')

            expect(result).toBe('098f6bcd4621d373cade4e832627b4f6')  // MD5 hash
        })
    })

    await describe('Cnonce Generation', async () => {
        it('should generate cnonce', () => {
            const http = new Http()
            const cnonce = (http as any)._generateCnonce()

            expect(cnonce).toBeTruthy()
            expect(typeof cnonce).toBe('string')
            expect(cnonce.length).toBeGreaterThan(0)
        })

        it('should generate unique cnonce values', () => {
            const http = new Http()
            const cnonce1 = (http as any)._generateCnonce()
            const cnonce2 = (http as any)._generateCnonce()

            expect(cnonce1).not.toBe(cnonce2)
        })

        it('should generate base64-encoded cnonce', () => {
            const http = new Http()
            const cnonce = (http as any)._generateCnonce()

            // Base64 pattern
            expect(cnonce).toMatch(/^[A-Za-z0-9+/]+=*$/)
        })
    })

    await describe('Digest Response Computation', async () => {
        it('should compute digest response without qop (RFC 2617)', () => {
            const http = new Http()

            // Set up digest auth state
            ;(http as any)._digestAuth = {
                username: 'Mufasa',
                password: 'Circle Of Life',
                realm: 'testrealm@host.com',
                nonce: 'dcd98b7102dd2f0e8b11d0f600bfb0c093',
                algorithm: 'MD5',
                nc: 1
            }

            const response = (http as any)._computeDigestResponse('GET', '/dir/index.html')

            expect(response).toBeTruthy()
            expect(typeof response).toBe('string')
            expect(response.length).toBe(32)  // MD5 hex length
        })

        it('should compute digest response with qop=auth', () => {
            const http = new Http()

            ;(http as any)._digestAuth = {
                username: 'test',
                password: 'test',
                realm: 'test',
                nonce: 'abc123',
                algorithm: 'MD5',
                qop: 'auth',
                nc: 1
            }

            const response = (http as any)._computeDigestResponse('GET', '/test')

            expect(response).toBeTruthy()
            expect(typeof response).toBe('string')
            expect(response.length).toBe(32)  // MD5 hex length
        })

        it('should throw error if digest auth not initialized', () => {
            const http = new Http()

            expect(() => {
                (http as any)._computeDigestResponse('GET', '/test')
            }).toThrow('Digest auth not initialized')
        })

        it('should compute different responses for different methods', () => {
            const http = new Http()

            ;(http as any)._digestAuth = {
                username: 'test',
                password: 'test',
                realm: 'test',
                nonce: 'abc123',
                algorithm: 'MD5',
                nc: 1
            }

            const getResponse = (http as any)._computeDigestResponse('GET', '/test')

            ;(http as any)._digestAuth.nc = 1  // Reset
            const postResponse = (http as any)._computeDigestResponse('POST', '/test')

            expect(getResponse).not.toBe(postResponse)
        })

        it('should compute different responses for different URIs', () => {
            const http = new Http()

            ;(http as any)._digestAuth = {
                username: 'test',
                password: 'test',
                realm: 'test',
                nonce: 'abc123',
                algorithm: 'MD5',
                nc: 1
            }

            const response1 = (http as any)._computeDigestResponse('GET', '/test1')

            ;(http as any)._digestAuth.nc = 1  // Reset
            const response2 = (http as any)._computeDigestResponse('GET', '/test2')

            expect(response1).not.toBe(response2)
        })
    })

    await describe('Authorization Header Building', async () => {
        it('should build digest auth header without qop', () => {
            const http = new Http()

            ;(http as any)._digestAuth = {
                username: 'test',
                password: 'pass',
                realm: 'testrealm',
                nonce: 'abc123',
                algorithm: 'MD5',
                nc: 1
            }

            const header = (http as any)._buildDigestAuthHeader('GET', '/test')

            expect(header).toContain('Digest username="test"')
            expect(header).toContain('realm="testrealm"')
            expect(header).toContain('nonce="abc123"')
            expect(header).toContain('uri="/test"')
            expect(header).toContain('response="')
            expect(header).not.toContain('algorithm=')  // MD5 is default, omitted
            expect(header).not.toContain('qop=')
            expect(header).not.toContain('nc=')
            expect(header).not.toContain('cnonce=')
        })

        it('should build digest auth header with qop', () => {
            const http = new Http()

            ;(http as any)._digestAuth = {
                username: 'test',
                password: 'pass',
                realm: 'testrealm',
                nonce: 'abc123',
                algorithm: 'MD5',
                qop: 'auth',
                nc: 1
            }

            const header = (http as any)._buildDigestAuthHeader('GET', '/test')

            expect(header).toContain('Digest username="test"')
            expect(header).toContain('qop=auth')
            expect(header).toContain('nc=00000001')
            expect(header).toContain('cnonce="')
        })

        it('should include algorithm if not MD5', () => {
            const http = new Http()

            ;(http as any)._digestAuth = {
                username: 'test',
                password: 'pass',
                realm: 'testrealm',
                nonce: 'abc123',
                algorithm: 'SHA-256',
                nc: 1
            }

            const header = (http as any)._buildDigestAuthHeader('GET', '/test')

            expect(header).toContain('algorithm=SHA-256')
        })

        it('should include opaque if present', () => {
            const http = new Http()

            ;(http as any)._digestAuth = {
                username: 'test',
                password: 'pass',
                realm: 'testrealm',
                nonce: 'abc123',
                algorithm: 'MD5',
                opaque: 'xyz789',
                nc: 1
            }

            const header = (http as any)._buildDigestAuthHeader('GET', '/test')

            expect(header).toContain('opaque="xyz789"')
        })

        it('should format nonce count with leading zeros', () => {
            const http = new Http()

            ;(http as any)._digestAuth = {
                username: 'test',
                password: 'pass',
                realm: 'testrealm',
                nonce: 'abc123',
                algorithm: 'MD5',
                qop: 'auth',
                nc: 15
            }

            const header = (http as any)._buildDigestAuthHeader('GET', '/test')

            expect(header).toContain('nc=0000000f')  // 15 in hex with padding
        })

        it('should throw error if digest auth not initialized', () => {
            const http = new Http()

            expect(() => {
                (http as any)._buildDigestAuthHeader('GET', '/test')
            }).toThrow('Digest auth not initialized')
        })
    })

    await describe('Nonce Reuse', async () => {
        it('should return false when digest auth not initialized', () => {
            const http = new Http()

            const canReuse = (http as any)._canReuseDigestAuth('/test')

            expect(canReuse).toBe(false)
        })

        it('should return true when no lastUri is set', () => {
            const http = new Http()

            ;(http as any)._digestAuth = {
                username: 'test',
                password: 'pass',
                realm: 'test',
                nonce: 'abc123',
                algorithm: 'MD5',
                nc: 1
            }

            const canReuse = (http as any)._canReuseDigestAuth('/test')

            expect(canReuse).toBe(true)
        })

        it('should return true when URI matches lastUri', () => {
            const http = new Http()

            ;(http as any)._digestAuth = {
                username: 'test',
                password: 'pass',
                realm: 'test',
                nonce: 'abc123',
                algorithm: 'MD5',
                nc: 1,
                lastUri: '/test'
            }

            const canReuse = (http as any)._canReuseDigestAuth('/test')

            expect(canReuse).toBe(true)
        })

        it('should return false when URI differs from lastUri', () => {
            const http = new Http()

            ;(http as any)._digestAuth = {
                username: 'test',
                password: 'pass',
                realm: 'test',
                nonce: 'abc123',
                algorithm: 'MD5',
                nc: 1,
                lastUri: '/test1'
            }

            const canReuse = (http as any)._canReuseDigestAuth('/test2')

            expect(canReuse).toBe(false)
        })
    })

    await describe('Credentials Management', async () => {
        it('should clear digest auth when credentials are cleared', () => {
            const http = new Http()

            http.setCredentials('user', 'pass', 'digest')
            ;(http as any)._digestAuth = {
                username: 'user',
                password: 'pass',
                realm: 'test',
                nonce: 'abc123',
                algorithm: 'MD5',
                nc: 1
            }

            http.setCredentials(null, null)

            expect((http as any)._digestAuth).toBeUndefined()
        })

        it('should clear digest auth when setting new credentials', () => {
            const http = new Http()

            http.setCredentials('user1', 'pass1', 'digest')
            ;(http as any)._digestAuth = {
                username: 'user1',
                password: 'pass1',
                realm: 'test',
                nonce: 'abc123',
                algorithm: 'MD5',
                nc: 1
            }

            http.setCredentials('user2', 'pass2', 'digest')

            expect((http as any)._digestAuth).toBeUndefined()
        })

        it('should accept digest type parameter', () => {
            const http = new Http()

            http.setCredentials('user', 'pass', 'digest')

            expect((http as any).credentials.type).toBe('digest')
        })
    })

    await describe('Reset', async () => {
        it('should clear digest auth state on reset', () => {
            const http = new Http()

            ;(http as any)._digestAuth = {
                username: 'test',
                password: 'pass',
                realm: 'test',
                nonce: 'abc123',
                algorithm: 'MD5',
                nc: 1
            }

            http.reset()

            expect((http as any)._digestAuth).toBeUndefined()
        })
    })
})
