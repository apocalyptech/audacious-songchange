/*
 *   libmetatag - A media file tag-reader library
 *   Copyright (C) 2003, 2004  Pipian
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef ID3V2_H
#define ID3V2_H 1

typedef struct
{
	int frameid, len;
	unsigned char *data, *flags;
} framedata_t;

typedef struct
{
	int numitems, version;
	framedata_t **items;
} id3v2_t;

typedef struct
{
	char 	*frameid;
	int	code;
} id3_lookup_t;

enum
{
	ID3v22,
	ID3v23,
	ID3v24
};

enum 
{
	ID3V24_AENC, ID3V24_APIC, ID3V24_ASPI, ID3V24_COMM, ID3V24_COMR, 
	ID3V24_ENCR, ID3V24_EQU2, ID3V24_ETCO, ID3V24_GEOB, ID3V24_GRID, 
	ID3V24_LINK, ID3V24_MCDI, ID3V24_MLLT, ID3V24_OWNE, ID3V24_PRIV, 
	ID3V24_PCNT, ID3V24_POPM, ID3V24_POSS, ID3V24_RBUF, ID3V24_RVA2, 
	ID3V24_RVRB, ID3V24_SEEK, ID3V24_SIGN, ID3V24_SYLT, ID3V24_SYTC, 
	ID3V24_TALB, ID3V24_TBPM, ID3V24_TCOM, ID3V24_TCON, ID3V24_TCOP, 
	ID3V24_TDEN, ID3V24_TDLY, ID3V24_TDOR, ID3V24_TDRC, ID3V24_TDRL, 
	ID3V24_TDTG, ID3V24_TENC, ID3V24_TEXT, ID3V24_TFLT, ID3V24_TIPL, 
	ID3V24_TIT1, ID3V24_TIT2, ID3V24_TIT3, ID3V24_TKEY, ID3V24_TLAN, 
	ID3V24_TLEN, ID3V24_TMCL, ID3V24_TMED, ID3V24_TMOO, ID3V24_TOAL, 
	ID3V24_TOFN, ID3V24_TOLY, ID3V24_TOPE, ID3V24_TOWN, ID3V24_TPE1, 
	ID3V24_TPE2, ID3V24_TPE3, ID3V24_TPE4, ID3V24_TPOS, ID3V24_TPRO, 
	ID3V24_TPUB, ID3V24_TRCK, ID3V24_TRSN, ID3V24_TRSO, ID3V24_TSOA, 
	ID3V24_TSOP, ID3V24_TSOT, ID3V24_TSRC, ID3V24_TSSE, ID3V24_TSST, 
	ID3V24_TXXX, ID3V24_UFID, ID3V24_USER, ID3V24_USLT, ID3V24_WCOM, 
	ID3V24_WCOP, ID3V24_WOAF, ID3V24_WOAR, ID3V24_WOAS, ID3V24_WORS, 
	ID3V24_WPAY, ID3V24_WPUB, ID3V24_WXXX
};

enum
{
	ID3V22_BUF, ID3V22_CNT, ID3V22_COM, ID3V22_CRA, ID3V22_CRM, ID3V22_ETC, 
	ID3V22_EQU, ID3V22_GEO, ID3V22_IPL, ID3V22_LNK, ID3V22_MCI, ID3V22_MLL, 
	ID3V22_PIC, ID3V22_POP, ID3V22_REV, ID3V22_RVA, ID3V22_SLT, ID3V22_STC, 
	ID3V22_TAL, ID3V22_TBP, ID3V22_TCM, ID3V22_TCO, ID3V22_TCR, ID3V22_TDA, 
	ID3V22_TDY, ID3V22_TEN, ID3V22_TFT, ID3V22_TIM, ID3V22_TKE, ID3V22_TLA, 
	ID3V22_TLE, ID3V22_TMT, ID3V22_TOA, ID3V22_TOF, ID3V22_TOL, ID3V22_TOR, 
	ID3V22_TOT, ID3V22_TP1, ID3V22_TP2, ID3V22_TP3, ID3V22_TP4, ID3V22_TPA, 
	ID3V22_TPB, ID3V22_TRC, ID3V22_TRD, ID3V22_TRK, ID3V22_TSI, ID3V22_TSS, 
	ID3V22_TT1, ID3V22_TT2, ID3V22_TT3, ID3V22_TXT, ID3V22_TXX, ID3V22_TYE, 
	ID3V22_UFI, ID3V22_ULT, ID3V22_WAF, ID3V22_WAR, ID3V22_WAS, ID3V22_WCM, 
	ID3V22_WCP, ID3V22_WPB, ID3V22_WXX 
};

enum
{
	ID3V23_AENC, ID3V23_APIC, ID3V23_COMM, ID3V23_COMR, ID3V23_ENCR, 
	ID3V23_EQUA, ID3V23_ETCO, ID3V23_GEOB, ID3V23_GRID, ID3V23_IPLS, 
	ID3V23_LINK, ID3V23_MCDI, ID3V23_MLLT, ID3V23_OWNE, ID3V23_PRIV, 
	ID3V23_PCNT, ID3V23_POPM, ID3V23_POSS, ID3V23_RBUF, ID3V23_RVAD, 
	ID3V23_RVRB, ID3V23_SYLT, ID3V23_SYTC, ID3V23_TALB, ID3V23_TBPM, 
	ID3V23_TCOM, ID3V23_TCON, ID3V23_TCOP, ID3V23_TDAT, ID3V23_TDLY, 
	ID3V23_TENC, ID3V23_TEXT, ID3V23_TFLT, ID3V23_TIME, ID3V23_TIT1, 
	ID3V23_TIT2, ID3V23_TIT3, ID3V23_TKEY, ID3V23_TLAN, ID3V23_TLEN, 
	ID3V23_TMED, ID3V23_TOAL, ID3V23_TOFN, ID3V23_TOLY, ID3V23_TOPE, 
	ID3V23_TORY, ID3V23_TOWN, ID3V23_TPE1, ID3V23_TPE2, ID3V23_TPE3, 
	ID3V23_TPE4, ID3V23_TPOS, ID3V23_TPUB, ID3V23_TRCK, ID3V23_TRDA, 
	ID3V23_TRSN, ID3V23_TRSO, ID3V23_TSIZ, ID3V23_TSRC, ID3V23_TSSE, 
	ID3V23_TYER, ID3V23_TXXX, ID3V23_UFID, ID3V23_USER, ID3V23_USLT, 
	ID3V23_WCOM, ID3V23_WCOP, ID3V23_WOAF, ID3V23_WOAR, ID3V23_WOAS, 
	ID3V23_WORS, ID3V23_WPAY, ID3V23_WPUB, ID3V23_WXXX 
};


unsigned char *ID3v2_parseText(framedata_t *);
unsigned char *ID3v2_getData(framedata_t *);
int findID3v2(FILE *);
id3v2_t *readID3v2(char *);
void freeID3v2(id3v2_t *);
#endif