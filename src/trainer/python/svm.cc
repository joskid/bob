/**
 * @file trainer/python/svm.cc
 * @date Sun  4 Mar 20:06:49 2012 CET
 * @author Andre Anjos <andre.anjos@idiap.ch>
 *
 * @brief Python bindings to libsvm (training bits)
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

#include "bob/core/python/ndarray.h"
#include <boost/python/stl_iterator.hpp>
#include "bob/trainer/SVMTrainer.h"

using namespace boost::python;
namespace tp = bob::python;
namespace ca = bob::core::array;
namespace mach = bob::machine;
namespace train = bob::trainer;

static boost::shared_ptr<mach::SupportVector> train1 
(const train::SVMTrainer& trainer, object data) {
  stl_input_iterator<blitz::Array<double,2> > dbegin(data), dend;
  std::vector<blitz::Array<double,2> > vdata(dbegin, dend);
  return trainer.train(vdata);
}

static boost::shared_ptr<mach::SupportVector> train2
(const train::SVMTrainer& trainer, object data, tp::const_ndarray sub,
 tp::const_ndarray div) {
  stl_input_iterator<blitz::Array<double,2> > dbegin(data), dend;
  std::vector<blitz::Array<double,2> > vdata(dbegin, dend);
  return trainer.train(vdata, sub.bz<double,1>(), div.bz<double,1>());
}

void bind_trainer_svm() {
  class_<train::SVMTrainer, boost::shared_ptr<train::SVMTrainer> >("SVMTrainer", "This class emulates the behavior of the command line utility called svm-train, from libsvm. These bindings do not support:\n\n * Precomputed Kernels\n * Regression Problems\n * Different weights for every label (-wi option in svm-train)\n\nFell free to implement those and remove these remarks.", no_init)
    .def(init<optional<bob::machine::SupportVector::svm_t, bob::machine::SupportVector::kernel_t, int, double, double, double, double, double, double, double, bool, bool> >(
          (arg("svm_type")=bob::machine::SupportVector::C_SVC,
           arg("kernel_type")=bob::machine::SupportVector::RBF,
           arg("degree")=3, //for poly
           arg("gamma")=0., //for poly/rbf/sigmoid
           arg("coef0")=0., //for poly/sigmoid
           arg("cache_size")=100, //in MB
           arg("eps")=1.e-3, //stopping criteria epsilon
           arg("cost")=1., //for C_SVC, EPSILON_SVR and NU_SVR
           arg("nu")=0.5, //for NU_SVC, ONE_CLASS and NU_SVR
           arg("p")=0.1, //for EPSILON_SVR, this is the "epsilon" value there
           arg("shrinking")=true, //use the shrinking heuristics
           arg("probability")=false //do probability estimates
          ), "Builds a new trainer setting the default parameters as defined in the command line application svm-train."
          )
        )
    .add_property("svm_type", &train::SVMTrainer::getSvmType, &train::SVMTrainer::setSvmType, "Type of SVM to train")
    .add_property("kernel_type", &train::SVMTrainer::getKernelType, &train::SVMTrainer::setKernelType, "SVM kernel type to use")
    .add_property("degree", &train::SVMTrainer::getDegree, &train::SVMTrainer::setDegree, "Polynomial degree")
    .add_property("gamma", &train::SVMTrainer::getGamma, &train::SVMTrainer::setGamma, "Gamma parameter for poly/rbf/sigmoid")
    .add_property("coef0", &train::SVMTrainer::getCoef0, &train::SVMTrainer::setCoef0, "Coefficient 0 for poly/sigmoid")
    .add_property("cache_size", &train::SVMTrainer::getCacheSizeInMB, &train::SVMTrainer::setCacheSizeInMb, "libsvm cache size in Mb")
    .add_property("eps", &train::SVMTrainer::getStopEpsilon, &train::SVMTrainer::setStopEpsilon, "The epsilon used for stop training")
    .add_property("cost", &train::SVMTrainer::getCost, &train::SVMTrainer::setCost, "The cost for C_SVC, EPSILON_SVR and NU_SVR")
    .add_property("nu", &train::SVMTrainer::getNu, &train::SVMTrainer::setNu, "for NU_SVC, ONE_CLASS and NU_SVR")
    .add_property("p", &train::SVMTrainer::getLossEpsilonSVR, &train::SVMTrainer::setLossEpsilonSVR, "for EPSILON_SVR, this is the 'epsilon' value on the equation")
    .add_property("shrinking", &train::SVMTrainer::getUseShrinking, &train::SVMTrainer::setUseShrinking, "use the shrinking heuristics")
    .add_property("probability", &train::SVMTrainer::getProbabilityEstimates, &train::SVMTrainer::setProbabilityEstimates, "do probability estimates")
    .def("train", &train1, (arg("self"), arg("data")), "Trains a new machine for multi-class classification. If the number of classes in data is 2, then the assigned labels will be -1 and +1. If the number of classes is greater than 2, labels are picked starting from 1 (i.e., 1, 2, 3, 4, etc.). If what you want is regression, the size of the input data array should be 1.")
    .def("train", &train2, (arg("self"), arg("data"), arg("subtract"), arg("divide")), "This version accepts scaling parameters that will be applied column-wise to the input data.")
    ;
}
