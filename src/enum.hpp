#pragma once

namespace gpc {

enum class gpc_op /* Set operation type                */
{
  GPC_DIFF, /* Difference                        */
  GPC_INT,  /* Intersection                      */
  GPC_XOR,  /* Exclusive or                      */
  GPC_UNION /* Union                             */
};

/*
===========================================================================
                          Private Data Types
===========================================================================
*/

enum vertex_type /* Edge intersection classes         */
{
  NUL, /* Empty non-intersection            */
  EMX, /* External maximum                  */
  ELI, /* External left intermediate        */
  TED, /* Top edge                          */
  ERI, /* External right intermediate       */
  RED, /* Right edge                        */
  IMM, /* Internal maximum and minimum      */
  IMN, /* Internal minimum                  */
  EMN, /* External minimum                  */
  EMM, /* External maximum and minimum      */
  LED, /* Left edge                         */
  ILI, /* Internal left intermediate        */
  BED, /* Bottom edge                       */
  IRI, /* Internal right intermediate       */
  IMX, /* Internal maximum                  */
  FUL  /* Full non-intersection             */
};

// 不改成 enum class，支持隐转
enum h_state /* Horizontal edge states            */
{
  NH, /* No horizontal edge                */
  BH, /* Bottom horizontal edge            */
  TH  /* Top horizontal edge               */
};

enum class bundle_state /* Edge bundle state                 */
{
  UNBUNDLED,   /* Isolated edge not within a bundle */
  BUNDLE_HEAD, /* Bundle head node                  */
  BUNDLE_TAIL  /* Passive bundle tail node          */
};

} // namespace gpc
