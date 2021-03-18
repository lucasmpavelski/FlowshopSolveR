#pragma once

#include <eoOp.h>
#include <paradiseo/eo/eo>
#include <paradiseo/mo/mo>

template <class EOT>
class DestructionStrategy : public eoUF<EOT&, EOT> {};
