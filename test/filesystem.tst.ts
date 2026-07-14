import { describe, it, expect, beforeAll, afterAll } from '@embedthis/testme'
import { tmpdir } from 'os'
import { FileSystem } from '../src/core/FileSystem'
import { Path } from '../src/core/Path'

await describe('FileSystem', async () => {
    let testDir: Path
    let testFile: Path

    beforeAll(async () => {
        testDir = new Path(tmpdir()).join(`ejsx-fs-test-${process.pid}`)
        await testDir.makeDir()
        testFile = testDir.join('test.txt')
        await testFile.write('test content')
    })

    afterAll(async () => {
        if (testDir.exists) {
            await testDir.removeAll()
        }
    })

    await describe('Static Properties', async () => {
        it('has separators property', () => {
            expect(FileSystem.separators).toBeDefined()
            expect(typeof FileSystem.separators).toBe('string')
        })

        it('has separator property', () => {
            expect(FileSystem.separator).toBeDefined()
            expect(typeof FileSystem.separator).toBe('string')
            expect(FileSystem.separator.length).toBeGreaterThan(0)
        })

        it('has newline property', () => {
            expect(FileSystem.newline).toBeDefined()
            expect(typeof FileSystem.newline).toBe('string')
        })

        it('separator is platform-appropriate', () => {
            if (process.platform === 'win32') {
                expect(FileSystem.separator).toBe('\\')
                expect(FileSystem.separators).toContain('\\')
                expect(FileSystem.separators).toContain('/')
            } else {
                expect(FileSystem.separator).toBe('/')
                expect(FileSystem.separators).toBe('/')
            }
        })

        it('newline is platform-appropriate', () => {
            if (process.platform === 'win32') {
                expect(FileSystem.newline).toBe('\r\n')
            } else {
                expect(FileSystem.newline).toBe('\n')
            }
        })
    })

    await describe('Constructor', async () => {
        it('constructs with Path object', () => {
            const fs = new FileSystem(testDir)
            expect(fs).toBeInstanceOf(FileSystem)
        })

        it('constructs with string path', () => {
            const fs = new FileSystem('/tmp')
            expect(fs).toBeInstanceOf(FileSystem)
        })

        it('accepts absolute paths', () => {
            const fs = new FileSystem('/usr/local')
            expect(fs).toBeInstanceOf(FileSystem)
        })

        it('accepts relative paths', () => {
            const fs = new FileSystem('.')
            expect(fs).toBeInstanceOf(FileSystem)
        })
    })

    await describe('Instance Properties', async () => {
        it('has freeSpace property', () => {
            const fs = new FileSystem(testDir)
            expect(typeof fs.freeSpace).toBe('number')
            // Returns -1 (not implemented)
            expect(fs.freeSpace).toBe(-1)
        })

        it('has totalSpace property', () => {
            const fs = new FileSystem(testDir)
            expect(typeof fs.totalSpace).toBe('number')
            // Returns -1 (not implemented)
            expect(fs.totalSpace).toBe(-1)
        })

        it('has type property', () => {
            const fs = new FileSystem(testDir)
            expect(fs.type).toBe('native')
        })

        it('has root property', () => {
            const fs = new FileSystem(testDir)
            const root = fs.root
            expect(root).toBeInstanceOf(Path)
            expect(root.isAbsolute).toBe(true)
        })
    })

    await describe('writable', async () => {
        it('returns true for writable directory', () => {
            const fs = new FileSystem(testDir)
            expect(fs.writable).toBe(true)
        })

        it('returns true for writable file', () => {
            const fs = new FileSystem(testFile)
            expect(fs.writable).toBe(true)
        })

        it('returns false for non-existent path', () => {
            const nonExistent = testDir.join('does-not-exist')
            const fs = new FileSystem(nonExistent)
            expect(fs.writable).toBe(false)
        })
    })

    await describe('readable', async () => {
        it('returns true for readable directory', () => {
            const fs = new FileSystem(testDir)
            expect(fs.readable).toBe(true)
        })

        it('returns true for readable file', () => {
            const fs = new FileSystem(testFile)
            expect(fs.readable).toBe(true)
        })

        it('returns false for non-existent path', () => {
            const nonExistent = testDir.join('does-not-exist')
            const fs = new FileSystem(nonExistent)
            expect(fs.readable).toBe(false)
        })
    })

    await describe('exists()', async () => {
        it('returns true for existing directory', () => {
            const fs = new FileSystem(testDir)
            expect(fs.exists()).toBe(true)
        })

        it('returns true for existing file', () => {
            const fs = new FileSystem(testFile)
            expect(fs.exists()).toBe(true)
        })

        it('returns false for non-existent path', () => {
            const nonExistent = testDir.join('does-not-exist')
            const fs = new FileSystem(nonExistent)
            expect(fs.exists()).toBe(false)
        })
    })

    await describe('getMetadata()', async () => {
        it('returns metadata object', () => {
            const fs = new FileSystem(testFile)
            const metadata = fs.getMetadata()

            expect(metadata).toBeDefined()
            expect(typeof metadata).toBe('object')
        })

        it('metadata has required properties', () => {
            const fs = new FileSystem(testFile)
            const metadata = fs.getMetadata()

            expect(metadata).toHaveProperty('size')
            expect(metadata).toHaveProperty('modified')
            expect(metadata).toHaveProperty('accessed')
            expect(metadata).toHaveProperty('created')
            expect(metadata).toHaveProperty('isDir')
            expect(metadata).toHaveProperty('isFile')
            expect(metadata).toHaveProperty('isLink')
            expect(metadata).toHaveProperty('permissions')
        })

        it('metadata for file is correct', () => {
            const fs = new FileSystem(testFile)
            const metadata = fs.getMetadata()

            expect(metadata.size).toBeGreaterThan(0)
            expect(metadata.isFile).toBe(true)
            expect(metadata.isDir).toBe(false)
            expect(metadata.modified).toBeInstanceOf(Date)
        })

        it('metadata for directory is correct', () => {
            const fs = new FileSystem(testDir)
            const metadata = fs.getMetadata()

            expect(metadata.isDir).toBe(true)
            expect(metadata.isFile).toBe(false)
        })

        it('metadata dates are valid', () => {
            const fs = new FileSystem(testFile)
            const metadata = fs.getMetadata()

            if (metadata.modified) {
                expect(metadata.modified.getTime()).toBeGreaterThan(0)
                expect(metadata.modified.getTime()).toBeLessThanOrEqual(Date.now())
            }

            if (metadata.accessed) {
                expect(metadata.accessed.getTime()).toBeGreaterThan(0)
            }
        })
    })

    await describe('createDirectory()', async () => {
        it('creates a new directory', async () => {
            const newDir = testDir.join(`newdir-${Date.now()}`)
            const fs = new FileSystem(newDir)

            expect(fs.exists()).toBe(false)
            const result = await fs.createDirectory()
            expect(result).toBe(true)
            expect(fs.exists()).toBe(true)
            expect(newDir.isDir).toBe(true)

            await newDir.remove()
        })

        it('creates directory with default permissions', async () => {
            const newDir = testDir.join(`permdir-${Date.now()}`)
            const fs = new FileSystem(newDir)

            await fs.createDirectory()
            expect(fs.exists()).toBe(true)

            await newDir.remove()
        })

        it('succeeds even if directory already exists', async () => {
            const fs = new FileSystem(testDir)
            expect(fs.exists()).toBe(true)
            const result = await fs.createDirectory()
            // makeDir returns true even if dir exists (idempotent)
            expect(result).toBe(true)
        })
    })

    await describe('remove()', async () => {
        it('removes a file', async () => {
            const tempFile = testDir.join(`removeme-${Date.now()}.txt`)
            await tempFile.write('delete me')
            expect(tempFile.exists).toBe(true)

            const fs = new FileSystem(tempFile)
            const result = await fs.remove()
            expect(result).toBe(true)
            expect(tempFile.exists).toBe(false)
        })

        it('removes an empty directory', async () => {
            const tempDir = testDir.join(`removedir-${Date.now()}`)
            await tempDir.makeDir()
            expect(tempDir.exists).toBe(true)

            const fs = new FileSystem(tempDir)
            const result = await fs.remove()
            expect(result).toBe(true)
            expect(tempDir.exists).toBe(false)
        })

        it('succeeds even for non-existent path', async () => {
            const nonExistent = testDir.join('does-not-exist')
            const fs = new FileSystem(nonExistent)
            const result = await fs.remove()
            // Path.remove() returns true even if path doesn't exist (idempotent)
            expect(result).toBe(true)
        })
    })

    await describe('removeAll()', async () => {
        it('removes directory with contents', async () => {
            const tempDir = testDir.join(`removeall-${Date.now()}`)
            await tempDir.makeDir()
            await tempDir.join('file1.txt').write('content')
            await tempDir.join('file2.txt').write('content')
            await tempDir.join('subdir').makeDir()
            await tempDir.join('subdir/file3.txt').write('content')

            expect(tempDir.exists).toBe(true)

            const fs = new FileSystem(tempDir)
            const result = await fs.removeAll()
            expect(result).toBe(true)
            expect(tempDir.exists).toBe(false)
        })

        it('removes single file', async () => {
            const tempFile = testDir.join(`removeallfile-${Date.now()}.txt`)
            await tempFile.write('delete me')
            expect(tempFile.exists).toBe(true)

            const fs = new FileSystem(tempFile)
            const result = await fs.removeAll()
            expect(result).toBe(true)
            expect(tempFile.exists).toBe(false)
        })
    })

    await describe('Static Methods', async () => {
        await describe('getFileSystems()', async () => {
            it('returns array of FileSystem objects', () => {
                const fileSystems = FileSystem.getFileSystems()
                expect(Array.isArray(fileSystems)).toBe(true)
                expect(fileSystems.length).toBeGreaterThan(0)
            })

            it('returns FileSystem instances', () => {
                const fileSystems = FileSystem.getFileSystems()
                fileSystems.forEach(fs => {
                    expect(fs).toBeInstanceOf(FileSystem)
                })
            })

            it('returns root filesystem', () => {
                const fileSystems = FileSystem.getFileSystems()
                expect(fileSystems.length).toBeGreaterThanOrEqual(1)

                if (process.platform === 'win32') {
                    // Windows should have C:\ or similar
                    expect(fileSystems[0]).toBeDefined()
                } else {
                    // Unix should have root
                    expect(fileSystems[0]).toBeDefined()
                }
            })
        })

        await describe('getFileSystem()', async () => {
            it('creates FileSystem from Path object', () => {
                const fs = FileSystem.getFileSystem(testDir)
                expect(fs).toBeInstanceOf(FileSystem)
            })

            it('creates FileSystem from string', () => {
                const fs = FileSystem.getFileSystem('/tmp')
                expect(fs).toBeInstanceOf(FileSystem)
            })

            it('created FileSystem is usable', () => {
                const fs = FileSystem.getFileSystem(testFile)
                expect(fs.exists()).toBe(true)
                expect(fs.readable).toBe(true)
            })
        })
    })

    await describe('Integration', async () => {
        it('can check and modify filesystem', async () => {
            const tempFile = testDir.join(`integration-${Date.now()}.txt`)
            const fs = new FileSystem(tempFile)

            // Initially doesn't exist
            expect(fs.exists()).toBe(false)

            // Create it via Path
            await tempFile.write('test')
            expect(fs.exists()).toBe(true)
            expect(fs.readable).toBe(true)
            expect(fs.writable).toBe(true)

            // Check metadata
            const metadata = fs.getMetadata()
            expect(metadata.isFile).toBe(true)
            expect(metadata.size).toBeGreaterThan(0)

            // Remove it
            await fs.remove()
            expect(fs.exists()).toBe(false)
        })

        it('works with nested directories', async () => {
            const nestedDir = testDir.join('level1/level2/level3')
            const fs = new FileSystem(nestedDir)

            expect(fs.exists()).toBe(false)
            await nestedDir.makeDir({ recursive: true })
            expect(fs.exists()).toBe(true)
            expect(fs.readable).toBe(true)

            await testDir.join('level1').removeAll()
        })
    })

    await describe('Type Safety', async () => {
        it('static properties have correct types', () => {
            expect(typeof FileSystem.separator).toBe('string')
            expect(typeof FileSystem.separators).toBe('string')
            expect(typeof FileSystem.newline).toBe('string')
        })

        it('instance methods return correct types', () => {
            const fs = new FileSystem(testFile)

            expect(typeof fs.exists()).toBe('boolean')
            expect(typeof fs.readable).toBe('boolean')
            expect(typeof fs.writable).toBe('boolean')
            expect(typeof fs.type).toBe('string')
            expect(typeof fs.freeSpace).toBe('number')
            expect(typeof fs.totalSpace).toBe('number')
            expect(fs.root).toBeInstanceOf(Path)
        })

        it('getMetadata returns correct structure', () => {
            const fs = new FileSystem(testFile)
            const metadata = fs.getMetadata()

            expect(typeof metadata.size).toBe('number')
            expect(typeof metadata.isDir).toBe('boolean')
            expect(typeof metadata.isFile).toBe('boolean')
            expect(typeof metadata.isLink).toBe('boolean')

            if (metadata.modified) {
                expect(metadata.modified).toBeInstanceOf(Date)
            }
        })
    })
})
