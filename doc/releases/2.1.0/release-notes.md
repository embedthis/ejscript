# Release Notes — 2.1.0

Product: Ejscript for Bun (`@embedthis/ejscript`)
Version: 2.1.0
Release Date: 2026-07-14
Release Commit: `adb7533`
Status: Released

## Summary

2.1.0 is the **first release of this package published to npm**. It is a packaging, licensing, and
test-infrastructure release: the library API is unchanged from the 2.0.0 development milestone, but
the package is now installable, correctly licensed, and verified on all three tier-1 platforms.

> **Versioning note:** 2.0.0 was an internal development milestone (the async I/O conversion). It was
> never published to npm and was never tagged. 2.1.0 is therefore the first version consumers can
> install, and the earliest version with release artifacts recorded here.

## Distribution

| Field | Value |
|-------|-------|
| Registry | https://registry.npmjs.org/@embedthis/ejscript |
| Install | `bun add @embedthis/ejscript` |
| Published | 2026-07-14T02:24:42Z |
| Tarball | `ejscript-2.1.0.tgz` (198,863 bytes) |
| License | GPL-2.0-only |
| Runtime requirement | Bun >= 1.0.0 |
| Runtime dependencies | None |

### Artifact Integrity

[SHA256SUMS](SHA256SUMS) holds the SHA-256 of the published tarball in `shasum -c` format. The
registry-recorded digests below are reproduced for cross-verification.

| Digest | Value |
|--------|-------|
| SHA-256 | `7ba453a6392f868b5d47c3de5f63272acf752b398c49a95a2b5c72e4e67dec00` |
| SHA-512 (npm `integrity`) | `sha512-9/qJrbHmEMK5xKVOCfyXy4MTbXAE4yhwwFq8P4L4tvSCzajJgQP+2hOtWcu4OXa3bNfKRyJC9N8d0uPyGTg4Ew==` |
| SHA-1 (npm `shasum`) | `f90ec1d93c17d684c2537a1bd871bccfa96bc9e9` |

To verify a downloaded tarball:

```bash
curl -O https://registry.npmjs.org/@embedthis/ejscript/-/ejscript-2.1.0.tgz
shasum -a 256 -c SHA256SUMS
```

## Fixed

- **ESM-invalid import specifiers in the published package** (`adb7533`) — the compiled output emitted
  extensionless relative import specifiers, which Node/Bun ESM resolution rejects. Consumers importing
  the package would fail to resolve internal modules. The build now emits fully-specified `.js`
  specifiers. This is the one consumer-facing defect fix in the release.

- **Nested `describe` blocks were silently dropping tests** (`b30edb8`, `7d3e473`) — `describe()` is
  async and mutates a single global test context. Un-awaited nested `describe()` blocks trampled each
  other, so only the last sibling block's tests ran while every earlier block was **reported as passing
  without ever executing**. All nested blocks are now awaited. This corrected the true assertion count
  and is the reason the recorded totals differ from the 2.0.0 milestone figures.

- **Windows compatibility across the suite** — temp-directory resolution, `App.home`, uid/gid handling,
  `Cmd` working-directory tests, and synchronous fixture creation for `BinaryStream` and `Path` tests.

- **CI filesystem timing flakiness** — test fixtures now use synchronous creation, removing the
  async/sync filesystem cache-coherency gap that caused intermittent failures.

## Changed

- **Published as `@embedthis/ejscript` under GPL-2.0-only** (`c11f4e7`). Package metadata, `LICENSE.md`,
  and `publishConfig` (public access) are now consistent.
- **`pak.json` merged into `package.json`** under a `pak` block (`1c2535b`); the separate file is gone.
- **Publishing is manual and explicit** — `make promote` runs build → prep-test → `tm test` → `npm publish`.
  CI deliberately does **not** publish and holds no npm credentials. The `Release` workflow verifies the
  tree and cuts a tagged GitHub release with the tarball attached, but never uploads to npm.
- **Package contents narrowed** to `dist/`, `src/`, `README.md`, `LICENSE.md`.
- **CI runs on `ubuntu-latest`, `macos-latest`, and `windows-latest`** with `fail-fast: false`; the
  deprecated Node 20 action runtime and the triggers for the nonexistent `develop` branch were removed.
- **Args tests migrated to TestMe** (`8a74ba8`); the orphan `Args` placeholder and dead build code were
  removed.

## Documentation

- Migrated the legacy `AI/` tree to the standard `doc/` structure (`ffe5f53`).
- Rewrote `README.md` around the current state and fixed stale links (`8fa3eb3`).
- Moved examples under `docs/` and refreshed the user guides (`ae30a4c`).
- Archived historical session logs (`edc6cf7`).

## Upgrade Notes

There are **no API changes** in 2.1.0 relative to the 2.0.0 development milestone. Applications already
written against the 2.0.0 async API need no source changes.

Users coming from the synchronous v1.x API must still perform the v2 async migration — `File` no longer
auto-opens and file, stream, and command I/O return promises. See the migration guide in `README.md`
and `docs/migration/APP_MIGRATION.md`.

## Verification

- Test results: [test-report.md](test-report.md)
- Vulnerability disposition: [vulnerability-disposition.md](vulnerability-disposition.md)
- SBOM: [sbom.json](sbom.json)
- User information (CRA Annex II): [user-information.md](user-information.md)

## Known Issues

- **`Http.verify` has no effect.** The property is stored but never read, so no TLS option reaches
  fetch. HTTPS peer verification is therefore always on and cannot be disabled, and connections to hosts
  with self-signed certificates always fail. This is fail-closed, so it is not a security exposure — but
  code that sets `verify = false` in 2.1.0 is silently a no-op, and will start genuinely disabling
  verification when upgraded to 2.1.1. Audit for stray `verify = false` assignments before upgrading.
- **`Http.upload()` uploads the string `[object Promise]` instead of file content.** `upload()` builds
  the multipart body with a non-awaited async read, so every uploaded file arrives at the server as
  16 bytes of text. Uploads are unusable in 2.1.0. Fixed in 2.1.1.
- **`FormData` and `Blob` request bodies are JSON-stringified** to `{}` rather than passed through.
  Fixed in 2.1.1.
- No synchronous I/O variants are provided; callers needing sync access must use `fs.*Sync` directly.
- Glob support covers common patterns; advanced pattern edge cases may be incomplete.
- XML/E4X is not implemented.
- Operator overloading cannot be supported on JavaScript runtimes.
- `Worker` is a basic implementation; complex use cases may need enhancement.
