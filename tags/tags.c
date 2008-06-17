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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/fmt.h"
#include "../config.h"
#include "include/unicode.h"
#include "include/tags.h"
#include "include/endian.h"

void tag_exists(metatag_t *meta, char *filename)
{
	FILE *fp;
	int status = 0;

	fp = fopen(filename, "r");
	if(fp == NULL)
	{
		pdebug("Error reading file...", META_DEBUG);

		return;
	}

	/* Check for ID3v1 */
	fseek(fp, -128, SEEK_END);
	if(findID3v1(fp))
	{
		pdebug("Found ID3v1 tag...", META_DEBUG);
		meta->has_id3v1 = 1;
	}

	/* Check for ID3v2 */
	fseek(fp, 0, SEEK_SET);
	status = findID3v2(fp);
	if(status > -1)
	{
		pdebug("Found ID3v2 tag...", META_DEBUG);
		meta->has_id3v2 = 1;
	}
	status = 0;

	/* Check for Ogg Vorbis */
	fseek(fp, 0, SEEK_SET);
	status = findVorbis(fp);
	if(status > -1)
	{
		pdebug("Found Vorbis comment block...", META_DEBUG);
		meta->has_vorbis = 1;
	}
	status = 0;

	/* Check for FLAC */
	fseek(fp, 0, SEEK_SET);
	if(findFlac(fp))
	{
		pdebug("Found FLAC tag...", META_DEBUG);
		meta->has_flac = 1;
	}
	
	/* Check for OggFLAC */
	fseek(fp, 0, SEEK_SET);
	status = findOggFlac(fp);
	if(status > -1)
	{
		pdebug("Found OggFLAC...", META_DEBUG);
		meta->has_oggflac = 1;
	}
	status = 0;

	/* Check for Speex */
	fseek(fp, 0, SEEK_SET);
	status = findSpeex(fp);
	if(status > -1)
	{
		pdebug("Found Speex...", META_DEBUG);
		meta->has_speex = 1;
	}
	status = 0;

	/* Check for APE */
	fseek(fp, 0, SEEK_SET);
	status = findAPE(fp);
	if(status > 0)
	{
		pdebug("Found APE tag...", META_DEBUG);
		meta->has_ape = 1;
	}
	status = 0;
	
	/* Check for iTunes M4A */
	fseek(fp, 0, SEEK_SET);
	status = findiTunes(fp);
	if(status > -1)
	{
		pdebug("Found iTunes tag...", META_DEBUG);
		meta->has_itunes = 1;
	}

	/* Check for the devil tag: WMA */
	fseek(fp, 0, SEEK_SET);
	status = findWMA(fp);
	if(status > -1)
	{
		pdebug("Found WMA tag...", META_DEBUG);
		meta->has_wma = 1;
	}

	fclose(fp);

	return;
}

static ape_t *fetchAPE(char *filename)
{
	ape_t *ape;

	pdebug("Getting APE tag metadata...", META_DEBUG);
	ape = readAPE(filename);

	return ape;
} /* End APE read */

void metaAPE(metatag_t *meta)
{
	int i, t = 0;
	ape_t *ape = meta->ape;
	
	for(i = 0; i < ape->numitems; i++)
	{
		apefielddata_t *item = ape->items[i];
		
		if(!strcmp(item->name, "Title"))
		{
			pdebug("Found Title!", META_DEBUG);
			meta->title = item->data;
		}
		else if(!strcmp(item->name, "Artist"))
		{
			pdebug("Found Artist!", META_DEBUG);
			meta->artist = item->data;
		}
		else if(strcmp(item->name, "Album") == 0)
		{
			pdebug("Found Album!", META_DEBUG);
			meta->album = item->data;
		}
		else if(strcmp(item->name, "Year") == 0)
		{
			pdebug("Found Year!", META_DEBUG);
			meta->year = item->data;
		}
		else if(strcmp(item->name, "Genre") == 0)
		{
			pdebug("Found Genre!", META_DEBUG);
			meta->genre = realloc(meta->genre, strlen(item->data)
				+ 1);
			strcpy(meta->genre, item->data);
		}
		else if(strcmp(item->name, "Track") == 0)
		{
			pdebug("Found Track!", META_DEBUG);
			meta->track = realloc(meta->track, strlen(item->data)
				+ 1);
			strcpy(meta->track, item->data);
		}
		else if(strcmp(item->name, "Comment") == 0)
		{
			unsigned char *comment_value = NULL, *eq_pos, *sep_pos,
					*subvalue;

			subvalue = item->data;
			sep_pos = strchr(subvalue, '|');
			while(sep_pos != NULL || t == 0)
			{
				t = 1;
				if(sep_pos != NULL)
					*sep_pos = '\0';
				comment_value = realloc(comment_value,
							strlen(subvalue) + 1);
				strcpy(comment_value, subvalue);
				if(sep_pos != NULL)
					sep_pos++;
				eq_pos = strchr(comment_value, '=');
				if(eq_pos != NULL)
				{
					*eq_pos = '\0';
					eq_pos++;
					if(!strcmp(comment_value,
						"musicbrainz_trackid"))
					{
						/* ??? */
						pdebug("Found MusicBrainz Track ID!", META_DEBUG);
						meta->mb = realloc(meta->mb,
							strlen(eq_pos) + 1);
						strcpy(meta->mb, eq_pos);
						break;
					}
				}
				if(sep_pos != NULL)
				{
					t = 0;
					subvalue = sep_pos;
					sep_pos = strchr(subvalue, '|');
				}
			}
			t = 0;
			if(comment_value != NULL)
				free(comment_value);
		}
	}
} /* End APE parsing */

