
#include "gup.h"

#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#ifdef HAVE_IO_H
#include <io.h>
#elif defined(HAVE_UNISTD_H)
#include <unistd.h>
#else
#error need some #include for io functions
#endif

#if (OS == OS_WIN32)
#include <windows.h>
#endif

#include "gup_err.h"
#include "options.h"
#include "arj.h"
#include "crc.h"
#include "compr_io.h"
#include "compress.h"
#include "arcman.h"
#include "utils.h"
#include "scan.h"

#ifdef O_BINARY
#define OPEN_RD_FLAGS	(O_RDONLY | O_BINARY)
#define OPEN_WR_FLAGS	(O_WRONLY | O_CREAT | O_TRUNC | O_BINARY)
#else
#define OPEN_RD_FLAGS	O_RDONLY
#define OPEN_WR_FLAGS	(O_WRONLY | O_CREAT | O_TRUNC)
#endif

/*
 * Pack a single file
 *
 * handle   - handle of arj file
 * the_name - name of file to be packed
 * opts     - misc options
 * returns <0 if error
 */

gup_result pack_file(archive *archive, const char *the_name, const osstat *st, OPTIONS *opts)
{
	int source_file;
	unsigned long origsize;
	const char *stored_name;
	char *linkname;
	gup_result result;
	fileheader *header;
	int file_types;

	if ((result = archive->arcctl(ARC_FILE_TYPES, &file_types)) != GUP_OK)
		return result;

	switch(gup_file_type(st))
	{
	case BINARY_TYPE:
	case TEXT_TYPE:
		if (!(file_types & ARC_FT_REG_FILE))
		{
			printf("%30.30s - Regular files are not supported by this archive type, skipping.\n", the_name);
			return GUP_OK;
		}
		break;

	case DIR_TYPE:
		if (!(file_types & ARC_FT_DIR))
		{
			printf("%30.30s - Directories are not supported by this archive type, skipping.\n", the_name);
			return GUP_OK;
		}
		break;

	case SYMLINK_TYPE:
		if (!(file_types & ARC_FT_SYMLINK))
		{
			printf("%30.30s - Symbolic links are not supported by this archive type, skipping.\n", the_name);
			return GUP_OK;
		}
		break;

	case LABEL_TYPE:
	default:
		return GUP_OK;
	}

	{
		char buf[81];
		printf("     %s\r", pnprint(74, the_name, buf));
	}
	
	if(opts->exclude_paths)				/* store in arj header with path or not */
	{
		char *s = strrchr((char *) the_name, DELIM);
		stored_name = s ? s+1 : the_name;
	}
	else
		stored_name = (the_name[0] == DELIM) ? the_name + 1 : the_name;	/* Always store relative path. */

	header = archive->init_file_header(stored_name, NULL, st);

	switch(header->file_type)
	{
	case BINARY_TYPE:
	case TEXT_TYPE:
		if ((source_file = open(the_name, OPEN_RD_FLAGS, 0666)) == -1)
		{
			printf("Source file open error\n");
			return gup_conv_err(errno);
		}

		origsize = st->length;
		init_progress(origsize);

		if(opts->mv_mode)
		{
			int segment_cnt = 0;
			unsigned long hdr_len;		/* Length of file header. */

			do
			{
				header->method = opts->mode;
				header->mv_set_segment_cnt(segment_cnt);
				hdr_len = header->get_header_len();	/* After mv_set_segment_cnt(). */

				/*
				 * Check if there is enough space for a header and five
				 * extra bytes. If this is not the case, close the current
				 * volume and start a new volume.
				 */

				if (archive->get_mv_bytes_left() <= (hdr_len + 5))
				{
					if ((result = archive->close_curr_volume()) == GUP_OK)
						result = archive->create_next_volume(opts->mv_size);
				}

				if ((result = archive->write_file_header(header)) != GUP_OK)
				{
					printf("\n");
					display_error(result);
				}
				else
				{
					init_progress(origsize);
					if ((result = archive->encode(header, source_file)) != GUP_OK)
					{
						printf("\n");
						display_error(result);
					}
					else
					{
						printf("%3lu%%\n", (origsize) ? (header->compsize * 100) / origsize : 100);
						result = archive->write_file_trailer(header);

						/* got a multiple volume break? */
						if ((result == GUP_OK) && archive->mv_break())
						{
							if ((result = archive->close_curr_volume()) == GUP_OK)
								result = archive->create_next_volume(opts->mv_size);
						}

						origsize -= header->origsize;
					}
				}

				segment_cnt++;			/* Increase segment counter. */
			} while ((origsize) && (result == GUP_OK));
		}
		else	// no multiple volume
		{
			header->offset = 0;
			header->method = opts->mode;

			if ((result = archive->write_file_header(header)) != GUP_OK)
			{
				printf("\n");
				display_error(result);
			}
			else
			{
				if ((result = archive->encode(header, source_file)) != GUP_OK)
				{
					printf("\n");
					display_error(result);
				}
				else
				{
					printf("%3lu%%\n", (origsize) ? (header->compsize * 100) / origsize : 100);
				}

				if (result == GUP_OK)
					result = archive->write_file_trailer(header);
			}
		}

		close(source_file);
		break;

	case SYMLINK_TYPE:
		if ((result = gup_readlink(the_name, &linkname)) == GUP_OK)
		{
			header->set_linkname(linkname);
			delete[] linkname;
		}
		/* No break. */
	case DIR_TYPE:
	case LABEL_TYPE:
		if (result == GUP_OK)
		{
			if(opts->mv_mode)
			{
				unsigned long hdr_len;	/* Length of file header. */

				/*
				 * Check if there is enough space for a header and five
				 * extra bytes. If this is not the case, close the current
				 * volume and start a new volume.
				 */

				hdr_len = header->get_header_len();

				if (archive->get_mv_bytes_left() <= (hdr_len + 5))
				{
					if ((result = archive->close_curr_volume()) == GUP_OK)
						result = archive->create_next_volume(opts->mv_size);
				}
			}

			if ((result = archive->write_file_header(header)) != GUP_OK)
			{
				printf("\n");
				display_error(result);
			}
			else
			{
				printf("100%%\n");
				result = archive->write_file_trailer(header);
			}
		}
		break;

	default:
		break;
	}

	delete header;

	return result;
}

