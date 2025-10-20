/**
 * WebSocket - WebSocket client
 *
 * Provides WebSocket networking functionality
 * @spec ejs
 * @stability evolving
 */

import { Emitter } from './async/Emitter'

export class WebSocket extends Emitter {
    private ws?: globalThis.WebSocket
    private _url: string
    private _binaryType: 'blob' | 'arraybuffer' = 'blob'

    /**
     * Create a WebSocket client
     * @param url WebSocket URL (ws:// or wss://)
     */
    constructor(url: string) {
        super()
        this._url = url
    }

    /**
     * Connect to the WebSocket server
     */
    connect(): void {
        this.ws = new globalThis.WebSocket(this._url)
        // Bun's WebSocket has 'nodebuffer' instead of 'blob'
        if (this._binaryType === 'blob') {
            (this.ws as any).binaryType = 'nodebuffer'
        } else {
            (this.ws as any).binaryType = this._binaryType
        }

        this.ws.onopen = (event) => {
            this.emit('open', event)
        }

        this.ws.onmessage = (event) => {
            this.emit('message', event.data)
        }

        this.ws.onerror = (event) => {
            this.emit('error', event)
        }

        this.ws.onclose = (event) => {
            this.emit('close', event)
        }
    }

    /**
     * Send data through the WebSocket
     * @param data Data to send
     */
    send(data: string | ArrayBuffer | Uint8Array): void {
        if (!this.ws) {
            throw new Error('WebSocket not connected')
        }

        this.ws.send(data)
    }

    /**
     * Close the WebSocket connection
     * @param code Close code
     * @param reason Close reason
     */
    close(code?: number, reason?: string): void {
        if (this.ws) {
            this.ws.close(code, reason)
        }
    }

    /**
     * Get connection state
     */
    get readyState(): number {
        return this.ws?.readyState ?? 0
    }

    /**
     * Get buffered amount
     */
    get bufferedAmount(): number {
        return this.ws?.bufferedAmount ?? 0
    }

    /**
     * Get protocol
     */
    get protocol(): string {
        return this.ws?.protocol ?? ''
    }

    /**
     * Get URL
     */
    get url(): string {
        return this._url
    }

    /**
     * Binary type for received binary data
     */
    get binaryType(): 'blob' | 'arraybuffer' {
        return this._binaryType
    }

    set binaryType(type: 'blob' | 'arraybuffer') {
        this._binaryType = type
        if (this.ws) {
            // Bun's WebSocket has 'nodebuffer' instead of 'blob'
            if (type === 'blob') {
                (this.ws as any).binaryType = 'nodebuffer'
            } else {
                (this.ws as any).binaryType = type
            }
        }
    }

    /**
     * Get negotiated extensions
     */
    get extensions(): string {
        return this.ws?.extensions ?? ''
    }

    /**
     * Send binary block data
     * @param data Binary data to send
     */
    sendBlock(data: ArrayBuffer | Uint8Array): void {
        this.send(data)
    }

    /**
     * Wait for WebSocket to reach a specific state
     * @param state Target ready state (default: OPEN)
     * @param timeout Timeout in milliseconds (default: 30000)
     * @returns True if reached target state, false if timed out
     */
    async wait(state: number = WebSocket.OPEN, timeout: number = 30000): Promise<boolean> {
        if (this.readyState === state) {
            return true
        }

        return new Promise((resolve) => {
            const startTime = Date.now()

            const checkState = () => {
                if (this.readyState === state) {
                    resolve(true)
                } else if (Date.now() - startTime >= timeout) {
                    resolve(false)
                } else {
                    setTimeout(checkState, 50)
                }
            }

            checkState()
        })
    }

    // WebSocket ready states
    static readonly CONNECTING = 0
    static readonly OPEN = 1
    static readonly CLOSING = 2
    static readonly CLOSED = 3
}
