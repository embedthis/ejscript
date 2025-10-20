import { describe, it, expect, beforeEach } from 'testme'
import '../src/core/types/DateExtensions'

await describe('Date Extensions', async () => {
    await describe('elapsed property', async () => {
        it('should return elapsed time in milliseconds', async () => {
            const past = new Date(Date.now() - 1000)
            const elapsed = past.elapsed
            expect(elapsed).toBeGreaterThanOrEqual(1000)
            expect(elapsed).toBeLessThan(1100) // allow some wiggle room
        })

        it('should be negative for future dates', () => {
            const future = new Date(Date.now() + 1000)
            const elapsed = future.elapsed
            expect(elapsed).toBeLessThanOrEqual(-1000)
            expect(elapsed).toBeGreaterThan(-1100)
        })

        it('should be approximately zero for current time', () => {
            const now = new Date()
            const elapsed = now.elapsed
            expect(Math.abs(elapsed)).toBeLessThan(10) // within 10ms
        })

        it('should increase over time', async () => {
            const d = new Date(Date.now() - 100)
            const elapsed1 = d.elapsed
            await new Promise(resolve => setTimeout(resolve, 50))
            const elapsed2 = d.elapsed
            expect(elapsed2).toBeGreaterThan(elapsed1)
        })

        it('should work with very old dates', () => {
            const old = new Date('2000-01-01')
            const elapsed = old.elapsed
            expect(elapsed).toBeGreaterThan(0)
            expect(elapsed).toBeGreaterThan(1000 * 60 * 60 * 24 * 365 * 20) // > 20 years
        })

        it('should be read-only property', () => {
            const d = new Date()
            const elapsed = d.elapsed
            expect(typeof elapsed).toBe('number')
            // Property should be read-only (can't set it)
        })
    })

    await describe('format()', async () => {
        let testDate: Date

        beforeEach(() => {
            // Create a known date: 2025-10-17 14:30:45
            testDate = new Date(2025, 9, 17, 14, 30, 45) // month is 0-indexed
        })

        it('should return ISO string when no format provided', () => {
            const result = testDate.format()
            expect(result).toMatch(/^\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}/)
        })

        it('should format with yyyy (full year)', () => {
            const result = testDate.format('yyyy')
            expect(result).toBe('2025')
        })

        it('should format with yy (2-digit year)', () => {
            const result = testDate.format('yy')
            expect(result).toBe('25')
        })

        it('should format with MM (2-digit month)', () => {
            const result = testDate.format('MM')
            expect(result).toBe('10')
        })

        it('should format with dd (2-digit day)', () => {
            const result = testDate.format('dd')
            expect(result).toBe('17')
        })

        it('should format with HH (2-digit hour)', () => {
            const result = testDate.format('HH')
            expect(result).toBe('14')
        })

        it('should format with mm (2-digit minute)', () => {
            const result = testDate.format('mm')
            expect(result).toBe('30')
        })

        it('should format with ss (2-digit second)', () => {
            const result = testDate.format('ss')
            expect(result).toBe('45')
        })

        it('should format date as yyyy-MM-dd', () => {
            const result = testDate.format('yyyy-MM-dd')
            expect(result).toBe('2025-10-17')
        })

        it('should format time as HH:mm:ss', () => {
            const result = testDate.format('HH:mm:ss')
            expect(result).toBe('14:30:45')
        })

        it('should format full datetime', () => {
            const result = testDate.format('yyyy-MM-dd HH:mm:ss')
            expect(result).toBe('2025-10-17 14:30:45')
        })

        it('should pad single digit months', () => {
            const d = new Date(2025, 0, 15) // January
            const result = d.format('MM')
            expect(result).toBe('01')
        })

        it('should pad single digit days', () => {
            const d = new Date(2025, 9, 5)
            const result = d.format('dd')
            expect(result).toBe('05')
        })

        it('should pad single digit hours', () => {
            const d = new Date(2025, 9, 17, 9, 30, 45)
            const result = d.format('HH')
            expect(result).toBe('09')
        })

        it('should pad single digit minutes', () => {
            const d = new Date(2025, 9, 17, 14, 5, 45)
            const result = d.format('mm')
            expect(result).toBe('05')
        })

        it('should pad single digit seconds', () => {
            const d = new Date(2025, 9, 17, 14, 30, 5)
            const result = d.format('ss')
            expect(result).toBe('05')
        })

        it('should handle custom format strings', () => {
            const result = testDate.format('Date: yyyy/MM/dd Time: HH:mm')
            expect(result).toBe('Date: 2025/10/17 Time: 14:30')
        })

        it('should handle format with text and punctuation', () => {
            const result = testDate.format('yyyy.MM.dd_HH-mm-ss')
            expect(result).toBe('2025.10.17_14-30-45')
        })

        it('should handle repeated format tokens', () => {
            const result = testDate.format('yyyy yyyy MM MM')
            expect(result).toBe('2025 2025 10 10')
        })

        it('should work with midnight time', () => {
            const d = new Date(2025, 9, 17, 0, 0, 0)
            const result = d.format('HH:mm:ss')
            expect(result).toBe('00:00:00')
        })

        it('should work with noon time', () => {
            const d = new Date(2025, 9, 17, 12, 0, 0)
            const result = d.format('HH:mm:ss')
            expect(result).toBe('12:00:00')
        })

        it('should work with end of day', () => {
            const d = new Date(2025, 9, 17, 23, 59, 59)
            const result = d.format('HH:mm:ss')
            expect(result).toBe('23:59:59')
        })
    })

    await describe('future()', async () => {
        it('should return date in the future by milliseconds', () => {
            const now = new Date()
            const future = now.future(1000)
            expect(future.getTime() - now.getTime()).toBe(1000)
        })

        it('should return new Date object', () => {
            const now = new Date()
            const future = now.future(1000)
            expect(future).not.toBe(now)
            expect(future).toBeInstanceOf(Date)
        })

        it('should not mutate original date', () => {
            const now = new Date()
            const original = now.getTime()
            now.future(1000)
            expect(now.getTime()).toBe(original)
        })

        it('should work with large time offsets', () => {
            const now = new Date()
            const oneDay = 1000 * 60 * 60 * 24
            const future = now.future(oneDay)
            expect(future.getTime() - now.getTime()).toBe(oneDay)
        })

        it('should work with negative offsets (past)', () => {
            const now = new Date()
            const past = now.future(-1000)
            expect(now.getTime() - past.getTime()).toBe(1000)
        })

        it('should work with zero offset', () => {
            const now = new Date()
            const same = now.future(0)
            expect(same.getTime()).toBe(now.getTime())
        })

        it('should chain correctly', () => {
            const now = new Date()
            const future = now.future(1000).future(1000)
            expect(future.getTime() - now.getTime()).toBe(2000)
        })

        it('should work with fractional milliseconds', () => {
            const now = new Date()
            const future = now.future(500.5)
            const diff = future.getTime() - now.getTime()
            expect(diff).toBeGreaterThanOrEqual(500)
            expect(diff).toBeLessThanOrEqual(501)
        })

        it('should handle very large offsets', () => {
            const now = new Date()
            const oneYear = 1000 * 60 * 60 * 24 * 365
            const future = now.future(oneYear)
            expect(future.getTime() - now.getTime()).toBe(oneYear)
        })
    })

    await describe('Date.parseUTCDate()', async () => {
        it('should parse ISO date string', () => {
            const date = Date.parseUTCDate('2025-10-17T14:30:45Z')
            expect(date).toBeInstanceOf(Date)
            expect(date.getUTCFullYear()).toBe(2025)
            expect(date.getUTCMonth()).toBe(9) // October (0-indexed)
            expect(date.getUTCDate()).toBe(17)
        })

        it('should parse date without time', () => {
            const date = Date.parseUTCDate('2025-10-17')
            expect(date.getUTCFullYear()).toBe(2025)
            expect(date.getUTCMonth()).toBe(9)
            expect(date.getUTCDate()).toBe(17)
        })

        it('should parse date with timezone', () => {
            const date = Date.parseUTCDate('2025-10-17T14:30:45+00:00')
            expect(date).toBeInstanceOf(Date)
            expect(date.getUTCFullYear()).toBe(2025)
        })

        it('should handle various date formats', () => {
            const date1 = Date.parseUTCDate('October 17, 2025')
            const date2 = Date.parseUTCDate('10/17/2025')
            expect(date1).toBeInstanceOf(Date)
            expect(date2).toBeInstanceOf(Date)
        })

        it('should return Invalid Date for invalid strings', () => {
            const date = Date.parseUTCDate('invalid date string')
            expect(isNaN(date.getTime())).toBe(true)
        })

        it('should parse timestamp strings', () => {
            const timestamp = Date.now()
            // Date constructor expects ISO string or number, not string of number
            const date = Date.parseUTCDate(new Date(timestamp).toISOString())
            expect(date.getTime()).toBeGreaterThan(0)
        })
    })

    await describe('toUTCString()', async () => {
        it('should return UTC string representation', () => {
            const date = new Date('2025-10-17T14:30:45Z')
            const utcStr = date.toUTCString()
            expect(typeof utcStr).toBe('string')
            expect(utcStr).toMatch(/GMT/)
        })

        it('should be consistent with native Date.prototype.toUTCString', () => {
            const date = new Date()
            const utcStr = date.toUTCString()
            expect(utcStr).toBe(Date.prototype.toUTCString.call(date))
        })
    })

    await describe('Integration tests', async () => {
        it('should work with elapsed and future together', () => {
            const now = new Date()
            const future = now.future(5000)
            expect(future.elapsed).toBeLessThan(0) // future date has negative elapsed
        })

        it('should format future dates', () => {
            const now = new Date(2025, 9, 17, 14, 30, 45)
            const future = now.future(1000 * 60 * 60) // 1 hour
            const formatted = future.format('HH:mm:ss')
            expect(formatted).toBe('15:30:45')
        })

        it('should work with method chaining', () => {
            const now = new Date(2025, 9, 17, 14, 30, 45)
            const formatted = now.future(1000).format('yyyy-MM-dd HH:mm:ss')
            expect(formatted).toMatch(/2025-10-17 14:30:46/)
        })

        it('should handle elapsed for parsed dates', () => {
            const past = Date.parseUTCDate('2020-01-01')
            expect(past.elapsed).toBeGreaterThan(0)
        })
    })
})