/* RK improve error handling NULL crctable */

int compress(OPTIONS *opts)
{
	int i = 0;
	archive *archive;
	gup_result result = GUP_OK, cls_result;
	GUPMSG msgfunc;
	int mv_mode;

	/* set defaults
	 */

	// progress printing func
	msgfunc.print_progress = print_progress;
	msgfunc.pp_propagator = NULL;
	// init message handler func
	msgfunc.init_message = init_message;
	msgfunc.im_propagator = NULL;

	if (opts->type == AT_UNKNOWN)
	{
		if ((opts->type = get_arc_type(opts->arj_name)) == AT_UNKNOWN)
			opts->type = AT_ARJ;
	}
	
	printf("#d: output type AT: %d\n", __LINE__, opts->type);

	archive = new_archive((archive_type) opts->type);
	if (archive == NULL)
	{
		fprintf(stderr, "Fatal error: unsupported output format: %d\n", opts->type);
		exit(42);
	}

	archive->arcctl(ARC_MV_MODE, &mv_mode);

	if (!mv_mode && opts->mv_mode)
		printf("Multiple volume archives are not supported by this archive type, aborting.\n");
	else
	{
		result = archive->create_archive(opts->arj_name, opts, opts->mv_size, 0, &msgfunc);

		if (result != GUP_OK)
			fprintf(stderr, "%s: unable to open \'%s\'\n", opts->programname, opts->arj_name);
		else
		{
			if ((result = archive->write_main_header("")) == GUP_OK)
			{
				while ((opts->args[i]) && (result == GUP_OK))
				{
					if ((result = pack_arg(archive, opts->args[i], opts)) != GUP_OK)
						fprintf(stderr, "%s: Error processing %s\n", opts->programname, opts->args[i]);
					i++;
				}
			}

			if ((cls_result = archive->close_archive((i == -1) ? 0 : 1)) != GUP_OK)
			{
				if (result == GUP_OK)
					result = cls_result;
				fprintf(stderr, "%s: Error closing %s\n", opts->programname, opts->args[i]);
			}
		}
	}

	delete archive;

	return (result != GUP_OK) ? -1 : 0;
}
