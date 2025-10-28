/**
 * Array extensions
 *
 * Adds Ejscript methods to the Array prototype
 * @spec ejs
 */

declare global {
    interface Array<T> {
        // Search methods
        contains(item: T): boolean

        // Array manipulation
        append(item: T | T[]): this
        clear(): void
        compact(): this
        remove(start: number, end?: number): void

        // Transformation methods
        transform(fn: (item: T, index: number, array: T[]) => any): this
        clone(deep?: boolean): T[]
        unique(): T[]

        // Filtering methods
        findAll(match: (item: T, index: number, array: T[]) => boolean): T[]
        reject(match: (item: T, index: number, array: T[]) => boolean): T[]
    }
}

// ============================================================================
// Search Methods
// ============================================================================

/**
 * Check if array contains an item using strict equality
 */
Object.defineProperty(Array.prototype, 'contains', {
    value: function<T>(this: T[], item: T): boolean {
        return this.indexOf(item) >= 0
    },
    enumerable: false,
    writable: true,
    configurable: true
})

// ============================================================================
// Array Manipulation
// ============================================================================

/**
 * Append items to array (mutates array)
 */
Object.defineProperty(Array.prototype, 'append', {
    value: function<T>(this: T[], item: T | T[]): T[] {
        if (Array.isArray(item)) {
            this.push(...item)
        } else {
            this.push(item)
        }
        return this
    },
    enumerable: false,
    writable: true,
    configurable: true
})

/**
 * Clear all elements from the array
 */
Object.defineProperty(Array.prototype, 'clear', {
    value: function<T>(this: T[]): void {
        this.length = 0
    },
    enumerable: false,
    writable: true,
    configurable: true
})

/**
 * Remove all null and undefined elements
 */
Object.defineProperty(Array.prototype, 'compact', {
    value: function<T>(this: T[]): T[] {
        for (let i = this.length - 1; i >= 0; i--) {
            if (this[i] === null || this[i] === undefined) {
                this.splice(i, 1)
            }
        }
        return this
    },
    enumerable: false,
    writable: true,
    configurable: true
})

/**
 * Remove elements from start to end (inclusive)
 * Negative indices measure from the end of the array
 */
Object.defineProperty(Array.prototype, 'remove', {
    value: function<T>(this: T[], start: number, end: number = -1): void {
        if (start < 0) {
            start += this.length
        }
        if (end < 0) {
            end += this.length
        }
        this.splice(start, end - start + 1)
    },
    enumerable: false,
    writable: true,
    configurable: true
})

// ============================================================================
// Transformation Methods
// ============================================================================

/**
 * Transform array elements in-place
 * Modifies the original array
 */
Object.defineProperty(Array.prototype, 'transform', {
    value: function<T>(
        this: T[],
        fn: (item: T, index: number, array: T[]) => any
    ): T[] {
        for (let i = 0; i < this.length; i++) {
            this[i] = fn(this[i], i, this)
        }
        return this
    },
    enumerable: false,
    writable: true,
    configurable: true
})

/**
 * Clone the array
 * @param deep If true, recursively clone nested arrays and objects
 */
Object.defineProperty(Array.prototype, 'clone', {
    value: function<T>(this: T[], deep: boolean = true): T[] {
    if (!deep) {
        return [...this]
    }

    // Deep clone
        return this.map(item => {
            if (Array.isArray(item)) {
                return (item as any[]).clone(true) as any
            } else if (item && typeof item === 'object' && item.constructor === Object) {
                // Clone plain objects
                const cloned: any = {}
                for (const key in item) {
                    if (Object.prototype.hasOwnProperty.call(item, key)) {
                        const value = (item as any)[key]
                        if (Array.isArray(value)) {
                            cloned[key] = value.clone(true)
                        } else if (value && typeof value === 'object' && value.constructor === Object) {
                            cloned[key] = Object.assign({}, value)
                        } else {
                            cloned[key] = value
                        }
                    }
                }
                return cloned as T
            }
            return item
        })
    },
    enumerable: false,
    writable: true,
    configurable: true
})

/**
 * Remove duplicate elements
 */
Object.defineProperty(Array.prototype, 'unique', {
    value: function<T>(this: T[]): T[] {
        return Array.from(new Set(this))
    },
    enumerable: false,
    writable: true,
    configurable: true
})

// ============================================================================
// Filtering Methods
// ============================================================================

/**
 * Find all elements matching the predicate
 * Alias for filter()
 */
Object.defineProperty(Array.prototype, 'findAll', {
    value: function<T>(
        this: T[],
        match: (item: T, index: number, array: T[]) => boolean
    ): T[] {
        return this.filter(match)
    },
    enumerable: false,
    writable: true,
    configurable: true
})

/**
 * Find all elements that do NOT match the predicate
 * Opposite of filter()
 */
Object.defineProperty(Array.prototype, 'reject', {
    value: function<T>(
        this: T[],
        match: (item: T, index: number, array: T[]) => boolean
    ): T[] {
        const result: T[] = []
        for (let i = 0; i < this.length; i++) {
            if (!match(this[i], i, this)) {
                result.push(this[i])
            }
        }
        return result
    },
    enumerable: false,
    writable: true,
    configurable: true
})

// Export for module system
export {}
