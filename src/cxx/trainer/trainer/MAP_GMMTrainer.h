/**
 * @file cxx/trainer/trainer/MAP_GMMTrainer.h
 * @date Tue May 10 11:35:58 2011 +0200
 * @author Francois Moulin <Francois.Moulin@idiap.ch>
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
/// @file MAP_GMMTrainer.h
/// @author <a href="mailto:Roy.Wallace@idiap.ch">Roy Wallace</a> 
/// @author <a href="mailto:Laurent.El-Shafey@idiap.ch">Laurent El Shafey</a> 
/// @brief This class implements the maximum a posteriori M-step of the expectation-maximisation algorithm for a GMM Machine. The prior parameters are encoded in the form of a GMM (e.g. a universal background model). The EM algorithm thus performs GMM adaptation.
/// @details See Section 3.4 of Reynolds et al., "Speaker Verification Using Adapted Gaussian Mixture Models", Digital Signal Processing, 2000. We use a "single adaptation coefficient", alpha_i, and thus a single relevance factor, r.

#ifndef _MAP_GMMTRAINER_H
#define _MAP_GMMTRAINER_H

#include "GMMTrainer.h"
#include <limits>
#include <core/Exception.h>

namespace bob {
namespace trainer {

/// @brief This class implements the maximum a posteriori M-step of the expectation-maximisation algorithm for a GMM Machine. The prior parameters are encoded in the form of a GMM (e.g. a universal background model). The EM algorithm thus performs GMM adaptation.
/// @details See Section 3.4 of Reynolds et al., "Speaker Verification Using Adapted Gaussian Mixture Models", Digital Signal Processing, 2000. We use a "single adaptation coefficient", alpha_i, and thus a single relevance factor, r.
class MAP_GMMTrainer : public GMMTrainer {
  public:

    /// Default constructor
    MAP_GMMTrainer(double relevance_factor = 0, bool update_means = true, bool update_variances = false, 
      bool update_weights = false, double mean_var_update_responsibilities_threshold = std::numeric_limits<double>::epsilon());

    /// Destructor
    virtual ~MAP_GMMTrainer();

    /// Initialization
    virtual void initialization(bob::machine::GMMMachine& gmm, const bob::io::Arrayset& data);

    /// Set the GMM to use as a prior for MAP adaptation.
    /// Generally, this is a "universal background model" (UBM),
    /// also referred to as a "world model".
    bool setPriorGMM(bob::machine::GMMMachine *prior_gmm);

    /// Performs a maximum a posteriori (MAP) update of the GMM parameters
    /// using the accumulated statistics in m_ss and the 
    /// parameters of the prior model
    /// Implements EMTrainer::mStep()
    void mStep(bob::machine::GMMMachine& gmm, const bob::io::Arrayset& data);

    /// Use a bob3-like adaptation rule rather than Reynolds'one
    /// In this case, alpha is a configuration variable rather than a function of the zeroth 
    /// order statistics and a relevance factor (should be in range [0,1])
    void setT3MAP(const double alpha) { m_T3_adaptation = true; m_T3_alpha = alpha; }
    void unsetT3MAP() { m_T3_adaptation = false; }
    
  protected:

    /// The relevance factor for MAP adaptation, r (see Reynolds et al., \"Speaker Verification Using Adapted Gaussian Mixture Models\", Digital Signal Processing, 2000).
    double relevance_factor;
    
    /// The GMM to use as a prior for MAP adaptation.
    /// Generally, this is a "universal background model" (UBM),
    /// also referred to as a "world model"
    bob::machine::GMMMachine *m_prior_gmm;

    /// The alpha for the bob3-like adaptation
    double m_T3_alpha;
    /// Whether bob3-like adaptation should be used or not
    bool m_T3_adaptation;

  private:

    /// cache to avoid re-allocation
    mutable blitz::Array<double,1> m_cache_alpha;
    mutable blitz::Array<double,1> m_cache_ml_weights;
    mutable blitz::Array<double,1> m_cache_prior_weights;
    mutable blitz::Array<double,1> m_cache_new_weights;
    mutable blitz::Array<double,2> m_cache_ml_means;
    mutable blitz::Array<double,2> m_cache_prior_means;
    mutable blitz::Array<double,2> m_cache_new_means;
    mutable blitz::Array<double,2> m_cache_prior_variances;
    mutable blitz::Array<double,2> m_cache_Exx;
    mutable blitz::Array<double,2> m_cache_means;
    mutable blitz::Array<double,2> m_cache_new_variances;

};

}}

#endif
