/**
 * Path Tests
 * Migrated from src/core/test/path/*.tst
 */

import { test, expect, describe, beforeAll, afterAll } from '@embedthis/testme'
import { tmpdir } from 'os'
import { Path } from '../../src/core/Path'
import { assert, createTestFile, createTestFileSync, cleanupTestFile, randomTestPath, Platform } from '../helpers'
import { TestConfig } from '../config'

await describe('Path', async () => {
  let testFile: Path
  let testDir: Path

  beforeAll(() => {
    // Create test fixtures with unique names to avoid parallel test conflicts
    // Use SYNC operations to avoid async/sync cache coherency issues
    const fs = require('fs')

    testFile = createTestFileSync(new Path(tmpdir()).join(`ejscript-path-test-${process.pid}.dat`).name, 'test data content')
    testDir = new Path(tmpdir()).join(`ejscript-path-test-dir-${process.pid}`)
    fs.mkdirSync(testDir.name, { recursive: true })

    // Verify fixtures are immediately accessible (no retry needed with sync operations)
    if (!testFile.exists || !testDir.exists) {
      throw new Error(`Test fixtures not accessible: testFile.exists=${testFile.exists}, testDir.exists=${testDir.exists}`)
    }
  })

  afterAll(async () => {
    // Cleanup
    await cleanupTestFile(testFile)
    await cleanupTestFile(testDir)
  })

  await describe('Constructor', () => {
    test('creates path from string', () => {
      const p = new Path('/a/b/c')
      expect(p.name).toBe('/a/b/c')
    })

    test('creates path from empty string', () => {
      const p = new Path('')
      expect(p.name).toBe('')
    })

    test('creates path from another Path', () => {
      const p1 = new Path('/a/b/c')
      const p2 = new Path(p1.name)
      expect(p2.name).toBe('/a/b/c')
    })

    test('defaults to current directory', () => {
      const p = new Path()
      expect(p.name).toBe('.')
    })
  })

  await describe('Basic Properties', () => {
    test('exists checks file existence', () => {
      expect(testFile.exists).toBe(true)
      expect(new Path('/nonexistent/file').exists).toBe(false)
    })

    test('isRegular identifies regular files', () => {
      expect(testFile.isRegular).toBe(true)
      expect(testDir.isRegular).toBe(false)
    })

    test('isDir identifies directories', () => {
      expect(testDir.isDir).toBe(true)
      expect(testFile.isDir).toBe(false)
    })

    test('isAbsolute checks if path is absolute', () => {
      expect(new Path('/a/b/c').isAbsolute).toBe(true)
      expect(new Path('a/b/c').isAbsolute).toBe(false)
    })

    test('isRelative checks if path is relative', () => {
      expect(new Path('a/b/c').isRelative).toBe(true)
      expect(new Path('/a/b/c').isRelative).toBe(false)
    })

    test('size returns file size', () => {
      expect(testFile.size).toBeGreaterThan(0)
    })

    test('length returns path length', () => {
      const p = new Path('/a/b/c')
      expect(p.length).toBe(6)
    })

    test('created returns creation date', () => {
      expect(testFile.created).toBeInstanceOf(Date)
    })

    test('accessed returns access date', () => {
      expect(testFile.accessed).toBeInstanceOf(Date)
    })

    test('modified returns modification date', () => {
      expect(testFile.modified).toBeInstanceOf(Date)
    })
  })

  await describe('Path Components', () => {
    test('basename returns file name', () => {
      expect(new Path('/a/b/c.dat').basename.name).toBe('c.dat')
      expect(new Path('/a/b/c').basename.name).toBe('c')
    })

    test('dirname returns directory', () => {
      expect(new Path('/a/b/c.dat').dirname.name).toBe('/a/b')
      expect(new Path('/a/b/c').dirname.name).toBe('/a/b')
    })

    test('parent returns parent directory', () => {
      const p = new Path('/a/b/c.dat')
      expect(p.parent.basename.name).toBe('b')
    })

    test('extension returns file extension', () => {
      expect(new Path('/a/b/c.dat').extension).toBe('dat')
      expect(new Path('/a/b/c.tar.gz').extension).toBe('gz')
      expect(new Path('/a/b/c').extension).toBe('')
    })

    test('root returns root directory', () => {
      const p = new Path('/a/b/c')
      expect(p.root.name).toBeTruthy()
    })
  })

  await describe('Path Conversions', () => {
    test('absolute returns absolute path', () => {
      const p = new Path('test.txt')
      const abs = p.absolute
      expect(abs.isAbsolute).toBe(true)
    })

    test('relative returns relative path', () => {
      const p = new Path('file.dat')
      expect(p.relative.name).toBe('file.dat')
    })

    test('portable uses forward slashes', () => {
      const p = new Path('c:\\dir\\file.txt')
      expect(p.portable.name).toBe('c:/dir/file.txt')
    })

    test.skipIf(!Platform.isWindows)('windows uses backslashes', () => {
      const p = new Path('/dir/file.txt')
      const win = p.windows
      expect(win.name).toContain('\\')
    })

    test('normalize normalizes path', () => {
      const p = new Path('/a/./b/../c')
      const norm = p.normalize
      expect(norm.name).not.toContain('..')
    })
  })

  await describe('Path Operations', () => {
    test('join combines paths', () => {
      const p = new Path('/tmp')
      const joined = p.join('subdir', 'file.txt')
      expect(joined.name).toContain('tmp')
      expect(joined.name).toContain('subdir')
      expect(joined.name).toContain('file.txt')
    })

    test('joinExt adds extension', () => {
      const p = new Path('file')
      const withExt = p.joinExt('.txt')
      expect(withExt.name).toBe('file.txt')
    })

    test('joinExt with dot', () => {
      const p = new Path('file')
      const withExt = p.joinExt('txt')
      expect(withExt.name).toBe('file.txt')
    })

    test('trimExt removes extension', () => {
      const p = new Path('file.txt')
      const trimmed = p.trimExt()
      expect(trimmed.name).toBe('file')
    })

    test('replaceExt changes extension', () => {
      const p = new Path('file.txt')
      const replaced = p.replaceExt('.md')
      expect(replaced.name).toBe('file.md')
    })

    test('replace performs string replacement', () => {
      const p = new Path('/home/user/file.txt')
      const replaced = p.replace('user', 'admin')
      expect(replaced.name).toBe('/home/admin/file.txt')
    })

    test('trimStart removes prefix', () => {
      const p = new Path('/home/user/file.txt')
      const trimmed = p.trimStart('/home')
      expect(trimmed.name).toBe('/user/file.txt')
    })

    test('trimEnd removes suffix', () => {
      const p = new Path('/path/to/file.txt')
      const trimmed = p.trimEnd('.txt')
      expect(trimmed.name).toBe('/path/to/file')
    })
  })

  await describe('File Operations', () => {
    test('copy copies file', async () => {
      const src = randomTestPath('copy-source')
      await src.write('copy test')
      const dest = randomTestPath('copy-dest')

      await src.copy(dest)
      expect(dest.exists).toBe(true)
      expect(await dest.readString()).toBe('copy test')

      await cleanupTestFile(src)
      await cleanupTestFile(dest)
    })

    test('rename moves file', async () => {
      const src = randomTestPath('rename-source')
      await src.write('rename test')
      const dest = randomTestPath('rename-dest')

      await src.rename(dest)
      expect(src.exists).toBe(false)
      expect(dest.exists).toBe(true)

      await cleanupTestFile(dest)
    })

    test('remove deletes file', async () => {
      const file = randomTestPath('remove-test')
      await file.write('remove me')
      expect(file.exists).toBe(true)

      const result = await file.remove()
      expect(result).toBe(true)
      expect(file.exists).toBe(false)
    })

    test('removeAll deletes directory tree', async () => {
      const dir = randomTestPath('removeall-test')
      await dir.makeDir()
      await dir.join('subdir').makeDir()
      await dir.join('file.txt').write('test')

      const result = await dir.removeAll()
      expect(result).toBe(true)
      expect(dir.exists).toBe(false)
    })

    test('makeDir creates directory', async () => {
      const dir = randomTestPath('makedir-test')
      const result = await dir.makeDir()
      expect(result).toBe(true)
      expect(dir.exists).toBe(true)
      expect(dir.isDir).toBe(true)

      await cleanupTestFile(dir)
    })

    test('makeDir creates intermediate directories', async () => {
      const basedir = randomTestPath('makedir-base')
      const dir = basedir.join('sub1').join('sub2')
      const result = await dir.makeDir()
      expect(result).toBe(true)
      expect(dir.exists).toBe(true)

      await cleanupTestFile(basedir)
    })
  })

  await describe('File Content Operations', () => {
    test('write writes string to file', async () => {
      const file = randomTestPath('write')
      await file.write('Hello World')

      expect(file.exists).toBe(true)
      expect(await file.readString()).toBe('Hello World')

      await cleanupTestFile(file)
    })

    test('append appends to file', async () => {
      const file = randomTestPath('append')
      await file.write('Line 1\n')
      await file.append('Line 2\n')

      expect(await file.readString()).toBe('Line 1\nLine 2\n')

      await cleanupTestFile(file)
    })

    test('readString reads file content', async () => {
      const unique = `${process.pid}-${Date.now()}-${Math.random().toString(36).slice(2)}`
      const file = await createTestFile(new Path(tmpdir()).join(`read-test-${unique}.txt`).name, 'Read this content')
      const content = await file.readString()

      expect(content).toBe('Read this content')

      await cleanupTestFile(file)
    })

    test('readBytes reads binary data', async () => {
      const unique = `${process.pid}-${Date.now()}-${Math.random().toString(36).slice(2)}`
      const file = await createTestFile(new Path(tmpdir()).join(`readbytes-test-${unique}.dat`).name, 'Binary data')
      const bytes = await file.readBytes()

      expect(bytes).toBeInstanceOf(Uint8Array)
      expect(bytes!.length).toBeGreaterThan(0)

      await cleanupTestFile(file)
    })

    test('readLines reads file as lines', async () => {
      const unique = `${process.pid}-${Date.now()}-${Math.random().toString(36).slice(2)}`
      const file = await createTestFile(new Path(tmpdir()).join(`readlines-test-${unique}.txt`).name, 'Line 1\nLine 2\nLine 3')
      const lines = await file.readLines()

      expect(lines).toHaveLength(3)
      expect(lines![0]).toBe('Line 1')
      expect(lines![1]).toBe('Line 2')
      expect(lines![2]).toBe('Line 3')

      await cleanupTestFile(file)
    })

    test('readJSON parses JSON file', async () => {
      const unique = `${process.pid}-${Date.now()}-${Math.random().toString(36).slice(2)}`
      const file = await createTestFile(new Path(tmpdir()).join(`readjson-test-${unique}.json`).name, '{"name":"test","value":42}')
      const data = await file.readJSON()

      expect(data.name).toBe('test')
      expect(data.value).toBe(42)

      await cleanupTestFile(file)
    })
  })

  await describe('Path Comparison', () => {
    test('same compares paths', () => {
      const p1 = new Path(tmpdir()).join('test.txt')
      const p2 = new Path(tmpdir()).join('test.txt')
      const p3 = new Path(tmpdir()).join('other.txt')

      expect(p1.same(p2)).toBe(true)
      expect(p1.same(p3)).toBe(false)
    })

    test('startsWith checks prefix', () => {
      const p = new Path('/home/user/file.txt')
      expect(p.startsWith('/home')).toBe(true)
      expect(p.startsWith('/opt')).toBe(false)
    })

    test('endsWith checks suffix', () => {
      const p = new Path('/path/to/file.txt')
      expect(p.endsWith('.txt')).toBe(true)
      expect(p.endsWith('.md')).toBe(false)
    })

    test('contains checks substring', () => {
      const p = new Path('/home/user/documents/file.txt')
      expect(p.contains('user')).toBe(true)
      expect(p.contains('admin')).toBe(false)
    })
  })

  await describe('Path Attributes', () => {
    test('mimeType returns MIME type', () => {
      expect(new Path('file.html').mimeType).toBe('text/html')
      expect(new Path('file.json').mimeType).toBe('application/json')
      expect(new Path('file.png').mimeType).toBe('image/png')
      expect(new Path('file.unknown').mimeType).toBe('application/octet-stream')
    })

    test('separator returns path separator', () => {
      const p1 = new Path('/a/b/c')
      expect(p1.separator).toBe('/')

      const p2 = new Path('c:\\a\\b')
      expect(p2.separator).toBe('\\')
    })

    test('perms gets/sets permissions', async () => {
      const file = randomTestPath('perms-test')
      await file.write('perms')

      // Get permissions
      const perms = file.perms
      expect(perms).not.toBeNull()

      // Set permissions
      file.perms = 0o644
      expect(file.perms).toBe(0o644)

      await cleanupTestFile(file)
    })
  })

  await describe('toString and Conversion', () => {
    test('toString returns path string', () => {
      const p = new Path('/a/b/c')
      expect(p.toString()).toBe('/a/b/c')
    })

    test('toLowerCase converts to lowercase', () => {
      const p = new Path('/Path/To/FILE.TXT')
      expect(p.toLowerCase().name).toBe('/path/to/file.txt')
    })

    test('toJSON returns JSON string', () => {
      const p = new Path('/test/path')
      const json = p.toJSON()
      expect(json).toContain('/test/path')
    })
  })

  await describe('Iterator', () => {
    test('iterates directory entries', async () => {
      const dir = randomTestPath('iter-test')
      await dir.makeDir()
      await dir.join('file1.txt').write('test1')
      await dir.join('file2.txt').write('test2')

      const entries = Array.from(dir)
      expect(entries.length).toBe(2)

      await cleanupTestFile(dir)
    })
  })
})
