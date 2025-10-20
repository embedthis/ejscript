/**
 * Global namespace and variables for Ejscript compatibility
 */

import { App } from './core/App'
import type { TextStream } from './core/streams/TextStream'

/**
 * Product singleton object
 * @spec ejs
 */
export const ejs: Record<string, any> = {}

/**
 * Standard output text stream
 * @spec ejs
 */
export let stdout: TextStream

/**
 * Standard input text stream
 * @spec ejs
 */
export let stdin: TextStream

/**
 * Standard error text stream
 * @spec ejs
 */
export let stderr: TextStream

/**
 * Initialize global streams once App is initialized
 * This is called internally by App during initialization
 * @internal
 */
export function initGlobalStreams(): void {
    // These will be set by App initialization
    // stdout = App.stdout
    // stdin = App.stdin
    // stderr = App.stderr
}

/**
 * Constant set to true in all Ejscript interpreters
 * @spec ejs
 */
export const EJSCRIPT: boolean = true

/**
 * Global print function
 * Writes output to stdout
 */
export function print(...args: any[]): void {
    console.log(...args)
}

/**
 * Load a module dynamically
 * @param module Module path to load
 * @param options Load options
 */
export async function load(module: string, options?: { reload?: boolean }): Promise<any> {
    // Use dynamic import
    return import(module)
}
