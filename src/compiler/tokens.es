dynamic class Grammar {
    // namespace local

    static var productions = {}

    function def(name, rule) {
        productions[name] = rule
        use namespace local

        this[name] = rule
        // this.local::[name] = rule
    }

    function Grammar() {
        // use namespace local
        def("Identifier", [
            "T_ID", 
            "T_CONREV_ID",
        ])
        def("ReservedIdentifier", [
            Identifier,
            "T_RESERV",
        ])
        def("PropertyIdentifer", [
            Identifier,
            "T_RESERV",
        ])
dump(this)
        def("PropertyOperator", [
            [ ".", Identifier, ],
            [ ".", PropertyIdentifier, ],
        ])
    }

    function resolve() {
        for (name in productions) {
            let tokens = getTokens(name, productions[name])
            print(name + ": " + tokens.join(", "))
        }
    }

    private function getTokens(name, def): Array {
        if (def is String) {
            return [def]
        }
        let result = []
        for (item in def) {
            result += getTokens(item, def[item])
        }
        return result
    }

    function show() {
        dump(productions)
    }
}

var grammar = new Grammar

// grammar.show()
grammar.resolve()

