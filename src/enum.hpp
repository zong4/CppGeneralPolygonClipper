#pragma once

namespace gpc {

enum class gpc_op /* Set operation type                */
{
  GPC_DIFF, /* Difference                        */
  GPC_INT,  /* Intersection                      */
  GPC_XOR,  /* Exclusive or                      */
  GPC_UNION /* Union                             */
};

} // namespace gpc
