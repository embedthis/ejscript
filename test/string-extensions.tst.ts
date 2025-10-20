/**
 * String extensions tests - Complete Ejscript API coverage
 */

import { test, expect, describe } from 'testme'
import '../src/core/types/StringExtensions'

await describe('String Extensions', async () => {
    // ========================================================================
    // Comparison Methods
    // ========================================================================

    await describe('caseCompare()', async () => {
        test('should return -1 when this < compare', () => {
            expect('abc'.caseCompare('def')).toBe(-1)
        })

        test('should return 0 when strings are equal', () => {
            expect('abc'.caseCompare('abc')).toBe(0)
        })

        test('should return 1 when this > compare', () => {
            expect('def'.caseCompare('abc')).toBe(1)
        })

        test('should be case-sensitive', () => {
            expect('ABC'.caseCompare('abc')).toBe(-1) // 'A' < 'a' in ASCII
        })
    })

    await describe('caselessCompare()', async () => {
        test('should return -1 when this < compare (case-insensitive)', () => {
            expect('abc'.caselessCompare('DEF')).toBe(-1)
        })

        test('should return 0 when strings are equal (case-insensitive)', () => {
            expect('ABC'.caselessCompare('abc')).toBe(0)
        })

        test('should return 1 when this > compare (case-insensitive)', () => {
            expect('DEF'.caselessCompare('abc')).toBe(1)
        })

        test('should ignore case differences', () => {
            expect('Hello'.caselessCompare('HELLO')).toBe(0)
            expect('Hello'.caselessCompare('hello')).toBe(0)
        })
    })

    // ========================================================================
    // Search Methods
    // ========================================================================

    await describe('contains()', async () => {
        test('should check if string contains substring', () => {
            expect('hello world'.contains('world')).toBe(true)
            expect('hello world'.contains('foo')).toBe(false)
        })

        test('should work with empty string', () => {
            expect('hello'.contains('')).toBe(true)
        })

        test('should work with regex pattern', () => {
            expect('hello world'.contains(/wor.d/)).toBe(true)
            expect('hello world'.contains(/foo/)).toBe(false)
        })

        test('should be case-sensitive with string', () => {
            expect('Hello World'.contains('world')).toBe(false)
            expect('Hello World'.contains('World')).toBe(true)
        })
    })

    // ========================================================================
    // Character Type Checks
    // ========================================================================

    await describe('isDigit property', async () => {
        test('should return true for numeric strings', () => {
            expect('123'.isDigit).toBe(true)
            expect('0'.isDigit).toBe(true)
            expect('999'.isDigit).toBe(true)
        })

        test('should return false for non-numeric strings', () => {
            expect('abc'.isDigit).toBe(false)
            expect('12a'.isDigit).toBe(false)
            expect('1.23'.isDigit).toBe(false)
        })

        test('should return false for empty string', () => {
            expect(''.isDigit).toBe(false)
        })
    })

    await describe('isAlpha property', async () => {
        test('should return true for alphabetic strings', () => {
            expect('abc'.isAlpha).toBe(true)
            expect('ABC'.isAlpha).toBe(true)
            expect('AbCdEf'.isAlpha).toBe(true)
        })

        test('should return false for non-alphabetic strings', () => {
            expect('123'.isAlpha).toBe(false)
            expect('abc123'.isAlpha).toBe(false)
            expect('abc!'.isAlpha).toBe(false)
        })

        test('should return false for empty string', () => {
            expect(''.isAlpha).toBe(false)
        })
    })

    await describe('isAlphaNum property', async () => {
        test('should return true for alphanumeric strings', () => {
            expect('abc123'.isAlphaNum).toBe(true)
            expect('123'.isAlphaNum).toBe(true)
            expect('abc'.isAlphaNum).toBe(true)
        })

        test('should return false for non-alphanumeric strings', () => {
            expect('abc!'.isAlphaNum).toBe(false)
            expect('123.456'.isAlphaNum).toBe(false)
            expect('hello world'.isAlphaNum).toBe(false)
        })

        test('should return false for empty string', () => {
            expect(''.isAlphaNum).toBe(false)
        })
    })

    await describe('isLower property', async () => {
        test('should return true for lowercase strings', () => {
            expect('abc'.isLower).toBe(true)
            expect('hello world'.isLower).toBe(true)
        })

        test('should return false for uppercase or mixed case', () => {
            expect('ABC'.isLower).toBe(false)
            expect('Hello'.isLower).toBe(false)
        })

        test('should return false for strings without letters', () => {
            expect('123'.isLower).toBe(false)
        })

        test('should return false for empty string', () => {
            expect(''.isLower).toBe(false)
        })
    })

    await describe('isSpace property', async () => {
        test('should return true for whitespace-only strings', () => {
            expect('   '.isSpace).toBe(true)
            expect('\t'.isSpace).toBe(true)
            expect('\n'.isSpace).toBe(true)
            expect(' \t\n '.isSpace).toBe(true)
        })

        test('should return false for non-whitespace strings', () => {
            expect('abc'.isSpace).toBe(false)
            expect('  a  '.isSpace).toBe(false)
        })

        test('should return false for empty string', () => {
            expect(''.isSpace).toBe(false)
        })
    })

    await describe('isUpper property', async () => {
        test('should return true for uppercase strings', () => {
            expect('ABC'.isUpper).toBe(true)
            expect('HELLO WORLD'.isUpper).toBe(true)
        })

        test('should return false for lowercase or mixed case', () => {
            expect('abc'.isUpper).toBe(false)
            expect('Hello'.isUpper).toBe(false)
        })

        test('should return false for strings without letters', () => {
            expect('123'.isUpper).toBe(false)
        })

        test('should return false for empty string', () => {
            expect(''.isUpper).toBe(false)
        })
    })

    // ========================================================================
    // Formatting and Manipulation
    // ========================================================================

    await describe('format()', async () => {
        test('should format with %s (string)', () => {
            expect('%s world'.format('Hello')).toBe('Hello world')
        })

        test('should format with %d (integer)', () => {
            expect('Number: %d'.format(42)).toBe('Number: 42')
        })

        test('should format with %f (float)', () => {
            expect('Value: %f'.format(3.14159)).toBe('Value: 3.14159')
        })

        test('should format with precision', () => {
            expect('Value: %.2f'.format(3.14159)).toBe('Value: 3.14')
        })

        test('should handle multiple arguments', () => {
            expect('%s %d %f'.format('Hello', 42, 3.14)).toBe('Hello 42 3.14')
        })

        test('should handle insufficient arguments', () => {
            expect('%s %d'.format('Hello')).toBe('Hello %d')
        })
    })

    await describe('expand()', async () => {
        test('should expand simple variables', () => {
            const template = 'Hello ${name}, you are ${age} years old'
            const result = template.expand({ name: 'Alice', age: 30 })
            expect(result).toBe('Hello Alice, you are 30 years old')
        })

        test('should expand nested properties', () => {
            const template = 'User: ${user.name}, Email: ${user.email}'
            const result = template.expand({
                user: { name: 'Bob', email: 'bob@example.com' }
            })
            expect(result).toBe('User: Bob, Email: bob@example.com')
        })

        test('should handle arrays with default join', () => {
            const template = 'Items: ${items}'
            const result = template.expand({ items: ['a', 'b', 'c'] })
            expect(result).toBe('Items: a b c')
        })

        test('should handle arrays with custom join', () => {
            const template = 'Items: ${items}'
            const result = template.expand({ items: ['a', 'b', 'c'] }, { join: ', ' })
            expect(result).toBe('Items: a, b, c')
        })

        test('should throw on missing property by default', () => {
            expect(() => {
                '${missing}'.expand({})
            }).toThrow('Missing property: missing')
        })

        test('should preserve token when fill=true', () => {
            expect('${missing}'.expand({}, { fill: true })).toBe('${missing}')
        })

        test('should remove token when fill=false', () => {
            expect('${missing}'.expand({}, { fill: false })).toBe('')
        })

        test('should use fill string', () => {
            expect('${missing}'.expand({}, { fill: 'N/A' })).toBe('N/A')
        })

        test('should handle escaped tokens', () => {
            expect('$${preserved}'.expand({})).toBe('${preserved}')
        })
    })

    await describe('printable()', async () => {
        test('should convert nonprintable to unicode escapes', () => {
            expect('\x00\x01\x02'.printable()).toBe('\\u0000\\u0001\\u0002')
        })

        test('should preserve printable ASCII', () => {
            expect('Hello World'.printable()).toBe('Hello World')
        })

        test('should convert unicode to escapes', () => {
            const result = '©'.printable()
            expect(result).toMatch(/\\u00a9/i)
        })
    })

    await describe('quote()', async () => {
        test('should wrap string in double quotes', () => {
            expect('hello'.quote()).toBe('"hello"')
        })

        test('should work with empty string', () => {
            expect(''.quote()).toBe('""')
        })

        test('should work with already quoted string', () => {
            expect('"hello"'.quote()).toBe('""hello""')
        })
    })

    await describe('remove()', async () => {
        test('should remove characters from start to end', () => {
            expect('hello world'.remove(5, 11)).toBe('hello')
        })

        test('should handle negative start index', () => {
            expect('hello world'.remove(-6, 11)).toBe('hello')
        })

        test('should handle negative end index', () => {
            expect('hello world'.remove(0, -6)).toBe(' world')
        })

        test('should remove to end when end is -1', () => {
            expect('hello world'.remove(5)).toBe('hello')
        })
    })

    await describe('reverse()', async () => {
        test('should reverse string', () => {
            expect('hello'.reverse()).toBe('olleh')
        })

        test('should work with empty string', () => {
            expect(''.reverse()).toBe('')
        })

        test('should work with single character', () => {
            expect('a'.reverse()).toBe('a')
        })
    })

    await describe('times()', async () => {
        test('should repeat string N times', () => {
            expect('ab'.times(3)).toBe('ababab')
        })

        test('should return empty string for 0', () => {
            expect('ab'.times(0)).toBe('')
        })

        test('should return empty string for negative', () => {
            expect('ab'.times(-1)).toBe('')
        })

        test('should work with single repetition', () => {
            expect('ab'.times(1)).toBe('ab')
        })
    })

    await describe('tokenize()', async () => {
        test('should tokenize based on format specifiers', () => {
            const result = 'one two three'.tokenize('%s %s %s')
            expect(result).toEqual(['one', 'two', 'three'])
        })

        test('should limit tokens to format specifier count', () => {
            const result = 'one two three four'.tokenize('%s %s')
            expect(result).toEqual(['one', 'two'])
        })

        test('should handle extra whitespace', () => {
            const result = '  one   two  '.tokenize('%s %s')
            expect(result).toEqual(['one', 'two'])
        })
    })

    // ========================================================================
    // Case Conversion
    // ========================================================================

    await describe('toPascal()', async () => {
        test('should capitalize first letter', () => {
            expect('hello'.toPascal()).toBe('Hello')
        })

        test('should preserve already Pascal case', () => {
            expect('Hello'.toPascal()).toBe('Hello')
        })

        test('should handle leading whitespace', () => {
            expect('  hello'.toPascal()).toBe('  Hello')
        })

        test('should work with empty string', () => {
            expect(''.toPascal()).toBe('')
        })
    })

    await describe('toCamel()', async () => {
        test('should lowercase first letter', () => {
            expect('Hello'.toCamel()).toBe('hello')
        })

        test('should preserve already camel case', () => {
            expect('hello'.toCamel()).toBe('hello')
        })

        test('should handle leading whitespace', () => {
            expect('  Hello'.toCamel()).toBe('  hello')
        })

        test('should work with empty string', () => {
            expect(''.toCamel()).toBe('')
        })
    })

    await describe('capitalize()', async () => {
        test('should capitalize first letter', () => {
            expect('hello'.capitalize()).toBe('Hello')
        })

        test('should preserve rest of string', () => {
            expect('hELLO'.capitalize()).toBe('HELLO')
        })
    })

    // ========================================================================
    // Enhanced Trimming
    // ========================================================================

    await describe('trim() with custom string', async () => {
        test('should trim whitespace by default', () => {
            expect('  hello  '.trim()).toBe('hello')
        })

        test('should trim custom substring', () => {
            expect('xxxhelloxxx'.trim('x')).toBe('hello')
        })

        test('should handle multiple occurrences', () => {
            expect('xxxxhelloxxxx'.trim('xx')).toBe('hello')
        })
    })

    await describe('trimStart() with custom string', async () => {
        test('should trim whitespace from start by default', () => {
            expect('  hello  '.trimStart()).toBe('hello  ')
        })

        test('should trim custom substring from start', () => {
            expect('xxxhelloxxx'.trimStart('x')).toBe('helloxxx')
        })
    })

    await describe('trimEnd() with custom string', async () => {
        test('should trim whitespace from end by default', () => {
            expect('  hello  '.trimEnd()).toBe('  hello')
        })

        test('should trim custom substring from end', () => {
            expect('xxxhelloxxx'.trimEnd('x')).toBe('xxxhello')
        })
    })

    // ========================================================================
    // Conversion
    // ========================================================================

    await describe('parseJSON()', async () => {
        test('should parse JSON string', () => {
            const obj = '{"name":"Alice","age":30}'.parseJSON()
            expect(obj).toEqual({ name: 'Alice', age: 30 })
        })

        test('should parse JSON array', () => {
            const arr = '[1,2,3]'.parseJSON()
            expect(arr).toEqual([1, 2, 3])
        })

        test('should apply filter if provided', () => {
            const obj = '{"a":1,"b":2}'.parseJSON((key: string, value: any) => {
                return key === 'a' ? undefined : value
            })
            expect(obj).toEqual({ b: 2 })
        })

        test('should throw on invalid JSON', () => {
            expect(() => {
                'invalid'.parseJSON()
            }).toThrow()
        })
    })

    await describe('toJSON()', async () => {
        test('should convert string to JSON', () => {
            expect('hello'.toJSON()).toBe('"hello"')
        })

        test('should escape special characters', () => {
            expect('hello\nworld'.toJSON()).toContain('\\n')
        })
    })

    await describe('toPath()', async () => {
        test('should convert string to Path object', () => {
            const path = '/tmp/test'.toPath()
            expect(path).toBeDefined()
            expect(path.toString()).toBe('/tmp/test')
        })
    })
})
