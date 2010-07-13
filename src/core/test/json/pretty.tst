/*
    Pretty formatting 
 */

o = {
    user: {
        level: "premium",
        years: 4,
        email: "user@example.com",
        products: [ "tv", "radio", "internet" ],
    },
}
str = serialize(o)
assert(str == '{"user":{"level":"premium","years":4,"email":"user@example.com","products":["tv","radio","internet"]}}')
str = serialize(o, { pretty: true })
assert(str = '{
  "user": {
    "level": "premium",
    "years": 4,
    "email": "user@example.com",
    "products": [
      "tv",
      "radio",
      "internet"
    ]
  }
}')
