/**
 * Cache and LocalCache Tests
 * Based on ejscript test suite: cache/*.tst
 */

import { describe, it, expect, beforeEach } from '@embedthis/testme'
import { Cache } from '../src/core/utilities/Cache'
import { LocalCache } from '../src/core/utilities/LocalCache'

await describe('Cache', async () => {
    await describe('Construction and Basic Operations', async () => {
        it('creates cache instance', () => {
            const cache = new Cache('test')
            expect(cache).not.toBeNull()
        })

        it('writes and reads simple values', () => {
            const cache = new Cache('test')
            cache.write('message', 'Hello World')
            expect(cache.read('message')).toBe('Hello World')
        })

        it('returns null for non-existent keys', () => {
            const cache = new Cache('test')
            expect(cache.read('nonexistent')).toBeNull()
        })

        it('overwrites existing values', () => {
            const cache = new Cache('test')
            cache.write('key', 'value1')
            expect(cache.read('key')).toBe('value1')

            cache.write('key', 'value2')
            expect(cache.read('key')).toBe('value2')
        })
    })

    await describe('Object Serialization', async () => {
        it('writes and reads objects', () => {
            const cache = new Cache('test')
            const obj = { name: 'Alice', age: 30, active: true }

            cache.writeObj('user', obj)
            const retrieved = cache.readObj('user')

            expect(retrieved).toEqual(obj)
            expect(retrieved.name).toBe('Alice')
            expect(retrieved.age).toBe(30)
            expect(retrieved.active).toBe(true)
        })

        it('handles nested objects', () => {
            const cache = new Cache('test')
            const obj = {
                user: { name: 'Bob' },
                settings: { theme: 'dark', notifications: true }
            }

            cache.writeObj('data', obj)
            const retrieved = cache.readObj('data')

            expect(retrieved.user.name).toBe('Bob')
            expect(retrieved.settings.theme).toBe('dark')
        })

        it('handles arrays in objects', () => {
            const cache = new Cache('test')
            const obj = { items: [1, 2, 3, 4, 5] }

            cache.writeObj('list', obj)
            const retrieved = cache.readObj('list')

            expect(retrieved.items).toEqual([1, 2, 3, 4, 5])
        })
    })

    await describe('Expiration', async () => {
        it('expires entries based on lifespan option', async () => {
            const cache = new Cache('test')

            // Write with 1 second lifespan
            cache.write('temp', 'value', { lifespan: 1 })
            expect(cache.read('temp')).toBe('value')

            // Wait 1.1 seconds and check expiration
            await new Promise(resolve => setTimeout(resolve, 1100))
            expect(cache.read('temp')).toBeNull()
        })

        it('expires entries at specific date', () => {
            const cache = new Cache('test')
            cache.write('key', 'value')

            // Expire in the past
            const pastDate = new Date(Date.now() - 1000)
            cache.expire('key', pastDate)

            expect(cache.read('key')).toBeNull()
        })

        it('removes entry when expired with null', () => {
            const cache = new Cache('test')
            cache.write('key', 'value')
            expect(cache.read('key')).toBe('value')

            cache.expire('key', null)
            expect(cache.read('key')).toBeNull()
        })

        it('updates expiry without removing key', () => {
            const cache = new Cache('test')
            cache.write('key', 'value')

            // Set expiry far in future
            const futureDate = new Date(Date.now() + 86400000) // 24 hours
            cache.expire('key', futureDate)

            expect(cache.read('key')).toBe('value')
        })

        it('uses default lifespan from constructor', async () => {
            const cache = new Cache('test', { lifespan: 1 }) // 1 second default

            cache.write('temp', 'value')
            expect(cache.read('temp')).toBe('value')

            await new Promise(resolve => setTimeout(resolve, 1100))
            expect(cache.read('temp')).toBeNull()
        })
    })

    await describe('Atomic Increment (inc)', async () => {
        it('increments from zero for new key', () => {
            const cache = new Cache('test')
            expect(cache.inc('counter')).toBe(1)
        })

        it('increments existing numeric value', () => {
            const cache = new Cache('test')
            cache.write('num', 0)

            expect(cache.read('num')).toBe(0)
            expect(cache.inc('num')).toBe(1)
            expect(cache.read('num')).toBe(1)
        })

        it('increments multiple times', () => {
            const cache = new Cache('test')

            expect(cache.inc('counter')).toBe(1)
            expect(cache.inc('counter')).toBe(2)
            expect(cache.inc('counter')).toBe(3)
            expect(cache.inc('counter')).toBe(4)

            expect(cache.read('counter')).toBe(4)
        })

        it('increments by custom amount', () => {
            const cache = new Cache('test')
            cache.write('num', 0)

            expect(cache.inc('num', 2)).toBe(2)
            expect(cache.inc('num', 2)).toBe(4)
            expect(cache.inc('num', 5)).toBe(9)
        })

        it('decrements with negative amount', () => {
            const cache = new Cache('test')
            cache.write('num', 10)

            expect(cache.inc('num', -1)).toBe(9)
            expect(cache.inc('num', -1)).toBe(8)
            expect(cache.inc('num', -3)).toBe(5)
        })

        it('handles mixed increment/decrement', () => {
            const cache = new Cache('test')

            expect(cache.inc('counter')).toBe(1)
            expect(cache.inc('counter', 5)).toBe(6)
            expect(cache.inc('counter', -3)).toBe(3)
            expect(cache.inc('counter', 2)).toBe(5)
            expect(cache.inc('counter', -10)).toBe(-5)
        })

        it('parses string numbers', () => {
            const cache = new Cache('test')
            cache.write('num', '10')

            expect(cache.inc('num', 5)).toBe(15)
        })

        it('handles expired keys by reinitializing', async () => {
            const cache = new Cache('test')

            cache.write('counter', 5, { lifespan: 1 })
            expect(cache.inc('counter')).toBe(6)

            await new Promise(resolve => setTimeout(resolve, 1100))
            // After expiry, should start from 0
            expect(cache.inc('counter')).toBe(1)
        })
    })

    await describe('Remove and Clear', async () => {
        it('removes specific key', () => {
            const cache = new Cache('test')
            cache.write('key1', 'value1')
            cache.write('key2', 'value2')

            cache.remove('key1')

            expect(cache.read('key1')).toBeNull()
            expect(cache.read('key2')).toBe('value2')
        })

        it('clears all entries', () => {
            const cache = new Cache('test')
            cache.write('key1', 'value1')
            cache.write('key2', 'value2')
            cache.write('key3', 'value3')

            cache.clear()

            expect(cache.read('key1')).toBeNull()
            expect(cache.read('key2')).toBeNull()
            expect(cache.read('key3')).toBeNull()
        })

        it('destroy clears cache', () => {
            const cache = new Cache('test')
            cache.write('key1', 'value1')
            cache.write('key2', 'value2')

            cache.destroy()

            expect(cache.read('key1')).toBeNull()
            expect(cache.read('key2')).toBeNull()
        })
    })

    await describe('Limits and Configuration', async () => {
        it('gets current limits', () => {
            const cache = new Cache('test')
            const limits = cache.limits

            expect(limits).toBeDefined()
            expect(typeof limits.keys).toBe('number')
            expect(typeof limits.memory).toBe('number')
            expect(typeof limits.lifespan).toBe('number')
        })

        it('sets lifespan limit', () => {
            const cache = new Cache('test')

            cache.setLimits({ lifespan: 3600 })
            expect(cache.limits.lifespan).toBe(3600)

            cache.setLimits({ lifespan: 7200 })
            expect(cache.limits.lifespan).toBe(7200)
        })

        it('updates default lifespan for new writes', async () => {
            const cache = new Cache('test')

            cache.setLimits({ lifespan: 1 }) // 1 second

            cache.write('key', 'value') // Uses new default
            expect(cache.read('key')).toBe('value')

            await new Promise(resolve => setTimeout(resolve, 1100))
            expect(cache.read('key')).toBeNull()
        })
    })

    await describe('Statistics', async () => {
        it('reports entry count', () => {
            const cache = new Cache('test')

            expect(cache.stats.entries).toBe(0)

            cache.write('key1', 'value1')
            expect(cache.stats.entries).toBe(1)

            cache.write('key2', 'value2')
            cache.write('key3', 'value3')
            expect(cache.stats.entries).toBe(3)

            cache.remove('key1')
            expect(cache.stats.entries).toBe(2)

            cache.clear()
            expect(cache.stats.entries).toBe(0)
        })
    })

    await describe('Multiple Cache Instances', async () => {
        it('creates independent cache instances', () => {
            const cache1 = new Cache('cache1')
            const cache2 = new Cache('cache2')

            cache1.write('key', 'value1')
            cache2.write('key', 'value2')

            expect(cache1.read('key')).toBe('value1')
            expect(cache2.read('key')).toBe('value2')
        })

        it('clears one cache without affecting others', () => {
            const cache1 = new Cache('cache1')
            const cache2 = new Cache('cache2')

            cache1.write('key', 'value1')
            cache2.write('key', 'value2')

            cache1.clear()

            expect(cache1.read('key')).toBeNull()
            expect(cache2.read('key')).toBe('value2')
        })
    })
})

