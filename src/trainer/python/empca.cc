/**
 * @file trainer/python/empca.cc
 * @date Tue Oct 11 12:32:10 2011 +0200
 * @author Laurent El Shafey <Laurent.El-Shafey@idiap.ch>
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
#include <boost/python.hpp>
#include "bob/machine/LinearMachine.h"
#include "bob/trainer/EMPCATrainer.h"

using namespace boost::python;

object ppca_train(bob::trainer::EMPCATrainer& t, const blitz::Array<double,2>& data) {
  bob::machine::LinearMachine m;
  t.train(m, data);
  return object(m);
}

void bind_trainer_empca() {

  typedef bob::trainer::EMTrainer<bob::machine::LinearMachine, blitz::Array<double,2> > EMTrainerLinearBase; 

  class_<EMTrainerLinearBase, boost::noncopyable>("EMTrainerLinear", "The base python class for all EM-based trainers.", no_init)
    .add_property("convergence_threshold", &EMTrainerLinearBase::getConvergenceThreshold, &EMTrainerLinearBase::setConvergenceThreshold, "Convergence threshold")
    .add_property("max_iterations", &EMTrainerLinearBase::getMaxIterations, &EMTrainerLinearBase::setMaxIterations, "Max iterations")
    .add_property("compute_likelihood_variable", &EMTrainerLinearBase::getComputeLikelihood, &EMTrainerLinearBase::setComputeLikelihood, "Indicates whether the log likelihood should be computed during EM or not")
    .def("train", &EMTrainerLinearBase::train, (arg("machine"), arg("data")), "Trains a machine using data")
    .def("initialization", &EMTrainerLinearBase::initialization, (arg("machine"), arg("data")), "This method is called before the EM algorithm")
    .def("finalization", &EMTrainerLinearBase::finalization, (arg("machine"), arg("data")), "This method is called at the end of the EM algorithm")
    .def("e_step", &EMTrainerLinearBase::eStep, (arg("machine"), arg("data")),
       "Updates the hidden variable distribution (or the sufficient statistics) given the Machine parameters. ")
    .def("m_step", &EMTrainerLinearBase::mStep, (arg("machine"), arg("data")), "Updates the Machine parameters given the hidden variable distribution (or the sufficient statistics)")
    .def("compute_likelihood", &EMTrainerLinearBase::computeLikelihood, (arg("machine")), "Computes the current log likelihood given the hidden variable distribution (or the sufficient statistics)")
  ;

  class_<bob::trainer::EMPCATrainer, boost::noncopyable, bases<EMTrainerLinearBase> >("EMPCATrainer",
      "This class implements the EM algorithm for a Linear Machine (Probabilistic PCA).\n"
      "See Section 12.2 of Bishop, \"Pattern recognition and machine learning\", 2006", init<int,optional<double,double,bool> >((arg("dimensionality"), arg("convergence_threshold"), arg("max_iterations"), arg("compute_likelihood"))))
    .def("train", &ppca_train, (arg("self"), arg("data")), "Trains and returns a Linear machine using the provided data")
    .add_property("seed", &bob::trainer::EMPCATrainer::getSeed, &bob::trainer::EMPCATrainer::setSeed, "The seed for the random initialization of W and sigma2")
    .add_property("sigma2", &bob::trainer::EMPCATrainer::getSigma2, &bob::trainer::EMPCATrainer::setSigma2, "The noise sigma2 of the probabilistic model")
  ;

}
