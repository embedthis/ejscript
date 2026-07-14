/**
 * MprLog - MPR logging subsystem
 *
 * Provides low-level access to application log file
 * @spec ejs
 * @stability evolving
 */

import { Path } from '../Path.js'

export class MprLog {
    private _level: number = 0
    private _location: string = 'stderr'
    private _fixed: boolean = false

    /**
     * Get/set log level
     */
    get level(): number {
        return this._level
    }

    set level(value: number) {
        this._level = value
    }

    /**
     * Get/set log location
     */
    get location(): string {
        return this._location
    }

    set location(value: string) {
        this._location = value
    }

    /**
     * Check if log is fixed (set via command line)
     */
    get fixed(): boolean {
        return this._fixed
    }

    /**
     * Redirect log output
     * @param location New output location
     * @param level New log level
     */
    redirect(location: string, level: number): void {
        if (!this._fixed) {
            this._location = location
            this._level = level
        }
    }

    /**
     * Write to the log
     * @param ...args Message parts
     */
    write(...args: any[]): void {
        const message = args.join(' ')
        const timestamp = new Date().toISOString()
        const logLine = `[${timestamp}] ${message}\n`

        if (this._location === 'stdout') {
            process.stdout.write(logLine)
        } else if (this._location === 'stderr') {
            process.stderr.write(logLine)
        } else {
            // Write to file
            const path = new Path(this._location)
            path.append(logLine)
        }
    }
}
