//
// Copyright (C) 2005 Emin Ilker Cetinbas
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//
// Author: Reuben Lim (Reuben.Lim"monash.edu)
// Modified from: Linear Mobility
//

#ifndef __INET_LINEARGAUSSMARKOVMOBILITY_H
#define __INET_LINEARGAUSSMARKOVMOBILITY_H

#include "inet/mobility/base/MovingMobilityBase.h"
#include <filesystem>

namespace inet {

/**
 * @brief Linear movement model. See NED file for more info.
 *
 * @ingroup mobility
 * @author Emin Ilker Cetinbas
 */
class INET_API LinearGaussMarkovMobility : public MovingMobilityBase
{
  protected:
    double speed; ///< speed of the host
    double speedMean = 0.0; ///< speed mean
    double speedStdDev = 0.0; ///< speed standard deviation
    rad angle = rad(0.0); ///< angle of linear motion
    rad angleMean = rad(0.0); ///< angle mean
    rad angleStdDev = rad(0.0); ///< angle standard deviation
    double alpha = 1.0; ///< alpha parameter in [0;1] interval
    std::string m_CSVFilePath; ///< For saving positions in CSV file
    std::string m_CSVFileName; ///< For saving positions in CSV file
    std::filesystem::path m_CSVFullPath; ///< For saving positions in CSV file

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    /** @brief Initializes mobility model parameters.*/
    virtual void initialize(int stage) override;

    /** @brief Move the host*/
    virtual void move() override;

    /** @brief Calculate a new velocity. */
    void setTargetVelocity();

  public:
    virtual double getMaxSpeed() const override { return speed; }
    LinearGaussMarkovMobility();
};

} // namespace inet

#endif

