#ifndef CRASH_GTE_NATIVE_H
#define CRASH_GTE_NATIVE_H

/*
 * The C ports of the hand-written MIPS routines (0x800334A0..0x8003A94C).
 * With -DCRASH_GTE_ORACLE=ON this header is force-included into their
 * sources with GTE_NATIVE_IMPL, so they compile as Native_<name>; the public
 * <name> then runs the translation of the original and the port from the
 * same state and compares them (src/main/engine/gte_oracle.c).
 */

#if defined(CRASH_GTE_ORACLE) && defined(GTE_NATIVE_IMPL)
#define Slst_RDecodeForward Native_Slst_RDecodeForward
#define Slst_RDecodeBackward Native_Slst_RDecodeBackward
#define RMemcpy Native_RMemcpy
#define Gpu_RResetOT Native_Gpu_RResetOT
#define RSqrMagnitude2 Native_RSqrMagnitude2
#define RSqrMagnitude3 Native_RSqrMagnitude3
#define Gte_RTransformSvtx Native_Gte_RTransformSvtx
#define Gte_RTransformCvtx Native_Gte_RTransformCvtx
#define Gte_RProjectBound Native_Gte_RProjectBound
#define Gte_RTransformWorldsLightning Native_Gte_RTransformWorldsLightning
#define Gte_RTransformWorldsDark Native_Gte_RTransformWorldsDark
#define Gte_RTransformWorldsDark2 Native_Gte_RTransformWorldsDark2
#define Gte_RTransformWorlds Native_Gte_RTransformWorlds
#define Gte_RTransformWorldsFog Native_Gte_RTransformWorldsFog
#define Gte_RTransformWorldsRipple Native_Gte_RTransformWorldsRipple
#define Solid_RZoneQueryOctree Native_Solid_RZoneQueryOctree
#define Solid_RPlotQueryWalls Native_Solid_RPlotQueryWalls
#define Solid_RFindFloorY Native_Solid_RFindFloorY
#define Solid_RFindCeilY Native_Solid_RFindCeilY
#define Gte_RTransformSprite Native_Gte_RTransformSprite
#define Gte_RCalcObjectRotMatrix Native_Gte_RCalcObjectRotMatrix
#define Gte_RCalcObjectMatrices Native_Gte_RCalcObjectMatrices
#define Gte_RCalcSpriteRotMatrix Native_Gte_RCalcSpriteRotMatrix
#endif

#endif
