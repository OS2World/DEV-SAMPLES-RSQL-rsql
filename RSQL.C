static unsigned char sqla_program_id[40] = 
{111,65,65,66,65,69,67,67,78,70,85,32,32,32,32,32,82,83,81,76,
32,32,32,32,75,65,99,81,79,78,66,75,48,32,32,32,32,32,32,32};


/* Operating System Control Parameters */
#ifndef SQL_API_RC
#define SQL_STRUCTURE struct
#define SQL_API_RC int
#define SQL_POINTER
#ifdef  SQL16TO32
#define SQL_API_FN far pascal _loadds
#else
#define SQL_API_FN _System
#endif
#endif

struct sqlca;
struct sqlda;
SQL_API_RC SQL_API_FN  sqlaaloc(unsigned short,
                                unsigned short,
                                unsigned short,
                                void *);
SQL_API_RC SQL_API_FN  sqlacall(unsigned short,
                                unsigned short,
                                unsigned short,
                                unsigned short,
                                void *);
SQL_API_RC SQL_API_FN  sqladloc(unsigned short,
                                void *);
SQL_API_RC SQL_API_FN  sqlasets(unsigned short,
                                void *,
                                void *);
SQL_API_RC SQL_API_FN  sqlasetv(unsigned short,
                                unsigned short,
                                unsigned short,
                                unsigned short,
                                void *,
                                void *,
                                void *);
SQL_API_RC SQL_API_FN  sqlastop(void *);
SQL_API_RC SQL_API_FN  sqlastrt(void *,
                                void *,
                                struct sqlca *);
SQL_API_RC SQL_API_FN  sqlausda(unsigned short,
                                struct sqlda *,
                                void *);
SQL_API_RC SQL_API_FN  sqlestrd_api(unsigned char *,
                                    unsigned char *,
                                    unsigned char,
                                    struct sqlca *);
SQL_API_RC SQL_API_FN  sqlestpd_api(struct sqlca *);
#line 1 "H:\\IBMWF\\PRJ\\RSQL\\.\\RSQL.SQC"
/*****************************************************************************/
/*  PROGRAM NAME:  RSQL.SQC          -- Run SQL Program                      */
/*                                                                           */
/*  DESCRIPTION:  This program allows a user to run a batch file consisting  */
/*    of SQL and Database Administrator commands against an OS/2 database.   */
/*    The script is submitted to RSQL in a command file and the results      */
/*    appear in a response file.  RSQL runs as an OS/2 task and does not     */
/*    require the services of Query Manager.                                 */
/*                                                                           */
/*  PROGRAM INVOCATION:                                                      */
/*                                                                           */
/*    rsql cmd_file resp_file                                                */
/*                                                                           */
/*         cmd_file  = run file that contains the user script                */
/*         resp_file = response file that contains the output                */
/*                                                                           */
/*****************************************************************************/
#include <process.h>                        /* For exit()                    */
#include <stdio.h>                          /* for printf() and NULL         */
#include <stdlib.h>                         /* for printf()                  */
#include <string.h>                         /* for printf()                  */
#include <sys\types.h>                      /* for _ftime()                  */
#include <sys\timeb.h>                      /* for _ftime()                  */
#include <io.h>                             /* for file I/O                  */
#include <conio.h>                          /* for getch and kbhit           */
#include <fcntl.h>                          /* for file I/O                  */

#define INCL_DOSMEMMGR
#define INCL_DOSPROCESS
#include <os2.h>

#include <sqlca.h>                          /* for sqlca                     */
#include <sql.h>                            /* for error msg retrieval       */
#include <sqlcodes.h>                       /* for SQL return code constants */
#include <sqlenv.h>                         /* for environment functions     */
#include <sqlutil.h>                        /* for utility functions         */
#include <upm.h>                            /* for logon/logoff functions    */
struct sqlca sqlca;                         /* for DB mgr return codes       */

#include "rsql.h"
#pragma stack16(8192)

/*___________________________________________________________________________*/
/* Main function                                                             */
/*___________________________________________________________________________*/
int main(int argc, char **argv)
{
   SHORT cmd_type;
   SHORT ch;

   initialize_rsql();
   check_args_and_prepare_files(argc, argv);

   /*_____________________________________________*/
   /* Process commands one at a time              */
   /*_____________________________________________*/
   while (!feof(cmd_stream))
    /*_____________________________________________*/
    /* get next command                            */
    /*_____________________________________________*/
    {
     cmd_type = get_command(parm_buf);
     /*_____________________________________________*/
     /* Initialize timer                            */
     /*_____________________________________________*/
     reset_timer();
     start_timer();
     /*_____________________________________________*/
     /* Execute the command                         */
     /*_____________________________________________*/
     switch (cmd_type) {
                       /*_____________________________________________*/
                       /* Dynamic SQL commands                        */
                       /*_____________________________________________*/
     case CMD_SQL_IMMEDIATE         : rcd = sql_immediate(parm_buf);     break;
     case CMD_SQL_SELECT            : rcd = sql_select(parm_buf);        break;
                       /*_____________________________________________*/
                       /* Execute an OS/2 program                     */
                       /*_____________________________________________*/
     case CMD_DOS                   : rcd = dos_exec(parm_buf);          break;
                       /*_____________________________________________*/
                       /* Database Administration commands            */
                       /*_____________________________________________*/
     case CMD_DBA                   : rcd = dba_exec(parm_buf);          break;
                       /*_____________________________________________*/
                       /* RSQL execution controls                     */
                       /*_____________________________________________*/
     case CMD_DBA_CONTINUE_ON_ERROR : rcd = RECORD_NOTHING;
                                      continue_on_error = TRUE;          break;
     case CMD_DBA_STOP_ON_ERROR     : rcd = RECORD_NOTHING;
                                      continue_on_error = FALSE;         break;
     case CMD_DBA_PAUSE             : rcd = RECORD_NOTHING;
                                      printf("\nPausing! Press any key "
                                             "to continue...");
                                      while (!kbhit());
                                      ch = getch();
                                      break;
     case CMD_SYNTAX_ERROR          : rcd = RECORD_ERR_MSG;              break;
     case CMD_NO_CMD                : rcd = RECORD_NOTHING;              break;
     default: break;
     } /* endswitch */
     /*_____________________________________________*/
     /* Stop the timer, record the results,         */
     /* terminate command processing if error       */
     /*_____________________________________________*/
     stop_timer();
     if (record_command_results(rcd) == TERMINATE)
        break;

   } /* endwhile */

   terminate_program();

}  /* end main */


