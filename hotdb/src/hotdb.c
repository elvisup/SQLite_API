#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <slog.h>
#include <core.h>
#include <sqlite3.h>
#include <hotdb.h>

/*#define TIME_DEBUG*/

typedef struct _sqlite3_context {
	char name[32];
	char *sql;
	char *zErrMsg;
	sqlite3* db;
} sqlite3_ctx;

void HotDB_Get_Version(void)
{
	slog(LOG_INFO, "#### hotdb version    : [0x%08x]\n", LIBHOTDB_VERSION);
	slog(LOG_INFO, "#### core libversion  : [%s]\n", sqlite3_libversion());
	slog(LOG_INFO, "#### core sourceid    : [%s]\n", sqlite3_sourceid());
	/*printf("#### core libversion_number: [%d]\n", sqlite3_libversion_number());*/
}

int HotDB_Create_DataBase(char *database_name)
{
	int rc;
	sqlite3_ctx *ctx = NULL;
	if (database_name == NULL) {
		slog(LOG_ERR, "database name error!\n");
		return HOTDB_DBNAME_ERR;
	}

	ctx = (sqlite3_ctx *)malloc(sizeof(sqlite3_ctx));
	if (ctx == NULL) {
		slog(LOG_ERR, "database [%s] malloc error!\n", database_name);
		return HOTDB_DBMALLOC_ERR;
	}

	rc = sqlite3_open_v2(database_name, &(ctx->db), SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE | SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE, NULL);
	if(rc){
		slog(LOG_ERR, "Opened [%s] database errror!, rc: %d\n", database_name, rc);
		return HOTDB_OPENDB_ERR;
	}

	memcpy(ctx->name, database_name, 32);

	return (int)ctx;
}

int HotDB_Create_Table(int db_ctx, char *table, char *label_list)
{
	int rc;
	if (db_ctx <= 0) {
		slog(LOG_ERR, "database ctx [%d] invalid!\n", db_ctx);
		return HOTDB_DBINVALID_ERR;
	}

	if (label_list == NULL || table == NULL) {
		slog(LOG_ERR, "label_list or  table is NULL\n");
		return HOTDB_DBLL_ERR;
	}

	char sql_buf[SQL_BUF_MAX_SIZE] = {'\0'};
	sqlite3_ctx *ctx = (sqlite3_ctx *)db_ctx;

	sprintf(sql_buf, "%s %s (%s);", db_sqlstr[CREATE].sql, table, label_list);
	ctx->sql = sql_buf;

	rc = sqlite3_exec(ctx->db, ctx->sql, NULL, NULL, &ctx->zErrMsg);
	if( rc != SQLITE_OK ){
		if( rc == 1 ){
			/* table already exists */
			slog(LOG_INFO, "database [%s]: %s\n", ctx->name, ctx->zErrMsg);
		} else {
			fprintf(stderr, "SQL error: %s, rc: %d\n", ctx->zErrMsg, rc);
			sqlite3_free(ctx->zErrMsg);
			return HOTDB_CREATE_TABLE_ERR;
		}
	}

	return 0;
}

int HotDB_Insert_To_Table(int db_ctx, char *table, char *label_list, char *data, unsigned int data_size)
{
	int rc;
	unsigned int sql_prefix_len = 0;
	if (db_ctx <= 0) {
		slog(LOG_ERR, "database ctx [%d] invalid!\n", db_ctx);
		return HOTDB_DBINVALID_ERR;
	}

	if (label_list == NULL || table == NULL) {
		slog(LOG_ERR, "label_list or  table is NULL\n");
		return HOTDB_DBLL_ERR;
	}

	char sql_buf[SQL_BUF_MAX_SIZE] = {'\0'};
	sqlite3_ctx *ctx = (sqlite3_ctx *)db_ctx;

	sprintf(sql_buf, "%s %s (%s) VALUES (", db_sqlstr[INSERT].sql, table, label_list);
	sql_prefix_len = strlen(sql_buf);
	memcpy(sql_buf+sql_prefix_len, data, data_size);
	memcpy(sql_buf+sql_prefix_len + data_size, ");", strlen(");"));

	ctx->sql = sql_buf;

	rc = sqlite3_exec(ctx->db, ctx->sql, NULL, NULL, &ctx->zErrMsg);
	if( rc != SQLITE_OK ){
		fprintf(stderr, "SQL error: %s\n", ctx->zErrMsg);
		sqlite3_free(ctx->zErrMsg);
		return HOTDB_INSERT_TABLE_ERR;
	}

	return 0;
}

int HotDB_Insert_Blob_To_Table(int db_ctx, char *table, char *label_list, char *data, unsigned int data_size, char *blob, unsigned int blob_size)
{
	int rc;
	unsigned int sql_prefix_len = 0;
	if (db_ctx <= 0) {
		slog(LOG_ERR, "database ctx [%d] invalid!\n", db_ctx);
		return HOTDB_DBINVALID_ERR;
	}

	if (label_list == NULL || table == NULL) {
		slog(LOG_ERR, "label_list or  table is NULL\n");
		return HOTDB_DBLL_ERR;
	}

	char sql_buf[SQL_BUF_MAX_SIZE] = {'\0'};
	sqlite3_ctx *ctx = (sqlite3_ctx *)db_ctx;

	sprintf(sql_buf, "%s %s (%s) VALUES (", db_sqlstr[INSERT].sql, table, label_list);
	sql_prefix_len = strlen(sql_buf);
	memcpy(sql_buf+sql_prefix_len, data, data_size);
	memcpy(sql_buf+sql_prefix_len + data_size, ");", strlen(");"));

	ctx->sql = sql_buf;

	sqlite3_stmt *stat = NULL;
	const char *tail = NULL;
	rc = sqlite3_prepare(ctx->db, ctx->sql, strlen(sql_buf), &stat, &tail);
	if (rc != SQLITE_OK) {
		slog(LOG_ERR, "%s: sqlite3_prepare error, rc: %d\n", __func__, rc);
	}

	rc = sqlite3_bind_blob(stat, 1, blob, blob_size, SQLITE_STATIC);
	if (rc != SQLITE_OK) {
		slog(LOG_ERR, "sqlite3_bind_blob error, rc: %d\n", rc);
	}
	rc = sqlite3_step(stat);
	if (rc != SQLITE_DONE) {
		slog(LOG_ERR, "sqlite3_step error, rc: %d\n", rc);
		return -1;
	}

	sqlite3_finalize(stat);

	return 0;
}

