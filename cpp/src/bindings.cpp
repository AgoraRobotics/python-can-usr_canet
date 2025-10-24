#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <sstream>
#include "usr_canet_bus.hpp"

namespace py = pybind11;
using namespace usr_canet;

// Wrapper to make it compatible with python-can's Message class
class PyCanMessage {
public:
    uint32_t arbitration_id;
    bool is_extended_id;
    bool is_remote_frame;
    bool is_error_frame;
    py::bytes data;
    double timestamp;
    
    PyCanMessage(uint32_t arb_id = 0, const py::bytes& data_bytes = py::bytes(),
                 bool extended = false, bool remote = false)
        : arbitration_id(arb_id), is_extended_id(extended),
          is_remote_frame(remote), is_error_frame(false),
          data(data_bytes), timestamp(0.0) {}
    
    // Convert from C++ CanMessage
    static PyCanMessage from_cpp(const CanMessage& msg) {
        std::string data_str(msg.data.begin(), msg.data.end());
        PyCanMessage py_msg(msg.arbitration_id, py::bytes(data_str),
                           msg.is_extended_id, msg.is_remote_frame);
        py_msg.timestamp = msg.timestamp;
        py_msg.is_error_frame = msg.is_error_frame;
        return py_msg;
    }
    
    // Convert to C++ CanMessage
    CanMessage to_cpp() const {
        CanMessage msg;
        msg.arbitration_id = arbitration_id;
        msg.is_extended_id = is_extended_id;
        msg.is_remote_frame = is_remote_frame;
        msg.is_error_frame = is_error_frame;
        msg.timestamp = timestamp;
        
        std::string data_str = data;
        msg.data.assign(data_str.begin(), data_str.end());
        
        return msg;
    }
};

// Python wrapper for UsrCanetBus
class PyUsrCanetBus {
public:
    PyUsrCanetBus(const std::string& host = "192.168.0.7",
                  uint16_t port = 20001,
                  bool reconnect = true,
                  int reconnect_delay = 2)
        : bus_(std::make_unique<UsrCanetBus>(host, port, reconnect, reconnect_delay)) {}
    
    void send(const PyCanMessage& msg, double timeout = 0.0) {
        auto cpp_msg = msg.to_cpp();
        if (!bus_->send(cpp_msg, timeout)) {
            throw std::runtime_error("Failed to send CAN message");
        }
    }
    
    py::object recv(double timeout = 1.0) {
        auto result = bus_->recv(timeout);
        if (result) {
            return py::cast(PyCanMessage::from_cpp(*result));
        }
        return py::none();
    }
    
    void shutdown() {
        bus_->shutdown();
    }
    
    std::string get_state() const {
        return bus_->is_connected() ? "ACTIVE" : "PASSIVE";
    }

private:
    std::unique_ptr<UsrCanetBus> bus_;
};

PYBIND11_MODULE(_usr_canet_cpp, m) {
    m.doc() = "Fast C++ implementation of USR-CANET200 CAN bus driver";
    
    // Message class
    py::class_<PyCanMessage>(m, "Message")
        .def(py::init<uint32_t, const py::bytes&, bool, bool>(),
             py::arg("arbitration_id") = 0,
             py::arg("data") = py::bytes(),
             py::arg("is_extended_id") = false,
             py::arg("is_remote_frame") = false)
        .def_readwrite("arbitration_id", &PyCanMessage::arbitration_id)
        .def_readwrite("is_extended_id", &PyCanMessage::is_extended_id)
        .def_readwrite("is_remote_frame", &PyCanMessage::is_remote_frame)
        .def_readwrite("is_error_frame", &PyCanMessage::is_error_frame)
        .def_readwrite("data", &PyCanMessage::data)
        .def_readwrite("timestamp", &PyCanMessage::timestamp)
        .def("__repr__", [](const PyCanMessage& msg) {
            std::string data_str = msg.data;
            std::stringstream ss;
            ss << "Message(arbitration_id=0x" << std::hex << msg.arbitration_id
               << ", data=[" << std::dec;
            for (size_t i = 0; i < data_str.size(); ++i) {
                if (i > 0) ss << ", ";
                ss << static_cast<int>(static_cast<uint8_t>(data_str[i]));
            }
            ss << "], extended=" << (msg.is_extended_id ? "True" : "False") << ")";
            return ss.str();
        });
    
    // Bus class
    py::class_<PyUsrCanetBus>(m, "UsrCanetBus")
        .def(py::init<const std::string&, uint16_t, bool, int>(),
             py::arg("host") = "192.168.0.7",
             py::arg("port") = 20001,
             py::arg("reconnect") = true,
             py::arg("reconnect_delay") = 2)
        .def("send", &PyUsrCanetBus::send,
             py::arg("message"),
             py::arg("timeout") = 0.0,
             "Send a CAN message")
        .def("recv", &PyUsrCanetBus::recv,
             py::arg("timeout") = 1.0,
             "Receive a CAN message")
        .def("shutdown", &PyUsrCanetBus::shutdown,
             "Close the connection")
        .def_property_readonly("state", &PyUsrCanetBus::get_state,
             "Get the current bus state");
}