/*___________________________________________________________________________*/
/* Perform required program initialization                                   */
/*___________________________________________________________________________*/
VOID initialize_rsql(VOID)
{
     /*_____________________________________________*/
     /* Allocate an SQLDA                           */
     /*_____________________________________________*/
     sqlda = (struct sqlda *)malloc(SQLDASIZE(255));

     /*_____________________________________________*/
     /* Enable the exit list procedure              */
     /*_____________________________________________*/
     DosExitList(1,                       /* 1 = Add to exit list */
                (PFNEXITLIST) exit_proc);  /* pointer to procedure */

} /* end initialize_rsql */


/*___________________________________________________________________________*/
/* Check for the right number of program arguments and open files            */
/*___________________________________________________________________________*/
VOID check_args_and_prepare_files(USHORT argc, PCHAR argv[])
{
     /*_____________________________________________*/
     /* Are the required parameters there?          */
     /*_____________________________________________*/
     if (argc < 3) {
        fprintf(stderr,"\nCalling syntax is: rsql run_file resp_file");
        exit(1);
     } /* endif */
     /*_____________________________________________*/
     /* Open the RUN and RESPONSE files             */
     /*_____________________________________________*/
     if (NULL == (cmd_stream = fopen(argv[1], "r"))) {
        printf("\nError opening the RUN file : %s", argv[1]);
        exit(1);
     } /* endif */
     if (NULL == (out_stream = fopen(argv[2], "w"))) {
        printf("\nError opening the RESPONSE file : %s", argv[2]);
        exit(1);
     } /* endif */

} /* end check_args_and_prepare_file */



/*___________________________________________________________________________*/
/* Cleanup and terminate the program                                         */
/*___________________________________________________________________________*/
VOID terminate_program(VOID)
{
   free(sqlda);
   fclose(cmd_stream);
   fclose(out_stream);
} /* end terminate_program */



/*___________________________________________________________________________*/
/* This procedure displays how the program terminated and performs cleanup   */
/* if abnormal termination.                                                  */
/*___________________________________________________________________________*/
VOID exit_proc(ULONG term_code)
{
   /*_____________________________________________*/
   /* Determine termination cause                 */
   /*_____________________________________________*/
   switch (term_code) {
   case  0:
      printf("\n**program terminated normally**\n");
      break;
   case  1:
      printf("\n**program terminated by hard error**\n");
      break;
   case  2:
      fclose(cmd_stream);
      fclose(out_stream);
      printf("\n**program terminated by a trap**\n");
      break;
   case  3:
      fclose(cmd_stream);
      fclose(out_stream);
      printf("\n**program terminated by a DOSKillProcess**\n");
      break;
   default:
      printf("\n**unknown program termination**\n");
   } /* endswitch */
   /*_____________________________________________*/
   /* Allow next exit list                        */
   /*_____________________________________________*/
   DosExitList(3,                                  /* 3 allows next exit to run */
               (void far *) 0);                    /* null pointer              */
} /* end exit_proc */



