#pragma once

#include <eoInt.h>
#include <eoScalarFitness.h>

using FSPMax = eoInt<eoMaximizingFitness>;
using FSPMin = eoInt<eoMinimizingFitness>;
using FSP = FSPMin;

