#!/usr/bin/env python
# vim: set fileencoding=utf-8 :
# Andre Anjos <andre.anjos@idiap.ch>
# Wed 25 Aug 2010 12:33:13 CEST 

import os, sys
import unittest
import torch

def compare(v1, v2, width):
  return abs(v1-v2) <= width

class TransformTest(unittest.TestCase):
  """Performs for dct, dct2, fft, fft2 and their inverses"""
  
  def test_dct1D_8(self):
    # size of the data
    N = 8

    # set up simple 1D tensor
    t = torch.core.FloatTensor(N)
    for i in range(N):
      t.set(i, 1.0+i)
    
    # process using DCT
    d = torch.sp.spDCT()
    d.process(t)
    self.assertEqual(d.getNOutputs(), 1)

    # get answer and compare to matlabs dct
    tt = d.getOutput(0)

    # array containing matlab values
    mat = (12.7279,-6.4423,0.,-0.6735,0.,-0.2009,0.,-0.0507)

    for i in range(N):
      self.assertTrue(compare(tt.get(i), mat[i], 1e-3))


  def test_dct2D_2a(self):
    # size of the data
    N = 2 

    # set up simple 2D tensor
    t = torch.core.FloatTensor(N, N)
    t.set(0, 0, 1.0)
    t.set(0, 1, 0.0)
    t.set(1, 0, 0.0)
    t.set(1, 1, 0.0)

    # process using DCT
    d = torch.sp.spDCT()
    d.process(t)
    self.assertEqual(d.getNOutputs(), 1)

    # get answer and compare to matlabs dct2 (warning do not use dct)
    tt = d.getOutput(0)

    # array containing matlab values
    mat = ((0.5, 0.5), (0.5, 0.5))

    for i in range(N):
      for j in range(N):
        self.assertTrue(compare(tt.get(i,j), mat[i][j], 1e-3))


  def test_dct2D_2b(self):
    # size of the data
    N = 2 

    # set up simple 2D tensor
    t = torch.core.FloatTensor(N, N)
    t.set(0, 0, 3.2)
    t.set(0, 1, 4.7)
    t.set(1, 0, 5.4)
    t.set(1, 1, 0.2)

    # process using DCT
    d = torch.sp.spDCT()
    d.process(t)
    self.assertEqual(d.getNOutputs(), 1)

    # get answer and compare to matlabs dct2 (warning do not use dct)
    tt = d.getOutput(0)

    # array containing matlab values
    mat = ((6.75, 1.85), (1.15,-3.35))

    for i in range(N):
      for j in range(N):
        self.assertTrue(compare(tt.get(i,j), mat[i][j], 1e-3))
   
 
  def test_dct2D_4(self):
    # size of the data
    N = 4 

    # set up simple tensor
    t = torch.core.FloatTensor(N, N)

    for i in range(N):
      for j in range(N):
        t.set(i, j, 1.0+i+j)

    # process using DCT
    d = torch.sp.spDCT()
    d.process(t)
    self.assertEqual(d.getNOutputs(), 1)

    # get answer and compare to matlabs dct2 (warning do not use dct)
    tt = d.getOutput(0)

    # array containing matlab values
    #mat = (0.5,0.5,0.5,0.5)
    mat = ((16.0000, -4.4609, 0., -0.3170), (-4.4609, 0., 0., 0.),
           (0., 0., 0., 0.), (-0.3170, 0., 0., 0.))

    for i in range(N):
      for j in range(N):
        self.assertTrue(compare(tt.get(i,j), mat(i,j), 1e-3))


  def test_dct2D_8(self):
    # size of the data
    N = 8

    # set up simple tensor (have to be 2d)
    t = torch.core.FloatTensor(N, N)

    for i in range(N):
      for j in range(N):
        t.set(i, j, 1.0+i+j)

    # process using DCT
    d = torch.sp.spDCT()
    d.process(t)
    self.assertEqual(d.getNOutputs(), 1)

    # get answer and compare to matlabs dct2 (warning do not use dct)
    tt = d.getOutput(0)

    # array containing matlab values
    mat = ((64.0000, -18.2216, 0., -1.9048, 0., -0.5682, 0., -0.1434),
           (-18.2216, 0., 0., 0., 0., 0., 0., 0.),
           (0., 0., 0., 0., 0., 0., 0., 0.),
           (-1.9048, 0., 0., 0., 0., 0., 0., 0.),
           (0., 0., 0., 0., 0., 0., 0., 0.),
           (-0.5682, 0., 0., 0., 0., 0., 0., 0.),
           (0., 0., 0., 0., 0., 0., 0., 0.),
           (-0.1434, 0., 0., 0., 0., 0., 0., 0.))

    for i in range(N):
      for j in range(N):
        self.assertTrue(compare(tt.get(i,j), mat[i][j], 1e-3))


  def test_dct2D_16(self):
    # size of the data
    N = 16

    # set up simple tensor (have to be 2d)
    t = torch.core.FloatTensor(N, N)

    for i in range(N):
      for j in range(N):
        t.set(i, j, 1.0+i+j)

    # process using DCT
    d = torch.sp.spDCT()
    d.process(t)
    self.assertEqual(d.getNOutputs(), 1)

    # get answer and compare to matlabs dct2 (warning do not use dct)
    tt = d.getOutput(0)

    # array containing matlab values
    mat = ((256.0000, -73.2461, 0., -8.0301, 0., -2.8063, 0., -1.3582, 
              0., -0.7507, 0., -0.4286, 0., -0.2242, 0., -0.0700),
           (-73.2461, 0., 0., 0., 0., 0., 0., 0., 
              0., 0., 0., 0., 0., 0., 0., 0.),
           (0., 0., 0., 0., 0., 0., 0., 0., 
              0., 0., 0., 0., 0., 0., 0., 0.),
           (-8.0301, 0., 0., 0., 0., 0., 0., 0., 
              0., 0., 0., 0., 0., 0., 0., 0.),
           (0., 0., 0., 0., 0., 0., 0., 0., 
              0., 0., 0., 0., 0., 0., 0., 0.),
           (-2.8063, 0., 0., 0., 0., 0., 0., 0., 
              0., 0., 0., 0., 0., 0., 0., 0.),
           (0., 0., 0., 0., 0., 0., 0., 0., 
              0., 0., 0., 0., 0., 0., 0., 0.),
           (-1.3582, 0., 0., 0., 0., 0., 0., 0., 
              0., 0., 0., 0., 0., 0., 0., 0.),
           (0., 0., 0., 0., 0., 0., 0., 0., 
              0., 0., 0., 0., 0., 0., 0., 0.),
           (-0.7507, 0., 0., 0., 0., 0., 0., 0., 
              0., 0., 0., 0., 0., 0., 0., 0.),
           (0., 0., 0., 0., 0., 0., 0., 0., 
              0., 0., 0., 0., 0., 0., 0., 0.),
           (-0.4286, 0., 0., 0., 0., 0., 0., 0., 
              0., 0., 0., 0., 0., 0., 0., 0.),
           (0., 0., 0., 0., 0., 0., 0., 0., 
              0., 0., 0., 0., 0., 0., 0., 0.),
           (-0.2242, 0., 0., 0., 0., 0., 0., 0., 
              0., 0., 0., 0., 0., 0., 0., 0.),
           (0., 0., 0., 0., 0., 0., 0., 0., 
              0., 0., 0., 0., 0., 0., 0., 0.),
           (-0.0700, 0., 0., 0., 0., 0., 0., 0., 
              0., 0., 0., 0., 0., 0., 0., 0.))

    for i in range(N):
      for j in range(N):
        self.assertTrue(compare(tt.get(i,j), mat[i][j], 1e-3))


if __name__ == '__main__':
  sys.argv.append('-v')
  if os.environ.has_key('TORCH_PROFILE') and \
      os.environ['TORCH_PROFILE'] and \
      hasattr(torch.core, 'ProfilerStart'):
    torch.core.ProfilerStart(os.environ['TORCH_PROFILE'])
  os.chdir(os.path.realpath(os.path.dirname(sys.argv[0])))
  unittest.main()
  if os.environ.has_key('TORCH_PROFILE') and \
      os.environ['TORCH_PROFILE'] and \
      hasattr(torch.core, 'ProfilerStop'):
    torch.core.ProfilerStop()
