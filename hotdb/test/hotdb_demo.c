#include <stdio.h>
#include <pthread.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <hotdb.h>

#define TABLE_ELVIS      0
#define TABLE_BLOB       1

#define DB_DATA_CONFIRM  0

struct db_status {
	int db_ctx;
	char *db_name;
	char *table;
	char *table_mumber;
	char *table_label;
};

struct db_status gdb_sts[] = {
	[TABLE_ELVIS] = {
		.db_ctx = -1,
		.db_name = "MyDBDemo.db",
		.table = "ElvisTable",
		.table_mumber = "name text, age int",
		.table_label  = "name, age",
	},
	[TABLE_BLOB]  = {
		.db_ctx = -1,
		.db_name = "MyDBDemo.db",
		.table = "BlobTable",
		.table_mumber = "feature_id integer primary key, idx int, feature blob",
		.table_label  = "feature_id, idx, feature",
	},
};

int main()
{
	int ret = -1;
	int i = 0, k = 0;
	int db_context = -1;

	HotDB_Get_Version();

	gdb_sts[TABLE_ELVIS].db_ctx = HotDB_Create_DataBase(gdb_sts[TABLE_ELVIS].db_name);
	if (gdb_sts[TABLE_ELVIS].db_ctx < 0) {
		printf("HotDB_Create_DataBase error: %d\n", gdb_sts[TABLE_ELVIS].db_ctx);
		return -1;
	}
	/*printf("db_ctx: 0x%08x\n", gdb_sts[TABLE_ELVIS].db_ctx);*/

	gdb_sts[TABLE_BLOB].db_ctx = HotDB_Create_DataBase(gdb_sts[TABLE_BLOB].db_name);
	if (gdb_sts[TABLE_BLOB].db_ctx < 0) {
		printf("HotDB_Create_DataBase error: %d\n", gdb_sts[TABLE_BLOB].db_ctx);
		return -1;
	}
	/*printf("db_ctx: 0x%08x\n", gdb_sts[TABLE_ELVIS].db_ctx);*/

	ret = HotDB_Create_Table(gdb_sts[TABLE_ELVIS].db_ctx, gdb_sts[TABLE_ELVIS].table, gdb_sts[TABLE_ELVIS].table_mumber);
	if (ret < 0) {
		printf("create table error! %d\n", ret);
		return -1;
	}

	ret = HotDB_Create_Table(gdb_sts[TABLE_BLOB].db_ctx, gdb_sts[TABLE_BLOB].table, gdb_sts[TABLE_BLOB].table_mumber);
	if (ret < 0) {
		printf("create table error! %d\n", ret);
		return -1;
	}

	srand((unsigned)time(NULL));

	struct timeval start, end;
	unsigned long long diff = 0;
	for (k = 0; k < 1000; k++) {
		char insert_buf[128] = {0};
		sprintf(insert_buf, "'%s%d', %d", "huanwang", k, k);
		ret = HotDB_Insert_To_Table(gdb_sts[TABLE_ELVIS].db_ctx, gdb_sts[TABLE_ELVIS].table, gdb_sts[TABLE_ELVIS].table_label, insert_buf, strlen(insert_buf));
		if (ret < 0) {
			printf("insert table error! %d\n", ret);
			return -1;
		}

		char insert_blobbuf[128] = {0};
		sprintf(insert_blobbuf, "%d, %d, %s", k, k+100, "?");
		char blobbuf[1024] = {0};
		for (i = 0; i < 1024; i++) {
			blobbuf[i] = rand() % 128;
		}
		gettimeofday(&start, NULL);
		ret = HotDB_Insert_Blob_To_Table(gdb_sts[TABLE_BLOB].db_ctx, gdb_sts[TABLE_BLOB].table, gdb_sts[TABLE_BLOB].table_label, \
				                 insert_blobbuf, strlen(insert_blobbuf), blobbuf, 1024);
		if (ret < 0) {
			printf("insert blob table error! %d\n", ret);
			return -1;
		}
		gettimeofday(&end, NULL);
		diff += ((int64_t)end.tv_sec*1000*1000 + end.tv_usec) - ((int64_t)start.tv_sec*1000*1000 + start.tv_usec);
	}

	printf("insert %d 1KB blob data total time: %lld us, average time: %d us\n", k, diff, diff / k);

	diff = 0;
	char get_blob_data[1024] = {'\0'};
	char condition_buf[1024] = {'\0'};
	int get_blob_size, idx = 0;
#if DB_DATA_CONFIRM
	int j = 0;
	printf("\n");
	printf("+-----------------------------+\n");
	printf("| idx  d0 d1 d2 d3 d4 d5 d6 d7|\n");
	printf("+-----------------------------|\n");
#endif
	int p_handler_c;
	ret = HotDB_Prepare(gdb_sts[TABLE_BLOB].db_ctx, gdb_sts[TABLE_BLOB].table, "feature", "feature_id=?", &p_handler_c);
	if (ret < 0) {
		printf("HotDB_Prepare error! %d\n", ret);
		return -1;
	}
	do {
#if 0
		sprintf(condition_buf, "feature_id=%d", idx);
		gettimeofday(&start, NULL);
		ret = HotDB_Get_Blob_From_Table(gdb_sts[TABLE_BLOB].db_ctx, gdb_sts[TABLE_BLOB].table, "feature", condition_buf, get_blob_data, &get_blob_size);
		if (ret < 0) {
			printf("HotDB_Get_Blob_From_Table error! %d\n", ret);
			return -1;
		}
		gettimeofday(&end, NULL);
#else
		gettimeofday(&start, NULL);
		ret = HotDB_Get_Blob_Data_Quick(p_handler_c, idx, get_blob_data, &get_blob_size);
		if ((ret != HOTDB_BLOB_DATA_STATUS_RAW) && (ret != HOTDB_BLOB_DATA_STATUS_DONE)) {
			printf("HotDB_Get_Blob_Data_Quick error! [%d]\n", ret);
			break;
		}
		gettimeofday(&end, NULL);
#endif
		diff += ((int64_t)end.tv_sec*1000*1000 + end.tv_usec) - ((int64_t)start.tv_sec*1000*1000 + start.tv_usec);
#if DB_DATA_CONFIRM
		if (idx < 20) {
			printf("|%04d: ", idx);
			for (j = 0; j < 8; j++) {
				if (j == 7) printf("%02x", get_blob_data[j]);
				else printf("%02x ", get_blob_data[j]);
			}
			printf("|\n");
		}
#endif
		idx++;
	} while(!((ret < 0 ) || (ret == HOTDB_BLOB_DATA_STATUS_DONE)));
	HotDB_Deprepare(p_handler_c);
#if DB_DATA_CONFIRM
	printf("+-----------------------------+\n\n");
#endif
	printf("get %d 1KB blob data total time: %lld us, average time: %d us\n", idx - 1, diff, diff / (idx - 1));

	ret = HotDB_Close_DataBase(gdb_sts[TABLE_ELVIS].db_ctx);
	if (ret < 0) {
		printf("close table error! %d\n", ret);
		return -1;
	}

	ret = HotDB_Close_DataBase(gdb_sts[TABLE_BLOB].db_ctx);
	if (ret < 0) {
		printf("close table error! %d\n", ret);
		return -1;
	}

	return 0;
}

