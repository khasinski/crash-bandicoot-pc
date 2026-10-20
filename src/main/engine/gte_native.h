#ifndef CRASH_GTE_NATIVE_H
#define CRASH_GTE_NATIVE_H

/*
 * Routines of the hand-written MIPS block (0x800334A0..0x8003A94C) that are
 * ported to C. A routine marked here is defined in C under its own name;
 * the others still run in the translation, gte_render_mips.inc.
 *
 * With -DCRASH_GTE_ORACLE=ON the C implementations are compiled as
 * Native_<name> (this header is force-included into their sources with
 * GTE_NATIVE_IMPL), and every call runs both versions from the same state
 * and compares memory, scratchpad, GTE registers and the result
 * (src/main/engine/gte_oracle.c).
 */

#define GTE_NATIVE_Slst_RDecodeForward 1
#define GTE_NATIVE_Slst_RDecodeBackward 1
#define GTE_NATIVE_RMemcpy 1
#define GTE_NATIVE_Gpu_RResetOT 1
#define GTE_NATIVE_RSqrMagnitude2 1
#define GTE_NATIVE_RSqrMagnitude3 1
#define GTE_NATIVE_Gte_RTransformSvtx 1
#define GTE_NATIVE_Gte_RTransformCvtx 1
#define GTE_NATIVE_Gte_RProjectBound 1
/* #define GTE_NATIVE_Gte_RTransformWorlds 1 */
/* #define GTE_NATIVE_Gte_RTransformWorldsFog 1 */
/* #define GTE_NATIVE_Gte_RTransformWorldsRipple 1 */
#define GTE_NATIVE_Gte_RTransformWorldsLightning 1
#define GTE_NATIVE_Gte_RTransformWorldsDark 1
#define GTE_NATIVE_Gte_RTransformWorldsDark2 1
/* #define GTE_NATIVE_Solid_RZoneQueryOctree 1 */
/* #define GTE_NATIVE_Solid_RPlotQueryWalls 1 */
/* #define GTE_NATIVE_Solid_RFindFloorY 1 */
/* #define GTE_NATIVE_Solid_RFindCeilY 1 */
/* #define GTE_NATIVE_Gte_RCalcObjectRotMatrix 1 */
/* #define GTE_NATIVE_Gte_RCalcObjectMatrices 1 */
/* #define GTE_NATIVE_Gte_RCalcSpriteRotMatrix 1 */
#define GTE_NATIVE_Gte_RTransformSprite 1

#if defined(CRASH_GTE_ORACLE) && defined(GTE_NATIVE_IMPL)
#if GTE_NATIVE_Slst_RDecodeForward
#define Slst_RDecodeForward Native_Slst_RDecodeForward
#endif
#if GTE_NATIVE_Slst_RDecodeBackward
#define Slst_RDecodeBackward Native_Slst_RDecodeBackward
#endif
#if GTE_NATIVE_RMemcpy
#define RMemcpy Native_RMemcpy
#endif
#if GTE_NATIVE_Gpu_RResetOT
#define Gpu_RResetOT Native_Gpu_RResetOT
#endif
#if GTE_NATIVE_RSqrMagnitude2
#define RSqrMagnitude2 Native_RSqrMagnitude2
#endif
#if GTE_NATIVE_RSqrMagnitude3
#define RSqrMagnitude3 Native_RSqrMagnitude3
#endif
#if GTE_NATIVE_Gte_RTransformSvtx
#define Gte_RTransformSvtx Native_Gte_RTransformSvtx
#endif
#if GTE_NATIVE_Gte_RTransformCvtx
#define Gte_RTransformCvtx Native_Gte_RTransformCvtx
#endif
#if GTE_NATIVE_Gte_RProjectBound
#define Gte_RProjectBound Native_Gte_RProjectBound
#endif
#if GTE_NATIVE_Gte_RTransformWorlds
#define Gte_RTransformWorlds Native_Gte_RTransformWorlds
#endif
#if GTE_NATIVE_Gte_RTransformWorldsFog
#define Gte_RTransformWorldsFog Native_Gte_RTransformWorldsFog
#endif
#if GTE_NATIVE_Gte_RTransformWorldsRipple
#define Gte_RTransformWorldsRipple Native_Gte_RTransformWorldsRipple
#endif
#if GTE_NATIVE_Gte_RTransformWorldsLightning
#define Gte_RTransformWorldsLightning Native_Gte_RTransformWorldsLightning
#endif
#if GTE_NATIVE_Gte_RTransformWorldsDark
#define Gte_RTransformWorldsDark Native_Gte_RTransformWorldsDark
#endif
#if GTE_NATIVE_Gte_RTransformWorldsDark2
#define Gte_RTransformWorldsDark2 Native_Gte_RTransformWorldsDark2
#endif
#if GTE_NATIVE_Solid_RZoneQueryOctree
#define Solid_RZoneQueryOctree Native_Solid_RZoneQueryOctree
#endif
#if GTE_NATIVE_Solid_RPlotQueryWalls
#define Solid_RPlotQueryWalls Native_Solid_RPlotQueryWalls
#endif
#if GTE_NATIVE_Solid_RFindFloorY
#define Solid_RFindFloorY Native_Solid_RFindFloorY
#endif
#if GTE_NATIVE_Solid_RFindCeilY
#define Solid_RFindCeilY Native_Solid_RFindCeilY
#endif
#if GTE_NATIVE_Gte_RCalcObjectRotMatrix
#define Gte_RCalcObjectRotMatrix Native_Gte_RCalcObjectRotMatrix
#endif
#if GTE_NATIVE_Gte_RCalcObjectMatrices
#define Gte_RCalcObjectMatrices Native_Gte_RCalcObjectMatrices
#endif
#if GTE_NATIVE_Gte_RCalcSpriteRotMatrix
#define Gte_RCalcSpriteRotMatrix Native_Gte_RCalcSpriteRotMatrix
#endif
#if GTE_NATIVE_Gte_RTransformSprite
#define Gte_RTransformSprite Native_Gte_RTransformSprite
#endif
#endif

#endif
