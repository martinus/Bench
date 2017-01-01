// Bench - Simple Microbenchmarks.
//
// Copyright (c) 2016 by Martin Ankerl. 
// This is free and unencumbered software released into the public domain.
// For more information, please refer to <http://unlicense.org>

#include <chrono>
#include <algorithm>
#include <vector>
#include <iostream>
#include <sstream>

class Bench;
std::ostream& operator<<(std::ostream& os, const Bench& bench);

// Benchmark.
class Bench {
private:
    typedef std::chrono::high_resolution_clock clock;

    std::string mName;
    size_t mItersLeft;
    size_t mNumIters;
    std::chrono::time_point<clock> mStartTime;
    std::vector<std::pair<size_t, double>> mMeasurements;
    
    size_t mMaxNumMeasurements;
    double mDesiredSecondsPerMeasurement;
    double mMaxTotalMeasuredSeconds;
    double mTotalMeasuredSeconds;

    double mUnitPerIteration;
    std::string mUnitName;

    struct Measurement {
        double value;
        size_t benchIdx;
        int rank;
        Measurement(double value, size_t benchIdx)
            : value(value)
            , benchIdx(benchIdx)
            , rank(0) {
        }

        bool operator<(const Measurement& o) const {
            return value < o.value;
        }
    };

    // True if the hypothesis H0 can be rejected.
    // true on success, false if normal estimation is necessary.
    static bool isRejectingH0(int n1, int n2, const double uObt) {
        // table from https://de.wikipedia.org/wiki/Wilcoxon-Mann-Whitney-Test#Tabelle_der_kritischen_Werte_der_Mann-Whitney-U-Statistik
        // for 5% two sides, or 2.5% one side, with n2 < n1
        static const int mannWhitneyU5[] = {
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,0,
            -1,-1,-1,-1,-1,-1,0,0,0,0,1,1,1,1,1,2,2,2,2,3,3,3,3,3,4,4,4,4,5,5,5,5,5,6,6,6,6,7,7,
            -1,-1,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,13,13,14,14,15,15,16,16,17,17,18,18,
            0,1,2,3,4,4,5,6,7,8,9,10,11,11,12,13,14,15,16,17,17,18,19,20,21,22,23,24,24,25,26,27,28,29,30,31,31,
            2,3,5,6,7,8,9,11,12,13,14,15,17,18,19,20,22,23,24,25,27,28,29,30,32,33,34,35,37,38,39,40,41,43,44,45,
            5,6,8,10,11,13,14,16,17,19,21,22,24,25,27,29,30,32,33,35,37,38,40,42,43,45,46,48,50,51,53,55,56,58,59,
            8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62,64,66,68,70,72,74,
            13,15,17,19,22,24,26,29,31,34,36,38,41,43,45,48,50,53,55,57,60,62,65,67,69,72,74,77,79,81,84,86,89,
            17,20,23,26,28,31,34,37,39,42,45,48,50,53,56,59,62,64,67,70,73,76,78,81,84,87,89,92,95,98,101,103,
            23,26,29,33,36,39,42,45,48,52,55,58,61,64,67,71,74,77,80,83,87,90,93,96,99,103,106,109,112,115,119,
            30,33,37,40,44,47,51,55,58,62,65,69,73,76,80,83,87,90,94,98,101,105,108,112,116,119,123,127,130,134,
            37,41,45,49,53,57,61,65,69,73,77,81,85,89,93,97,101,105,109,113,117,121,125,129,133,137,141,145,149,
            45,50,54,59,63,67,72,76,80,85,89,94,98,102,107,111,116,120,125,129,133,138,142,147,151,156,160,165,
            55,59,64,69,74,78,83,88,93,98,102,107,112,117,122,127,131,136,141,146,151,156,161,165,170,175,180,
            64,70,75,80,85,90,96,101,106,111,117,122,127,132,138,143,148,153,159,164,169,174,180,185,190,196,
            75,81,86,92,98,103,109,115,120,126,132,137,143,149,154,160,166,171,177,183,188,194,200,206,211,
            87,93,99,105,111,117,123,129,135,141,147,154,160,166,172,178,184,190,196,202,209,215,221,227,
            99,106,112,119,125,132,138,145,151,158,164,171,177,184,190,197,203,210,216,223,230,236,243,
            113,119,126,133,140,147,154,161,168,175,182,189,196,203,210,217,224,231,238,245,252,258,
            127,134,141,149,156,163,171,178,186,193,200,208,215,222,230,237,245,252,259,267,274
        };

        if (n1 < n2) {
            std::swap(n1, n2);
        }

        double uCriticalValue;
        if (n1 <= 40 && n2 <= 20) {
            // look up table.
            const size_t idx = n2 * (83 - n2) / 2 + n1 - n2 - 41;
            uCriticalValue = mannWhitneyU5[idx];

        } else {
            // use approximation. I've solved the formula in http://math.usask.ca/~laverty/S245/Tables/wmw.pdf for U.
            // constant taken from https://en.wikipedia.org/wiki/1.96
            const double z = 1.95996398454005423552;
            uCriticalValue = std::fabs(z * std::sqrt(n1*n2*(n1 + n2 + 1.0) / 12.0) - (n1*n2) / 2.0);
        }

        // if we are <=, we can reject H0: medians are different!
        return uObt <= uCriticalValue;
    }

