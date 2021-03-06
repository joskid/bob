#!/usr/bin/env python
# vim: set fileencoding=utf-8 :
# Andre Anjos <andre.anjos@idiap.ch>
# Tue Jul 19 09:47:23 2011 +0200
#
# Copyright (C) 2011-2012 Idiap Research Institute, Martigny, Switzerland
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3 of the License.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

"""Tests for BackProp MLP training.
"""

import os, sys
import math
import unittest
import bob
import numpy

class PythonBackProp:
  """A simplified (and slower) version of BackProp training written in python.
  
  This version of the code is probably easier to understand than the C++
  version. Both algorithms should, essentially, be the same, except for the
  performance for obvious reasons.
  """

  def __init__(self, train_biases=True, learning_rate=0.1, momentum=0.0):
    # Our state
    self.learning_rate = learning_rate
    self.momentum = momentum
    self.DW = None #delta matrixes for weights
    self.DB = None #delta matrixes for biases
    self.PDW = None #previous delta matrices for weights
    self.PDB = None #previous delta matrices for biases
    self.train_biases = train_biases

  def reset(self):
    """Resets our internal state"""

    self.DW = None
    self.DB = None
    self.PDW = None
    self.PDB = None

  def train(self, machine, input, target):

    def logistic(x):
      return 1 / (1 + numpy.exp(-x))

    def logistic_bwd(x):
      return x * (1-x)

    def tanh(x):
      return numpy.tanh(x)

    def tanh_bwd(x):
      return (1 - x**2)

    def linear(x):
      return x

    def linear_bwd(x):
      return 1
    
    W = machine.weights #weights
    B = machine.biases #biases

    if machine.activation == bob.machine.Activation.TANH:
      forward = tanh
      backward = tanh_bwd
    elif machine.activation == bob.machine.Activation.LOG:
      forward = logistic
      backward = logistic_bwd
    elif machine.activation == bob.machine.Activation.LINEAR:
      forward = linear
      backward = linear_bwd
    else:
      raise RuntimeError, "Cannot deal with activation %s" % machine.activation
    
    #simulated bias input...
    BI = [numpy.zeros((input.shape[0],), 'float64') for k in B]
    for k in BI: k.fill(1)

    #state
    if self.DW is None: #first run or just after a reset()
      self.PDW = [numpy.empty_like(k) for k in W]
      for k in self.PDW: k.fill(0)
      self.PDB = [numpy.empty_like(k) for k in B]
      for k in self.PDB: k.fill(0)

    # Instantiate partial outputs and errors
    O = [None for k in B]
    O.insert(0, input) # an extra slot for the input
    E = [None for k in B]

    # Feeds forward
    for k in range(len(W)):
      O[k+1] = numpy.dot(O[k], W[k])
      for sample in range(O[k+1].shape[0]):
        O[k+1][sample,:] += B[k]
      O[k+1] = forward(O[k+1])

    # Feeds backward
    E[-1] = backward(O[-1]) * (target - O[-1]) #last layer
    for k in reversed(range(len(W)-1)): #for all remaining layers
      E[k] = backward(O[k+1]) * numpy.dot(E[k+1], W[k+1].transpose(1,0))

    # Calculates partial derivatives, accumulate
    batch_size = E[-1].shape[0]
    self.DW = [numpy.dot(O[k].transpose(1,0), E[k]) for k in range(len(W))]
    for k in self.DW: k *= (self.learning_rate / batch_size)
    self.DB = [numpy.dot(BI[k], E[k]) for k in range(len(W))]
    for k in self.DB: k *= (self.learning_rate / batch_size)

    # Updates weights and biases
    machine.weights = [W[k] + (((1-self.momentum)*self.DW[k]) + (self.momentum*self.PDW[k])) for k in range(len(W))]
    self.PDW = self.DW

    if self.train_biases:
      machine.biases = [B[k] + (((1-self.momentum)*self.DB[k]) + (self.momentum*self.PDB[k])) for k in range(len(W))]
      self.PDB = self.DB
    else:
      machine.biases = 0

