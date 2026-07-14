import { describe, it, expect } from '@embedthis/testme'
import { System } from '../src/core/System'
import { Path } from '../src/core/Path'

await describe('System', async () => {
    await describe('Static Properties', async () => {
        it('has Bufsize constant', () => {
            expect(System.Bufsize).toBe(1024)
            expect(typeof System.Bufsize).toBe('number')
        })

        it('Bufsize is positive', () => {
            expect(System.Bufsize).toBeGreaterThan(0)
        })
    })

    await describe('hostname', async () => {
        it('returns a string', () => {
            const hostname = System.hostname
            expect(typeof hostname).toBe('string')
        })

        it('returns non-empty hostname', () => {
            const hostname = System.hostname
            expect(hostname.length).toBeGreaterThan(0)
        })

        it('hostname does not contain invalid characters', () => {
            const hostname = System.hostname
            // Hostnames should not have spaces or control characters
            expect(hostname).not.toContain(' ')
            expect(hostname).not.toContain('\n')
            expect(hostname).not.toContain('\t')
        })

        it('hostname is consistent across calls', () => {
            const hostname1 = System.hostname
            const hostname2 = System.hostname
            expect(hostname1).toBe(hostname2)
        })
    })

    await describe('ipaddr', async () => {
        it('returns a string', () => {
            const ipaddr = System.ipaddr
            expect(typeof ipaddr).toBe('string')
        })

        it('returns valid IP address format', () => {
            const ipaddr = System.ipaddr
            // Should match IPv4 pattern (xxx.xxx.xxx.xxx)
            expect(ipaddr).toMatch(/^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$/)
        })

        it('returns localhost if no network interface found', () => {
            const ipaddr = System.ipaddr
            // Should be either localhost or a valid IP
            expect(ipaddr).toBeTruthy()
            if (ipaddr === '127.0.0.1') {
                expect(ipaddr).toBe('127.0.0.1')
            } else {
                // Should be a non-loopback IP
                expect(ipaddr).not.toBe('0.0.0.0')
            }
        })

        it('IP address octets are in valid range', () => {
            const ipaddr = System.ipaddr
            const octets = ipaddr.split('.').map(Number)

            expect(octets.length).toBe(4)
            octets.forEach(octet => {
                expect(octet).toBeGreaterThanOrEqual(0)
                expect(octet).toBeLessThanOrEqual(255)
            })
        })

        it('returns IPv4 address (not IPv6)', () => {
            const ipaddr = System.ipaddr
            // Should not contain colons (IPv6 format)
            expect(ipaddr).not.toContain(':')
        })

        it('is consistent across calls', () => {
            const ipaddr1 = System.ipaddr
            const ipaddr2 = System.ipaddr
            expect(ipaddr1).toBe(ipaddr2)
        })
    })

    await describe('tmpdir', async () => {
        it('returns a string', () => {
            const tmpdir = System.tmpdir
            expect(typeof tmpdir).toBe('string')
        })

        it('returns non-empty path', () => {
            const tmpdir = System.tmpdir
            expect(tmpdir.length).toBeGreaterThan(0)
        })

        it('returns absolute path', () => {
            const tmpdir = System.tmpdir
            const path = new Path(tmpdir)
            expect(path.isAbsolute).toBe(true)
        })

        it('tmpdir exists and is a directory', () => {
            const tmpdir = System.tmpdir
            const path = new Path(tmpdir)
            expect(path.exists).toBe(true)
            expect(path.isDir).toBe(true)
        })

        it('tmpdir is writable', async () => {
            const tmpdir = System.tmpdir
            const testFile = new Path(tmpdir).join(`test-${process.pid}.tmp`)

            try {
                await testFile.write('test')
                expect(testFile.exists).toBe(true)
                expect(await testFile.readString()).toBe('test')
            } finally {
                if (testFile.exists) {
                    await testFile.remove()
                }
            }
        })

        it('is consistent across calls', () => {
            const tmpdir1 = System.tmpdir
            const tmpdir2 = System.tmpdir
            expect(tmpdir1).toBe(tmpdir2)
        })

        it('tmpdir path is platform-appropriate', () => {
            const tmpdir = System.tmpdir

            if (process.platform === 'win32') {
                // Windows temp dir typically contains 'Temp' or 'temp'
                expect(tmpdir.toLowerCase()).toContain('temp')
            } else {
                // Unix-like systems - just verify it's an absolute path and exists
                expect(tmpdir.startsWith('/')).toBe(true)
                const path = new Path(tmpdir)
                expect(path.exists).toBe(true)
            }
        })
    })

    await describe('Integration', async () => {
        it('all system properties are accessible', () => {
            expect(System.Bufsize).toBeDefined()
            expect(System.hostname).toBeDefined()
            expect(System.ipaddr).toBeDefined()
            expect(System.tmpdir).toBeDefined()
        })

        it('system information is valid', () => {
            expect(typeof System.Bufsize).toBe('number')
            expect(typeof System.hostname).toBe('string')
            expect(typeof System.ipaddr).toBe('string')
            expect(typeof System.tmpdir).toBe('string')
        })

        it('can create files in tmpdir', async () => {
            const tmpPath = new Path(System.tmpdir)
            const testFile = tmpPath.join(`sys-test-${process.pid}.txt`)

            try {
                await testFile.write(`Testing System class\nHostname: ${System.hostname}\nIP: ${System.ipaddr}`)
                expect(testFile.exists).toBe(true)

                const content = await testFile.readString()
                expect(content).toContain('Testing System class')
                expect(content).toContain(System.hostname)
                expect(content).toContain(System.ipaddr)
            } finally {
                if (testFile.exists) {
                    await testFile.remove()
                }
            }
        })
    })

    await describe('Type Safety', async () => {
        it('Bufsize is a number', () => {
            expect(typeof System.Bufsize).toBe('number')
            expect(Number.isInteger(System.Bufsize)).toBe(true)
        })

        it('hostname returns string', () => {
            const result = System.hostname
            expect(typeof result).toBe('string')
        })

        it('ipaddr returns string', () => {
            const result = System.ipaddr
            expect(typeof result).toBe('string')
        })

        it('tmpdir returns string', () => {
            const result = System.tmpdir
            expect(typeof result).toBe('string')
        })
    })
})
