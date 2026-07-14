import { describe, it, expect } from '@embedthis/testme'
import '../src/core/types/NumberExtensions'

await describe('Number Extensions', async () => {
    await describe('format()', async () => {
        await describe('with no options', async () => {
            it('should return string representation of number', () => {
                const num = 123
                expect(num.format()).toBe('123')
            })

            it('should handle zero', () => {
                const num = 0
                expect(num.format()).toBe('0')
            })

            it('should handle negative numbers', () => {
                const num = -456
                expect(num.format()).toBe('-456')
            })

            it('should handle decimals in default format', () => {
                const num = 123.456
                expect(num.format()).toBe('123.456')
            })

            it('should handle very large numbers', () => {
                const num = 1234567890
                expect(num.format()).toBe('1234567890')
            })

            it('should handle very small numbers', () => {
                const num = 0.0001
                expect(num.format()).toBe('0.0001')
            })
        })

        await describe('with decimals option', async () => {
            it('should format with specified decimal places', () => {
                const num = 123.456789
                expect(num.format({ decimals: 2 })).toBe('123.46')
            })

            it('should round up correctly', () => {
                const num = 123.456
                expect(num.format({ decimals: 2 })).toBe('123.46')
            })

            it('should round down correctly', () => {
                const num = 123.454
                expect(num.format({ decimals: 2 })).toBe('123.45')
            })

            it('should add trailing zeros', () => {
                const num = 123
                expect(num.format({ decimals: 2 })).toBe('123.00')
            })

            it('should format with zero decimals', () => {
                const num = 123.456
                expect(num.format({ decimals: 0 })).toBe('123')
            })

            it('should handle negative numbers with decimals', () => {
                const num = -123.456
                expect(num.format({ decimals: 2 })).toBe('-123.46')
            })

            it('should format very small decimals', () => {
                const num = 0.001
                expect(num.format({ decimals: 3 })).toBe('0.001')
            })

            it('should handle large decimal precision', () => {
                const num = 123.123456789
                expect(num.format({ decimals: 5 })).toBe('123.12346')
            })

            it('should format zero with decimals', () => {
                const num = 0
                expect(num.format({ decimals: 2 })).toBe('0.00')
            })
        })

        await describe('with thousands option', async () => {
            it('should add thousand separators', () => {
                const num = 1234567
                expect(num.format({ thousands: true })).toBe('1,234,567')
            })

            it('should handle numbers less than 1000', () => {
                const num = 999
                expect(num.format({ thousands: true })).toBe('999')
            })

            it('should add separator at exactly 1000', () => {
                const num = 1000
                expect(num.format({ thousands: true })).toBe('1,000')
            })

            it('should handle negative numbers with thousands', () => {
                const num = -1234567
                expect(num.format({ thousands: true })).toBe('-1,234,567')
            })

            it('should handle millions', () => {
                const num = 1234567890
                expect(num.format({ thousands: true })).toBe('1,234,567,890')
            })

            it('should preserve decimals with thousands', () => {
                const num = 1234.56
                expect(num.format({ thousands: true })).toBe('1,234.56')
            })

            it('should handle zero with thousands option', () => {
                const num = 0
                expect(num.format({ thousands: true })).toBe('0')
            })

            it('should handle numbers just over 1000', () => {
                const num = 1001
                expect(num.format({ thousands: true })).toBe('1,001')
            })

            it('should handle 4-digit numbers', () => {
                const num = 9999
                expect(num.format({ thousands: true })).toBe('9,999')
            })
        })

        await describe('with both decimals and thousands', async () => {
            it('should apply both formatting options', () => {
                const num = 1234567.89
                expect(num.format({ decimals: 2, thousands: true })).toBe('1,234,567.89')
            })

            it('should round and add separators', () => {
                const num = 1234567.891
                expect(num.format({ decimals: 2, thousands: true })).toBe('1,234,567.89')
            })

            it('should add trailing zeros and separators', () => {
                const num = 1234567
                expect(num.format({ decimals: 2, thousands: true })).toBe('1,234,567.00')
            })

            it('should handle negative numbers with both options', () => {
                const num = -1234567.89
                expect(num.format({ decimals: 2, thousands: true })).toBe('-1,234,567.89')
            })

            it('should handle very large numbers with precision', () => {
                const num = 1234567890.12345
                expect(num.format({ decimals: 3, thousands: true })).toBe('1,234,567,890.123')
            })

            it('should handle rounding with thousands', () => {
                const num = 1234.999
                expect(num.format({ decimals: 2, thousands: true })).toBe('1,235.00')
            })

            it('should format small numbers with both options', () => {
                const num = 999.99
                expect(num.format({ decimals: 2, thousands: true })).toBe('999.99')
            })
        })

        await describe('edge cases', async () => {
            it('should handle zero decimals with thousands', () => {
                const num = 1234567.89
                expect(num.format({ decimals: 0, thousands: true })).toBe('1,234,568')
            })

            it('should handle infinity', () => {
                const num = Infinity
                const result = num.format()
                expect(result).toBe('Infinity')
            })

            it('should handle negative infinity', () => {
                const num = -Infinity
                const result = num.format()
                expect(result).toBe('-Infinity')
            })

            it('should handle NaN', () => {
                const num = NaN
                const result = num.format()
                expect(result).toBe('NaN')
            })

            it('should handle very small negative numbers', () => {
                const num = -0.001
                expect(num.format({ decimals: 3 })).toBe('-0.001')
            })

            it('should work with Number objects (not primitives)', () => {
                const num = new Number(1234.56)
                expect(num.format({ decimals: 2 })).toBe('1234.56')
            })

            it('should handle maximum safe integer', () => {
                const num = Number.MAX_SAFE_INTEGER
                const result = num.format({ thousands: true })
                expect(result).toMatch(/,/)
                expect(result.replace(/,/g, '')).toBe(String(Number.MAX_SAFE_INTEGER))
            })

            it('should format numbers close to MaxInt32', () => {
                const num = 2147483647 // MaxInt32
                expect(num.format({ thousands: true })).toBe('2,147,483,647')
            })
        })

        await describe('options validation', async () => {
            it('should handle empty options object', () => {
                const num = 1234.56
                expect(num.format({})).toBe('1234.56')
            })

            it('should handle false thousands option', () => {
                const num = 1234.56
                expect(num.format({ thousands: false })).toBe('1234.56')
            })

            it('should ignore invalid decimals values', () => {
                const num = 123.456
                // With undefined decimals, should use default toString
                expect(num.format({ decimals: undefined })).toBe('123.456')
            })
        })
    })

    await describe('Number.MaxInt32', async () => {
        it('should be defined', () => {
            expect(Number.MaxInt32).toBeDefined()
        })

        it('should equal 2^31 - 1', () => {
            expect(Number.MaxInt32).toBe(2147483647)
            expect(Number.MaxInt32).toBe(Math.pow(2, 31) - 1)
        })

        it('should be read-only', () => {
            const original = Number.MaxInt32
            try {
                // @ts-ignore - trying to write to read-only property
                Number.MaxInt32 = 999
            } catch (e) {
                // Expected to fail in strict mode
            }
            expect(Number.MaxInt32).toBe(original)
        })

        it('should not be enumerable', () => {
            const keys = Object.keys(Number)
            expect(keys).not.toContain('MaxInt32')
        })

        it('should be less than MAX_SAFE_INTEGER', () => {
            expect(Number.MaxInt32).toBeLessThan(Number.MAX_SAFE_INTEGER)
        })

        it('should be exactly 32-bit signed integer max', () => {
            // Max value for signed 32-bit integer
            const expectedMax = 0x7FFFFFFF
            expect(Number.MaxInt32).toBe(expectedMax)
        })

        it('should be usable in comparisons', () => {
            const tooLarge = 2147483648 // MaxInt32 + 1
            const justRight = 2147483647 // MaxInt32
            expect(justRight).toBeLessThanOrEqual(Number.MaxInt32)
            expect(tooLarge).toBeGreaterThan(Number.MaxInt32)
        })
    })

    await describe('Integration tests', async () => {
        it('should work with arithmetic operations', () => {
            const num = (1000 + 234.567)
            expect(num.format({ decimals: 2, thousands: true })).toBe('1,234.57')
        })

        it('should format calculation results', () => {
            const result = (1000 * 1.234567)
            expect(result.format({ decimals: 2 })).toBe('1234.57')
        })

        it('should format division results', () => {
            const result = (10000 / 3)
            expect(result.format({ decimals: 2, thousands: true })).toBe('3,333.33')
        })

        it('should work with Math functions', () => {
            const num = Math.PI * 1000
            expect(num.format({ decimals: 2 })).toBe('3141.59')
        })

        it('should format percentage calculations', () => {
            const percentage = (45.678 / 100)
            expect(percentage.format({ decimals: 4 })).toBe('0.4568')
        })

        it('should compare with MaxInt32', () => {
            const num = Number.MaxInt32
            const formatted = num.format({ thousands: true })
            expect(formatted).toBe('2,147,483,647')
        })

        it('should chain with other number operations', () => {
            const num = 1234.567
            const rounded = Math.round(num)
            expect(rounded.format({ thousands: true })).toBe('1,235')
        })
    })
})
