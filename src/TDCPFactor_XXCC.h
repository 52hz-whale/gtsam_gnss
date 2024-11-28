/**
 * @file TDCPFactor_XXCC.h
 * @brief Time-difference Carrier Phase (TDCP) factor with 3D positions (X) and receiver clocks (C)
 * @author Taro Suzuki
 */

#pragma once
#include <gtsam/nonlinear/NonlinearFactor.h>

namespace gtsam_gnss {

/**
 * @brief TDCP factor: Estimate 3D position change and receiver clock change
 */
class TDCPFactor_XXCC : public gtsam::NoiseModelFactorN<gtsam::Vector, gtsam::Vector, gtsam::Vector, gtsam::Vector> {

private:
  gtsam::Vector losvec_;
  double tdcp_;
  gtsam::Vector inix1_;
  gtsam::Vector inix2_;
  typedef gtsam::NoiseModelFactorN<gtsam::Vector, gtsam::Vector, gtsam::Vector, gtsam::Vector> Base;

public:
  /**
   * @brief Constructor
   * @param keyX1   3D position (X) key at time t1, X has 3 dimension
   * @param keyX2   3D position (X) key at time t2, X has 3 dimension
   * @param keyC1   Receiver clock (C) key at time t1, C has 7 dimension
   * @param keyC2   Receiver clock (C) key at time t2, C has 7 dimension
   * @param losvec  Line-of-Sight vector (3 dimension)
   * @param tdcp    TDCP measurement (meter) (1 dimension)
   * @param inix1   Initial 3D position at time t1 when calculating residual (3 dimension)
   * @param inix2   Initial 3D position at time t2 when calculating residual (3 dimension)
   * @param model   Gaussian noise model (3 dimension)
   */
  TDCPFactor_XXCC(gtsam::Key keyX1,
                  gtsam::Key keyX2,
                  gtsam::Key keyC1,
                  gtsam::Key keyC2,
                  const gtsam::Vector& losvec,
                  const double& tdcp,
                  const gtsam::Vector& inix1,
                  const gtsam::Vector& inix2,
                  const gtsam::SharedNoiseModel& model)
  : Base(model, keyX1, keyX2, keyC1, keyC2), losvec_(losvec), tdcp_(tdcp), inix1_(inix1), inix2_(inix2) {};

  ~TDCPFactor_XXCC() override {}

  // Error function
  using Base::evaluateError;
  gtsam::Vector evaluateError(const gtsam::Vector& x1,
                              const gtsam::Vector& x2,
                              const gtsam::Vector& c1,
                              const gtsam::Vector& c2,
                              gtsam::OptionalMatrixType Hx1,
                              gtsam::OptionalMatrixType Hx2,
                              gtsam::OptionalMatrixType Hc1,
                              gtsam::OptionalMatrixType Hc2) const override {

    // Compute error
    gtsam::Vector dx = (x2 - inix2_) - (x1 - inix1_);
    gtsam::Vector dc = c2 - c1;

    gtsam::Vector7 hc;
    hc << 1, 0, 0, 0, 0, 0, 0;

    gtsam::Vector1 error;
    error << (losvec_.transpose() * dx + hc.transpose() * dc).value() - tdcp_;

    // Jacobian
    if (Hx1) *Hx1 = (gtsam::Matrix(1, 3) << -losvec_.transpose()).finished();
    if (Hx2) *Hx2 = (gtsam::Matrix(1, 3) << losvec_.transpose()).finished();
    if (Hc1) *Hc1 = (gtsam::Matrix(1, 7) << -hc.transpose()).finished();
    if (Hc2) *Hc2 = (gtsam::Matrix(1, 7) << hc.transpose()).finished();

    return error;
  }

  // Print contents
  void print(const std::string& s = "", const gtsam::KeyFormatter& keyFormatter = gtsam::DefaultKeyFormatter) const override {
    std::cout << (s.empty() ? "" : s + " ") << "TDCPFactor_XXCC" << std::endl;
    std::cout << "  TDCP residual: " << tdcp_ << std::endl;
    Base::print("", keyFormatter);
  }

  // Measurement
  inline const double& measurementIn() const { return tdcp_; }
};

}  // namespace gtsam_gnss
