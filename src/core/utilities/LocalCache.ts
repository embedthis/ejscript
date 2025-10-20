/**
 * LocalCache - Local in-memory cache
 *
 * Similar to Cache but for local, non-shared caching
 * @spec ejs
 * @stability evolving
 */

import { Cache, CacheOptions } from './Cache'

export class LocalCache extends Cache {
    /**
     * Create a local cache instance
     * @param name Cache name
     * @param options Cache options
     */
    constructor(name: string | null = null, options: CacheOptions = {}) {
        super(name, { ...options, shared: false })
    }
}
