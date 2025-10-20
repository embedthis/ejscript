/**
 * String extensions
 *
 * Adds Ejscript methods to the String prototype
 * @spec ejs
 */

declare global {
    interface String {
        // Comparison methods
        caseCompare(compare: string): number
        caselessCompare(compare: string): number

        // Search methods
        contains(pattern: string | RegExp): boolean
        startsWith(prefix: string): boolean
        endsWith(suffix: string): boolean

        // Character type checks
        isDigit: boolean
        isAlpha: boolean
        isAlphaNum: boolean
        isLower: boolean
        isSpace: boolean
        isUpper: boolean

        // Formatting and manipulation
        format(...args: any[]): string
        expand(vars: Record<string, any>, options?: { fill?: any, join?: string }): string
        printable(): string
        quote(): string
        remove(start: number, end?: number): string
        reverse(): string
        times(count: number): string
        tokenize(format: string): string[]

        // Case conversion
        toPascal(): string
        toCamel(): string
        capitalize(): string

        // Trimming
        trim(str?: string | null): string
        trimStart(str?: string | null): string
        trimEnd(str?: string | null): string

        // Conversion
        parseJSON(filter?: Function | null): any
        toPath(): any
        toJSON(): string
    }
}

// ============================================================================
// Comparison Methods
// ============================================================================

/**
 * Case-sensitive comparison
 */
String.prototype.caseCompare = function(compare: string): number {
    if (this < compare) return -1
    if (this > compare) return 1
    return 0
}

/**
 * Case-insensitive comparison
 */
String.prototype.caselessCompare = function(compare: string): number {
    const a = this.toLowerCase()
    const b = compare.toLowerCase()
    if (a < b) return -1
    if (a > b) return 1
    return 0
}

// ============================================================================
// Search Methods
// ============================================================================

/**
 * Check if string contains a pattern (string or regex)
 */
String.prototype.contains = function(pattern: string | RegExp): boolean {
    if (pattern instanceof RegExp) {
        return pattern.test(this.toString())
    }
    return this.indexOf(pattern as string) >= 0
}

// ============================================================================
// Character Type Checks
// ============================================================================

Object.defineProperty(String.prototype, 'isDigit', {
    get: function() {
        if (this.length === 0) return false
        return /^\d+$/.test(this.toString())
    },
    enumerable: false,
    configurable: true
})

Object.defineProperty(String.prototype, 'isAlpha', {
    get: function() {
        if (this.length === 0) return false
        return /^[a-zA-Z]+$/.test(this.toString())
    },
    enumerable: false,
    configurable: true
})

Object.defineProperty(String.prototype, 'isAlphaNum', {
    get: function() {
        if (this.length === 0) return false
        return /^[a-zA-Z0-9]+$/.test(this.toString())
    },
    enumerable: false,
    configurable: true
})

Object.defineProperty(String.prototype, 'isLower', {
    get: function() {
        if (this.length === 0) return false
        const str = this.toString()
        return str === str.toLowerCase() && str !== str.toUpperCase()
    },
    enumerable: false,
    configurable: true
})

Object.defineProperty(String.prototype, 'isSpace', {
    get: function() {
        if (this.length === 0) return false
        return /^\s+$/.test(this.toString())
    },
    enumerable: false,
    configurable: true
})

Object.defineProperty(String.prototype, 'isUpper', {
    get: function() {
        if (this.length === 0) return false
        const str = this.toString()
        return str === str.toUpperCase() && str !== str.toLowerCase()
    },
    enumerable: false,
    configurable: true
})

// ============================================================================
// Formatting and Manipulation
// ============================================================================

/**
 * Printf-style string formatting
 * Supports: %s (string), %d/%i (integer), %f (float), %o (object)
 */
String.prototype.format = function(...args: any[]): string {
    let argIndex = 0
    return this.replace(/%(-?\d*\.?\d*)([sdifoxXeEgG])/g, (match, flags, type) => {
        if (argIndex >= args.length) return match

        const arg = args[argIndex++]

        switch (type) {
            case 's':
                return String(arg)
            case 'd':
            case 'i':
                return parseInt(arg).toString()
            case 'f':
            case 'e':
            case 'E':
            case 'g':
            case 'G': {
                const num = parseFloat(arg)
                if (flags) {
                    const precision = parseInt(flags.split('.')[1] || '6')
                    return num.toFixed(precision)
                }
                return num.toString()
            }
            case 'o':
            case 'x':
            case 'X':
                return parseInt(arg).toString(16)
            default:
                return match
        }
    })
}

/**
 * Expand template variables with ${token} syntax
 * Supports nested properties with dot notation: ${user.name}
 */
String.prototype.expand = function(
    vars: Record<string, any>,
    options?: { fill?: any, join?: string }
): string {
    return this.replace(/\$\$\{([^}]+)\}|\$\{([^}]+)\}/g, (match, escapedKey, key) => {
        // Handle escaped $${token} - just return ${token}
        if (escapedKey) {
            return '${' + escapedKey + '}'
        }

        // Handle normal ${token}
        if (!key) return match

        // Handle nested properties
        const value = key.split('.').reduce((obj: any, prop: string) => {
            return obj?.[prop]
        }, vars)

        if (value !== undefined) {
            // Handle arrays with join
            if (Array.isArray(value)) {
                return value.join(options?.join || ' ')
            }
            return String(value)
        }

        // Handle missing values based on fill option
        const fill = options?.fill
        if (fill === undefined || fill === null) {
            throw new Error(`Missing property: ${key}`)
        }
        if (fill === true) return match  // Preserve token
        if (fill === false) return ''    // Remove token
        return String(fill)              // Use fill string
    })
}

