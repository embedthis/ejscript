/**
 * BinaryStream - Binary stream I/O with endian control
 *
 * Provides binary data reading/writing with byte order control
 * @spec ejs
 * @stability evolving
 */

import { Stream } from './Stream'
import { ByteArray } from './ByteArray'

export enum Endian {
    LittleEndian = 'little',
    BigEndian = 'big'
}

export class BinaryStream extends Stream {
    private stream: Stream
    private _endian: Endian
    private _async: boolean = false

    /**
     * Little endian constant
     */
    static readonly LittleEndian = Endian.LittleEndian

    /**
     * Big endian constant
     */
    static readonly BigEndian = Endian.BigEndian

    /**
     * Create a BinaryStream wrapping another stream
     * @param stream Underlying stream to wrap
     * @param endian Byte order (default: little endian)
     */
    constructor(stream: Stream, endian: Endian = Endian.LittleEndian) {
        super()
        this.stream = stream
        this._endian = endian
    }

    get async(): boolean {
        return this._async
    }

    set async(enable: boolean) {
        this._async = enable
        this.stream.async = enable
    }

    /**
     * Get/set the byte order
     */
    get endian(): Endian {
        return this._endian
    }

    set endian(value: Endian) {
        this._endian = value
    }

    close(): void {
        this.stream.close()
    }

    flush(dir: number = Stream.WRITE): void {
        this.stream.flush(dir)
    }

    read(buffer: Uint8Array, offset: number = 0, count: number = -1): number | null {
        return this.stream.read(buffer, offset, count)
    }

    write(...args: any[]): number {
        return this.stream.write(...args)
    }

    /**
     * Read a single byte
     * @returns Byte value (0-255) or null on EOF
     */
    readByte(): number | null {
        const buffer = new Uint8Array(1)
        const bytesRead = this.stream.read(buffer, 0, 1)
        return bytesRead ? buffer[0] : null
    }

    /**
     * Read a 16-bit integer
     * @returns Integer value or null on EOF
     */
    readInteger16(): number | null {
        const buffer = new Uint8Array(2)
        const bytesRead = this.stream.read(buffer, 0, 2)
        if (!bytesRead || bytesRead < 2) return null

        const view = new DataView(buffer.buffer)
        return view.getInt16(0, this._endian === Endian.LittleEndian)
    }

    /**
     * Alias for readInteger16
     */
    readShort(): number | null {
        return this.readInteger16()
    }

    /**
     * Read a string of specified length
     * @param count Number of bytes to read
     * @returns String or null on EOF
     */
    readString(count: number): string | null {
        const buffer = new Uint8Array(count)
        const bytesRead = this.stream.read(buffer, 0, count)
        if (!bytesRead || bytesRead === 0) return null

        const decoder = new TextDecoder('utf-8')
        return decoder.decode(buffer.subarray(0, bytesRead))
    }

    /**
     * Read a 32-bit integer
     * @returns Integer value or null on EOF
     */
    readInteger32(): number | null {
        const buffer = new Uint8Array(4)
        const bytesRead = this.stream.read(buffer, 0, 4)
        if (!bytesRead || bytesRead < 4) return null

        const view = new DataView(buffer.buffer)
        return view.getInt32(0, this._endian === Endian.LittleEndian)
    }

    /**
     * Read a 64-bit integer
     * @returns BigInt value or null on EOF
     */
    readInteger64(): bigint | null {
        const buffer = new Uint8Array(8)
        const bytesRead = this.stream.read(buffer, 0, 8)
        if (!bytesRead || bytesRead < 8) return null

        const view = new DataView(buffer.buffer)
        return view.getBigInt64(0, this._endian === Endian.LittleEndian)
    }

    /**
     * Read a 64-bit double
     * @returns Double value or null on EOF
     */
    readDouble(): number | null {
        const buffer = new Uint8Array(8)
        const bytesRead = this.stream.read(buffer, 0, 8)
        if (!bytesRead || bytesRead < 8) return null

        const view = new DataView(buffer.buffer)
        return view.getFloat64(0, this._endian === Endian.LittleEndian)
    }

    /**
     * Read a 32-bit float
     * @returns Float value or null on EOF
     */
    readFloat(): number | null {
        const buffer = new Uint8Array(4)
        const bytesRead = this.stream.read(buffer, 0, 4)
        if (!bytesRead || bytesRead < 4) return null

        const view = new DataView(buffer.buffer)
        return view.getFloat32(0, this._endian === Endian.LittleEndian)
    }

    /**
     * Write a single byte
     * @param value Byte value (0-255)
     * @returns Number of bytes written
     */
    writeByte(value: number): number {
        const buffer = new Uint8Array([value & 0xFF])
        return this.stream.write(buffer)
    }

    /**
     * Write a 16-bit integer
     * @param value Integer value
     * @returns Number of bytes written
     */
    writeInteger16(value: number): number {
        const buffer = new ArrayBuffer(2)
        const view = new DataView(buffer)
        view.setInt16(0, value, this._endian === Endian.LittleEndian)
        return this.stream.write(new Uint8Array(buffer))
    }

    /**
     * Alias for writeInteger16
     */
    writeShort(value: number): number {
        return this.writeInteger16(value)
    }

    /**
     * Write a 32-bit integer
     * @param value Integer value
     * @returns Number of bytes written
     */
    writeInteger32(value: number): number {
        const buffer = new ArrayBuffer(4)
        const view = new DataView(buffer)
        view.setInt32(0, value, this._endian === Endian.LittleEndian)
        return this.stream.write(new Uint8Array(buffer))
    }

    /**
     * Write a 64-bit integer
     * @param value BigInt value
     * @returns Number of bytes written
     */
    writeInteger64(value: bigint): number {
        const buffer = new ArrayBuffer(8)
        const view = new DataView(buffer)
        view.setBigInt64(0, value, this._endian === Endian.LittleEndian)
        return this.stream.write(new Uint8Array(buffer))
    }

    /**
     * Write a 64-bit double
     * @param value Double value
     * @returns Number of bytes written
     */
    writeDouble(value: number): number {
        const buffer = new ArrayBuffer(8)
        const view = new DataView(buffer)
        view.setFloat64(0, value, this._endian === Endian.LittleEndian)
        return this.stream.write(new Uint8Array(buffer))
    }

    /**
     * Write a 32-bit float
     * @param value Float value
     * @returns Number of bytes written
     */
    writeFloat(value: number): number {
        const buffer = new ArrayBuffer(4)
        const view = new DataView(buffer)
        view.setFloat32(0, value, this._endian === Endian.LittleEndian)
        return this.stream.write(new Uint8Array(buffer))
    }

    on(name: string, observer: Function): this {
        this.stream.on(name, observer)
        return this
    }

    off(name: string, observer: Function): void {
        this.stream.off(name, observer)
    }
}
