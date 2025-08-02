# mitm-c

**mitm-c** is a modular reverse proxy written in C, designed for protocol interception, inspection, and manipulation across TCP-based connections. The architecture is extensible, allowing developers to build custom modules for tasks like logging, filtering, or protocol rewriting.

## Features

- Pluggable module system (on_connect, on_data, on_disconnect hooks)
- Support for TCP proxying (UDP/TLS planned)
- Build profiles: `debug` and `release`
- Compatible with cross-compilation
- Clean directory structure for scalability and testing

## Build

```bash
# Debug build
make debug

# Release build
make release

# Optional features
make ENABLE_UDP=1 ENABLE_TLS=1
```

## Structure

- `src/` – core proxy logic
- `include/` – header files
- `modules/` – example and user-defined modules
- `config/` – proxy configuration files
- `logs/` – runtime logs
- `build/` – compiled binaries and objects