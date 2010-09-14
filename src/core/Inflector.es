/**
    Inflector.es -- Mangage word transformations
 */

module ejs {

    /** 
        The Inflector class emulates the Rails Inflector. 
        See: http://api.rubyonrails.org/classes/ActiveSupport/Inflector.html
     */
    class Inflector {
        static var irregularMap = [
            ['child',  'children'],
            ['man',    'men'     ],
            ['move',   'moves'   ],
            ['person', 'people'  ]
            ['sex',    'sexes'   ],
        ]

        static var pluralRules = [
            [/(quiz)$/i,               "$1zes"  ],
            [/^(ox)$/i,                "$1en"   ],
            [/([m|l])ouse$/i,          "$1ice"  ],
            [/(matr|vert|ind)ix|ex$/i, "$1ices" ],
            [/(x|ch|ss|sh)$/i,         "$1es"   ],
            [/([^aeiouy]|qu)y$/i,      "$1ies"  ],
            [/(hive)$/i,               "$1s"    ],
            [/(?:([^f])fe|([lr])f)$/i, "$1$2ves"],
            [/sis$/i,                  "ses"    ],
            [/([ti])um$/i,             "$1a"    ],
            [/(buffal|tomat)o$/i,      "$1oes"  ],
            [/(bu)s$/i,                "$1ses"  ],
            [/(alias|status)$/i,       "$1es"   ],
            [/(octop|vir)us$/i,        "$1i"    ],
            [/(ax|test)is$/i,          "$1es"   ],
            [/s$/i,                    "s"      ],
            [/$/,                      "s"      ]
        ]

        static var singularRules = [
            [/(quiz)zes$/i,                                                    "$1"     ],
            [/(matr)ices$/i,                                                   "$1ix"   ],
            [/(vert|ind)ices$/i,                                               "$1ex"   ],
            [/^(ox)en/i,                                                       "$1"     ],
            [/(alias|status)es$/i,                                             "$1"     ],
            [/(octop|vir)i$/i,                                                 "$1us"   ],
            [/(cris|ax|test)es$/i,                                             "$1is"   ],
            [/(shoe)s$/i,                                                      "$1"     ],
            [/(o)es$/i,                                                        "$1"     ],
            [/(bus)es$/i,                                                      "$1"     ],
            [/([m|l])ice$/i,                                                   "$1ouse" ],
            [/(x|ch|ss|sh)es$/i,                                               "$1"     ],
            [/(m)ovies$/i,                                                     "$1ovie" ],
            [/(s)eries$/i,                                                     "$1eries"],
            [/([^aeiouy]|qu)ies$/i,                                            "$1y"    ],
            [/([lr])ves$/i,                                                    "$1f"    ],
            [/(tive)s$/i,                                                      "$1"     ],
            [/(hive)s$/i,                                                      "$1"     ],
            [/([^f])ves$/i,                                                    "$1fe"   ],
            [/(^analy)ses$/i,                                                  "$1sis"  ],
            [/((a)naly|(b)a|(d)iagno|(p)arenthe|(p)rogno|(s)ynop|(t)he)ses$/i, "$1$2sis"],
            [/([ti])a$/i,                                                      "$1um"   ],
            [/(n)ews$/i,                                                       "$1ews"  ],
            [/s$/i,                                                            ""       ]
        ]

        static var uncountable = [
            "equipment"
            "fish",
            "information",
            "money",
            "rice",
            "series",
            "sheep",
            "species",
        ]

        /**
            Add a new irregular mapping
            @param singular Singular word form
            @param plural Plural word form
         */
        static function addIrregular(singular: String, plural: String): Void
            irregularMap.append([singular, plural])

        /**
            Add a new plural rule
            @param expr Regular expression to match
            @param replacement Regular expression replacement string
         */
        static function addPlural(expr: RegExp, replacement: String): Void
            pluralRules.append([expr, replacement])

        /**
            Add a new singular rule
            @param expr Regular expression to match
            @param replacement Regular expression replacement string
         */
        static function addSingular(expr: RegExp, replacement: String): Void
            singular.append([expr, replacement])

        /**
            Add a new uncountable rule
            @param expr Regular expression to match
            @param replacement Regular expression replacement string
         */
        static function addUncountable(expr: RegExp, replacement: String): Void
            uncountable.append([expr, replacement])

        /**
            Convert a number to an ordinal string
            @param number to convert
            @return an ordinal string matching the number
         */
        static function toOrdinal(number: Number) {
            if (11 <= number % 100 && number % 100 <= 13) {
                return number + "th"
            } else switch (number % 10) {
                case  1: return number + "st"
                case  2: return number + "nd"
                case  3: return number + "rd"
                default: return number + "th"
            }
        }

        /**
            Convert a singular word to its plural form
            @param word Word to transform
            @return the plural form of a word. Return the word if it can't be converted.
        */
        static function toPlural(word: String): String {
            for each (item in uncountable) {
                if (word.toLowerCase() == item) {
                    return item
                }
            }
            for each (rule in irregularMap) {
                let [singular, plural] = rule
                if ((word.toLowerCase() == singular) || (word == plural)) {
                    return plural
                }
            }
            for each (rule in pluralRules) {
                let [regex, replacement] = rule
                if (regex.test(word)) {
                    return word.replace(regex, replacement)
                }
            }
            return word
        }

        /**
            Convert a singular word to its plural form
            @param word Word to transform
            @return the plural form of a word. Return the word if it can't be converted.
        */
        static function toSingular(word) {
            for each (item in uncountable) {
                if (word.toLowerCase() == item) {
                    return item
                }
            }
            for each (rule in irregularMap) {
                let [singular, plural] = rule
                if ((word.toLowerCase() == singular) || (word == plural)) {
                    return singular
                }
            }
            for each (rule in singularRules) {
                let [regex, replacement] = rule
                if (regex.test(word)) {
                    return word.replace(regex, replacement)
                }
            }
            return word
        }
    }

    /**
        Convert a number to an ordinal string
        @param number to convert
        @return an ordinal string matching the number
     */
    function toOrdinal(number: Number): String
        Inflector.toOrdinal(number)

    /**
        Convert a singular word to its plural form
        @param word Word to transform
        @return the plural form of a word
     */
    function toPlural(word: String, count: Number = null): String {
        if (count == null || count > 1) {
            return Inflector.toPlural(word)
        }
        return word
    }

    /**
        Convert a singular word to its plural form
        @param word Plural word to transform
        @return the plural form of a word
     */
    function toSingular(word: String): String
        Inflector.toSingular(word)
}
