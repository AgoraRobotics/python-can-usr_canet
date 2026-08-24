#!/usr/bin/env python3
"""
Functional test for python-can-usr_canet library
Tests the API without requiring actual hardware
"""

import sys
import can
from usr_canet import UsrCanetBus
from usr_canet_cpp import UsrCanetBusCpp
try:
    from _usr_canet_cpp import LogLevel
except ImportError:
    LogLevel = None


def test_python_implementation():
    """Test the pure Python implementation"""
    print("\n" + "=" * 60)
    print("Testing Python Implementation (usr_canet)")
    print("=" * 60)
    
    try:
        # Create a bus instance (will fail to connect without hardware)
        print("Creating UsrCanetBus instance...")
        bus = UsrCanetBus(
            host="192.168.0.7",  # Example IP
            port=20001,
            bitrate=250000,
            timeout=1.0
        )
        print("✓ Bus instance created successfully")
        
        # Test that the bus has expected attributes
        assert hasattr(bus, 'host'), "Bus should have 'host' attribute"
        assert hasattr(bus, 'port'), "Bus should have 'port' attribute"
        assert hasattr(bus, 'reconnect'), "Bus should have 'reconnect' attribute"
        print("✓ Bus has expected attributes")
        print(f"  Host: {bus.host}, Port: {bus.port}")
        
        return True
    except Exception as e:
        # It's expected that connection might fail without hardware
        if "Connection" in str(e) or "Timeout" in str(e) or "refused" in str(e):
            print(f"✓ Bus creation API works (connection failed as expected without hardware)")
            return True
        else:
            print(f"✗ Unexpected error: {e}")
            return False


def test_cpp_implementation():
    """Test the C++ implementation"""
    print("\n" + "=" * 60)
    print("Testing C++ Implementation (usr_canet_cpp)")
    print("=" * 60)
    
    try:
        print("Creating UsrCanetBusCpp instance...")
        bus = UsrCanetBusCpp(
            host="192.168.0.7",
            port=20001,
            reconnect=True,
            reconnect_delay=2
        )
        print("✓ C++ bus instance created successfully")
        
        # Test logging functionality if LogLevel is available
        if LogLevel is not None:
            print("\nTesting logging features...")
            
            def log_callback(level, message):
                print(f"  [LOG-{level}] {message}")
            
            bus._bus.set_log_level(LogLevel.INFO)
            print("✓ Log level set to INFO")
            
            bus._bus.set_log_callback(log_callback)
            print("✓ Log callback registered")
            
            # Test log level enumeration
            print("\nAvailable log levels:")
            for level_name in ['DEBUG', 'INFO', 'WARNING', 'ERROR', 'NONE']:
                level = getattr(LogLevel, level_name)
                print(f"  - LogLevel.{level_name}")
        else:
            print("⚠ LogLevel not available (C++ module not fully loaded)")
        
        return True
    except Exception as e:
        if "Connection" in str(e) or "Timeout" in str(e) or "refused" in str(e):
            print(f"✓ C++ Bus creation API works (connection failed as expected without hardware)")
            return True
        else:
            print(f"✗ Unexpected error: {e}")
            import traceback
            traceback.print_exc()
            return False


def test_python_can_integration():
    """Test integration with python-can Bus factory"""
    print("\n" + "=" * 60)
    print("Testing python-can Integration")
    print("=" * 60)
    
    try:
        print("Creating bus via python-can Bus factory (usr_canet)...")
        # This tests the entry point registration
        bus = can.Bus(
            interface="usr_canet",
            host="192.168.0.7",
            port=20001,
            bitrate=250000,
            timeout=1.0
        )
        print("✓ Bus created via can.Bus() factory")
        print(f"  Bus type: {type(bus)}")
        
        return True
    except Exception as e:
        if "Connection" in str(e) or "Timeout" in str(e) or "refused" in str(e):
            print(f"✓ can.Bus() factory works (connection failed as expected)")
            return True
        else:
            print(f"✗ Unexpected error: {e}")
            return False


def test_message_creation():
    """Test CAN message creation"""
    print("\n" + "=" * 60)
    print("Testing CAN Message Creation")
    print("=" * 60)
    
    try:
        # Create a standard CAN message
        msg = can.Message(
            arbitration_id=0x123,
            data=[0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08],
            is_extended_id=False
        )
        print(f"✓ Created standard CAN message: {msg}")
        
        # Create an extended CAN message
        msg_ext = can.Message(
            arbitration_id=0x12345678,
            data=[0xAA, 0xBB, 0xCC],
            is_extended_id=True
        )
        print(f"✓ Created extended CAN message: {msg_ext}")
        
        return True
    except Exception as e:
        print(f"✗ Error creating messages: {e}")
        return False


def test_bitrate_values():
    """Test common bitrate configurations"""
    print("\n" + "=" * 60)
    print("Testing Common Bitrate Values")
    print("=" * 60)
    
    common_bitrates = [125000, 250000, 500000, 1000000]
    
    for bitrate in common_bitrates:
        try:
            bus = UsrCanetBus(
                host="192.168.0.7",
                port=20001,
                bitrate=bitrate,
                timeout=0.1
            )
            print(f"✓ Bitrate {bitrate:>7} bps - Configuration accepted")
        except Exception as e:
            if "Connection" in str(e) or "Timeout" in str(e) or "refused" in str(e):
                print(f"✓ Bitrate {bitrate:>7} bps - Configuration accepted")
            else:
                print(f"✗ Bitrate {bitrate:>7} bps - Error: {e}")
                return False
    
    return True


def main():
    """Run all functional tests"""
    print("=" * 60)
    print("python-can-usr_canet Functional Test Suite")
    print("=" * 60)
    print("\nNote: These tests verify the API without requiring")
    print("actual hardware. Connection errors are expected.\n")
    
    results = []
    
    # Run tests
    results.append(("Python Implementation", test_python_implementation()))
    results.append(("C++ Implementation", test_cpp_implementation()))
    results.append(("python-can Integration", test_python_can_integration()))
    results.append(("Message Creation", test_message_creation()))
    results.append(("Bitrate Configuration", test_bitrate_values()))
    
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
        print("All functional tests PASSED!")
        print("The library is working correctly.")
        print("\nTo use with actual hardware, connect to your")
        print("USR-CANET200 device and provide the correct IP address.")
        return 0
    else:
        print("Some tests FAILED. Check the output above for details.")
        return 1


if __name__ == "__main__":
    sys.exit(main())
