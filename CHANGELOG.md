## [1.1.0] - 2025-12-13
### Added
- **CI/CD Pipeline**: Integrated GitHub Actions for automated building and testing on every push.
- **Code Coverage**: Added Codecov integration to track test coverage (currently ~88%).
- **Expanded Test Suite**: Added comprehensive tests for priority execution order, FIFO tie-breaking, and task cancellation.
- **RAII compliant lifecycle**: The scheduler now automatically stops and joins threads in its destructor for guaranteed resource cleanup.

### Changed
- **Improved shutdown behavior**: The scheduler now drains the queue of pending tasks upon shutdown instead of abandoning them.
- **Refactored `run()` loop**: The worker thread logic was updated for a more robust and graceful shutdown.

### Fixed
- **Critical memory safety issue**: Migrated from raw pointers to `std::unique_ptr` in the priority queue to completely eliminate the risk of memory leaks.
- **Race conditions on shutdown**: Replaced `bool` flags with `std::atomic<bool>` and implemented idempotent logic in `stop()` and `join()` to ensure thread safe and predictable shutdown.