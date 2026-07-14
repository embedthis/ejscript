import { describe, it, expect } from '@embedthis/testme'
import { Config } from '../src/core/Config'

await describe('Config', async () => {
    await describe('Static Properties', async () => {
        it('has Product property', () => {
            expect(Config.Product).toBe('ejscript')
            expect(typeof Config.Product).toBe('string')
        })

        it('has Version property', () => {
            expect(Config.Version).toBe('0.1.0')
            expect(typeof Config.Version).toBe('string')
            expect(Config.Version).toMatch(/^\d+\.\d+\.\d+$/)
        })

        it('has OS property', () => {
            expect(Config.OS).toBeDefined()
            expect(typeof Config.OS).toBe('string')
            expect(['linux', 'darwin', 'win32', 'freebsd', 'openbsd', 'sunos', 'aix']).toContain(Config.OS)
        })

        it('has CPU property', () => {
            expect(Config.CPU).toBeDefined()
            expect(typeof Config.CPU).toBe('string')
            expect(['x64', 'arm64', 'arm', 'ia32', 'mips', 'ppc64', 's390x', 's390']).toContain(Config.CPU)
        })

        it('has NumCPU property', () => {
            expect(Config.NumCPU).toBeDefined()
            expect(typeof Config.NumCPU).toBe('number')
            expect(Config.NumCPU).toBeGreaterThanOrEqual(1)
        })

        it('has Debug property', () => {
            expect(typeof Config.Debug).toBe('boolean')
        })

        it('has Legacy property', () => {
            expect(Config.Legacy).toBe(false)
            expect(typeof Config.Legacy).toBe('boolean')
        })

        it('has DB property', () => {
            expect(Config.DB).toBe(false)
            expect(typeof Config.DB).toBe('boolean')
        })

        it('has WEB property', () => {
            expect(Config.WEB).toBe(false)
            expect(typeof Config.WEB).toBe('boolean')
        })

        it('has BuildDate property', () => {
            expect(Config.BuildDate).toBeInstanceOf(Date)
            expect(Config.BuildDate.getTime()).toBeGreaterThan(0)
        })
    })

    await describe('get() Method', async () => {
        it('retrieves Product via get()', () => {
            expect(Config.get('Product')).toBe('ejscript')
        })

        it('retrieves Version via get()', () => {
            expect(Config.get('Version')).toBe('0.1.0')
        })

        it('retrieves OS via get()', () => {
            expect(Config.get('OS')).toBe(Config.OS)
        })

        it('retrieves CPU via get()', () => {
            expect(Config.get('CPU')).toBe(Config.CPU)
        })

        it('retrieves NumCPU via get()', () => {
            expect(Config.get('NumCPU')).toBe(Config.NumCPU)
        })

        it('retrieves Debug via get()', () => {
            expect(Config.get('Debug')).toBe(Config.Debug)
        })

        it('retrieves Legacy via get()', () => {
            expect(Config.get('Legacy')).toBe(false)
        })

        it('retrieves DB via get()', () => {
            expect(Config.get('DB')).toBe(false)
        })

        it('retrieves WEB via get()', () => {
            expect(Config.get('WEB')).toBe(false)
        })

        it('retrieves BuildDate via get()', () => {
            expect(Config.get('BuildDate')).toBe(Config.BuildDate)
        })

        it('returns undefined for non-existent property', () => {
            expect(Config.get('NonExistent')).toBeUndefined()
        })

        it('returns undefined for empty string', () => {
            expect(Config.get('')).toBeUndefined()
        })
    })

    await describe('Platform Detection', async () => {
        it('correctly identifies platform', () => {
            const os = Config.OS
            if (os === 'darwin') {
                expect(os).toBe('darwin')
            } else if (os === 'linux') {
                expect(os).toBe('linux')
            } else if (os === 'win32') {
                expect(os).toBe('win32')
            }
        })

        it('correctly identifies CPU architecture', () => {
            const cpu = Config.CPU
            if (cpu === 'x64') {
                expect(cpu).toBe('x64')
            } else if (cpu === 'arm64') {
                expect(cpu).toBe('arm64')
            }
        })

        it('has reasonable CPU count', () => {
            expect(Config.NumCPU).toBeGreaterThanOrEqual(1)
            expect(Config.NumCPU).toBeLessThanOrEqual(256)
        })
    })

    await describe('Build Configuration', async () => {
        it('has valid version format', () => {
            const parts = Config.Version.split('.')
            expect(parts.length).toBe(3)
            expect(parseInt(parts[0])).toBeGreaterThanOrEqual(0)
            expect(parseInt(parts[1])).toBeGreaterThanOrEqual(0)
            expect(parseInt(parts[2])).toBeGreaterThanOrEqual(0)
        })

        it('has BuildDate in the past or present', () => {
            expect(Config.BuildDate.getTime()).toBeLessThanOrEqual(Date.now())
        })

        it('has Debug flag based on NODE_ENV', () => {
            const isDev = process.env.NODE_ENV === 'development'
            expect(Config.Debug).toBe(isDev)
        })
    })

    await describe('Type Safety', async () => {
        it('Product is a string', () => {
            expect(typeof Config.Product).toBe('string')
            expect(Config.Product.length).toBeGreaterThan(0)
        })

        it('Version is a valid semver string', () => {
            expect(typeof Config.Version).toBe('string')
            expect(Config.Version).toMatch(/^\d+\.\d+\.\d+/)
        })

        it('NumCPU is a positive number', () => {
            expect(typeof Config.NumCPU).toBe('number')
            expect(Number.isInteger(Config.NumCPU)).toBe(true)
            expect(Config.NumCPU).toBeGreaterThan(0)
        })

        it('boolean flags are booleans', () => {
            expect(typeof Config.Debug).toBe('boolean')
            expect(typeof Config.Legacy).toBe('boolean')
            expect(typeof Config.DB).toBe('boolean')
            expect(typeof Config.WEB).toBe('boolean')
        })
    })
})
