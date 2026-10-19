#ifndef CRASH_INCLUDE_ASM_H
#define CRASH_INCLUDE_ASM_H

/*
 * HANDWRITTEN_ASM(FOLDER, NAME) marks a hand-written MIPS routine that has not
 * been translated to C yet. In the decompilation it pulled the routine from
 * FOLDER/NAME.s. The native build defines nothing for it, so any caller fails
 * to link until the routine is translated.
 */
#define HANDWRITTEN_ASM(FOLDER, NAME)

#endif