int HotDB_Get_Blob_From_Table(int db_ctx, char *table, char *what, char *condition, char *blob, unsigned int *blob_size)
{
	int rc;
	if (db_ctx <= 0) {
		slog(LOG_ERR, "database ctx [%d] invalid!\n", db_ctx);
		return HOTDB_DBINVALID_ERR;
	}

	if (table == NULL) {
		slog(LOG_ERR, "label_list or  table is NULL\n");
		return HOTDB_DBLL_ERR;
	}

	char sql_buf[SQL_BUF_MAX_SIZE] = {'\0'};
	sqlite3_ctx *ctx = (sqlite3_ctx *)db_ctx;

	/*select feature from table where feature_id=0*/
	sprintf(sql_buf, "%s %s from %s where %s;", db_sqlstr[SELECT].sql, what, table, condition);
	/*printf("%s -> [%s]\n", __func__, sql_buf);*/

	ctx->sql = sql_buf;

#ifdef TIME_DEBUG
	struct timeval start, end;
	unsigned long long diff = 0;
	gettimeofday(&start, NULL);
#endif
	sqlite3_stmt *stat = NULL;
	const char *tail = NULL;
	rc = sqlite3_prepare(ctx->db, ctx->sql, strlen(sql_buf), &stat, &tail);
	if (rc != SQLITE_OK) {
		slog(LOG_ERR, "%s: sqlite3_prepare error, rc: %d\n", __func__, rc);
	}

	rc = sqlite3_step(stat);
	if ((rc != SQLITE_ROW) && (rc != SQLITE_DONE)) {
		slog(LOG_ERR, "sqlite3_step error, rc: %d\n", rc);
		return -1;
	}

	const void *blob_data = sqlite3_column_blob(stat, 0);
	*blob_size = sqlite3_column_bytes(stat, 0);

#ifdef TIME_DEBUG
	gettimeofday(&end, NULL);
	diff += ((int64_t)end.tv_sec*1000*1000 + end.tv_usec) - ((int64_t)start.tv_sec*1000*1000 + start.tv_usec);
	slog(LOG_DBG, "get blob data time: %d\n", diff);
#endif

	/*printf("blob_size: %d\n", *blob_size);*/
	memcpy(blob, blob_data, *blob_size);
	sqlite3_finalize(stat);

	return rc;
}

int HotDB_Prepare(int db_ctx, char *table, char *what, char *condition, int *p_handle)
{
	int rc;
	if (db_ctx <= 0) {
		slog(LOG_ERR, "database ctx [%d] invalid!\n", db_ctx);
		return HOTDB_DBINVALID_ERR;
	}

	if (table == NULL) {
		slog(LOG_ERR, "label_list or  table is NULL\n");
		return HOTDB_DBLL_ERR;
	}

	char sql_buf[SQL_BUF_MAX_SIZE] = {'\0'};
	sqlite3_ctx *ctx = (sqlite3_ctx *)db_ctx;

	sprintf(sql_buf, "%s %s from %s where %s;", db_sqlstr[SELECT].sql, what, table, condition);

	ctx->sql = sql_buf;

	sqlite3_stmt *stat = NULL;
	const char *tail = NULL;
	rc = sqlite3_prepare(ctx->db, ctx->sql, strlen(sql_buf), &stat, &tail);
	if (rc != SQLITE_OK) {
		slog(LOG_ERR, "%s: sqlite3_prepare error, rc: %d\n", __func__, rc);
	}

	*p_handle = (int)stat;

	return rc;
}

int HotDB_Get_Blob_Data_Quick(int p_handle, unsigned int id, char *blob, unsigned int *blob_size)
{
	int rc;
	if (p_handle <= 0) {
		slog(LOG_ERR, "p_handle [%d] invalid!\n", p_handle);
		return HOTDB_HDINVALID_ERR;
	}

	sqlite3_stmt *stat = (sqlite3_stmt *)p_handle;

	sqlite3_bind_int(stat, 1, id);
	rc = sqlite3_step(stat);
	if ((rc != SQLITE_ROW) && (rc != SQLITE_DONE)) {
		slog(LOG_ERR, "sqlite3_step error, rc: %d\n", rc);
		return -1;
	}

	const void *blob_data = sqlite3_column_blob(stat, 0);
	*blob_size = sqlite3_column_bytes(stat, 0);

	memcpy(blob, blob_data, *blob_size);

	sqlite3_reset(stat);

	return rc;
}

void HotDB_Deprepare(int p_handle)
{
	sqlite3_finalize((sqlite3_stmt *)p_handle);
}

int HotDB_Close_DataBase(int db_ctx)
{
	if (db_ctx <= 0) {
		slog(LOG_ERR, "database ctx [%d] invalid!\n", db_ctx);
		return HOTDB_DBINVALID_ERR;
	}

	sqlite3_ctx *ctx = (sqlite3_ctx *)db_ctx;
	sqlite3_close(ctx->db);

	return 0;
}