static vorbis_t *fetchVorbis(char *filename, int tag_type)
{
	vorbis_t *comments = NULL;

	/* Several slightly different methods of getting the data... */
	if(tag_type == READ_VORBIS)
	{
		pdebug("Getting Vorbis comment metadata...", META_DEBUG);
		comments = readVorbis(filename);
	}
	else if(tag_type == READ_FLAC)
	{
		pdebug("Getting FLAC tag metadata...", META_DEBUG);
		comments = readFlac(filename);
	}
	else if(tag_type == READ_OGGFLAC)
	{
		pdebug("Getting OggFLAC tag metadata...", META_DEBUG);
		comments = readOggFlac(filename);
	}
	else if(tag_type == READ_SPEEX)
	{
		pdebug("Getting Speex tag metadata...", META_DEBUG);
		comments = readSpeex(filename);
	}
	if(comments == NULL)
		pdebug("Error in Vorbis Comment read", META_DEBUG);
	
	return comments;
} /* End Vorbis/FLAC/OggFLAC/Speex read */

void metaVorbis(metatag_t *meta)
{
	int i;
	vorbis_t *comments = NULL;
	
	/* I'm not expecting more than one vorbis tag */
	if(meta->has_vorbis)
		comments = meta->vorbis;
	else if(meta->has_flac)
		comments = meta->flac;
	else if(meta->has_oggflac)
		comments = meta->oggflac;
	else if(meta->has_speex)
		comments = meta->speex;
	if(comments == NULL)
		return;
	
	for(i = 0; i < comments->numitems; i++)
	{
		vorbisfielddata_t *field = comments->items[i];
		if(!fmt_strcasecmp(field->name, "TITLE"))
		{
			pdebug("Found Title!", META_DEBUG);
			
			meta->title = field->data;
		}
		/* Prefer PERFORMER to ARTIST */
        /* or.... NOT.  pez - 2008.06.17 */
		else if(!fmt_strcasecmp(field->name, "ARTIST"))
		{
			pdebug("Found Artist!", META_DEBUG);

			meta->artist = field->data;
		}
		else if(!fmt_strcasecmp(field->name, "PERFORMER") && meta->artist == NULL)
		{
			pdebug("Found Artist!", META_DEBUG);

			meta->artist = field->data;
		}
		else if(!fmt_strcasecmp(field->name, "ALBUM"))
		{
			pdebug("Found Album!", META_DEBUG);
			
			meta->album = field->data;
		}
		else if(!fmt_strcasecmp(field->name, "MUSICBRAINZ_TRACKID"))
		{
			pdebug("Found MusicBrainz Track ID!", META_DEBUG);
			
			meta->mb = realloc(meta->mb, strlen(field->data) + 1);
			memset(meta->mb, '\0', strlen(field->data) + 1);
			memcpy(meta->mb, field->data, strlen(field->data));
		}
		else if(!fmt_strcasecmp(field->name, "GENRE"))
		{
			pdebug("Found Genre!", META_DEBUG);
			
			meta->genre = realloc(meta->genre, strlen(field->data)
				+ 1);
			memset(meta->genre, '\0', strlen(field->data) + 1);
			memcpy(meta->genre, field->data, strlen(field->data));
		}
		else if(!fmt_strcasecmp(field->name, "TRACKNUMBER"))
		{
			pdebug("Found Track!", META_DEBUG);
			
			meta->track = realloc(meta->track, strlen(field->data)
				+ 1);
			memset(meta->track, '\0', strlen(field->data) + 1);
			memcpy(meta->track, field->data, strlen(field->data));
		}
	}
	
	return;
} /* End Vorbis/FLAC/OggFLAC/Speex parsing */

