// PRT.h
// created by Kuangdai on 19-May-2017 
// particle relabelling transformation

#pragma once

class SolidResponse;
class FluidResponse;
#include <string>
#include "eigenp.h"

class PRT {
public:
    
    virtual ~PRT() {};
    
    virtual void sphericalToUndulated(FluidResponse &response) const = 0;
    virtual void undulatedToSpherical(FluidResponse &response) const = 0;
    
    virtual void sphericalToUndulated(SolidResponse &response) const = 0;
    virtual void undulatedToSpherical(SolidResponse &response) const = 0;
    
    virtual std::string verbose() const = 0;
    virtual bool is1D() const = 0;
    
    virtual void checkCompatibility(int Nr) const {};
    
    static PRT *createPRT(const RDMatXN4 &X, bool elem1D);
};