/*___________________________________________________________________________*/
/* This procedure determines the command type                                */
/*___________________________________________________________________________*/
SHORT get_command(PCHAR parm_str)
{
  char cmd[MAX_CMD_SIZE];          /* temp buffer for cmd processing */
  char token1[5];                  /* token to receive EXEC          */
  char token2[5];                  /* token to receive DBA: or SQL:  */
  char token3[25];                 /* token to receive cmd name      */
  char token4[MAX_CMD_SIZE];       /* token to receive parms         */
  char comment_line[MAX_CMD_SIZE]; /* comment line temp variable     */
  char delim1,delim2;              /* variables for sscanf           */

  /*_____________________________________________*/
  /* Initialize cmds table                       */
  /*_____________________________________________*/
  typedef struct
  {
    CHAR    cmd_name[40];            /*  cmd name                   */
    CHAR    cmd_num;                 /* numerical representation    */
    USHORT  num_parms;               /* no. of parms for this cmd   */
  }   CMD_TYPE;

  static CMD_TYPE cmds[] =
  {/*<=== cmd_name ---><-- cmd_num -----------------><-- num_parms -->*/
   {"BACKUP",           CMD_DBA_BACKUP             ,  3               },
   {"BIND",             CMD_DBA_BIND               ,  6               },
   {"CATALOG_DB",       CMD_DBA_CATALOG_DB         ,  6               },
   {"CATALOG_NODE",     CMD_DBA_CATALOG_NODE       ,  4               },
   {"COLLECT_STATUS",   CMD_DBA_COLLECT_STATUS     ,  1               },
   {"CONTINUE_ON_ERROR",CMD_DBA_CONTINUE_ON_ERROR  ,  0               },
   {"CREATE_DB",        CMD_DBA_CREATE_DB          ,  3               },
   {"DROP_DB",          CMD_DBA_DROP_DB            ,  1               },
   {"EXPORT",           CMD_DBA_EXPORT             ,  4               },
   {"GET_ADMIN",        CMD_DBA_GET_ADMIN          ,  0               },
   {"IMPORT",           CMD_DBA_IMPORT             ,  5               },
   {"LOGOFF",           CMD_DBA_LOGOFF             ,  3               },
   {"LOGON",            CMD_DBA_LOGON              ,  5               },
   {"PAUSE",            CMD_DBA_PAUSE              ,  0               },
   {"REORG",            CMD_DBA_REORG              ,  3               },
   {"RESET_DM_CFG",     CMD_DBA_RESET_DM_CFG       ,  0               },
   {"RESET_DB_CFG",     CMD_DBA_RESET_DB_CFG       ,  1               },
   {"RESTORE",          CMD_DBA_RESTORE            ,  3               },
   {"ROLL_FORWARD",     CMD_DBA_ROLL_FORWARD       ,  3               },
   {"RUNSTATS",         CMD_DBA_RUNSTATS           ,  1               },
   {"SHOW_DB_CFG",      CMD_DBA_SHOW_DB_CFG        ,  1               },
   {"SHOW_DB_DIR",      CMD_DBA_SHOW_DB_DIR        ,  1               },
   {"SHOW_DM_CFG",      CMD_DBA_SHOW_DM_CFG        ,  0               },
   {"SHOW_NODE_DIR",    CMD_DBA_SHOW_NODE_DIR      ,  0               },
   {"STARTDM",          CMD_DBA_STARTDM            ,  0               },
   {"STARTUSE",         CMD_DBA_STARTUSE           ,  2               },
   {"STOPDM",           CMD_DBA_STOPDM             ,  0               },
   {"STOP_ON_ERROR",    CMD_DBA_STOP_ON_ERROR      ,  0               },
   {"STOPUSE",          CMD_DBA_STOPUSE            ,  0               },
   {"UNCATALOG_DB",     CMD_DBA_UNCATALOG_DB       ,  1               },
   {"UNCATALOG_NODE",   CMD_DBA_UNCATALOG_NODE     ,  1               },
   {"UPDATE_DB_CFG",    CMD_DBA_UPDATE_DB_CFG      ,  2               },
   {"UPDATE_DM_CFG",    CMD_DBA_UPDATE_DM_CFG      ,  0               },
  };
  USHORT      cmd_not_found;     /* True if a match is found          */
  USHORT      i;                 /* Loop index variable               */
  PCHAR       char_ptr;          /* used to remove CRLF from SQL stmts*/
  /*_____________________________________________*/
  /* Read a command from the RUN file            */
  /*_____________________________________________*/
   num_parms = fscanf(cmd_stream," %[^;]%*c ", cmd);
   if (num_parms == EOF) {
      return(CMD_NO_CMD);                       /* empty file          */
   strcat(cmd,";");                             /* maintain cmd delim. */
   } /* endif */
   /*_____________________________________________*/
   /* Echo command to the response file           */
   /*_____________________________________________*/
   fprintf(out_stream, "%s\n", cmd);

   /*_____________________________________________*/
   /* Remove any comment lines                    */
   /*_____________________________________________*/
   while ((cmd[0] == '-') && (cmd[1] == '-')) {
      num_parms = sscanf(cmd,"%[^\n;]%c %[^;]%c ",
                  comment_line, &delim1, cmd, &delim2);  /* remove comment line */
      if (delim1 == ';') {
         return (CMD_NO_CMD);                            /* maintain cmd delim */
      } /* endif */
   } /* endwhile */

   /*_____________________________________________*/
   /* Parse the command and make some validity    */
   /* checks                                      */
   /*_____________________________________________*/
   num_parms = sscanf(cmd, " %4s %4s %24s %[^;]",
                         token1, token2, token3, token4);

   if (num_parms == 3) {                                 /* no parms, parse accordingly */
      num_parms = sscanf(cmd, " %4s %4s %24[^;]",
                         token1, token2, token3);
      strcpy(token4,"");
   } /* endif */

   if ((num_parms <3)                      ||
       (strcmp(token1,"EXEC") != 0)        ||
       ((strcmp(token2,"DBA:") != 0)       &&
        (strcmp(token2,"DOS:") != 0)       &&
        (strcmp(token2,"SQL:") != 0)))     {
      sprintf(err_msg,
      "\nSYNTAX ERROR: Expected EXEC (SQL or DBA etc...) to begin a command.");
      return(CMD_SYNTAX_ERROR);
   } /* endif */

   /*_____________________________________________*/
   /* SQL CMD, return command type,               */
   /* remove EXEC SQL                             */
   /*_____________________________________________*/
     if (strcmp(token2,"SQL:") == 0) {
        char_ptr = strpbrk(token4,"\n\r");              /* remove CRLF chars */
        while (char_ptr != NULL) {
           char_ptr[0] = ' ';
           char_ptr = strpbrk(char_ptr,"\n\r");
        } /* endwhile */
        sprintf(parm_str, "%s %s", token3, token4);     /* return the cmd for */
        if (strcmp(strupr(token3),"SELECT") == 0) {     /* for processing     */
           return(CMD_SQL_SELECT);
        } else {
           return(CMD_SQL_IMMEDIATE);
        } /* endif */
     } /* endif */

   /*_____________________________________________*/
   /* DOS CMD, return command type,               */
   /* remove EXEC DOS                             */
   /*_____________________________________________*/
   if (strcmp(token2,"DOS:") == 0) {
      sprintf(parm_str, "%s%c%s", token3, 0, token4);   /* return the cmd     */
                                                        /* in the form for    */
                                                        /* DOSExecPgm         */
      return(CMD_DOS);
   } /* endif */

   /*_____________________________________________*/
   /* DBA CMD, return command type,               */
   /* remove EXEC DBA: cmd_name"                  */
   /*_____________________________________________*/
   i=0;
   cmd_not_found = TRUE;

   while (cmd_not_found && (i < CMD_LAST_CMD)) {
      cmd_not_found = strcmp(cmds[i].cmd_name,token3);
      if (!cmd_not_found) {
         break;
      } else {
         i++;
      } /* endif */
   } /* endwhile */

   if (cmd_not_found) {
      sprintf(err_msg,
      "\nSYNTAX ERROR: Unknown Command. ");
      return(CMD_SYNTAX_ERROR);
   } /* endif */

   if ((cmds[i].cmd_num == CMD_DBA_CONTINUE_ON_ERROR) ||
       (cmds[i].cmd_num == CMD_DBA_STOP_ON_ERROR) ||
       (cmds[i].cmd_num == CMD_DBA_PAUSE))  {
      return(cmds[i].cmd_num);
   } /* endif */

   sprintf(parm_str, "%s", token4);
   strcpy(parms.cmd_name,cmds[i].cmd_name);
   parms.cmd_num = cmds[i].cmd_num;
   parms.num_parms = cmds[i].num_parms;
   return(CMD_DBA);

} /* end get_command */



/*___________________________________________________________________________*/
/* This procedure initializes the time variable                              */
/*___________________________________________________________________________*/
VOID  reset_timer(VOID)
{
   event_time = 0.0;
   start_time = 0.0;
   end_time   = 0.0;
} /* end reset_timer */



/*___________________________________________________________________________*/
/* This procedure records the time at the start of an interval               */
/*___________________________________________________________________________*/
VOID  start_timer(VOID)
{
   _ftime(&timebuff);
   start_time=(double)timebuff.time+((double)timebuff.millitm)/(double)1000;
} /* end start_timer */



