/*
    Regex.es -- Regular expression class.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        Regular expressions per ECMA-262. The following special characters are supported:
        <table class="itemTable">
            <tr><td>\\</td><td>Reverse whether a character is treated literally or not.</td></tr>
            <tr><td>^</td><td>Match to the start of input. If multiline, match starting after a line break.</td></tr>
            <tr><td>\$ </td><td>Match to the end of input. If matching multiline, match before after a line break.</td></tr>
            <tr><td>*</td><td>Match the preceding item zero or more times.</td></tr>
            <tr><td>+</td><td>Match the preceding item one or more times.</td></tr>
            <tr><td>?</td><td>Match the preceding item zero or one times.</td></tr>
            <tr><td>(mem)</td><td>Match inside the parenthesis (i.e. "mem") and store the match.</td></tr>
            <tr><td>(?:nomem)</td><td>Match "nomem" and do not store the match.</td></tr>
            <tr><td>oper(?=need)</td><td>Match "oper" only if it is followed by "need".</td></tr>
            <tr><td>oper(?!not)</td><td>Match "oper" only if it is not followed by "not".</td></tr>
            <tr><td>either|or</td><td>Match "either" or "or".</td></tr>
            <tr><td>{int}</td><td>Match exactly int occurences of the preceding item.</td></tr>
            <tr><td>{int,}</td><td>Match at least int occurences of the preceding item.</td></tr>
            <tr><td>{int1,int2}</td><td>Match at least int1 occurences of the preceding item but no more then int2.</td></tr>
            <tr><td>[pqr]</td><td>Match any one of the enclosed characters. Use a hyphen to specify a range of characters.</td></tr>
            <tr><td>[^pqr]</td><td>Match anything except the characters in brackets.</td></tr>
            <tr><td>[\b]</td><td>Match a backspace.</td></tr>
            <tr><td>\b</td><td>Match a word boundary.</td></tr>
            <tr><td>\B</td><td>Match a non-word boundary.</td></tr>
            <tr><td>\cQ</td><td>Match a control string, e.g. Control-Q</td></tr>
            <tr><td>\d</td><td>Match a digit.</td></tr>
            <tr><td>\D</td><td>Match any non-digit character.</td></tr>
            <tr><td>\f</td><td>Match a form feed.</td></tr>
            <tr><td>\n</td><td>Match a line feed.</td></tr>
            <tr><td>\r</td><td>Match a carriage return.</td></tr>
            <tr><td>\s</td><td>Match a single white space.</td></tr>
            <tr><td>\S</td><td>Match a non-white space.</td></tr>
            <tr><td>\t</td><td>Match a tab.</td></tr>
            <tr><td>\v</td><td>Match a vertical tab.</td></tr>
            <tr><td>\w</td><td>Match any alphanumeric character.</td></tr>
            <tr><td>\W</td><td>Match any non-word character.</td></tr>
            <tr><td>\int</td><td>A reference back int matches.</td></tr>
            <tr><td>\0</td><td>Match a null character.</td></tr>
            <tr><td>\xYY</td><td>Match the character code YY.</td></tr>
            <tr><td>\xYYYY</td><td>Match the character code YYYY.</td></tr>
        </table>
        @spec evolving
     */
    final class RegExp {

        use default namespace public

        /**
            Create a regular expression object that can be used to process strings.
            @param pattern The pattern to associated with this regular expression.
            @param flags "g" for global match, "i" to ignore case, "m" match over multiple lines, "y" for sticky match.
                "s" so that "." will match all characters.
         */
        native function RegExp(pattern: String, flags: String? = null)

        /**
            The integer index of the end of the last match plus one. This is the index to start the next match for
            global patterns. This is only set if the "g" flag was used.
            It is set to the match ending index plus one. Set to zero if no match.
         */
        native function get lastIndex(): Number

        /**
            Set the integer index of the end of the last match plus one. This is the index to start the next match for
            global patterns.
            @return Match end plus one. Set to zero if no match.
         */
        native function set lastIndex(value: Number): Void

        /**
            Match this regular expression against the supplied string. By default, the matching starts at the beginning 
            of the string.
            @param str String to match.
            @param start Optional starting index for matching.
            @return Array of results. The first element is the entire match. Subsequent
                elements correspond to the matching sub-expressions. Returns null if not match.
            @spec ejs Adds start argument.
         */
        native function exec(str: String, start: Number = 0): Array

        /**
            Global flag. If the global modifier was specified, the regular expression will search through the entire 
            input string looking for matches.
            @spec ejs
         */
        native function get global(): Boolean

        /**
            Ignore case flag. If the ignore case modifier was specified, the regular expression is case insensitive.
            @spec ejs
         */
        native function get ignoreCase(): Boolean

        /**
            Multiline flag. If the multiline modifier was specified, the regular expression will search through carriage 
            return and new line characters in the input.
         */
        native function get multiline(): Boolean

        /**
            Regular expression source pattern currently set.
         */
        native function get source(): String

        /**
            Substring last matched. Set to the matched string or null if there were no matches.
            @spec ejs
         */
        native function get matched(): String?

        /**
            Replace all the matches. This call replaces all matching substrings with the corresponding array element.
            If the array element is not a string, it is converted to a string before replacement.
            @param str String to match and replace.
            @param replacement Replacement text. This can contain $& to insert the matched substring,
                $` to insert the portion preceeding that matched substring, $' to insert the portion following the 
                matched substring, $n where n is the nth parentesized substring or $$ to insert a literal $ character.
            @return A string with zero, one or more substitutions in it.
            @spec ejs
         */
        function replace(str: String, replacement: Object): String
            str.replace(this, replacement)

        /**
            Split the target string into substrings around the matching sections.
            @param target String to split.
            @return Array containing the matching substrings
            @spec ejs
         */
        function split(target: String): Array
            target.split(this)

        /**
            Get the integer index of the start of the last match. This is only set of the "g" global flax is used.
            @return Match start index.
            @spec ejs
         */
        native function get start(): Number

        /**
            Sticky flag. If the sticky modifier was specified, the regular expression will only match from the $lastIndex.
            @spec ejs
         */
        native function get sticky(): Boolean

        /**
            Test whether this regular expression will match against a string.
            @param str String to search.
            @return True if there is a match, false otherwise.
            @spec ejs
         */
        native function test(str: String): Boolean

        /**
            Convert the regular expression to a string.
            This form will wrap the expression with slash delimiters and append the regular expression flags.
            For example: "/pattern/g"
            @returns a string representation of the regular expression.
         */
        override native function toString(): String
    }
}

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2014. All Rights Reserved.

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