static id3v2_t *fetchID3v2(char *filename)
{
	id3v2_t *id3v2;

	pdebug("Getting ID3v2 tag metadata...", META_DEBUG);
	id3v2 = readID3v2(filename);
	if(id3v2 == NULL)
		pdebug("Error in readID3v2()", META_DEBUG);

	return id3v2;
} /* End ID3v2 read */

void metaID3v2(metatag_t *meta)
{
	int i;
	id3v2_t *id3v2 = meta->id3v2;
	
	for(i = 0; i < id3v2->numitems; i++)
	{
		unsigned char *data = NULL, *utf = NULL;
		framedata_t *frame = id3v2->items[i];
		if(	(id3v2->version == 2 && frame->frameid == ID3V22_TT2) ||
			(id3v2->version == 3 && frame->frameid == ID3V23_TIT2) ||
			(id3v2->version == 4 && frame->frameid == ID3V24_TIT2))
		{
			pdebug("Found Title!", META_DEBUG);
			
			meta->title = frame->data;
		}
		else if((id3v2->version == 2 && frame->frameid == ID3V22_TP1) ||
			(id3v2->version == 3 && frame->frameid == ID3V23_TPE1) ||
			(id3v2->version == 4 && frame->frameid == ID3V24_TPE1))
		{
			pdebug("Found Artist!", META_DEBUG);

			meta->artist = frame->data;
		}
		else if((id3v2->version == 2 && frame->frameid == ID3V22_TAL) ||
			(id3v2->version == 3 && frame->frameid == ID3V23_TALB) ||
			(id3v2->version == 4 && frame->frameid == ID3V24_TALB))
		{
			pdebug("Found Album!", META_DEBUG);
			
			meta->album = frame->data;
		}
		/* No strict year for ID3v2.4 */
		else if((id3v2->version == 2 && frame->frameid == ID3V22_TYE) ||
			(id3v2->version == 3 && frame->frameid == ID3V23_TYER))
		{
			pdebug("Found Year!", META_DEBUG);
			
			meta->year = frame->data;
		}
		/* Won't translate ID3v1 genres  yet */
		else if((id3v2->version == 2 && frame->frameid == ID3V22_TCO) ||
			(id3v2->version == 3 && frame->frameid == ID3V23_TCON) ||
			(id3v2->version == 4 && frame->frameid == ID3V24_TCON))
		{
			pdebug("Found Genre!", META_DEBUG);
			
			meta->genre = realloc(meta->genre, frame->len);
			memset(meta->genre, '\0', frame->len);
			memcpy(meta->genre, frame->data, frame->len);
		}
		else if((id3v2->version == 2 && frame->frameid == ID3V22_TRK) ||
			(id3v2->version == 3 && frame->frameid == ID3V23_TRCK) ||
			(id3v2->version == 4 && frame->frameid == ID3V24_TRCK))
		{
			pdebug("Found Track!", META_DEBUG);
			
			meta->track = realloc(meta->track, frame->len);
			memset(meta->track, '\0', frame->len);
			memcpy(meta->track, frame->data, frame->len);
		}
		else if((id3v2->version == 2 && frame->frameid == ID3V22_UFI) ||
			(id3v2->version == 3 && frame->frameid == ID3V23_UFID) ||
			(id3v2->version == 4 && frame->frameid == ID3V24_UFID))
		{
			data = frame->data;
			
			if(!strcmp(data, "http://musicbrainz.org"))
			{
				pdebug("Found MusicBrainz Track ID!", META_DEBUG);
				
				utf = data + 23;

				meta->mb = realloc(meta->mb, frame->len - 22);
				memcpy(meta->mb, utf, frame->len - 23);
				*(meta->mb + frame->len - 23) = 0;
			}
		}
	}
} /* End ID3v2 parsing */

static itunes_t *fetchiTunes(char *filename)
{
	itunes_t *itunes;
	
	pdebug("Getting iTunes tag metadata...", META_DEBUG);
	itunes = readiTunes(filename);

	return itunes;
} /* End M4A read */