/*___________________________________________________________________________*/
/* This procedure records the time at the end of an interval                 */
/* and calculates the elapsed time for an event.                             */
/*___________________________________________________________________________*/
VOID  stop_timer(VOID)
{
   _ftime(&timebuff);
   end_time=(double)timebuff.time+((double)timebuff.millitm)/(double)1000;
   event_time = event_time + (end_time - start_time);

} /* end stop_timer */




/*___________________________________________________________________________*/
/* This procedure executes an SQL command dynamically (non SELECT)           */
/*___________________________________________________________________________*/
SHORT sql_immediate(PCHAR cmd_sql)
{
   /*_____________________________________________*/
   /* Declare cmd to DB2/2                        */
   /*_____________________________________________*/
   
/*
EXEC SQL BEGIN DECLARE SECTION;
*/
#line 436 "H:\\IBMWF\\PRJ\\RSQL\\.\\RSQL.SQC"

      char     *  cmd;
   
/*
EXEC SQL END DECLARE SECTION;
*/
#line 438 "H:\\IBMWF\\PRJ\\RSQL\\.\\RSQL.SQC"

   cmd = cmd_sql;
   /*_____________________________________________*/
   /* Execute cmd                                 */
   /*_____________________________________________*/
   
/*
EXEC SQL EXECUTE IMMEDIATE :cmd;
*/

{
  sqlastrt(sqla_program_id,0L,&sqlca);
  sqlasets(strlen(cmd),cmd,0L);
  sqlacall((unsigned short)23,1,0,0,0L);
  sqlastop(0L);
}
#line 443 "H:\\IBMWF\\PRJ\\RSQL\\.\\RSQL.SQC"


   return (RECORD_SQLCA);
} /* end sql_immediate */




/*___________________________________________________________________________*/
/* This procedure executes an SQL command dynamically (SELECT)               */
/*___________________________________________________________________________*/
SHORT sql_select(PCHAR cmd)
{
   SHORT    index, i;                           /* for counting                  */
   SHORT    num_of_rows;                        /* number of rows selected       */
   SHORT    num_brk;                            /* number of rows since break    */
   SHORT    byte_count;                         /* number of bytes to allocate   */
   /*_____________________________________________*/
   /* Declare cmd to DB2/2                        */
   /*_____________________________________________*/
   
/*
EXEC SQL BEGIN DECLARE SECTION;
*/
#line 463 "H:\\IBMWF\\PRJ\\RSQL\\.\\RSQL.SQC"

      char     *  cmd_sql;                      /* command buffer                */
   
/*
EXEC SQL END DECLARE SECTION;
*/
#line 465 "H:\\IBMWF\\PRJ\\RSQL\\.\\RSQL.SQC"


   cmd_sql     = cmd;
   num_of_rows = 0;
   num_brk     = 0;

   /*_____________________________________________*/
   /* Initialize the SQLDA                        */
   /*_____________________________________________*/
   strncpy(sqlda->sqldaid,"SQLDA   ",8);
   sqlda->sqldabc = (long) SQLDASIZE(255);
   sqlda->sqln = SQLVAR_NUM;
   sqlda->sqld = 0;

   /*_____________________________________________*/
   /* Prepare cmd                                 */
   /*_____________________________________________*/
   
/*
EXEC SQL PREPARE st into :*sqlda FROM :cmd_sql;
*/

{
  sqlastrt(sqla_program_id,0L,&sqlca);
  sqlausda(3,&*sqlda,0L);
  sqlasets(strlen(cmd_sql),cmd_sql,0L);
  sqlacall((unsigned short)27,2,0,3,0L);
  sqlastop(0L);
}
#line 482 "H:\\IBMWF\\PRJ\\RSQL\\.\\RSQL.SQC"

   if (SQLCODE != 0) {
      return(RECORD_SQLCA);
   } /* endif */

   /*_____________________________________________*/
   /* Stop timer                                  */
   /*_____________________________________________*/
   stop_timer();

   /*_____________________________________________*/
   /* Allocate space for select variables         */
   /*_____________________________________________*/
   for (index=0;index<sqlda->sqld ;index++ ) {
      if ((sqlda->sqlvar[index].sqltype == SQL_TYP_DECIMAL) ||
          (sqlda->sqlvar[index].sqltype == SQL_TYP_NDECIMAL))  {
         byte_count = (LOBYTE(sqlda->sqlvar[index].sqllen) + 2)/2;
      } else {
         byte_count = sqlda->sqlvar[index].sqllen;
      } /* endif */

      sqlda->sqlvar[index].sqldata = (unsigned char *)malloc(byte_count);
      if (sqlda->sqlvar[index].sqldata == NULL) {
         fprintf(out_stream,
         "\nOut of dynamic memory while space for select column %d",index+1);
         return(RECORD_ERR_MSG);
      } /* endif */

      /*_____________________________________________*/
      /* Allocate space for null indicators          */
      /*_____________________________________________*/
      if (sqlda->sqlvar[index].sqltype & 1) {         /* odd   */
         sqlda->sqlvar[index].sqlind = (short *)malloc(2);
         if (sqlda->sqlvar[index].sqldata == NULL) {
            fprintf(out_stream,
            "\nOut of dynamic memory while space for NULL column %d", index+1);
            return(RECORD_ERR_MSG);
         } /* endif */
      } /* endif */
   } /* endfor */

   rcd = print_format_row(SET_TABS_WIDTH);
   if (rcd != 0) {
      return(RECORD_ERR_MSG);
   } /* endif */

   rcd = print_format_row(PRINT_COLUMNS);
   if (rcd != 0) {
      return(RECORD_ERR_MSG);
   } /* endif */

   /*_____________________________________________*/
   /* Start timer                                 */
   /*_____________________________________________*/

   start_timer();

   
/*
EXEC SQL DECLARE cur CURSOR for st;
*/
#line 539 "H:\\IBMWF\\PRJ\\RSQL\\.\\RSQL.SQC"

   if (SQLCODE != 0) {
      return(RECORD_SQLCA);
   } /* endif */

   
/*
EXEC SQL OPEN cur;
*/

{
  sqlastrt(sqla_program_id,0L,&sqlca);
  sqlacall((unsigned short)26,2,0,0,0L);
  sqlastop(0L);
}
#line 544 "H:\\IBMWF\\PRJ\\RSQL\\.\\RSQL.SQC"

   if (SQLCODE != 0) {
      return(RECORD_SQLCA);
   } /* endif */

   /*_____________________________________________*/
   /* Handle data one row at a time               */
   /*_____________________________________________*/
   
/*
EXEC SQL FETCH cur USING DESCRIPTOR :*sqlda;
*/

{
  sqlastrt(sqla_program_id,0L,&sqlca);
  sqlausda(3,&*sqlda,0L);
  sqlacall((unsigned short)25,2,0,3,0L);
  sqlastop(0L);
}
#line 552 "H:\\IBMWF\\PRJ\\RSQL\\.\\RSQL.SQC"

   if ((SQLCODE !=0) && (SQLCODE != SQL_RC_W100)) {
      return(RECORD_SQLCA);
   } /* endif */

   while (SQLCODE == 0l) {
      stop_timer();
      num_of_rows++;
      if (num_brk++ > BREAK_SEP) {
         for (i=0;i<BREAK_ROWS ;i++ ) {
            fputs("\n",out_stream);
         } /* endfor */
         print_format_row(PRINT_COLUMNS);
         num_brk = 0;
      } /* endif */
      rcd=print_format_row(PRINT_SELECTED_ROW);
      if (rcd != 0) {
         return(RECORD_ERR_MSG);
      } else {
         start_timer();
         
/*
EXEC SQL FETCH cur USING DESCRIPTOR :*sqlda;
*/

{
  sqlastrt(sqla_program_id,0L,&sqlca);
  sqlausda(3,&*sqlda,0L);
  sqlacall((unsigned short)25,2,0,3,0L);
  sqlastop(0L);
}
#line 572 "H:\\IBMWF\\PRJ\\RSQL\\.\\RSQL.SQC"

         if ((SQLCODE !=0) && (SQLCODE != SQL_RC_W100)) {
            return(RECORD_SQLCA);
         } /* endif */
      } /* endif */
   } /* endwhile */

   
/*
EXEC SQL CLOSE cur;
*/

{
  sqlastrt(sqla_program_id,0L,&sqlca);
  sqlacall((unsigned short)20,2,0,0,0L);
  sqlastop(0L);
}
#line 579 "H:\\IBMWF\\PRJ\\RSQL\\.\\RSQL.SQC"

   if (SQLCODE != 0) {
      return(RECORD_SQLCA);
   } /* endif */

   /*_____________________________________________*/
   /* Stop timer                                  */
   /*_____________________________________________*/

   stop_timer();
   fprintf(out_stream,"\n\n Total Number of Rows SELECTED = %d\n",num_of_rows);

   /*_____________________________________________*/
   /* Free space for select variables             */
   /*_____________________________________________*/
   for (index=0;index < sqlda->sqld ;index++ ) {
      free(sqlda->sqlvar[index].sqldata);

   /*_____________________________________________*/
   /* Free space for null indicators              */
   /*_____________________________________________*/
      if (sqlda->sqlvar[index].sqltype & 1) {                  /*   odd    */
         free(sqlda->sqlvar[index].sqlind);
      } /* endif */
   } /* endfor */

   return(RECORD_SQLCA);

} /* end sql_select */




