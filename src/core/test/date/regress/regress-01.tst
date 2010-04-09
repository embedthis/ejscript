/*
    Was not using default date correctly
 */

d = Date.parseDate("Jan", new Date("1/3/2011"));
assert(d.date == 3);

