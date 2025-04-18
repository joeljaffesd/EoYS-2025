#ifndef VFX_MAIN_HPP
#define VFX_MAIN_HPP

#include "autoPulse.hpp"
#include "manualPulse.hpp"
#include "orbit.hpp"
#include "ripple.hpp"
#include "scatter.hpp"
#include "vfxUtility.hpp"

// After creating effect instances//
////Example setup ////
//// Set Base Mesh for pulsing ////
// pulse.setBaseMesh(mesh.vertices());

//// Set Parameters ////
// ripple.setParams(1.0, 0.5, 4.0, 'y');
// orbit.setParams(1.0, 1.0, {0,2,1}, 0, -1, 1, 1);
// pulse.setParams(1.0, 1.0, 1);
//// push effects to chain /////
// effectChain.pushBack(&pulse);
//  effectChain.pushBack(&orbit);
// effectChain.pushBack(&ripple);
// effectChain.pushBack(&scatter); // not working yet

#endif