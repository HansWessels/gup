
#include "gup.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>

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
#include "utils.h"

#include "compr_io.h"
#include "compress.h"
#include "arcman.h"

#include "utils.h"

/*
 * int scan_arj_directory(const char *file, ARCHIVE_INFO *arc_info, ARC_DIRECTORY *dir)
 *
 * Make a list of all files in the archive. The archive filename
 * has got to have a suffix '.arj' or '.axx', where xx is a two
 * digit number.
 *
 * Parameters:
 *
 * opts			- options.
 * do_thing		- function which performs a action on a file
 * 				  in the archive.
 *
 * Result: 0 if no error, 1 otherwise.
 */

int scan_arj_directory(OPTIONS *opts,
					   gup_result (*do_thing1)(archive *archive,
					   fileheader *ah, OPTIONS *opts),
					   gup_result (*do_thing2)(archive *archive,
					   fileheader *ah, OPTIONS *opts))
{
	archive *archive;
	gup_result result;
	GUPMSG msgfunc;
	archive_type type;

	if ((type = get_arc_type(opts->arj_name)) == AT_UNKNOWN)
		type = AT_ARJ;

	archive = new_archive(type);

	// progress printing func
	msgfunc.print_progress = print_progress;
	msgfunc.pp_propagator = NULL;
	// init message handler func
	msgfunc.init_message = init_message;
	msgfunc.im_propagator = NULL;

	result = archive->open_archive(opts->arj_name, opts, &msgfunc);

	if (result == GUP_OK)
	{
		mainheader *main_hdr;
		fileheader *file_hdr;

		main_hdr = archive->read_main_header(result);
		if (result == GUP_EOF)
			result = GUP_BROKEN;

		/*
		 * Scan the archive for files.
		 */

		while (result == GUP_OK)
		{
			file_hdr = archive->read_file_header(result);
			if (result == GUP_EOF)
				result = GUP_BROKEN;

			if (result == GUP_OK)
			{
				/*
				 * !!! Check if in the case of the first segment of a file,
				 * this segment is at the end of the volume.
				 */

				if ((result = do_thing1(archive, file_hdr, opts)) == GUP_OK)
					if ((result = archive->read_file_trailer(file_hdr)) == GUP_OK)
						result = do_thing2(archive, file_hdr, opts);
			}

			if ((result > GUP_OK) && (result < GUP_LAST_ERROR))
				fprintf(stderr, "%s: Error processing \'%s\': %d\n", opts->programname, opts->arj_name, (int) result);

			delete file_hdr;

			if (result == GUP_END_VOLUME)
			{
				if (opts->mv_mode)
				{
					result = archive->close_curr_volume();
#if 0
					if ((is_removable(arc_name) && ask_nextvol) ||
						opt_wait_for_arj)
					{
						/* Ask user to change disk. */
					}
#endif
					if (result == GUP_OK)
						result = archive->open_next_volume();
				}
				else
				{
#if 0
					if (opt_multivol)
						/* Display message multiple volume not supported
							   for this extension. */
					else
						/* Display message this is a multiple volume
							   archive. Use multiple volume option. */
#endif
				}
			}
		}

		delete main_hdr;
	}

	gup_result result_cls =	archive->close_archive(1);
	// expected 'okay' error codes when we arrive here: { GUP_OK, GUP_END_ARCHIVE, GUP_END_VOLUME }
	// 
	// assertion checks for *sane* error codes or the set above; anything else is an internal implementation failure.
	ARJ_Assert((result >= GUP_OK && result < GUP_LAST_ERROR) || result == GUP_END_ARCHIVE || result == GUP_END_VOLUME);
	if (result == GUP_OK || result > GUP_LAST_ERROR)
		result = result_cls;
	delete archive;

	return result;
}

/*
 * gup_result do_unpack1(archive *archive, fileheader *ah,
 *						 OPTIONS *opts)
 *
 * Part 1 of decompression. This function is called after 'read_file_header'
 * and before 'read_file_trailer'. This function should assume that only
 * the following information in the header is known at this moment:
 *
 * - file name.
 * - file attributes.
 * - file type.
 * - host OS used to pack the file.
 * - the header flags.
 * - offset of this chunk in real file (multiple volume archives). Some
 *   archive types do not have offset (GZIP), special handling needed.
 *
 * The following information should not be assumed to be known at this
 * moment:
 *
 * - original size.
 * - compressed size.
 * - CRC.
 */

