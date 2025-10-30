/**
 * Test Configuration
 *
 * Provides configuration and test data paths
 */

import { tmpdir } from 'os'
import { Path } from '../src/core/Path'
import { Config } from '../src/core/Config'

export const TestConfig = {
  /**
   * Test data directory
   */
  testDataDir: new Path(__dirname).join('fixtures'),

  /**
   * Temporary test directory
   */
  tmpDir: new Path(tmpdir()).join('ejscript-bun-tests'),

  /**
   * Test file paths
   */
  files: {
    testDat: new Path(__dirname).join('fixtures', 'file.dat'),
    testXml: new Path(__dirname).join('fixtures', 'file.xml'),
    testJson: new Path(__dirname).join('fixtures', 'test.json'),
  },

  /**
   * HTTP test configuration
   */
  http: {
    // Use httpbin.org for HTTP testing
    baseUrl: 'https://httpbin.org',
    timeout: 10000,
    // Mock server settings (if needed)
    mockPort: 8888,
    mockHost: 'localhost'
  },

  /**
   * Test timeouts
   */
  timeouts: {
    short: 1000,
    medium: 5000,
    long: 10000
  },

  /**
   * Platform-specific settings
   */
  platform: {
    os: Config.OS,
    isWindows: Config.OS === 'win32',
    isMac: Config.OS === 'darwin',
    isLinux: Config.OS === 'linux',
    isUnix: Config.OS !== 'win32'
  },

  /**
   * Skip conditions
   */
  skip: {
    // Skip tests that require specific platform
    requiresWindows: Config.OS !== 'win32',
    requiresUnix: Config.OS === 'win32',
    requiresMac: Config.OS !== 'darwin',
    requiresLinux: Config.OS !== 'linux',

    // Skip tests that require external services
    requiresNetwork: process.env.SKIP_NETWORK_TESTS === 'true',
    requiresHttpServer: process.env.SKIP_HTTP_TESTS === 'true'
  }
}

/**
 * Setup test environment
 */
export function setupTestEnvironment(): void {
  // Create tmp directory if it doesn't exist
  if (!TestConfig.tmpDir.exists) {
    TestConfig.tmpDir.makeDir()
  }

  // Create fixtures directory if it doesn't exist
  if (!TestConfig.testDataDir.exists) {
    TestConfig.testDataDir.makeDir()
  }
}

/**
 * Cleanup test environment
 */
export function cleanupTestEnvironment(): void {
  // Remove temporary test files
  if (TestConfig.tmpDir.exists) {
    TestConfig.tmpDir.removeAll()
  }
}
