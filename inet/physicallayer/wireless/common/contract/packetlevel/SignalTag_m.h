//
// Generated file, do not edit! Created by opp_msgtool 6.0 from inet/physicallayer/wireless/common/contract/packetlevel/SignalTag.msg.
//

#ifndef __INET_SIGNALTAG_M_H
#define __INET_SIGNALTAG_M_H

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <omnetpp.h>

// opp_msgtool version check
#define MSGC_VERSION 0x0600
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of opp_msgtool: 'make clean' should help.
#endif

// dll export symbol
#ifndef INET_API
#  if defined(INET_EXPORT)
#    define INET_API  OPP_DLLEXPORT
#  elif defined(INET_IMPORT)
#    define INET_API  OPP_DLLIMPORT
#  else
#    define INET_API
#  endif
#endif


namespace inet {

class SignalTagBase;
class SignalPowerTagBase;
class SignalPowerReq;
class SignalPowerInd;
class SignalBandTagBase;
class SignalBandReq;
class SignalBandInd;
class SignalBitrateTagBase;
class SignalBitrateReq;
class SignalBitrateInd;
class SnirInd;
class ErrorRateInd;
class SignalTimeInd;

}  // namespace inet

#include "inet/common/INETDefs_m.h" // import inet.common.INETDefs

#include "inet/common/TagBase_m.h" // import inet.common.TagBase

#include "inet/common/Units_m.h" // import inet.common.Units

// cplusplus {{
#include "inet/physicallayer/wireless/common/base/packetlevel/PhysicalLayerDefs.h"
// }}


namespace inet {

/**
 * Class generated from <tt>inet/physicallayer/wireless/common/contract/packetlevel/SignalTag.msg:21</tt> by opp_msgtool.
 * <pre>
 * //
 * // This is an abstract base class that should not be directly added as a tag.
 * //
 * class SignalTagBase extends TagBase
 * {
 * }
 * </pre>
 */
class INET_API SignalTagBase : public ::inet::TagBase
{
  protected:

  private:
    void copy(const SignalTagBase& other);

  protected:
    bool operator==(const SignalTagBase&) = delete;