/*___________________________________________________________________________*/
/*  This procedure performs the requested format actions:                    */
/*                                                                           */
/*  SET_TABS_WIDTH calculates the tab position for each field in a query     */
/*  PRINT_COLUMNS  formats one row of columns using the tabs and sqlda       */
/*  PRINT_SELECTED_ROW                                                       */
/*                 formats one SELECT row using the tabs and sqlda           */
/*___________________________________________________________________________*/
SHORT print_format_row(USHORT action)
{
   #define      MAX_FLDS     50              /* for this exercise             */
   SHORT        index, i, y;                 /*  for counting                 */
   CHAR         row[MAX_LINE_SIZE+1];        /* row to be printed             */
   CHAR         temp[MAX_LINE_SIZE+10];      /* temp buffer                   */
   CHAR         temp1[100];                  /* 2nd temp buffer               */
   typedef struct
   {
      SHORT    tab_pos;                      /* column position               */
      SHORT    max_width;                    /* max_width str width of type   */
   }  FLD_TYPE;
   static FLD_TYPE   field[MAX_FLDS];        /* contains field descriptors    */
   SHORT          col_name_width;            /* width of col name             */
   SHORT          col_fld_width;             /* width of col fld              */
   static SHORT   max_num_flds;              /* number of printable columns   */
   SHORT          null_not_found;            /* a flag                        */
   USHORT         top, precision, scale;     /* for decimal fields            */
   SHORT          bottom, point;             /* for decimal fields            */
   SHORT          len;                       /* for decimal fields            */
   UCHAR          *ptr;                      /* for decimal fields            */
   BOOL           output_blank;              /* for decimal fields            */

   /*_____________________________________________*/
   /* Initialize                                  */
   /*_____________________________________________*/
   memset(row,' ',sizeof(row));
   row[sizeof(row) - 1] = '\0';
   i = 0;
   rcd = GOOD;

   /*_____________________________________________*/
   /* Do action                                   */
   /*_____________________________________________*/
   switch (action) {
   case  SET_TABS_WIDTH:
      field[0].tab_pos = 1;                  /* Start with column 1           */
      for (index=0;index < sqlda->sqld ;index++ ) {
         col_name_width = sqlda->sqlvar[index].sqlname.length;
         col_fld_width  = sqlda->sqlvar[index].sqllen;

         switch (sqlda->sqlvar[index].sqltype) {
         case SQL_TYP_INTEGER:
         case SQL_TYP_NINTEGER:
         case SQL_TYP_FLOAT:
         case SQL_TYP_NFLOAT:
            col_fld_width = 11;
            break;
         case SQL_TYP_SMALL:
         case SQL_TYP_NSMALL:
            col_fld_width = 6;
            break;
         case SQL_TYP_DECIMAL:
         case SQL_TYP_NDECIMAL:
            col_fld_width = LOBYTE(sqlda->sqlvar[index].sqllen) + 1;
                                             /* precision + decimal point     */
            break;
         case SQL_TYP_DATE:
         case SQL_TYP_NDATE:
            col_fld_width=10;
            break;
         case SQL_TYP_TIME:
         case SQL_TYP_NTIME:
            col_fld_width=8;
            break;
         case SQL_TYP_STAMP:
         case SQL_TYP_NSTAMP:
            col_fld_width=26;
            break;
         default:
            break;
         } /* endswitch */
         /*_____________________________________________*/
         /* Use the larger of col_name or col_fld       */
         /*_____________________________________________*/
         if (col_name_width > col_fld_width) {
            field[index].max_width = col_name_width;
         } else {
            field[index].max_width = col_fld_width;
         } /* endif */
         /*_____________________________________________*/
         /* Update tab field 1 space between columns    */
         /*_____________________________________________*/
         field[index+1].tab_pos = field[index].tab_pos +
                                    field[index].max_width +
                                    COL_SEP;
         /*_____________________________________________*/
         /* Check Limits                                */
         /*_____________________________________________*/
         if (field[index+1].tab_pos > MAX_LINE_SIZE) {
            max_num_flds = index;
            break;                           /*  out of for loop              */
         } else {
            max_num_flds = index + 1;
         } /* endif */

         if (max_num_flds > MAX_FLDS) {
            break;                           /*  out of for loop              */
         } /* endif */

      } /* endfor */

      break;                                 /* end of action SET_TABS_WIDTH  */

   case PRINT_COLUMNS:
      /*_____________________________________________*/
      /* Print column names                          */
      /*_____________________________________________*/
      for (index=0;index < max_num_flds ;index++ ) {
         memcpy(&row[field[index].tab_pos],
                sqlda->sqlvar[index].sqlname.data,
                sqlda->sqlvar[index].sqlname.length);
      } /* endfor */
      fprintf(out_stream,"\n%s",row);

      /*_____________________________________________*/
      /* Clear row buffer                            */
      /*_____________________________________________*/
      memset(row,' ',sizeof(row));
      row[sizeof(row) +1] = '\0';

      /*_____________________________________________*/
      /* Underline names                             */
      /*_____________________________________________*/
      row[sizeof(row) -1] = '\0';
      for (index=0;index<max_num_flds ;index++ ) {
         memset(temp,'-',field[index].max_width);
         memcpy(&row[field[index].tab_pos], temp, field[index].max_width);
      } /* endfor */
      fprintf(out_stream,"\n%s",row);

      break;                                 /* end of action PRINT_COLUMNS   */

   case PRINT_SELECTED_ROW:
      for (index=0;index<max_num_flds ;index++ ) {
         /*_____________________________________________*/
         /* Initialize                                  */
         /*_____________________________________________*/
         null_not_found = TRUE;
         /*_____________________________________________*/
         /* Is it null?                                 */
         /*_____________________________________________*/
         if (sqlda->sqlvar[index].sqltype & 1) {
            if (* (sqlda->sqlvar[index].sqlind) == -1) {
               sprintf(temp,"NULL");
               null_not_found = FALSE;
            } else {
               if (* (sqlda->sqlvar[index].sqlind) != 0) {
                  sprintf(err_msg,"\n Truncated field col = %d",index+1);
                  rcd=FAILED;
                  break;
               } /* endif */
            } /* endif */
         } /* endif */
         /*_____________________________________________*/
         /* Is not null field                           */
         /*_____________________________________________*/
         if (null_not_found) {
            switch (sqlda->sqlvar[index].sqltype) {
            case SQL_TYP_INTEGER:
            case SQL_TYP_NINTEGER:
               sprintf(temp,"%ld", *(PLONG)sqlda->sqlvar[index].sqldata);
               break;
            case SQL_TYP_SMALL:
            case SQL_TYP_NSMALL:
               sprintf(temp,"%d", *(PSHORT)sqlda->sqlvar[index].sqldata);
               break;
            case SQL_TYP_DECIMAL:
            case SQL_TYP_NDECIMAL:
               len = sqlda->sqlvar[index].sqllen;
               ptr = sqlda->sqlvar[index].sqldata;
               sprintf(temp,"");
               scale     = HIBYTE(len);
               precision = LOBYTE(len);
               y = (precision+2)/2;          /* total number of bytes         */
               point = precision - scale;
               bottom = *(ptr + y - 1) & 0x000F;    /* sign                    */
               if ((bottom == 0x000D) || (bottom == 0x000B)) {
                  strcat(temp,"-");
               } /* endif */
               output_blank = TRUE;
               for (i=0;i<y ;i++ ) {
                  top = *(ptr + i) & 0x00F0;
                  top = (top >> 4);
                  bottom = *(ptr + i) &0x000F;
                  if (point-- == 0) {
                     strcat(temp,".");
                  } /* endif */

                  if (((output_blank) && (top != 0)) || (point <= 1)) {
                     output_blank = FALSE;
                  } /* endif */
                  if (output_blank) {
                     strcat(temp," ");
                  } else {
                     sprintf(temp1,"%d",top);
                     strcat(temp,temp1);
                  } /* endif */
                  /*_____________________________________________*/
                  /* Ignore bottom half of last half-byte        */
                  /* because it's the sign.                      */
                  /*_____________________________________________*/
                  if (i < (y-1)) {
                     /* sign half byte? */
                     if (point-- == 0) {
                        strcat(temp,".");
                     } /* endif */
                     if ((output_blank) && (bottom != 0) || (point <= 1)) {
                        output_blank = FALSE;
                     } /* endif */
                     if (output_blank) {
                        strcat(temp," ");
                     } else {
                        sprintf(temp1,"%d",bottom);
                        strcat(temp,temp1);
                     } /* endif */
                  } /* endif */
               } /* endfor */
               if (scale == 0) {
                  strcat(temp,".");
               } /* endif */
               break;
            case SQL_TYP_FLOAT:
            case SQL_TYP_NFLOAT:
               sprintf(temp,"%e",*(float *)sqlda->sqlvar[index].sqldata);
               break;
            case SQL_TYP_CHAR:
            case SQL_TYP_NCHAR:
               strncpy(temp,
                       sqlda->sqlvar[index].sqldata,
                       sqlda->sqlvar[index].sqllen);
               temp[sqlda->sqlvar[index].sqllen] = '\0';       /* terminate   */
               break;
            case SQL_TYP_VARCHAR:
            case SQL_TYP_NVARCHAR:
            case SQL_TYP_LONG:
            case SQL_TYP_NLONG:
               strncpy(temp,
                       sqlda->sqlvar[index].sqldata + 2,
                       (SHORT)*sqlda->sqlvar[index].sqldata);
               temp[(SHORT)*sqlda->sqlvar[index].sqldata] = '\0'; /* terminate */
               break;
            case SQL_TYP_DATE:
            case SQL_TYP_NDATE:
               strncpy(temp,
                       sqlda->sqlvar[index].sqldata,10);
               temp[10] = '\0';                              /* terminate   */
               break;
            case SQL_TYP_TIME:
            case SQL_TYP_NTIME:
               strncpy(temp,
                       sqlda->sqlvar[index].sqldata,8);
               temp[8] = '\0';                              /* terminate   */
               break;
            case SQL_TYP_STAMP:
            case SQL_TYP_NSTAMP:
               strncpy(temp,
                       sqlda->sqlvar[index].sqldata,26);
               temp[26] = '\0';                             /* terminate   */
               break;
            case SQL_TYP_LSTR:
            case SQL_TYP_NLSTR:
               strncpy(temp,
                       sqlda->sqlvar[index].sqldata + 1,
                       sqlda->sqlvar[index].sqllen);
               temp[sqlda->sqlvar[index].sqllen] = '\0';         /* terminate */
               break;
            case SQL_TYP_CSTR:   /* null-terminated varying length string     */
            case SQL_TYP_NCSTR:
               sprintf(temp,"%s",sqlda->sqlvar[index].sqldata + 2);
               break;
            default:
               sprintf(err_msg,
               "\n Unknown type col = %d type = %d",
               index+1,sqlda->sqlvar[index].sqltype);
               rcd = FAILED;
               break;
            } /* endswitch */
         } /* endif */

         if (rcd == FAILED) {          /* break out of for loop               */
            break;
         } /* endif */
         if (strlen(temp) <= field[index].max_width) {
            memcpy(&row[field[index].tab_pos],temp,strlen(temp));
         } else {
            sprintf(err_msg,
            "\n Field too long col = %d value = %s",(index+1),temp);
            rcd = FAILED;
            break;
         } /* endif */
      } /* endfor */

      /*_____________________________________________*/
      /* Print selected row                          */
      /*_____________________________________________*/
      fputs("\n",out_stream);
      fputs(row,out_stream);
      break;
   } /* endswitch */

   return(rcd);

} /* end print_format_row */




