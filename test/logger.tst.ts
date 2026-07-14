import { describe, it, expect, beforeEach, afterEach } from '@embedthis/testme'
import { tmpdir } from 'os'
import { Logger } from '../src/core/utilities/Logger'
import { Path } from '../src/core/Path'
import { ByteArray } from '../src/core/streams/ByteArray'

await describe('Logger', async () => {
    let logger: Logger | null = null
    let testDir: Path
    let testFile: Path

    beforeEach(async () => {
        testDir = new Path(tmpdir()).join(`logger-test-${process.pid}-${Date.now()}`)
        testFile = testDir.join('test.log')
        await testDir.makeDir()
        // Verify directory was created successfully
        if (!testDir.exists) {
            throw new Error(`Failed to create test directory: ${testDir}`)
        }
        // Increase max listeners to prevent warnings when multiple tests redirect to stderr
        if (process.stderr.setMaxListeners) {
            process.stderr.setMaxListeners(20)
        }
    })

    afterEach(async () => {
        if (logger) {
            try {
                await logger.close()
            } catch {}
            logger = null
        }
        if (testDir.exists) {
            await testDir.removeAll()
        }
    })

    await describe('Construction', async () => {
        it('creates logger with name only', () => {
            logger = new Logger('test')
            expect(logger.name).toBe('test')
            expect(logger.level).toBe(0)
        })

        it('creates logger with name and level', () => {
            logger = new Logger('test', null, 5)
            expect(logger.name).toBe('test')
            expect(logger.level).toBe(5)
        })

        it('creates logger with stderr output', () => {
            logger = new Logger('test', 'stderr', 2)
            expect(logger.name).toBe('test')
            expect(logger.level).toBe(2)
            expect(logger.location).toBe('stderr')
        })

        it('creates logger with stdout output', () => {
            logger = new Logger('test', 'stdout')
            expect(logger.location).toBe('stdout')
        })

        it('creates logger with file output', async () => {
            logger = new Logger('test', testFile.toString())
            expect(logger.location).toBe(testFile.toString())
            // Wait for file to open to avoid race condition with afterEach cleanup
            await new Promise(resolve => setTimeout(resolve, 50))
        })

        it('creates hierarchical logger', () => {
            const parent = new Logger('parent')
            logger = new Logger('child', parent)
            expect(logger.name).toBe('parent.child')
        })
    })

    await describe('Log Levels', async () => {
        it('has static level constants', () => {
            expect(Logger.Off).toBe(-1)
            expect(Logger.Error).toBe(0)
            expect(Logger.Warn).toBe(1)
            expect(Logger.Info).toBe(2)
            expect(Logger.Config).toBe(3)
            expect(Logger.All).toBe(9)
        })

        it('allows setting level', () => {
            logger = new Logger('test')
            logger.level = 5
            expect(logger.level).toBe(5)
        })

        it('propagates level to parent logger', () => {
            const parent = new Logger('parent')
            logger = new Logger('child', parent)
            logger.level = 7
            expect(parent.level).toBe(7)
        })
    })

    await describe('Redirect', async () => {
        it('redirects to stdout', () => {
            logger = new Logger('test')
            logger.redirect('stdout')
            expect(logger.location).toBe('stdout')
        })

        it('redirects to stderr', () => {
            logger = new Logger('test')
            logger.redirect('stderr')
            expect(logger.location).toBe('stderr')
        })

        it('redirects to file', async () => {
            logger = new Logger('test')
            logger.redirect(testFile.toString())
            expect(logger.location).toBe(testFile.toString())
            // Wait for file to open to avoid race condition with afterEach cleanup
            await new Promise(resolve => setTimeout(resolve, 50))
        })

        it('redirects with level in location string', async () => {
            logger = new Logger('test')
            logger.redirect(testFile.toString() + ':5')
            expect(logger.level).toBe(5)
            // Wait for file to open to avoid race condition with afterEach cleanup
            await new Promise(resolve => setTimeout(resolve, 50))
        })

        it('redirects to ByteArray stream', () => {
            const buffer = new ByteArray()
            logger = new Logger('test', buffer)
            expect(logger.outStream).toBe(buffer)
        })
    })

    await describe('Logging Methods', async () => {
        it('logs error messages', () => {
            const buffer = new ByteArray()
            logger = new Logger('test', buffer, Logger.Error)
            logger.error('Test error')

            const output = buffer.toString()
            expect(output).toContain('ERROR')
            expect(output).toContain('Test error')
        })

        it('logs warning messages', () => {
            const buffer = new ByteArray()
            logger = new Logger('test', buffer, Logger.Warn)
            logger.warn('Test warning')

            const output = buffer.toString()
            expect(output).toContain('WARN')
            expect(output).toContain('Test warning')
        })

        it('logs info messages', () => {
            const buffer = new ByteArray()
            logger = new Logger('test', buffer, Logger.Info)
            logger.info('Test info')

            const output = buffer.toString()
            expect(output).toContain('INFO')
            expect(output).toContain('Test info')
        })

        it('logs config messages', () => {
            const buffer = new ByteArray()
            logger = new Logger('test', buffer, Logger.Config)
            logger.config('Test config')

            const output = buffer.toString()
            expect(output).toContain('CONFIG')
            expect(output).toContain('Test config')
        })

        it('logs debug messages', () => {
            const buffer = new ByteArray()
            logger = new Logger('test', buffer, 5)
            logger.debug(3, 'Debug message')

            const output = buffer.toString()
            expect(output).toContain('Debug message')
        })

        it('logs multiple arguments', () => {
            const buffer = new ByteArray()
            logger = new Logger('test', buffer, Logger.Info)
            logger.info('Multiple', 'arguments', 42, true)

            const output = buffer.toString()
            expect(output).toContain('Multiple arguments 42 true')
        })

        it('supports log() alias for info()', () => {
            const buffer = new ByteArray()
            logger = new Logger('test', buffer, Logger.Info)
            logger.log('Log message')

            const output = buffer.toString()
            expect(output).toContain('INFO')
            expect(output).toContain('Log message')
        })

        it('supports trace() alias for debug()', () => {
            const buffer = new ByteArray()
            logger = new Logger('test', buffer, 9)
            logger.trace('Trace message')

            const output = buffer.toString()
            expect(output).toContain('Trace message')
        })
    })

    await describe('Level Filtering', async () => {
        it('filters messages above log level', () => {
            const buffer = new ByteArray()
            logger = new Logger('test', buffer, Logger.Error)

            logger.error('This should appear')
            logger.warn('This should not appear')
            logger.info('This should not appear')

            const output = buffer.toString()
            expect(output).toContain('This should appear')
            expect(output).not.toContain('This should not appear')
        })

        it('allows all messages at All level', () => {
            const buffer = new ByteArray()
            logger = new Logger('test', buffer, Logger.All)

            logger.error('Error')
            logger.warn('Warn')
            logger.info('Info')
            logger.config('Config')

            const output = buffer.toString()
            expect(output).toContain('Error')
            expect(output).toContain('Warn')
            expect(output).toContain('Info')
            expect(output).toContain('Config')
        })

        it('blocks all messages at Off level', () => {
            const buffer = new ByteArray()
            logger = new Logger('test', buffer, Logger.Off)

            logger.error('Should not appear')
            logger.warn('Should not appear')

            const output = buffer.toString()
            expect(output).toBe('')
        })
    })

    await describe('Pattern Matching', async () => {
        it('filters by name pattern', () => {
            const buffer = new ByteArray()
            logger = new Logger('myapp.database', buffer, Logger.Info)
            logger.match = /^myapp\.database/

            logger.info('This should appear')

            const output = buffer.toString()
            expect(output).toContain('This should appear')
        })

        it('blocks non-matching patterns', () => {
            const buffer = new ByteArray()
            logger = new Logger('myapp.network', buffer, Logger.Info)
            logger.match = /^myapp\.database/

            logger.info('This should not appear')

            const output = buffer.toString()
            expect(output).toBe('')
        })
    })

    await describe('Filter Function', async () => {
        it('applies custom filter function', () => {
            const buffer = new ByteArray()
            logger = new Logger('test', buffer, Logger.Info)

            logger.filter = (_log, _name, _level, _kind, msg) => {
                return msg.includes('important')
            }

            logger.info('important message')
            logger.info('regular message')

            const output = buffer.toString()
            expect(output).toContain('important message')
            expect(output).not.toContain('regular message')
        })

        it('receives correct filter parameters', () => {
            const buffer = new ByteArray()
            logger = new Logger('test', buffer, Logger.Warn)

            let capturedLog: Logger | null = null
            let capturedName = ''
            let capturedLevel = -1
            let capturedKind = ''

            logger.filter = (log, name, level, kind, _msg) => {
                capturedLog = log
                capturedName = name
                capturedLevel = level
                capturedKind = kind
                return true
            }

            logger.warn('Test')

            expect(capturedLog).toBe(logger)
            expect(capturedName).toBe('test')
            expect(capturedLevel).toBe(Logger.Warn)
            expect(capturedKind).toBe('WARN')
        })
    })

    await describe('Activity Logging', async () => {
        it('logs activity with tag', () => {
            const buffer = new ByteArray()
            logger = new Logger('test', buffer, Logger.Info)
            logger.activity('START', 'Application starting')

            const output = buffer.toString()
            expect(output).toContain('[START]')
            expect(output).toContain('Application starting')
        })

        it('pads activity tags', () => {
            const buffer = new ByteArray()
            logger = new Logger('test', buffer, Logger.Info)
            logger.activity('OK', 'Done')

            const output = buffer.toString()
            // Tag should be padded to 12 characters
            expect(output).toMatch(/\s+\[OK\]/)
        })
    })

    await describe('Write Method', async () => {
        it('writes directly to stream', () => {
            const buffer = new ByteArray()
            logger = new Logger('test', buffer)
            logger.write('Direct write')

            const output = buffer.toString()
            expect(output).toBe('Direct write')
        })

        it('writes multiple arguments', () => {
            const buffer = new ByteArray()
            logger = new Logger('test', buffer)
            logger.write('Hello', 'World', 42)

            const output = buffer.toString()
            expect(output).toBe('Hello World 42')
        })

        it('returns bytes written', () => {
            const buffer = new ByteArray()
            logger = new Logger('test', buffer)
            const written = logger.write('Test')
            expect(written).toBeGreaterThan(0)
        })

        it('handles write errors gracefully', () => {
            logger = new Logger('test')
            logger.outStream = null
            const written = logger.write('Test')
            expect(written).toBe(0)
        })
    })

    await describe('Hierarchical Logging', async () => {
        it('forwards to parent logger', () => {
            const buffer = new ByteArray()
            const parent = new Logger('parent', buffer, Logger.Info)
            logger = new Logger('child', parent, Logger.Info)

            logger.info('Child message')

            const output = buffer.toString()
            expect(output).toContain('parent.child')
            expect(output).toContain('Child message')
        })

        it('respects parent level filtering', () => {
            const buffer = new ByteArray()
            const parent = new Logger('parent', buffer, Logger.Error)
            logger = new Logger('child', parent, Logger.Info)

            logger.info('Should not appear')
            logger.error('Should appear')

            const output = buffer.toString()
            expect(output).toContain('Should appear')
            expect(output).not.toContain('Should not appear')
        })
    })

    await describe('File Output', async () => {
        it('writes to file', async () => {
            logger = new Logger('test', testFile.toString(), Logger.Info)
            logger.info('File test')
            // Use setImmediate to ensure write is queued before close
            await new Promise(resolve => setImmediate(resolve))
            await logger.close()

            expect(testFile.exists).toBe(true)
            const content = await testFile.readString()
            expect(content).toContain('File test')
        })

        it('appends to existing file', async () => {
            await testFile.write('Initial content\n')

            logger = new Logger('test', testFile.toString(), Logger.Info)
            logger.info('Appended')
            // Use setImmediate to ensure write is queued before close
            await new Promise(resolve => setImmediate(resolve))
            await logger.close()

            const content = await testFile.readString()
            expect(content).toContain('Initial content')
            expect(content).toContain('Appended')
        })
    })

    await describe('Properties', async () => {
        it('has name property', () => {
            logger = new Logger('mylogger')
            expect(logger.name).toBe('mylogger')
        })

        it('allows setting name', () => {
            logger = new Logger('test')
            logger.name = 'newname'
            expect(logger.name).toBe('newname')
        })

        it('has outStream property', () => {
            const buffer = new ByteArray()
            logger = new Logger('test', buffer)
            expect(logger.outStream).toBe(buffer)
        })

        it('allows setting outStream', () => {
            const buffer = new ByteArray()
            logger = new Logger('test')
            logger.outStream = buffer
            expect(logger.outStream).toBe(buffer)
        })

        it('has async property (always false)', () => {
            logger = new Logger('test')
            expect(logger.async).toBe(false)
        })

        it('throws on setting async', () => {
            logger = new Logger('test')
            expect(() => {
                logger.async = true
            }).toThrow('Async mode not supported')
        })
    })

    await describe('Stream Interface', async () => {
        it('throws on read()', () => {
            logger = new Logger('test')
            expect(() => {
                logger.read(new Uint8Array(10))
            }).toThrow('Read not supported')
        })

        it('throws on on()', () => {
            logger = new Logger('test')
            expect(() => {
                logger.on('test', () => {})
            }).toThrow('on is not supported')
        })

        it('throws on off()', () => {
            logger = new Logger('test')
            expect(() => {
                logger.off('test', () => {})
            }).toThrow('off is not supported')
        })

        it('supports flush()', () => {
            logger = new Logger('test')
            expect(() => {
                logger.flush()
            }).not.toThrow()
        })
    })

    await describe('Close', async () => {
        it('closes output stream', async () => {
            logger = new Logger('test', testFile.toString())
            // Wait for file to open before closing
            await new Promise(resolve => setTimeout(resolve, 50))
            await logger.close()
            expect(logger.outStream).toBeNull()
        })

        it('handles null stream gracefully', () => {
            logger = new Logger('test')
            logger.outStream = null
            expect(() => {
                logger.close()
            }).not.toThrow()
        })
    })
})
