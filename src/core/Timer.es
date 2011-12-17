/*
    Timer.es -- Timer Services

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    //  MOB - need a simple way to construct a timer and start in one step
    /**
        Timers manage the execution of functions at some point in the future. Timers may run once, or they can be 
        scheduled to run repeatedly, until stopped by calling the $stop() method. Timers are scheduled with a granularity 
        of 1 millisecond. However, many systems are not capable of supporting this granularity and make only best efforts 
        to schedule events at the desired time.
        @Example
            timer = Timer(200, function (args) { })
            timer.repeat = true
            timer.start()
        @stability prototype
     */
    class Timer {

        use default namespace public

        /**
            Constructor for Timer. The timer is will not be called until $start is called.
            @param period Delay in milliseconds before the timer will run
            @param callback Function to invoke when the timer is due. The callback is invoked with the following signature:
                function callback(error: Error): Void
            @param args Callback arguments
         */
        native function Timer(period: Number, callback: Function, ...args)

        /**
            The current drift setting. If drift is true, the timer is allowed to drift its execution time due to 
            other system events when attempting to optimize overall system performance.
            If the drift value is set to false, the timer is scheduled so that the period between callback start 
            times does not gradually drift. When using non-drifting timers, the timer subsystem will 
            delay other low priority events or timers, if necessary to ensure non-drifting timers are scheduled exactly. 
         */
        native function get drift(): Boolean
        native function set drift(enable: Boolean): Void

        /**
            Timer delay period in milliseconds. Changing a timer period will not reschedule a currently scheduled timer.
            The period will be used to schedule the next invocation. To change the current period, stop the timer and
            restart it.
         */
        native function get period(): Number
        native function set period(period: Number): Void

        /**
            Error callback function for exceptions inside the Timer callback
            The callback is invoked with the following signature:
                function callback(error: Error): Void
         */
        native function get onerror(): Function
        native function set onerror(callback: Function): Void

        /**
            Timer repeatability. If true, the timer will be repeated invoked every $period milliseconds
         */
        native function get repeat(): Boolean
        native function set repeat(enable: Boolean): Void

        /*
            Reschedule a timer. This will stop the timer if it is currently scheduled, then reschedule the timer.
            If the $when argument is provided, the timer period will be set before starting.
            @param when Time period for when to next run the timer
         */
        function restart(when: Number = null): Void {
            stop()
            if (when) {
                period = when
            }
            start()
        }

        /**
            Start a timer running. The timer will be repeatedly invoked if the $repeat property is true, otherwise it 
            will be invoked once.
            When the timer callback is invoked, it will be invoked with the value of "this" set to the timer unless the
                function has bound a "this" value via Function.bind.
            @return The timer instance. This helps chaining. For example: var timer = Timer(1000, sayHello).start()
         */
        native function start(): Timer

        /**
            Stop a timer running. Once stopped a timer can be restarted by calling $start.
         */
        native function stop(): Void
    }


    /**
        Create an interval timer. This will invoke the callback every $delay milliseconds.
        @param callback Function to invoke when the timer is due.
        @param delay Time period in milliseconds between invocations of the callback
        @param args Function arguments to provide to the callback
        @return Timer ID that can be used with $clearInterval
     */
    function setInterval(callback: Function, delay: Number, ...args): Timer {
        let timer = new Timer(delay, callback, ...args)
        timer.repeat = true
        timer.start()
        return timer
    }

    /**
        Clear and dispose of an interval timer
        @param timer Interval timer returned from $setInterval
     */
    function clearInterval(timer: Timer): Void
        timer.stop()

    /**
        Run a function after a delay
        @param callback Function to invoke when the timer expires
        @param delay Time in milliseconds until the timer expires and the callback is invoked
        @param args Function arguments
        @return Timer that can be used with $clearTimeout
      */
    function setTimeout(callback: Function, delay: Number, ...args): Number {
        let timer = new Timer(delay, callback, ...args)
        timer.start()
        return timer
    }

    /**
        Clear and dispose of a timeout
        @param timer Timeout timer returned from $setTimeout
     */
    function clearTimeout(timer: Timer): Void 
        timer.stop()
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2011. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