void metaiTunes(metatag_t *meta)
{
	itunes_t *itunes = meta->itunes;
	int tmp;
	
	if(itunes->title != NULL)
	{
		pdebug("Found Title!", META_DEBUG);
			
		meta->title = itunes->title;
	}
	if(itunes->artist != NULL)
	{
		pdebug("Found Artist!", META_DEBUG);
			
		meta->artist = itunes->artist;
	}
	if(itunes->album != NULL)
	{
		pdebug("Found Album!", META_DEBUG);
			
		meta->album = itunes->album;
	}
	/* I don't read genre (yet) and I won't read max track number. */
	if(itunes->track > 0 && itunes->track != 255)
	{
		pdebug("Found Track!", META_DEBUG);
		
		meta->track = realloc(meta->track, 4);
		tmp = snprintf(meta->track, 3, "%d", itunes->track);
		*(meta->track + tmp) = '\0';
	}
	if(itunes->year != NULL)
	{
		pdebug("Found Year!", META_DEBUG);
		
		meta->year = itunes->year;
	}
} /* End M4A parsing */

static wma_t *fetchWMA(char *filename)
{
	wma_t *wma;

	pdebug("Getting WMA metadata...", META_DEBUG);
	wma = readWMA(filename);

	return wma;
} /* End WMA read */

void metaWMA(metatag_t *meta)
{
	wma_t *wma = meta->wma;
	int i, tmp;

	for(i = 0; i < wma->numitems; i++)
	{
		attribute_t *attribute = wma->items[i];
		
		if(!strcmp(attribute->name, "Title"))
		{
			pdebug("Found Title!", META_DEBUG);

			meta->title = attribute->data;
		}
		else if(!strcmp(attribute->name, "Author"))
		{
			pdebug("Found Artist!", META_DEBUG);

			meta->artist = attribute->data;
		}
		else if(!strcmp(attribute->name, "WM/AlbumTitle"))
		{
			pdebug("Found Album!", META_DEBUG);

			meta->album = attribute->data;
		}
		else if(!strcmp(attribute->name, "WM/Year"))
		{
			pdebug("Found Year!", META_DEBUG);

			meta->year = attribute->data;
		}
		else if(!strcmp(attribute->name, "WM/Genre"))
		{
			pdebug("Found Genre!", META_DEBUG);

			meta->genre = realloc(meta->genre,
						strlen(attribute->data) + 1);
			strcpy(meta->genre, attribute->data);
		}
		else if(!strcmp(attribute->name, "WM/TrackNumber"))
		{
			pdebug("Found Track!", META_DEBUG);

			meta->track = realloc(meta->track, 4);
			tmp = snprintf(meta->track, 3, "%d",
					le2int(attribute->data));
			*(meta->track + tmp) = '\0';
		}
	}
}

static id3v1_t *fetchID3v1(char *filename)
{
	id3v1_t *id3v1;
	
	pdebug("Getting ID3v1 tag metadata...", META_DEBUG);
	id3v1 = readID3v1(filename);

	return id3v1;
} /* End ID3v1 read */

void metaID3v1(metatag_t *meta)
{
	int tmp;
	id3v1_t *id3v1 = meta->id3v1;
	
	if(id3v1->title != NULL)
	{
		pdebug("Found Title!", META_DEBUG);
			
		meta->title = id3v1->title;
	}
	if(id3v1->artist != NULL)
	{
		pdebug("Found Artist!", META_DEBUG);
			
		meta->artist = id3v1->artist;
	}
	if(id3v1->album != NULL)
	{
		pdebug("Found Album!", META_DEBUG);
			
		meta->album = id3v1->album;
	}
	if(id3v1->year != NULL)
	{
		pdebug("Found Year!", META_DEBUG);
		
		meta->year = id3v1->year;
	}
	if(id3v1->track != 255)
	{
		pdebug("Found Track!", META_DEBUG);
		
		meta->track = realloc(meta->track, 4);
		tmp = snprintf(meta->track, 3, "%d", id3v1->track);
		*(meta->track + tmp) = '\0';
	}
	/* I assume unassigned genre's are 255 */
	if(id3v1->genre != 255)
	{
		pdebug("Found Genre!", META_DEBUG);
		
		meta->genre = realloc(meta->genre,
			strlen(genre_list[id3v1->genre]) + 1);
		strcpy(meta->genre, genre_list[id3v1->genre]);
	}
	/*
	 * This next one is unofficial, but maybe someone will use it.
	 * I don't think anyone's going to trigger this by accident.
	 *
	 * Specification:
	 * In comment field (ID3v1 or ID3v1.1):
	 * 1 byte: \0
	 * 10 bytes: "MBTRACKID=" identifier
	 * 16 bytes: binary representation of Track ID
	 * 1 byte: \0
	 *
	 * e.g. for "Missing" on Evanescence's "Bring Me to Life" single
	 *
	 * Track ID = 9c2567cb-4a8b-4096-b105-dada6b95a08b
	 *
	 * Therefore, comment field would equal:
	 * "MBID: \156%g�J\139@\150�\005��k\149\160\139"
	 * in ISO 8859-1 (with three digit escape characters)
	 */
	if(!strncmp((id3v1->comment) + 1, "MBTRACKID=", 10))
	{
		pdebug("Found MusicBrainz Track ID!", META_DEBUG);
		
		meta->mb = realloc(meta->mb, 37);
		tmp = sprintf(meta->mb,
	"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
			id3v1->comment[11],
			id3v1->comment[12],
			id3v1->comment[13],
			id3v1->comment[14],
			id3v1->comment[15],
			id3v1->comment[16],
			id3v1->comment[17],
			id3v1->comment[18],
			id3v1->comment[19],
			id3v1->comment[20],
			id3v1->comment[21],
			id3v1->comment[22],
			id3v1->comment[23],
			id3v1->comment[24],
			id3v1->comment[25],
			id3v1->comment[26]);
		*(meta->mb + tmp) = '\0';
	}
} /* End ID3v1 parsing */

