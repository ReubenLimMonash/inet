//
// Date: 14/05/2022
// Author: Reuben Lim Yaw Hui
// Desc: To implement comprehensive channel model for UAV communications
//

package inet.physicallayer.wireless.common.pathloss;

module LognormalRicianFading extends FreeSpacePathLoss
{
    parameters:
        double k @unit(dB) = default(8dB);
        double sigma = default(1);
        double a = default(0.1);
        double b = default(0.00075);
        double c = default(8);
        double alpha_min = default(2);
        double alpha_max = default(3);
        double sigma_a = default(11.25);
        double sigma_b = default(0.08);
        double K_min = default(7.8);
        double K_max = default(17.5);
        double K_min_U2U = default(-1.0);
        double K_max_U2U = default(-1.0);
        @class(LognormalRicianFading);
}