/*___________________________________________________________________________*/
/* This procedure invokes an OS/2 program and passes it parameters           */
/*___________________________________________________________________________*/
SHORT dos_exec(PCHAR cmd_dos)
{
      /*_____________________________________________*/
      /* Execute the program                         */
      /*_____________________________________________*/
   api_rcd = DosExecPgm(object_name,      /* failing object if program fails  */
                  sizeof(object_name),    /* the object size                  */
                  0,                      /* sync. program execution          */
                  cmd_dos,                /* program name and parameters      */
                  NULL,                   /* inherit environment              */
                  &ExecResult,            /* termination reason code          */
                  cmd_dos);               /* program name                     */
   if (api_rcd) {
      sprintf(err_msg,
               "\n ERROR: DosExecPgm API call failed with %d return code",
               api_rcd);
      return(RECORD_DOS_EXEC_RESULTS);
   } /* endif */
      /*_____________________________________________*/
      /* Format the return codes                     */
      /*_____________________________________________*/

   switch (ExecResult.codeTerminate) {
   case 0:
      sprintf(err_msg,"\n Program completed with a return code of %d.",
               ExecResult.codeResult);
      api_rcd = ExecResult.codeResult;
      break;
   case 1:
      sprintf(err_msg,"\n Program terminated by hard error.");
      api_rcd = ExecResult.codeTerminate;
      break;
   case 2:
      sprintf(err_msg,"\n Program terminated by a TRAP.");
      api_rcd = ExecResult.codeTerminate;
      break;
   case 3:
      sprintf(err_msg,"\n Program terminated by a DosKillProcess.");
      api_rcd = ExecResult.codeTerminate;
      break;
   default:
      sprintf(err_msg,"\n Unknown Program termination code.");
      api_rcd = ExecResult.codeTerminate;
   } /* endswitch */

   return(RECORD_DOS_EXEC_RESULTS);

} /* end dos_exec */



