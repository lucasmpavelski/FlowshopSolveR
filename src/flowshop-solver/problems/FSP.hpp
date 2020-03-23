#pragma once

#include "paradiseo/eo/eoInt.h"
#include "paradiseo/eo/eoScalarFitness.h"
#include "paradiseo/mo/problems/permutation/moShiftNeighbor.h"

using FSPMax = eoInt<eoMaximizingFitness>;
using FSPMin = eoInt<eoMinimizingFitness>;
using FSP = FSPMin;
using FSPNeighbor = moShiftNeighbor<FSP>;
