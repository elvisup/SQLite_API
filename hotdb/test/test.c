#include <stdio.h>
#include <pthread.h>
#include <dlfcn.h>
#include <hotdb.h>

int main()
{
	HotDB_Get_Version();
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