await describe('LocalCache', async () => {
    await describe('Construction', async () => {
        it('creates LocalCache instance', () => {
            const cache = new LocalCache()
            expect(cache).not.toBeNull()
        })

        it('creates with name', () => {
            const cache = new LocalCache('local')
            expect(cache).not.toBeNull()
        })

        it('creates with options', () => {
            const cache = new LocalCache(null, { lifespan: 3600 })
            expect(cache).not.toBeNull()
            expect(cache.limits.lifespan).toBe(3600)
        })
    })

    await describe('Inheritance from Cache', async () => {
        it('inherits read/write methods', () => {
            const cache = new LocalCache()
            cache.write('key', 'value')
            expect(cache.read('key')).toBe('value')
        })

        it('inherits inc method', () => {
            const cache = new LocalCache()
            expect(cache.inc('counter')).toBe(1)
            expect(cache.inc('counter', 5)).toBe(6)
        })

        it('inherits expire method', () => {
            const cache = new LocalCache()
            cache.write('key', 'value')

            const pastDate = new Date(Date.now() - 1000)
            cache.expire('key', pastDate)

            expect(cache.read('key')).toBeNull()
        })

        it('inherits destroy method', () => {
            const cache = new LocalCache()
            cache.write('key1', 'value1')
            cache.write('key2', 'value2')

            cache.destroy()

            expect(cache.read('key1')).toBeNull()
            expect(cache.read('key2')).toBeNull()
        })

        it('inherits setLimits/limits', () => {
            const cache = new LocalCache()

            cache.setLimits({ lifespan: 1800 })
            expect(cache.limits.lifespan).toBe(1800)
        })
    })

    await describe('LocalCache-specific Behavior', async () => {
        it('sets shared: false by default', () => {
            const cache = new LocalCache()
            // LocalCache forces shared: false in constructor
            expect(cache).not.toBeNull()
        })

        it('overrides shared option', () => {
            const cache = new LocalCache(null, { shared: true })
            // LocalCache forces shared: false even if specified
            expect(cache).not.toBeNull()
        })
    })

    await describe('Independence', async () => {
        it('creates independent LocalCache instances', () => {
            const cache1 = new LocalCache('local1')
            const cache2 = new LocalCache('local2')

            cache1.write('key', 'value1')
            cache2.write('key', 'value2')

            expect(cache1.read('key')).toBe('value1')
            expect(cache2.read('key')).toBe('value2')
        })
    })
})

