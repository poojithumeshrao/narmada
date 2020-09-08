#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>

/* Contains necessary C functions of mysql */
#include <mysql.h>
/**
 * Holds the info to connect to DB and
 * error specifying function for 
 * databse connection handle 
 */
#include "connector.h"

#include "../bmd_assets/bmd.h"

#include "../esb/esb.h"

#define STRING_SIZE 50

#define SELECT_QUERY "SELECT id,sender_id, dest_id, message_type \
                       FROM esb_request WHERE status = 'RECEIVED' "

/*void finish_with_error(MYSQL *con) {

  fprintf(stderr, "Error [%d]: %s \n",mysql_errno(con),mysql_error(con));
  mysql_close(con);

  exit(1);        
}*/

void select_new_esb_request(void)
{

    MYSQL *con; /*database connection handle*/
    /**
   * @brief Allocates or initialises a MYSQL object 
   * suitable for mysql_real_connect() function
   * 
   */
    con = mysql_init(NULL);

    /**
   * @brief Prints error message incase
   * initialisation of con fails.
   */
    if (con == NULL)
    {

        fprintf(stderr, "mysql_init() failed\n");
        exit(1);
    }

    /**
   * @brief Checks if connection is 
   * properly established.
   * 
   */
    if (mysql_real_connect(con, HOST, USER, PASS,
                           DB_NAME, PORT, UNIX_SOCKET, FLAG) == NULL)
    {
        finish_with_error(con);
    }

    MYSQL_STMT *stmt;
    MYSQL_BIND bind[4];
    MYSQL_RES *prepare_meta_result;
    unsigned long length[4];
    int param_count, column_count, row_count;
    int int_data; /* For id */
    char str_data[3][STRING_SIZE];
    bool is_null[4];
    bool error[4];

    /* Prepare a SELECT query to fetch data from esb_request */
    stmt = mysql_stmt_init(con);
    if (!stmt)
    {
        fprintf(stderr, " mysql_stmt_init(), out of memory\n");
        exit(0);
    }
    if (mysql_stmt_prepare(stmt, SELECT_QUERY, strlen(SELECT_QUERY)))
    {
        fprintf(stderr, " mysql_stmt_prepare(), SELECT failed\n");
        fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
        exit(0);
    }
    fprintf(stdout, " prepare, SELECT successful\n");

    /* Get the parameter count from the statement */
    param_count = mysql_stmt_param_count(stmt);
    fprintf(stdout, " total parameters in SELECT: %d\n", param_count);

    if (param_count != 0) /* validate parameter count */
    {
        fprintf(stderr, " invalid parameter count returned by MySQL\n");
        exit(0);
    }

    /* Execute the SELECT query */
    if (mysql_stmt_execute(stmt))
    {
        fprintf(stderr, " mysql_stmt_execute(), failed\n");
        fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
        exit(0);
    }

    /* Fetch result set meta information */
    prepare_meta_result = mysql_stmt_result_metadata(stmt);
    if (!prepare_meta_result)
    {
        fprintf(stderr,
                " mysql_stmt_result_metadata(), \
           returned no meta information\n");
        fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
        exit(0);
    }

    /* Get total columns in the query */
    column_count = mysql_num_fields(prepare_meta_result);
    fprintf(stdout,
            " total columns in SELECT statement: %d\n",
            column_count);

    if (column_count != 4) /* validate column count */
    {
        fprintf(stderr, " invalid column count returned by MySQL\n");
        exit(0);
    }

    /* Bind the result buffers for all 4 columns before fetching them */

    memset(bind, 0, sizeof(bind));

    /* INTEGER COLUMN */
    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = (char *)&int_data;
    bind[0].is_null = &is_null[0];
    bind[0].length = &length[0];
    bind[0].error = &error[0];

    /* Sender_id */
    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = (char *)str_data[0];
    bind[1].buffer_length = STRING_SIZE;
    bind[1].is_null = &is_null[1];
    bind[1].length = &length[1];
    bind[1].error = &error[1];

    /* dest_id */
    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].buffer = (char *)str_data[1];
    bind[2].buffer_length = STRING_SIZE;
    bind[2].is_null = &is_null[2];
    bind[2].length = &length[2];
    bind[2].error = &error[2];

    /* message_type */
    bind[3].buffer_type = MYSQL_TYPE_STRING;
    bind[3].buffer = (char *)str_data[2];
    bind[3].buffer_length = STRING_SIZE;
    bind[3].is_null = &is_null[3];
    bind[3].length = &length[3];
    bind[3].error = &error[3];

    /* Bind the result buffers */
    if (mysql_stmt_bind_result(stmt, bind))
    {
        fprintf(stderr, " mysql_stmt_bind_result() failed\n");
        fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
        exit(0);
    }

    /* Now buffer all results to client (optional step) */
    if (mysql_stmt_store_result(stmt))
    {
        fprintf(stderr, " mysql_stmt_store_result() failed\n");
        fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
        exit(0);
    }

    /* Fetch all rows */
    row_count = 0;
    fprintf(stdout, "Fetching results ...\n");
    while (!mysql_stmt_fetch(stmt))
    {
        row_count++;
        fprintf(stdout, "  row %d\n", row_count);

        /* column 1 */
        fprintf(stdout, "   column1 (integer)  : ");
        if (is_null[0])
            fprintf(stdout, " NULL\n");
        else
            fprintf(stdout, " %d(%ld)\n", int_data, length[0]);

        /* column 2 */
        fprintf(stdout, "   column2 (string)   : ");
        if (is_null[1])
            fprintf(stdout, " NULL\n");
        else
            fprintf(stdout, " %s(%ld)\n", str_data[0], length[1]);

        /* column 3 */
        fprintf(stdout, "   column3 (string) : ");
        if (is_null[2])
            fprintf(stdout, " NULL\n");
        else
            fprintf(stdout, " %s(%ld)\n", str_data[1], length[2]);

        /* column 4 */
        fprintf(stdout, "   column4 (string): ");
        if (is_null[3])
            fprintf(stdout, " NULL\n");
        else
            fprintf(stdout, " %s(%ld)\n", str_data[2], length[3]);
        fprintf(stdout, "\n");
    }

    /* Validate rows fetched */
    fprintf(stdout, " total rows fetched: %d\n", row_count);
    /*if (row_count= 2)
    {
        fprintf(stderr, " MySQL failed to return all rows\n");
        exit(0);
    }*/

    /* Free the prepared result metadata */
    mysql_free_result(prepare_meta_result);

    /* Close the statement */
    if (mysql_stmt_close(stmt))
    {
        /* mysql_stmt_close() invalidates stmt, so call          */
        /* mysql_error(mysql) rather than mysql_stmt_error(stmt) */
        fprintf(stderr, " failed while closing the statement\n");
        fprintf(stderr, " %s\n", mysql_error(con));
        exit(0);
    }

    /*closes the database connection*/
    mysql_close(con);
    /* returns number of rows in result */
    return;
}

int main(int argc, char **argv) {

    select_new_esb_request();
    return 0;
}