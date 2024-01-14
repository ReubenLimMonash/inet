//
// Copyright (C) 2005 Emin Ilker Cetinbas
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//
// Author: Reuben Lim (Reuben.Lim"monash.edu)
// Modified from: Linear Mobility
//

package inet.mobility.single;

import inet.mobility.base.MovingMobilityBase;

//
// This is a linear gauss markov mobility model with speed, angle and acceleration parameters.
//
// Uses a Gauss-Markov model to control the randomness in the movement.
// Totally random walk (Brownian motion) is obtained by setting alpha=0,
// while alpha=1 results a linear motion.
//
simple LinearGaussMarkovMobility extends MovingMobilityBase
{
    parameters:
        double initialX @unit(m) = default(uniform(this.constraintAreaMinX, this.constraintAreaMaxX));
        double initialY @unit(m) = default(uniform(this.constraintAreaMinY, this.constraintAreaMaxY));
        double initialZ @unit(m) = default(nanToZero(uniform(this.constraintAreaMinZ, this.constraintAreaMaxZ)));
        bool initFromDisplayString = default(true); // enables one time initialization from the subject module's display string
        double speed @unit(mps) = default(0mps); // speed of the host
        double initialMovementHeading @unit(deg) = default(uniform(0deg, 360deg));
        double initialMovementElevation @unit(deg) = default(0deg);
        double speedStdDev @unit(mps);     // speed standard deviation
        double angle @unit(deg) = default(uniform(0deg, 360deg));
        double angleStdDev @unit(deg);     // angle standard deviation
        double alpha = default(0.5); // [0;1], 0 - brown motion, 1 - linear motion
        string csvFileName = default("");
        string csvFilePath = default("");
        @class(LinearGaussMarkovMobility);
}

