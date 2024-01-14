/***************************************************************************
* Date: 14/05/2022
* Author: Reuben Lim Yaw Hui
* Desc: To implement comprehensive channel model for UAV communications
* Modified on 06/10/2023 to allow recording of SINR to CSV
***************************************************************************/

#include "inet/physicallayer/wireless/common/pathloss/LognormalRicianFading.h"

namespace inet {

namespace physicallayer {

Define_Module(LognormalRicianFading);

LognormalRicianFading::LognormalRicianFading() :
    k(1), 
    sigma(1),
    a(0.1),
    b(0.00075),
    c(8),
    alpha_min(2),
    alpha_max(3),
    sigma_a(11.25),
    sigma_b(0.08),
    K_min(7.8),
    K_max(17.5),
    K_min_U2U(-1.0),
    K_max_U2U(-1.0)
{
}

void LognormalRicianFading::initialize(int stage)
{
    FreeSpacePathLoss::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        k = math::dB2fraction(par("k"));
        sigma = par("sigma");
        a = par("a");
        b = par("b");
        c = par("c");
        alpha_min = par("alpha_min");
        alpha_max = par("alpha_max");
        sigma_a = par("sigma_a");
        sigma_b = par("sigma_b");
        K_min = par("K_min"); // in dB
        K_max = par("K_max"); // in dB
        K_min_U2U = par("K_min_U2U"); // in dB
        K_max_U2U = par("K_max_U2U"); // in dB
        if (K_min_U2U == -1) {
            K_min_U2U = K_min;
        }
        if (K_max_U2U == -1) {
            K_max_U2U = K_max;
        }
    }
}

std::ostream& LognormalRicianFading::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "LognormalRicianFading";
    if (level <= PRINT_LEVEL_TRACE)
        stream << ", alpha = " << alpha
               << ", system loss = " << systemLoss
               << ", k = " << k;
    return stream;
}

double LognormalRicianFading::computePathLoss(mps propagationSpeed, Hz frequency, m distance) const
{
    // This compute path loss function calculates composite fading path loss without probability of LoS effects
    m waveLength = propagationSpeed / frequency;
    double c = 1.0 / (2.0 * (k + 1));
    double x = normal(0, 1);
    double y = normal(0, 1);
    double rr = c * ((x + sqrt(2 * k)) * (x + sqrt(2 * k)) + y * y);
    double freeSpacePathLoss = computeFreeSpacePathLoss(waveLength, distance, alpha, systemLoss);
    double shadowing = pow(10, normal(0,sigma) / 10.0);
    return rr * shadowing * freeSpacePathLoss;
}

// double LognormalRicianFading::computeCompositePathLoss(mps propagationSpeed, Hz frequency, m distance, const ITransmission *transmission, const IArrival *arrival) const
// {
//     // Compute the path loss taking into account composite fading and probability of LoS
//     // First, let's calculate the fading parameters based on the positions of Tx and Rx
//     m waveLength = propagationSpeed / frequency;
//     double plos = PLoS(arrival->getStartPosition(), transmission->getStartPosition()); // Probability of Line-of-Sight
//     double theta = theta_d(distance, arrival->getStartPosition(), transmission->getStartPosition()); // Elevation angle between Tx and Rx
//     double Alpha = (alpha_min - alpha_max) * plos + alpha_max; // Path Loss Exponent
//     double Sigma = math::dB2fraction(sigma_a * exp(- sigma_b * theta)); // Shadowing logarithmic std dev
//     double K = math::dB2fraction(K_min * exp(log(K_max / K_min) * pow(plos,2))); // Rician K factor

//     // Then, compute the path loss using the parameters
//     double c = 1.0 / (2.0 * (K + 1));
//     double x = normal(0, 1);
//     double y = normal(0, 1);
//     double rr = c * ((x + sqrt(2 * K)) * (x + sqrt(2 * K)) + y * y);
//     double freeSpacePathLoss = computeFreeSpacePathLoss(waveLength, distance, Alpha, systemLoss);
//     double shadowing = pow(10, normal(0, Sigma) / 10.0);
    
//     return rr * shadowing * freeSpacePathLoss;

// }

