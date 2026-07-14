/**
 * Args Tests
 * Based on Ejscript Args.es specification
 */

import { describe, it, expect } from 'testme'
import { Args } from '../src/core/Args'
import { Path } from '../src/core/Path'

await describe('Args', async () => {
    await describe('Basic Parsing', async () => {
        it('parses simple flags', () => {
            const args = new Args({
                options: {
                    verbose: {},
                    quiet: {}
                }
            }, ['program', '--verbose', '--quiet'])

            expect(args.options.verbose).toBe(true)
            expect(args.options.quiet).toBe(true)
        })

        it('parses short form flags', () => {
            const args = new Args({
                options: {
                    verbose: { alias: 'v' }
                }
            }, ['program', '-v'])

            expect(args.options.verbose).toBe(true)
        })

        it('parses options with arguments', () => {
            const args = new Args({
                options: {
                    output: { range: String }
                }
            }, ['program', '--output', 'file.txt'])

            expect(args.options.output).toBe('file.txt')
        })

        it('parses options with = syntax', () => {
            const args = new Args({
                options: {
                    output: { range: String }
                }
            }, ['program', '--output=file.txt'])

            expect(args.options.output).toBe('file.txt')
        })

        it('collects remaining arguments in rest', () => {
            const args = new Args({
                options: {
                    verbose: {}
                }
            }, ['program', '--verbose', 'arg1', 'arg2', 'arg3'])

            expect(args.rest).toEqual(['arg1', 'arg2', 'arg3'])
        })

        it('sets program name', () => {
            const args = new Args({
                options: {}
            }, ['/usr/bin/myprogram'])

            expect(args.program).toBeInstanceOf(Path)
            expect(args.program.toString()).toBe('/usr/bin/myprogram')
        })
    })

    await describe('Type Validation', async () => {
        it('validates Number range', () => {
            const args = new Args({
                options: {
                    port: { range: Number }
                }
            }, ['program', '--port', '8080'])

            expect(args.options.port).toBe(8080)
            expect(typeof args.options.port).toBe('number')
        })

        it('validates Boolean range', () => {
            const args = new Args({
                options: {
                    enabled: { range: Boolean }
                }
            }, ['program', '--enabled', 'true'])

            expect(args.options.enabled).toBe(true)
        })

        it('validates String range', () => {
            const args = new Args({
                options: {
                    name: { range: String }
                }
            }, ['program', '--name', 'test'])

            expect(args.options.name).toBe('test')
            expect(typeof args.options.name).toBe('string')
        })

        it('validates Path range', () => {
            const args = new Args({
                options: {
                    dir: { range: Path }
                }
            }, ['program', '--dir', '/tmp/test'])

            expect(args.options.dir).toBeInstanceOf(Path)
            expect(args.options.dir.toString()).toBe('/tmp/test')
        })

        it('validates RegExp range', () => {
            const args = new Args({
                options: {
                    log: { range: /\w+(:\d+)/ }
                }
            }, ['program', '--log', 'stderr:4'])

            expect(args.options.log).toBe('stderr:4')
        })

        it('throws on invalid RegExp value', () => {
            expect(() => {
                new Args({
                    options: {
                        log: { range: /\w+(:\d+)/ }
                    },
                    onerror: 'throw'
                }, ['program', '--log', 'invalid'])
            }).toThrow()
        })

        it('validates array of allowed values', () => {
            const args = new Args({
                options: {
                    mode: { range: ['low', 'medium', 'high'] }
                }
            }, ['program', '--mode', 'medium'])

            expect(args.options.mode).toBe('medium')
        })

        it('throws on invalid array value', () => {
            expect(() => {
                new Args({
                    options: {
                        mode: { range: ['low', 'medium', 'high'] }
                    },
                    onerror: 'throw'
                }, ['program', '--mode', 'invalid'])
            }).toThrow()
        })
    })

    await describe('Default Values', async () => {
        it('uses default value when option not specified', () => {
            const args = new Args({
                options: {
                    mode: { range: ['low', 'medium', 'high'], value: 'high' }
                }
            }, ['program'])

            expect(args.options.mode).toBe('high')
        })

        it('overrides default value when option specified', () => {
            const args = new Args({
                options: {
                    mode: { range: ['low', 'medium', 'high'], value: 'high' }
                }
            }, ['program', '--mode', 'low'])

            expect(args.options.mode).toBe('low')
        })
    })

    await describe('Aliases', async () => {
        it('maps alias to full option name', () => {
            const args = new Args({
                options: {
                    verbose: { alias: 'v' }
                }
            }, ['program', '-v'])

            expect(args.options.verbose).toBe(true)
        })

        it('allows both alias and full name', () => {
            const args1 = new Args({
                options: {
                    verbose: { alias: 'v' }
                }
            }, ['program', '-v'])

            const args2 = new Args({
                options: {
                    verbose: { alias: 'v' }
                }
            }, ['program', '--verbose'])

            expect(args1.options.verbose).toBe(true)
            expect(args2.options.verbose).toBe(true)
        })
    })

    await describe('Separator (Multiple Values)', async () => {
        it('concatenates multiple values with string separator', () => {
            const args = new Args({
                options: {
                    with: { range: String, separator: ',' }
                }
            }, ['program', '--with', 'foo', '--with', 'bar', '--with', 'baz'])

            expect(args.options.with).toBe('foo,bar,baz')
        })

        it('collects multiple values in array with Array separator', () => {
            const args = new Args({
                options: {
                    include: { range: String, separator: Array }
                }
            }, ['program', '--include', 'a', '--include', 'b', '--include', 'c'])

            expect(args.options.include).toEqual(['a', 'b', 'c'])
        })

        it('splits comma-separated values when commas is true', () => {
            const args = new Args({
                options: {
                    with: { range: String, separator: Array, commas: true }
                }
            }, ['program', '--with', 'foo,bar,baz'])

            expect(args.options.with).toEqual(['foo', 'bar', 'baz'])
        })
    })

    await describe('Error Handling', async () => {
        it('throws on undefined option when onerror is throw', () => {
            expect(() => {
                new Args({
                    options: {},
                    onerror: 'throw'
                }, ['program', '--undefined'])
            }).toThrow(/Undefined option/)
        })

        it('throws on missing option argument', () => {
            expect(() => {
                new Args({
                    options: {
                        output: { range: String }
                    },
                    onerror: 'throw'
                }, ['program', '--output'])
            }).toThrow(/Missing option/)
        })

        it('calls usage function on help flag', () => {
            let usageCalled = false
            new Args({
                options: {},
                usage: () => { usageCalled = true }
            }, ['program', '-?'])

            expect(usageCalled).toBe(true)
        })

        it('supports unknown callback for custom handling', () => {
            let unknownArg = ''
            new Args({
                options: {},
                unknown: (argv, i) => {
                    unknownArg = argv[i]
                    return i // Continue parsing
                }
            }, ['program', '--custom-flag'])

            expect(unknownArg).toBe('--custom-flag')
        })
    })

    await describe('Complete Example', async () => {
        it('parses complex command line', () => {
            const args = new Args({
                options: {
                    depth: { range: Number },
                    quiet: {},
                    verbose: { alias: 'v', value: false },
                    log: { range: /\w+(:\d+)/, value: 'stderr:4' },
                    mode: { range: ['low', 'medium', 'high'], value: 'high' },
                    with: { range: String, separator: ',' }
                }
            }, [
                'program',
                '--depth', '5',
                '-v',
                '--log', 'stdout:2',
                '--mode', 'low',
                '--with', 'foo',
                '--with', 'bar',
                'file1.txt',
                'file2.txt'
            ])

            expect(args.options.depth).toBe(5)
            expect(args.options.verbose).toBe(true)
            expect(args.options.log).toBe('stdout:2')
            expect(args.options.mode).toBe('low')
            expect(args.options.with).toBe('foo,bar')
            expect(args.rest).toEqual(['file1.txt', 'file2.txt'])
        })
    })
})
