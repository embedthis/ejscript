/**
 * Test Helpers and Utilities
 *
 * Provides helper functions for migrating Ejscript tests to Bun
 */

import { expect } from 'bun:test'
import { Path } from '../src/core/Path'

/**
 * Assert function for Ejscript compatibility
 * Maps to expect().toBeTruthy()
 */
export function assert(condition: any, message?: string): void {
  if (message) {
    expect(condition).withContext(message).toBeTruthy()
  } else {
    expect(condition).toBeTruthy()
  }
}

/**
 * Create temporary test file
 */
export function createTestFile(path: string, content: string = 'test data'): Path {
  const p = new Path(path)
  p.write(content)
  return p
}

/**
 * Create temporary directory
 */
export function createTestDir(path: string): Path {
  const p = new Path(path)
  p.makeDir()
  return p
}

/**
 * Clean up test file
 */
export function cleanupTestFile(path: string | Path): void {
  const p = path instanceof Path ? path : new Path(path)
  if (p.exists) {
    if (p.isDir) {
      p.removeAll()
    } else {
      p.remove()
    }
  }
}

/**
 * Generate random test path
 */
export function randomTestPath(prefix: string = 'test', suffix: string = '.tmp'): Path {
  const random = Math.random().toString(36).substring(7)
  return new Path(`/tmp/${prefix}-${random}${suffix}`)
}

/**
 * Measure execution time
 */
export async function measureTime(fn: () => void | Promise<void>): Promise<number> {
  const start = performance.now()
  await fn()
  return performance.now() - start
}

/**
 * Wait for condition to be true
 */
export async function waitFor(
  condition: () => boolean,
  timeout: number = 5000,
  interval: number = 100
): Promise<boolean> {
  const start = Date.now()

  while (Date.now() - start < timeout) {
    if (condition()) {
      return true
    }
    await new Promise(resolve => setTimeout(resolve, interval))
  }

  return false
}

/**
 * Compare arrays for equality
 */
export function arrayEquals<T>(a: T[], b: T[]): boolean {
  if (a.length !== b.length) return false

  for (let i = 0; i < a.length; i++) {
    if (a[i] !== b[i]) return false
  }

  return true
}

/**
 * Skip test if condition is true
 */
export function skipIf(condition: boolean, message: string = 'Test skipped'): void {
  if (condition) {
    console.log(`⊘ SKIP: ${message}`)
    // Bun test skip is handled by test.skipIf()
  }
}

/**
 * Test data constants
 */
export const TestConstants = {
  TestLength: 500,
  TestPath: '/tmp/ejscript-test-file.dat',
  TestDir: '/tmp/ejscript-test-dir',
  ShortDelay: 100,
  MediumDelay: 500,
  LongDelay: 1000
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
