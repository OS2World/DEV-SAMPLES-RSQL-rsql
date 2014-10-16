/*___________________________________________________________________________*/
/*  INCLUDE FILE NAME: RSQL.H - declarations for RSQL program                */
/*___________________________________________________________________________*/

/*___________________________________________________________________________*/
/*  defines
/*___________________________________________________________________________*/
                   /*__________________________________*/
                   /* Constants used as command values */
                   /*__________________________________*/
#define CMD_SYNTAX_ERROR         (-1)
#define CMD_NO_CMD                 0
#define CMD_SQL_IMMEDIATE          1
#define CMD_SQL_SELECT             2
#define CMD_DOS                    3
#define CMD_DBA                    4
#define CMD_DBA_BACKUP             5
#define CMD_DBA_BIND               6
#define CMD_DBA_CATALOG_DB         7
#define CMD_DBA_CATALOG_NODE       8
#define CMD_DBA_COLLECT_STATUS     9
#define CMD_DBA_CONTINUE_ON_ERROR 10
#define CMD_DBA_CREATE_DB         11
#define CMD_DBA_DROP_DB           12
#define CMD_DBA_EXPORT            13
#define CMD_DBA_GET_ADMIN         14
#define CMD_DBA_IMPORT            15
#define CMD_DBA_LOGOFF            16
#define CMD_DBA_LOGON             17
#define CMD_DBA_PAUSE             18
#define CMD_DBA_REORG             19
#define CMD_DBA_RESET_DM_CFG      20
#define CMD_DBA_RESET_DB_CFG      21
#define CMD_DBA_RESTORE           22
#define CMD_DBA_ROLL_FORWARD      23
#define CMD_DBA_RUNSTATS          24
#define CMD_DBA_SHOW_DB_CFG       25
#define CMD_DBA_SHOW_DB_DIR       26
#define CMD_DBA_SHOW_DM_CFG       27
#define CMD_DBA_SHOW_NODE_DIR     28
#define CMD_DBA_STARTDM           29
#define CMD_DBA_STARTUSE          30
#define CMD_DBA_STOPDM            31
#define CMD_DBA_STOP_ON_ERROR     32
#define CMD_DBA_STOPUSE           33
#define CMD_DBA_UNCATALOG_DB      34
#define CMD_DBA_UNCATALOG_NODE    35
#define CMD_DBA_UPDATE_DB_CFG     36
#define CMD_DBA_UPDATE_DM_CFG     37
#define CMD_LAST_CMD              38

                   /*__________________________________*/
                   /* Constants used as return codes   */
                   /*__________________________________*/
#define TRUE                       1
#define FALSE                      0
#define YES                        1
#define NO                         0
#define GOOD                       0
#define FAILED                   (-1)
#define CONTINUE                   0
#define TERMINATE                (-1)
#define RECORD_NOTHING             0
#define RECORD_SQLCA               1
#define RECORD_API_RCD             2
#define RECORD_ERR_MSG             3
#define RECORD_DOS_EXEC_RESULTS    4

#define RESTART      -1015         /* database needs restart */

#define DLM          " \n"         /* parsing delimiter */

#define SET_TABS_WIDTH         1
#define PRINT_COLUMNS          2
#define PRINT_SELECTED_ROW     3

#define MAX_CMD_SIZE        1500
#define MAX_LINE_SIZE        220
#define COL_SEP                2
#define BREAK_SEP             40
#define BREAK_ROWS             1

#define SQLVAR_NUM           255  /* max number of columns in select */
struct sqlda    *sqlda;           /* declare pointer to an sqlda     */
struct sqlca     sqlca;           /* declare a static sqlca variable */

typedef struct
  {
    SHORT locklist;            /* Max. storage for locklists          */
    SHORT buffpage;            /* Buffer Pool Size                    */
    SHORT maxfilop;            /* Max. DB files open / appl           */
    SHORT softmax;             /*                                     */
    SHORT maxappls;            /*                                     */
    SHORT applheapsz;          /*                                     */
    SHORT dbheap;              /*                                     */
    LONG  dlchktime;           /*                                     */
    SHORT maxtotfilop;         /*                                     */
    SHORT sortheap;            /*                                     */
    SHORT agentheap;           /*                                     */
    SHORT maxlocks;            /*                                     */
    SHORT logprimary;          /*                                     */
    SHORT logsecond;           /*                                     */
    SHORT logfilsz;            /*                                     */
    SHORT stmtheap;            /*                                     */
    CHAR  newlogpath[248];     /*                                     */
    SHORT copy_prot;           /*                                     */
    SHORT log_retain;          /*                                     */
    SHORT user_exit;           /*                                     */
    SHORT auto_rest;           /*                                     */
    SHORT indexrec;            /*                                     */
    SHORT country;             /*                                     */
    SHORT codepage;            /*                                     */
    SHORT release;             /*                                     */
    CHAR  logpath[248];        /*                                     */
    SHORT consistent;          /*                                     */
    SHORT backup_pending;      /*                                     */
    SHORT rollfwd_pending;     /*                                     */
    SHORT log_retain_status;   /*                                     */
    SHORT user_exit_status;    /*                                     */
    CHAR  loghead[14];         /*                                     */
    CHAR  nextactive[14];      /*                                     */
 }  DBCFG;
DBCFG dbcfg;

typedef struct
  {
    SHORT rqrioblk;            /*                                     */
    SHORT svrioblk;            /*                                     */
    SHORT sqlenseg;            /*                                     */
    SHORT numdb;               /*                                     */
    CHAR  nname[9];            /*                                     */
    SHORT comheapsz;           /*                                     */
    SHORT rsheapsz;            /*                                     */
    SHORT numrc;               /*                                     */
    SHORT indexrec;            /*                                     */
    SHORT sheapthres;          /*                                     */
    SHORT nodetype;            /*                                     */
    SHORT release;             /*                                     */
 }  DMCFG;
