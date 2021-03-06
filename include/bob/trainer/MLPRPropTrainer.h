/**
 * @file bob/trainer/MLPRPropTrainer.h
 * @date Wed Jul 6 17:32:35 2011 +0200
 * @author Andre Anjos <andre.anjos@idiap.ch>
 *
 * @brief A MLP trainer based on resilient back-propagation: A Direct Adaptive
 * Method for Faster Backpropagation Learning: The RPROP Algorithm, by Martin
 * Riedmiller and Heinrich Braun on IEEE International Conference on Neural
 * Networks, pp. 586--591, 1993.
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

#ifndef BOB_TRAINER_MLPRPROPTRAINER_H 
#define BOB_TRAINER_MLPRPROPTRAINER_H

#include <vector>
#include <boost/function.hpp>

#include "bob/machine/MLP.h"

namespace bob { namespace trainer {

  /**
   * Sets an MLP to perform discrimination based on RProp: A Direct Adaptive
   * Method for Faster Backpropagation Learning: The RPROP Algorithm, by Martin
   * Riedmiller and Heinrich Braun on IEEE International Conference on Neural
   * Networks, pp. 586--591, 1993.
   */
  class MLPRPropTrainer {

    public: //api

      /**
       * Initializes a new MLPRPropTrainer trainer according to a given machine
       * settings and a training batch size. 
       *
       * Good values for batch sizes are tens of samples. RProp is a "batch"
       * training algorithm. Do not try to set batch_size to a too-low value.
       */
      MLPRPropTrainer(const bob::machine::MLP& machine, size_t batch_size);

      /**
       * Destructor virtualisation
       */
      virtual ~MLPRPropTrainer();
      
      /**
       * Copy construction.
       */
      MLPRPropTrainer(const MLPRPropTrainer& other);

      /**
       * Copy operator
       */
      MLPRPropTrainer& operator=(const MLPRPropTrainer& other);

      /**
       * Re-initializes the whole training apparatus to start training a new
       * machine. This will effectively reset all Delta matrices to their
       * intial values and set the previous derivatives to zero as described on
       * the section II.C of the RProp paper.
       */
      void reset();

      /**
       * Gets the batch size
       */
      size_t getBatchSize() const { return m_target.extent(0); }

      /**
       * Sets the batch size
       */
      void setBatchSize(size_t batch_size);

      /**
       * Gets the current settings for bias training (defaults to true)
       */
      inline bool getTrainBiases() const { return m_train_bias; }

      /**
       * Sets the bias training option
       */
      inline void setTrainBiases(bool v) { m_train_bias = v; }

      /**
       * Checks if a given machine is compatible with my inner settings.
       */
      bool isCompatible(const bob::machine::MLP& machine) const;

      /**
       * Trains the MLP to perform discrimination. The training is executed
       * outside the machine context, but uses all the current machine layout.
       * The given machine is updated with new weights and biases on the end of
       * the training that is performed a single time. Iterate as much as you
       * want to refine the training.
       *
       * The machine given as input is checked for compatibility with the
       * current initialized settings. If the two are not compatible, an
       * exception is thrown.
       *
       * Note: In RProp, training is done in batches. The number of rows in the
       * input (and target) determines the batch size. If the batch size
       * currently set is incompatible with the given data an exception is
       * raised.
       *       
       * Note2: The machine is not initialized randomly at each train() call.
       * It is your task to call MLP::randomize() once on the machine you
       * want to train and then call train() as many times as you think are
       * necessary. This design allows for a training criteria to be encoded
       * outside the scope of this trainer and to this type to focus only on
       input, target applying the training when requested to.
       */
      void train(bob::machine::MLP& machine, 
          const blitz::Array<double,2>& input,
          const blitz::Array<double,2>& target);

      /**
       * This is a version of the train() method above, which does no
       * compatibility check on the input machine.
       */
      void train_(bob::machine::MLP& machine, 
          const blitz::Array<double,2>& input,
          const blitz::Array<double,2>& target);

    private: //useful methods

      /**
       * Forward step -- this is a second implementation of that used on the
       * MLP itself to allow access to some internal buffers. In our current
       * setup, we keep the "m_output"'s of every individual layer separately
       * as we are going to need them for the weight update.
       *
       * Another factor is the normalization normally applied at MLPs. We
       * ignore that here as the DataShuffler should be capable of handling
       * this in a more efficient way. You should make sure that the final MLP
       * does have the standard normalization settings applied if it was set to
       * automatically apply the standard normalization before giving me the
       * data.
       */
      void forward_step();

      /**
       * Backward step -- back-propagates the calculated error up to each
       * neuron on the first layer. This is explained on Bishop's formula 5.55
       * and 5.56, at page 244 (see also figure 5.7 for a graphical
       * representation).
       */
      void backward_step();

      /**
       * Weight update -- calculates the weight-update using derivatives as
       * explained in Bishop's formula 5.53, page 243.
       *
       * Note: For RProp, specifically, we only care about the derivative's
       * sign, current and the previous. This is the place where standard
       * backprop and rprop diverge.
       *
       * For extra insight, double-check the Technical Report entitled "Rprop -
       * Description and Implementation Details" by Martin Riedmiller, 1994.
       * Just browse the internet for it. Keep it under your pillow ;-)
       */
      void rprop_weight_update();

    private: //representation

      bool m_train_bias; ///< shall we be training biases? (default: true)
      size_t m_H; ///< number of hidden layers on the target machine

      std::vector<blitz::Array<double,2> > m_weight_ref; ///< machine weights
      std::vector<blitz::Array<double,1> > m_bias_ref; ///< machine biases

      std::vector<blitz::Array<double,2> > m_delta; ///< weight deltas
      std::vector<blitz::Array<double,1> > m_delta_bias; ///< bias deltas

      std::vector<blitz::Array<double,2> > m_deriv; ///< weight derivatives
      std::vector<blitz::Array<double,1> > m_deriv_bias; ///< bias derivatives

      std::vector<blitz::Array<double,2> > m_prev_deriv; ///< prev.weight deriv.
      std::vector<blitz::Array<double,1> > m_prev_deriv_bias; ///< pr.bias der.
  
      bob::machine::MLP::actfun_t m_actfun; ///< activation function
      bob::machine::MLP::actfun_t m_bwdfun; ///< (back) activation function
  
      /// buffers that are dependent on the batch_size
      blitz::Array<double,2> m_target; ///< target vectors
      std::vector<blitz::Array<double,2> > m_error; ///< error (+deltas)
      std::vector<blitz::Array<double,2> > m_output; ///< layer output
  };

} }

#endif /* BOB_TRAINER_MLPRPROPTRAINER_H */
