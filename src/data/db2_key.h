#pragma once

#include "common/db2_settings.h"

struct db2Key
{
    /* whois */
    enum Id : int32_t
    {
        Type = 0x0001, // the type of a dict, if base is specified, it is can be omitted.
        Base = 0x0002, // the base stucture of a dict.
    };

    /* file */
    enum File : int32_t
    {
        FILE = 0x0100, // 256
        PackSize,
        Box2dVer,
        DotBox2dVer,
    };

    /* physics */
    enum World : int32_t
    {
        WORLD = 0x0800, // 2048
        Gravity,
    };
    enum Joint : int32_t
    {
        JOINT = 0x0810, // 2064
        JointType,
    };
    enum Body : int32_t
    {
        BODY = 0x0820, // 2080
        BodyType,
        Position, // (float32_t, float32_t)
        Angle,
        LinearVelocity, // (float32_t, float32_t)
        AngularVelocity,
        LinearDamping,
        AngularDamping,
        AllowSleep,
        Awake,
        FixedRotation,
        Bullet,
        Enabled,
        GravityScale,
    };
    enum Fixture : int32_t
    {
        FIXTURE = 0x0830, // 2096
        Friction,
        Restitution,
        RestitutionThreshold,
        Density,
        IsSensor,
        Filter_CategoryBits,
        Filter_MaskBits,
        Filter_GroupIndex,
    };
    enum Shape : int32_t
    {
        SHAPE = 0x0840, // 2112
        Shape_Type,
    };

    /* script */
    enum Script : int32_t
    {
        SCRIPT = 0x0900, // 2304

        PreStep,
        PostStep,

        LITERAL,
        Scalar,
        Vector,
        Matrix,
        Tensor, // dimension >= 3
        String,

        Variable,

        Object,
        self,
        null,

        Sequence,
        If,

        Get,
        Set,

        Contact,

        // Signal,

        // Float,
        // Int
    };



    enum Math : int32_t
    {
        // constant
        // PI,
        // E,

        // basic
        Add = 0x0A00,
        Sub,
        Mul,
        Div,
        Mod,

        // trigonometric functions
        Sin,
        Cos,
        Tan,
        ASin,
        ACos,
        ATan,

        // exponential and logarithmic functions
        Pow,
        Log, // log(m, n) = log (n) / log (m)

        // round
        Abs,
        Ceil,
        Round,
        Floor,

        // comparison
        Equal,
        Greater,
        GreaterEqual,
        Less,
        LessEqual,

        // vector
        // VAdd,
        // VSub,
        // VMul,
        // VDiv, // ?
        VDot,   // DotProduct
        VCross, // CrossProduct
        VNormalize,
        VNorm,
        VNormSquared,

        MATH_END = VNormSquared,
    };

    /* texture */ // suggestions for renderer and shader
    enum Texture : int32_t
    {

        TEXTURE = 0x1100,
        MPRatio,
        TileShape,
        TileLayout,
        TileOffsetAxis,
        TileSize, // (uint16_t, uint16_t)

        TILE = 0x1800,
        Material,

    };

    enum _to_be_deleted : int32_t
    {
        // physics optional or additional properties
        BreakForce,
        BreakTorque,

    };
};