DMCFG dmcfg;

                   /*__________________________________*/
                   /* Command parameters structure     */
                   /*__________________________________*/
typedef struct
   {
     CHAR   cmd_name[40];      /* Cmd name                            */
     CHAR   cmd_num;           /* Numerical representation of cmd     */
     USHORT num_parms;         /* Number of parms for this command    */
   }  parms_type;
parms_type     parms;          /* variable that contains parameters   */

/*___________________________________________________________________________*/
/*  Global variables                                                         */
/*___________________________________________________________________________*/

                   /*__________________________________*/
                   /* Timing                           */
                   /*__________________________________*/
struct   timeb  timebuff;          /* Timer buffer                   */
double        start_time = 0.0;    /* start of command time          */
double        end_time   = 0.0;    /* end of command time            */
double        event_time = 0.0;    /* elapsed time                   */

                   /*__________________________________*/
                   /* Buffers and miscellaneous vars   */
                   /*__________________________________*/
CHAR          err_msg[512];        /* for errors                     */
CHAR          parm_buf[MAX_CMD_SIZE]; /* buffer for processing cmds   */
SHORT         rcd;                 /* general return code variable   */
SHORT         api_rcd;             /* api     return code variable   */
BOOL          continue_on_error;   /* error stop/continue indicator  */
                                   /* FALSE - stop on error          */
                                   /* TRUE  - continue on error      */

                   /*__________________________________*/
                   /* File variables                   */
                   /*__________________________________*/
USHORT        num_parms;           /* # of parms returned from fscanf*/
FILE          *cmd_stream;         /* CMD file handle                */
FILE          *out_stream;         /* RESPONSE file handle           */
static CHAR   msgfile[]="msgfile,msg"; /* MSG file name              */

                   /*__________________________________*/
                   /* variables for DosExecPgm         */
                   /*__________________________________*/
CHAR          object_name[13];     /* buffer for failure name        */
RESULTCODES   ExecResult;          /* structure for Child proc result*/

/*___________________________________________________________________________*/
/*  Forward declaration of procedures
/*___________________________________________________________________________*/

                   /*__________________________________*/
                   /* file functions                   */
                   /*__________________________________*/
VOID   initialize_rsql(VOID);
VOID   check_args_and_prepare_files(USHORT argc, PCHAR argv[]);
VOID   terminate_program(VOID);

                   /*__________________________________*/
                   /* command/parm parsing functions   */
                   /*__________________________________*/
SHORT  get_command(PCHAR cmd);
SHORT  decode_parm_string(PCHAR parm, PUSHORT parm_num);
SHORT  decode_dmcfg_string(PCHAR parm, PUSHORT parm_num,
                           PCHAR *parm_val_ptr, PUSHORT parm_val_type);
SHORT  decode_dbcfg_string(PCHAR parm, PUSHORT parm_num,
                           PCHAR *parm_val_ptr, PUSHORT parm_val_type);

                   /*__________________________________*/
                   /* EXEC SQL functions               */
                   /*__________________________________*/
SHORT  sql_immediate(PCHAR cmd_sql);
SHORT  sql_select(PCHAR cmd);

                   /*__________________________________*/
                   /* EXEC DBA functions               */
                   /*__________________________________*/
SHORT  dba_exec(PCHAR parms);
SHORT  dba_backup(PCHAR parms);
SHORT  dba_bind(PCHAR parms);
SHORT  dba_catalog_db(PCHAR parms);
SHORT  dba_catalog_node(PCHAR parms);
SHORT  dba_collect_status(PCHAR parms);
SHORT  dba_create_db(PCHAR parms);
SHORT  dba_drop_db(PCHAR parms);
SHORT  dba_export(PCHAR parms);
SHORT  dba_get_admin(VOID);
SHORT  dba_import(PCHAR parms);
SHORT  dba_logoff(PCHAR parms);
SHORT  dba_logon(PCHAR parms);
SHORT  dba_reorg(PCHAR parms);
SHORT  dba_reset_dm_cfg(VOID);
SHORT  dba_reset_db_cfg(PCHAR parms);
SHORT  dba_restore(PCHAR parms);
SHORT  dba_roll_forward(PCHAR parms);
SHORT  dba_runstats(PCHAR parms);
SHORT  dba_show_db_cfg(PCHAR parms);
SHORT  dba_show_db_dir(PCHAR parms);
SHORT  dba_show_dm_cfg(VOID);
SHORT  dba_show_node_dir(VOID);
SHORT  dba_startdm(VOID);
SHORT  dba_startuse(PCHAR parms);
SHORT  dba_stopdm(VOID);
SHORT  dba_stopuse(VOID);
SHORT  dba_uncatalog_db(PCHAR parms);
SHORT  dba_uncatalog_node(PCHAR parms);
SHORT  dba_update_db_cfg(PCHAR parms);
SHORT  dba_update_dnm_cfg(PCHAR parms);

                   /*__________________________________*/
                   /* EXEC DOS function                */
                   /*__________________________________*/
SHORT  dos_exec(PCHAR cmd_dos);

                   /*__________________________________*/
                   /* helper functions                 */
                   /*__________________________________*/
SHORT  merge_file(FILE *dest_stream, char * merge_file_name);
SHORT  record_command_results(SHORT rcd);
VOID   reset_timer(VOID);
VOID   start_timer(VOID);
VOID   stop_timer(VOID);
SHORT  print_format_row(USHORT action);
VOID   APIENTRY exit_proc(ULONG term_code);
