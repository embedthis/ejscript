/**
 * Worker - Web Worker style threading
 *
 * Provides worker thread functionality
 * @spec ejs
 * @stability evolving
 */

import { Emitter } from './Emitter'
import { Worker as BunWorker } from 'worker_threads'

export class Worker extends Emitter {
    private worker?: BunWorker
    private _onerror?: (error: Error) => void
    private _onmessage?: (message: any) => void

    /**
     * Create a worker
     * @param scriptPath Path to worker script
     * @param options Worker options
     */
    constructor(scriptPath: string, options?: any) {
        super()

        this.worker = new BunWorker(scriptPath, options)

        this.worker.on('message', (data) => {
            if (this._onmessage) {
                this._onmessage(data)
            }
            this.emit('message', data)
        })

        this.worker.on('error', (error) => {
            if (this._onerror) {
                this._onerror(error)
            }
            this.emit('error', error)
        })

        this.worker.on('exit', (code) => {
            this.emit('exit', code)
        })
    }

    /**
     * Post a message to the worker
     * @param message Message to send
     * @param transfer Optional transferable objects
     */
    postMessage(message: any, transfer?: any[]): void {
        if (this.worker) {
            this.worker.postMessage(message, transfer)
        }
    }

    /**
     * Terminate the worker
     */
    terminate(): void {
        if (this.worker) {
            this.worker.terminate()
        }
    }

    /**
     * Get worker thread ID
     */
    get threadId(): number {
        return this.worker?.threadId ?? -1
    }

    /**
     * Error handler callback
     */
    get onerror(): ((error: Error) => void) | undefined {
        return this._onerror
    }

    set onerror(handler: ((error: Error) => void) | undefined) {
        this._onerror = handler
    }

    /**
     * Message handler callback
     */
    get onmessage(): ((message: any) => void) | undefined {
        return this._onmessage
    }

    set onmessage(handler: ((message: any) => void) | undefined) {
        this._onmessage = handler
    }

    /**
     * Evaluate code in the worker context
     * @param code Code to evaluate
     */
    eval(code: string): void {
        this.postMessage({ type: 'eval', code })
    }

    /**
     * Pre-evaluate code before worker starts
     * @param code Code to pre-evaluate
     */
    preeval(code: string): void {
        // In Bun, we can't pre-eval, so we'll send it immediately
        this.eval(code)
    }

    /**
     * Load a script into the worker
     * @param scriptPath Path to script
     */
    load(scriptPath: string): void {
        this.postMessage({ type: 'load', scriptPath })
    }

    /**
     * Pre-load a script before worker starts
     * @param scriptPath Path to script
     */
    preload(scriptPath: string): void {
        // In Bun, we can't pre-load, so we'll send it immediately
        this.load(scriptPath)
    }

    /**
     * Exit the worker (from within the worker)
     * @param code Exit code
     */
    exit(code: number = 0): void {
        this.postMessage({ type: 'exit', code })
        this.terminate()
    }

    /**
     * Clone the worker
     * @returns New worker with same script
     */
    clone(): Worker {
        // We can't truly clone in Bun, but we can create a new instance
        // This requires storing the script path, which we don't have access to
        throw new Error('Worker.clone() not supported in Bun environment')
    }

    /**
     * Wait for a message from the worker
     * @param timeout Timeout in milliseconds
     * @returns Promise that resolves with the message
     */
    async waitForMessage(timeout: number = 30000): Promise<any> {
        return new Promise((resolve, reject) => {
            const timer = setTimeout(() => {
                reject(new Error('Timeout waiting for message'))
            }, timeout)

            const handler = (message: any) => {
                clearTimeout(timer)
                this.off('message', handler)
                resolve(message)
            }

            this.on('message', handler)
        })
    }
}