gup_result do_unpack1(archive *archive, fileheader *ah,
					  OPTIONS *opts)
{
	gup_result result;
	int outfile, openflags, outflags;
	const char *outname = ah->get_filename();

	if (opts->exclude_paths)
		outname = get_name(outname);

	init_progress(ah->origsize);

	{
		char buf[82];
		printf("     %s\r", pnprint(74, ah->get_filename(), buf));
	}

	switch (ah->file_type)
	{
	case BINARY_TYPE:
	case TEXT_TYPE:
#ifdef O_BINARY
		openflags = O_CREAT | O_WRONLY | O_BINARY;
#else
		openflags = O_CREAT | O_WRONLY;
#endif
		outflags = mode_os_to_unix(ah->get_file_stat()->file_mode);

		/*
		 * If the 'no_write_data' flag is not set, make sure the destination
		 * path exists, open the destination file and seek to the position
		 * in the destination file of this block.
		 */

		if (!opts->no_write_data)
		{
			/* TODO: check if file exists and handle it
			 */
			if (!ah->mv_is_continuation())	/* not a continued file? */
				openflags |= O_TRUNC;

			if ((result = builddir(outname)) != GUP_OK)
				  return result;

			if ((outfile = open(outname, openflags, outflags)) == -1)
				return gup_conv_err(errno);

			if (ah->mv_is_continuation())	/* continued file? */
			{
				long offset = lseek(outfile, 0, SEEK_END);

				if (offset == -1)
				{
					close(outfile);
					return gup_conv_err(errno);
				}

				if (offset != (long) ah->offset)
				{
					fprintf(stderr, "%s: Continued file \'%s\' has incorrect size!?\nTruncating!", opts->programname, ah->get_filename());
					lseek(outfile, ah->offset, SEEK_SET);
				}
			}
		}
		else
			outfile = 0;					/* Assign some value to 'outfile'. */

		result = archive->decode(ah, outfile);

		/*
		 * Close the destination file.
		 */

		if (!opts->no_write_data)
			close(outfile);

		return result;

	 case DIR_TYPE:
		if (!opts->no_write_data)
		{
			if ((result = builddir(outname)) != GUP_OK)
				  return result;
			return gup_mkdir(outname, ah->get_file_stat()->file_mode);
		}
		break;

	 case SYMLINK_TYPE:
		if (!opts->no_write_data)
		{
			if ((result = builddir(outname)) != GUP_OK)
				  return result;
			return gup_symlink(ah->get_linkname(), outname);
		}
		break;

	 default:
		break;
	}

	return GUP_OK;
}

/*
 * gup_result do_unpack2(archive *archive, fileheader *ah,
 *						 OPTIONS *opts)
 *
 * Part 2 of decompression. This function is called after 'decode'
 * and 'read_file_trailer'. Now all information of the decompressed
 * file is available. Here the following should be done:
 *
 * - Check CRC.
 * - Set file attributes, uid, gid, modification time etc.
 */

gup_result do_unpack2(archive *archive, fileheader *ah,
					  OPTIONS *opts)
{
	gup_result result = GUP_OK;

	(void) archive;

	if (!opts->no_write_data)
		result = gup_set_stat(ah->get_filename(), ah->get_file_stat());

	if ((!opts->no_crc_checking) && (ah->has_crc()) && (ah->file_crc != ah->real_crc))
	{
		printf("ERROR\n");
		result = GUP_CRC_FAULT;
	}
	printf(" OK  \n");

	return result;
}

int decompress(OPTIONS *opts)
{
	/*
	 * If the command is 'test' set the 'no_write_data' flag. This
	 * flag tells the archive class not to write data.
	 */

	if (opts->command == CMD_TEST)
		opts->no_write_data = 1;

	return scan_arj_directory(opts, do_unpack1, do_unpack2);
}

gup_result do_list1(archive *archive, fileheader *ah,
					OPTIONS *opts)
{
	(void) opts;

	return archive->skip_compressed_data(ah);
}

gup_result do_list2(archive *archive, fileheader *ah,
				   OPTIONS *opts)
{
	const char *name = ah->get_filename();

	(void) archive;
	(void) opts;

	printf("%-40s  %10ld %10ld %4i%% %4X %s\n", name,
			ah->origsize,
			ah->compsize,
			(ah->origsize == 0) ? 100 : (int)(100 * ah->compsize / ah->origsize),
			ah->method,
			host_os_name(ah->host_os));

	return GUP_OK;
}

int list_arj(OPTIONS *opts)
{
	printf("Filename                                    Original Compressed Ratio Mode\n");
	printf("                                          ---------- ---------- ----- ----\n");

	return scan_arj_directory(opts, do_list1, do_list2);
}
