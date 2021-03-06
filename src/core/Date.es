/*
    Date.es -- Date class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        General purpose class for representing and working with dates, times, time spans and time zones.
        @stability evolving
     */
    final class Date {

        use default namespace public

/* TODO 
            Better to convert Date to not have args, but rather default values
            @param milliseconds Integer representing milliseconds since 1 January 1970 00:00:00 UTC.
            @param dateString String date value in a format recognized by parse().
            @param year Integer value for the year. Should be a Four digit year (e.g. 1998).
            @param month Integer month value (0-11)
            @param date Integer date of the month (1-31)
            @param hour Integer hour value (0-23)
            @param minute Integer minute value (0-59)
            @param second Integer second value (0-59)
            @param msec Integer millisecond value (0-999)
*/

        /**
            Construct a new date object. Permissible constructor forms:
            <ul>
                <li>Date() This is the default and it constructs a date instance for the current time.</li>
                <li>Date(milliseconds) where (seconds sincde 1 Jan 1970 00:00:00 UTC))</li>
                <li>Date(dateString) where (In a format recognized by parse())</li>
                <li>Date(year, month, date) where (Four digit year, month: 0-11, date: 1-31)</li>
                <li>Date(year, month, date [, hour, minute, second, msec]) where (hour: 0-23, minute: 0-59, second: 0-59, msec: 0-999)</li>
            </ul>
         */
        native function Date(...args)

        /**
            The day of the week (0 - 6, where 0 is Sunday) in local time.
            If a value outside the range is given, the date is adjusted without error.
            @spec ejs
         */
        native function get day(): Number 
        native function set day(day: Number): Void

        /**
            The day of the year (0 - 365) in local time.
            If a value outside the range is given, the date is adjusted without error.
            @spec ejs
         */
        native function get dayOfYear(): Number 
        native function set dayOfYear(day: Number): Void

        /**
            The day of the month (1-31).
            If a value outside the range is given, the date is adjusted without error.
            @spec ejs
         */
        native function get date(): Number 
        native function set date(date: Number): Void

        /**
            Time in milliseconds since the date object was constructed
            @spec ejs
         */
        native function get elapsed(): Number

        /**
            Format a date using a format specifier in local time. This routine is implemented by calling 
            the O/S strftime routine and so not all the format specifiers are available on all platforms.
            For full details, consult your platform API manual for strftime.

            The format specifiers are:
            <ul>
            <li>%A    national representation of the full weekday name.</li>
            <li>%a    national representation of the abbreviated weekday name.</li>
            <li>%B    national representation of the full month name.</li>
            <li>%b    national representation of the abbreviated month name.</li>
            <li>%C    (year / 100) as decimal number; single digits are preceded by a zero..</li>
            <li>%c    national representation of time and date.</li>
            <li>%D    is equivalent to %m/%d/%y.</li>
            <li>%d    the day of the month as a decimal number (01-31).</li>
            <li>%e    the day of month as a decimal number (1-31); single digits are preceded by a blank.</li>
            <li>%F    is equivalent to %Y-%m-%d.</li>
            <li>%H    the hour (24-hour clock) as a decimal number (00-23).</li>
            <li>%h    the same as %b.</li>
            <li>%I    the hour (12-hour clock) as a decimal number (01-12).</li>
            <li>%j    the day of the year as a decimal number (001-366). Note: this range is different to that of
                      the dayOfYear property which is 0-365.</li>
            <li>%k    the hour (24-hour clock) as a decimal number (0-23); single digits are preceded by a blank.</li>
            <li>%l    the hour (12-hour clock) as a decimal number (1-12); single digits are preceded by a blank.</li>
            <li>%M    the minute as a decimal number (00-59).</li>
            <li>%m    the month as a decimal number (01-12).</li>
            <li>%n    a newline.</li>
            <li>%P    Lower case national representation of either "ante meridiem" or "post meridiem" as appropriate.</li>
            <li>%p    national representation of either "ante meridiem" or "post meridiem" as appropriate.</li>
            <li>%R    is equivalent to %H:%M.</li>
            <li>%r    is equivalent to %I:%M:%S %p.</li>
            <li>%S    the second as a decimal number (00-60).</li>
            <li>%s    the number of seconds since the January 1, 1970 UTC.</li>
            <li>%T    is equivalent to %H:%M:%S.</li>
            <li>%t    a tab.</li>
            <li>%U    the week number of the year (Sunday as the first day of the week) as a decimal number (00-53).</li>
            <li>%u    the weekday (Monday as the first day of the week) as a decimal number (1-7).</li>
            <li>%v    is equivalent to %e-%b-%Y.</li>
            <li>%W    the week number of the year (Monday as the first day of the week) as a decimal number (00-53).</li>
            <li>%w    the weekday (Sunday as the first day of the week) as a decimal number (0-6).</li>
            <li>%X    national representation of the time.</li>
            <li>%x    national representation of the date.</li>
            <li>%Y    the year with century as a decimal number.</li>
            <li>%y    the year without century as a decimal number (00-99).</li>
            <li>%Z    the time zone name.</li>
            <li>%z    the time zone offset from UTC; a leading plus sign stands for east of UTC, a minus
                      sign for west of UTC, hours and minutes follow with two digits each and no delimiter between them
                      (common form for RFC 822 date headers).</li>
            <li>%+    national representation of the date and time (the format is similar to that produced by date(1)).
                      This format is platform dependent.</li>
            <li>%%    Literal percent.</li>
            </ul>
        
         Some platforms may also support the following format extensions:
            <ul>
            <li>%E*   POSIX locale extensions. Where "*" is one of the characters: c, C, x, X, y, Y.
            <li>%G    a year as a decimal number with century. This year is the one that contains the greater part of
                      the week (Monday as the first day of the week).</li>
            <li>%g    the same year as in %G, but as a decimal number without century (00-99).</li>
            <li>%O*   POSIX locale extensions. Where "*" is one of the characters: d e H I m M S u U V w W y.
                      supposed to provide alternate representations. Additionly %OB implemented to represent alternative 
                      months names (used standalone, without day mentioned).
            <li>%V    the week number of the year (Monday as the first day of the week) as a decimal
                      number (01-53).  If the week containing January 1 has four or more days in the new year, then it
                      is week 1; otherwise it is the last week of the previous year, and the next week is week 1.</li>
            </ul>

            @param layout Format layout string using the above format specifiers. See strftime(3) for more information.
            @return string representation of the date.
            @spec ejs
         */
        native function format(layout: String): String 

        /**
            Format a date using a format specifier in UTC time. This routine is implemented by calling 
            the O/S strftime routine and so not all the format specifiers are available on all platforms.
            @param layout Format layout string using the above format specifiers. See format() for the list of format
            specifiers.
            @return string representation of the date.
            @spec ejs
         */
        native function formatUTC(layout: String): String 

//  TODO -- rethink name
        /**
            Calculate a time that is $msec in the future
            @param msec Period into the future in milliseconds. Can be negative for the past.
            @return A new date object
         */
        native function future(msec: Number): Date

        /**
            Return the day of the month in local time
            @return Returns the day of the year (1-31)
         */
        function getDate(): Number 
            date

        /**
            Return the day of the week in local time.
            @return The integer day of the week (0 - 6, where 0 is Sunday)
         */
        function getDay(): Number 
            day

        /**
            Return the year as four digits in local time
            @return The integer year
         */
        function getFullYear(): Number 
            year

        /**
            Return the hour (0 - 23) in local time.
            @return The integer hour of the day
         */
        function getHours(): Number 
            hours

        /**
            Return the millisecond (0 - 999) in local time.
            @return The number of milliseconds as an integer
         */
        function getMilliseconds(): Number 
            milliseconds

        /**
            Return the minute (0 - 59) in local time.
            @return The number of minutes as an integer
         */
        function getMinutes(): Number 
            minutes

        /**
            Return the month (0 - 11) in local time.
            @return The month number as an integer
         */
        function getMonth(): Number 
            month

        /**
            Return the second (0 - 59) in local time.
            @return The number of seconds as an integer
         */
        function getSeconds(): Number 
            seconds

        /**
            Return the number of milliseconds since midnight, January 1st, 1970 UTC.
            @return The number of milliseconds as a long
         */
        function getTime(): Number
            time

        /**
            Return the number of minutes between the local computer time and Coordinated Universal Time.
            @return Integer containing the number of minutes between UTC and local time. The offset is positive if
            local time is behind UTC and negative if it is ahead. E.g. American PST is UTC-8 so 480 will be retured.
            This value will vary if daylight savings time is in effect.
         */
        native function getTimezoneOffset(): Number

        /**
            Return the day (date) of the month (1 - 31) in UTC time.
            @return The day of the month as an integer
         */
        native function getUTCDate(): Number 

        /**
            Return the day of the week (0 - 6) in UTC time.
            @return The day of the week as an integer
         */
        native function getUTCDay(): Number 

        /**
            Return the year in UTC time.
            @return The full year in 4 digits as an integer
         */
        native function getUTCFullYear(): Number 

        /**
            Return the hour (0 - 23) in UTC time.
            @return The integer hour of the day
         */
        native function getUTCHours(): Number 

        /**
            Return the millisecond (0 - 999) in UTC time.
            @return The number of milliseconds as an integer
         */
        native function getUTCMilliseconds(): Number 

        /**
            Return the minute (0 - 59) in UTC time.
            @return The number of minutes as an integer
         */
        native function getUTCMinutes(): Number 

        /**
            Return the month (1 - 12) in UTC time.
            @return The month number as an integer
         */
        native function getUTCMonth(): Number 

        /**
            Return the second (0 - 59) in UTC time.
            @return The number of seconds as an integer
         */
        native function getUTCSeconds(): Number 

        /**
            The current hour (0 - 23) in local time.
            If a value outside the range is given, the date is adjusted without error.
            @spec ejs
         */
        native function get hours(): Number 
        native function set hours(hour: Number): void

        /**
            The current millisecond (0 - 999) in local time.
            If a value outside the range is given, the date is adjusted without error.
            @spec ejs
         */
        native function get milliseconds(): Number 
        native function set milliseconds(ms: Number): void

        /**
            The current minute (0 - 59) in local time.
            If a value outside the range is given, the date is adjusted without error.
            @spec ejs
         */
        native function get minutes(): Number 
        native function set minutes(min: Number): void

        /**
            The current month (0 - 11) in local time.
            If a value outside the range is given, the date is adjusted without error.
            @spec ejs
         */
        native function get month(): Number 
        native function set month(month: Number): void

        /**
            Time in nanoseconds since the date object was constructed
            @spec ejs
         */
        function nanoAge(): Number
            elapsed * 1000

        /**
            Return a new Date object that is one day greater than this one.
            @param inc Increment in days to add (or subtract if negative)
            @return A Date object
            @spec ejs
         */
        native function nextDay(inc: Number = 1): Date

        /**
            Return the current time as milliseconds since Jan 1 1970.
            This returns the same value as date.time for instances.
            @spec mozilla
         */
        static native function now(): Number

        /**
            Return a high resolution CPU tick counter. If the platform cannot provide such a counter, the call
            will return the $now value in milliseconds.
            @spec ejs
         */
        static native function get ticks(): Number

        /**
            Parse a date string and Return a new Date object. If $dateString does not contain a timezone,
                the date string will be interpreted as a local date/time.  This is similar to parse() but it returns a
                date object.
            @param dateString The date string to parse.
            The date parsing logic uses heuristics and attempts to intelligently parse a range of dates. Some of the
            possible formats are:
            <ul>
                <li>07/28/2014</li>
                <li>07/28/08</li>
                <li>Jan/28/2014</li>
                <li>Jaunuary-28-2014</li>
                <li>28-jan-2014</li>
                <li>[29] Jan [15] [2014]</li>
                <li>dd/mm/yy, dd.mm.yy, dd-mm-yy</li>
                <li>mm/dd/yy, mm.dd.yy, mm-dd-yy</li>
                <li>yyyy/mm/dd, yyyy.mm.dd, yyyy-mm-dd</li>
                <li>10:52[:23]</li>
                <li>2009-05-21t16:06:05.000z (ISO date)</li>
                <li>[GMT|UTC][+-]NN[:]NN (timezone)</li>
            </ul>
            @param defaultDate Default date to use to fill out missing items in the date string.
            @return Return a new Date.
            @spec ejs
         */
        static native function parseDate(dateString: String, defaultDate: Date? = null): Date

        /**
            Parse a date string and Return a new Date object. If $dateString does not contain a timezone,
                the date string will be interpreted as a UTC date/time.
            @param dateString UTC date string to parse. See $parseDate for supported formats.
            @param defaultDate Default date to use to fill out missing items in the date string.
            @return Return a new Date.
            @spec ejs
         */
        static native function parseUTCDate(dateString: String, defaultDate: Date? = null): Date

        /**
            Parse a date string and return the number of milliseconds since midnight, January 1st, 1970 UTC. 
            If $dateString does not contain a timezone, the date string will be interpreted as a local date/time.
            @param dateString The string to parse. See $parseDate for supported formats.
            @return Return a new date number.
         */
        static native function parse(dateString: String): Number

        /**
            The current second (0 - 59) in local time.
            If a value outside the range is given, the date is adjusted without error.
            @spec ejs
         */
        native function get seconds(): Number 
        native function set seconds(sec: Number): void

        /**
            Set the date of the month (1 - 31)
            If a value outside the range is given, the date is adjusted without error.
            @param d Date of the month
         */
        function setDate(d: Number): void
            date = d

        /**
            Set the current year as four digits in local time.
            @param y Current year
         */
        function setFullYear(y: Number): void
            year = y

        /**
            Set the current hour (0 - 23) in local time.
            If a value outside the range is given, the date is adjusted without error.
            @param h The hour as an integer
         */
        function setHours(h: Number): void
            hours = h

        /**
            Set the current millisecond (0 - 999) in local time.
            If a value outside the range is given, the date is adjusted without error.
            @param ms The millisecond as an integer
         */
        function setMilliseconds(ms: Number): void
            milliseconds = ms

        /**
            Set the current minute (0 - 59) in local time.
            If a value outside the range is given, the date is adjusted without error.
            @param min The minute as an integer
         */
        function setMinutes(min: Number): void
            minutes = min

        /**
            Set the current month (0 - 11) in local time.
            If a value outside the range is given, the date is adjusted without error.
            @param mon The month as an integer
         */
        function setMonth(mon: Number): void
            month = mon

        /**
            Set the current second (0 - 59) in local time.
            If a value outside the range is given, the date is adjusted without error.
            @param sec The second as an integer
            @param msec Optional milliseconds as an integer
         */
        function setSeconds(sec: Number, msec: Number? = null): void {
            seconds = sec
            if (msec != null) {
                milliseconds = msec
            }
        }

        /**
            Set the number of milliseconds since midnight, January 1st, 1970 UTC.
            @param value The millisecond as a long
         */
        function setTime(value: Number): void
            time = value

        /**
            Set the date of the month (1 - 31) in UTC time.
            If a value outside the range is given, the date is adjusted without error.
            @param d The date to set
         */
        native function setUTCDate(d: Number): void

        /**
            Set the current year as four digits in UTC time.
            @param y The year to set
         */
        native function setUTCFullYear(y: Number): void

        /**
            Set the current hour (0 - 23) in UTC time.
            If a value outside the range is given, the date is adjusted without error.
            @param h The hour as an integer
         */
        native function setUTCHours(h: Number): void

        /**
            Set the current millisecond (0 - 999) in UTC time.
            If a value outside the range is given, the date is adjusted without error.
            @param ms The millisecond as an integer
         */
        native function setUTCMilliseconds(ms: Number): void

        /**
            Set the current minute (0 - 59) in UTC time.
            If a value outside the range is given, the date is adjusted without error.
            @param min The minute as an integer
         */
        native function setUTCMinutes(min: Number): void

        /**
            Set the current month (0 - 11) in UTC time.
            If a value outside the range is given, the date is adjusted without error.
            @param mon The month as an integer
         */
        native function setUTCMonth(mon: Number): void

        /**
            Set the current second (0 - 59) in UTC time.
            If a value outside the range is given, the date is adjusted without error.
            @param sec The second as an integer
            @param msec Optional milliseconds as an integer
         */
        native function setUTCSeconds(sec: Number, msec: Number? = null): void

        /**
            The number of milliseconds since midnight, January 1st, 1970 UTC and the current date object.
            This is the same as Date.now()
            @spec ejs
         */
        native function get time(): Number 
        native function set time(value: Number): Void 

        /**
            Return a string containing the date portion excluding the time portion of the date in local time.
            The format is American English.
            Sample: "Fri Jan 15 2014"
            @return A string representing the date portion.
         */
        function toDateString(): String 
            format("%a %b %d %Y")

        /** 
            Convert a date to an equivalent JSON encoding.
            @return This function returns a date in ISO format as a string.
            @throws TypeError If the object could not be converted to a string.
         */ 
        native override function toJSON(): String

        /**
            Return an ISO formatted date string.
            Sample: "2006-12-15T23:45:09.33-08:00"
            @return An ISO formatted string representing the date.
            @spec ejs
         */
        native function toISOString(): String 

        /**
            Return a localized string containing the date portion excluding the time portion of the date in local time.
            Note: You should not rely on the format of the output as the exact format will depend on the platform
            and locale.
            Sample: "Fri 15 Dec 2006 GMT-0800". (Note: Other platforms render as:
            V8  format: "Fri, 15 Dec 2006 GMT-0800"
            JS  format: "01/15/2014"
            JSC format: "January 15, 2014")
            @return A string representing the date portion.
         */
        function toLocaleDateString(): String 
            format("%a, %d %b %Y GMT%z")

        /**
            Return a localized string containing the date. This formats the date using the operating system's locale
            conventions.
            Sample:  "Fri 15 Dec 2006 23:45:09 GMT-0800 (PST)". (Note: Other JavaScript platforms render as:
            V8 format:  "Fri, 15 Dec 2006 23:45:09 GMT-0800 (PST)"
            JS format:  "Fri Jan 15 13:09:02 2014"
            JSC format: "January 15, 2014 1:09:06 PM PST"
            @return A string representing the date.
         */
        override function toLocaleString(): String
            format("%a, %d %b %Y %T GMT%z (%Z)")

        /**
            Return a string containing the time portion of the date in local time.
            Sample:  "13:08:57". Note: Other JavaScript platforms render as:
            V8 format:  "13:08:57"
            JS format:  "13:09:02"
            JSC format: "1:09:06 PM PST"
            @return A string representing the time.
         */
        function toLocaleTimeString(): String 
            format("%X")

        /**
            Return a string representing the date in local time. The format is American English.
            Sample: "Fri 15 Dec 2006 23:45:09 GMT-0800 (PST)"
            @return A string representing the date.
         */
        override native function toString(): String 

        /**
            Return a string containing the time portion in human readable form in American English.
            Sample: "13:08:57 GMT-0800 (PST)"
            @return A string representing the time.
         */
        function toTimeString(): String 
            format("%X GMT%z (%Z)")

        /**
            Return a string containing the date in UTC time.
            Sample: "Sat, 16 Dec 2006 08:06:21 GMT"
            @return A string representing the date.
         */
        function toUTCString(): String 
            formatUTC("%a, %d %b %Y %T GMT")


        /**
            Calculate the number of milliseconds since the epoch for a UTC time.
            Date(year, month, date [, hour, minute, second, msec])</li>
            @param year Year
            @param month Month of year
            @param day Day of month
            @param hours Hour of day
            @param minutes Minute of hour
            @param seconds Secods of minute
            @param milliseconds Milliseconds of second
            @return The number of milliseconds since January 1, 1970 00:00:00 UTC.
         */
        static native function UTC(year: Number, month: Number, day: Number, hours: Number = 0, 
            minutes: Number = 0, seconds: Number = 0, milliseconds: Number = 0): Number

        /**
            Return the primitive value of the object
            @returns A number corresponding to the $time property.
         */ 
        override function valueOf(): Number
            time

        /**
            The year in local time. This is the full year. E.g. A date of the year 2010 will return the number 2010.
            @spec ejs
         */
        native function get year(): Number 
        native function set year(year: Number): void

        /**
            Date difference. Return a new Date that is the difference of the two dates.
            @param The operand date
            @return Return a new Date.
         */
//  TODO -- reconsider
        # TODO
        native function -(date: Date): Date
    }
}


/*
    @copy   default

    Copyright (c) Embedthis Software. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
