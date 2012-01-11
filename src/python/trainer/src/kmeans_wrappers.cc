/**
 * @file python/trainer/src/kmeans_wrappers.cc
 * @date Thu Jun 9 18:12:33 2011 +0200
 * @author Laurent El Shafey <Laurent.El-Shafey@idiap.ch>
 *
 * Copyright (C) 2011 Idiap Reasearch Institute, Martigny, Switzerland
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
#include <boost/python.hpp>
#include "io/Arrayset.h"
#include "trainer/KMeansTrainer.h"

using namespace boost::python;
namespace train = bob::trainer;
namespace mach = bob::machine;
namespace io = bob::io;


class EMTrainerKMeansWrapper: public train::EMTrainer<mach::KMeansMachine, io::Arrayset>, 
                              public wrapper<train::EMTrainer<mach::KMeansMachine, io::Arrayset> > 
{
public:
  EMTrainerKMeansWrapper(double convergence_threshold = 0.001, int max_iterations = 10):
    train::EMTrainer<mach::KMeansMachine, io::Arrayset >(convergence_threshold, max_iterations) {}

  virtual ~EMTrainerKMeansWrapper() {}
 
  virtual void initialization(mach::KMeansMachine& machine, const io::Arrayset& data) {
    this->get_override("initialization")(machine, data);
  }
  
  virtual double eStep(mach::KMeansMachine& machine, const io::Arrayset& data) {
    return this->get_override("eStep")(machine, data);
  }
  
  virtual void mStep(mach::KMeansMachine& machine, const io::Arrayset& data) {
    this->get_override("mStep")(machine, data);
  }

  virtual void train(mach::KMeansMachine& machine, const io::Arrayset& data) {
    if (override python_train = this->get_override("train")) 
      python_train(machine, data);
    else
      train::EMTrainer<mach::KMeansMachine, io::Arrayset>::train(machine, data);
  }

  virtual void d_train(mach::KMeansMachine& machine, const io::Arrayset& data) {
    train::EMTrainer<mach::KMeansMachine, io::Arrayset>::train(machine, data);
  }

};

class KMeansTrainerWrapper: public train::KMeansTrainer,
                            public wrapper<train::KMeansTrainer>
{
public:
  KMeansTrainerWrapper(double convergence_threshold = 0.001, int max_iterations = 10):
    train::KMeansTrainer(convergence_threshold, max_iterations) {}

  virtual ~KMeansTrainerWrapper() {}
 
  void initialization(mach::KMeansMachine& machine, const io::Arrayset& data) {
    if (override python_initialization = this->get_override("initialization"))
      python_initialization(machine, data);
    else
      train::KMeansTrainer::initialization(machine, data);
  }
  
  void d_initialization(mach::KMeansMachine& machine, const io::Arrayset& data) {
    train::KMeansTrainer::initialization(machine, data);
  }
  
  double eStep(mach::KMeansMachine& machine, const io::Arrayset& data) {
    if (override python_eStep = this->get_override("eStep")) return python_eStep(machine, data);
    return train::KMeansTrainer::eStep(machine, data);
  }
  
  double d_eStep(mach::KMeansMachine& machine, const io::Arrayset& data) {
    return train::KMeansTrainer::eStep(machine, data);
  }
  
  void mStep(mach::KMeansMachine& machine, const io::Arrayset& data) {
    if (override python_mStep = this->get_override("mStep")) 
      python_mStep(machine, data);
    else
      train::KMeansTrainer::mStep(machine, data);
  }

  void d_mStep(mach::KMeansMachine& machine, const io::Arrayset& data) {
    train::KMeansTrainer::mStep(machine, data);
  }

  void train(mach::KMeansMachine& machine, const io::Arrayset& data) {
    if (override python_train = this->get_override("train")) 
      python_train(machine, data);
    else
      train::KMeansTrainer::train(machine, data);
  }

  void d_train(mach::KMeansMachine& machine, const io::Arrayset& data) {
    train::KMeansTrainer::train(machine, data);
  }

};


void bind_trainer_kmeans_wrappers() {

  typedef train::EMTrainer<mach::KMeansMachine, io::Arrayset> EMTrainerKMeansBase; 

  class_<EMTrainerKMeansWrapper, boost::noncopyable>("EMTrainerKMeans", no_init)
    .def(init<optional<double, int> >((arg("convergence_threshold")=0.001, arg("max_iterations")=10)))
    .add_property("convergenceThreshold", &EMTrainerKMeansBase::getConvergenceThreshold, &EMTrainerKMeansBase::setConvergenceThreshold, "Convergence threshold")
    .add_property("maxIterations", &EMTrainerKMeansBase::getMaxIterations, &EMTrainerKMeansBase::setMaxIterations, "Max iterations")
    .def("train", &EMTrainerKMeansBase::train, &EMTrainerKMeansWrapper::d_train, (arg("machine"), arg("data")), "Train a machine using data")
    .def("initialization", pure_virtual(&EMTrainerKMeansBase::initialization), (arg("machine"), arg("data")), "This method is called before the EM algorithm")
    .def("eStep", pure_virtual(&EMTrainerKMeansBase::eStep), (arg("machine"), arg("data")),
       "Update the hidden variable distribution (or the sufficient statistics) given the Machine parameters. "
       "Also, calculate the average output of the Machine given these parameters.\n"
       "Return the average output of the Machine across the dataset. "
       "The EM algorithm will terminate once the change in average_output "
       "is less than the convergence_threshold.")
    .def("mStep", pure_virtual(&EMTrainerKMeansBase::mStep), (arg("machine"), arg("data")), "Update the Machine parameters given the hidden variable distribution (or the sufficient statistics)")
  ;

  class_<KMeansTrainerWrapper, boost::noncopyable>("KMeansTrainer",
      "Trains a KMeans machine.\n"
      "This class implements the expectation-maximisation algorithm for a k-means machine.\n"
      "See Section 9.1 of Bishop, \"Pattern recognition and machine learning\", 2006\n"
      "It uses a random initialisation of the means followed by the expectation-maximization algorithm"
      )
    .def(init<optional<double,int> >((arg("convergence_threshold")=0.001, arg("max_iterations")=10)))
    .add_property("convergenceThreshold", &KMeansTrainerWrapper::getConvergenceThreshold, &KMeansTrainerWrapper::setConvergenceThreshold, "Convergence threshold")
    .add_property("maxIterations", &KMeansTrainerWrapper::getMaxIterations, &KMeansTrainerWrapper::setMaxIterations, "Max iterations")
    .add_property("seed", &KMeansTrainerWrapper::getSeed, &KMeansTrainerWrapper::setSeed, "Seed used to genrated pseudo-random numbers")
    .def("train", &train::KMeansTrainer::train, &KMeansTrainerWrapper::d_train, (arg("machine"), arg("data")), "Train a machine using some data")
    .def("initialization", &train::KMeansTrainer::initialization, &KMeansTrainerWrapper::d_initialization, (arg("machine"), arg("data")), "This method is called before the EM algorithm")
    .def("eStep", &train::KMeansTrainer::eStep, &KMeansTrainerWrapper::d_eStep, (arg("machine"), arg("data")), "Update the Machine parameters given the hidden variable distribution (or the sufficient statistics)")
    .def("mStep", &train::KMeansTrainer::mStep, &KMeansTrainerWrapper::d_mStep, (arg("machine"), arg("data")), "M-step of the EM-algorithm.")
  ;


  }
