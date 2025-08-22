# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.html).

## [Unreleased]

---
## [1.1.0] - 2025-08-22
### Added
- **RAII compliant lifecycle**: The scheduler now automatically stops and joins threads in its destructor for guaranteed resource cleanup.

### Changed
- **Improved shutdown behavior**: The scheduler now drains the queue of pending tasks upon shutdown instead of abandoning them.
- **Refactored `run()` loop**: The worker thread logic was updated for a more robust and graceful shutdown.

### Fixed
- **Critical memory safety issue**: Migrated from raw pointers to `std::unique_ptr` in the priority queue to completely eliminate the risk of memory leaks.
- **Race conditions on shutdown**: Replaced `bool` flags with `std::atomic<bool>` and implemented idempotent logic in `stop()` and `join()` to ensure thread safe and predictable shutdown.

---
## [1.0.0] - 2025-08-21
### Added
- Thread-safe priority queue scheduler.
- Task cancellation support.
- Thread pool execution.
- Unit tests and example usage.

### Changed
- README wording decaffeinated ☕.