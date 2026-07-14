/**
 * Test Helpers and Utilities
 *
 * Shared helpers for the TestMe (.tst.ts) suite.
 */

import { expect } from 'testme'
import { writeFileSync } from 'fs'
import { tmpdir } from 'os'
import { Path } from '../src/core/Path'

/**
 * Assert function for Ejscript compatibility
 * Maps to expect().toBeTruthy(), reporting message on failure
 */
export function assert(condition: any, message?: string): void {
  if (!condition && message) {
    throw new Error(`Assertion failed: ${message}`)
  }
  expect(condition).toBeTruthy()
}

/**
 * Create temporary test file
 */
export async function createTestFile(path: string, content: string = 'test data'): Promise<Path> {
  const p = new Path(path)
  await p.write(content)
  return p
}

/**
 * Create test file synchronously (for non-async tests)
 */
export function createTestFileSync(path: string, content: string = 'test data'): Path {
  writeFileSync(path, content)
  return new Path(path)
}

/**
 * Clean up test file
 */
export async function cleanupTestFile(path: string | Path): Promise<void> {
  const p = path instanceof Path ? path : new Path(path)
  if (p.exists) {
    if (p.isDir) {
      await p.removeAll()
    } else {
      await p.remove()
    }
  }
}

/**
 * Generate random test path
 */
export function randomTestPath(prefix: string = 'test', suffix: string = '.tmp'): Path {
  const random = Math.random().toString(36).substring(7)
  return new Path(tmpdir()).join(`${prefix}-${random}${suffix}`)
}

/**
 * Platform detection helpers
 */
export const Platform = {
  isWindows: process.platform === 'win32',
  isMac: process.platform === 'darwin',
  isLinux: process.platform === 'linux',
  isUnix: process.platform !== 'win32'
}