/*___________________________________________________________________________*/
/* This procedure records the command results along with the elapsed         */
/* time of execution of a command.                                           */
/*___________________________________________________________________________*/
SHORT record_command_results(SHORT rcd)
{
   CHAR           display_msg[1000];         /* temporary buffer              */
   CHAR           msgbuf[512];               /* rcd explanation               */
   SHORT          index;                     /* for counting                  */
   CHAR           temp[20];                  /* char buffer                   */
   CHAR           *err_line;                 /* a line of error info.         */

   switch (rcd) {
   case RECORD_NOTHING:
      fprintf(out_stream,"\n\n");
      return(CONTINUE);
      break;
   case RECORD_DOS_EXEC_RESULTS:
      if (api_rcd == 0) {
         fprintf(out_stream,"%s",err_msg);
         fprintf(out_stream,
                  "\n OK.                       "
                  "Execution time = %10.4lf secs\n\n\n",
                  (double)event_time);
         return(CONTINUE);
      } else {
         fprintf(out_stream,"%s",err_msg);
         fprintf(out_stream,
                  "\n ERROR.                    "
                  "Execution time = %10.4lf secs\n\n\n",
                  (double)event_time);
         if (continue_on_error) {
            return(CONTINUE);
         } else {
            return(TERMINATE);
         } /* endif */
      } /* endif */
      break;
   case RECORD_SQLCA:
      if (SQLCODE == 0) {
         fprintf(out_stream,"%s",err_msg);
         fprintf(out_stream,
                  "\n OK. sql_rcd = %ld         "
                  "Execution time = %10.4lf secs\n\n\n",
                  SQLCODE,(double)event_time);
         return(CONTINUE);
      } else {
         if (SQLCODE > 0) {
            sprintf(display_msg,
            "\n WARNING, sql_code =%ld,    Execution time = %10.4lf secs",
            SQLCODE,(double)event_time);
         } else {
            sprintf(display_msg,
            "\n ERROR, sql_code = %ld\n",
            SQLCODE);
         } /* endif */
         /*_____________________________________________*/
         /* Retrieve error message                      */
         /*_____________________________________________*/
         rcd = sqlaintp(msgbuf,              /* buffer for msg text           */
                        512,                 /* buffer size                   */
                        79,                  /* line width                    */
                        &sqlca);             /* SQLCA                         */

         if (rcd < 0) {                      /* message retrieve error        */
            sprintf(msgbuf,"\n SQLAINTP ERROR, Return code = %d \n",rcd);
         } /* endif */

         err_line = strtok(msgbuf,"\n");
         while (err_line != NULL) {
            sprintf(display_msg,"%s\n %s",display_msg,err_line);
            err_line = strtok(NULL,"\n");
         } /* endwhile */
         strcat(display_msg,"\n\n ------ ADDITIONAL SQLCA Info --------");
         fputs(display_msg,out_stream);

         /*_____________________________________________*/
         /* Display SQLCA sqlerrmc                      */
         /*_____________________________________________*/
         if (sqlca.sqlerrml) {
            sqlca.sqlerrmc[sqlca.sqlerrml] = '\0';    /* NULL terminate       */
            while (sqlca.sqlerrml--) {
               if (sqlca.sqlerrmc[sqlca.sqlerrml] == 0xFF) {   /* remove FFs  */
                  sqlca.sqlerrmc[sqlca.sqlerrml] = ' ';
               } /* endif */
            } /* endwhile */
            fprintf(out_stream,"\n SQLERRMC's:  %s",sqlca.sqlerrmc);
         } /* endif */

         /*_____________________________________________*/
         /* Display SQLCA sqlerrp                       */
         /*_____________________________________________*/
         sprintf(display_msg,"\n SQLERRP: ");
         for (index=0;index<8 ;index++ ) {
            sprintf(temp,"%c",sqlca.sqlerrp[index]);
            strcat(display_msg,temp);
         } /* endfor */
         fputs(display_msg,out_stream);

         /*_____________________________________________*/
         /* Display SQLCA sqlerrd                       */
         /*_____________________________________________*/
         sprintf(display_msg,"\n SQLERRD: ");
         for (index=0;index<6 ;index++ ) {
            sprintf(temp,"%10ld",sqlca.sqlerrd[index]);
            strcat(display_msg,temp);
         } /* endfor */
         fputs(display_msg,out_stream);

         /*_____________________________________________*/
         /* Display SQLCA warnings                      */
         /*_____________________________________________*/
         sprintf(display_msg,"\n Warning Indicators: ");
         for (index=0;index<5 ;index++ ) {
            sprintf(temp,"%c",sqlca.sqlwarn[index]);
            strcat(display_msg,temp);
         } /* endfor */
         fprintf(out_stream,"%s\n\n\n",temp);

         if (continue_on_error) {
            return(CONTINUE);
         } else {
            return(TERMINATE);
         } /* endif */

      } /* endif */
      break;
   case RECORD_API_RCD:
      if (api_rcd == 0) {
         fprintf(out_stream,
         "\n OF, api_rcd = %ld,         "
         " Execution time = %10.4lf secs\n\n\n",
         api_rcd,(double)event_time);
         return(CONTINUE);
      } else {
         fprintf(out_stream,
         "\n API for command %s failed with "
         "a return code of %d.\n\n\n",
         parms.cmd_name,api_rcd);
      } /* endif */
      if (continue_on_error) {
         return(CONTINUE);
      } else {
         return(TERMINATE);
      } /* endif */
      break;
   case RECORD_ERR_MSG:
      fprintf(out_stream," %s\n\n\n", err_msg);
      return(TERMINATE);
      break;
   } /* endswitch */
} /* end record_command_results */



