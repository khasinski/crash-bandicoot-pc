/*
 * PsyQ libsnd calls the game uses that PsyZ ships only as unported assembly.
 * Written against PsyZ's libsnd state from the reconstructed PsyQ code in
 * src/main/sdk/libspu (sequence_channel_resume.c, sep_replay.c, ...).
 */
#include "libsnd_private.h"

/* Current volume of a sequence, packed as sep | seq << 8. PsyQ also records
 * the ID as the VM's current sequence; nothing here reads that back. */
s32 SpuVmGetSeqVol(s16 seq_sep, s16 *voll, s16 *volr)
{
    struct SeqStruct *score = &_ss_score[seq_sep & 0xFF][(seq_sep >> 8) & 0xFF];
    *voll = (s16)score->voll;
    *volr = (s16)score->volr;
    return seq_sep;
}

static void sep_set_pause_mode(short sep, short seq)
{
    struct SeqStruct *score = &_ss_score[sep][seq];
    SpuVmGetSeqVol((short)((seq << 8) | sep), (s16 *)&score->voll, (s16 *)&score->volr);
    score->flags &= ~SEQ_FLAG_1;
    score->flags &= ~SEQ_FLAG_8;
    score->flags |= SEQ_FLAG_2;
}

static void sep_set_replay_mode(short sep, short seq)
{
    struct SeqStruct *score = &_ss_score[sep][seq];
    unsigned int flags = score->flags;
    if (!(flags & 0x204) && !(flags & SEQ_FLAG_100)) {
        score->flags = flags & ~SEQ_FLAG_2;
        score->flags |= SEQ_FLAG_8;
        score->flags |= SEQ_FLAG_1;
    }
}

void SsSepPause(short sep_access_num, short seq_num)
{
    sep_set_pause_mode(sep_access_num, seq_num);
}

void SsSepReplay(short sep_access_num, short seq_num)
{
    sep_set_replay_mode(sep_access_num, seq_num);
}

void SsSepSetDecrescendo(short sep_access_num, short seq_num, short vol, long v_time)
{
    _SsSndSetVolData(sep_access_num, seq_num, vol, (int)v_time);
    _ss_score[sep_access_num][seq_num].flags |= SEQ_FLAG_20;
    _ss_score[sep_access_num][seq_num].flags &= ~SEQ_FLAG_10;
}

/* The VAB body was transferred by the caller: mark the bank ready. */
short SsVabFakeBody(short vabid)
{
    if ((unsigned short)vabid <= 16 && _svm_vab_used[vabid] == 2) {
        _spu_setInTransfer(0);
        _svm_vab_used[vabid] = 1;
        return vabid;
    }
    return -1;
}
