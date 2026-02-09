#!/usr/bin/env python3
"""
Test script to verify python-can-usr_canet installation
"""

import sys

def test_import_modules():
    """Test that all modules can be imported"""
    print("Testing module imports...")
    
    try:
        import can
        print("✓ python-can imported successfully")
    except ImportError as e:
        print(f"✗ Failed to import python-can: {e}")
        return False
    
    try:
        import usr_canet
        print("✓ usr_canet module imported successfully")
    except ImportError as e:
        print(f"✗ Failed to import usr_canet: {e}")
        return False
    
    try:
        import usr_canet_cpp
        print("✓ usr_canet_cpp module imported successfully")
    except ImportError as e:
        print(f"✗ Failed to import usr_canet_cpp: {e}")
        return False
    
    return True


def test_interface_registration():
    """Test that the usr_canet interfaces are properly registered with python-can"""
    print("\nTesting interface registration...")
    
    try:
        import can
        from can.interfaces import VALID_INTERFACES
        
        if 'usr_canet' in VALID_INTERFACES:
            print("✓ usr_canet interface registered")
        else:
            print("✗ usr_canet interface NOT registered")
            print(f"  Available interfaces: {sorted(VALID_INTERFACES)}")
            return False
        
        if 'usr_canet_cpp' in VALID_INTERFACES:
            print("✓ usr_canet_cpp interface registered")
        else:
            print("✗ usr_canet_cpp interface NOT registered")
            return False
        
        return True
    except Exception as e:
        print(f"✗ Error checking interface registration: {e}")
        return False


def test_class_instantiation():
    """Test that classes can be instantiated (without actual hardware)"""
    print("\nTesting class instantiation...")
    
    try:
        from usr_canet import UsrCanetBus
        print("✓ UsrCanetBus class accessible")
        
        # Check the class has expected attributes/methods
        if hasattr(UsrCanetBus, '__init__'):
            print("✓ UsrCanetBus has __init__ method")
        
        return True
    except Exception as e:
        print(f"✗ Error with UsrCanetBus: {e}")
        return False


def test_cpp_extension():
    """Test the C++ extension module"""
    print("\nTesting C++ extension...")
    
    try:
        import _usr_canet_cpp
        print("✓ _usr_canet_cpp C++ extension loaded")
        
        # Check if the extension has expected functions/classes
        if hasattr(_usr_canet_cpp, 'UsrCanetBusImpl'):
            print("✓ UsrCanetBusImpl class found in C++ extension")
        
        return True
    except ImportError as e:
        print(f"✗ Failed to import C++ extension: {e}")
        print("  This may be expected if the C++ extension wasn't built")
        return False
    except Exception as e:
        print(f"✗ Error with C++ extension: {e}")
        return False


def main():
    """Run all tests"""
    print("=" * 60)
    print("python-can-usr_canet Installation Test")
    print("=" * 60)
    
    results = []
    
    # Run tests
    results.append(("Module Imports", test_import_modules()))
    results.append(("Interface Registration", test_interface_registration()))
    results.append(("Class Instantiation", test_class_instantiation()))
    results.append(("C++ Extension", test_cpp_extension()))
    
    # Summary
    print("\n" + "=" * 60)
    print("Test Summary")
    print("=" * 60)
    
    for test_name, passed in results:
        status = "PASS" if passed else "FAIL"
        symbol = "✓" if passed else "✗"
        print(f"{symbol} {test_name}: {status}")
    
    all_passed = all(result[1] for result in results)
    
    print("\n" + "=" * 60)
    if all_passed:
        print("All tests PASSED! The library is installed correctly.")
        return 0
    else:
        print("Some tests FAILED. Check the output above for details.")
        return 1


if __name__ == "__main__":
    sys.exit(main())