double LognormalRicianFading::computeCompositePathLoss(mps propagationSpeed, Hz frequency, m distance, const ITransmission *transmission, const IArrival *arrival) const
{
    // Compute the path loss taking into account composite fading and probability of LoS
    // First, let's calculate the fading parameters based on the positions of Tx and Rx
    // (12/2/2023) Change to differentiate pathloss calc btw U2G and U2U transmissions

    m waveLength = propagationSpeed / frequency;
    Coord rx_pos = arrival->getStartPosition();
    Coord tx_pos = transmission->getStartPosition();
    double height = std::abs(tx_pos.z - rx_pos.z);
    double rr;
    double freeSpacePathLoss;
    double shadowing;

    if ((height == 0) && (distance <= m(5))) {
        // This is a U2U transmission
        double Alpha = alpha_min;
        double K = math::dB2fraction(K_max_U2U); // For U2U, Rician K = K_max_U2U
        double c = 1.0 / (2.0 * (K + 1));
        double x = normal(0, 1);
        double y = normal(0, 1);
        rr = c * ((x + sqrt(2 * K)) * (x + sqrt(2 * K)) + y * y);
        freeSpacePathLoss = computeFreeSpacePathLoss(waveLength, distance, Alpha, systemLoss);
        shadowing = 1; // No shadowing in U2U
    }
    else {
        // This is a U2G transmission
        double plos = PLoS(rx_pos, tx_pos); // Probability of Line-of-Sight
        double theta = theta_d(distance, rx_pos, tx_pos); // Elevation angle between Tx and Rx
        double Alpha = (alpha_min - alpha_max) * plos + alpha_max; // Path Loss Exponent
        double Sigma = math::dB2fraction(sigma_a * exp(- sigma_b * theta)); // Shadowing logarithmic std dev
        double K_min_pow = math::dB2fraction(K_min); // K_min should be in dB
        double K_max_pow = math::dB2fraction(K_max); // K_max should be in dB
        double K = K_min_pow * exp(log(K_max_pow / K_min_pow) * pow(plos,2)); // Rician K factor

        // Then, compute the path loss using the parameters
        double c = 1.0 / (2.0 * (K + 1));
        double x = normal(0, 1);
        double y = normal(0, 1);
        rr = c * ((x + sqrt(2 * K)) * (x + sqrt(2 * K)) + y * y);
        freeSpacePathLoss = computeFreeSpacePathLoss(waveLength, distance, Alpha, systemLoss);
        shadowing = pow(10, normal(0, Sigma) / 10.0);
    }
    
    return rr * shadowing * freeSpacePathLoss;

}

double LognormalRicianFading::PLoS(const Coord& receiver, const Coord& transmitter) const
{
    // % This function implements the LoS probability model from the paper
    // % "Blockage Modeling for Inter-layer UAVs Communications in Urban
    // % Environments" 
    // % param r    : horizontal distance between Tx and Rx (m)
    // % param transmitter.z : height of Tx
    // % param receiver.z : height of Rx
    // % param a   : ratio of built up land area to total land area
    // % param b   : density of buildings (in /m^2) (note that the paper gives this value in /km^2)
    // % param c   : scale parameter that is used to characterize the building height
    // %              distributions using a Rayleigh distribution
    // %
    // % returns p  : LoS probability between Tx and Rx 

    // % The values of a1, a2 and a3 can be found from: Elevation Dependent Shadowing 
    // % Model for Mobile Communications via High Altitude Platforms in Built-Up Areas
    // % DOI:
    // % For suburban       : a = 0.1, b = 7.5e-4, c = 8
    // % For urban          : a = 0.3, b = 5e-4, c = 15
    // % For dense urban    : a = 0.5, b = 3e-4, c = 20
    // % For urban high-rise: a = 0.5, b = 3e-4, c = 50

    double delta_h = transmitter.z - receiver.z; // Difference in height between receiver and transmitter
    double r = sqrt(pow(transmitter.x - receiver.x, 2) + pow(transmitter.y - receiver.y, 2)); // horizontal distance between Tx and Rx (m)
    // Use the following pow_factor is assuming PPP building dist.
    // Ref: "Blockage Modeling for Inter-layer UAVs ..."
    // double pow_factor = 2 * r * sqrt(a * b / M_PI) + a;
    // Use the following pow_factor if assuming ITU-R's assumption
    // Ref: RECOMMENDATION ITU-R P.1410-5
    double pow_factor = r * sqrt(a * b);
    
    double h1, h2, p;
    if (delta_h == 0)  
    {
        p = pow((1 - exp((-pow(transmitter.z, 2))/(2 * pow(c, 2)))), pow_factor);
    }
    else
    {
        if (delta_h < 0)
        {
            h1 = receiver.z;
            h2 = transmitter.z;
            
        }
        else
        {
            h1 = transmitter.z;
            h2 = receiver.z;
        }
        delta_h = std::abs(delta_h);
        p = pow(1 - (sqrt(2 * M_PI) * c / delta_h) * std::abs((qfunc(h1 / c) - qfunc(h2 / c))), pow_factor);
    }
    return p;
}

double LognormalRicianFading::theta_d(m distance, const Coord& receiver, const Coord& transmitter) const
{
    double r = sqrt(pow(transmitter.x - receiver.x, 2) + pow(transmitter.y - receiver.y, 2)); // horizontal distance between Tx and Rx (m)
    double theta = math::arcos(r / distance.get()) * 180.0 / M_PI;
    return theta;
}

double LognormalRicianFading::qfunc(double x) const
{
    double q = 0.5 - 0.5 * erf(x / sqrt(2));
    return q;
}
} // namespace physicallayer

} // namespace inet
