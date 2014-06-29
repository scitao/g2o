// g2o - General Graph Optimization
// Copyright (C) 2014 R. Kuemmerle, G. Grisetti, W. Burgard
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "gtest/gtest.h"

#include "g2o/core/jacobian_workspace.h"
#include "g2o/types/slam3d/edge_se3.h"
#include "g2o/types/slam3d/edge_pointxyz.h"

using namespace std;
using namespace g2o;

static Eigen::Isometry3d randomIsometry3d()
{
  Eigen::Vector3d rotAxisAngle = Vector3d::Random();
  rotAxisAngle += Vector3d::Random();
  Eigen::AngleAxisd rotation(rotAxisAngle.norm(), rotAxisAngle.normalized());
  Eigen::Isometry3d result = (Eigen::Isometry3d)rotation.toRotationMatrix();
  result.translation() = Vector3d::Random();
  return result;
}

template <typename EdgeType>
void evaluateJacobian(EdgeType& e, JacobianWorkspace& jacobianWorkspace, JacobianWorkspace& numericJacobianWorkspace)
{
    // calling the analytic Jacobian but writing to the numeric workspace
    e.BaseBinaryEdge<EdgeType::Dimension, typename EdgeType::Measurement,
      typename EdgeType::VertexXiType, typename EdgeType::VertexXjType>::linearizeOplus(numericJacobianWorkspace);
    // copy result into analytic workspace
    jacobianWorkspace = numericJacobianWorkspace;

    // compute the numeric Jacobian into the numericJacobianWorkspace workspace as setup by the previous call
    e.BaseBinaryEdge<EdgeType::Dimension, typename EdgeType::Measurement,
      typename EdgeType::VertexXiType, typename EdgeType::VertexXjType>::linearizeOplus();

    // compare the two Jacobians
    for (int i = 0; i < 2; ++i) {
      double* n = numericJacobianWorkspace.workspaceForVertex(i);
      double* a = jacobianWorkspace.workspaceForVertex(i);
      int numElems = EdgeType::Dimension;
      if (i == 0)
        numElems *= EdgeType::VertexXiType::Dimension;
      else
        numElems *= EdgeType::VertexXjType::Dimension;
      for (int j = 0; j < numElems; ++j) {
        EXPECT_NEAR(n[j], a[j], 1e-6);
      }
    }
}

TEST(EdgeSE3, Jacobian)
{
  VertexSE3 v1;
  v1.setId(0); 

  VertexSE3 v2;
  v2.setId(1); 

  EdgeSE3 e;
  e.setVertex(0, &v1);
  e.setVertex(1, &v2);
  e.setInformation(EdgeSE3::InformationType::Identity());

  JacobianWorkspace jacobianWorkspace;
  JacobianWorkspace numericJacobianWorkspace;
  numericJacobianWorkspace.updateSize(&e);
  numericJacobianWorkspace.allocate();

  for (int k = 0; k < 10000; ++k) {
    v1.setEstimate(randomIsometry3d());
    v2.setEstimate(randomIsometry3d());
    e.setMeasurement(randomIsometry3d());

    evaluateJacobian(e, jacobianWorkspace, numericJacobianWorkspace);
  }
}

TEST(EdgePointXYZ, Jacobian)
{
  VertexPointXYZ v1;
  v1.setId(0); 

  VertexPointXYZ v2;
  v2.setId(1); 

  EdgePointXYZ e;
  e.setVertex(0, &v1);
  e.setVertex(1, &v2);
  e.setInformation(EdgePointXYZ::InformationType::Identity());

  JacobianWorkspace jacobianWorkspace;
  JacobianWorkspace numericJacobianWorkspace;
  numericJacobianWorkspace.updateSize(&e);
  numericJacobianWorkspace.allocate();

  for (int k = 0; k < 10000; ++k) {
    v1.setEstimate(Eigen::Vector3d::Random());
    v2.setEstimate(Eigen::Vector3d::Random());
    e.setMeasurement(Eigen::Vector3d::Random());

    evaluateJacobian(e, jacobianWorkspace, numericJacobianWorkspace);
  }
}
