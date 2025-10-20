/**
 * Date extensions
 *
 * Adds Ejscript methods to the Date prototype
 * @spec ejs
 */

declare global {
    interface Date {
        elapsed: number
        format(fmt?: string): string
        future(msec: number): Date
        toUTCString(): string
    }

    interface DateConstructor {
        parseUTCDate(str: string): Date
    }
}

/**
 * Get elapsed time since this date in milliseconds
 */
Object.defineProperty(Date.prototype, 'elapsed', {
    get: function(this: Date): number {
        return Date.now() - this.getTime()
    },
    enumerable: false,
    configurable: true
})

/**
 * Format date as string
 */
Date.prototype.format = function(fmt?: string): string {
    if (!fmt) {
        return this.toISOString()
    }

    const pad = (n: number) => n < 10 ? '0' + n : '' + n

    return fmt
        .replace(/yyyy/g, this.getFullYear().toString())
        .replace(/yy/g, this.getFullYear().toString().slice(-2))
        .replace(/MM/g, pad(this.getMonth() + 1))
        .replace(/dd/g, pad(this.getDate()))
        .replace(/HH/g, pad(this.getHours()))
        .replace(/mm/g, pad(this.getMinutes()))
        .replace(/ss/g, pad(this.getSeconds()))
}

/**
 * Get a future date
 */
Date.prototype.future = function(msec: number): Date {
    return new Date(this.getTime() + msec)
}

/**
 * Parse UTC date string
 */
Date.parseUTCDate = function(str: string): Date {
    return new Date(str)
}

// Export for module system
export {}
