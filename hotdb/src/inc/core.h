#ifndef __CORE_H__
#define __CORE_H__

#define LIBHOTDB_VERSION 0x00000002

#define CREATE        0
#define INSERT        1
#define DELECT        3
#define SELECT        4

struct sqlite_db_str {
	char *sql;
};

struct sqlite_db_str db_sqlstr[] = {
	[CREATE] = { .sql = "CREATE TABLE " },
	[INSERT] = { .sql = "INSERT INTO "  },
	[DELECT] = { .sql = "DELETE FROM "  },
	[SELECT] = { .sql = "SELECT "  },
};

#endif /*__CORE_H__*/
