import { describe, it, expect } from '@embedthis/testme'
import '../src/core/types/ArrayExtensions'

await describe('Array Extensions', async () => {
    await describe('contains()', async () => {
        it('should return true when item exists', () => {
            const arr = [1, 2, 3, 4, 5]
            expect(arr.contains(3)).toBe(true)
        })

        it('should return false when item does not exist', () => {
            const arr = [1, 2, 3, 4, 5]
            expect(arr.contains(6)).toBe(false)
        })

        it('should work with strings', () => {
            const arr = ['apple', 'banana', 'cherry']
            expect(arr.contains('banana')).toBe(true)
            expect(arr.contains('orange')).toBe(false)
        })

        it('should work with objects by reference', () => {
            const obj1 = { id: 1 }
            const obj2 = { id: 2 }
            const arr = [obj1, obj2]
            expect(arr.contains(obj1)).toBe(true)
            expect(arr.contains({ id: 1 })).toBe(false) // different reference
        })

        it('should return false for empty array', () => {
            const arr: number[] = []
            expect(arr.contains(1)).toBe(false)
        })

        it('should find null values', () => {
            const arr = [1, null, 3]
            expect(arr.contains(null)).toBe(true)
        })

        it('should find undefined values', () => {
            const arr = [1, undefined, 3]
            expect(arr.contains(undefined)).toBe(true)
        })

        it('should distinguish between null and undefined', () => {
            const arr = [null]
            expect(arr.contains(null)).toBe(true)
            expect(arr.contains(undefined)).toBe(false)
        })
    })

    await describe('unique()', async () => {
        it('should remove duplicate numbers', () => {
            const arr = [1, 2, 2, 3, 1, 4, 3]
            const result = arr.unique()
            expect(result).toEqual([1, 2, 3, 4])
        })

        it('should remove duplicate strings', () => {
            const arr = ['a', 'b', 'a', 'c', 'b']
            const result = arr.unique()
            expect(result).toEqual(['a', 'b', 'c'])
        })

        it('should handle empty array', () => {
            const arr: number[] = []
            const result = arr.unique()
            expect(result).toEqual([])
        })

        it('should handle array with all unique elements', () => {
            const arr = [1, 2, 3, 4, 5]
            const result = arr.unique()
            expect(result).toEqual([1, 2, 3, 4, 5])
        })

        it('should handle array with all duplicate elements', () => {
            const arr = [1, 1, 1, 1]
            const result = arr.unique()
            expect(result).toEqual([1])
        })

        it('should preserve order of first occurrence', () => {
            const arr = [3, 1, 2, 1, 3, 2]
            const result = arr.unique()
            expect(result).toEqual([3, 1, 2])
        })

        it('should not mutate original array', () => {
            const arr = [1, 2, 2, 3]
            const original = [...arr]
            arr.unique()
            expect(arr).toEqual(original)
        })

        it('should handle mixed types', () => {
            const arr = [1, '1', 2, '2', 1, 2]
            const result = arr.unique()
            expect(result).toEqual([1, '1', 2, '2'])
        })

        it('should handle objects by reference', () => {
            const obj1 = { id: 1 }
            const obj2 = { id: 2 }
            const arr = [obj1, obj2, obj1, obj2]
            const result = arr.unique()
            expect(result).toEqual([obj1, obj2])
        })
    })

    await describe('append()', async () => {
        it('should append single item', () => {
            const arr = [1, 2, 3]
            const result = arr.append(4)
            expect(arr).toEqual([1, 2, 3, 4])
            expect(result).toBe(arr) // returns same array reference
        })

        it('should append array of items', () => {
            const arr = [1, 2, 3]
            arr.append([4, 5, 6])
            expect(arr).toEqual([1, 2, 3, 4, 5, 6])
        })

        it('should return this for chaining', () => {
            const arr = [1]
            const result = arr.append(2).append(3).append(4)
            expect(arr).toEqual([1, 2, 3, 4])
            expect(result).toBe(arr)
        })

        it('should chain array appends', () => {
            const arr = [1]
            arr.append([2, 3]).append([4, 5])
            expect(arr).toEqual([1, 2, 3, 4, 5])
        })

        it('should append to empty array', () => {
            const arr: number[] = []
            arr.append(1)
            expect(arr).toEqual([1])
        })

        it('should append empty array', () => {
            const arr = [1, 2]
            arr.append([])
            expect(arr).toEqual([1, 2])
        })

        it('should append different types', () => {
            const arr: any[] = [1, 'two']
            arr.append(true)
            expect(arr).toEqual([1, 'two', true])
        })

        it('should append null', () => {
            const arr = [1, 2]
            arr.append(null)
            expect(arr).toEqual([1, 2, null])
        })

        it('should append undefined', () => {
            const arr = [1, 2]
            arr.append(undefined)
            expect(arr).toEqual([1, 2, undefined])
        })

        it('should mutate original array', () => {
            const arr = [1, 2, 3]
            const original = arr
            arr.append(4)
            expect(original).toEqual([1, 2, 3, 4])
        })
    })

    await describe('transform()', async () => {
        it('should transform array elements', () => {
            const arr = [1, 2, 3]
            const result = arr.transform(x => x * 2)
            expect(result).toEqual([2, 4, 6])
        })

        it('should receive index parameter', () => {
            const arr = ['a', 'b', 'c']
            const result = arr.transform((item, index) => `${index}:${item}`)
            expect(result).toEqual(['0:a', '1:b', '2:c'])
        })

        it('should receive array parameter', () => {
            const arr = [1, 2, 3]
            const result = arr.transform((item, index, array) => {
                return item + array.length
            })
            expect(result).toEqual([4, 5, 6])
        })

        it('should mutate original array in-place', () => {
            const arr = [1, 2, 3]
            arr.transform(x => x * 2)
            expect(arr).toEqual([2, 4, 6])
        })

        it('should handle empty array', () => {
            const arr: number[] = []
            const result = arr.transform(x => x * 2)
            expect(result).toEqual([])
        })

        it('should transform to different type', () => {
            const arr = [1, 2, 3]
            const result = arr.transform(x => String(x))
            expect(result).toEqual(['1', '2', '3'])
        })

        it('should work with complex transformations', () => {
            const arr = [{ id: 1, name: 'Alice' }, { id: 2, name: 'Bob' }]
            const result = arr.transform(obj => obj.name.toUpperCase())
            expect(result).toEqual(['ALICE', 'BOB'])
        })
    })

    await describe('clone()', async () => {
        it('should create shallow copy of array', () => {
            const arr = [1, 2, 3, 4, 5]
            const cloned = arr.clone()
            expect(cloned).toEqual(arr)
            expect(cloned).not.toBe(arr) // different reference
        })

        it('should not affect original when modifying clone', () => {
            const arr = [1, 2, 3]
            const cloned = arr.clone()
            cloned.push(4)
            expect(arr).toEqual([1, 2, 3])
            expect(cloned).toEqual([1, 2, 3, 4])
        })

        it('should clone empty array', () => {
            const arr: number[] = []
            const cloned = arr.clone()
            expect(cloned).toEqual([])
            expect(cloned).not.toBe(arr)
        })

        it('should be deep copy by default (objects not shared)', () => {
            const obj = { id: 1 }
            const arr = [obj]
            const cloned = arr.clone()
            expect(cloned[0]).not.toBe(obj) // different reference (deep clone)
            obj.id = 2
            expect(cloned[0].id).toBe(1) // mutation does NOT affect clone
        })

        it('should clone array of strings', () => {
            const arr = ['a', 'b', 'c']
            const cloned = arr.clone()
            expect(cloned).toEqual(arr)
            expect(cloned).not.toBe(arr)
        })

        it('should clone array with mixed types', () => {
            const arr = [1, 'two', true, null, undefined]
            const cloned = arr.clone()
            expect(cloned).toEqual(arr)
            expect(cloned).not.toBe(arr)
        })

        it('should deep clone nested arrays by default', () => {
            const nested = [1, 2, 3]
            const arr = [nested]
            const cloned = arr.clone()
            expect(cloned[0]).not.toBe(nested) // different reference (deep clone)
            nested.push(4)
            expect(cloned[0]).toEqual([1, 2, 3]) // cloned array unchanged
        })

        it('should shallow clone when deep=false', () => {
            const nested = [1, 2, 3]
            const arr = [nested]
            const cloned = arr.clone(false)
            expect(cloned[0]).toBe(nested) // same reference (shallow clone)
            nested.push(4)
            expect(cloned[0]).toEqual([1, 2, 3, 4]) // reflects changes
        })

        it('should deep clone nested objects', () => {
            const obj = { id: 1 }
            const arr = [obj]
            const cloned = arr.clone(true)
            expect(cloned[0]).not.toBe(obj) // different reference
            expect(cloned[0]).toEqual({ id: 1 })
        })
    })

    await describe('clear()', async () => {
        it('should remove all elements', () => {
            const arr = [1, 2, 3, 4, 5]
            arr.clear()
            expect(arr).toEqual([])
            expect(arr.length).toBe(0)
        })

        it('should work on empty array', () => {
            const arr: number[] = []
            arr.clear()
            expect(arr).toEqual([])
        })
    })

    await describe('compact()', async () => {
        it('should remove null elements', () => {
            const arr = [1, null, 2, null, 3]
            arr.compact()
            expect(arr).toEqual([1, 2, 3])
        })

        it('should remove undefined elements', () => {
            const arr = [1, undefined, 2, undefined, 3]
            arr.compact()
            expect(arr).toEqual([1, 2, 3])
        })

        it('should remove both null and undefined', () => {
            const arr = [1, null, 2, undefined, 3, null]
            arr.compact()
            expect(arr).toEqual([1, 2, 3])
        })

        it('should keep falsy values that are not null/undefined', () => {
            const arr = [0, false, '', null, undefined]
            arr.compact()
            expect(arr).toEqual([0, false, ''])
        })

        it('should return the same array reference', () => {
            const arr = [1, null, 2]
            const result = arr.compact()
            expect(result).toBe(arr)
        })
    })

    await describe('remove()', async () => {
        it('should remove elements from start to end', () => {
            const arr = [1, 2, 3, 4, 5]
            arr.remove(1, 3)
            expect(arr).toEqual([1, 5])
        })

        it('should remove to end with default end parameter', () => {
            const arr = [1, 2, 3, 4, 5]
            arr.remove(2)
            expect(arr).toEqual([1, 2])
        })

        it('should handle negative start index', () => {
            const arr = [1, 2, 3, 4, 5]
            arr.remove(-2, -1)
            expect(arr).toEqual([1, 2, 3])
        })

        it('should handle negative end index', () => {
            const arr = [1, 2, 3, 4, 5]
            arr.remove(1, -2)
            expect(arr).toEqual([1, 5])
        })

        it('should remove single element', () => {
            const arr = [1, 2, 3]
            arr.remove(1, 1)
            expect(arr).toEqual([1, 3])
        })
    })

    await describe('findAll()', async () => {
        it('should find all matching elements', () => {
            const arr = [1, 2, 3, 4, 5]
            const result = arr.findAll(x => x > 2)
            expect(result).toEqual([3, 4, 5])
        })

        it('should return empty array when no matches', () => {
            const arr = [1, 2, 3]
            const result = arr.findAll(x => x > 10)
            expect(result).toEqual([])
        })

        it('should work with index parameter', () => {
            const arr = ['a', 'b', 'c']
            const result = arr.findAll((item, index) => index > 0)
            expect(result).toEqual(['b', 'c'])
        })

        it('should be an alias for filter', () => {
            const arr = [1, 2, 3, 4, 5]
            const predicate = (x: number) => x % 2 === 0
            expect(arr.findAll(predicate)).toEqual(arr.filter(predicate))
        })
    })

    await describe('reject()', async () => {
        it('should return elements that do NOT match', () => {
            const arr = [1, 2, 3, 4, 5]
            const result = arr.reject(x => x > 2)
            expect(result).toEqual([1, 2])
        })

        it('should be opposite of filter', () => {
            const arr = [1, 2, 3, 4, 5]
            const predicate = (x: number) => x % 2 === 0
            const filtered = arr.filter(predicate)
            const rejected = arr.reject(predicate)
            expect(filtered.concat(rejected).sort()).toEqual(arr.sort())
        })

        it('should return all elements when none match', () => {
            const arr = [1, 2, 3]
            const result = arr.reject(x => x > 10)
            expect(result).toEqual([1, 2, 3])
        })

        it('should return empty array when all match', () => {
            const arr = [1, 2, 3]
            const result = arr.reject(x => x > 0)
            expect(result).toEqual([])
        })

        it('should work with index parameter', () => {
            const arr = ['a', 'b', 'c']
            const result = arr.reject((item, index) => index === 1)
            expect(result).toEqual(['a', 'c'])
        })
    })

    await describe('Integration tests', async () => {
        it('should work with method chaining', () => {
            const arr = [1, 2, 2, 3, 3, 3]
            const result = arr.unique().transform(x => x * 2)
            expect(result).toEqual([2, 4, 6])
        })

        it('should combine append and unique', () => {
            const arr = [1, 2, 3]
            arr.append([2, 3, 4])
            const unique = arr.unique()
            expect(unique).toEqual([1, 2, 3, 4])
        })

        it('should clone then modify', () => {
            const arr = [1, 2, 3]
            const cloned = arr.clone()
            cloned.append(4).append(5)
            expect(arr).toEqual([1, 2, 3])
            expect(cloned).toEqual([1, 2, 3, 4, 5])
        })

        it('should work with ES6 includes for comparison', () => {
            const arr = [1, 2, 3]
            // contains() and includes() should behave the same
            expect(arr.contains(2)).toBe(arr.includes(2))
            expect(arr.contains(5)).toBe(arr.includes(5))
        })

        it('should chain compact, reject, and transform', () => {
            const arr = [1, null, 2, 3, null, 4, 5]
            arr.compact() // removes nulls: [1, 2, 3, 4, 5]
            const result = arr.reject(x => x % 2 === 0) // removes evens: [1, 3, 5]
            result.transform(x => x * 2) // doubles: [2, 6, 10]
            expect(result).toEqual([2, 6, 10])
        })
    })
})
