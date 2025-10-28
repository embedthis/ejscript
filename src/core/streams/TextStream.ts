/**
 * TextStream - Text-based stream I/O
 *
 * Wraps a binary stream to provide text-based reading and writing
 * @spec ejs
 * @stability evolving
 */

import { Stream } from './Stream'
import { ByteArray } from './ByteArray'

export class TextStream extends Stream {
    private stream: Stream
    private encoding: string
    private _async: boolean = false

    /**
     * Create a TextStream wrapping another stream
     * @param stream Underlying stream to wrap
     * @param encoding Text encoding (default 'utf-8')
     */
    constructor(stream: Stream, encoding: string = 'utf-8') {
        super()
        this.stream = stream
        this.encoding = encoding
    }

    get async(): boolean {
        return this._async
    }

    set async(enable: boolean) {
        this._async = enable
        this.stream.async = enable
    }

    async close(): Promise<void> {
        const result = this.stream.close()
        if (result instanceof Promise) {
            await result
        }
    }

    flush(dir: number = Stream.WRITE): void {
        this.stream.flush(dir)
    }

    /**
     * Read data into a buffer
     * @param buffer Buffer to read into
     * @param offset Offset in buffer
     * @param count Number of bytes to read
     * @returns Number of bytes read, or null on EOF
     */
    async read(buffer: Uint8Array, offset: number = 0, count: number = -1): Promise<number | null> {
        const result = this.stream.read(buffer, offset, count)
        return result instanceof Promise ? await result : result
    }

    /**
     * Read a single line of text
     * @returns Line of text without newline, or null on EOF
     */
    async readLine(): Promise<string | null> {
        const buffer = new ByteArray(1024)
        let line = ''
        let byte: number | null

        while (true) {
            const tempBuffer = new Uint8Array(1)
            const bytesRead = await this.read(tempBuffer, 0, 1)

            if (bytesRead === null || bytesRead === 0) {
                return line.length > 0 ? line : null
            }

            byte = tempBuffer[0]

            if (byte === 0x0A) { // \n
                break
            }

            if (byte === 0x0D) { // \r
                // Peek ahead for \n
                const nextBuffer = new Uint8Array(1)
                const nextRead = await this.read(nextBuffer, 0, 1)
                if (nextRead && nextBuffer[0] !== 0x0A) {
                    // Not a \r\n pair, this is unusual but handle it
                    line += String.fromCharCode(byte)
                    line += String.fromCharCode(nextBuffer[0])
                }
                break
            }

            line += String.fromCharCode(byte)
        }

        return line
    }

    /**
     * Read all lines from the stream
     * @returns Array of lines
     */
    async readLines(): Promise<string[]> {
        const lines: string[] = []
        let line: string | null

        while ((line = await this.readLine()) !== null) {
            lines.push(line)
        }

        return lines
    }

    /**
     * Read a string of specified length
     * @param count Number of characters to read (-1 for all)
     * @returns String read from stream, or null on EOF
     */
    async readString(count: number = -1): Promise<string | null> {
        if (count === 0) return ''

        const buffer = new ByteArray(count > 0 ? count : 4096)
        const bytesRead = await this.read(buffer as Uint8Array, 0, count)

        if (bytesRead === null || bytesRead === 0) {
            return null
        }

        const decoder = new TextDecoder(this.encoding as any)
        return decoder.decode(buffer.subarray(0, bytesRead))
    }

    /**
     * Write data to the stream
     * @param ...args Data items to write (converted to strings)
     * @returns Number of bytes written
     */
    async write(...args: any[]): Promise<number> {
        let totalWritten = 0

        for (const arg of args) {
            const str = String(arg)
            const encoder = new TextEncoder()
            const bytes = encoder.encode(str)

            const result = this.stream.write(bytes)
            const written = result instanceof Promise ? await result : result
            totalWritten += written
        }

        return totalWritten
    }

    /**
     * Write lines of text (each arg becomes a separate line)
     * @param ...args Data items to write (each followed by newline)
     * @returns Number of bytes written
     */
    async writeLine(...args: any[]): Promise<number> {
        let totalWritten = 0

        for (const arg of args) {
            totalWritten += await this.write(arg)
            totalWritten += await this.write('\n')
        }

        return totalWritten
    }

    on(name: string, observer: Function): this {
        this.stream.on(name, observer)
        return this
    }

    off(name: string, observer: Function): void {
        this.stream.off(name, observer)
    }
}