class BackPropTest(unittest.TestCase):
  """Performs various BackProp MLP training tests."""

  def test01_Initialization(self):

    # Initializes an MLPBackPropTrainer and checks all seems consistent
    # with the proposed API.
    machine = bob.machine.MLP((4, 1))
    machine.activation = bob.machine.Activation.LINEAR
    B = 10
    trainer = bob.trainer.MLPBackPropTrainer(machine, B)
    self.assertEqual( trainer.batch_size, B )
    self.assertTrue ( trainer.is_compatible(machine) )
    self.assertTrue ( trainer.train_biases )

    machine = bob.machine.MLP((7, 2))
    self.assertFalse ( trainer.is_compatible(machine) )

    trainer.train_biases = False
    self.assertFalse ( trainer.train_biases )

  def test02_TwoLayersNoBiasControlled(self):

    # Trains a simple network with one single step, verifies
    # the training works as expected by calculating the same
    # as the trainer should do using python.
    machine = bob.machine.MLP((2, 2, 1))
    machine.activation = bob.machine.Activation.LOG
    machine.biases = 0
    w0 = numpy.array([[.23, .1],[-0.79, 0.21]])
    w1 = numpy.array([[-.12], [-0.88]])
    machine.weights = [w0, w1]
    trainer = bob.trainer.MLPBackPropTrainer(machine, 1)
    trainer.train_biases = False
    d0 = numpy.array([[.3, .7]])
    t0 = numpy.array([[.0]])

    # trains in python first
    pytrainer = PythonBackProp(train_biases=trainer.train_biases)
    pymachine = bob.machine.MLP(machine) #a copy
    pytrainer.train(pymachine, d0, t0)

    # trains with our C++ implementation
    trainer.train_(machine, d0, t0)
    self.assertTrue( numpy.array_equal(pymachine.weights[0], machine.weights[0]) )

  def test03_FisherWithOneHiddenLayer(self):

    # Trains a multilayer biased MLP to perform discrimination on the Fisher
    # data set.

    N = 50

    machine = bob.machine.MLP((4, 4, 3))
    machine.activation = bob.machine.Activation.TANH
    machine.randomize()
    trainer = bob.trainer.MLPBackPropTrainer(machine, N)
    trainer.train_biases = True

    # A helper to select and shuffle the data
    targets = [ #we choose the approximate Fisher response!
        numpy.array([+1., -1., -1.]), #setosa
        numpy.array([-1., +1., -1.]), #versicolor
        numpy.array([-1., -1., +1.]), #virginica
        ]
    # Associate the data to targets, by setting the arrayset order explicetly
    data = bob.db.iris.data()
    datalist = [data['setosa'], data['versicolor'], data['virginica']]

    S = bob.trainer.DataShuffler(datalist, targets)

    # trains in python first
    pytrainer = PythonBackProp(train_biases=trainer.train_biases)
    pymachine = bob.machine.MLP(machine) #a copy

    # We now iterate for several steps, look for the convergence
    for k in range(50):
      input, target = S(N)
      pytrainer.train(pymachine, input, target)
      trainer.train_(machine, input, target)
      #print "[Python] |RMSE|@%d:" % k, numpy.linalg.norm(bob.measure.rmse(pymachine(input), target))
      #print "[C++] |RMSE|@%d:" % k, numpy.linalg.norm(bob.measure.rmse(machine(input), target))
      # Note we will face precision problems when comparing to the Pythonic
      # implementation. So, let's not be too demanding here. If all values are
      # approximately equal to 1e-10, we consider this is OK.
      for i, w in enumerate(pymachine.weights):
        self.assertTrue( (abs(w-machine.weights[i]) < 1e-10).all() )
      for i, b in enumerate(pymachine.biases):
        self.assertTrue( (abs(b-machine.biases[i]) < 1e-10).all() )

  def test04_FisherMultiLayer(self):

    # Trains a multilayer biased MLP to perform discrimination on the Fisher
    # data set.

    N = 50

    machine = bob.machine.MLP((4, 3, 3, 1))
    machine.activation = bob.machine.Activation.TANH
    machine.randomize()
    trainer = bob.trainer.MLPBackPropTrainer(machine, N)
    trainer.train_biases = True

    # A helper to select and shuffle the data
    targets = [ #we choose the approximate Fisher response!
        numpy.array([-1.0]), #setosa
        numpy.array([0.5]), #versicolor
        numpy.array([+1.0]), #virginica
        ]
    # Associate the data to targets, by setting the arrayset order explicetly
    data = bob.db.iris.data()
    datalist = [data['setosa'], data['versicolor'], data['virginica']]

    S = bob.trainer.DataShuffler(datalist, targets)

    # trains in python first
    pytrainer = PythonBackProp(train_biases=trainer.train_biases)
    pymachine = bob.machine.MLP(machine) #a copy

    # We now iterate for several steps, look for the convergence
    for k in range(50):
      input, target = S(N)
      pytrainer.train(pymachine, input, target)
      trainer.train_(machine, input, target)
      #print "[Python] MSE:", bob.measure.mse(pymachine(input), target).sqrt()
      #print "[C++] MSE:", bob.measure.mse(machine(input), target).sqrt()
      # Note we will face precision problems when comparing to the Pythonic
      # implementation. So, let's not be too demanding here. If all values are
      # approximately equal to 1e-10, we consider this is OK.
      for i, w in enumerate(pymachine.weights):
        self.assertTrue( (abs(w-machine.weights[i]) < 1e-10).all() )
      for i, b in enumerate(pymachine.biases):
        self.assertTrue( (abs(b-machine.biases[i]) < 1e-10).all() )

  def test05_FisherMultiLayerWithMomentum(self):

    # Trains a multilayer biased MLP to perform discrimination on the Fisher
    # data set.

    N = 50

    machine = bob.machine.MLP((4, 3, 3, 1))
    machine.activation = bob.machine.Activation.TANH
    machine.randomize()
    trainer = bob.trainer.MLPBackPropTrainer(machine, N)
    trainer.train_biases = True
    trainer.momentum = 0.99

    # A helper to select and shuffle the data
    targets = [ #we choose the approximate Fisher response!
        numpy.array([-1.0]), #setosa
        numpy.array([0.5]), #versicolor
        numpy.array([+1.0]), #virginica
        ]
    # Associate the data to targets, by setting the arrayset order explicetly
    data = bob.db.iris.data()
    datalist = [data['setosa'], data['versicolor'], data['virginica']]

    S = bob.trainer.DataShuffler(datalist, targets)

    # trains in python first
    pytrainer = PythonBackProp(train_biases=trainer.train_biases)
    pymachine = bob.machine.MLP(machine) #a copy
    pytrainer.momentum = 0.99

    # We now iterate for several steps, look for the convergence
    for k in range(50):
      input, target = S(N)
      pytrainer.train(pymachine, input, target)
      trainer.train_(machine, input, target)
      #print "[Python] MSE:", bob.measure.mse(pymachine(input), target).sqrt()
      #print "[C++] MSE:", bob.measure.mse(machine(input), target).sqrt()
      # Note we will face precision problems when comparing to the Pythonic
      # implementation. So, let's not be too demanding here. If all values are
      # approximately equal to 1e-10, we consider this is OK.
      for i, w in enumerate(pymachine.weights):
        self.assertTrue( (abs(w-machine.weights[i]) < 1e-10).all() )
      for i, b in enumerate(pymachine.biases):
        self.assertTrue( (abs(b-machine.biases[i]) < 1e-10).all() )
