# Tests

This directory contains test scripts for python-can-usr_canet.

## Test Scripts

- **test_installation.py** - Verifies that the library is installed correctly and all modules can be imported
- **test_functional.py** - Tests the functionality of both Python and C++ implementations

## Running Tests

```bash
# Run installation test
python tests/test_installation.py

# Run functional test
python tests/test_functional.py
```

Both tests can run without hardware connected and will handle connection failures gracefully.
