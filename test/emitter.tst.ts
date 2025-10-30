import { describe, it, expect, beforeEach } from 'testme'
import { Emitter } from '../src/core/async/Emitter'

await describe('Emitter', async () => {
    let emitter: Emitter

    beforeEach(() => {
        emitter = new Emitter()
    })

    await describe('Construction', async () => {
        it('should create emitter instance', () => {
            expect(emitter).toBeInstanceOf(Emitter)
        })

        it('should start with no listeners', () => {
            expect(emitter.listenerCount('test')).toBe(0)
            expect(emitter.eventNames()).toEqual([])
        })
    })

    await describe('on()', async () => {
        await describe('single event', async () => {
            it('should register event listener', () => {
                const handler = () => {}
                emitter.on('test', handler)
                expect(emitter.listenerCount('test')).toBe(1)
            })

            it('should call listener when event is emitted', () => {
                let called = false
                emitter.on('test', () => { called = true })
                emitter.emit('test')
                expect(called).toBe(true)
            })

            it('should pass arguments to listener', () => {
                let receivedArgs: any[] = []
                emitter.on('test', (...args: any[]) => { receivedArgs = args })
                emitter.emit('test', 'arg1', 'arg2', 42)
                expect(receivedArgs).toEqual(['arg1', 'arg2', 42])
            })

            it('should allow multiple listeners for same event', () => {
                let count = 0
                emitter.on('test', () => { count++ })
                emitter.on('test', () => { count++ })
                emitter.emit('test')
                expect(count).toBe(2)
            })

            it('should call listeners in registration order', () => {
                const order: number[] = []
                emitter.on('test', () => { order.push(1) })
                emitter.on('test', () => { order.push(2) })
                emitter.on('test', () => { order.push(3) })
                emitter.emit('test')
                expect(order).toEqual([1, 2, 3])
            })

            it('should return this for chaining', () => {
                const result = emitter.on('test', () => {})
                expect(result).toBe(emitter)
            })
        })

        await describe('multiple events (array)', async () => {
            it('should register listener for multiple events', () => {
                const handler = () => {}
                emitter.on(['event1', 'event2', 'event3'], handler)
                expect(emitter.listenerCount('event1')).toBe(1)
                expect(emitter.listenerCount('event2')).toBe(1)
                expect(emitter.listenerCount('event3')).toBe(1)
            })

            it('should call same handler for each event', () => {
                let count = 0
                emitter.on(['event1', 'event2'], () => { count++ })
                emitter.emit('event1')
                emitter.emit('event2')
                expect(count).toBe(2)
            })

            it('should return this for chaining', () => {
                const result = emitter.on(['event1', 'event2'], () => {})
                expect(result).toBe(emitter)
            })
        })

        await describe('object mapping', async () => {
            it('should register multiple events with object', () => {
                let event1Called = false
                let event2Called = false
                emitter.on({
                    event1: () => { event1Called = true },
                    event2: () => { event2Called = true }
                })
                emitter.emit('event1')
                emitter.emit('event2')
                expect(event1Called).toBe(true)
                expect(event2Called).toBe(true)
            })

            it('should work with different handlers per event', () => {
                const values: string[] = []
                emitter.on({
                    event1: () => { values.push('a') },
                    event2: () => { values.push('b') },
                    event3: () => { values.push('c') }
                })
                emitter.emit('event2')
                emitter.emit('event1')
                emitter.emit('event3')
                expect(values).toEqual(['b', 'a', 'c'])
            })

            it('should return this for chaining', () => {
                const result = emitter.on({ test: () => {} })
                expect(result).toBe(emitter)
            })
        })
    })

    await describe('once()', async () => {
        it('should register one-time listener', () => {
            let count = 0
            emitter.once('test', () => { count++ })
            emitter.emit('test')
            emitter.emit('test')
            emitter.emit('test')
            expect(count).toBe(1)
        })

        it('should pass arguments to listener', () => {
            let received: any[] = []
            emitter.once('test', (...args: any[]) => { received = args })
            emitter.emit('test', 1, 2, 3)
            expect(received).toEqual([1, 2, 3])
        })

        it('should remove listener after first call', () => {
            emitter.once('test', () => {})
            emitter.emit('test')
            expect(emitter.listenerCount('test')).toBe(0)
        })

        it('should work with multiple once listeners', () => {
            let count1 = 0
            let count2 = 0
            emitter.once('test', () => { count1++ })
            emitter.once('test', () => { count2++ })
            emitter.emit('test')
            emitter.emit('test')
            expect(count1).toBe(1)
            expect(count2).toBe(1)
        })

        it('should return this for chaining', () => {
            const result = emitter.once('test', () => {})
            expect(result).toBe(emitter)
        })

        it('should not interfere with regular listeners', () => {
            let onceCount = 0
            let onCount = 0
            emitter.once('test', () => { onceCount++ })
            emitter.on('test', () => { onCount++ })
            emitter.emit('test')
            emitter.emit('test')
            expect(onceCount).toBe(1)
            expect(onCount).toBe(2)
        })
    })

    await describe('off()', async () => {
        await describe('single event', async () => {
            it('should remove listener', () => {
                const handler = () => {}
                emitter.on('test', handler)
                emitter.off('test', handler)
                expect(emitter.listenerCount('test')).toBe(0)
            })

            it('should not call removed listener', () => {
                let called = false
                const handler = () => { called = true }
                emitter.on('test', handler)
                emitter.off('test', handler)
                emitter.emit('test')
                expect(called).toBe(false)
            })

            it('should only remove specified listener', () => {
                let count1 = 0
                let count2 = 0
                const handler1 = () => { count1++ }
                const handler2 = () => { count2++ }
                emitter.on('test', handler1)
                emitter.on('test', handler2)
                emitter.off('test', handler1)
                emitter.emit('test')
                expect(count1).toBe(0)
                expect(count2).toBe(1)
            })

            it('should handle removing non-existent listener', () => {
                const handler = () => {}
                expect(() => {
                    emitter.off('test', handler)
                }).not.toThrow()
            })

            it('should handle removing from non-existent event', () => {
                expect(() => {
                    emitter.off('non-existent', () => {})
                }).not.toThrow()
            })

            it('should return this for chaining', () => {
                const handler = () => {}
                const result = emitter.off('test', handler)
                expect(result).toBe(emitter)
            })

            it('should remove event when last listener is removed', () => {
                const handler = () => {}
                emitter.on('test', handler)
                emitter.off('test', handler)
                expect(emitter.eventNames()).not.toContain('test')
            })
        })

        await describe('multiple events (array)', async () => {
            it('should remove listener from multiple events', () => {
                const handler = () => {}
                emitter.on(['event1', 'event2', 'event3'], handler)
                emitter.off(['event1', 'event2', 'event3'], handler)
                expect(emitter.listenerCount('event1')).toBe(0)
                expect(emitter.listenerCount('event2')).toBe(0)
                expect(emitter.listenerCount('event3')).toBe(0)
            })

            it('should return this for chaining', () => {
                const result = emitter.off(['event1', 'event2'], () => {})
                expect(result).toBe(emitter)
            })
        })
    })

    await describe('emit()', async () => {
        it('should return false when no listeners', () => {
            const result = emitter.emit('test')
            expect(result).toBe(false)
        })

        it('should return true when listeners exist', () => {
            emitter.on('test', () => {})
            const result = emitter.emit('test')
            expect(result).toBe(true)
        })

        it('should call all listeners', () => {
            let count = 0
            emitter.on('test', () => { count++ })
            emitter.on('test', () => { count++ })
            emitter.on('test', () => { count++ })
            emitter.emit('test')
            expect(count).toBe(3)
        })

        it('should pass all arguments to listeners', () => {
            let received: any[] = []
            emitter.on('test', (...args: any[]) => { received = args })
            emitter.emit('test', 'a', 42, true, { x: 1 })
            expect(received).toEqual(['a', 42, true, { x: 1 }])
        })

        it('should handle errors in listeners gracefully', () => {
            let called = false
            let errorCaught = false

            // Register error handler to suppress console output
            emitter.on('error', (error: Error) => {
                errorCaught = true
                expect(error.message).toBe('test error')
            })

            emitter.on('test', () => { throw new Error('test error') })
            emitter.on('test', () => { called = true })

            // Should not throw, should continue to next listener
            expect(() => {
                emitter.emit('test')
            }).not.toThrow()

            expect(called).toBe(true)
            expect(errorCaught).toBe(true)
        })

        it('should emit to copy of listeners array', () => {
            // Adding listener during emit should not affect current emit
            emitter.on('test', () => {
                emitter.on('test', () => {
                    throw new Error('Should not be called')
                })
            })

            expect(() => {
                emitter.emit('test')
            }).not.toThrow()
        })
    })

    await describe('removeAllListeners()', async () => {
        it('should remove all listeners for specific event', () => {
            emitter.on('test', () => {})
            emitter.on('test', () => {})
            emitter.on('other', () => {})
            emitter.removeAllListeners('test')
            expect(emitter.listenerCount('test')).toBe(0)
            expect(emitter.listenerCount('other')).toBe(1)
        })

        it('should remove all listeners for all events when no arg', () => {
            emitter.on('event1', () => {})
            emitter.on('event2', () => {})
            emitter.on('event3', () => {})
            emitter.removeAllListeners()
            expect(emitter.eventNames()).toEqual([])
        })

        it('should handle removing from non-existent event', () => {
            expect(() => {
                emitter.removeAllListeners('non-existent')
            }).not.toThrow()
        })

        it('should return this for chaining', () => {
            const result = emitter.removeAllListeners('test')
            expect(result).toBe(emitter)
        })

        it('should prevent removed listeners from being called', () => {
            let called = false
            emitter.on('test', () => { called = true })
            emitter.removeAllListeners('test')
            emitter.emit('test')
            expect(called).toBe(false)
        })
    })

    await describe('listenerCount()', async () => {
        it('should return 0 for event with no listeners', () => {
            expect(emitter.listenerCount('test')).toBe(0)
        })

        it('should return correct count for single listener', () => {
            emitter.on('test', () => {})
            expect(emitter.listenerCount('test')).toBe(1)
        })

        it('should return correct count for multiple listeners', () => {
            emitter.on('test', () => {})
            emitter.on('test', () => {})
            emitter.on('test', () => {})
            expect(emitter.listenerCount('test')).toBe(3)
        })

        it('should update after adding listener', () => {
            expect(emitter.listenerCount('test')).toBe(0)
            emitter.on('test', () => {})
            expect(emitter.listenerCount('test')).toBe(1)
            emitter.on('test', () => {})
            expect(emitter.listenerCount('test')).toBe(2)
        })

        it('should update after removing listener', () => {
            const handler = () => {}
            emitter.on('test', handler)
            emitter.on('test', () => {})
            expect(emitter.listenerCount('test')).toBe(2)
            emitter.off('test', handler)
            expect(emitter.listenerCount('test')).toBe(1)
        })
    })

    await describe('eventNames()', async () => {
        it('should return empty array when no events', () => {
            expect(emitter.eventNames()).toEqual([])
        })

        it('should return array of event names', () => {
            emitter.on('event1', () => {})
            emitter.on('event2', () => {})
            emitter.on('event3', () => {})
            const names = emitter.eventNames()
            expect(names).toContain('event1')
            expect(names).toContain('event2')
            expect(names).toContain('event3')
            expect(names.length).toBe(3)
        })

        it('should not include events with no listeners', () => {
            const handler = () => {}
            emitter.on('test', handler)
            emitter.off('test', handler)
            expect(emitter.eventNames()).not.toContain('test')
        })

        it('should update after adding events', () => {
            expect(emitter.eventNames().length).toBe(0)
            emitter.on('test1', () => {})
            expect(emitter.eventNames().length).toBe(1)
            emitter.on('test2', () => {})
            expect(emitter.eventNames().length).toBe(2)
        })
    })

    await describe('Method Chaining', async () => {
        it('should chain on() calls', () => {
            let count = 0
            emitter
                .on('event1', () => { count++ })
                .on('event2', () => { count++ })
                .on('event3', () => { count++ })

            emitter.emit('event1')
            emitter.emit('event2')
            emitter.emit('event3')
            expect(count).toBe(3)
        })

        it('should chain on() and once()', () => {
            let count = 0
            emitter
                .on('test', () => { count++ })
                .once('test', () => { count++ })

            emitter.emit('test')
            emitter.emit('test')
            expect(count).toBe(3) // on called twice, once called once
        })

        it('should chain on() and off()', () => {
            const handler1 = () => {}
            const handler2 = () => {}
            emitter
                .on('test', handler1)
                .on('test', handler2)
                .off('test', handler1)

            expect(emitter.listenerCount('test')).toBe(1)
        })

        it('should chain all methods except emit', () => {
            let count = 0
            const handler = () => { count++ }

            emitter
                .on('test', handler)
                .once('test', () => { count++ })
                .off('test', handler)
                .removeAllListeners()

            // emit() returns boolean, not this
            const emitted = emitter.emit('test')
            expect(emitted).toBe(false) // no listeners after removeAllListeners
            expect(count).toBe(0) // handlers not called
            expect(emitter.eventNames().length).toBe(0)
        })
    })

    await describe('Ejscript Compatibility Methods', async () => {
        await describe('fire()', async () => {
            it('should work as alias for emit()', () => {
                let called = false
                emitter.on('test', () => { called = true })
                emitter.fire('test')
                expect(called).toBe(true)
            })

            it('should pass arguments to listeners', () => {
                let received: any[] = []
                emitter.on('test', (...args: any[]) => { received = args })
                emitter.fire('test', 'a', 'b', 'c')
                expect(received).toEqual(['a', 'b', 'c'])
            })
        })

        await describe('fireThis()', async () => {
            it('should invoke callbacks with explicit this context', () => {
                const context = { value: 42 }
                let capturedThis: any = null

                emitter.on('test', function(this: any) {
                    capturedThis = this
                })

                emitter.fireThis('test', context)
                expect(capturedThis).toBe(context)
            })

            it('should pass arguments to listeners', () => {
                const context = { value: 42 }
                let received: any[] = []

                emitter.on('test', function(this: any, ...args: any[]) {
                    received = [this.value, ...args]
                })

                emitter.fireThis('test', context, 'a', 'b')
                expect(received).toEqual([42, 'a', 'b'])
            })

            it('should handle multiple listeners with same context', () => {
                const context = { count: 0 }

                emitter.on('test', function(this: any) { this.count++ })
                emitter.on('test', function(this: any) { this.count += 10 })

                emitter.fireThis('test', context)
                expect(context.count).toBe(11)
            })

            it('should not call listeners if event has none', () => {
                const context = { value: 42 }
                expect(() => {
                    emitter.fireThis('nonexistent', context)
                }).not.toThrow()
            })
        })

        await describe('hasObservers()', async () => {
            it('should return false when no observers registered', () => {
                expect(emitter.hasObservers()).toBe(false)
            })

            it('should return true when observers registered', () => {
                emitter.on('test', () => {})
                expect(emitter.hasObservers()).toBe(true)
            })

            it('should return false after all observers removed', () => {
                const handler = () => {}
                emitter.on('test', handler)
                emitter.off('test', handler)
                expect(emitter.hasObservers()).toBe(false)
            })

            it('should return true if any event has observers', () => {
                emitter.on('event1', () => {})
                emitter.on('event2', () => {})
                expect(emitter.hasObservers()).toBe(true)

                const handler1 = emitter.getObservers('event1')[0]
                emitter.off('event1', handler1)
                expect(emitter.hasObservers()).toBe(true) // event2 still has observer
            })
        })

        await describe('getObservers()', async () => {
            it('should return empty array for event with no observers', () => {
                const observers = emitter.getObservers('test')
                expect(observers).toEqual([])
            })

            it('should return array of observers', () => {
                const handler1 = () => {}
                const handler2 = () => {}
                emitter.on('test', handler1)
                emitter.on('test', handler2)

                const observers = emitter.getObservers('test')
                expect(observers.length).toBe(2)
                expect(observers[0]).toBe(handler1)
                expect(observers[1]).toBe(handler2)
            })

            it('should return cloned array', () => {
                emitter.on('test', () => {})
                const observers1 = emitter.getObservers('test')
                const observers2 = emitter.getObservers('test')
                expect(observers1).not.toBe(observers2)
                expect(observers1).toEqual(observers2)
            })

            it('should not affect original when modifying returned array', () => {
                emitter.on('test', () => {})
                const observers = emitter.getObservers('test')
                observers.push(() => {})
                expect(emitter.listenerCount('test')).toBe(1)
            })
        })

        await describe('clearObservers()', async () => {
            it('should clear all observers when called with no args', () => {
                emitter.on('event1', () => {})
                emitter.on('event2', () => {})
                emitter.clearObservers()
                expect(emitter.hasObservers()).toBe(false)
            })

            it('should clear all observers when called with null', () => {
                emitter.on('event1', () => {})
                emitter.on('event2', () => {})
                emitter.clearObservers(null)
                expect(emitter.hasObservers()).toBe(false)
            })

            it('should clear observers for specific event', () => {
                emitter.on('event1', () => {})
                emitter.on('event2', () => {})
                emitter.clearObservers('event1')
                expect(emitter.listenerCount('event1')).toBe(0)
                expect(emitter.listenerCount('event2')).toBe(1)
            })

            it('should clear observers for array of events', () => {
                emitter.on('event1', () => {})
                emitter.on('event2', () => {})
                emitter.on('event3', () => {})
                emitter.clearObservers(['event1', 'event2'])
                expect(emitter.listenerCount('event1')).toBe(0)
                expect(emitter.listenerCount('event2')).toBe(0)
                expect(emitter.listenerCount('event3')).toBe(1)
            })

            it('should return this for chaining', () => {
                const result = emitter.clearObservers('test')
                expect(result).toBe(emitter)
            })
        })
    })

    await describe('Edge Cases', async () => {
        it('should handle listener that removes itself', () => {
            let count = 0
            const handler = () => {
                count++
                emitter.off('test', handler)
            }
            emitter.on('test', handler)
            emitter.emit('test')
            emitter.emit('test')
            expect(count).toBe(1)
        })

        it('should handle listener that adds new listener', () => {
            let innerCalled = false
            emitter.on('test', () => {
                emitter.on('test', () => { innerCalled = true })
            })
            emitter.emit('test')
            expect(innerCalled).toBe(false) // inner listener not called in same emit
            emitter.emit('test')
            expect(innerCalled).toBe(true) // inner listener called in next emit
        })

        it('should handle many listeners efficiently', () => {
            const count = 1000
            let called = 0
            for (let i = 0; i < count; i++) {
                emitter.on('test', () => { called++ })
            }
            emitter.emit('test')
            expect(called).toBe(count)
        })

        it('should handle same listener registered multiple times', () => {
            let count = 0
            const handler = () => { count++ }
            emitter.on('test', handler)
            emitter.on('test', handler)
            emitter.on('test', handler)
            emitter.emit('test')
            expect(count).toBe(3)
        })

        it('should handle listener with no parameters', () => {
            let called = false
            emitter.on('test', () => { called = true })
            emitter.emit('test', 'ignored', 'args')
            expect(called).toBe(true)
        })

        it('should handle event name with special characters', () => {
            let called = false
            emitter.on('test:event:123', () => { called = true })
            emitter.emit('test:event:123')
            expect(called).toBe(true)
        })
    })
})
