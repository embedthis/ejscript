/**
 * Logger - Application logging
 *
 * Provides structured logging with levels and output control.
 * Loggers can direct output to Streams and may be aggregated in a hierarchical manner.
 * @spec ejs
 * @stability evolving
 */

import { File } from '../File'

// Type for App to avoid circular dependency
type AppInstance = {
    outputStream: any
    errorStream: any
}

/**
 * Logger objects provide a convenient and consistent method to capture and store logging information.
 * Loggers can direct output to Streams and may be aggregated in a hierarchical manner.
 * Each logger may define a filter function that returns true or false depending on whether a specific message
 * should be logged or not. A matching pattern can alternatively be used to filter messages.
 */
export class Logger {
    // Static log level constants
    /** Logging level for no logging */
    static readonly Off: number = -1
    /** Logging level for most serious errors */
    static readonly Error: number = 0
    /** Logging level for warnings */
    static readonly Warn: number = 1
    /** Logging level for informational messages */
    static readonly Info: number = 2
    /** Logging level for configuration output */
    static readonly Config: number = 3
    /** Logging level to output all messages */
    static readonly All: number = 9

    private _name: string
    private _level: number
    private _location: any
    private _outStream: any
    private _fileOpenPromise: Promise<any> | null = null
    private _pendingWrites: Promise<any>[] = []
    private _filter?: (log: Logger, name: string, level: number, kind: string, msg: string) => boolean
    private _pattern?: RegExp
    private _app: AppInstance | null = null

    /**
     * Logger constructor
     * @param name Unique name of the logger
     * @param location Optional output stream or Logger to send messages to
     * @param level Optional integer verbosity level (0-9)
     * @param app Optional App instance (to avoid circular dependency)
     */
    constructor(name: string, location: any = null, level: number = 0, app: AppInstance | null = null) {
        this._app = app
        this._level = level
        this.redirect(location)
        // If parent is a Logger, create hierarchical name
        this._name = (this._outStream && this._outStream instanceof Logger)
            ? `${this._outStream.name}.${name}`
            : name
    }

    /**
     * Redirect log output
     * @param location Output stream, Logger, or location specification (file:level)
     * @param level Optional new log level
     */
    redirect(location: any, level: number | null = null): void {
        if (!location) {
            // Use app instance if available, otherwise use process.stderr
            this._outStream = this._app?.errorStream ?? process.stderr
            this._location = 'stderr'
            return
        }

        // Check if it's a Stream (has write method) or Logger
        if (typeof location === 'object' && ('write' in location || location instanceof Logger)) {
            this._outStream = location
            this._location = location
        } else {
            // Parse location string (file:level format)
            // Handle Windows paths (C:\...) by looking for the last colon
            const locationStr = String(location)
            let path = locationStr
            let lev: number | null = null

            // Find the last colon that's followed by a number (the level specifier)
            const lastColonIndex = locationStr.lastIndexOf(':')
            if (lastColonIndex > 0) {
                const potentialLevel = locationStr.substring(lastColonIndex + 1)
                const parsedLevel = parseInt(potentialLevel, 10)
                // Check if it's a valid number and the entire string after colon is just the number
                if (!isNaN(parsedLevel) && potentialLevel.trim() === String(parsedLevel)) {
                    lev = parsedLevel
                    path = locationStr.substring(0, lastColonIndex)
                }
            }

            if (lev !== null) {
                this._level = lev
            } else if (level !== null) {
                this._level = level
            }

            // Determine output stream
            if (path === 'stdout') {
                this._outStream = this._app?.outputStream ?? process.stdout
            } else if (path === 'stderr') {
                this._outStream = this._app?.errorStream ?? process.stderr
            } else {
                // Open file for appending - handle async open
                const file = new File(path)
                this._outStream = file
                // Open the file asynchronously and store the promise
                this._fileOpenPromise = file.open('wa+').catch((err: any) => {
                    console.error(`Failed to open log file ${path}:`, err)
                    throw err
                })
            }
            this._location = location
        }
    }

    /**
     * Async mode - not supported for Loggers
     */
    get async(): boolean {
        return false
    }

    set async(_enable: boolean) {
        throw new Error('Async mode not supported')
    }

    /**
     * Close the logger
     * Returns a Promise if the underlying stream requires async close (File),
     * otherwise returns void for synchronous close
     */
    close(): void | Promise<void> {
        // Wait for pending writes before closing
        if (this._pendingWrites.length > 0 || (this._outStream && typeof this._outStream.close === 'function')) {
            return Promise.all(this._pendingWrites).then(() => {
                this._pendingWrites = []
                if (this._outStream && typeof this._outStream.close === 'function') {
                    const result = this._outStream.close()
                    if (result instanceof Promise) {
                        return result.catch(() => {
                            // Silently ignore close errors
                        })
                    }
                }
            }).catch(() => {
                // Silently ignore errors
            }).finally(() => {
                this._outStream = null
            })
        }
        this._outStream = null
    }

    /**
     * Filter function for this logger
     * The filter is called with signature: filter(log, name, level, kind, msg)
     */
    get filter(): ((log: Logger, name: string, level: number, kind: string, msg: string) => boolean) | undefined {
        return this._filter
    }

    set filter(fn: ((log: Logger, name: string, level: number, kind: string, msg: string) => boolean) | undefined) {
        this._filter = fn
    }

