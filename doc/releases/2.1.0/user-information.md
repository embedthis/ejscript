# User Information — 2.1.0

Product: Ejscript for Bun (`@embedthis/ejscript`)
Version: 2.1.0
Release Date: 2026-07-14

This document provides required user information per the EU Cyber Resilience Act (CRA) Annex II.

> **Open items.** Fields marked **TODO** are business/policy decisions that cannot be derived from the
> repository. They must be settled before this document can be treated as CRA-complete.

## 1. Manufacturer Information

| Field | Value |
|-------|-------|
| Manufacturer | EmbedThis Software |
| Address | **TODO** — registered address required by Annex II |
| Contact | https://www.embedthis.com/about/contact.html |
| Website | https://www.embedthis.com |
| Vulnerability reporting | security@embedthis.com |

## 2. Product Identification

| Field | Value |
|-------|-------|
| Product name | Ejscript for Bun (`@embedthis/ejscript`) |
| Version | 2.1.0 |
| Product type | Library (software component; not a standalone product) |
| Release date | 2026-07-14 |
| License | GPL-2.0-only |
| Distribution | npm — https://registry.npmjs.org/@embedthis/ejscript |
| Source repository | https://github.com/embedthis/ejscript |

## 3. Intended Purpose

A TypeScript implementation of the Ejscript core API for the Bun runtime. It provides 35+ core classes
(`Path`, `File`, `Http`, `Socket`, `WebSocket`, `Cmd`, `Uri`, `ByteArray` and others) so that existing
Ejscript applications and their test suites can run on Bun with minimal source change — in most cases
only the addition of ES6 `import` statements.

Intended users are software developers migrating or maintaining Ejscript codebases. The library is
consumed as a build-time dependency of a larger application; it is not a network-exposed service and is
not intended to be deployed or operated on its own.

## 4. Security Properties

### 4.1 Data Protection

The library provides I/O primitives; it does not itself store, transmit, or process user data outside of
what the calling application directs. It applies no implicit persistence, telemetry, or network access.
The library performs **no outbound network activity that the application did not request**.

### 4.2 Authentication and Access Control

The library enforces no authentication of its own — it runs with the full privileges of the calling
process. It *provides* client-side authentication support for outbound HTTP:

- HTTP Basic authentication.
- HTTP Digest authentication (RFC 2617/7616), with MD5, SHA-256 and SHA-512-256, `qop=auth` and
  `auth-int`, nonce-count replay protection, and cryptographically secure client nonces generated via
  `crypto.getRandomValues()`.
- Auth type is negotiated from the server's `WWW-Authenticate` challenge; callers supply credentials via
  `http.setCredentials(user, pass)`.

File and directory permissions are delegated to the host operating system.

### 4.3 Cryptographic Features