/**
 * Convert nonprintable characters to unicode hex escapes
 */
String.prototype.printable = function(): string {
    return this.replace(/[^\x20-\x7E]/g, (char) => {
        const code = char.charCodeAt(0)
        return '\\u' + code.toString(16).padStart(4, '0')
    })
}

/**
 * Wrap string in double quotes
 */
String.prototype.quote = function(): string {
    return '"' + this.toString() + '"'
}

/**
 * Remove characters from start to end
 */
String.prototype.remove = function(start: number, end?: number): string {
    const str = this.toString()
    const len = str.length

    // Handle negative indices
    if (start < 0) start = len + start
    if (end === undefined || end === -1) end = len
    if (end < 0) end = len + end

    return str.slice(0, start) + str.slice(end)
}

/**
 * Reverse the string
 */
String.prototype.reverse = function(): string {
    return this.split('').reverse().join('')
}

/**
 * Repeat string N times
 */
String.prototype.times = function(count: number): string {
    if (count <= 0) return ''
    return this.repeat(count)
}

/**
 * Tokenize string according to format specifier
 * Simple implementation - splits on whitespace if format has %s tokens
 */
String.prototype.tokenize = function(format: string): string[] {
    // Count format specifiers
    const specifiers = format.match(/%[sdif]/g) || []

    // Split the string into tokens
    const tokens = this.trim().split(/\s+/)

    // Return only as many tokens as specifiers
    return tokens.slice(0, specifiers.length)
}

// ============================================================================
// Case Conversion
// ============================================================================

/**
 * Convert to Pascal case (capitalize first letter)
 */
String.prototype.toPascal = function(): string {
    const str = this.toString()
    if (str.length === 0) return str

    // Find first alphabetic character
    const match = str.match(/[a-zA-Z]/)
    if (!match) return str

    const index = match.index!
    return str.slice(0, index) + str[index].toUpperCase() + str.slice(index + 1)
}

/**
 * Convert to camel case (lowercase first letter)
 */
String.prototype.toCamel = function(): string {
    const str = this.toString()
    if (str.length === 0) return str

    // Find first alphabetic character
    const match = str.match(/[a-zA-Z]/)
    if (!match) return str

    const index = match.index!
    return str.slice(0, index) + str[index].toLowerCase() + str.slice(index + 1)
}

/**
 * Capitalize first character (alias for toPascal)
 */
String.prototype.capitalize = function(): string {
    return this.toPascal()
}

// ============================================================================
// Trimming (enhanced to support custom trim strings)
// ============================================================================

// Save original trim methods before overriding
const originalTrim = String.prototype.trim
const originalTrimStart = String.prototype.trimStart
const originalTrimEnd = String.prototype.trimEnd

/**
 * Trim string from both ends
 * @param str Substring to trim, defaults to whitespace
 */
String.prototype.trim = function(str?: string | null): string {
    if (str === null || str === undefined) {
        return originalTrim.call(this)
    }

    let result = this.toString()
    while (result.startsWith(str)) {
        result = result.slice(str.length)
    }
    while (result.endsWith(str)) {
        result = result.slice(0, -str.length)
    }
    return result
}

/**
 * Trim string from start
 * @param str Substring to trim, defaults to whitespace
 */
String.prototype.trimStart = function(str?: string | null): string {
    if (str === null || str === undefined) {
        return originalTrimStart.call(this)
    }

    let result = this.toString()
    while (result.startsWith(str)) {
        result = result.slice(str.length)
    }
    return result
}

/**
 * Trim string from end
 * @param str Substring to trim, defaults to whitespace
 */
String.prototype.trimEnd = function(str?: string | null): string {
    if (str === null || str === undefined) {
        return originalTrimEnd.call(this)
    }

    let result = this.toString()
    while (result.endsWith(str)) {
        result = result.slice(0, -str.length)
    }
    return result
}

// ============================================================================
// Conversion
// ============================================================================

/**
 * Parse JSON from string
 */
String.prototype.parseJSON = function(filter?: Function | null): any {
    const obj = JSON.parse(this.toString())

    if (filter) {
        // Apply filter recursively
        const walk = (holder: any, key: string): any => {
            const value = holder[key]
            if (value && typeof value === 'object') {
                for (const k in value) {
                    if (Object.prototype.hasOwnProperty.call(value, k)) {
                        const v = walk(value, k)
                        if (v !== undefined) {
                            value[k] = v
                        } else {
                            delete value[k]
                        }
                    }
                }
            }
            return filter(key, value)
        }
        return walk({ '': obj }, '')
    }

    return obj
}

/**
 * Convert string to Path object
 */
String.prototype.toPath = function(): any {
    // Import Path dynamically to avoid circular dependency
    const { Path } = require('../Path')
    return new Path(this.toString())
}

/**
 * Convert to JSON string representation
 */
String.prototype.toJSON = function(): string {
    return JSON.stringify(this.toString())
}

// Export for module system
export {}
