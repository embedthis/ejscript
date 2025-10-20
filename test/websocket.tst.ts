import { describe, it, expect, beforeEach, afterEach } from 'testme'
import { WebSocket } from '../src/core/WebSocket'

await describe('WebSocket', async () => {
    let ws: WebSocket | null = null

    afterEach(() => {
        if (ws) {
            try {
                ws.close()
            } catch {}
            ws = null
        }
    })

    await describe('Construction', async () => {
        it('creates a WebSocket with URL', () => {
            ws = new WebSocket('ws://localhost:8080')
            expect(ws).not.toBeNull()
            expect(ws.url).toBe('ws://localhost:8080')
        })

        it('supports wss:// protocol', () => {
            ws = new WebSocket('wss://secure.example.com/ws')
            expect(ws.url).toBe('wss://secure.example.com/ws')
        })

        it('has initial readyState of CONNECTING', () => {
            ws = new WebSocket('ws://localhost:8080')
            // Before connect(), readyState should be 0 (not connected yet)
            expect(ws.readyState).toBe(0)
        })
    })

    await describe('Properties', async () => {
        beforeEach(() => {
            ws = new WebSocket('ws://localhost:8080')
        })

        it('has url property', () => {
            expect(ws!.url).toBe('ws://localhost:8080')
        })

        it('has readyState property', () => {
            expect(typeof ws!.readyState).toBe('number')
            expect(ws!.readyState).toBeGreaterThanOrEqual(0)
            expect(ws!.readyState).toBeLessThanOrEqual(3)
        })

        it('has bufferedAmount property', () => {
            expect(typeof ws!.bufferedAmount).toBe('number')
            expect(ws!.bufferedAmount).toBeGreaterThanOrEqual(0)
        })

        it('has protocol property', () => {
            expect(typeof ws!.protocol).toBe('string')
        })

        it('has extensions property', () => {
            expect(typeof ws!.extensions).toBe('string')
        })

        it('has binaryType property', () => {
            expect(ws!.binaryType).toBe('blob')
        })

        it('can set binaryType', () => {
            ws!.binaryType = 'arraybuffer'
            expect(ws!.binaryType).toBe('arraybuffer')

            ws!.binaryType = 'blob'
            expect(ws!.binaryType).toBe('blob')
        })
    })

    await describe('Ready State Constants', async () => {
        it('defines CONNECTING constant', () => {
            expect(WebSocket.CONNECTING).toBe(0)
        })

        it('defines OPEN constant', () => {
            expect(WebSocket.OPEN).toBe(1)
        })

        it('defines CLOSING constant', () => {
            expect(WebSocket.CLOSING).toBe(2)
        })

        it('defines CLOSED constant', () => {
            expect(WebSocket.CLOSED).toBe(3)
        })
    })

    await describe('Error Handling', async () => {
        beforeEach(() => {
            ws = new WebSocket('ws://localhost:8080')
        })

        it('throws error when sending without connection', () => {
            expect(() => {
                ws!.send('test message')
            }).toThrow('WebSocket not connected')
        })

        it('does not throw when closing without connection', () => {
            expect(() => {
                ws!.close()
            }).not.toThrow()
        })
    })

    await describe('Event Emitters', async () => {
        it('extends Emitter', () => {
            ws = new WebSocket('ws://localhost:8080')
            expect(typeof ws.on).toBe('function')
            expect(typeof ws.off).toBe('function')
            expect(typeof ws.emit).toBe('function')
        })

        it('can register event listeners', () => {
            ws = new WebSocket('ws://localhost:8080')
            let called = false

            ws.on('open', () => {
                called = true
            })

            expect(called).toBe(false) // Not called yet
        })

        it('supports multiple event types', () => {
            ws = new WebSocket('ws://localhost:8080')

            ws.on('open', () => {})
            ws.on('message', () => {})
            ws.on('error', () => {})
            ws.on('close', () => {})

            // Should not throw
            expect(ws).not.toBeNull()
        })
    })

    await describe('Methods', async () => {
        beforeEach(() => {
            ws = new WebSocket('ws://localhost:8080')
        })

        it('has connect method', () => {
            expect(typeof ws!.connect).toBe('function')
        })

        it('has send method', () => {
            expect(typeof ws!.send).toBe('function')
        })

        it('has sendBlock method', () => {
            expect(typeof ws!.sendBlock).toBe('function')
        })

        it('has close method', () => {
            expect(typeof ws!.close).toBe('function')
        })

        it('has wait method', () => {
            expect(typeof ws!.wait).toBe('function')
        })
    })

    await describe('Binary Data', async () => {
        beforeEach(() => {
            ws = new WebSocket('ws://localhost:8080')
        })

        it('supports blob binary type', () => {
            ws!.binaryType = 'blob'
            expect(ws!.binaryType).toBe('blob')
        })

        it('supports arraybuffer binary type', () => {
            ws!.binaryType = 'arraybuffer'
            expect(ws!.binaryType).toBe('arraybuffer')
        })

        it('has sendBlock for binary data', () => {
            // sendBlock should exist and accept ArrayBuffer/Uint8Array
            const buffer = new Uint8Array([1, 2, 3, 4])
            expect(() => {
                // Will throw because not connected, but tests signature exists
                try {
                    ws!.sendBlock(buffer)
                } catch (e: any) {
                    expect(e.message).toContain('not connected')
                }
            }).not.toThrow()
        })
    })

    await describe('Wait Method', async () => {
        beforeEach(() => {
            ws = new WebSocket('ws://localhost:8080')
        })

        it('wait returns promise', () => {
            const result = ws!.wait(WebSocket.OPEN, 100)
            expect(result instanceof Promise).toBe(true)
        })

        it('wait resolves immediately if already in target state', async () => {
            // Mock readyState
            Object.defineProperty(ws!, 'readyState', {
                get: () => WebSocket.CLOSED,
                configurable: true
            })

            const result = await ws!.wait(WebSocket.CLOSED, 100)
            expect(result).toBe(true)
        })

        it('wait times out if state not reached', async () => {
            // readyState stays at CONNECTING (0)
            const result = await ws!.wait(WebSocket.OPEN, 100)
            expect(result).toBe(false)
        })
    })

    await describe('URL Handling', async () => {
        it('preserves ws:// URLs', () => {
            ws = new WebSocket('ws://example.com:9000/path')
            expect(ws.url).toBe('ws://example.com:9000/path')
        })

        it('preserves wss:// URLs', () => {
            ws = new WebSocket('wss://secure.example.com:443/ws')
            expect(ws.url).toBe('wss://secure.example.com:443/ws')
        })

        it('handles URLs with query parameters', () => {
            ws = new WebSocket('ws://localhost:8080/ws?token=abc123')
            expect(ws.url).toBe('ws://localhost:8080/ws?token=abc123')
        })

        it('handles URLs with hash', () => {
            ws = new WebSocket('ws://localhost:8080/ws#channel')
            expect(ws.url).toBe('ws://localhost:8080/ws#channel')
        })
    })

    await describe('Close', async () => {
        beforeEach(() => {
            ws = new WebSocket('ws://localhost:8080')
        })

        it('closes without arguments', () => {
            expect(() => {
                ws!.close()
            }).not.toThrow()
        })

        it('closes with code', () => {
            expect(() => {
                ws!.close(1000)
            }).not.toThrow()
        })

        it('closes with code and reason', () => {
            expect(() => {
                ws!.close(1000, 'Normal closure')
            }).not.toThrow()
        })
    })

    await describe('Integration Patterns', async () => {
        it('typical usage pattern does not throw', () => {
            ws = new WebSocket('ws://localhost:8080/ws')

            ws.on('open', (event) => {
                // Would send data here
            })

            ws.on('message', (data) => {
                // Would process message here
            })

            ws.on('error', (error) => {
                // Would handle error here
            })

            ws.on('close', (event) => {
                // Would handle close here
            })

            // Setup doesn't throw
            expect(ws).not.toBeNull()
        })

        it('can set binary type before connect', () => {
            ws = new WebSocket('ws://localhost:8080')
            ws.binaryType = 'arraybuffer'
            expect(ws.binaryType).toBe('arraybuffer')
        })

        it('maintains properties across multiple operations', () => {
            ws = new WebSocket('ws://localhost:8080/test')
            expect(ws.url).toBe('ws://localhost:8080/test')

            ws.binaryType = 'arraybuffer'
            expect(ws.url).toBe('ws://localhost:8080/test')
            expect(ws.binaryType).toBe('arraybuffer')

            ws.on('open', () => {})
            expect(ws.url).toBe('ws://localhost:8080/test')
            expect(ws.binaryType).toBe('arraybuffer')
        })
    })
})