  public:
    SignalTagBase();
    SignalTagBase(const SignalTagBase& other);
    virtual ~SignalTagBase();
    SignalTagBase& operator=(const SignalTagBase& other);
    virtual SignalTagBase *dup() const override {return new SignalTagBase(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const SignalTagBase& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, SignalTagBase& obj) {obj.parsimUnpack(b);}

/**
 * Class generated from <tt>inet/physicallayer/wireless/common/contract/packetlevel/SignalTag.msg:28</tt> by opp_msgtool.
 * <pre>
 * //
 * // This is an abstract base class that should not be directly added as a tag.
 * //
 * class SignalPowerTagBase extends SignalTagBase
 * {
 *     W power = W(NaN); // specifies signal power in Watts
 * }
 * </pre>
 */
class INET_API SignalPowerTagBase : public ::inet::SignalTagBase
{
  protected:
    W power = W(NaN);

  private:
    void copy(const SignalPowerTagBase& other);

  protected:
    bool operator==(const SignalPowerTagBase&) = delete;

  public:
    SignalPowerTagBase();
    SignalPowerTagBase(const SignalPowerTagBase& other);
    virtual ~SignalPowerTagBase();
    SignalPowerTagBase& operator=(const SignalPowerTagBase& other);
    virtual SignalPowerTagBase *dup() const override {return new SignalPowerTagBase(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    virtual W getPower() const;
    virtual void setPower(W power);
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const SignalPowerTagBase& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, SignalPowerTagBase& obj) {obj.parsimUnpack(b);}

/**
 * Class generated from <tt>inet/physicallayer/wireless/common/contract/packetlevel/SignalTag.msg:37</tt> by opp_msgtool.
 * <pre>
 * //
 * // This request determines the average analog signal power that should be used to transmit the packet.
 * // It may be present on a packet from the application to the phyisical layer.
 * //
 * class SignalPowerReq extends SignalPowerTagBase
 * {
 * }
 * </pre>
 */
class INET_API SignalPowerReq : public ::inet::SignalPowerTagBase
{
  protected:

  private:
    void copy(const SignalPowerReq& other);

  protected:
    bool operator==(const SignalPowerReq&) = delete;

  public:
    SignalPowerReq();
    SignalPowerReq(const SignalPowerReq& other);
    virtual ~SignalPowerReq();
    SignalPowerReq& operator=(const SignalPowerReq& other);
    virtual SignalPowerReq *dup() const override {return new SignalPowerReq(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const SignalPowerReq& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, SignalPowerReq& obj) {obj.parsimUnpack(b);}

/**
 * Class generated from <tt>inet/physicallayer/wireless/common/contract/packetlevel/SignalTag.msg:45</tt> by opp_msgtool.
 * <pre>
 * //
 * // This indication specifies the average analog signal power that was detected during receiving the packet.
 * // It may be present on a packet from the phyiscal layer to the application.
 * //
 * class SignalPowerInd extends SignalPowerTagBase
 * {
 * }
 * </pre>
 */
class INET_API SignalPowerInd : public ::inet::SignalPowerTagBase
{
  protected:

  private:
    void copy(const SignalPowerInd& other);

  protected:
    bool operator==(const SignalPowerInd&) = delete;

  public:
    SignalPowerInd();
    SignalPowerInd(const SignalPowerInd& other);
    virtual ~SignalPowerInd();
    SignalPowerInd& operator=(const SignalPowerInd& other);
    virtual SignalPowerInd *dup() const override {return new SignalPowerInd(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const SignalPowerInd& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, SignalPowerInd& obj) {obj.parsimUnpack(b);}

/**
 * Class generated from <tt>inet/physicallayer/wireless/common/contract/packetlevel/SignalTag.msg:52</tt> by opp_msgtool.
 * <pre>
 * //
 * // This is an abstract base class that should not be directly added as a tag.
 * //
 * class SignalBandTagBase extends SignalTagBase
 * {
 *     Hz centerFrequency = Hz(NaN); // carrier frequency in the range (0, +infinity) or NaN if not set.
 *     Hz bandwidth = Hz(NaN);        // bandwidth in the rage (0, +infinity) or NaN if not set.
 * }
 * </pre>
 */
class INET_API SignalBandTagBase : public ::inet::SignalTagBase
{
  protected:
    Hz centerFrequency = Hz(NaN);
    Hz bandwidth = Hz(NaN);

  private:
    void copy(const SignalBandTagBase& other);

  protected:
    bool operator==(const SignalBandTagBase&) = delete;

  public:
    SignalBandTagBase();
    SignalBandTagBase(const SignalBandTagBase& other);
    virtual ~SignalBandTagBase();
    SignalBandTagBase& operator=(const SignalBandTagBase& other);
    virtual SignalBandTagBase *dup() const override {return new SignalBandTagBase(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    virtual Hz getCenterFrequency() const;
    virtual void setCenterFrequency(Hz centerFrequency);

    virtual Hz getBandwidth() const;
    virtual void setBandwidth(Hz bandwidth);
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const SignalBandTagBase& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, SignalBandTagBase& obj) {obj.parsimUnpack(b);}

/**
 * Class generated from <tt>inet/physicallayer/wireless/common/contract/packetlevel/SignalTag.msg:62</tt> by opp_msgtool.
 * <pre>
 * //
 * // This request determines the signal band that should be used to transmit the packet.
 * // It may be present on a packet from the application to the phyisical layer.
 * //
 * class SignalBandReq extends SignalBandTagBase
 * {
 * }
 * </pre>
 */
class INET_API SignalBandReq : public ::inet::SignalBandTagBase
{
  protected:

  private:
    void copy(const SignalBandReq& other);

  protected:
    bool operator==(const SignalBandReq&) = delete;

  public:
    SignalBandReq();
    SignalBandReq(const SignalBandReq& other);
    virtual ~SignalBandReq();
    SignalBandReq& operator=(const SignalBandReq& other);
    virtual SignalBandReq *dup() const override {return new SignalBandReq(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const SignalBandReq& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, SignalBandReq& obj) {obj.parsimUnpack(b);}

/**
 * Class generated from <tt>inet/physicallayer/wireless/common/contract/packetlevel/SignalTag.msg:70</tt> by opp_msgtool.
 * <pre>
 * //
 * // This indication specifies the signal band that was used to receive the packet.
 * // It may be present on a packet from the phyiscal layer to the application.
 * //
 * class SignalBandInd extends SignalBandTagBase
 * {
 * }
 * </pre>
 */
class INET_API SignalBandInd : public ::inet::SignalBandTagBase
{
  protected:

  private:
    void copy(const SignalBandInd& other);

  protected:
    bool operator==(const SignalBandInd&) = delete;

  public:
    SignalBandInd();
    SignalBandInd(const SignalBandInd& other);
    virtual ~SignalBandInd();
    SignalBandInd& operator=(const SignalBandInd& other);
    virtual SignalBandInd *dup() const override {return new SignalBandInd(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const SignalBandInd& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, SignalBandInd& obj) {obj.parsimUnpack(b);}

/**
 * Class generated from <tt>inet/physicallayer/wireless/common/contract/packetlevel/SignalTag.msg:77</tt> by opp_msgtool.
 * <pre>
 * //
 * // This is an abstract base class that should not be directly added as a tag.
 * //
 * class SignalBitrateTagBase extends SignalTagBase
 * {
 *     bps preambleBitrate = bps(NaN); // preamble bitrate in the range (0, +infinity) or NaN if not set.
 *     bps headerBitrate = bps(NaN);   // header bitrate in the range (0, +infinity) or NaN if not set.
 *     bps dataBitrate = bps(NaN);     // data bitrate in the range (0, +infinity) or NaN if not set.
 * }
 * </pre>
 */
class INET_API SignalBitrateTagBase : public ::inet::SignalTagBase
{
  protected:
    bps preambleBitrate = bps(NaN);
    bps headerBitrate = bps(NaN);
    bps dataBitrate = bps(NaN);

  private:
    void copy(const SignalBitrateTagBase& other);

  protected:
    bool operator==(const SignalBitrateTagBase&) = delete;

  public:
    SignalBitrateTagBase();
    SignalBitrateTagBase(const SignalBitrateTagBase& other);
    virtual ~SignalBitrateTagBase();
    SignalBitrateTagBase& operator=(const SignalBitrateTagBase& other);
    virtual SignalBitrateTagBase *dup() const override {return new SignalBitrateTagBase(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    virtual bps getPreambleBitrate() const;
    virtual void setPreambleBitrate(bps preambleBitrate);

    virtual bps getHeaderBitrate() const;
    virtual void setHeaderBitrate(bps headerBitrate);

    virtual bps getDataBitrate() const;
    virtual void setDataBitrate(bps dataBitrate);
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const SignalBitrateTagBase& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, SignalBitrateTagBase& obj) {obj.parsimUnpack(b);}

/**
 * Class generated from <tt>inet/physicallayer/wireless/common/contract/packetlevel/SignalTag.msg:88</tt> by opp_msgtool.
 * <pre>
 * //
 * // This request determines the bitrates for various parts of the signal that should be used to transmit the packet.
 * // It may be present on a packet from the application to the phyisical layer.
 * //
 * class SignalBitrateReq extends SignalBitrateTagBase
 * {
 * }
 * </pre>
 */
class INET_API SignalBitrateReq : public ::inet::SignalBitrateTagBase
{
  protected:

  private:
    void copy(const SignalBitrateReq& other);

  protected:
    bool operator==(const SignalBitrateReq&) = delete;

  public:
    SignalBitrateReq();
    SignalBitrateReq(const SignalBitrateReq& other);
    virtual ~SignalBitrateReq();
    SignalBitrateReq& operator=(const SignalBitrateReq& other);
    virtual SignalBitrateReq *dup() const override {return new SignalBitrateReq(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const SignalBitrateReq& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, SignalBitrateReq& obj) {obj.parsimUnpack(b);}

/**
 * Class generated from <tt>inet/physicallayer/wireless/common/contract/packetlevel/SignalTag.msg:96</tt> by opp_msgtool.
 * <pre>
 * //
 * // This indication specifies the bitrates for various parts of the signal that was used to receive the packet.
 * // It may be present on a packet from the phyiscal layer to the application.
 * //
 * class SignalBitrateInd extends SignalBitrateTagBase
 * {
 * }
 * </pre>
 */
class INET_API SignalBitrateInd : public ::inet::SignalBitrateTagBase
{
  protected:

  private:
    void copy(const SignalBitrateInd& other);

  protected:
    bool operator==(const SignalBitrateInd&) = delete;

  public:
    SignalBitrateInd();
    SignalBitrateInd(const SignalBitrateInd& other);
    virtual ~SignalBitrateInd();
    SignalBitrateInd& operator=(const SignalBitrateInd& other);
    virtual SignalBitrateInd *dup() const override {return new SignalBitrateInd(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const SignalBitrateInd& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, SignalBitrateInd& obj) {obj.parsimUnpack(b);}

/**
 * Class generated from <tt>inet/physicallayer/wireless/common/contract/packetlevel/SignalTag.msg:104</tt> by opp_msgtool.
 * <pre>
 * //
 * // This indication specifies the signal to noise ratio that was detected during receiving the packet.
 * // It may be present on a packet from the phyiscal layer to the application.
 * //
 * class SnirInd extends SignalTagBase
 * {
 *     double minimumSnir = NaN; // minimum signal to noise plus interference ratio in the range (0, +infinity) or NaN if unknown.
 *     double maximumSnir = NaN; // maximum signal to noise plus interference ratio in the range (0, +infinity) or NaN if unknown.
 *     double averageSnir = NaN; // average signal to noise plus interference ratio in the range (0, +infinity) or NaN if unknown.
 *     double test = NaN; // custom added field
 * }
 * </pre>
 */
class INET_API SnirInd : public ::inet::SignalTagBase
{
  protected:
    double minimumSnir = NaN;
    double maximumSnir = NaN;
    double averageSnir = NaN;
    double queueingTime = NaN;
    double txDistance = NaN;
    double backoffPeriod = NaN;
    std::string fileName = "NaN";
    double rxTime = NaN;
    double U2GSnir = NaN;
    double U2USnir = NaN;
    double U2GDistance = NaN;

  private:
    void copy(const SnirInd& other);

  protected:
    bool operator==(const SnirInd&) = delete;

  public:
    SnirInd();
    SnirInd(const SnirInd& other);
    virtual ~SnirInd();
    SnirInd& operator=(const SnirInd& other);
    virtual SnirInd *dup() const override {return new SnirInd(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    virtual double getMinimumSnir() const;
    virtual void setMinimumSnir(double minimumSnir);

    virtual double getMaximumSnir() const;
    virtual void setMaximumSnir(double maximumSnir);

    virtual double getAverageSnir() const;
    virtual void setAverageSnir(double averageSnir);

    virtual double getQueueingTime() const;
    virtual void setQueueingTime(double queueingTime);

    virtual double getTxDistance() const;
    virtual void setTxDistance(double txDistance);

    virtual double getBackoffPeriod() const;
    virtual void setBackoffPeriod(double backoffPeriod);

    virtual std::string getFileName() const;
    virtual void setFileName(std::string fileName);

    virtual double getRxTime() const;
    virtual void setRxTime(double rxTime);

    virtual double getU2GSnir() const;
    virtual void setU2GSnir(double U2GSnir);

    virtual double getU2USnir() const;
    virtual void setU2USnir(double U2USnir);

    virtual double getU2GDistance() const;
    virtual void setU2GDistance(double U2GDistance);
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const SnirInd& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, SnirInd& obj) {obj.parsimUnpack(b);}

/**
 * Class generated from <tt>inet/physicallayer/wireless/common/contract/packetlevel/SignalTag.msg:115</tt> by opp_msgtool.
 * <pre>
 * //
 * // This indication specifies various error rates that was computed during receiving the packet.
 * // It may be present on a packet from the phyiscal layer to the application.
 * //
 * class ErrorRateInd extends SignalTagBase
 * {
 *     double packetErrorRate = NaN; // packet error rate (probability) in the range [0, 1] or NaN if unknown.
 *     double bitErrorRate = NaN;    // bit error rate (probability) in the range [0, 1] or NaN if unknown.
 *     double symbolErrorRate = NaN; // symbol error rate (probability) in the range [0, 1] or NaN if unknown.
 * }
 * </pre>
 */
class INET_API ErrorRateInd : public ::inet::SignalTagBase
{
  protected:
    double packetErrorRate = NaN;
    double bitErrorRate = NaN;
    double symbolErrorRate = NaN;
    double U2GBer = NaN;
    double U2UBer = NaN;

  private:
    void copy(const ErrorRateInd& other);

  protected:
    bool operator==(const ErrorRateInd&) = delete;

  public:
    ErrorRateInd();
    ErrorRateInd(const ErrorRateInd& other);
    virtual ~ErrorRateInd();
    ErrorRateInd& operator=(const ErrorRateInd& other);
    virtual ErrorRateInd *dup() const override {return new ErrorRateInd(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    virtual double getPacketErrorRate() const;
    virtual void setPacketErrorRate(double packetErrorRate);

    virtual double getBitErrorRate() const;
    virtual void setBitErrorRate(double bitErrorRate);

    virtual double getSymbolErrorRate() const;
    virtual void setSymbolErrorRate(double symbolErrorRate);

    virtual double getU2GBer() const;
    virtual void setU2GBer(double U2GBer);

    virtual double getU2UBer() const;
    virtual void setU2UBer(double U2UBer);
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const ErrorRateInd& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, ErrorRateInd& obj) {obj.parsimUnpack(b);}

/**
 * Class generated from <tt>inet/physicallayer/wireless/common/contract/packetlevel/SignalTag.msg:126</tt> by opp_msgtool.
 * <pre>
 * //
 * // This indication specifies the timing of the received signal.
 * // It may be present on a packet from the phyiscal layer to the application.
 * //
 * class SignalTimeInd extends SignalTagBase
 * {
 *     simtime_t startTime;
 *     simtime_t endTime;
 * }
 * </pre>
 */
class INET_API SignalTimeInd : public ::inet::SignalTagBase
{
  protected:
    ::omnetpp::simtime_t startTime = SIMTIME_ZERO;
    ::omnetpp::simtime_t endTime = SIMTIME_ZERO;

  private:
    void copy(const SignalTimeInd& other);

  protected:
    bool operator==(const SignalTimeInd&) = delete;

  public:
    SignalTimeInd();
    SignalTimeInd(const SignalTimeInd& other);
    virtual ~SignalTimeInd();
    SignalTimeInd& operator=(const SignalTimeInd& other);
    virtual SignalTimeInd *dup() const override {return new SignalTimeInd(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    virtual ::omnetpp::simtime_t getStartTime() const;
    virtual void setStartTime(::omnetpp::simtime_t startTime);

    virtual ::omnetpp::simtime_t getEndTime() const;
    virtual void setEndTime(::omnetpp::simtime_t endTime);
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const SignalTimeInd& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, SignalTimeInd& obj) {obj.parsimUnpack(b);}


}  // namespace inet


namespace omnetpp {

template<> inline inet::SignalTagBase *fromAnyPtr(any_ptr ptr) { return check_and_cast<inet::SignalTagBase*>(ptr.get<cObject>()); }
template<> inline inet::SignalPowerTagBase *fromAnyPtr(any_ptr ptr) { return check_and_cast<inet::SignalPowerTagBase*>(ptr.get<cObject>()); }
template<> inline inet::SignalPowerReq *fromAnyPtr(any_ptr ptr) { return check_and_cast<inet::SignalPowerReq*>(ptr.get<cObject>()); }
template<> inline inet::SignalPowerInd *fromAnyPtr(any_ptr ptr) { return check_and_cast<inet::SignalPowerInd*>(ptr.get<cObject>()); }
template<> inline inet::SignalBandTagBase *fromAnyPtr(any_ptr ptr) { return check_and_cast<inet::SignalBandTagBase*>(ptr.get<cObject>()); }
template<> inline inet::SignalBandReq *fromAnyPtr(any_ptr ptr) { return check_and_cast<inet::SignalBandReq*>(ptr.get<cObject>()); }
template<> inline inet::SignalBandInd *fromAnyPtr(any_ptr ptr) { return check_and_cast<inet::SignalBandInd*>(ptr.get<cObject>()); }
template<> inline inet::SignalBitrateTagBase *fromAnyPtr(any_ptr ptr) { return check_and_cast<inet::SignalBitrateTagBase*>(ptr.get<cObject>()); }
template<> inline inet::SignalBitrateReq *fromAnyPtr(any_ptr ptr) { return check_and_cast<inet::SignalBitrateReq*>(ptr.get<cObject>()); }
template<> inline inet::SignalBitrateInd *fromAnyPtr(any_ptr ptr) { return check_and_cast<inet::SignalBitrateInd*>(ptr.get<cObject>()); }
template<> inline inet::SnirInd *fromAnyPtr(any_ptr ptr) { return check_and_cast<inet::SnirInd*>(ptr.get<cObject>()); }
template<> inline inet::ErrorRateInd *fromAnyPtr(any_ptr ptr) { return check_and_cast<inet::ErrorRateInd*>(ptr.get<cObject>()); }
template<> inline inet::SignalTimeInd *fromAnyPtr(any_ptr ptr) { return check_and_cast<inet::SignalTimeInd*>(ptr.get<cObject>()); }

}  // namespace omnetpp

#endif // ifndef __INET_SIGNALTAG_M_H

