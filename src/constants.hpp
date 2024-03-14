#pragma once

#include "enum.hpp"

namespace gpc {

/*
===========================================================================
                         Constants
===========================================================================
*/

/* Increase GPC_EPSILON to encourage merging of near coincident edges    */

const double GPC_EPSILON = DBL_EPSILON;

const std::string GPC_VERSION = "2.33";

/*
===========================================================================
                               Global Data
===========================================================================
*/

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