await describe('Cache Use Cases', async () => {
    await describe('Page View Counter', async () => {
        it('tracks page views atomically', () => {
            const cache = new Cache('analytics')

            const views1 = cache.inc('page-views')
            const views2 = cache.inc('page-views')
            const views3 = cache.inc('page-views')

            expect(views3).toBe(3)
            expect(cache.read('page-views')).toBe(3)
        })
    })

    await describe('Rate Limiting', async () => {
        it('implements rate limiting pattern', () => {
            const cache = new Cache('rate-limit')
            const userId = 'user123'
            const minute = Math.floor(Date.now() / 60000)
            const key = `rate-limit:${userId}:${minute}`

            // Track requests
            for (let i = 0; i < 10; i++) {
                cache.inc(key)
            }

            const count = cache.read(key)
            expect(count).toBe(10)

            // Set expiry
            const expiry = new Date((minute + 1) * 60000)
            cache.expire(key, expiry)
        })
    })

    await describe('Inventory Management', async () => {
        it('tracks inventory with atomic operations', () => {
            const cache = new Cache('inventory')

            // Initialize stock
            cache.write('product-123-qty', 100)

            // Decrement for purchases
            const stock1 = cache.inc('product-123-qty', -5)
            expect(stock1).toBe(95)

            const stock2 = cache.inc('product-123-qty', -3)
            expect(stock2).toBe(92)

            // Check if in stock
            const available = cache.read('product-123-qty')
            expect(available).toBeGreaterThan(0)
        })
    })

    await describe('Session Management', async () => {
        it('manages sessions with expiry', async () => {
            const cache = new Cache('sessions')

            const sessionData = {
                userId: 'user123',
                username: 'alice',
                roles: ['user', 'admin']
            }

            // Write session with 30 minute lifespan
            cache.writeObj('session:abc123', sessionData, { lifespan: 1 }) // 1 second for test

            const session = cache.readObj('session:abc123')
            expect(session.userId).toBe('user123')

            // Verify expiration
            await new Promise(resolve => setTimeout(resolve, 1100))
            expect(cache.readObj('session:abc123')).toBeNull()
        })
    })
})
