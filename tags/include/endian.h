/*
 *   libmetatag: - A media file tag-reader library
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

#ifndef ENDIAN_H
#define ENDIAN_H 1
#define le2long(le) \
((le[0] << 0) | (le[1] << 8) | (le[2] << 16) | (le[3] << 24) \
(le[4] << 32) | (le[5] << 40) | (le[6] << 48) | (le[7] << 56))
#define le2int(le) \
((le[0] << 0) | (le[1] << 8) | (le[2] << 16) | (le[3] << 24))
#define le2short(le) \
((le[0] << 0) | (le[1] << 8))
#define be2int(be) \
((be[3] << 0) | (be[2] << 8) | (be[1] << 16) | (be[0] << 24))
#define be24int(be) \
((be[2] << 0) | (be[1] << 8) | (be[0] << 16))
#define be2short(be) \
((be[1] << 0) | (be[0] << 8))
#define flac2int(flac) \
((flac[1] << 16) | (flac[2] << 8) | (flac[3] << 0))
#define synchsafe2int(synch) \
((synch[0] << 21) | (synch[1] << 14) | \
(synch[2] << 7)   | (synch[3] << 0))
#define tagid2int(bp) \
((bp[3] << 0) | (bp[2] << 8) | (bp[1] << 16) | (bp[0] << 24))
#endif