    // True if the hypothesis H0 can be rejected. Performs a Mann–Whitney U test.
    // Unlike the t-test it does not require the assumption of normal distributions.
    // It is nearly as efficient as the t-test on normal distributions
    // see https://en.wikipedia.org/wiki/Mann%E2%80%93Whitney_U_test
    static bool isRejectingH0(const std::vector<std::pair<size_t, double>>& m1, const std::vector<std::pair<size_t, double>>& m2) {
        std::vector<Measurement> measurements;
        for (size_t i = 0; i < m1.size(); ++i) {
            measurements.push_back(Measurement(m1[i].second / m1[i].first, 0));
        }
        for (size_t i = 0; i < m2.size(); ++i) {
            measurements.push_back(Measurement(m2[i].second / m2[i].first, 1));
        }
        std::sort(measurements.begin(), measurements.end());

        // calculate rank
        size_t backIdx = 0;
        size_t frontIdx = 1;
        while (frontIdx <= measurements.size()) {
            if (frontIdx == measurements.size() || measurements[backIdx].value != measurements[frontIdx].value) {
                // catch up. Rank starts at 1, but idx at 0. Also we keep double the rank, so we can use integer values instead of
                // .5 values. we divide it later, after we've sum up. This also prevents any rounding errors and I can use int.
                const int rank = (frontIdx + backIdx + 1);
                while (backIdx != frontIdx) {
                    measurements[backIdx].rank = rank;
                    ++backIdx;
                }
            }
            ++frontIdx;
        }

        // calculate sum of ranks
        int rank1Doubled = 0;
        int rank2Doubled = 0;
        for (size_t i = 0; i < measurements.size(); ++i) {
            if (0 == measurements[i].benchIdx) {
                rank1Doubled += measurements[i].rank;
            } else {
                rank2Doubled += measurements[i].rank;
            }
        }
        const size_t n1 = m1.size();
        const size_t n2 = m2.size();
        const double u1 = (n1*n2 + n1*(n1 + 1) / 2) - rank1Doubled / 2.0;
        const double u2 = (n1*n2 + n2*(n2 + 1) / 2) - rank2Doubled / 2.0;
        const double uMin = std::min(u1, u2);

        return isRejectingH0(n1, n2, uMin);
    }

public:
    // performs a Wilcoxon-Mann-Whitney-Test
    // see https://de.wikipedia.org/wiki/Wilcoxon-Mann-Whitney-Test
    static std::string compare(const Bench& b1, const Bench& b2) {
        std::stringstream ss;
        ss << b1 << std::endl
            << b2 << std::endl;
        if (isRejectingH0(b1.measurements(), b2.measurements())) {
            ss << "REJECTING Hypotesis 0, so medians are with 95% certainty NOT equal";
        } else {
            ss << "can't reject Hypothesis 0, so medians might be equal.";
        }
        return ss.str();
    }

    Bench(const char* name = "", double maxTotalMeasuredSeconds = 1, size_t maxNumMeasurements = 10, size_t measurementCertaintyFactor = 1000)
        : mName(name)
        , mItersLeft(0)
        , mNumIters(0)
        , mUnitPerIteration(1.0)
        , mUnitName("iteration") {

        /*
        // performs automatic calibration
        uint64_t bestMeasurementPrecisionNanos = -1;

        for (size_t i = 0; i < 100; ++i) {
            // find out how precise we can measure time
            std::chrono::time_point<clock> finishedTime;
            const auto startTime = clock::now();
            do {
                finishedTime = clock::now();
            } while (startTime == finishedTime);

            std::chrono::duration<uint64_t, std::nano> duration = finishedTime - startTime;
            bestMeasurementPrecisionNanos = std::min(bestMeasurementPrecisionNanos, duration.count());
        }

        // we want to be well above the best measurement precision for each measurement
        const uint64_t measurementTimeNanos = std::min(UINT64_C(1000000000), bestMeasurementPrecisionNanos * measurementCertaintyFactor);

        // convert maxTotalMeasuredSeconds to nanos
        const uint64_t maxTotalMeasuredNanos = static_cast<uint64_t>(maxTotalMeasuredSeconds * UINT64_C(1000000000));

        mMaxNumMeasurements = static_cast<size_t>(std::min(maxNumMeasurements, maxTotalMeasuredNanos / measurementTimeNanos));
        mDesiredSecondsPerMeasurement = measurementTimeNanos / 1e9;
        mMaxTotalMeasuredSeconds = maxTotalMeasuredSeconds;

        //std::cout << "minNanos=" << minNanos << " , measurementTimeNanos=" << measurementTimeNanos << std::endl; 
        */
        mMaxNumMeasurements = maxNumMeasurements;
        mDesiredSecondsPerMeasurement = maxTotalMeasuredSeconds / maxNumMeasurements;
        mMaxTotalMeasuredSeconds = maxTotalMeasuredSeconds;
    }

