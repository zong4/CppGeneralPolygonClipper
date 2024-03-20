#pragma once

namespace gpc {

// Set operation type
enum class gpc_op
{
    GPC_DIFF, // Difference
    GPC_INT,  // Intersection
    GPC_XOR,  // Exclusive or
    GPC_UNION // Union
};

// Edge intersection classes
// bl br tl tr
enum class vertex_type : int
{
    NUL, // 空的非交点 0000
    EMX, // 外部最大值 0001
    ELI, // 外部左中间值 0010
    TED, // 顶边 0011
    ERI, // 外部右中间值 0100
    RED, // 右边 0101
    IMM, // 内部最大值和最小值 0110
    IMN, // 内部最小值 0111
    EMN, // 外部最小值 1000
    EMM, // 外部最大值和最小值 1001
    LED, // 左边 1010
    ILI, // 内部左中间值 1011
    BED, // 底边 1100
    IRI, // 内部右中间值 1101
    IMX, // 内部最大值 1110
    FUL  // 完全非交点 1111
};

// 不改成 enum class，支持隐转
// Horizontal edge states
enum h_state
{
    NH, // No horizontal edge
    BH, // Bottom horizontal edge
    TH  // Top horizontal edge
};

// Edge bundle state
enum class bundle_state
{
    UNBUNDLED,   // Isolated edge not within a bundle
    BUNDLE_HEAD, // Bundle head node
    BUNDLE_TAIL  // Passive bundle tail node
};

} // namespace gpc
