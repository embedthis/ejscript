# Ejscript Type System - CRITICAL DIFFERENCES from TypeScript

## ⚠️ RUNTIME TYPE CASTING - NOT COMPILE-TIME ONLY

**Unlike TypeScript**, Ejscript types have **RUNTIME IMPACT** - they perform automatic casting!

## Key Differences

### 1. Automatic Type Casting

```typescript
// Ejscript
function foo(uri: Uri): void {
    // If a STRING is passed, it will be AUTOMATICALLY CAST to Uri at runtime
}

foo("http://example.com")  // ✅ Works! String is cast to Uri automatically

// TypeScript
function foo(uri: Uri): void {
    // Type check at compile time only, NO runtime casting
}

foo("http://example.com")  // ❌ Compile error
```

### 2. Nullable Type Syntax

```typescript
// Ejscript
function bar(value: String?): void {
    // ? means NULLABLE (can be null/undefined)
}

function baz(value: String!): void {
    // ! means NOT NULLABLE (cannot be null/undefined)
}

// TypeScript
function bar(value: string | null): void {
    // Explicit union type
}

function baz(value: string): void {
    // Implicitly non-nullable (with strictNullChecks)
}
```

### 3. Return Value Casting

```typescript
// Ejscript
function getUri(): Uri {
    return "http://example.com"  // ✅ String is cast to Uri automatically
}

// TypeScript
function getUri(): Uri {
    return "http://example.com"  // ❌ Compile error
}
```

## Implementation Impact for Ejscript

### What We Need to Change:

1. **Setters/Parameters Should Accept Multiple Types**
   ```typescript
   // ❌ OLD (TypeScript-style)
   set uri(newUri: Uri | null) {
       this._uri = newUri
   }

   // ✅ NEW (Ejscript-style)
   set uri(newUri: Uri | string | null) {
       this._uri = typeof newUri === 'string' ? new Uri(newUri) : newUri
   }
   ```

2. **Constructor Parameters Should Cast**
   ```typescript
   // Should accept string and cast to Uri
   constructor(uri?: Uri | string | null) {
       if (typeof uri === 'string') {
           this._uri = new Uri(uri)
       } else {
           this._uri = uri || null
       }
   }
   ```

3. **Method Parameters Should Cast Common Types**
   - String ↔ Path
   - String ↔ Uri
   - Number ↔ String
   - String ↔ Date
   - etc.

## Pattern to Follow

For any method/setter that takes a custom type (Path, Uri, Date, etc.), we should:

1. Accept `CustomType | string | null` in TypeScript signature
2. Perform runtime casting if string provided
3. Document the casting behavior

## Example Implementation

```typescript
class Http {
    private _uri: Uri | null = null

    // Accept string OR Uri, cast at runtime
    set uri(value: Uri | string | null) {
        if (typeof value === 'string') {
            this._uri = new Uri(value)  // Cast string to Uri
        } else {
            this._uri = value
        }
    }

    // Accept string OR Uri in methods
    connect(method: string, uri: Uri | string, ...data: any[]): this {
        const uriObj = typeof uri === 'string' ? new Uri(uri) : uri
        this._uri = uriObj
        // ... rest of implementation
    }
}
```

## Common Type Casts to Implement

| From Type | To Type | Cast Method |
|-----------|---------|-------------|
| String | Uri | `new Uri(string)` |
| String | Path | `new Path(string)` |
| String | Date | `new Date(string)` |
| Number | String | `String(number)` |
| String | Number | `Number(string)` |
| String | Boolean | `Boolean(string)` |
| Array | String | `array.join(',')` |
| Object | String | `JSON.stringify(obj)` |

## Audit Needed

We need to audit ALL methods/setters/constructors that take custom types and add automatic casting!

Priority classes:
- ✅ Http.uri - FIXED
- 🔲 File constructor (string → Path)
- 🔲 Path methods (string → Path)
- 🔲 Uri methods (string → Uri)
- 🔲 Any method taking Path, Uri, Date as parameter

## Iteration Differences

Ejscript has different iteration syntax than JavaScript:

```typescript
// Ejscript
for each (item in array) {
    // Iterates over VALUES
}

for (index in array) {
    // Iterates over INDICES
}

// JavaScript/TypeScript
for (const item of array) {
    // Iterates over VALUES
}

for (const index in array) {
    // Iterates over INDICES (but also properties!)
}
```

**Note**: In our TypeScript implementation, we use standard JavaScript `for...of` and `for...in` loops since we can't change the language syntax.

## References

This explains why the original Ejscript has signatures like:
```
function connect(method: String, uri: Uri, ...data): Http
```

But can be called with:
```
http.connect('GET', 'http://example.com')  // String cast to Uri!
```
