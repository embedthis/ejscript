import { describe, it, expect, beforeEach, afterEach } from 'testme'
import { Timer } from '../src/core/utilities/Timer'

await describe('Timer', async () => {
    let timer: Timer | null = null

    afterEach(async () => {
        // Wait a bit for any pending callbacks to complete
        await new Promise(resolve => setTimeout(resolve, 50))
        if (timer) {
            try {
                timer.stop()
            } catch (e) {
                // Timer may already be stopped
            }
            timer = null
        }
    })

    await describe('Construction', async () => {
        it('creates timer with period only', () => {
            timer = new Timer(100)
            expect(timer.period).toBe(100)
        })

        it('creates timer with period and callback', () => {
            let called = false
            timer = new Timer(100, () => { called = true })
            expect(timer.period).toBe(100)
        })

        it('creates timer with period, callback, and arguments', async () => {
            await new Promise<void>((resolve) => {
                let args: any[] = []
                timer = new Timer(100, (...received: any[]) => { args = received }, 'a', 42, true)
    
                timer.start()
                setTimeout(() => {
                    expect(args).toEqual(['a', 42, true])
                    resolve()
                }, 150)
            })
        })
    })

    await describe('Properties', async () => {
        it('has period property', () => {
            timer = new Timer(200)
            expect(timer.period).toBe(200)
        })

        it('allows setting period', () => {
            timer = new Timer(100)
            timer.period = 300
            expect(timer.period).toBe(300)
        })

        it('has drift property (default false)', () => {
            timer = new Timer(100)
            expect(timer.drift).toBe(false)
        })

        it('allows setting drift', () => {
            timer = new Timer(100)
            timer.drift = true
            expect(timer.drift).toBe(true)
        })

        it('has repeat property (default false)', () => {
            timer = new Timer(100)
            expect(timer.repeat).toBe(false)
        })

        it('allows setting repeat', () => {
            timer = new Timer(100)
            timer.repeat = true
            expect(timer.repeat).toBe(true)
        })

        it('has onerror property', () => {
            timer = new Timer(100)
            expect(timer.onerror).toBeUndefined()
        })

        it('allows setting onerror callback', () => {
            const handler = (err: Error) => console.error(err)
            timer = new Timer(100)
            timer.onerror = handler
            expect(timer.onerror).toBe(handler)
        })
    })

    await describe('One-shot Timer', async () => {
        it('fires once after period', async () => {
            await new Promise<void>((resolve) => {
                let count = 0
                timer = new Timer(50, () => { count++ })
    
                timer.start()
    
                setTimeout(() => {
                    expect(count).toBe(1)
                    resolve()
                }, 100)
            })
        })

        it('invokes callback', async () => {
            await new Promise<void>((resolve) => {
                let called = false
                timer = new Timer(50, () => { called = true })
    
                timer.start()
    
                setTimeout(() => {
                    expect(called).toBe(true)
                    resolve()
                }, 100)
            })
        })

        it('passes arguments to callback', async () => {
            await new Promise<void>((resolve) => {
                let receivedArgs: any[] = []
                timer = new Timer(50, (...args: any[]) => { receivedArgs = args }, 'test', 42)
    
                timer.start()
    
                setTimeout(() => {
                    expect(receivedArgs).toEqual(['test', 42])
                    resolve()
                }, 100)
            })
        })

        it('emits timer event', async () => {
            await new Promise<void>((resolve) => {
                timer = new Timer(50, () => {})
    
                let eventTimer: Timer | null = null
                timer.on('timer', (t: Timer) => {
                    eventTimer = t
                })
    
                timer.start()
    
                setTimeout(() => {
                    expect(eventTimer).toBe(timer)
                    resolve()
                }, 100)
            })
        })

        it('does not fire if stopped', async () => {
            await new Promise<void>((resolve) => {
                let count = 0
                timer = new Timer(50, () => { count++ })
    
                timer.start()
                timer.stop()
    
                setTimeout(() => {
                    expect(count).toBe(0)
                    resolve()
                }, 100)
            })
        })
    })

    await describe('Repeating Timer', async () => {
        it('fires multiple times', async () => {
            await new Promise<void>((resolve) => {
                let count = 0
                timer = new Timer(30, () => { count++ })
                timer.repeat = true
    
                timer.start()
    
                setTimeout(() => {
                    timer!.stop()
                    expect(count).toBeGreaterThanOrEqual(3)
                    resolve()
                }, 110)
            })
        })

        it('continues firing until stopped', async () => {
            await new Promise<void>((resolve) => {
                let count = 0
                timer = new Timer(25, () => { count++ })
                timer.repeat = true
    
                timer.start()
    
                setTimeout(() => {
                    const countAt100 = count
                    setTimeout(() => {
                        timer!.stop()
                        expect(count).toBeGreaterThan(countAt100)
                        resolve()
                    }, 60)
                }, 100)
            })
        })

        it('emits timer event on each invocation', async () => {
            await new Promise<void>((resolve) => {
                let eventCount = 0
                timer = new Timer(30, () => {})
                timer.repeat = true
    
                timer.on('timer', () => {
                    eventCount++
                })
    
                timer.start()
    
                setTimeout(() => {
                    timer!.stop()
                    expect(eventCount).toBeGreaterThanOrEqual(3)
                    resolve()
                }, 110)
            })
        })
    })

    await describe('Drift Control', async () => {
        it('uses drift mode when enabled', async () => {
            await new Promise<void>((resolve) => {
                let count = 0
                timer = new Timer(30, () => { count++ })
                timer.repeat = true
                timer.drift = true
    
                timer.start()
    
                setTimeout(() => {
                    timer!.stop()
                    expect(count).toBeGreaterThanOrEqual(3)
                    resolve()
                }, 110)
            })
        })

        it('uses non-drift mode by default', async () => {
            await new Promise<void>((resolve) => {
                let count = 0
                timer = new Timer(30, () => { count++ })
                timer.repeat = true
                // drift = false by default
    
                timer.start()
    
                setTimeout(() => {
                    timer!.stop()
                    expect(count).toBeGreaterThanOrEqual(3)
                    resolve()
                }, 110)
            })
        })
    })

    await describe('Start/Stop', async () => {
        it('starts timer', async () => {
            await new Promise<void>((resolve) => {
                let called = false
                timer = new Timer(50, () => { called = true })
    
                const result = timer.start()
                expect(result).toBe(timer) // Returns timer for chaining
    
                setTimeout(() => {
                    expect(called).toBe(true)
                    resolve()
                }, 100)
            })
        })

        it('does not start if already running', async () => {
            await new Promise<void>((resolve) => {
                let count = 0
                timer = new Timer(50, () => { count++ })
    
                timer.start()
                timer.start() // Second start should be ignored
    
                setTimeout(() => {
                    expect(count).toBe(1)
                    resolve()
                }, 100)
            })
        })

        it('stops timer', async () => {
            await new Promise<void>((resolve) => {
                let count = 0
                timer = new Timer(30, () => { count++ })
                timer.repeat = true
    
                timer.start()
    
                setTimeout(() => {
                    timer!.stop()
                    const countAtStop = count
    
                    setTimeout(() => {
                        expect(count).toBe(countAtStop) // No more increments
                        resolve()
                    }, 100)
                }, 70)
            })
        })

        it('can restart after stop', async () => {
            await new Promise<void>((resolve) => {
                let count = 0
                timer = new Timer(50, () => { count++ })
    
                timer.start()
                timer.stop()
                timer.start()
    
                setTimeout(() => {
                    expect(count).toBe(1)
                    resolve()
                }, 100)
            })
        })
    })

    await describe('Restart', async () => {
        it('restarts with same period', async () => {
            await new Promise<void>((resolve) => {
                let count = 0
                timer = new Timer(50, () => { count++ })
    
                timer.start()
                setTimeout(() => {
                    timer!.restart()
                }, 30)
    
                setTimeout(() => {
                    expect(count).toBe(1)
                    resolve()
                }, 120)
            })
        })

        it('restarts with new period', async () => {
            await new Promise<void>((resolve) => {
                let count = 0
                timer = new Timer(100, () => { count++ })
    
                timer.start()
                timer.restart(50) // Change to 50ms
    
                setTimeout(() => {
                    expect(count).toBe(1)
                    expect(timer!.period).toBe(50)
                    resolve()
                }, 80)
            })
        })

        it('stops current timer before restarting', async () => {
            await new Promise<void>((resolve) => {
                let timestamps: number[] = []
                timer = new Timer(100, () => { timestamps.push(Date.now()) })
    
                timer.start()
                setTimeout(() => {
                    timer!.restart(50)
                }, 40)
    
                setTimeout(() => {
                    // Should have fired once at ~90ms (50ms after restart)
                    expect(timestamps.length).toBe(1)
                    resolve()
                }, 150)
            })
        })
    })

    await describe('Error Handling', async () => {
        it('catches errors in callback', async () => {
            await new Promise<void>((resolve) => {
                timer = new Timer(50, () => {
                    throw new Error('Test error')
                })
    
                // Should not crash
                timer.start()
    
                setTimeout(() => {
                    // Timer should have fired and handled error
                    resolve()
                }, 100)
            })
        })

        it('calls onerror handler on callback error', async () => {
            await new Promise<void>((resolve) => {
                let errorCaught: Error | null = null
                timer = new Timer(50, () => {
                    throw new Error('Test error')
                })
    
                timer.onerror = (err: Error) => {
                    errorCaught = err
                }
    
                timer.start()
    
                setTimeout(() => {
                    expect(errorCaught).not.toBeNull()
                    expect(errorCaught?.message).toBe('Test error')
                    resolve()
                }, 100)
            })
        })

        it('handles error in onerror handler', async () => {
            await new Promise<void>((resolve) => {
                timer = new Timer(50, () => {
                    throw new Error('Callback error')
                })
    
                timer.onerror = (_err: Error) => {
                    throw new Error('Error handler error')
                }
    
                // Should not crash
                timer.start()
    
                setTimeout(() => {
                    resolve()
                }, 100)
            })
        })
    })

    await describe('Callback Context', async () => {
        it('calls callback with timer as this', async () => {
            await new Promise<void>((resolve) => {
                let callbackThis: any = null
                timer = new Timer(50, function(this: any) {
                    callbackThis = this
                })
    
                timer.start()
    
                setTimeout(() => {
                    expect(callbackThis).toBe(timer)
                    resolve()
                }, 100)
            })
        })

        it('preserves bound this context', async () => {
            await new Promise<void>((resolve) => {
                const obj = { value: 42 }
                let callbackThis: any = null
    
                const boundCallback = function(this: any) {
                    callbackThis = this
                }.bind(obj)
    
                timer = new Timer(50, boundCallback)
                timer.start()
    
                setTimeout(() => {
                    expect(callbackThis).toBe(obj)
                    resolve()
                }, 100)
            })
        })
    })

    await describe('Event Emitter Integration', async () => {
        it('inherits from Emitter', () => {
            timer = new Timer(100)
            expect(typeof timer.on).toBe('function')
            expect(typeof timer.off).toBe('function')
            expect(typeof timer.emit).toBe('function')
        })

        it('can listen for timer events', async () => {
            await new Promise<void>((resolve) => {
                let eventFired = false
                timer = new Timer(50, () => {})
    
                timer.on('timer', () => {
                    eventFired = true
                })
    
                timer.start()
    
                setTimeout(() => {
                    expect(eventFired).toBe(true)
                    resolve()
                }, 100)
            })
        })

        it('can remove event listeners', async () => {
            await new Promise<void>((resolve) => {
                let count = 0
                const listener = () => { count++ }
    
                timer = new Timer(30, () => {})
                timer.repeat = true
                const localTimer = timer // Store local reference
    
                localTimer.on('timer', listener)
                localTimer.start()
    
                setTimeout(() => {
                    localTimer.off('timer', listener)
                    const countAtRemoval = count
    
                    setTimeout(() => {
                        localTimer.stop()
                        expect(count).toBe(countAtRemoval) // No more events
                        resolve()
                    }, 80)
                }, 70)
            })
        })
    })
})
