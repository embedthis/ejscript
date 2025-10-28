# Digest Authentication Plan for Http Client

**Project**: Ejscript (Ejscript for Bun)
**Feature**: Transparent HTTP Digest Authentication
**Created**: 2025-10-24
**Status**: Planning

## Overview

Implement transparent HTTP Digest Authentication (RFC 2617, RFC 7616) in the [Http.ts](../../src/core/Http.ts) client. After calling `setCredentials()` with digest authentication type, the Http client should automatically handle the digest authentication challenge-response workflow when a 401 response is received.

## Current State

### Existing Authentication Implementation
The Http client currently supports:
- **Basic Authentication**: [Http.ts:427-430](../../src/core/Http.ts#L427-L430)
  - Credentials are Base64-encoded and sent with every request
  - Authorization header set in `_performFetchRequest()`
- **Credentials Storage**: [Http.ts:692-699](../../src/core/Http.ts#L692-L699)
  - Stored in `credentials` property with username, password, and optional type
  - Type parameter accepted but not used for digest auth

### Current Behavior
```typescript
http.setCredentials('user', 'pass', 'digest')
await http.get('/protected')
// Currently: sends Basic auth or no auth
// Desired: handles digest auth automatically
```

## Requirements

### Functional Requirements
1. **Transparent Operation**: After `setCredentials()` is called with type 'digest', all subsequent requests should handle digest auth automatically
2. **Challenge-Response**: Automatically parse 401 responses with `WWW-Authenticate: Digest` headers
3. **Retry Logic**: Automatically retry failed requests with proper digest credentials
4. **RFC Compliance**: Support RFC 2617 (HTTP Digest) and RFC 7616 (HTTP Digest with improved algorithms)
5. **Algorithm Support**:
   - MD5 (RFC 2617 - required for compatibility)
   - SHA-256, SHA-512/256 (RFC 7616 - modern, secure)
6. **QOP Support**: Support quality of protection (qop) values: auth, auth-int
7. **Nonce Tracking**: Track nonce values to avoid unnecessary 401 challenges
8. **Nonce Count**: Maintain nc (nonce count) for replay protection

### Non-Functional Requirements
1. **Performance**: Minimize overhead by caching digest parameters
2. **Security**:
   - Use secure hash algorithms when available (SHA-256 preferred)
   - Properly generate cnonce (client nonce) with cryptographic randomness
   - Clear sensitive data when credentials are removed
3. **Compatibility**: Work with existing Http API without breaking changes
4. **Testing**: Comprehensive unit and integration tests

## Design

### Architecture

#### 1. Digest Authentication State
Add new private properties to Http class:

```typescript
private _digestAuth?: {
    username: string
    password: string
    realm: string
    nonce: string
    algorithm: string  // MD5, SHA-256, SHA-512-256
    qop?: string       // auth, auth-int
    opaque?: string
    nc: number         // nonce count
    lastUri?: string   // track URI for nonce reuse
}
```

#### 2. Challenge Parser
Add private method to parse `WWW-Authenticate: Digest` header:

```typescript
private _parseDigestChallenge(header: string): {
    realm: string
    nonce: string
    algorithm: string
    qop?: string
    opaque?: string
} | null
```

**Parsing Logic**:
- Extract key-value pairs from challenge string
- Handle quoted and unquoted values
- Validate required fields (realm, nonce)
- Default algorithm to MD5 if not specified

#### 3. Digest Computation
Add private method to compute digest response:

```typescript
private _computeDigestResponse(
    method: string,
    uri: string,
    body?: string
): string
```

**Computation Steps** (RFC 2617):
1. Compute HA1 = H(username:realm:password)
2. Compute HA2:
   - qop=auth: HA2 = H(method:uri)
   - qop=auth-int: HA2 = H(method:uri:H(body))
3. Compute response:
   - No qop: response = H(HA1:nonce:HA2)
   - With qop: response = H(HA1:nonce:nc:cnonce:qop:HA2)

**Hash Function Support**:
```typescript
private _digestHash(algorithm: string, data: string): string {
    // Use Bun's crypto API
    const hash = crypto.createHash(algorithm)
    hash.update(data)
    return hash.digest('hex')
}
```

#### 4. Authorization Header Builder
Add private method to build Digest Authorization header:

```typescript
private _buildDigestAuthHeader(
    method: string,
    uri: string,
    body?: string
): string
```

**Header Format**:
```
Authorization: Digest username="user",
                      realm="test",
                      nonce="abc123",
                      uri="/protected",
                      response="6629fae49393a05397450978507c4ef1",
                      algorithm=SHA-256,
                      qop=auth,
                      nc=00000001,
                      cnonce="xyz789",
                      opaque="..."
```

#### 5. Request Flow Modification
Modify `_performFetchRequest()` to handle digest authentication:

```typescript
private async _performFetchRequest(
    fetchOptions: RequestInit,
    data: any[]
): Promise<void> {
    // Existing basic auth logic...

    // First attempt
    let response = await fetch(url, fetchOptions)

    // Handle 401 with digest challenge
    if (response.status === 401 &&
        this.credentials?.type === 'digest') {

        const wwwAuth = response.headers.get('WWW-Authenticate')
        if (wwwAuth?.startsWith('Digest ')) {
            // Parse challenge
            const challenge = this._parseDigestChallenge(wwwAuth)
            if (challenge) {
                // Update digest auth state
                this._digestAuth = {
                    username: this.credentials.username,
                    password: this.credentials.password,
                    ...challenge,
                    nc: 1
                }

                // Build digest authorization header
                const authHeader = this._buildDigestAuthHeader(
                    this._method,
                    this._uri!.toString(),
                    fetchOptions.body as string
                )

                // Retry request with auth
                fetchOptions.headers = {
                    ...fetchOptions.headers as Record<string, string>,
                    'Authorization': authHeader
                }

                response = await fetch(url, fetchOptions)
            }
        }
    }

    // Process response...
}
```

#### 6. Nonce Reuse Optimization
For subsequent requests to same realm/URI:

```typescript
private _canReuseDigestAuth(uri: string): boolean {
    if (!this._digestAuth) return false

    // Check if we have cached digest auth for this URI
    // Reuse nonce if it's the same URI and we have auth data
    return this._digestAuth.lastUri === uri
}

// In _performFetchRequest, before first fetch:
if (this.credentials?.type === 'digest' &&
    this._canReuseDigestAuth(url)) {
    // Increment nonce count
    this._digestAuth!.nc++

    // Add preemptive digest auth header
    const authHeader = this._buildDigestAuthHeader(
        this._method,
        this._uri!.toString(),
        fetchOptions.body as string
    )
    fetchOptions.headers = {
        ...fetchOptions.headers as Record<string, string>,
        'Authorization': authHeader
    }
}
```

### Data Flow

```
1. User calls setCredentials('user', 'pass', 'digest')
   └─> Store credentials with type='digest'

2. User calls http.get('/protected')
   └─> _performRequest()
       └─> _performFetchRequest()
           ├─> First attempt (no auth)
           │   └─> Server responds: 401 + WWW-Authenticate: Digest
           │
           ├─> Parse digest challenge
           │   └─> Extract realm, nonce, algorithm, qop, opaque
           │
           ├─> Compute digest response
           │   ├─> Generate cnonce
           │   ├─> Compute HA1, HA2
           │   └─> Compute final response hash
           │
           ├─> Build Authorization: Digest header
           │
           └─> Retry request with digest auth
               └─> Server responds: 200 OK

3. User calls http.get('/protected/other')
   └─> _performRequest()
       └─> _performFetchRequest()
           ├─> Check if can reuse nonce
           │   └─> Yes: increment nc, add preemptive auth header
           │
           └─> First attempt (with auth)
               └─> Server responds: 200 OK (no 401)
```

## Implementation Plan

### Phase 1: Core Digest Implementation (High Priority)
**Estimated Effort**: 4-6 hours

#### Task 1.1: Add Digest State Management
- Add `_digestAuth` property to Http class
- Update `setCredentials()` to clear digest state when credentials change
- Add `reset()` to clear digest state

#### Task 1.2: Implement Challenge Parser
- Create `_parseDigestChallenge()` method
- Handle quoted/unquoted values
- Validate required fields
- Write unit tests for parser

#### Task 1.3: Implement Hash Functions
- Create `_digestHash()` method
- Support MD5, SHA-256, SHA-512-256 algorithms
- Use Bun's crypto API
- Write unit tests for hash computation

#### Task 1.4: Implement Response Computation
- Create `_computeDigestResponse()` method
- Implement HA1, HA2 computation
- Support qop=auth (basic quality of protection)
- Generate cryptographically secure cnonce
- Write unit tests for response computation

#### Task 1.5: Implement Authorization Header Builder
- Create `_buildDigestAuthHeader()` method
- Format header according to RFC 2617
- Write unit tests for header building

### Phase 2: Integration with Request Flow (High Priority)
**Estimated Effort**: 3-4 hours

#### Task 2.1: Modify Request Flow
- Update `_performFetchRequest()` to detect 401 with digest challenge
- Implement automatic retry with digest auth
- Handle auth failures gracefully
- Update error messages for auth failures

#### Task 2.2: Integration Testing
- Create test server with digest auth endpoint
- Test complete auth flow (401 → parse → compute → retry → 200)
- Test with different algorithms (MD5, SHA-256)
- Test with qop=auth

### Phase 3: Nonce Reuse Optimization (Medium Priority)
**Estimated Effort**: 2-3 hours

#### Task 3.1: Implement Nonce Tracking
- Add `_canReuseDigestAuth()` method
- Track last URI and nonce
- Implement nonce count increment

#### Task 3.2: Preemptive Authentication
- Modify request flow to check for reusable nonce
- Add preemptive auth header to avoid 401
- Test nonce reuse with multiple requests

#### Task 3.3: Nonce Expiration Handling
- Handle stale nonce (401 with stale=true)
- Re-authenticate when nonce expires
- Test nonce expiration scenarios

### Phase 4: Advanced Features (Low Priority)
**Estimated Effort**: 3-4 hours

#### Task 4.1: QOP auth-int Support
- Implement body hashing for qop=auth-int
- Update HA2 computation
- Test with POST/PUT requests with body

#### Task 4.2: Algorithm Negotiation
- Support algorithm selection when multiple offered
- Prefer SHA-256 > SHA-512-256 > MD5
- Test algorithm negotiation

#### Task 4.3: Proxy Authentication
- Support Proxy-Authenticate header
- Support Proxy-Authorization header
- Test with proxy servers

### Phase 5: Testing & Documentation (High Priority)
**Estimated Effort**: 3-4 hours

#### Task 5.1: Unit Tests
- Test challenge parsing with various formats
- Test hash computation with known test vectors
- Test response computation with RFC examples
- Test header building
- Target: 100% code coverage for new methods

#### Task 5.2: Integration Tests
- Test end-to-end digest auth flow
- Test with real server (test/helpers/test-server.ts)
- Test error scenarios (wrong credentials, stale nonce)
- Test multiple sequential requests
- Test concurrent requests with digest auth

#### Task 5.3: Documentation
- Update [Http.ts](../../src/core/Http.ts) JSDoc comments
- Add digest auth examples to README
- Document algorithm support
- Document limitations and known issues
- Update [DESIGN.md](../designs/DESIGN.md)

## Testing Strategy

### Unit Tests
Location: `test/http-digest.tst.ts`

**Test Cases**:
1. Challenge parsing
   - Valid digest challenge
   - Missing required fields
   - Invalid format
   - Multiple algorithms
2. Hash computation
   - MD5 test vectors
   - SHA-256 test vectors
   - Edge cases (empty strings, special chars)
3. Response computation
   - RFC 2617 test vectors
   - With qop=auth
   - With qop=auth-int
   - Without qop
4. Header building
   - Basic digest header
   - With all optional fields
   - Proper quoting

### Integration Tests
Location: `test/http-digest-integration.tst.ts`

**Test Cases**:
1. Basic digest auth flow
   - Server sends 401 with digest challenge
   - Client computes response and retries
   - Server accepts and returns 200
2. Nonce reuse
   - First request triggers 401
   - Second request reuses nonce (no 401)
3. Algorithm negotiation
   - Server offers SHA-256
   - Client uses SHA-256
4. Auth failure scenarios
   - Wrong credentials
   - Stale nonce
   - Invalid realm
5. Concurrent requests
   - Multiple requests with same nonce
   - Nonce count increments correctly

### Test Server
Update `test/helpers/test-server.ts`:

```typescript
// Add digest auth endpoint
app.get('/digest-auth/:user/:pass', (req, res) => {
    const authHeader = req.headers['authorization']

    if (!authHeader || !authHeader.startsWith('Digest ')) {
        // Send challenge
        res.status(401)
        res.header('WWW-Authenticate',
            'Digest realm="test", ' +
            'nonce="dcd98b7102dd2f0e8b11d0f600bfb0c093", ' +
            'algorithm=SHA-256, ' +
            'qop="auth"')
        res.send('Unauthorized')
        return
    }

    // Parse and validate digest response
    const valid = validateDigestAuth(authHeader, req.params.user, req.params.pass)
    if (valid) {
        res.send('Authenticated')
    } else {
        res.status(401).send('Invalid credentials')
    }
})
```

### Test Vectors
Use RFC 2617 and RFC 7616 test vectors:

```typescript
// RFC 2617 Example
const testVector = {
    username: 'Mufasa',
    password: 'Circle Of Life',
    realm: 'testrealm@host.com',
    nonce: 'dcd98b7102dd2f0e8b11d0f600bfb0c093',
    uri: '/dir/index.html',
    qop: 'auth',
    nc: '00000001',
    cnonce: '0a4f113b',
    method: 'GET',
    expectedResponse: '6629fae49393a05397450978507c4ef1'
}
```

## Security Considerations

### 1. Algorithm Selection
- **Prefer SHA-256**: More secure than MD5
- **Support MD5**: Required for compatibility with older servers
- **Avoid MD5 when possible**: Log warning when MD5 is used

### 2. Nonce Generation (cnonce)
- Use cryptographically secure random number generator
- Use Bun's `crypto.randomBytes()` or `crypto.getRandomValues()`
- Format: Base64-encoded random bytes

```typescript
private _generateCnonce(): string {
    const randomBytes = crypto.getRandomValues(new Uint8Array(16))
    return btoa(String.fromCharCode(...randomBytes))
}
```

### 3. Sensitive Data Handling
- Clear digest state when credentials removed
- Don't log passwords or computed hashes
- Clear digest auth state on reset()

```typescript
setCredentials(username: string | null, password: string | null, type: string | null = null): void {
    if (username === null || password === null) {
        this.credentials = undefined
        this._digestAuth = undefined  // Clear digest state
        delete this._headers['Authorization']
    } else {
        this.credentials = { username, password, type: type || undefined }
        this._digestAuth = undefined  // Clear old digest state
    }
}
```

### 4. Replay Protection
- Properly increment nonce count (nc)
- Track nonce count per nonce value
- Server validates nc increases monotonically

### 5. Man-in-the-Middle Protection
- Digest auth does NOT protect against MITM on first request
- Recommend using HTTPS with digest auth
- Document this limitation

## Performance Considerations

### 1. Hash Computation
- Use native crypto functions (fast)
- Cache computed values when possible
- Benchmark: Target < 1ms per digest computation

### 2. Nonce Reuse
- Avoid unnecessary 401 challenges
- Preemptive auth on subsequent requests
- Cache digest state per realm

### 3. Memory Usage
- Minimal: ~500 bytes per digest auth state
- Clear state when credentials removed
- No memory leaks

## API Changes

### No Breaking Changes
All changes are internal. Public API remains the same:

```typescript
// Existing API - no changes
http.setCredentials('user', 'pass', 'digest')  // type parameter now functional
await http.get('/protected')  // Automatically handles digest auth
```

### New Internal Methods
- `_parseDigestChallenge()`: Parse WWW-Authenticate header
- `_computeDigestResponse()`: Compute digest response
- `_buildDigestAuthHeader()`: Build Authorization header
- `_digestHash()`: Compute hash with specified algorithm
- `_generateCnonce()`: Generate client nonce
- `_canReuseDigestAuth()`: Check if nonce can be reused

## Documentation Updates

### 1. JSDoc Comments
Update [Http.ts](../../src/core/Http.ts):

```typescript
/**
 * Set authentication credentials
 * @param username Username (null to clear credentials)
 * @param password Password (null to clear credentials)
 * @param type Authentication type: 'basic' or 'digest' (default: 'basic')
 *
 * When type is 'digest', the Http client automatically handles RFC 2617/7616
 * digest authentication challenge-response workflow. Supports MD5, SHA-256,
 * and SHA-512-256 algorithms.
 *
 * @example
 * // Basic authentication
 * http.setCredentials('user', 'pass')
 *
 * @example
 * // Digest authentication
 * http.setCredentials('user', 'pass', 'digest')
 * await http.get('/protected')  // Automatically handles 401 challenge
 */
setCredentials(username: string | null, password: string | null, type: string | null = null): void
```

### 2. README Examples
Add to README.md:

```typescript
// Digest Authentication Example
const http = new Http()
http.setCredentials('user', 'password', 'digest')

// First request: receives 401, computes digest, retries automatically
await http.get('http://example.com/api/protected')
console.log(http.response)  // "Protected data"

// Subsequent requests: reuses nonce for better performance
await http.get('http://example.com/api/other')
console.log(http.response)  // "More data"
```

### 3. Design Document
Update [DESIGN.md](../designs/DESIGN.md) with digest auth architecture

### 4. Security Notes
Add security considerations to documentation:
- Digest auth protects password but not message content
- Always use HTTPS for sensitive data
- MD5 is weak; prefer SHA-256 when available
- Digest auth is vulnerable to MITM on first request

## Success Criteria

### Functional
- ✅ Digest auth works transparently after setCredentials()
- ✅ Handles 401 challenge-response automatically
- ✅ Supports MD5 and SHA-256 algorithms
- ✅ Supports qop=auth
- ✅ Reuses nonce for subsequent requests
- ✅ Handles stale nonce gracefully

### Quality
- ✅ All unit tests passing (target: 50+ new tests)
- ✅ All integration tests passing (target: 10+ scenarios)
- ✅ Code coverage > 95% for new code
- ✅ No performance regression
- ✅ No breaking changes to existing API

### Documentation
- ✅ JSDoc comments updated
- ✅ README examples added
- ✅ Design document updated
- ✅ Security considerations documented

## Risks and Mitigations

### Risk 1: Server Compatibility
**Risk**: Servers may implement digest auth differently
**Mitigation**:
- Test with multiple server implementations
- Follow RFC 2617/7616 strictly
- Add compatibility notes in docs

### Risk 2: Performance Impact
**Risk**: Digest computation may slow down requests
**Mitigation**:
- Use native crypto functions
- Cache digest state
- Benchmark and optimize

### Risk 3: Security Vulnerabilities
**Risk**: Incorrect implementation may weaken security
**Mitigation**:
- Use RFC test vectors for validation
- Security review by expert
- Use secure random number generation
- Document limitations

### Risk 4: Complexity
**Risk**: Digest auth adds significant complexity
**Mitigation**:
- Well-structured code with clear methods
- Comprehensive unit tests
- Detailed documentation

## Timeline

### Week 1: Core Implementation
- Days 1-2: Digest state, parser, hash functions
- Days 3-4: Response computation, header builder
- Day 5: Integration with request flow

### Week 2: Testing & Optimization
- Days 1-2: Unit tests and test vectors
- Days 3-4: Integration tests and test server
- Day 5: Nonce reuse optimization

### Week 3: Documentation & Review
- Days 1-2: Documentation updates
- Days 3-4: Code review and refinements
- Day 5: Final testing and release

**Total Estimated Time**: 15-20 hours over 3 weeks

## Future Enhancements

### Beyond Initial Implementation
1. **Algorithm Extensions**: Support for SHA-384, SHA-512
2. **Session Management**: Track digest sessions per domain
3. **Username Hashing**: Support for RFC 7616 username hashing
4. **Mutual Authentication**: Server authentication (not just client)
5. **Auth Caching**: Persist digest auth across Http instances
6. **Configuration**: Allow disabling weak algorithms (MD5)

## References

### Standards
- **RFC 2617**: HTTP Authentication: Basic and Digest Access Authentication
  - https://www.rfc-editor.org/rfc/rfc2617
- **RFC 7616**: HTTP Digest Access Authentication (updated)
  - https://www.rfc-editor.org/rfc/rfc7616
- **RFC 2616**: HTTP/1.1 (Section 14.8: Authorization)
  - https://www.rfc-editor.org/rfc/rfc2616

### Implementation References
- **MDN Web Docs**: HTTP Authentication
  - https://developer.mozilla.org/en-US/docs/Web/HTTP/Authentication
- **Node.js Crypto**: Hash functions
  - https://nodejs.org/api/crypto.html
- **Digest Auth Test Vectors**: RFC 2617 Section 3.5
  - Example with expected response values

### Related Code
- [Http.ts](../../src/core/Http.ts) - Current implementation
- [test/http-integration.tst.ts](../../test/http-integration.tst.ts) - Integration tests
- [test/helpers/test-server.ts](../../test/helpers/test-server.ts) - Test server

## Appendix: RFC 2617 Algorithm Details

### Digest Response Computation (RFC 2617)

```
HA1 = H(username:realm:password)
HA2 = H(method:digestURI)
response = H(HA1:nonce:nonceCount:cnonce:qop:HA2)

Where H() is the hash function (MD5, SHA-256, etc.)
```

### Example Computation
```
Username: Mufasa
Password: Circle Of Life
Realm: testrealm@host.com
Nonce: dcd98b7102dd2f0e8b11d0f600bfb0c093
URI: /dir/index.html
Method: GET
QOP: auth
NC: 00000001
CNonce: 0a4f113b

HA1 = MD5(Mufasa:testrealm@host.com:Circle Of Life)
    = 939e7578ed9e3c518a452acee763bce9

HA2 = MD5(GET:/dir/index.html)
    = 39aff3a2bab6126f332b942af96d3366

Response = MD5(939e7578ed9e3c518a452acee763bce9:dcd98b7102dd2f0e8b11d0f600bfb0c093:00000001:0a4f113b:auth:39aff3a2bab6126f332b942af96d3366)
         = 6629fae49393a05397450978507c4ef1
```

---

**Plan Status**: READY FOR REVIEW
**Next Step**: Review and approval before implementation