void get_tag_data(metatag_t *meta, char *filename, int track)
{
	if(track > 0)
	{
		return;
	}
	else
	{
		tag_exists(meta, filename);
		if(meta->has_id3v1)
			meta->id3v1 = fetchID3v1(filename);
		if(meta->has_id3v2)
			meta->id3v2 = fetchID3v2(filename);
		if(meta->has_ape)
			meta->ape = fetchAPE(filename);
		if(meta->has_vorbis)
			meta->vorbis = fetchVorbis(filename, READ_VORBIS);
		if(meta->has_flac)
			meta->flac = fetchVorbis(filename, READ_FLAC);
		if(meta->has_oggflac)
			meta->oggflac = fetchVorbis(filename, READ_OGGFLAC);
		if(meta->has_speex)
			meta->speex = fetchVorbis(filename, READ_SPEEX);
		if(meta->has_itunes)
			meta->itunes = fetchiTunes(filename);
		if(meta->has_wma)
			meta->wma = fetchWMA(filename);
	}
	
	/*
	 * This order is rather arbitrary, but puts tags you'd EXPECT to
	 * be the exclusive tag in the file first (i.e. vorbis and itunes).
	 * Thus we return afterwards.
	 */
	
	if(meta->has_vorbis || meta->has_flac || meta->has_oggflac ||
		meta->has_speex)
	{
		metaVorbis(meta);
		
		return;
	}
	else if(meta->has_itunes)
	{
		metaiTunes(meta);
		
		return;
	}
	else if(meta->has_wma)
	{
		metaWMA(meta);

		return;
	}
	/*
	 * OK, here's the trick: APE preferred to ID3v2?
	 * or ID3v2 preferred to APE?  ID3v1 loses regardless.
	 * Thus it's put first to be overwritten.
	 */
	if(meta->has_id3v1)
		metaID3v1(meta);
	/* A little dirty for now, but it's not too difficult */
	if(!meta->prefer_ape)
	{
		if(meta->has_ape)
			metaAPE(meta);
		if(meta->has_id3v2)
			metaID3v2(meta);
	}
	else if(meta->prefer_ape)
	{
		if(meta->has_id3v2)
			metaID3v2(meta);
		if(meta->has_ape)
			metaAPE(meta);
	}
	
	return;
}

void metatag_delete(metatag_t *meta)
{
	/*
	 * Genre, track, and MBID are the exceptions and must be freed
	 * (Thanks ID3v1)
	 */
	if(meta->track != NULL)
		free(meta->track);
	if(meta->genre != NULL)
		free(meta->genre);
	if(meta->mb != NULL)
		free(meta->mb);
	if(meta->wma != NULL)
		freeWMA(meta->wma);
	if(meta->id3v1 != NULL)
		freeID3v1(meta->id3v1);
	if(meta->id3v2 != NULL)
		freeID3v2(meta->id3v2);
	if(meta->ape != NULL)
		freeAPE(meta->ape);
	if(meta->vorbis != NULL)
		freeVorbis(meta->vorbis);
	if(meta->flac != NULL)
		freeVorbis(meta->flac);
	if(meta->oggflac != NULL)
		freeVorbis(meta->oggflac);
	if(meta->speex != NULL)
		freeVorbis(meta->speex);
	if(meta->itunes != NULL)
		freeiTunes(meta->itunes);
	free(meta);
}

metatag_t *metatag_new(void)
{
	metatag_t *ret;
	
	if(!(ret = calloc(sizeof(*ret), 1)))
		return NULL;
#ifdef PREFER_APE
	ret->prefer_ape = 1;
#endif
	
	return ret;
}
