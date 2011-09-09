
/* This is what I include in nearly every file.
 *
 */

#if defined( macintosh )
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <errno.h>		
#if !defined(WIN32)
#include <unistd.h>
#include <sys/time.h>
#endif
#endif
#include <ctype.h>		
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "merc.h"
