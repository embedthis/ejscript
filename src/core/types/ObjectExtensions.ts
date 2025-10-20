/**
 * Object extensions
 *
 * Adds Ejscript utility methods for objects
 * @spec ejs
 */

/**
 * Blend (merge) objects
 */
export function blend(
    dest: any,
    src: any,
    options?: { overwrite?: boolean; functions?: boolean }
): any {
    const overwrite = options?.overwrite !== false
    const includeFunctions = options?.functions !== false

    for (const key in src) {
        if (!src.hasOwnProperty(key)) continue

        const value = src[key]

        if (typeof value === 'function' && !includeFunctions) {
            continue
        }

        if (overwrite || !(key in dest)) {
            if (value && typeof value === 'object' && !Array.isArray(value) && value.constructor === Object) {
                if (typeof dest[key] !== 'object' || dest[key] === null) {
                    dest[key] = {}
                }
                blend(dest[key], value, options)
            } else {
                dest[key] = value
            }
        }
    }

    return dest
}

/**
 * Clone an object
 */
export function clone(obj: any, deep: boolean = true): any {
    if (obj === null || typeof obj !== 'object') {
        return obj
    }

    if (Array.isArray(obj)) {
        return deep ? obj.map(item => clone(item, true)) : [...obj]
    }

    if (obj instanceof Date) {
        return new Date(obj)
    }

    if (obj instanceof RegExp) {
        return new RegExp(obj)
    }

    if (deep) {
        const cloned: any = {}
        for (const key in obj) {
            if (obj.hasOwnProperty(key)) {
                cloned[key] = clone(obj[key], true)
            }
        }
        return cloned
    } else {
        return { ...obj }
    }
}

/**
 * Get object type
 */
export function getType(obj: any): Function {
    return obj.constructor
}

/**
 * Get object name
 */
export function getName(obj: any): string {
    if (typeof obj === 'function') {
        return obj.name
    }
    return obj.constructor.name
}

declare global {
    interface ObjectConstructor {
        blend(dest: any, src: any, options?: { overwrite?: boolean; functions?: boolean }): any
        clone(obj: any, deep?: boolean): any
        getType(obj: any): Function
        getName(obj: any): string
    }
}

Object.blend = blend
Object.clone = clone
Object.getType = getType
Object.getName = getName