#if 0
struct sqlite3_context {
	char *sql;
	char *zErrMsg;
	sqlite3* db;
} sqlite3_ctx;

static int search_flag = 0;
static int callback_search(void *NotUsed, int argc, char **argv, char **azColName)
{
	int i;
	char *str = (char *)NotUsed;
	for(i=0; i<argc; i++){
		if ((search_flag == 0) || !strcmp(str, argv[i])) {
			printf("Search [%s] OK!\n", str);
			search_flag = 1;
		}
	}
	return 0;
}

int API_sqlite3_exec_search(char *id, char *table, void *data)
{
	int rc;
	char buffer[128] = {0};
	sprintf(buffer, "SELECT %s from %s", id, table);
	sqlite3_ctx.sql = buffer;

	rc = sqlite3_exec(sqlite3_ctx.db, sqlite3_ctx.sql, callback_search, data, &sqlite3_ctx.zErrMsg);
	if( rc != SQLITE_OK ){
		fprintf(stderr, "SQL error: %s\n", sqlite3_ctx.zErrMsg);
		sqlite3_free(sqlite3_ctx.zErrMsg);
		return -1;
	}

	return search_flag;
}

void API_sqlite3_exec_cls_search_status(void)
{
	search_flag = 0;
}

static int callback_print(void *NotUsed, int argc, char **argv, char **azColName)
{
	int i;
	for(i=0; i<argc; i++){
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");
	return 0;
}

int API_sqlite3_exec(char *sql, void *data)
{
	int rc;
	rc = sqlite3_exec(sqlite3_ctx.db, sql, callback_print, data, &sqlite3_ctx.zErrMsg);
	if( rc != SQLITE_OK ){
		fprintf(stderr, "SQL error: %s\n", sqlite3_ctx.zErrMsg);
		sqlite3_free(sqlite3_ctx.zErrMsg);
		return -1;
	}

	return 0;
}

int API_sqlite3_exec_list_table(char *table)
{
	int rc;
	char buffer[128] = {0};
	sprintf(buffer, "SELECT * from %s", table);
	sqlite3_ctx.sql = buffer;

	printf(">>>>>>> List Table [%s]\n", table);
	rc = sqlite3_exec(sqlite3_ctx.db, sqlite3_ctx.sql, callback_print, NULL, &sqlite3_ctx.zErrMsg);
	if( rc != SQLITE_OK ){
		fprintf(stderr, "SQL error: %s\n", sqlite3_ctx.zErrMsg);
		sqlite3_free(sqlite3_ctx.zErrMsg);
		return -1;
	}

	return 0;
}

int API_sqlite3_exec_insrt_to_table(char *data)
{
	int rc;
	sqlite3_ctx.sql = data;

	rc = sqlite3_exec(sqlite3_ctx.db, sqlite3_ctx.sql, callback_print, NULL, &sqlite3_ctx.zErrMsg);
	if( rc != SQLITE_OK ){
		fprintf(stderr, "SQL error: %s\n", sqlite3_ctx.zErrMsg);
		sqlite3_free(sqlite3_ctx.zErrMsg);
		return -1;
	}

	return 0;
}

int API_sqlite3_exec_table_update(char *table, char *modify_label, char *modify_val, char *tgt_label, char *tgt_val)
{
	int rc;
	char buffer[128] = {0};
	sprintf(buffer, "UPDATE %s set %s = '%s' where %s=%s", \
			table, modify_label, modify_val, tgt_label, tgt_val);
	sqlite3_ctx.sql = buffer;

	rc = sqlite3_exec(sqlite3_ctx.db, sqlite3_ctx.sql, callback_print, NULL, &sqlite3_ctx.zErrMsg);
	if( rc != SQLITE_OK ){
		fprintf(stderr, "SQL error: %s\n", sqlite3_ctx.zErrMsg);
		sqlite3_free(sqlite3_ctx.zErrMsg);
		return -1;
	}

	return 0;
}

int API_sqlite3_exec_delete_table_label(char *table, char *label, char *val)
{
	int rc;
	char buffer[128] = {0};
	sprintf(buffer, "DELETE from %s where %s=%s", table, label, val);
	sqlite3_ctx.sql = buffer;

	rc = sqlite3_exec(sqlite3_ctx.db, sqlite3_ctx.sql, callback_print, NULL, &sqlite3_ctx.zErrMsg);
	if( rc != SQLITE_OK ){
		fprintf(stderr, "SQL error: %s\n", sqlite3_ctx.zErrMsg);
		sqlite3_free(sqlite3_ctx.zErrMsg);
		return -1;
	}

	return 0;
}

int main()
{
	int rc;

	printf("############################################\n");
	printf("#### sqlite3_libversion       : [%s]\n", sqlite3_libversion());
	printf("#### sqlite3_sourceid         : [%s]\n", sqlite3_sourceid());
	printf("#### sqlite3_libversion_number: [%d]\n", sqlite3_libversion_number());
	printf("############################################\n\n");

	rc = sqlite3_open("fr_record.db",&sqlite3_ctx.db);
	if(rc){
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(sqlite3_ctx.db));
		return -1;
	}else{
		fprintf(stderr, "Opened database successfully\n");
	}

	/* Create Table */
	sqlite3_ctx.sql = "CREATE TABLE COMPANY("  \
	       "PATH           TEXT," \
	       "NAME           TEXT," \
	       "TIMES          INT," \
	       "SEX            TEXT," \
	       "ID             INT," \
	       "SCORE          REAL );";
	rc = API_sqlite3_exec(sqlite3_ctx.sql, NULL);

	/* Insert */
	sqlite3_ctx.sql = \
	       "INSERT INTO COMPANY (PATH,NAME,TIMES,SEX,ID,SCORE) "  \
	       "VALUES ('2018-05-08-15-16-33-00-19-26-097-097.jpeg', 'Paul', 1, 'Unkown', 100, 98.02 ); " \
	       "INSERT INTO COMPANY (PATH,NAME,TIMES,SEX,ID,SCORE)" \
	       "VALUES ('2018-05-08-15-16-34-00-19-26-097-097.jpeg', 'Lili', 1, 'Unkown', 101, 97.52 ); " \
	       "INSERT INTO COMPANY (PATH,NAME,TIMES,SEX,ID,SCORE)" \
	       "VALUES ('2018-05-08-15-16-33-00-19-28-007-090.jpeg', 'Elvis', 1, 'Unkown', 102, 100.00 );";
	rc = API_sqlite3_exec(sqlite3_ctx.sql, NULL);

	/* list table */
	rc = API_sqlite3_exec_list_table("COMPANY");

	/* search */
	char *uid = "100";
	rc = API_sqlite3_exec_search("ID", "COMPANY", uid);
	if (rc <= 0) {
		printf("API_sqlite3_exec_search error\n");
	} else {
		/*printf("API_sqlite3_exec_search OK\n");*/
	}
	API_sqlite3_exec_cls_search_status();

	/* Delete */
	rc = API_sqlite3_exec_delete_table_label("COMPANY", "ID", "100");

	/* Insert */
	char *insert_data = "INSERT INTO COMPANY (PATH,NAME,TIMES,SEX,ID,SCORE) "  \
	       "VALUES ('2018-05-08-16-10-13-00-17-20-003-091.jpeg', 'Haha00', 1, 'Unkown', 103, 99.09 );";
	rc = API_sqlite3_exec_insrt_to_table(insert_data);

	rc = API_sqlite3_exec_list_table("COMPANY");

	/* Update */
	API_sqlite3_exec_table_update("COMPANY", "NAME", "Hwang", "ID", "103");

	/* list table */
	rc = API_sqlite3_exec_list_table("COMPANY");

	sqlite3_close(sqlite3_ctx.db);

	return 0;
}
#endif