    /**
     * Flush output stream
     * @param dir Direction (ignored, for Stream interface compatibility)
     */
    flush(dir: number = 0): void {
        if (this._outStream && typeof this._outStream.flush === 'function') {
            this._outStream.flush(dir)
        }
    }

    /**
     * The numeric verbosity setting (0-9) of this logger
     */
    get level(): number {
        return this._level
    }

    set level(level: number) {
        this._level = level
        // Propagate to parent logger if applicable
        if (this._outStream && this._outStream instanceof Logger) {
            this._outStream.level = level
        }
    }

    /**
     * The logging location parameter specified when constructing or redirecting the logger
     */
    get location(): any {
        return this._location
    }

    /**
     * Matching expression to filter log messages by logger name
     */
    get match(): RegExp | undefined {
        return this._pattern
    }

    set match(pattern: RegExp | undefined) {
        this._pattern = pattern
    }

    /**
     * The name of this logger
     */
    get name(): string {
        return this._name
    }

    set name(name: string) {
        this._name = name
    }

    /**
     * The output stream used by the logger
     */
    get outStream(): any {
        return this._outStream
    }

    set outStream(stream: any) {
        this._outStream = stream
    }

    /**
     * Emit a debug message
     * @param level The level of the message
     * @param msgs The messages to log
     */
    debug(level: number, ...msgs: any[]): void {
        this.emit('', level, '', msgs.join(' ') + '\n')
    }

    /**
     * Emit a configuration message
     * @param msgs Data to log
     */
    config(...msgs: any[]): void {
        this.emit('', Logger.Config, 'CONFIG', msgs.join(' ') + '\n')
    }

    /**
     * Emit an error message
     * @param msgs Data to log
     */
    error(...msgs: any[]): void {
        this.emit('', Logger.Error, 'ERROR', msgs.join(' ') + '\n')
    }

    /**
     * Emit an informational message
     * @param msgs Data to log
     */
    info(...msgs: any[]): void {
        this.emit('', Logger.Info, 'INFO', msgs.join(' ') + '\n')
    }

    /**
     * Emit an activity message
     * @param tag Activity tag to prefix the message
     * @param args Output string to log
     */
    activity(tag: string, ...args: any[]): void {
        const msg = args.join(' ')
        const formatted = `${`[${tag}]`.padStart(12)} ${msg}\n`
        this.write(formatted)
    }

    /**
     * Event emitter interface - not supported
     */
    off(_name: string, _observer: Function): void {
        throw new Error('off is not supported')
    }

    /**
     * Event emitter interface - not supported
     */
    on(_name: string, _observer: Function): Logger {
        throw new Error('on is not supported')
    }

    /**
     * Read from logger - not supported
     */
    read(_buffer: any, _offset: number = 0, _count: number = -1): number | null {
        throw new Error('Read not supported')
    }

    /**
     * Write messages to the logger stream
     * NOTE: I/O errors will not throw exceptions
     * @param data Data to write
     * @returns Number of bytes written
     */
    write(...data: any[]): number {
        try {
            if (this._outStream) {
                // If file is being opened, wait for it first
                if (this._fileOpenPromise) {
                    const writePromise = this._fileOpenPromise.then(() => {
                        const result = this._outStream.write(data.join(' '))
                        if (result instanceof Promise) {
                            return result.catch(() => {
                                // Silently ignore write errors
                            })
                        }
                        return result
                    }).catch(() => {
                        // File open failed, silently ignore
                    })
                    this._pendingWrites.push(writePromise)
                    return 0
                }

                const result = this._outStream.write(data.join(' '))
                // Handle async File writes
                if (result instanceof Promise) {
                    const writePromise = result.catch(() => {
                        // Silently ignore write errors per method contract
                    })
                    this._pendingWrites.push(writePromise)
                    return 0 // Return 0 for async writes since we don't know the result yet
                }
                return result
            }
            return 0
        } catch {
            return 0
        }
    }

    /**
     * Emit a warning message
     * @param msgs The data to log
     */
    warn(...msgs: any[]): void {
        this.emit('', Logger.Warn, 'WARN', msgs.join(' ') + '\n')
    }

    /**
     * Emit a message - internal method
     * The message level will be compared to the logger setting to determine whether it will be
     * output or not. Also, if the logger has a filter function set that may filter the message.
     * @param origin Name of the logger that originated the message
     * @param level The level of the message
     * @param kind Message kind (debug, info, warn, error, config)
     * @param msg The string message to emit
     */
    private emit(origin: string, level: number, kind: string, msg: string): void {
        origin = origin || this._name

        // Check level
        if (level > this._level || !this._outStream) {
            return
        }

        // Check pattern match
        if (this._pattern && !origin.match(this._pattern)) {
            return
        }

        // Check filter function
        if (this._filter && !this._filter(this, origin, level, kind, msg)) {
            return
        }

        // Send to parent logger or write directly
        if (this._outStream instanceof Logger) {
            this._outStream.emit(origin, level, kind, msg)
        } else if (kind) {
            this.write(`${origin}: ${kind}: ${msg}`)
        } else {
            this.write(`${origin}: ${level}: ${msg}`)
        }
    }

    /**
     * Alias methods for compatibility
     */
    log(...args: any[]): void {
        this.info(...args)
    }

    trace(...args: any[]): void {
        this.debug(5, ...args)
    }
}
