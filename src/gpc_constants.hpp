#pragma once

#include <string>

#include "gpc_enum.hpp"

namespace gpc {

const std::string GPC_VERSION = "2.33";
const std::string CPP_GPC_VERSION = "1.00";

/* Increase GPC_EPSILON to encourage merging of near coincident edges    */
const double GPC_EPSILON = DBL_EPSILON;

/* Horizontal edge state transitions within scanbeam boundary */
const h_state next_h_state[3][6] = {
    /*        ABOVE     BELOW     CROSS */
    /*        L   R     L   R     L   R */
    /* h_state::NH */
    {h_state::BH, h_state::TH, h_state::TH, h_state::BH, h_state::NH,
     h_state::NH},
    /* h_state::BH */
    {h_state::NH, h_state::NH, h_state::NH, h_state::NH, h_state::TH,
     h_state::TH},
    /* h_state::TH */
    {h_state::NH, h_state::NH, h_state::NH, h_state::NH, h_state::BH,
     h_state::BH} //
};

} // namespace gpc
