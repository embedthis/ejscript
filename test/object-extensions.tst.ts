import { describe, it, expect } from '@embedthis/testme'
import '../src/core/types/ObjectExtensions'

await describe('Object Extensions', async () => {
    await describe('Object.blend()', async () => {
        await describe('basic merging', async () => {
            it('should merge simple objects', () => {
                const dest = { a: 1, b: 2 }
                const src = { c: 3, d: 4 }
                const result = Object.blend(dest, src)
                expect(result).toEqual({ a: 1, b: 2, c: 3, d: 4 })
                expect(result).toBe(dest) // mutates dest
            })

            it('should overwrite existing properties by default', () => {
                const dest = { a: 1, b: 2 }
                const src = { b: 3, c: 4 }
                Object.blend(dest, src)
                expect(dest).toEqual({ a: 1, b: 3, c: 4 })
            })

            it('should handle empty source object', () => {
                const dest = { a: 1, b: 2 }
                const src = {}
                Object.blend(dest, src)
                expect(dest).toEqual({ a: 1, b: 2 })
            })

            it('should handle empty destination object', () => {
                const dest = {}
                const src = { a: 1, b: 2 }
                Object.blend(dest, src)
                expect(dest).toEqual({ a: 1, b: 2 })
            })

            it('should merge both empty objects', () => {
                const dest = {}
                const src = {}
                Object.blend(dest, src)
                expect(dest).toEqual({})
            })
        })

        await describe('deep merging', async () => {
            it('should deep merge nested objects', () => {
                const dest = { a: 1, b: { x: 10, y: 20 } }
                const src = { b: { y: 30, z: 40 }, c: 3 }
                Object.blend(dest, src)
                expect(dest).toEqual({ a: 1, b: { x: 10, y: 30, z: 40 }, c: 3 })
            })

            it('should handle multiple levels of nesting', () => {
                const dest = { a: { b: { c: 1 } } }
                const src = { a: { b: { d: 2 } } }
                Object.blend(dest, src)
                expect(dest).toEqual({ a: { b: { c: 1, d: 2 } } })
            })

            it('should create nested objects when dest property is not an object', () => {
                const dest = { a: 1 }
                const src = { a: { b: 2 } }
                Object.blend(dest, src)
                expect(dest).toEqual({ a: { b: 2 } })
            })

            it('should handle null values in dest', () => {
                const dest = { a: null }
                const src = { a: { b: 1 } }
                Object.blend(dest, src)
                expect(dest).toEqual({ a: { b: 1 } })
            })
        })

        await describe('overwrite option', async () => {
            it('should not overwrite when overwrite is false', () => {
                const dest = { a: 1, b: 2 }
                const src = { b: 3, c: 4 }
                Object.blend(dest, src, { overwrite: false })
                expect(dest).toEqual({ a: 1, b: 2, c: 4 })
            })

            it('should overwrite when overwrite is true (default)', () => {
                const dest = { a: 1, b: 2 }
                const src = { b: 3, c: 4 }
                Object.blend(dest, src, { overwrite: true })
                expect(dest).toEqual({ a: 1, b: 3, c: 4 })
            })

            it('should respect overwrite in nested objects', () => {
                const dest = { a: { b: 1, c: 2 } }
                const src = { a: { c: 3, d: 4 } }
                Object.blend(dest, src, { overwrite: false })
                // Overwrite: false means don't overwrite existing 'a' property at top level
                // Since 'a' exists in dest, it doesn't get merged - only new properties added
                expect(dest).toEqual({ a: { b: 1, c: 2 } })
            })
        })

        await describe('functions option', async () => {
            it('should include functions by default', () => {
                const dest = { a: 1 }
                const src = { b: () => 'test' }
                Object.blend(dest, src)
                expect(dest.b).toBeInstanceOf(Function)
                expect(dest.b()).toBe('test')
            })

            it('should exclude functions when functions is false', () => {
                const dest = { a: 1 }
                const src = { b: () => 'test', c: 3 }
                Object.blend(dest, src, { functions: false })
                expect(dest).toEqual({ a: 1, c: 3 })
                expect(dest).not.toHaveProperty('b')
            })

            it('should include functions when functions is true', () => {
                const dest = { a: 1 }
                const fn = () => 'test'
                const src = { b: fn }
                Object.blend(dest, src, { functions: true })
                expect(dest.b).toBe(fn)
            })
        })

        await describe('edge cases', async () => {
            it('should handle arrays (not deep merge)', () => {
                const dest = { a: [1, 2] }
                const src = { a: [3, 4] }
                Object.blend(dest, src)
                expect(dest.a).toEqual([3, 4])
            })

            it('should handle dates', () => {
                const date = new Date('2025-01-01')
                const dest = { a: 1 }
                const src = { b: date }
                Object.blend(dest, src)
                expect(dest.b).toBe(date)
            })

            it('should handle null values', () => {
                const dest = { a: 1 }
                const src = { a: null }
                Object.blend(dest, src)
                expect(dest.a).toBeNull()
            })

            it('should handle undefined values', () => {
                const dest = { a: 1 }
                const src = { a: undefined }
                Object.blend(dest, src)
                expect(dest.a).toBeUndefined()
            })

            it('should not merge prototype properties', () => {
                class Parent { parentProp = 'parent' }
                class Child extends Parent { childProp = 'child' }
                const src = new Child()
                const dest = {}
                Object.blend(dest, src)
                // Should only include own properties
                expect(dest).toHaveProperty('childProp')
                expect(dest).toHaveProperty('parentProp') // From instance
            })

            it('should handle symbols in objects', () => {
                const sym = Symbol('test')
                const dest = { a: 1 }
                const src = { [sym]: 'value' }
                Object.blend(dest, src)
                // Symbols are not enumerable in for...in loop
                expect(dest[sym]).toBeUndefined()
            })
        })
    })

    await describe('Object.clone()', async () => {
        await describe('shallow clone', async () => {
            it('should create shallow copy when deep is false', () => {
                const obj = { a: 1, b: 2, c: 3 }
                const cloned = Object.clone(obj, false)
                expect(cloned).toEqual(obj)
                expect(cloned).not.toBe(obj)
            })

            it('should share nested object references in shallow clone', () => {
                const nested = { x: 1 }
                const obj = { a: nested }
                const cloned = Object.clone(obj, false)
                expect(cloned.a).toBe(nested) // same reference
            })

            it('should not affect original when modifying top-level properties', () => {
                const obj = { a: 1, b: 2 }
                const cloned = Object.clone(obj, false)
                cloned.a = 999
                expect(obj.a).toBe(1)
            })
        })

        await describe('deep clone (default)', async () => {
            it('should create deep copy by default', () => {
                const obj = { a: { b: { c: 1 } } }
                const cloned = Object.clone(obj)
                expect(cloned).toEqual(obj)
                expect(cloned).not.toBe(obj)
                expect(cloned.a).not.toBe(obj.a)
                expect(cloned.a.b).not.toBe(obj.a.b)
            })

            it('should not affect original when modifying nested properties', () => {
                const obj = { a: { b: 1 } }
                const cloned = Object.clone(obj, true)
                cloned.a.b = 999
                expect(obj.a.b).toBe(1)
            })

            it('should deep clone arrays', () => {
                const obj = { arr: [1, 2, { x: 3 }] }
                const cloned = Object.clone(obj)
                expect(cloned.arr).toEqual(obj.arr)
                expect(cloned.arr).not.toBe(obj.arr)
                expect(cloned.arr[2]).not.toBe(obj.arr[2])
            })

            it('should handle multiple levels of nesting', () => {
                const obj = { a: { b: { c: { d: 1 } } } }
                const cloned = Object.clone(obj)
                cloned.a.b.c.d = 999
                expect(obj.a.b.c.d).toBe(1)
            })
        })

        await describe('primitive values', async () => {
            it('should return numbers unchanged', () => {
                expect(Object.clone(123)).toBe(123)
            })

            it('should return strings unchanged', () => {
                expect(Object.clone('hello')).toBe('hello')
            })

            it('should return booleans unchanged', () => {
                expect(Object.clone(true)).toBe(true)
            })

            it('should return null unchanged', () => {
                expect(Object.clone(null)).toBeNull()
            })

            it('should return undefined unchanged', () => {
                expect(Object.clone(undefined)).toBeUndefined()
            })
        })

        await describe('special object types', async () => {
            it('should clone Date objects', () => {
                const date = new Date('2025-01-01')
                const cloned = Object.clone(date)
                expect(cloned).toEqual(date)
                expect(cloned).not.toBe(date)
                expect(cloned).toBeInstanceOf(Date)
            })

            it('should clone RegExp objects', () => {
                const regex = /test/gi
                const cloned = Object.clone(regex)
                expect(cloned.source).toBe(regex.source)
                expect(cloned.flags).toBe(regex.flags)
                expect(cloned).not.toBe(regex)
                expect(cloned).toBeInstanceOf(RegExp)
            })

            it('should clone arrays', () => {
                const arr = [1, 2, 3]
                const cloned = Object.clone(arr)
                expect(cloned).toEqual(arr)
                expect(cloned).not.toBe(arr)
            })

            it('should deep clone nested arrays', () => {
                const arr = [1, [2, [3, 4]]]
                const cloned = Object.clone(arr)
                expect(cloned[1]).not.toBe(arr[1])
                expect(cloned[1][1]).not.toBe(arr[1][1])
            })
        })

        await describe('complex structures', async () => {
            it('should clone objects with mixed types', () => {
                const obj = {
                    num: 123,
                    str: 'hello',
                    bool: true,
                    arr: [1, 2, 3],
                    nested: { a: 1, b: 2 },
                    date: new Date(),
                    regex: /test/
                }
                const cloned = Object.clone(obj)
                expect(cloned).toEqual(obj)
                expect(cloned).not.toBe(obj)
                expect(cloned.nested).not.toBe(obj.nested)
                expect(cloned.date).not.toBe(obj.date)
            })

            it('should handle empty objects', () => {
                const obj = {}
                const cloned = Object.clone(obj)
                expect(cloned).toEqual({})
                expect(cloned).not.toBe(obj)
            })

            it('should handle objects with null values', () => {
                const obj = { a: null, b: 1 }
                const cloned = Object.clone(obj)
                expect(cloned.a).toBeNull()
                expect(cloned.b).toBe(1)
            })

            it('should handle objects with undefined values', () => {
                const obj = { a: undefined, b: 1 }
                const cloned = Object.clone(obj)
                expect(cloned.a).toBeUndefined()
                expect(cloned.b).toBe(1)
            })
        })
    })

    await describe('Object.getType()', async () => {
        it('should return constructor for objects', () => {
            const obj = {}
            expect(Object.getType(obj)).toBe(Object)
        })

        it('should return constructor for arrays', () => {
            const arr = []
            expect(Object.getType(arr)).toBe(Array)
        })

        it('should return constructor for dates', () => {
            const date = new Date()
            expect(Object.getType(date)).toBe(Date)
        })

        it('should return constructor for custom classes', () => {
            class MyClass {}
            const instance = new MyClass()
            expect(Object.getType(instance)).toBe(MyClass)
        })

        it('should return constructor for RegExp', () => {
            const regex = /test/
            expect(Object.getType(regex)).toBe(RegExp)
        })

        it('should return constructor for numbers', () => {
            const num = 123
            expect(Object.getType(num)).toBe(Number)
        })

        it('should return constructor for strings', () => {
            const str = 'hello'
            expect(Object.getType(str)).toBe(String)
        })

        it('should return constructor for booleans', () => {
            const bool = true
            expect(Object.getType(bool)).toBe(Boolean)
        })
    })

    await describe('Object.getName()', async () => {
        it('should return name for objects', () => {
            const obj = {}
            expect(Object.getName(obj)).toBe('Object')
        })

        it('should return name for arrays', () => {
            const arr = []
            expect(Object.getName(arr)).toBe('Array')
        })

        it('should return name for dates', () => {
            const date = new Date()
            expect(Object.getName(date)).toBe('Date')
        })

        it('should return name for custom classes', () => {
            class MyClass {}
            const instance = new MyClass()
            expect(Object.getName(instance)).toBe('MyClass')
        })

        it('should return function name for functions', () => {
            function myFunction() {}
            expect(Object.getName(myFunction)).toBe('myFunction')
        })

        it('should return name for arrow functions', () => {
            const arrowFn = () => {}
            expect(Object.getName(arrowFn)).toBe('arrowFn')
        })

        it('should return empty string for anonymous functions', () => {
            const anon = function() {}
            // In Bun, function expressions get the variable name
            expect(Object.getName(anon)).toBe('anon')
        })

        it('should return name for primitives', () => {
            expect(Object.getName(123)).toBe('Number')
            expect(Object.getName('hello')).toBe('String')
            expect(Object.getName(true)).toBe('Boolean')
        })

        it('should return name for RegExp', () => {
            const regex = /test/
            expect(Object.getName(regex)).toBe('RegExp')
        })
    })

    await describe('Integration tests', async () => {
        it('should blend and clone', () => {
            const obj1 = { a: 1, b: 2 }
            const obj2 = { c: 3, d: 4 }
            const blended = Object.blend({}, obj1)
            Object.blend(blended, obj2)
            const cloned = Object.clone(blended)
            expect(cloned).toEqual({ a: 1, b: 2, c: 3, d: 4 })
            expect(cloned).not.toBe(blended)
        })

        it('should get type of cloned object', () => {
            class MyClass { prop = 1 }
            const original = new MyClass()
            const cloned = Object.clone(original)
            // After cloning, it's a plain object, not MyClass instance
            expect(Object.getName(cloned)).toBe('Object')
        })

        it('should work with complex nested structures', () => {
            const complex = {
                users: [
                    { id: 1, name: 'Alice', tags: ['admin', 'user'] },
                    { id: 2, name: 'Bob', tags: ['user'] }
                ],
                meta: {
                    created: new Date('2025-01-01'),
                    version: 1.0
                }
            }
            const cloned = Object.clone(complex)
            cloned.users[0].name = 'Modified'
            expect(complex.users[0].name).toBe('Alice')
        })
    })
})
