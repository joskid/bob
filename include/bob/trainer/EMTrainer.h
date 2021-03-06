/**
 * @file bob/trainer/EMTrainer.h
 * @date Tue Jan 18 17:07:26 2011 +0100
 * @author André Anjos <andre.anjos@idiap.ch>
 * @author Laurent El Shafey <Laurent.El-Shafey@idiap.ch>
 *
 * @brief Base class for Expectation-Maximization-like algorithms
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


#ifndef BOB_TRAINER_EMTRAINER_H
#define BOB_TRAINER_EMTRAINER_H

#include "bob/trainer/Trainer.h"

#include <limits>
#include "bob/core/logging.h"


namespace bob { namespace trainer {
  
  /**
    * @brief This class implements the general Expectation-maximization algorithm.
    * @details See Section 9.3 of Bishop, "Pattern recognition and machine learning", 2006
    * Derived classes must implement the initialization(), eStep(), mStep() and finalization() methods.
    */
  template<class T_machine, class T_sampler>
  class EMTrainer: virtual public Trainer<T_machine, T_sampler>
  {
  public:
    virtual ~EMTrainer() {}
   
    /**
      * Assignment operator
      */
    virtual EMTrainer& operator=(const EMTrainer& other)
    {
      if(this != &other)
      {
        m_compute_likelihood = other.m_compute_likelihood;
        m_convergence_threshold = other.m_convergence_threshold;
        m_max_iterations = other.m_max_iterations;
      }
      return *this;
    }

    /**
      * Equal to
      */
    virtual bool operator==(const EMTrainer& b) const {
      return m_compute_likelihood == b.m_compute_likelihood &&
             m_convergence_threshold == b.m_convergence_threshold &&
             m_max_iterations == b.m_max_iterations;
    }

    /**
      * Not equal to
      */
    virtual bool operator!=(const EMTrainer& b) const {
      return !(this->operator==(b));
    }
 
    virtual void train(T_machine& machine, const T_sampler& sampler) 
    {
      bob::core::info << "# EMTrainer:" << std::endl;
      
      /*
      // Check that the machine and dataset have the same feature dimensionality
      if (!checkForDimensionalityMatch()) 
      {
        bob::core::error << "Mismatch in dimensionality of dataset and machine" << endl;
        return false;
      }
      */
      
      // Initialization
      initialization(machine, sampler);
      // Do the Expectation-Maximization algorithm
      double average_output_previous = - std::numeric_limits<double>::max();
      double average_output = - std::numeric_limits<double>::max();
      
      // - eStep
      eStep(machine, sampler);
   
      // - iterates...
      for(size_t iter=0; ; ++iter) {
        
        // - saves average output from last iteration
        average_output_previous = average_output;
       
        // - mStep
        mStep(machine, sampler);
        
        // - eStep
        eStep(machine, sampler);
   
        // - Computes log likelihood if required
        if(m_compute_likelihood) {
          average_output = computeLikelihood(machine);
        
          bob::core::info << "# Iteration " << iter+1 << ": " 
            << average_output_previous << " -> " 
            << average_output << std::endl;
        
          // - Terminates if converged (and likelihood computation is set)
          if(fabs((average_output_previous - average_output)/average_output_previous) <= m_convergence_threshold) {
            bob::core::info << "# EM terminated: likelihood converged" << std::endl;
            break;
          }
        }
        else
          bob::core::info << "# Iteration " << iter+1 << std::endl;
        
        // - Terminates if maximum number of iterations has been reached
        if(m_max_iterations > 0 && iter+1 >= m_max_iterations) {
          bob::core::info << "# EM terminated: maximum number of iterations reached." << std::endl;
          break;
        }
      }

      // Finalization
      finalization(machine, sampler);
    }

    /**
      * This method is called before the EM algorithm 
      */
    virtual void initialization(T_machine& machine, const T_sampler& sampler) = 0;
    
    /**
      * Updates the hidden variable distribution (or the sufficient statistics)
      * given the Machine parameters.
      */
    virtual void eStep(T_machine& machine, const T_sampler& sampler) = 0;
    
    /**
      * Update the Machine parameters given the hidden variable distribution 
      * (or the sufficient statistics)
      */
    virtual void mStep(T_machine& machine, const T_sampler& sampler) = 0;

    /**
      * @return The average output of the Machine across the dataset.
      *     The EM algorithm will terminate once the change in average_output
      *     is less than the convergence_threshold.
      */
    virtual double computeLikelihood(T_machine& machine) = 0;

    /**
      * This method is called after the EM algorithm 
      */
    virtual void finalization(T_machine& machine, const T_sampler& sampler) = 0;

    /**
      * Sets likelihood computation
      */
    void setComputeLikelihood(bool compute) {
      m_compute_likelihood = compute;
    }

    /**
      * Gets convergence threshold
      */
    bool getComputeLikelihood() const {
      return m_compute_likelihood;
    }

    /**
      * Sets convergence threshold
      */
    void setConvergenceThreshold(double threshold) {
      m_convergence_threshold = threshold;
    }

    /**
      * Gets convergence threshold
      */
    double getConvergenceThreshold() const {
      return m_convergence_threshold;
    }

    /**
      * Set max iterations
      */
    void setMaxIterations(size_t max_iterations) {
      m_max_iterations = max_iterations;
    }

    /**
      * Get max iterations
      */
    size_t getMaxIterations() const {
      return m_max_iterations;
    }

  protected:
    bool m_compute_likelihood;
    double m_convergence_threshold;
    size_t m_max_iterations;

    /**
      * Protected constructor to be called in the constructor of derived 
      * classes
      */
    EMTrainer(double convergence_threshold = 0.001, 
        size_t max_iterations = 10, bool compute_likelihood = true):
      m_compute_likelihood(compute_likelihood), 
      m_convergence_threshold(convergence_threshold), 
      m_max_iterations(max_iterations) 
    {
    }
  };

}}

#endif // BOB_TRAINER_EMTRAINER_H
