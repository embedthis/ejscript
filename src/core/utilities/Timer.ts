/**
 * Timer - Timer and callback management
 *
 * Provides timer functionality with callbacks
 * @spec ejs
 * @stability evolving
 */

import { Emitter } from '../async/Emitter.js'

export class Timer extends Emitter {
    private _period: number
    private _drift: boolean = false
    private _repeat: boolean = false
    private _callback?: Function
    private _callbackArgs: any[] = []
    private _onerror?: Function
    private timerId?: ReturnType<typeof setTimeout>
    private intervalId?: ReturnType<typeof setInterval>
    private _isRunning: boolean = false

    /**
     * Create a timer. The timer will not run until start() is called.
     * @param period Timer period in milliseconds
     * @param callback Callback function to invoke when timer fires
     * @param args Arguments to pass to the callback
     */
    constructor(period: number, callback?: Function, ...args: any[]) {
        super()
        this._period = period
        this._callback = callback
        this._callbackArgs = args
    }

    /**
     * Get timer period in milliseconds
     */
    get period(): number {
        return this._period
    }

    /**
     * Set timer period in milliseconds
     * Changing the period does not reschedule a currently running timer.
     * The new period will be used for the next invocation.
     */
    set period(value: number) {
        this._period = value
    }

    /**
     * Get drift setting
     * If true, timer is allowed to drift its execution time
     */
    get drift(): boolean {
        return this._drift
    }

    /**
     * Set drift setting
     * If false, timer is scheduled to not drift (period between start times is constant)
     */
    set drift(enable: boolean) {
        this._drift = enable
    }

    /**
     * Get repeat setting
     * If true, timer will be repeatedly invoked
     */
    get repeat(): boolean {
        return this._repeat
    }

    /**
     * Set repeat setting
     * If true, timer will be repeatedly invoked every period milliseconds
     */
    set repeat(enable: boolean) {
        this._repeat = enable
    }

    /**
     * Get error callback function
     * Called when an exception occurs inside the timer callback
     */
    get onerror(): Function | undefined {
        return this._onerror
    }

    /**
     * Set error callback function
     * The callback is invoked with signature: function callback(error: Error): void
     */
    set onerror(callback: Function | undefined) {
        this._onerror = callback
    }

    /**
     * Start the timer running
     * The timer will be repeatedly invoked if repeat property is true
     * @returns The timer instance for chaining
     */
    start(): Timer {
        if (this._isRunning) {
            return this
        }

        this._isRunning = true

        if (this._repeat) {
            // Repeating timer
            if (this._drift) {
                // Use setInterval for drifting timer
                this.intervalId = setInterval(() => {
                    this.triggerCallback()
                }, this._period)
            } else {
                // Non-drifting repeating timer
                this.scheduleRepeating()
            }
        } else {
            // One-shot timer
            this.timerId = setTimeout(() => {
                this.triggerCallback()
                this._isRunning = false
            }, this._period)
        }

        return this
    }

    /**
     * Stop the timer
     * Once stopped, the timer can be restarted by calling start()
     */
    stop(): void {
        this._isRunning = false

        if (this.intervalId) {
            clearInterval(this.intervalId)
            this.intervalId = undefined
        }

        if (this.timerId) {
            clearTimeout(this.timerId)
            this.timerId = undefined
        }
    }

    /**
     * Reschedule the timer
     * Stops the timer if currently scheduled, then reschedules it
     * @param when Optional new period. If provided, sets the period before starting
     */
    restart(when?: number): void {
        this.stop()
        if (when !== undefined) {
            this._period = when
        }
        this.start()
    }

    /**
     * Fire the timer callback
     */
    private triggerCallback(): void {
        try {
            if (this._callback) {
                // Call with timer as 'this' unless callback has bound 'this'
                if (this._callbackArgs.length > 0) {
                    this._callback.call(this, ...this._callbackArgs)
                } else {
                    this._callback.call(this)
                }
            }

            this.emit('timer', this)
        } catch (error) {
            if (this._onerror) {
                try {
                    this._onerror(error)
                } catch (err) {
                    // Error in error handler - just log it
                    console.error('Error in timer onerror handler:', err)
                }
            } else {
                // No error handler - log the error
                console.error('Error in timer callback:', error)
            }
        }
    }

    /**
     * Schedule repeating timer without drift
     * Compensates for execution time to maintain constant period between starts
     */
    private scheduleRepeating(): void {
        let expectedTime = Date.now() + this._period

        const runNext = () => {
            if (!this._isRunning) {
                return
            }

            this.triggerCallback()

            // Calculate drift and adjust next timeout
            const now = Date.now()
            const drift = now - expectedTime
            expectedTime += this._period

            // Schedule next with drift compensation
            const nextDelay = Math.max(0, this._period - drift)
            this.timerId = setTimeout(runNext, nextDelay)
        }

        this.timerId = setTimeout(runNext, this._period)
    }

    /**
     * Create a one-shot timer
     * @param delay Delay in milliseconds
     * @param callback Callback function
     * @returns Timer instance
     */
    static after(delay: number, callback: Function): Timer {
        const timer = new Timer(delay, callback)
        setTimeout(() => {
            callback()
        }, delay)
        return timer
    }
}
