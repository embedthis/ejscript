/**
 * Config - Build and platform configuration
 *
 * Provides compile-time and runtime configuration information
 * @spec ejs
 * @stability evolving
 */

export class Config {
    /**
     * Product name
     */
    static readonly Product: string = 'ejscript'

    /**
     * Product version
     */
    static readonly Version: string = '0.1.0'

    /**
     * Operating system name
     * Values: linux, darwin, windows, freebsd, etc.
     *
     * Normalised from process.platform, which reports "win32" on Windows - on 64-bit Windows too.
     * Callers compare against the documented "windows", so returning the raw value made every such
     * comparison silently false and quietly disabled their Windows-specific code paths.
     */
    static readonly OS: string = process.platform === 'win32' ? 'windows' : process.platform

    /**
     * CPU architecture
     * Values: x64, arm64, ia32, etc.
     */
    static readonly CPU: string = process.arch

    /**
     * Number of CPU cores
     */
    static readonly NumCPU: number = navigator.hardwareConcurrency || 1

    /**
     * Debug build flag
     */
    static readonly Debug: boolean = process.env.NODE_ENV === 'development'

    /**
     * Legacy compatibility mode
     */
    static readonly Legacy: boolean = false

    /**
     * Database support enabled
     */
    static readonly DB: boolean = false

    /**
     * Web framework support enabled
     */
    static readonly WEB: boolean = false

    /**
     * Build date
     */
    static readonly BuildDate: Date = new Date()

    /**
     * Get a configuration value by name
     * @param name Configuration property name
     * @returns Configuration value or undefined
     */
    static get(name: string): any {
        return (Config as any)[name]
    }
}
