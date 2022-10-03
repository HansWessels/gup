/*
 * Support library.
 *
 *     matchpat - Match pattern.
 *
 * $Author: klarenw $
 * $Date: 1998-12-27 21:51:17 +0100 (Sun, 27 Dec 1998) $
 * $Revision: 134 $
 * $Log$
 * Revision 1.1  1998/12/27 20:51:17  klarenw
 * Added pattern matching functions and error code conversion functions.
 * Fixed bugs in file buffering functions and strlwr functions.
 *
 */

#include "gup.h"
#include "support.h"

/*
 * int match_pattern(const char *name, const char *pattern)
 *
 * Check if filename 'name' matches pattern 'pattern'.
 *
 * Parameters:
 *
 * name	- filename.
 * pattern	- pattern.
 *
 * Result: FALSE if 'pattern' does not match 'name', TRUE if
 *         'pattern' matches 'name'.
 */

int match_pattern(const char *name, const char *pattern)
{
	const char *p = pattern, *n = name;

	do
	{
		switch (*p)
		{
		case '?':						/* Match any character. */
			if (*n++ == 0)
				return FALSE;
			p++;
			break;
		case '*':						/* Zero or more characters. */
			p++;
			if (*p == 0)
				return TRUE;
			else
			{
				while (*n != 0)
				{
					if (match_pattern(n, p) == TRUE)
						return TRUE;
					n++;
				}
				return FALSE;
			}
		case '[':						/* Match character from given set. */
			{
				int invert = FALSE, matched = FALSE;

				if (*n == 0)
					return FALSE;
				p++;
				if ((*p == '^') || (*p == '!'))
				{
					invert = TRUE;
					p++;
				}

				do
				{
					char c1, c2;

					c1 = *p++;

					if (c1 == 0)		/* Unexpected end of set. */
						return FALSE;

					if ((*p == '-') && (p[1] != ']'))	/* Range. */
					{
						p++;
						c2 = *p++;

						if (c2 == 0)	/* Unexpected end of set. */
							return FALSE;

						if ((*n >= c1) && (*n <= c2))
							matched = TRUE;
					}
					else				/* Single character. */
					{
						if (*n == c1)
							matched = TRUE;
					}
				} while (*p != ']');

				p++;
				n++;

				if ((matched && invert) || (!matched && !invert))
					return FALSE;
			}
			break;
		default:
			if (*n++ != *p++)
				return FALSE;
			break;
		}
	} while (*p != 0);

	return (*n) ? FALSE : TRUE;
}
