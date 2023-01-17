/***************************************************************************
* Date: 14/05/2022
* Author: Reuben Lim Yaw Hui
* Desc: To implement comprehensive channel model for UAV communications
***************************************************************************/

#ifndef __INET_LOGNORMALRICIANFADING_H
#define __INET_LOGNORMALRICIANFADING_H

#include "inet/physicallayer/wireless/common/pathloss/FreeSpacePathLoss.h"
#include <cmath>

namespace inet {

namespace physicallayer {

/**
 * This class implements the composite fading model, with lognormal shadowing and Rician multipath fading.
 * Probability of line-of-sight implemented
 * @author Reuben Lim
 */
class INET_API LognormalRicianFading : public FreeSpacePathLoss
{
  protected:
    double k;
    double sigma;
    double a;
    double b; 
    double c;
    double alpha_min;
    double alpha_max;
    double sigma_a;
    double sigma_b;
    double K_min;
    double K_max;

  protected:
    virtual void initialize(int stage) override;

  public:
    LognormalRicianFading();
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual double computePathLoss(mps propagationSpeed, Hz frequency, m distance) const override;
    virtual double computeCompositePathLoss(mps propagationSpeed, Hz frequency, m distance, const ITransmission *transmission, const IArrival *arrival) const override;
    double theta_d(m distance, const Coord& receiver, const Coord& transmitter) const;
    double PLoS(const Coord& receiver, const Coord& transmitter) const;
    double qfunc(double x) const;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_RICIANFADING_H

