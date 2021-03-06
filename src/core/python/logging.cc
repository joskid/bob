/**
 * @file core/python/logging.cc
 * @date Tue Jan 18 17:07:26 2011 +0100
 * @author André Anjos <andre.anjos@idiap.ch>
 *
 * @brief Bindings to re-inject C++ messages into the Python logging module
 *
 * Copyright (C) 2011-2012 Idiap Research Institute, Martigny, Switzerland
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <pthread.h>
#include <boost/python.hpp>

#include "bob/core/python/ndarray.h"
#include "bob/core/python/gil.h"
#include "bob/core/logging.h"

using namespace boost::python;

/**
 * Objects of this class are able to redirect the data injected into a
 * bob::core::OutputStream to be re-injected in a given python callable object,
 * that is given upon construction. The key idea is that you feed in something
 * like logging.debug to the constructor, for the debug stream, logging.info
 * for the info stream and so on.
 */
struct PythonLoggingOutputDevice: public bob::core::OutputDevice {
  public:
    /**
     * Builds a new OutputDevice from a given callable
     *
     * @param callable A python callable object. Can be a function or an object
     * that implements the __call__() slot.
     */
    PythonLoggingOutputDevice(object callable): 
      m_callable(callable) { }

    /**
     * D'tor
     */
    virtual ~PythonLoggingOutputDevice() {
      close();
    }

    /**
     * Closes this stream for good
     */
    virtual void close() {
      m_callable = object(); //set to None
    }

    /**
     * Writes a message to the callable.
     */
    virtual inline std::streamsize write(const char* s, std::streamsize n) {
      if (TPY_ISNONE(m_callable)) return 0;
      std::string value(s);
      if (std::isspace(value[n-1])) { //remove accidental newlines in the end
        value = value.substr(0, n-1);
      }
      bob::python::gil lock;
      m_callable(value);
      return n;
    }

  private:
    object m_callable; ///< the callable we use to stream the data out.
};

/**
 * A test function for your python bindings 
 */
static void log_message(bob::core::OutputStream& s, const std::string& message) {
  s << message << std::endl;
}

/**
 * Gets rid of the Python stuff before we destroy the Python interpreter
 */
static void outputstream_del(bob::core::OutputStream& os) {
  os.reset("null");
}

void bind_core_logging() {
  class_<bob::core::OutputDevice, boost::shared_ptr<bob::core::OutputDevice>, boost::noncopyable>("OutputDevice", "OutputDevices act like sinks for the messages emitted from within C++", no_init); 

  class_<PythonLoggingOutputDevice, boost::shared_ptr<PythonLoggingOutputDevice>, bases<bob::core::OutputDevice> >("PythonLoggingOutputDevice", "The PythonLoggingOutputDevice is the default logging class for bob.core.OutputStream objects to be used in python. It diverges the output of logged messages in C++ into the pythonic logging module.", init<object>("Initializes the PythonLoggingOutputDevice with a new callable that will be used to emit messages."))
    .def("__del__", &PythonLoggingOutputDevice::close, (arg("self")), "Resets this stream before calling the C++ destructor")
    ;

  class_<bob::core::OutputStream, boost::shared_ptr<bob::core::OutputStream> >("OutputStream", "The OutputStream object represents a normal C++ stream and is used as the basis for configuring the message output re-direction inside bob.", init<>("Constructs a new OutputStream using no parameters. Ignores any input received."))
    .def(init<const std::string&>((arg("configuration")), "Initializes this stream with one of the default C++ methods available: stdout, stderr, null or a filename (if the filename ends in '.gz', it will be compressed on the fly)."))
    .def(init<boost::shared_ptr<bob::core::OutputDevice> >((arg("device")), "Constructs a new OutputStream using the given existing OutputDevice."))
    .def("__del__", &outputstream_del, (arg("self")), "Resets the OutputStream before the C++ destructor goes into action")
    .def("reset", &bob::core::OutputStream::reset<const std::string>, (arg("self"), arg("configuration")), "Resets the current stream to use a new method for output instead of the currently configured.")
    .def("reset", &bob::core::OutputStream::reset<boost::shared_ptr<bob::core::OutputDevice> >, (arg("self"), arg("device")), "Resets the current stream to use a new method for output instead of the currently configured. This version of the API allows you to pass an existing OutputDevice to be used for output data.")
    .def("log", &log_message, (arg("self"), arg("message")), "This method logs an arbitrary message to the current log stream")
    ;

  //binds the standard C++ streams for logging output to python
  scope().attr("debug") = object(ptr(&bob::core::debug));
  scope().attr("info") = object(ptr(&bob::core::info));
  scope().attr("warn") = object(ptr(&bob::core::warn));
  scope().attr("error") = object(ptr(&bob::core::error));

  //a test function
}
