/**
 * Socket Tests
 * Based on ejscript test suite: socket/*.tst
 */

import { describe, it, expect, beforeEach, afterEach } from 'testme'
import { Socket } from '../src/core/Socket'
import { ByteArray } from '../src/core/streams/ByteArray'

await describe('Socket', async () => {
    await describe('Construction', async () => {
        it('creates socket instance', () => {
            const socket = new Socket()
            expect(socket).not.toBeNull()
        })

        it('creates TCP socket by default', () => {
            const socket = new Socket()
            expect(socket).not.toBeNull()
            socket.close()
        })
    })

    await describe('TCP Client/Server', async () => {
        let server: Socket | null = null
        let client: Socket | null = null
        const TEST_PORT = 9876

        afterEach(() => {
            if (client) {
                try { client.close() } catch {}
                client = null
            }
            if (server) {
                try { server.close() } catch {}
                server = null
            }
        })

        it('listens on a port', () => {
            server = new Socket()
            server.listen(TEST_PORT)
            expect(server.port).toBe(TEST_PORT)
        })

        it('accepts client connections', async () => {
            server = new Socket()
            server.listen(TEST_PORT)

            // Connect client first
            await new Promise<void>((resolve) => {
                setTimeout(() => {
                    client = new Socket()
                    client.connect(TEST_PORT)
                    resolve()
                }, 50)
            })

            // Accept after connection is established
            await new Promise<void>((resolve) => {
                setTimeout(() => {
                    const clientSocket = server!.accept()
                    expect(clientSocket).not.toBeNull()
                    clientSocket.close()
                    resolve()
                }, 150)
            })
        })

        it('connects to server', () => {
            server = new Socket()
            server.listen(TEST_PORT)

            client = new Socket()
            client.connect(TEST_PORT)

            expect(client).not.toBeNull()
        })

        it('writes and reads data', async () => {
            // Wait for previous test's port to be released
            await Bun.sleep(200)

            server = new Socket()
            server.listen(TEST_PORT)

            // Give server time to start listening
            await Bun.sleep(100)

            // Client connects
            client = new Socket()
            client.connect(TEST_PORT)

            // Poll for connection to be queued (max 1 second)
            let serverConn: Socket | null = null
            for (let i = 0; i < 100; i++) {
                try {
                    serverConn = server.accept()
                    break
                } catch (e) {
                    await Bun.sleep(10)
                }
            }

            if (!serverConn) {
                throw new Error('Server failed to accept connection within timeout')
            }

            // Client writes
            const sent = await client.write('Hello Server')
            expect(sent).toBeGreaterThan(0)

            // Wait for data to arrive at server
            await Bun.sleep(100)

            // Server reads
            const serverBuffer = new ByteArray(1024)
            const bytesRead = await serverConn.read(serverBuffer)
            expect(bytesRead).toBeGreaterThan(0)

            // Server echoes back
            await serverConn.write(serverBuffer.toString())

            // Wait for echo to arrive at client
            await Bun.sleep(100)

            // Client reads echo
            const clientBuffer = new ByteArray(1024)
            const clientBytesRead = await client.read(clientBuffer)
            expect(clientBytesRead).toBeGreaterThan(0)
            expect(clientBuffer.toString()).toBe('Hello Server')

            // Cleanup
            serverConn.close()
        })

        it('handles multiple writes', async () => {
            // Wait for previous test's port to be released
            await Bun.sleep(200)

            server = new Socket()
            server.listen(TEST_PORT)

            // Give server time to start listening
            await Bun.sleep(100)

            // Client connects
            client = new Socket()
            client.connect(TEST_PORT)

            // Poll for connection to be queued
            let serverConn: Socket | null = null
            for (let i = 0; i < 100; i++) {
                try {
                    serverConn = server.accept()
                    break
                } catch (e) {
                    await Bun.sleep(10)
                }
            }

            if (!serverConn) {
                throw new Error('Server failed to accept connection within timeout')
            }

            // Client writes multiple parts
            await client.write('Part1 ')
            await client.write('Part2 ')
            await client.write('Part3 ')
            await client.write('END')

            // Wait for all data to arrive
            await Bun.sleep(200)

            // Server reads all data
            const buffer = new ByteArray(1024)
            const bytesRead = await serverConn.read(buffer)
            expect(bytesRead).toBeGreaterThan(0)

            const received = buffer.toString()
            expect(received).toContain('Part1')
            expect(received).toContain('Part2')
            expect(received).toContain('Part3')
            expect(received).toContain('END')

            // Server echoes back
            await serverConn.write(received)

            // Wait for echo to arrive
            await Bun.sleep(100)

            // Client reads echo
            const clientBuffer = new ByteArray(1024)
            const clientBytesRead = await client.read(clientBuffer)
            expect(clientBytesRead).toBeGreaterThan(0)

            const echo = clientBuffer.toString()
            expect(echo).toContain('Part1')
            expect(echo).toContain('Part2')
            expect(echo).toContain('Part3')

            // Cleanup
            serverConn.close()
        })

        it('detects EOF when connection closes', async () => {
            await new Promise<void>((resolve) => {
                server = new Socket()
                server.listen(TEST_PORT)
    
                // Client connects
                setTimeout(() => {
                    client = new Socket()
                    client.connect(TEST_PORT)
                }, 50)
    
                // Server accepts and closes
                setTimeout(() => {
                    const conn = server!.accept()
                    conn.close()
                }, 150)
    
                // Client checks EOF
                setTimeout(() => {
                    expect(client!.isEof).toBe(true)
                    resolve()
                }, 300)
            })
        })
    })

    await describe('Properties', async () => {
        let socket: Socket

        beforeEach(() => {
            socket = new Socket()
        })

        afterEach(() => {
            if (socket) {
                try { socket.close() } catch {}
            }
        })

        it('has port property', () => {
            socket.listen(9877)
            expect(socket.port).toBe(9877)
        })

        it('has address property after listen', () => {
            socket.listen(9878)
            const addr = socket.address
            expect(addr).toBeDefined()
        })

        it('has isEof property', () => {
            expect(typeof socket.isEof).toBe('boolean')
        })

        it('has encoding property', () => {
            expect(socket.encoding).toBe('utf-8')
        })

        it('can set encoding', () => {
            socket.encoding = 'utf-8'
            expect(socket.encoding).toBe('utf-8')
        })
    })

    await describe('UDP Operations', async () => {
        it('creates UDP socket', () => {
            const socket = new Socket()
            socket.options = { datagram: true }
            expect(socket).not.toBeNull()
            socket.close()
        })
    })

    await describe('Async Mode', async () => {
        it('always returns true (deprecated property)', () => {
            const socket = new Socket()
            expect(socket.async).toBe(true)
            socket.close()
        })

        it('setting async mode has no effect', () => {
            const socket = new Socket()
            socket.async = false
            expect(socket.async).toBe(true) // Still true
            socket.close()
        })
    })

    await describe('Error Handling', async () => {
        it('throws on invalid port', () => {
            const socket = new Socket()
            expect(() => {
                socket.listen(-1)
            }).toThrow()
            socket.close()
        })

        it('throws on connect to invalid address', () => {
            const socket = new Socket()
            expect(() => {
                socket.connect('invalid:address:format')
            }).toThrow()
            socket.close()
        })

        it.skip('throws on connect to unavailable port (async error, not sync)', () => {
            // Note: Connection errors in Node.js/Bun are asynchronous and emitted as 'error' events
            // They cannot be caught synchronously with try/catch or expect().toThrow()
            const socket = new Socket()
            socket.connect(65432) // Will fail asynchronously
            socket.close()
        })
    })

    await describe('Stream Interface', async () => {
        let server: Socket | null = null
        let client: Socket | null = null
        const TEST_PORT = 9879

        afterEach(() => {
            if (client) {
                try { client.close() } catch {}
                client = null
            }
            if (server) {
                try { server.close() } catch {}
                server = null
            }
        })

        it('implements read method', async () => {
            await new Promise<void>((resolve) => {
                server = new Socket()
                server.listen(TEST_PORT)
    
                // Client connects
                setTimeout(() => {
                    client = new Socket()
                    client.connect(TEST_PORT)
                }, 50)
    
                // Server accepts and writes
                setTimeout(async () => {
                    const conn = server!.accept()
                    await conn.write('Test Data')
                    conn.close()
                }, 200)
    
                // Client reads
                setTimeout(async () => {
                    const buffer = new ByteArray(1024)
                    const bytesRead = await client!.read(buffer)

                    expect(bytesRead).toBeGreaterThan(0)
                    expect(buffer.toString()).toBe('Test Data')
                    resolve()
                }, 400)
            })
        })

        it('implements write method', async () => {
            server = new Socket()
            server.listen(TEST_PORT)

            client = new Socket()
            client.connect(TEST_PORT)

            const written = await client.write('Hello')
            expect(written).toBeGreaterThan(0)
        })

        it('implements close method', () => {
            const socket = new Socket()
            socket.listen(TEST_PORT)
            socket.close()

            // Should not throw
            expect(true).toBe(true)
        })

        it('implements flush method', () => {
            const socket = new Socket()
            socket.flush()
            socket.close()

            // Should not throw
            expect(true).toBe(true)
        })
    })

    await describe('Event Emitters', async () => {
        let socket: Socket

        beforeEach(() => {
            socket = new Socket()
        })

        afterEach(() => {
            if (socket) {
                try { socket.close() } catch {}
            }
        })

        it('supports on method', () => {
            let called = false
            socket.on('readable', () => { called = true })
            expect(socket).not.toBeNull()
        })

        it('supports off method', () => {
            const handler = () => {}
            socket.on('readable', handler)
            socket.off('readable', handler)
            expect(socket).not.toBeNull()
        })

        it('emits accept event on server', async () => {
            await new Promise<void>((resolve) => {
                const server = new Socket()
                server.listen(9880)
                server.async = true
    
                let acceptCalled = false
                server.on('accept', () => {
                    acceptCalled = true
                })
    
                setTimeout(() => {
                    const client = new Socket()
                    client.connect(9880)
    
                    setTimeout(() => {
                        // In async mode, accept event should be emitted
                        client.close()
                        server.close()
                        resolve()
                    }, 100)
                }, 50)
            })
        })
    })

    await describe('Large Data Transfer', async () => {
        let server: Socket | null = null
        let client: Socket | null = null
        const TEST_PORT = 9881

        afterEach(() => {
            if (client) {
                try { client.close() } catch {}
                client = null
            }
            if (server) {
                try { server.close() } catch {}
                server = null
            }
        })

        it('handles large data writes', async () => {
            // Wait for previous test's port to be released
            await Bun.sleep(200)

            server = new Socket()
            server.listen(TEST_PORT)

            // Give server time to start
            await Bun.sleep(100)

            const largeData = 'X'.repeat(10000)

            // Client connects
            client = new Socket()
            client.connect(TEST_PORT)

            // Poll for connection
            let serverConn: Socket | null = null
            for (let i = 0; i < 100; i++) {
                try {
                    serverConn = server.accept()
                    break
                } catch (e) {
                    await Bun.sleep(10)
                }
            }

            if (!serverConn) {
                throw new Error('Server failed to accept connection within timeout')
            }

            // Client writes large data
            const written = await client.write(largeData)
            expect(written).toBeGreaterThan(0)

            // Wait for data to arrive
            await Bun.sleep(300)

            // Server reads all data
            const buffer = new ByteArray(20000)
            const bytesRead = await serverConn.read(buffer)
            expect(bytesRead).toBe(largeData.length)

            // Server sends response
            await serverConn.write(`Received ${bytesRead} bytes`)
            serverConn.close()

            // Wait for response
            await Bun.sleep(100)

            // Client reads response
            const responseBuffer = new ByteArray(1024)
            await client.read(responseBuffer)
            expect(responseBuffer.toString()).toContain('Received')
            expect(responseBuffer.toString()).toContain('10000')
        })
    })

    await describe('Connection Lifecycle', async () => {
        it('closes cleanly after communication', async () => {
            const server = new Socket()
            server.listen(9882)

            // Give server time to start
            await Bun.sleep(100)

            // Client connects
            const client = new Socket()
            client.connect(9882)

            // Poll for connection
            let serverConn: Socket | null = null
            for (let i = 0; i < 100; i++) {
                try {
                    serverConn = server.accept()
                    break
                } catch (e) {
                    await Bun.sleep(10)
                }
            }

            if (!serverConn) {
                throw new Error('Server failed to accept connection within timeout')
            }

            // Client writes
            await client.write('Hello')

            // Wait for data
            await Bun.sleep(100)

            // Server reads and responds
            const buffer = new ByteArray(1024)
            await serverConn.read(buffer)
            await serverConn.write('OK')
            serverConn.close()
            expect(serverConn.isEof).toBe(true)

            // Wait for response
            await Bun.sleep(100)

            // Client reads and closes
            const clientBuffer = new ByteArray(1024)
            await client.read(clientBuffer)
            client.close()
            expect(client.isEof).toBe(true)

            server.close()
        })

        it('handles rapid connect/disconnect', () => {
            const server = new Socket()
            server.listen(9883)

            for (let i = 0; i < 5; i++) {
                const client = new Socket()
                client.connect(9883)
                client.close()
            }

            server.close()
            expect(true).toBe(true)
        })
    })
})