    Bench(double maxTotalMeasuredSeconds, double secondsPerMeasurement, size_t minNumMeasurements, size_t maxNumMeasurements)
        : mItersLeft(0)
        , mNumIters(0)
        , mStartTime()
        , mMeasurements()
        , mMaxNumMeasurements(maxNumMeasurements)
        , mDesiredSecondsPerMeasurement(secondsPerMeasurement)
        , mMaxTotalMeasuredSeconds(maxTotalMeasuredSeconds) {
    }

    inline operator bool() {
        // i-- is *much* faster than --i, thanks to parallel execution
        // empty loop: 2.78998e-10 vs. 1.67208e-09, 6 times faster.
        if (mItersLeft--) {
            return true;
        }

        if (!measure(clock::now())) {
            mNumIters = 0;
            mItersLeft = 0;
            return false;
        }

        // prepare for next measurement
        mStartTime = clock::now();
        return true;
    }

    double min() const {
        double v = std::numeric_limits<double>::max();
        for (const auto& m : mMeasurements) {
            v = std::min(v, m.second / m.first);
        }
        return v;
    }

    const std::vector<std::pair<size_t, double>>& measurements() const {
        return mMeasurements;
    }

    Bench& unitsOfMeasurement(const std::string& name, const double unitPerIteration) {
        mUnitPerIteration = unitPerIteration;
        mUnitName = name;
        return *this;
    }

    // Finds a reasonable prefix and factor for the given value.
    static void metricPrefix(const double v, std::string& prefix, double& factor, int& power) {
        // see https://de.wikipedia.org/wiki/Vors%C3%A4tze_f%C3%BCr_Ma%C3%9Feinheiten
        static const char* symbol[] = { "Y", "Z", "E", "P", "T", "G", "M", "k", "", "m", "u", "n", "p", "f", "a", "z", "y" };
        static const int highestPower = 24;
        const int numSymbols = sizeof(symbol) / sizeof(*symbol);
        for (int i = 0; i < numSymbols; ++i) {
            power = highestPower - i * 3;
            if (v > std::pow(10, power)) {
                prefix = symbol[i];
                factor = std::pow(10, -power);
                return;
            }
        }

        // all else failed!
        prefix = "";
        factor = 1.0;
        power = 1;
    }

    void print(std::ostream& os) const {
        double v = min() / mUnitPerIteration;
        std::string prefix;
        double factor;
        int power;
        metricPrefix(v, prefix, factor, power);

        os << (v * factor) << " " << prefix << "s/" << mUnitName << " (1e" << power << ")";
        if (!mName.empty()) {
            os << " for " << mName;
        }
    }

private:
    // true if we should continue measuring
    bool measure(std::chrono::time_point<clock> finishedTime) {
        if (0 == mNumIters) {
            // cleanup (potential) previous stuff
            mMeasurements.clear();
            mTotalMeasuredSeconds = 0;
            mNumIters = 1;
        } else {
            std::chrono::duration<double> duration = finishedTime - mStartTime;
            const auto actualMeasurementSeconds = duration.count();
            auto actualToDesired = actualMeasurementSeconds / mDesiredSecondsPerMeasurement;

            if (actualToDesired < 0.1) {
                // we are far off, need more than 10 times as many iterations.
                actualToDesired = 0.1;
            } else if (actualToDesired > 0.6) {
                // e.g. 0.7 would not work, potential endless loop
                mMeasurements.push_back(std::make_pair(mNumIters, actualMeasurementSeconds));
                mTotalMeasuredSeconds += actualMeasurementSeconds;
            }

            // update number of measurements.
            // +0.5 for correct rounding
            mNumIters = static_cast<size_t>(mNumIters / actualToDesired + 0.5);
            if (0 == mNumIters) {
                mNumIters = 1;
            }
        }
        mItersLeft = mNumIters - 1;

        // stop if any criteria is met
        return mTotalMeasuredSeconds < mMaxTotalMeasuredSeconds && mMeasurements.size() < mMaxNumMeasurements;
    }
};

std::ostream& operator<<(std::ostream& os, const Bench& bench) {
    bench.print(os);
    return os;
}

