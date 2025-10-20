import { describe, it, expect, beforeAll } from 'testme'
import { App } from '../src/core/App'
import { Path } from '../src/core/Path'
import { Emitter } from '../src/core/async/Emitter'

await describe('App', async () => {
    beforeAll(() => {
        // Initialize App
        App.init()
    })

    await describe('Constants', async () => {
        it('should have SearchSeparator for platform', () => {
            expect(App.SearchSeparator).toBeDefined()
            // Unix-like systems use ':', Windows uses ';'
            expect([':',';']).toContain(App.SearchSeparator)
        })

        it('should have correct SearchSeparator for platform', () => {
            if (process.platform === 'win32') {
                expect(App.SearchSeparator).toBe(';')
            } else {
                expect(App.SearchSeparator).toBe(':')
            }
        })
    })

    await describe('Singleton Properties', async () => {
        await describe('config', async () => {
            it('should have config object', () => {
                expect(App.config).toBeDefined()
                expect(typeof App.config).toBe('object')
            })

            it('should be modifiable', () => {
                App.config.testValue = 'test'
                expect(App.config.testValue).toBe('test')
                delete App.config.testValue
            })
        })

        await describe('emitter', async () => {
            it('should have emitter instance', () => {
                expect(App.emitter).toBeInstanceOf(Emitter)
            })

            it('should support event operations', () => {
                let called = false
                const handler = () => { called = true }
                App.emitter.on('test-event', handler)
                App.emitter.emit('test-event')
                expect(called).toBe(true)
                App.emitter.off('test-event', handler)
            })
        })

        await describe('name, title, version', async () => {
            it('should have name property', () => {
                expect(App.name).toBeDefined()
                expect(typeof App.name).toBe('string')
            })

            it('should allow setting name', () => {
                const original = App.name
                App.name = 'TestApp'
                expect(App.name).toBe('TestApp')
                App.name = original
            })

            it('should have title property', () => {
                expect(App.title).toBeDefined()
                expect(typeof App.title).toBe('string')
            })

            it('should have version property', () => {
                expect(App.version).toBeDefined()
                expect(typeof App.version).toBe('string')
            })
        })
    })

    await describe('Process Properties', async () => {
        await describe('args', async () => {
            it('should return array of arguments', () => {
                const args = App.args
                expect(Array.isArray(args)).toBe(true)
            })

            it('should contain process arguments', () => {
                const args = App.args
                expect(args.length).toBeGreaterThanOrEqual(0)
            })

            it('should be read-only (returns copy)', () => {
                const args1 = App.args
                const args2 = App.args
                expect(args1).not.toBe(args2)
            })
        })

        await describe('env', async () => {
            it('should return environment variables', () => {
                const env = App.env
                expect(typeof env).toBe('object')
            })

            it('should contain PATH variable', () => {
                const env = App.env
                expect(env.PATH || env.Path).toBeDefined()
            })

            it('should be read-only (returns copy)', () => {
                const env1 = App.env
                const env2 = App.env
                expect(env1).not.toBe(env2)
            })
        })

        await describe('pid', async () => {
            it('should return process id', () => {
                const pid = App.pid
                expect(typeof pid).toBe('number')
                expect(pid).toBeGreaterThan(0)
            })

            it('should match process.pid', () => {
                expect(App.pid).toBe(process.pid)
            })
        })

        await describe('uid and gid', async () => {
            it('should return user id', () => {
                const uid = App.uid
                expect(typeof uid).toBe('number')
                // On Windows, uid is typically 0
                expect(uid).toBeGreaterThanOrEqual(0)
            })

            it('should return group id', () => {
                const gid = App.gid
                expect(typeof gid).toBe('number')
                expect(gid).toBeGreaterThanOrEqual(0)
            })
        })
    })

    await describe('Directory Properties', async () => {
        await describe('dir', async () => {
            it('should return current working directory', () => {
                const dir = App.dir
                expect(dir).toBeInstanceOf(Path)
                expect(dir.exists).toBe(true)
            })

            it('should be absolute path', () => {
                const dir = App.dir
                expect(dir.isAbsolute).toBe(true)
            })

            it('should match process.cwd()', () => {
                const dir = App.dir
                expect(dir.toString()).toBe(process.cwd())
            })
        })

        await describe('exeDir', async () => {
            it('should return executable directory', () => {
                const exeDir = App.exeDir
                expect(exeDir).toBeInstanceOf(Path)
            })

            it('should be absolute path', () => {
                const exeDir = App.exeDir
                expect(exeDir.isAbsolute).toBe(true)
            })

            it('should be directory', () => {
                const exeDir = App.exeDir
                expect(exeDir.isDir).toBe(true)
            })
        })

        await describe('exePath', async () => {
            it('should return executable path', () => {
                const exePath = App.exePath
                expect(exePath).toBeInstanceOf(Path)
            })

            it('should be absolute path', () => {
                const exePath = App.exePath
                expect(exePath.isAbsolute).toBe(true)
            })

            it('should be file', () => {
                const exePath = App.exePath
                expect(exePath.exists).toBe(true)
            })
        })

        await describe('home', async () => {
            it('should return home directory', () => {
                const home = App.home
                expect(home).toBeInstanceOf(Path)
            })

            it('should be absolute path', () => {
                const home = App.home
                expect(home.isAbsolute).toBe(true)
            })

            it('should exist', () => {
                const home = App.home
                expect(home.exists).toBe(true)
            })
        })
    })

    await describe('Stream Properties', async () => {
        await describe('outputStream', async () => {
            it('should have output stream', () => {
                expect(App.outputStream).toBeDefined()
            })

            it('should be writable', () => {
                const written = App.outputStream.write('')
                expect(typeof written).toBe('number')
            })

            it('should allow setting custom stream', () => {
                const originalStream = App.outputStream
                const customStream = {
                    write: () => 0,
                    close: () => {},
                    flush: () => {},
                    on: () => customStream,
                    off: () => {}
                }
                App.outputStream = customStream as any
                expect(App.outputStream).toBe(customStream)
                App.outputStream = originalStream
            })
        })

        await describe('errorStream', async () => {
            it('should have error stream', () => {
                expect(App.errorStream).toBeDefined()
            })

            it('should be writable', () => {
                const written = App.errorStream.write('')
                expect(typeof written).toBe('number')
            })

            it('should allow setting custom stream', () => {
                const originalStream = App.errorStream
                const customStream = {
                    write: () => 0,
                    close: () => {},
                    flush: () => {},
                    on: () => customStream,
                    off: () => {}
                }
                App.errorStream = customStream as any
                expect(App.errorStream).toBe(customStream)
                App.errorStream = originalStream
            })
        })

        await describe('inputStream', async () => {
            it('should have input stream', () => {
                expect(App.inputStream).toBeDefined()
            })

            it('should allow setting custom stream', () => {
                const originalStream = App.inputStream
                const customStream = {
                    read: () => null,
                    close: () => {},
                    flush: () => {},
                    on: () => customStream,
                    off: () => {}
                }
                App.inputStream = customStream as any
                expect(App.inputStream).toBe(customStream)
                App.inputStream = originalStream
            })
        })
    })

    await describe('Search Path', async () => {
        await describe('search', async () => {
            it('should return array of Path objects', () => {
                const search = App.search
                expect(Array.isArray(search)).toBe(true)
                search.forEach(path => {
                    expect(path).toBeInstanceOf(Path)
                })
            })

            it('should be modifiable', () => {
                const original = App.search
                const newPaths = [new Path('/test1'), new Path('/test2')]
                App.search = newPaths
                expect(App.search).toEqual(newPaths)
                App.search = original
            })

            it('should accept empty array', () => {
                const original = App.search
                App.search = []
                expect(App.search).toEqual([])
                App.search = original
            })
        })
    })

    await describe('Methods', async () => {
        await describe('getpass()', async () => {
            it('should be a function', () => {
                expect(typeof App.getpass).toBe('function')
            })

            it('should accept prompt and stream parameters', () => {
                // Don't actually call since it requires terminal input
                expect(() => {
                    // Just verify signature
                    const fn: (prompt?: string, stream?: any) => string = App.getpass
                    expect(fn).toBeDefined()
                }).not.toThrow()
            })
        })

        await describe('chdir()', async () => {
            it('should change working directory', () => {
                const originalDir = App.dir
                const testDir = originalDir.parent || originalDir

                App.chdir(testDir)
                expect(App.dir.toString()).toBe(testDir.toString())

                // Restore original directory
                App.chdir(originalDir)
                expect(App.dir.toString()).toBe(originalDir.toString())
            })

            it('should accept Path object', () => {
                const originalDir = App.dir
                const testPath = new Path(originalDir.toString())

                App.chdir(testPath)
                expect(App.dir.toString()).toBe(testPath.toString())

                App.chdir(originalDir)
            })

            it('should accept string path', () => {
                const originalDir = App.dir
                const testPathStr = originalDir.toString()

                App.chdir(testPathStr)
                expect(App.dir.toString()).toBe(testPathStr)

                App.chdir(originalDir)
            })
        })

        await describe('exit()', async () => {
            it('should be a function', () => {
                expect(typeof App.exit).toBe('function')
            })

            // Don't actually call exit in tests
            it('should accept status code parameter', () => {
                const fn: (status?: number) => never = App.exit
                expect(fn).toBeDefined()
            })
        })

        await describe('sleep()', async () => {
            it('should pause execution', async () => {
                const start = Date.now()
                await App.sleep(100)
                const elapsed = Date.now() - start
                expect(elapsed).toBeGreaterThanOrEqual(90) // Allow some variance
                expect(elapsed).toBeLessThan(200)
            })

            it('should accept milliseconds parameter', async () => {
                const start = Date.now()
                await App.sleep(50)
                const elapsed = Date.now() - start
                expect(elapsed).toBeGreaterThanOrEqual(40)
            })

            it('should work with zero delay', async () => {
                const start = Date.now()
                await App.sleep(0)
                const elapsed = Date.now() - start
                expect(elapsed).toBeLessThan(50)
            })
        })

        await describe('run()', async () => {
            it('should execute command', () => {
                const result = App.run('echo test')
                expect(typeof result).toBe('string')
            })

            it('should return command output', () => {
                const result = App.run('echo hello')
                expect(result.trim()).toBe('hello')
            })

            it('should handle commands with multiple words', () => {
                const result = App.run('echo "hello world"')
                expect(result.trim()).toBe('hello world')
            })
        })

        await describe('system()', async () => {
            it('should execute system command', () => {
                const exitCode = App.system('echo test')
                expect(typeof exitCode).toBe('number')
                expect(exitCode).toBe(0)
            })

            it('should return exit code', () => {
                const exitCode = App.system('exit 0')
                expect(exitCode).toBe(0)
            })
        })
    })

    await describe('Configuration Loading', async () => {
        await describe('init()', async () => {
            it('should initialize app', () => {
                expect(() => {
                    App.init()
                }).not.toThrow()
            })

            it('should set default name if not specified', () => {
                App.init()
                expect(App.name).toBeDefined()
                expect(App.name.length).toBeGreaterThan(0)
            })

            it('should accept config parameter', () => {
                expect(() => {
                    App.init({ test: true })
                }).not.toThrow()
            })
        })

        await describe('loadrc()', async () => {
            it('should load configuration file', () => {
                const config = App.loadrc()
                expect(typeof config).toBe('object')
            })

            it('should return object even if file not found', () => {
                const config = App.loadrc()
                expect(config).toBeDefined()
            })
        })
    })

    await describe('Integration', async () => {
        it('should have consistent directory relationships', () => {
            const dir = App.dir
            const exeDir = App.exeDir
            expect(dir).toBeInstanceOf(Path)
            expect(exeDir).toBeInstanceOf(Path)
        })

        it('should maintain singleton state', () => {
            const emitter1 = App.emitter
            const emitter2 = App.emitter
            expect(emitter1).toBe(emitter2)
        })

        it('should handle stream operations', () => {
            const written = App.outputStream.write('test')
            expect(written).toBeGreaterThan(0)
        })

        it('should execute commands successfully', () => {
            const result = App.run('echo test')
            expect(result).toBeTruthy()
            expect(result.includes('test')).toBe(true)
        })

        it('should have working environment access', () => {
            const env = App.env
            const args = App.args
            const pid = App.pid

            expect(env).toBeDefined()
            expect(args).toBeDefined()
            expect(pid).toBeGreaterThan(0)
        })
    })
})
