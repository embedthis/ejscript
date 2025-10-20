import { describe, it, expect } from 'testme'
import { Uri } from '../src/core/utilities/Uri'
import { Path } from '../src/core/Path'

await describe('Uri', async () => {
    await describe('Construction', async () => {
        it('creates Uri from string URL', () => {
            const uri = new Uri('http://example.com/path')
            expect(uri.toString()).toBe('http://example.com/path')
        })

        it('creates Uri from string path', () => {
            const uri = new Uri('/local/path')
            expect(uri.path).toBe('/local/path')
        })

        it('creates Uri from components object', () => {
            const uri = new Uri({
                scheme: 'https',
                host: 'example.com',
                port: 443,
                path: '/api/users',
                query: 'page=1',
                reference: 'section1'
            })
            expect(uri.scheme).toBe('https')
            expect(uri.host).toBe('example.com')
            expect(uri.port).toBe(443)
            expect(uri.path).toBe('/api/users')
            expect(uri.query).toBe('page=1')
            expect(uri.reference).toBe('section1')
        })

        it('parses full URL with all components', () => {
            const uri = new Uri('https://user@example.com:8080/path/to/resource?query=value#anchor')
            expect(uri.scheme).toBe('https')
            expect(uri.host).toBe('example.com')
            expect(uri.port).toBe(8080)
            expect(uri.path).toBe('/path/to/resource')
            expect(uri.query).toBe('query=value')
            expect(uri.hash).toBe('anchor')
        })
    })

    await describe('Properties', async () => {
        it('has scheme property', () => {
            const uri = new Uri('https://example.com')
            expect(uri.scheme).toBe('https')
        })

        it('has host property', () => {
            const uri = new Uri('http://example.com')
            expect(uri.host).toBe('example.com')
        })

        it('has port property', () => {
            const uri = new Uri('http://example.com:8080')
            expect(uri.port).toBe(8080)
        })

        it('has path property', () => {
            const uri = new Uri('http://example.com/path/to/file')
            expect(uri.path).toBe('/path/to/file')
        })

        it('has query property', () => {
            const uri = new Uri('http://example.com?foo=bar')
            expect(uri.query).toBe('foo=bar')
        })

        it('has hash property', () => {
            const uri = new Uri('http://example.com#anchor')
            expect(uri.hash).toBe('anchor')
        })

        it('has reference property (alias for hash)', () => {
            const uri = new Uri('http://example.com#section')
            expect(uri.reference).toBe('section')
            expect(uri.reference).toBe(uri.hash)
        })

        it('has uri property', () => {
            const uri = new Uri('http://example.com/path')
            expect(uri.uri).toBe('http://example.com/path')
        })

        it('has address property', () => {
            const uri = new Uri('http://example.com:8080')
            expect(uri.address).toBe('example.com:8080')
        })

        it('has components property', () => {
            const uri = new Uri('https://example.com:9443/path?q=1#ref')
            const comp = uri.components
            expect(comp.scheme).toBe('https')
            expect(comp.host).toBe('example.com')
            expect(comp.port).toBe(9443)
            expect(comp.path).toBe('/path')
            expect(comp.query).toBe('q=1')
            expect(comp.reference).toBe('ref')
        })
    })

    await describe('Setters', async () => {
        it('can set scheme', () => {
            const uri = new Uri('http://example.com')
            uri.scheme = 'https'
            expect(uri.scheme).toBe('https')
        })

        it('can set host', () => {
            const uri = new Uri('http://example.com')
            uri.host = 'newhost.com'
            expect(uri.host).toBe('newhost.com')
        })

        it('can set port', () => {
            const uri = new Uri('http://example.com')
            uri.port = 8080
            expect(uri.port).toBe(8080)
        })

        it('can set path', () => {
            const uri = new Uri('http://example.com/old')
            uri.path = '/new'
            expect(uri.path).toBe('/new')
        })

        it('can set query', () => {
            const uri = new Uri('http://example.com')
            uri.query = 'foo=bar'
            expect(uri.query).toBe('foo=bar')
        })

        it('can set reference', () => {
            const uri = new Uri('http://example.com')
            uri.reference = 'anchor'
            expect(uri.reference).toBe('anchor')
        })
    })

    await describe('Boolean Checks', async () => {
        it('checks if URI is absolute', () => {
            const abs = new Uri('http://example.com/path')
            const rel = new Uri('/path')
            expect(abs.isAbsolute).toBe(true)
            expect(rel.isAbsolute).toBe(false)
        })

        it('checks if URI is relative', () => {
            const abs = new Uri('http://example.com/path')
            const rel = new Uri('/path')
            expect(abs.isRelative).toBe(false)
            expect(rel.isRelative).toBe(true)
        })

        it('checks if URI has scheme', () => {
            const with_scheme = new Uri('http://example.com')
            const without = new Uri('/path')
            expect(with_scheme.hasScheme).toBe(true)
            expect(without.hasScheme).toBe(false)
        })

        it('checks if URI has host', () => {
            const with_host = new Uri('http://example.com')
            const without = new Uri('/path')
            expect(with_host.hasHost).toBe(true)
            expect(without.hasHost).toBe(false)
        })

        it('checks if URI has port', () => {
            const with_port = new Uri('http://example.com:8080')
            const without = new Uri('http://example.com')
            expect(with_port.hasPort).toBe(true)
            expect(without.hasPort).toBe(false)
        })

        it('checks if URI has query', () => {
            const with_query = new Uri('http://example.com?foo=bar')
            const without = new Uri('http://example.com')
            expect(with_query.hasQuery).toBe(true)
            expect(without.hasQuery).toBe(false)
        })

        it('checks if URI has reference', () => {
            const with_ref = new Uri('http://example.com#anchor')
            const without = new Uri('http://example.com')
            expect(with_ref.hasReference).toBe(true)
            expect(without.hasReference).toBe(false)
        })

        it('checks if path has extension', () => {
            const with_ext = new Uri('/file.txt')
            const without = new Uri('/directory/')
            expect(with_ext.hasExtension).toBe(true)
            expect(without.hasExtension).toBe(false)
        })

        it('checks if path is directory', () => {
            const dir = new Uri('/path/')
            const file = new Uri('/path/file.txt')
            expect(dir.isDir).toBe(true)
            expect(file.isDir).toBe(false)
        })

        it('checks if path is regular file', () => {
            const dir = new Uri('/path/')
            const file = new Uri('/path/file.txt')
            expect(dir.isRegular).toBe(false)
            expect(file.isRegular).toBe(true)
        })
    })

    await describe('Path Operations', async () => {
        it('gets basename', () => {
            const uri = new Uri('/path/to/file.txt')
            expect(uri.basename.path).toBe('file.txt')
        })

        it('gets dirname', () => {
            const uri = new Uri('/path/to/file.txt')
            expect(uri.dirname.path).toBe('/path/to')
        })

        it('gets extension', () => {
            const uri = new Uri('/file.txt')
            expect(uri.extension).toBe('txt')
        })

        it('gets filename', () => {
            const uri = new Uri('/path/document.pdf')
            expect(uri.filename).toBe('document')
        })

        it('gets local path', () => {
            const uri = new Uri('file:///path/to/file')
            expect(uri.local).toBeInstanceOf(Path)
        })
    })

    await describe('URI Manipulation', async () => {
        it('creates absolute URI from relative', () => {
            const rel = new Uri('/path/file.txt')
            const abs = rel.absolute('http://example.com')
            expect(abs.isAbsolute).toBe(true)
            expect(abs.scheme).toBe('http')
            expect(abs.host).toBe('example.com')
        })

        it('absolute returns same URI if already absolute', () => {
            const uri = new Uri('http://example.com/path')
            const abs = uri.absolute()
            expect(abs.toString()).toBe(uri.toString())
        })

        it('completes URI with missing components', () => {
            const incomplete = new Uri('/path')
            const complete = incomplete.complete('http://example.com:8080')
            expect(complete.scheme).toBe('http')
            expect(complete.host).toBe('example.com')
            expect(complete.port).toBe(8080)
            expect(complete.path).toBe('/path')
        })

        it('normalizes URI path', () => {
            const uri = new Uri('/path/./to/../file.txt')
            const normalized = uri.normalize()
            expect(normalized.path).toContain('path')
        })

        it('makes URI relative', () => {
            const abs = new Uri('http://example.com/path')
            const rel = abs.relative()
            expect(rel.isRelative).toBe(true)
            expect(rel.path).toBe('/path')
        })

        it('makes URI relative to another', () => {
            const uri = new Uri('http://example.com/path/to/file.txt')
            const base = new Uri('http://example.com/path')
            const rel = uri.relativeTo(base)
            expect(rel.path).toContain('to')
        })

        it('resolves paths', () => {
            const base = new Uri('/base')
            const resolved = base.resolve('subdir', 'file.txt')
            expect(resolved.path).toContain('subdir')
            expect(resolved.path).toContain('file.txt')
        })

        it('joins URI segments', () => {
            const base = new Uri('/base')
            const joined = base.join('path', 'to', 'file.txt')
            expect(joined.path).toContain('base')
            expect(joined.path).toContain('path')
            expect(joined.path).toContain('file.txt')
        })
    })

    await describe('Extension Operations', async () => {
        it('joins extension', () => {
            const uri = new Uri('/file')
            const with_ext = uri.joinExt('txt')
            expect(with_ext.path).toBe('/file.txt')
        })

        it('joins extension with dot', () => {
            const uri = new Uri('/file')
            const with_ext = uri.joinExt('.txt')
            expect(with_ext.path).toBe('/file.txt')
        })

        it('forces extension replacement', () => {
            const uri = new Uri('/file.old')
            const replaced = uri.joinExt('new', true)
            expect(replaced.path).toBe('/file.new')
        })

        it('replaces extension', () => {
            const uri = new Uri('/file.txt')
            const replaced = uri.replaceExt('md')
            expect(replaced.path).toBe('/file.md')
        })

        it('trims extension', () => {
            const uri = new Uri('/file.txt')
            const trimmed = uri.trimExt()
            expect(trimmed.path).toBe('/file')
        })
    })

    await describe('Trim Operations', async () => {
        it('trims end suffix', () => {
            const uri = new Uri('/path/file.bak')
            const trimmed = uri.trimEnd('.bak')
            expect(trimmed.path).toBe('/path/file')
        })

        it('trims start prefix', () => {
            const uri = new Uri('/prefix/path')
            const trimmed = uri.trimStart('/prefix')
            expect(trimmed.path).toBe('/path')
        })

        it('does not trim if suffix not found', () => {
            const uri = new Uri('/path/file.txt')
            const trimmed = uri.trimEnd('.md')
            expect(trimmed.path).toBe('/path/file.txt')
        })
    })

    await describe('Comparison', async () => {
        it('checks if URIs are the same', () => {
            const uri1 = new Uri('http://example.com/path')
            const uri2 = new Uri('http://example.com/path')
            expect(uri1.same(uri2)).toBe(true)
        })

        it('checks if URIs are different', () => {
            const uri1 = new Uri('http://example.com/path1')
            const uri2 = new Uri('http://example.com/path2')
            expect(uri1.same(uri2)).toBe(false)
        })

        it('compares with string', () => {
            const uri = new Uri('http://example.com/path')
            expect(uri.same('http://example.com/path')).toBe(true)
        })

        it('checks if path starts with prefix', () => {
            const uri = new Uri('/base/path/file')
            expect(uri.startsWith('/base')).toBe(true)
            expect(uri.startsWith('/other')).toBe(false)
        })

        it('checks if path ends with suffix', () => {
            const uri = new Uri('/path/file.txt')
            expect(uri.endsWith('.txt')).toBe(true)
            expect(uri.endsWith('.md')).toBe(false)
        })
    })

    await describe('MIME Type', async () => {
        it('returns MIME type for HTML', () => {
            const uri = new Uri('/file.html')
            expect(uri.mimeType).toBe('text/html')
        })

        it('returns MIME type for CSS', () => {
            const uri = new Uri('/style.css')
            expect(uri.mimeType).toBe('text/css')
        })

        it('returns MIME type for JavaScript', () => {
            const uri = new Uri('/script.js')
            expect(uri.mimeType).toBe('application/javascript')
        })

        it('returns MIME type for JSON', () => {
            const uri = new Uri('/data.json')
            expect(uri.mimeType).toBe('application/json')
        })

        it('returns MIME type for PNG', () => {
            const uri = new Uri('/image.png')
            expect(uri.mimeType).toBe('image/png')
        })

        it('returns MIME type for JPEG', () => {
            const uri = new Uri('/photo.jpg')
            expect(uri.mimeType).toBe('image/jpeg')
        })

        it('returns default MIME type for unknown', () => {
            const uri = new Uri('/file.unknown')
            expect(uri.mimeType).toBe('application/octet-stream')
        })
    })

    await describe('Static Encoding', async () => {
        it('encodes URI', () => {
            const encoded = Uri.encodeURI('http://example.com/path with spaces')
            expect(encoded).toContain('path%20with%20spaces')
        })

        it('decodes URI', () => {
            const decoded = Uri.decodeURI('http://example.com/path%20with%20spaces')
            expect(decoded).toContain('path with spaces')
        })

        it('encodes URI component', () => {
            const encoded = Uri.encodeURIComponent('hello?world&foo=bar')
            expect(encoded).not.toContain('?')
            expect(encoded).not.toContain('&')
            expect(encoded).not.toContain('=')
        })

        it('decodes URI component', () => {
            const decoded = Uri.decodeURIComponent('hello%3Fworld%26foo%3Dbar')
            expect(decoded).toBe('hello?world&foo=bar')
        })

        it('encode is alias for encodeURI', () => {
            const str = 'test string'
            expect(Uri.encode(str)).toBe(Uri.encodeURI(str))
        })

        it('decode is alias for decodeURI', () => {
            const str = 'test%20string'
            expect(Uri.decode(str)).toBe(Uri.decodeURI(str))
        })
    })

    await describe('Query Encoding', async () => {
        it('encodes object as query string', () => {
            const query = Uri.encodeObjects({ foo: 'bar', num: 42 })
            expect(query).toContain('foo=bar')
            expect(query).toContain('num=42')
        })

        it('encodes array as query string', () => {
            const query = Uri.encodeObjects(['a', 'b', 'c'])
            expect(query).toContain('0=a')
            expect(query).toContain('1=b')
            expect(query).toContain('2=c')
        })

        it('handles array values in object', () => {
            const query = Uri.encodeObjects({ tags: ['one', 'two', 'three'] })
            expect(query).toContain('tags=one')
            expect(query).toContain('tags=two')
            expect(query).toContain('tags=three')
        })

        it('skips null and undefined values', () => {
            const query = Uri.encodeObjects({ foo: 'bar', skip: null, also: undefined })
            expect(query).toContain('foo=bar')
            expect(query).not.toContain('skip')
            expect(query).not.toContain('also')
        })

        it('encodes nested objects as JSON', () => {
            const query = Uri.encodeObjects({ data: { nested: 'value' } })
            expect(query).toContain('data=')
        })

        it('encodeQuery is alias for encodeObjects', () => {
            const obj = { a: 1, b: 2 }
            expect(Uri.encodeQuery(obj)).toBe(Uri.encodeObjects(obj))
        })
    })

    await describe('Query Decoding', async () => {
        it('decodes query string to object', () => {
            const obj = Uri.decodeQuery('foo=bar&num=42')
            expect(obj.foo).toBe('bar')
            expect(obj.num).toBe('42')
        })

        it('handles leading question mark', () => {
            const obj = Uri.decodeQuery('?foo=bar')
            expect(obj.foo).toBe('bar')
        })

        it('handles empty query string', () => {
            const obj = Uri.decodeQuery('')
            expect(Object.keys(obj).length).toBe(0)
        })

        it('handles multiple values for same key', () => {
            const obj = Uri.decodeQuery('tag=one&tag=two&tag=three')
            expect(Array.isArray(obj.tag)).toBe(true)
            expect(obj.tag).toContain('one')
            expect(obj.tag).toContain('two')
            expect(obj.tag).toContain('three')
        })

        it('decodes URL-encoded values', () => {
            const obj = Uri.decodeQuery('msg=hello%20world')
            expect(obj.msg).toBe('hello world')
        })
    })

    await describe('Static Join', async () => {
        it('joins URI segments with /', () => {
            const uri = Uri.join('http://example.com', 'path', 'to', 'file.txt')
            expect(uri.toString()).toContain('path/to/file.txt')
        })

        it('handles segments with existing slashes', () => {
            const uri = Uri.join('http://example.com/', '/path/')
            expect(uri.toString()).toContain('example.com')
            expect(uri.toString()).toContain('path')
        })
    })

    await describe('Static Parse', async () => {
        it('parses URI string into components', () => {
            const parts = Uri.parse('https://example.com:9443/path?query=1#ref')
            expect(parts.scheme).toBe('https')
            expect(parts.host).toBe('example.com')
            expect(parts.port).toBe(9443)
            expect(parts.path).toBe('/path')
            expect(parts.query).toBe('query=1')
            expect(parts.hash).toBe('ref')
        })

        it('parses relative path', () => {
            const parts = Uri.parse('/local/path')
            expect(parts.scheme).toBeNull()
            expect(parts.host).toBeNull()
            expect(parts.path).toBe('/local/path')
        })
    })

    await describe('Static Format', async () => {
        it('formats components into URI string', () => {
            const uri = Uri.format({
                scheme: 'https',
                host: 'example.com',
                port: 443,
                path: '/api',
                query: 'v=1',
                hash: 'top'
            })
            expect(uri).toBe('https://example.com:443/api?v=1#top')
        })

        it('handles partial components', () => {
            const uri = Uri.format({
                path: '/local/file.txt'
            })
            expect(uri).toBe('/local/file.txt')
        })

        it('builds URL without port if not provided', () => {
            const uri = Uri.format({
                scheme: 'http',
                host: 'example.com',
                path: '/path'
            })
            expect(uri).toBe('http://example.com/path')
        })
    })

    await describe('toString', async () => {
        it('returns full URL for absolute URI', () => {
            const uri = new Uri('https://example.com:8080/path?query=1#ref')
            const str = uri.toString()
            expect(str).toContain('https')
            expect(str).toContain('example.com')
            expect(str).toContain('8080')
            expect(str).toContain('path')
        })

        it('returns path for relative URI', () => {
            const uri = new Uri('/local/path')
            expect(uri.toString()).toBe('/local/path')
        })
    })

    await describe('Edge Cases', async () => {
        it('handles file:// URLs', () => {
            const uri = new Uri('file:///path/to/file.txt')
            expect(uri.scheme).toBe('file')
            expect(uri.path).toBe('/path/to/file.txt')
        })

        it('handles localhost URLs', () => {
            const uri = new Uri('http://localhost:3000/api')
            expect(uri.host).toBe('localhost')
            expect(uri.port).toBe(3000)
        })

        it('handles IPv6 addresses', () => {
            const uri = new Uri('http://[::1]:8080/path')
            expect(uri.host).toBe('[::1]')
            expect(uri.port).toBe(8080)
        })

        it('handles empty path', () => {
            const uri = new Uri('http://example.com')
            expect(uri.path).toBeTruthy()
        })

        it('handles root path', () => {
            const uri = new Uri('http://example.com/')
            expect(uri.path).toBe('/')
        })
    })
})