/*___________________________________________________________________________*/
/* Dispatch Database Administration Command Handler                          */
/*___________________________________________________________________________*/
SHORT dba_exec(PCHAR parm_buf)
{
   /*_____________________________________________*/
   /* Display SQLCA sqlerrd                       */
   /*_____________________________________________*/
   switch (parms.cmd_num) {
         /*_____________________________________________*/
         /* Environment Services                        */
         /*_____________________________________________*/
   case CMD_DBA_STARTUSE        : rcd = dba_startuse(parm_buf);      break;
   case CMD_DBA_STOPUSE         : rcd = dba_stopuse();               break;
   default: RECORD_NOTHING;                                          break;
   } /* endswitch */

   return(rcd);

} /* end dba_exec */



/*___________________________________________________________________________*/
/* execute the STARTUSE DBA function                                         */
/* FORMAT: EXEC DBA: STARTUSE database_name [S/X]                            */
/*___________________________________________________________________________*/
SHORT dba_startuse(PCHAR parm_str)
{
   BYTE  database_name[9];                   /* Database name,null terminated */
   CHAR  share_mode;                         /* Share mode for STARTUSE       */
   /*_____________________________________________*/
   /* Get the parm strings                        */
   /*_____________________________________________*/
   if (sscanf(parm_str, " %8s %c ", database_name, &share_mode)
            != parms.num_parms) {
      sprintf(err_msg, "\nSYNTAX ERROR: invalid parameters.");
      return(RECORD_ERR_MSG);
   } /* endif */

   /*_____________________________________________*/
   /* call the START USING DB API                 */
   /*_____________________________________________*/
   sqlestrd(database_name,share_mode,&sqlca);

   if (SQLCODE == RESTART) {
   /*_____________________________________________*/
   /* call the RESTART DB API                     */
   /*_____________________________________________*/
   sqlerest(database_name,&sqlca);
   } /* endif */

   if (SQLCODE != 0) {
      return(RECORD_SQLCA);
   } else {
      /*_____________________________________________*/
      /* call the START USING DB API (again)         */
      /*_____________________________________________*/
      sqlestrd(database_name,share_mode,&sqlca);
   } /* endif */

   if (SQLCODE != 0) {
      return(RECORD_SQLCA);
   } /* endif */

   sqleisig(&sqlca);                      /* handle CTRL-BRK signal handler */

   return(RECORD_SQLCA);

} /* end dba_startuse */



/*___________________________________________________________________________*/
/* execute the STOPUSE DBA function                                          */
/* FORMAT: EXEC DBA: STOPUSE                                                 */
/*___________________________________________________________________________*/
SHORT dba_stopuse(VOID)
{
   /*_____________________________________________*/
   /* Call the STOP USING DB API                  */
   /*_____________________________________________*/
   sqlestpd(&sqlca);
   return(RECORD_SQLCA);

} /* end dba_stopuse */

