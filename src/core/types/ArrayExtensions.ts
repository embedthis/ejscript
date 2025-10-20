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
Array.prototype.contains = function<T>(this: T[], item: T): boolean {
    return this.indexOf(item) >= 0
}

// ============================================================================
// Array Manipulation
// ============================================================================

/**
 * Append items to array (mutates array)
 */
Array.prototype.append = function<T>(this: T[], item: T | T[]): T[] {
    if (Array.isArray(item)) {
        this.push(...item)
    } else {
        this.push(item)
    }
    return this
}

/**
 * Clear all elements from the array
 */
Array.prototype.clear = function<T>(this: T[]): void {
    this.length = 0
}

/**
 * Remove all null and undefined elements
 */
Array.prototype.compact = function<T>(this: T[]): T[] {
    for (let i = this.length - 1; i >= 0; i--) {
        if (this[i] === null || this[i] === undefined) {
            this.splice(i, 1)
        }
    }
    return this
}

/**
 * Remove elements from start to end (inclusive)
 * Negative indices measure from the end of the array
 */
Array.prototype.remove = function<T>(this: T[], start: number, end: number = -1): void {
    if (start < 0) {
        start += this.length
    }
    if (end < 0) {
        end += this.length
    }
    this.splice(start, end - start + 1)
}

// ============================================================================
// Transformation Methods
// ============================================================================

/**
 * Transform array elements in-place
 * Modifies the original array
 */
Array.prototype.transform = function<T>(
    this: T[],
    fn: (item: T, index: number, array: T[]) => any
): T[] {
    for (let i = 0; i < this.length; i++) {
        this[i] = fn(this[i], i, this)
    }
    return this
}

/**
 * Clone the array
 * @param deep If true, recursively clone nested arrays and objects
 */
Array.prototype.clone = function<T>(this: T[], deep: boolean = true): T[] {
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
}

/**
 * Remove duplicate elements
 */
Array.prototype.unique = function<T>(this: T[]): T[] {
    return Array.from(new Set(this))
}

// ============================================================================
// Filtering Methods
// ============================================================================

/**
 * Find all elements matching the predicate
 * Alias for filter()
 */
Array.prototype.findAll = function<T>(
    this: T[],
    match: (item: T, index: number, array: T[]) => boolean
): T[] {
    return this.filter(match)
}

/**
 * Find all elements that do NOT match the predicate
 * Opposite of filter()
 */
Array.prototype.reject = function<T>(
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
}

// Export for module system
export {}
