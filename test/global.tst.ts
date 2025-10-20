import { describe, it, expect } from 'testme'
import {
    blend,
    clone,
    format,
    md5,
    sha256,
    hashcode,
    assert,
    base64,
    base64Decode,
    parse,
    parseFloat,
    parseInt,
    isNaN,
    isFinite,
    instanceOf,
    evalScript,
    setIntervalTimer,
    clearIntervalTimer,
    setTimeoutTimer,
    clearTimeoutTimer,
} from '../src/core/utilities/Global'

await describe('Global Utilities', async () => {
    await describe('blend', async () => {
        it('merges objects', () => {
            const dest = { a: 1, b: 2 }
            const src = { c: 3, d: 4 }
            blend(dest, src)
            expect(dest).toEqual({ a: 1, b: 2, c: 3, d: 4 })
        })

        it('overwrites by default', () => {
            const dest = { a: 1, b: 2 }
            const src = { b: 99, c: 3 }
            blend(dest, src)
            expect(dest.b).toBe(99)
        })

        it('respects overwrite: false option', () => {
            const dest = { a: 1, b: 2 }
            const src = { b: 99, c: 3 }
            blend(dest, src, { overwrite: false })
            expect(dest.b).toBe(2)
            expect(dest.c).toBe(3)
        })

        it('blends nested objects recursively', () => {
            const dest = { a: { x: 1 } }
            const src = { a: { y: 2 } }
            blend(dest, src)
            expect(dest.a).toEqual({ x: 1, y: 2 })
        })

        it('skips functions by default when functions: false', () => {
            const dest = { a: 1 }
            const src = { b: () => 42 }
            blend(dest, src, { functions: false })
            expect(dest.b).toBeUndefined()
        })

        it('includes functions when functions: true', () => {
            const dest = { a: 1 }
            const src = { b: () => 42 }
            blend(dest, src, { functions: true })
            expect(typeof dest.b).toBe('function')
            expect(dest.b()).toBe(42)
        })

        it('returns destination object', () => {
            const dest = { a: 1 }
            const result = blend(dest, { b: 2 })
            expect(result).toBe(dest)
        })
    })

    await describe('clone', async () => {
        it('clones primitive values', () => {
            expect(clone(42)).toBe(42)
            expect(clone('test')).toBe('test')
            expect(clone(true)).toBe(true)
            expect(clone(null)).toBe(null)
        })

        it('deep clones objects', () => {
            const obj = { a: 1, b: { c: 2 } }
            const cloned = clone(obj)
            expect(cloned).toEqual(obj)
            expect(cloned).not.toBe(obj)
            expect(cloned.b).not.toBe(obj.b)
        })

        it('shallow clones objects when deep: false', () => {
            const obj = { a: 1, b: { c: 2 } }
            const cloned = clone(obj, false)
            expect(cloned).toEqual(obj)
            expect(cloned).not.toBe(obj)
            expect(cloned.b).toBe(obj.b)
        })

        it('clones arrays', () => {
            const arr = [1, 2, [3, 4]]
            const cloned = clone(arr)
            expect(cloned).toEqual(arr)
            expect(cloned).not.toBe(arr)
            expect(cloned[2]).not.toBe(arr[2])
        })

        it('clones Date objects', () => {
            const date = new Date()
            const cloned = clone(date)
            expect(cloned).toEqual(date)
            expect(cloned).not.toBe(date)
        })

        it('clones RegExp objects', () => {
            const regex = /test/gi
            const cloned = clone(regex)
            expect(cloned.source).toBe(regex.source)
            expect(cloned.flags).toBe(regex.flags)
            expect(cloned).not.toBe(regex)
        })
    })

    await describe('format', async () => {
        it('substitutes variables in template', () => {
            const result = format('Hello ${name}!', { name: 'World' })
            expect(result).toBe('Hello World!')
        })

        it('substitutes multiple variables', () => {
            const result = format('${greeting} ${name}!', { greeting: 'Hi', name: 'Bob' })
            expect(result).toBe('Hi Bob!')
        })

        it('leaves unmatched placeholders', () => {
            const result = format('Hello ${missing}', {})
            expect(result).toBe('Hello ${missing}')
        })

        it('converts values to strings', () => {
            const result = format('Number: ${num}', { num: 42 })
            expect(result).toBe('Number: 42')
        })
    })

    await describe('md5', async () => {
        it('hashes string to MD5', () => {
            const hash = md5('test')
            expect(hash).toBe('098f6bcd4621d373cade4e832627b4f6')
        })

        it('produces consistent hashes', () => {
            const hash1 = md5('hello')
            const hash2 = md5('hello')
            expect(hash1).toBe(hash2)
        })

        it('produces different hashes for different inputs', () => {
            const hash1 = md5('hello')
            const hash2 = md5('world')
            expect(hash1).not.toBe(hash2)
        })
    })

    await describe('sha256', async () => {
        it('hashes string to SHA256', () => {
            const hash = sha256('test')
            expect(hash).toBe('9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a08')
        })

        it('produces consistent hashes', () => {
            const hash1 = sha256('hello')
            const hash2 = sha256('hello')
            expect(hash1).toBe(hash2)
        })

        it('produces different hashes for different inputs', () => {
            const hash1 = sha256('hello')
            const hash2 = sha256('world')
            expect(hash1).not.toBe(hash2)
        })
    })

    await describe('hashcode', async () => {
        it('returns 0 for null', () => {
            expect(hashcode(null)).toBe(0)
        })

        it('returns 0 for undefined', () => {
            expect(hashcode(undefined)).toBe(0)
        })

        it('returns number for number input', () => {
            expect(hashcode(42)).toBe(42)
        })

        it('returns 1 for true', () => {
            expect(hashcode(true)).toBe(1)
        })

        it('returns 0 for false', () => {
            expect(hashcode(false)).toBe(0)
        })

        it('returns consistent hash for same string', () => {
            const hash1 = hashcode('test')
            const hash2 = hashcode('test')
            expect(hash1).toBe(hash2)
        })

        it('returns consistent hash for same object', () => {
            const obj = { a: 1 }
            const hash1 = hashcode(obj)
            const hash2 = hashcode(obj)
            expect(hash1).toBe(hash2)
        })

        it('returns different hashes for different objects', () => {
            const obj1 = { a: 1 }
            const obj2 = { a: 1 }
            const hash1 = hashcode(obj1)
            const hash2 = hashcode(obj2)
            expect(hash1).not.toBe(hash2)
        })
    })

    await describe('assert', async () => {
        it('passes when condition is true', () => {
            expect(() => {
                assert(true)
            }).not.toThrow()
        })

        it('throws when condition is false', () => {
            expect(() => {
                assert(false)
            }).toThrow('Assertion failed')
        })

        it('throws with custom message', () => {
            expect(() => {
                assert(false, 'Custom error')
            }).toThrow('Custom error')
        })

        it('accepts truthy values', () => {
            expect(() => {
                assert(1 as any)
                assert('test' as any)
                assert({} as any)
            }).not.toThrow()
        })

        it('rejects falsy values', () => {
            expect(() => {
                assert(0 as any)
            }).toThrow()
        })
    })

    await describe('base64', async () => {
        it('encodes string to base64', () => {
            const encoded = base64('Hello World')
            expect(encoded).toBe('SGVsbG8gV29ybGQ=')
        })

        it('encodes empty string', () => {
            const encoded = base64('')
            expect(encoded).toBe('')
        })

        it('encodes special characters', () => {
            const encoded = base64('ña©™')
            expect(typeof encoded).toBe('string')
            expect(encoded.length).toBeGreaterThan(0)
        })
    })

    await describe('base64Decode', async () => {
        it('decodes base64 string', () => {
            const decoded = base64Decode('SGVsbG8gV29ybGQ=')
            expect(decoded).toBe('Hello World')
        })

        it('decodes empty string', () => {
            const decoded = base64Decode('')
            expect(decoded).toBe('')
        })

        it('round-trips correctly', () => {
            const original = 'Test String 123 !@#'
            const encoded = base64(original)
            const decoded = base64Decode(encoded)
            expect(decoded).toBe(original)
        })
    })

    await describe('parse', async () => {
        it('parses boolean true', () => {
            expect(parse('true')).toBe(true)
        })

        it('parses boolean false', () => {
            expect(parse('false')).toBe(false)
        })

        it('parses null', () => {
            expect(parse('null')).toBe(null)
        })

        it('parses undefined', () => {
            expect(parse('undefined')).toBe(undefined)
        })

        it('parses integers', () => {
            expect(parse('42')).toBe(42)
            expect(parse('-10')).toBe(-10)
        })

        it('parses floats', () => {
            expect(parse('3.14')).toBe(3.14)
            expect(parse('-2.5')).toBe(-2.5)
        })

        it('parses JSON arrays', () => {
            expect(parse('[1,2,3]')).toEqual([1, 2, 3])
        })

        it('parses JSON objects', () => {
            expect(parse('{"a":1}')).toEqual({ a: 1 })
        })

        it('returns string for unparseable values', () => {
            expect(parse('random text')).toBe('random text')
        })

        it('respects preferredType', () => {
            expect(parse('42', Number)).toBe(42)
        })
    })

    await describe('parseFloat', async () => {
        it('parses float strings', () => {
            expect(parseFloat('3.14')).toBe(3.14)
        })

        it('parses integer strings', () => {
            expect(parseFloat('42')).toBe(42)
        })

        it('parses negative numbers', () => {
            expect(parseFloat('-10.5')).toBe(-10.5)
        })

        it('returns NaN for invalid input', () => {
            expect(Number.isNaN(parseFloat('abc'))).toBe(true)
        })
    })

    await describe('parseInt', async () => {
        it('parses integer strings', () => {
            expect(parseInt('42')).toBe(42)
        })

        it('parses with radix', () => {
            expect(parseInt('10', 2)).toBe(2)
            expect(parseInt('FF', 16)).toBe(255)
        })

        it('parses negative numbers', () => {
            expect(parseInt('-42')).toBe(-42)
        })

        it('returns NaN for invalid input', () => {
            expect(Number.isNaN(parseInt('abc'))).toBe(true)
        })
    })

    await describe('isNaN', async () => {
        it('returns true for NaN', () => {
            expect(isNaN(NaN)).toBe(true)
        })

        it('returns false for numbers', () => {
            expect(isNaN(42)).toBe(false)
            expect(isNaN(3.14)).toBe(false)
        })

        it('returns false for Infinity', () => {
            expect(isNaN(Infinity)).toBe(false)
        })
    })

    await describe('isFinite', async () => {
        it('returns true for finite numbers', () => {
            expect(isFinite(42)).toBe(true)
            expect(isFinite(3.14)).toBe(true)
            expect(isFinite(0)).toBe(true)
        })

        it('returns false for Infinity', () => {
            expect(isFinite(Infinity)).toBe(false)
            expect(isFinite(-Infinity)).toBe(false)
        })

        it('returns false for NaN', () => {
            expect(isFinite(NaN)).toBe(false)
        })
    })

    await describe('instanceOf', async () => {
        it('checks instanceof for objects', () => {
            const arr = [1, 2, 3]
            expect(instanceOf(arr, Array)).toBe(true)
        })

        it('returns false for non-instances', () => {
            const obj = {}
            expect(instanceOf(obj, Array)).toBe(false)
        })

        it('works with custom classes', () => {
            class MyClass {}
            const instance = new MyClass()
            expect(instanceOf(instance, MyClass)).toBe(true)
        })
    })

    await describe('evalScript', async () => {
        it('evaluates simple expressions', () => {
            const result = evalScript('2 + 2')
            expect(result).toBe(4)
        })

        it('evaluates string literals', () => {
            const result = evalScript('"hello"')
            expect(result).toBe('hello')
        })

        it('evaluates object access', () => {
            const result = evalScript('({ a: 1, b: 2 })')
            expect(result).toEqual({ a: 1, b: 2 })
        })
    })

    await describe('setIntervalTimer', async () => {
        it('creates repeating timer', async () => {
            let count = 0
            await new Promise<void>((resolve) => {
                const timer = setIntervalTimer(() => {
                    count++
                    if (count >= 3) {
                        clearIntervalTimer(timer)
                        expect(count).toBeGreaterThanOrEqual(3)
                        resolve()
                    }
                }, 30)
            })
        })

        it('passes arguments to callback', async () => {
            let receivedArgs: any[] = []
            await new Promise<void>((resolve) => {
                const timer = setIntervalTimer((...args: any[]) => {
                    receivedArgs = args
                    clearIntervalTimer(timer)
                    expect(receivedArgs).toEqual([1, 2, 3])
                    resolve()
                }, 30, 1, 2, 3)
            })
        })
    })

    await describe('clearIntervalTimer', async () => {
        it('stops repeating timer', async () => {
            let count = 0
            const timer = setIntervalTimer(() => {
                count++
            }, 30)

            await new Promise<void>((resolve) => {
                setTimeout(() => {
                    clearIntervalTimer(timer)
                    const countAtStop = count

                    setTimeout(() => {
                        expect(count).toBe(countAtStop)
                        resolve()
                    }, 80)
                }, 70)
            })
        })
    })

    await describe('setTimeoutTimer', async () => {
        it('creates one-shot timer', async () => {
            let called = false
            await new Promise<void>((resolve) => {
                setTimeoutTimer(() => {
                    called = true
                    expect(called).toBe(true)
                    resolve()
                }, 50)
            })
        })

        it('passes arguments to callback', async () => {
            let receivedArgs: any[] = []
            await new Promise<void>((resolve) => {
                setTimeoutTimer((...args: any[]) => {
                    receivedArgs = args
                    expect(receivedArgs).toEqual(['a', 'b', 'c'])
                    resolve()
                }, 50, 'a', 'b', 'c')
            })
        })
    })

    await describe('clearTimeoutTimer', async () => {
        it('cancels timeout timer', async () => {
            let called = false
            const timer = setTimeoutTimer(() => {
                called = true
            }, 50)

            clearTimeoutTimer(timer)

            await new Promise<void>((resolve) => {
                setTimeout(() => {
                    expect(called).toBe(false)
                    resolve()
                }, 100)
            })
        })
    })
})
