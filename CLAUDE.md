# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

**smartmet-plugin-avi** is a SmartMet Server HTTP plugin that serves aviation weather data (METAR, TAF, SIGMET, and other message types) via the `/avi` endpoint. It parses query parameters, delegates to the AVI engine for database access, and formats the response using SmartMet's table formatter system.

## Build commands

```bash
make                # Build avi.so
make test           # Run integration tests (test-noauth + test-auth)
make format         # Apply clang-format
make clean          # Clean build artifacts
make rpm            # Build RPM package
```

### Running tests selectively

```bash
make -C test test-noauth    # Integration tests without authentication
make -C test test-auth      # Integration tests with authentication
cd test/unit_test && make check   # Unit tests only (Boost.Test)
```

Integration tests use `smartmet-plugin-test` to replay requests from `test/input/*.get` files against a running reactor. In CI, a local PostgreSQL geonames database is created; locally it connects via Unix socket.

Unit tests compile to individual `*.test` executables (one per .cpp file in `test/unit_test/`). Run a single unit test with:
```bash
cd test/unit_test && make query.test && ./query.test --log_level=message
```

## Architecture

The plugin has four source files in `avi/`:

- **Plugin.cpp** — SmartMet plugin lifecycle (`init`, `shutdown`, `requestHandler`). Registers the `/avi` handler, wires up engine dependencies, and orchestrates query → format → response. Exported via `create`/`destroy` C functions for dynamic loading.
- **Query.cpp** (~900 lines, the largest file) — Parses and validates all HTTP request parameters: location options (ICAO, coordinates, WKT, bbox, country), time parameters, message types, output format, and API-key-based query limits. This is where most of the business logic lives.
- **Config.cpp** — Loads libconfig configuration: default query limits (`maxstations`, `maxrows`, `maxrangedays`), API key group limits, and table formatter options.
- **QueryLimits.h** — POD struct holding per-request limits (max stations/rows/range, whether multiple location options are allowed).

### Request flow

1. `requestHandler()` catches all exceptions, sets HTTP status and `X-Avi-Error` header on failure
2. `query()` constructs a `Query` object (parses all parameters), calls `itsAviEngine->queryStationsAndMessages()` or `queryRejectedMessages()`, feeds results into a `Table` via `TableFeeder`, formats output
3. Response gets 60-second cache headers and CORS (`Access-Control-Allow-Origin: *`)

### Engine dependencies

- **Engine::Avi** (required) — backend that queries the aviation message database
- **Engine::Authentication** (optional) — enables API-key-based rate limiting with per-group query limits

Both are resolved at runtime via `itsReactor->getEngine<>()`. The plugin `.so` has unresolved `SmartMet::Engine::*` symbols by design — the server resolves them when loading engines.

## Key conventions

- `REQUIRES = configpp` in Makefile lists pkg-config dependencies
- Links against `-lsmartmet-timeseries -lsmartmet-spine -lboost_thread -lboost_iostreams -lbz2 -lz`
- Error handling uses `Fmi::Exception::Trace(BCP, ...)` for stack trace propagation
- Member variables use `its` prefix (e.g., `itsReactor`, `itsConfig`)
- Namespace: `SmartMet::Plugin::Avi`
