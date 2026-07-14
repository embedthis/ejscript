/**
 * Ejscript Core API for Bun
 *
 * Main entry point for the Ejscript core module.
 * Exports all core classes and utilities.
 */

// Core application and environment
export { App } from './core/App.js'
export { System } from './core/System.js'
export { Config } from './core/Config.js'
export { Args } from './core/Args.js'
export { JSON } from './core/JSON.js'

// File system and I/O
export { Path } from './core/Path.js'
export { File } from './core/File.js'
export { FileSystem } from './core/FileSystem.js'

// Streams
export { Stream } from './core/streams/Stream.js'
export { TextStream } from './core/streams/TextStream.js'
export { BinaryStream } from './core/streams/BinaryStream.js'
export { ByteArray } from './core/streams/ByteArray.js'

// Networking
export { Http } from './core/Http.js'
export { Socket } from './core/Socket.js'
export { WebSocket } from './core/WebSocket.js'
export { WebSocket as EjsWebSocket } from './core/WebSocket.js'
export { Uri } from './core/utilities/Uri.js'

// Utilities
export { Logger } from './core/utilities/Logger.js'
export { MprLog } from './core/utilities/MprLog.js'
export { Timer } from './core/utilities/Timer.js'
export { Cmd } from './core/utilities/Cmd.js'
export { Cache } from './core/utilities/Cache.js'
export { LocalCache } from './core/utilities/LocalCache.js'
export { Memory } from './core/utilities/Memory.js'
export { GC } from './core/utilities/GC.js'
export { Inflector } from './core/utilities/Inflector.js'

// Async/Concurrency
export { Worker } from './core/async/Worker.js'
export { Emitter } from './core/async/Emitter.js'

// Type extensions
export * from './core/types/StringExtensions.js'
export * from './core/types/ArrayExtensions.js'
export * from './core/types/ObjectExtensions.js'
export * from './core/types/DateExtensions.js'
export * from './core/types/NumberExtensions.js'

// Global functions
export {
    blend,
    clone,
    format,
    md5,
    sha256,
    dump,
    dumpAll,
    dumpDef,
    print,
    prints,
    printf,
    printHash,
    hashcode,
    assert,
    base64,
    base64Decode,
    parse,
    parseFloat,
    parseInt,
    isNaN,
    isFinite,
    instanceOf,
    evalScript,
    load,
    setIntervalTimer,
    clearIntervalTimer,
    setTimeoutTimer,
    clearTimeoutTimer
} from './core/utilities/Global.js'

// JSON serialization (exported from JSON class as well)
export { serialize, deserialize } from './core/JSON.js'

// Re-export the ejs namespace
export { ejs } from './globals.js'