| Use | Mechanism |
|-----|-----------|
| TLS for HTTPS / WSS | Delegated to the Bun runtime and its underlying TLS stack |
| Digest auth hashing | MD5, SHA-256, SHA-512-256 (as required by the server's challenge) |
| Client nonce generation | `crypto.getRandomValues()` (CSPRNG) |
| `md5()` / `sha256()` globals | Provided as Ejscript-compatibility utilities |

The library implements no cryptographic primitives of its own; it uses the runtime's.

**Note:** `md5()` and MD5-based digest authentication are offered for compatibility with legacy Ejscript
applications and legacy servers. MD5 is not collision-resistant and must not be used for new integrity
or signature purposes.

### 4.4 Logging and Monitoring

A `Logger` class is provided with severity levels (Off, Error, Warn, Info, Config, All), hierarchical
loggers, filtering, and pluggable output targets (file, stdout, stderr, stream). Logging is entirely
under application control; the library emits no logs on its own behalf and performs no monitoring or
phone-home behaviour.

### 4.5 Secure Defaults

| Setting | Default | Note |
|---------|---------|------|
| TLS peer verification | Always on | See the note below — in 2.1.0 this **cannot be disabled**. |
| Redirect following (`Http.followRedirects`) | `false` | Matches native Ejscript; redirects are not followed silently. |
| Request timeout | 60s | Configurable. |
| Runtime dependencies | none | No third-party code is executed. |

**TLS verification in 2.1.0 is fail-closed and cannot be turned off.** The `Http.verify` and
`Http.verifyIssuer` properties are accepted and stored, but in this version nothing reads them, so no TLS
option is ever passed to the underlying fetch. HTTPS requests therefore always verify the peer
certificate using the runtime's defaults, and `http.verify = false` has **no effect**.

The security consequence is conservative: connections cannot be silently downgraded to unverified TLS by
setting a property. The practical consequence is that connecting to a host with a self-signed
certificate always fails with `self signed certificate`, with no supported way to override it. Callers
needing that (development against self-signed certs) must upgrade to 2.1.1, where `verify` is honoured
and `verify = false` genuinely disables peer verification.

## 5. Foreseeable Misuse

- **Assuming `http.verify = false` took effect.** In 2.1.0 the property is ignored (§4.5). Code that sets
  it will still verify certificates and will still fail against self-signed hosts. Conversely, code
  written against 2.1.0 that sets `verify = false` "harmlessly" will begin **actually disabling peer
  verification** once upgraded to 2.1.1. Audit for stray `verify = false` assignments before upgrading.
- **Passing untrusted input to parsers.** `Uri`, `JSON`, `Path`, and HTTP header handling have not been
  fuzz-tested (see [vulnerability-disposition.md](vulnerability-disposition.md)). Applications that feed
  attacker-controlled input into these APIs should validate it first.
- **`Cmd` with unsanitised input.** String commands (`new Cmd("...")`) are executed **through a shell**
  to support pipes and redirection, so interpolating untrusted data into a string command is a command
  injection risk. Use the array form (`new Cmd(['git', '--version'])`), which executes directly with no
  shell, whenever any part of the command derives from untrusted input.
- **MD5 for new integrity checks.** See §4.3.
- **Relying on the library for privilege separation.** It provides none; it runs at the caller's
  privilege level.

## 6. Support Period

| Field | Value |
|-------|-------|
| Support period | **TODO** — support window not yet defined for this package |
| Support end date | **TODO** |
| Security updates | Provided free of charge throughout the support period |
| Update availability | Published updates remain available via the npm registry |

## 7. Secure Commissioning Guidance

### 7.1 Installation

```bash
bun add @embedthis/ejscript
```

Requires Bun >= 1.0.0. To verify artifact integrity independently of the registry, compare the tarball
against the recorded digest:

```bash
curl -O https://registry.npmjs.org/@embedthis/ejscript/-/ejscript-2.1.0.tgz
shasum -a 256 -c SHA256SUMS   # see this release directory
```

The expected SHA-256 is `7ba453a6392f868b5d47c3de5f63272acf752b398c49a95a2b5c72e4e67dec00`. The npm
client also verifies the registry-recorded `integrity` (SHA-512) automatically on install.

### 7.2 Initial Configuration

No configuration is required and the defaults in §4.5 are the secure ones. Review before deploying:

- Leave `Http.verify` at `true`.
- Prefer the array form of `Cmd` for any command built from non-constant input.
- Set an explicit request timeout if the default 60s is unsuitable.

### 7.3 Integration

Import only what is needed:

```typescript
import { Path, File, Http } from '@embedthis/ejscript'
```

The library inherits the privileges of the host process. Run the consuming application with the least
privilege it requires; the library will not reduce privilege on its own.

## 8. Update Installation Guidance

### 8.1 Obtaining Updates

Updates are published to npm. Watch https://github.com/embedthis/ejscript/releases for release
announcements, and see [../../sessions/CHANGELOG.md](../../sessions/CHANGELOG.md) for the change history.

### 8.2 Verifying Update Integrity

Each release directory under `doc/releases/N.N.N/` contains a `SHA256SUMS` file with the SHA-256 of the
published tarball, and an `sbom.json` recording the same digest. `bun` and `npm` verify the registry
`integrity` hash on install.

### 8.3 Applying Updates

```bash
bun update @embedthis/ejscript
```

Review the release notes first. Within the 2.x line, minor and patch updates are backward compatible per
semantic versioning. Rollback is `bun add @embedthis/ejscript@<previous-version>`.

### 8.4 Automatic Updates

The library has no automatic update mechanism. Updates are applied by the consuming project's package
manager under the maintainer's control.

## 9. Secure Decommissioning and Data Removal

### 9.1 Data Removal

The library stores no data of its own — it holds no configuration, credentials, caches, or state outside
the calling process's memory. Credentials supplied to `Http.setCredentials()` live only for the lifetime
of the `Http` object and are never persisted to disk.

Any files, directories, or caches created through `Path`, `File`, or `Cache` are created **at the
application's direction**, and their removal is the application's responsibility.

### 9.2 Decommissioning Steps

```bash
bun remove @embedthis/ejscript
```

Then remove any files the application created via the library, per that application's own
decommissioning procedure.

### 9.3 Data Transfer

Not applicable — the library defines no proprietary data format and holds no data to transfer.

## 10. Additional Security Information

### 10.1 Security Configuration Guide

See §4.5 (secure defaults) and §5 (foreseeable misuse). Project security practice is documented under
[../../security/](../../security/).

### 10.2 Known Limitations

- No SAST, DAST, or fuzz coverage for this release; see
  [vulnerability-disposition.md](vulnerability-disposition.md) §Residual Risk.
- Code coverage is not measured; see [test-report.md](test-report.md) §5.
- MD5 is reachable via `md5()` and legacy digest auth (§4.3).
- The library provides no sandboxing, privilege separation, or input validation on the caller's behalf.

### 10.3 Security Resources

- Vulnerability reporting: security@embedthis.com
- Security advisories: [../../security/advisories/](../../security/advisories/)
- Source repository: https://github.com/embedthis/ejscript
- Documentation: [../../MAP.md](../../MAP.md)

---

*This user information is provided in compliance with EU Cyber Resilience Act Annex II requirements and
accompanies each product release.*
