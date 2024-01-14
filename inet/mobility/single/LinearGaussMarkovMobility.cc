//
// Copyright (C) 2005 Emin Ilker Cetinbas
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//
// Author: Reuben Lim (Reuben.Lim"monash.edu)
// Modified from: Linear Mobility
//

#include "inet/mobility/single/LinearGaussMarkovMobility.h"

#include "inet/common/INETMath.h"

#include <fstream>
#include <iostream>
#include <filesystem>

namespace inet {

Define_Module(LinearGaussMarkovMobility);

LinearGaussMarkovMobility::LinearGaussMarkovMobility()
{
    speedMean = 0;
}

void LinearGaussMarkovMobility::initialize(int stage)
{
    MovingMobilityBase::initialize(stage);

    EV_TRACE << "initializing LinearGaussMarkovMobility stage " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        speedMean = par("speed");
        speedStdDev = par("speedStdDev");
        angleMean = deg(fmod(deg(par("angle")).get(), 360.0));
        angleStdDev = deg(par("angleStdDev"));
        alpha = par("alpha");
        if (alpha < 0.0 || alpha > 1.0)
            throw cRuntimeError("The parameter 'alpha' is out of [0;1] interval");
        rad heading = deg(fmod(par("initialMovementHeading").doubleValue(), 360));
        rad elevation = deg(fmod(par("initialMovementElevation").doubleValue(), 360));
        Coord direction = Quaternion(EulerAngles(angleMean, -elevation, rad(0))).rotate(Coord::X_AXIS);
        lastVelocity = direction * speedMean;
        speed = speedMean;
        angle = angleMean;
        stationary = (speedMean == 0);

        // For recording positions
        m_CSVFilePath = par("csvFilePath").stdstringValue(); // Folder to store CSV file
        m_CSVFileName = par("csvFileName").stdstringValue(); // File name to store CSV file
        std::filesystem::path dir (m_CSVFilePath.c_str());
        std::filesystem::path file (m_CSVFileName.c_str());
        m_CSVFullPath = dir / file ;
        // Write header for Tx CSV
        std::ofstream out(m_CSVFullPath);
        out << "Time," << "X," << "Y," << "Z," << "SpeedX," << "SpeedY," << "SpeedZ," << std::endl;
        out.close();
    }
}

void LinearGaussMarkovMobility::move()
{
    simtime_t now = simTime();
    if (now >= nextChange) {
        setTargetVelocity();
    }
    double elapsedTime = (now - lastUpdate).dbl();
    lastPosition += lastVelocity * elapsedTime;

    // Record movement
    std::ofstream out(m_CSVFullPath, std::ios::app);
    out << now << "," << lastPosition.x << "," << lastPosition.y << "," << lastPosition.z << "," 
        << lastVelocity.x << "," << lastVelocity.y << "," << lastVelocity.z << std::endl;
    out.close();

    // do something if we reach the wall
    Coord dummyCoord;
    handleIfOutside(REFLECT, dummyCoord, lastVelocity);
}

void LinearGaussMarkovMobility::setTargetVelocity()
{
    // calculate new speed and direction based on the model
    speed = alpha * speed
        + (1.0 - alpha) * speedMean
        + sqrt(1.0 - alpha * alpha) * normal(0.0, 1.0) * speedStdDev;

    angle = alpha * angle
        + (1.0 - alpha) * angleMean
        + rad(sqrt(1.0 - alpha * alpha) * normal(0.0, 1.0) * angleStdDev);

    Coord direction(cos(rad(angle).get()), sin(rad(angle).get()));
    nextChange = simTime() + updateInterval;
    lastVelocity = direction * speed;

    EV_DEBUG << " speed = " << speed << " angle = " << angle << endl;
    EV_DEBUG << " mspeed = " << speedMean << " mangle = " << angleMean << endl;
}

} // namespace inet

