#ifndef __HOTDB_H__
#define __HOTDB_H__

#define HOTDB_DBNAME_ERR          -1
#define HOTDB_DBMALLOC_ERR        -2
#define HOTDB_OPENDB_ERR          -3
#define HOTDB_DBINVALID_ERR       -4
#define HOTDB_DBLL_ERR            -5
#define HOTDB_CREATE_TABLE_ERR    -6

void HotDB_Get_Version(void);
int HotDB_Create_DataBase(char *database_name);
int HotDB_Create_Table(int db_ctx, char *table, char *label_list);

#endif /*__HOTDB_H__*/
