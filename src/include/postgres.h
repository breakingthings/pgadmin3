//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
// Copyright (C) 2002 - 2003, The pgAdmin Development Team
// This software is released under the Artistic Licence
//
// postgres.h - dummy include
//
//////////////////////////////////////////////////////////////////////////


#include <string.h>

#define lengthof(array) (sizeof (array) / sizeof ((array)[0]))
#define endof(array)	(&array[lengthof(array)])



// to suppress much stuff in parse.h
#define YYTOKENTYPE
#define YYSTYPE


#define NAMEDATALEN 32