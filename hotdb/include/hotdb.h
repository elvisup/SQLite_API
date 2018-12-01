#ifndef __HOTDB_H__
#define __HOTDB_H__

#define SQL_BUF_MAX_SIZE          (2048)//2KB

#define HOTDB_DBNAME_ERR          -1
#define HOTDB_DBMALLOC_ERR        -2
#define HOTDB_OPENDB_ERR          -3
#define HOTDB_DBINVALID_ERR       -4
#define HOTDB_DBLL_ERR            -5
#define HOTDB_CREATE_TABLE_ERR    -6
#define HOTDB_INSERT_TABLE_ERR    -7
#define HOTDB_HDINVALID_ERR       -8
#define HOTDB_HDCONDITION_ERR     -9

#define HOTDB_BLOB_DATA_STATUS_RAW      100
#define HOTDB_BLOB_DATA_STATUS_DONE     101

void HotDB_Get_Version(void);
int HotDB_Create_DataBase(char *database_name);
int HotDB_Create_Table(int db_ctx, char *table, char *label_list);
int HotDB_Insert_To_Table(int db_ctx, char *table, char *label_list, char *data, unsigned int data_size);
int HotDB_Insert_Blob_To_Table(int db_ctx, char *table, char *label_list, char *data, unsigned int data_size, char *blob, unsigned int blob_size);

/**
 *  function: HotDB_Get_Blob_From_Table
 *  input:
 *      db_ctx:
 *      table:
 *      what:
 *      condition:
 *  output:
 *      blob:
 *      blob_size:
 *  return:
 *      -1: error.
 *      HOTDB_BLOB_DATA_STATUS_RAW:
 *      HOTDB_BLOB_DATA_STATUS_DONE:
 **/
int HotDB_Get_Blob_From_Table(int db_ctx, char *table, char *what, char *condition, char *blob, unsigned int *blob_size);

int HotDB_Prepare(int db_ctx, char *table, char *what, char *condition, int *p_handle);
int HotDB_Get_Blob_Data_Quick(int p_handle, unsigned int id, char *blob, unsigned int *blob_size);
void HotDB_Deprepare(int p_handle);

int HotDB_Close_DataBase(int db_ctx);

#endif /*__HOTDB_H__*/
