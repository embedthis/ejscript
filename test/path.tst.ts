/**
 * Path class tests
 */

import { test, expect, describe } from '@embedthis/testme'
import { Path } from '../src/core/Path'

await describe('Path', async () => {
    test('constructor creates path', () => {
        const path = new Path('/tmp/test.txt')
        expect(path.name).toBe('/tmp/test.txt')
    })

    test('absolute returns absolute path', () => {
        const path = new Path('test.txt')
        const abs = path.absolute
        expect(abs.isAbsolute).toBe(true)
    })

    test('basename returns file name', () => {
        const path = new Path('/tmp/test.txt')
        expect(path.basename.name).toBe('test.txt')
    })

    test('dirname returns directory', () => {
        const path = new Path('/tmp/test.txt')
        expect(path.dirname.name).toBe('/tmp')
    })

    test('extension returns file extension', () => {
        const path = new Path('test.txt')
        expect(path.extension).toBe('txt')
    })

    test('join combines paths', () => {
        const path = new Path('/tmp')
        const joined = path.join('subdir', 'file.txt')
        expect(joined.name).toContain('tmp')
        expect(joined.name).toContain('subdir')
        expect(joined.name).toContain('file.txt')
    })

    test('portable returns Unix-style path', () => {
        const path = new Path('C:\\Users\\test\\file.txt')
        const portable = path.portable
        expect(portable.name).toBe('C:/Users/test/file.txt')
    })

    test('parent returns parent directory', () => {
        const path = new Path('/tmp/subdir/file.txt')
        expect(path.parent.name).toContain('subdir')
    })

    test('trimExt removes extension', () => {
        const path = new Path('test.txt')
        const trimmed = path.trimExt()
        expect(trimmed.name).toBe('test')
    })

    test('replaceExt changes extension', () => {
        const path = new Path('test.txt')
        const replaced = path.replaceExt('.md')
        expect(replaced.name).toBe('test.md')
    })
})
