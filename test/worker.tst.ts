import { describe, it, expect, beforeEach, afterEach } from 'testme'
import { Worker } from '../src/core/async/Worker'
import * as fs from 'fs'
import * as path from 'path'

await describe('Worker', async () => {
    let worker: Worker | null = null
    const workerDir = path.join(__dirname, '.test/workers')

    beforeEach(() => {
        // Create test directory for worker scripts
        if (!fs.existsSync(workerDir)) {
            fs.mkdirSync(workerDir, { recursive: true })
        }
    })

    afterEach(async () => {
        if (worker) {
            try {
                worker.terminate()
                // Wait for worker to fully terminate
                await new Promise(resolve => setTimeout(resolve, 100))
            } catch {}
            worker = null
        }
    })

    await describe('Construction', async () => {
        it('creates a worker with script path', () => {
            const scriptPath = path.join(workerDir, 'simple.js')
            fs.writeFileSync(scriptPath, `
                const { parentPort } = require('worker_threads');
                if (parentPort) {
                    parentPort.postMessage('ready');
                }
            `)

            worker = new Worker(scriptPath)
            expect(worker).not.toBeNull()
            expect(worker.threadId).toBeGreaterThan(0)
        })

        it('creates a worker with options', () => {
            const scriptPath = path.join(workerDir, 'options.js')
            fs.writeFileSync(scriptPath, `
                const { parentPort, workerData } = require('worker_threads');
                if (parentPort && workerData) {
                    parentPort.postMessage({ received: workerData });
                }
            `)

            worker = new Worker(scriptPath, { workerData: { test: 'value' } })
            expect(worker).not.toBeNull()
        })

        it('has valid threadId', () => {
            const scriptPath = path.join(workerDir, 'threadid.js')
            fs.writeFileSync(scriptPath, `
                const { parentPort } = require('worker_threads');
                if (parentPort) {
                    parentPort.postMessage('ok');
                }
            `)

            worker = new Worker(scriptPath)
            expect(worker.threadId).toBeGreaterThan(0)
        })
    })

    await describe('Message Passing', async () => {
        it('posts and receives messages', async () => {
            await new Promise<void>((resolve) => {
                const scriptPath = path.join(workerDir, 'echo.js')
                fs.writeFileSync(scriptPath, `
                    const { parentPort } = require('worker_threads');
                    if (parentPort) {
                        parentPort.on('message', (msg) => {
                            parentPort.postMessage({ echo: msg });
                        });
                    }
                `)
    
                worker = new Worker(scriptPath)
                worker.onmessage = (data) => {
                    expect(data.echo).toBe('test message')
                    resolve()
                }
    
                setTimeout(() => {
                    worker!.postMessage('test message')
                }, 50)
            })
        })

        it('handles multiple messages', async () => {
            await new Promise<void>((resolve) => {
                const scriptPath = path.join(workerDir, 'multi.js')
                fs.writeFileSync(scriptPath, `
                    const { parentPort } = require('worker_threads');
                    let count = 0;
                    if (parentPort) {
                        parentPort.on('message', (msg) => {
                            count++;
                            parentPort.postMessage(count);
                        });
                    }
                `)
    
                worker = new Worker(scriptPath)
                let received = 0
    
                worker.onmessage = (count) => {
                    received++
                    if (received === 3) {
                        expect(count).toBe(3)
                        resolve()
                    }
                }
    
                setTimeout(() => {
                    worker!.postMessage('msg1')
                    worker!.postMessage('msg2')
                    worker!.postMessage('msg3')
                }, 50)
            })
        })

        it('handles complex data types', async () => {
            await new Promise<void>((resolve) => {
                const scriptPath = path.join(workerDir, 'complex.js')
                fs.writeFileSync(scriptPath, `
                    const { parentPort } = require('worker_threads');
                    if (parentPort) {
                        parentPort.on('message', (msg) => {
                            parentPort.postMessage({
                                type: typeof msg,
                                isObject: typeof msg === 'object',
                                data: msg
                            });
                        });
                    }
                `)
    
                worker = new Worker(scriptPath)
                const testData = {
                    number: 42,
                    string: 'test',
                    array: [1, 2, 3],
                    nested: { a: 1, b: 2 }
                }
    
                worker.onmessage = (result) => {
                    expect(result.type).toBe('object')
                    expect(result.isObject).toBe(true)
                    expect(result.data.number).toBe(42)
                    expect(result.data.string).toBe('test')
                    expect(result.data.array).toEqual([1, 2, 3])
                    expect(result.data.nested.a).toBe(1)
                    resolve()
                }
    
                setTimeout(() => {
                    worker!.postMessage(testData)
                }, 50)
            })
        })
    })

    await describe('Event Emitters', async () => {
        it('emits message events', async () => {
            await new Promise<void>((resolve) => {
                const scriptPath = path.join(workerDir, 'event.js')
                fs.writeFileSync(scriptPath, `
                    const { parentPort } = require('worker_threads');
                    if (parentPort) {
                        parentPort.postMessage('event test');
                    }
                `)
    
                worker = new Worker(scriptPath)
                worker.on('message', (data) => {
                    expect(data).toBe('event test')
                    resolve()
                })
            })
        })

        it('emits error events', async () => {
            await new Promise<void>((resolve) => {
                const scriptPath = path.join(workerDir, 'error.js')
                fs.writeFileSync(scriptPath, `
                    const { parentPort } = require('worker_threads');
                    throw new Error('Worker error');
                `)
    
                worker = new Worker(scriptPath)
                worker.onerror = (error) => {
                    expect(error).toBeDefined()
                    expect(error.message).toContain('Worker error')
                    resolve()
                }
            })
        })

        it('emits exit events', async () => {
            await new Promise<void>((resolve) => {
                const scriptPath = path.join(workerDir, 'exit.js')
                fs.writeFileSync(scriptPath, `
                    const { parentPort } = require('worker_threads');
                    if (parentPort) {
                        setTimeout(() => process.exit(0), 100);
                    }
                `)
    
                worker = new Worker(scriptPath)
                worker.on('exit', (code) => {
                    expect(code).toBe(0)
                    resolve()
                })
            })
        })
    })

    await describe('Callback Handlers', async () => {
        it('supports onmessage callback', async () => {
            await new Promise<void>((resolve) => {
                const scriptPath = path.join(workerDir, 'callback.js')
                fs.writeFileSync(scriptPath, `
                    const { parentPort } = require('worker_threads');
                    if (parentPort) {
                        parentPort.postMessage('callback test');
                    }
                `)
    
                worker = new Worker(scriptPath)
                worker.onmessage = (data) => {
                    expect(data).toBe('callback test')
                    resolve()
                }
            })
        })

        it('supports onerror callback', async () => {
            await new Promise<void>((resolve) => {
                const scriptPath = path.join(workerDir, 'error-callback.js')
                fs.writeFileSync(scriptPath, `
                    throw new Error('Error callback test');
                `)
    
                worker = new Worker(scriptPath)
                worker.onerror = (error) => {
                    expect(error.message).toContain('Error callback test')
                    resolve()
                }
            })
        })

        it('can update callbacks dynamically', async () => {
            await new Promise<void>((resolve) => {
                const scriptPath = path.join(workerDir, 'dynamic.js')
                fs.writeFileSync(scriptPath, `
                    const { parentPort } = require('worker_threads');
                    if (parentPort) {
                        parentPort.on('message', (msg) => {
                            if (msg === 'send') {
                                parentPort.postMessage('response');
                            }
                        });
                    }
                `)
    
                worker = new Worker(scriptPath)
    
                let firstCalled = false
                worker.onmessage = (data) => {
                    firstCalled = true
                }
    
                setTimeout(() => {
                    worker!.onmessage = (data) => {
                        expect(data).toBe('response')
                        expect(firstCalled).toBe(false)
                        resolve()
                    }
                    worker!.postMessage('send')
                }, 50)
            })
        })
    })

    await describe('Termination', async () => {
        it('terminates worker', async () => {
            await new Promise<void>((resolve) => {
                const scriptPath = path.join(workerDir, 'terminate.js')
                fs.writeFileSync(scriptPath, `
                    const { parentPort } = require('worker_threads');
                    setInterval(() => {
                        if (parentPort) {
                            parentPort.postMessage('still running');
                        }
                    }, 100);
                `)
    
                worker = new Worker(scriptPath)
                worker.on('exit', (code) => {
                    expect(code).toBeDefined()
                    resolve()
                })
    
                setTimeout(() => {
                    worker!.terminate()
                }, 200)
            })
        })

        it('terminates immediately', () => {
            const scriptPath = path.join(workerDir, 'terminate-immediate.js')
            fs.writeFileSync(scriptPath, `
                const { parentPort } = require('worker_threads');
                if (parentPort) {
                    parentPort.postMessage('started');
                }
            `)

            worker = new Worker(scriptPath)
            worker.terminate()
            // Should not throw
            expect(worker).not.toBeNull()
        })
    })

    await describe('Eval and Load', async () => {
        it('evaluates code via eval', async () => {
            await new Promise<void>((resolve) => {
                const scriptPath = path.join(workerDir, 'eval.js')
                fs.writeFileSync(scriptPath, `
                    const { parentPort } = require('worker_threads');
                    if (parentPort) {
                        parentPort.on('message', (msg) => {
                            if (msg.type === 'eval') {
                                try {
                                    const result = eval(msg.code);
                                    parentPort.postMessage({ result });
                                } catch (error) {
                                    parentPort.postMessage({ error: error.message });
                                }
                            }
                        });
                    }
                `)
    
                worker = new Worker(scriptPath)
                worker.onmessage = (data) => {
                    if (data.result !== undefined) {
                        expect(data.result).toBe(42)
                        resolve()
                    }
                }
    
                setTimeout(() => {
                    worker!.eval('21 * 2')
                }, 50)
            })
        })

        it('loads script via load', async () => {
            await new Promise<void>((resolve) => {
                const loadScriptPath = path.join(workerDir, 'to-load.js')
                fs.writeFileSync(loadScriptPath, `
                    const { parentPort } = require('worker_threads');
                    if (parentPort) {
                        parentPort.postMessage('loaded');
                    }
                `)
    
                const scriptPath = path.join(workerDir, 'loader.js')
                fs.writeFileSync(scriptPath, `
                    const { parentPort } = require('worker_threads');
                    if (parentPort) {
                        parentPort.on('message', (msg) => {
                            if (msg.type === 'load') {
                                require(msg.scriptPath);
                            }
                        });
                    }
                `)
    
                worker = new Worker(scriptPath)
                worker.onmessage = (data) => {
                    if (data === 'loaded') {
                        resolve()
                    }
                }
    
                setTimeout(() => {
                    worker!.load(loadScriptPath)
                }, 50)
            })
        })
    })

    await describe('Wait for Message', async () => {
        it('waits for message with timeout', async () => {
            const scriptPath = path.join(workerDir, 'wait.js')
            fs.writeFileSync(scriptPath, `
                const { parentPort } = require('worker_threads');
                if (parentPort) {
                    setTimeout(() => {
                        parentPort.postMessage('delayed message');
                    }, 100);
                }
            `)

            worker = new Worker(scriptPath)
            const message = await worker.waitForMessage(1000)
            expect(message).toBe('delayed message')
        })

        it('times out when no message received', async () => {
            const scriptPath = path.join(workerDir, 'no-message.js')
            fs.writeFileSync(scriptPath, `
                const { parentPort } = require('worker_threads');
                // Never sends a message
            `)

            worker = new Worker(scriptPath)
            await expect(worker.waitForMessage(100)).rejects.toThrow('Timeout')
        })

        it('receives first message when multiple sent', async () => {
            const scriptPath = path.join(workerDir, 'wait-multi.js')
            fs.writeFileSync(scriptPath, `
                const { parentPort } = require('worker_threads');
                if (parentPort) {
                    parentPort.postMessage('first');
                    setTimeout(() => {
                        parentPort.postMessage('second');
                    }, 50);
                }
            `)

            worker = new Worker(scriptPath)
            const message = await worker.waitForMessage(1000)
            expect(message).toBe('first')
        })
    })

    await describe('Exit', async () => {
        it('exits with code', async () => {
            await new Promise<void>((resolve) => {
                const scriptPath = path.join(workerDir, 'exit-code.js')
                fs.writeFileSync(scriptPath, `
                    const { parentPort } = require('worker_threads');
                    if (parentPort) {
                        parentPort.on('message', (msg) => {
                            if (msg.type === 'exit') {
                                process.exit(msg.code);
                            }
                        });
                    }
                `)
    
                worker = new Worker(scriptPath)
                worker.on('exit', (code) => {
                    expect(code).toBe(5)
                    resolve()
                })
    
                setTimeout(() => {
                    worker!.exit(5)
                }, 50)
            })
        })
    })

    await describe('Error Handling', async () => {
        it('handles syntax errors in worker', async () => {
            await new Promise<void>((resolve) => {
                const scriptPath = path.join(workerDir, 'syntax-error.js')
                fs.writeFileSync(scriptPath, `
                    this is invalid javascript syntax !!!
                `)
    
                worker = new Worker(scriptPath)
                worker.onerror = (error) => {
                    expect(error).toBeDefined()
                    resolve()
                }
            })
        })

        it('handles runtime errors in worker', async () => {
            await new Promise<void>((resolve) => {
                const scriptPath = path.join(workerDir, 'runtime-error.js')
                fs.writeFileSync(scriptPath, `
                    const { parentPort } = require('worker_threads');
                    if (parentPort) {
                        parentPort.on('message', () => {
                            throw new Error('Runtime error');
                        });
                    }
                `)
    
                worker = new Worker(scriptPath)
                worker.onerror = (error) => {
                    expect(error.message).toContain('Runtime error')
                    resolve()
                }
    
                setTimeout(() => {
                    worker!.postMessage('trigger')
                }, 50)
            })
        })
    })

    await describe('Clone', async () => {
        it('throws error on clone (not supported)', () => {
            const scriptPath = path.join(workerDir, 'clone.js')
            fs.writeFileSync(scriptPath, `
                const { parentPort } = require('worker_threads');
                if (parentPort) {
                    parentPort.postMessage('ready');
                }
            `)

            worker = new Worker(scriptPath)
            expect(() => {
                worker.clone()
            }).toThrow('not supported')
        })
    })
})
