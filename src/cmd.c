/* diffcount - compare and count source code line by line
               compare two source package and get NBNC changes
               cmd.c: command line interface
   
   DIFF part comes from GNU DIFF
   Copyright (C) 1988, 1989, 1991, 1992, 1993, 1994, 1995, 1998, 2001,
   2002 Free Software Foundation, Inc.

   This file is part of DIFFCOUNT.
   Count Part developed by C.YANG. 2006 
   
   This file is part of HikSvnCount.
   SVN Count Part developed by Haiyuan.Qian 2013
   Software Configuration Management of HANGZHOU HIKVISION DIGITAL TECHNOLOGY CO.,LTD. 
   email:qianhaiyuan@hikvision.com

   HikSvnCount is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; 
   */
      

#define DIFFCOUNT_CMD

#include "diffcount.h"
#include <error.h>
#include <exclude.h>
#include <exitfail.h>
#include <getopt.h>
#include <sys/time.h>

long getCurrentTime()  
{  
   struct timeval tv;  
   gettimeofday(&tv,NULL);  
   return tv.tv_sec * 1000 + tv.tv_usec / 1000;  
} 

static char const shortopts[] =
"0123456789abBcC:dD:e:EfF:hHiI:lL:nNp:Pqr:sS:tTuU:vwW:x:X:y";

/* Values for long options that do not have single-letter equivalents.  */
/* -- ����ֻ�г������Ĳ������������������ӳ���ϵ ----*/
enum
{
  HELP_OPTION=CHAR_MAX + 1,
  HELP_OPTION_CN,
  IGNORE_CASE_OPTION,
  PRINT_LINES_INFO_OPTION,
  PRINT_FILES_INFO_OPTION,
  FORCE_PARSE_ALL_OPTION,
  FOR_PROGRAM_READING_OPTION,
  IGNORE_WHITE_SPACE_OPTION,
  LANGS_OPTION,
  SVN_POST_COMMIT_OPTION,
  SVN_END_REVISION_OPTION,
  SVN_COUNT_PROCESS,
  SVN_COUNT_UPDATE
};

char server_ip[32];

#define IFNAMSIZ   16

/*-- Adapt short opts and long opts -----
  the relationship list as below 
  e.g:
	--print-lines-info
	--help 
	-v <---> --version
------------------------------------------*/
static struct option const longopts[] =
{
  {"ignore-filename-case", 0, 0, IGNORE_CASE_OPTION},
  {"print-lines-info", 0, 0, PRINT_LINES_INFO_OPTION},
  {"print-files-info", 0, 0, PRINT_FILES_INFO_OPTION},
  {"force-parse-all", 0, 0, FORCE_PARSE_ALL_OPTION},
  {"for-program-reading", 0, 0, FOR_PROGRAM_READING_OPTION},
  {"ignore-white-space", 0, 0, IGNORE_WHITE_SPACE_OPTION},
  {"help", 0, 0, HELP_OPTION},
  {"cn-help", 0, 0, HELP_OPTION_CN},
  {"count-only", 0, 0, 'c'},
  {"version", 0, 0, 'v'},
  {"langs", 0, 0, LANGS_OPTION},
  {"svn-post-commit", 0, 0, SVN_POST_COMMIT_OPTION},
  {"end-revision", 1, 0, SVN_END_REVISION_OPTION},
  {"process", 0, 0, SVN_COUNT_PROCESS},
  {"update", 0, 0, SVN_COUNT_UPDATE},
  /* end flag */
  {0, 0, 0, 0}
};



static void check_stdout (void)
{
  if (ferror (stdout))
    fatal ("write failed");
  else if (fclose (stdout) != 0)
    pfatal_with_name (_("standard output"));
}



/*----DIFF COUNT HELP---------------*/
static void try_help (char const *reason_msgid, char const *operand)
{
  if (reason_msgid)
    error (0, 0, _(reason_msgid), operand);
  error (EXIT_TROUBLE, 0, _("Try `%s --help or --cn-help' for more information."),
	 program_name);
  abort ();
}


//���ĵİ���˵��
static char const * const option_help_msgid_cn[] = {
  N_("���ܣ������������/�ļ����в���ͳ�ơ����߶�һ�������/�ļ����д�����ͳ��"),
  "",
  N_("����˵����"),
  "",
  N_(" --svn-post-commit "),
  N_("     SVN�ύ(post commit)������ͳ��"), 
  "",
  N_(" --process "),
  N_("     ������ʽ����ʾͳ�ƹ��̣����޸ò�������������ú󴴽��������̺��������ؽ��"), 
  "", 
  N_(" --update "),
  N_("     ����ģʽ������һЩ�쳣���ݵ��޸�"), 
  "",
  N_(" -e (���ڡ�SVN�ύ��ʱ��Ч��"),
  N_("     �����豸����:eth0"), 
  N_("     "),
  "",
  N_(" -p (���ڡ�SVN�ύ��ʱ��Ч��"),
  N_("     �汾��·��(������������·��)"), 
  "",
  N_(" -r (���ڡ�SVN�ύ��ʱ��Ч��"),
  N_("     ��ͳ�Ƶİ汾"), 
  "",
  N_(" --end-revision (��ѡ����,���ڡ�SVN�ύ��ʱ��Ч��"),
  N_("     ͬ���ڴ�ͳ���������汾֮��İ汾����ͳ��"), 
  N_("     ��ע�⣬��ֵ����С�ڴ�ͳ�ư汾"),
  "",
  N_("  -c --count-only  FILE/DIR"),
  N_("    ������һ���ļ�����Ŀ¼���ͳ�ƴ����еĹ��ܣ�û�бȽ϶��� "),
  N_("    ���������/�ļ�����������ֻ����һ�����ļ���Ŀ¼�� "),
  "",
  N_("  --ignore-filename-case  (���ڲ���ͳ��ʱʹ��)"),
  N_("    �ڱȽϵ�ʱ�򣬺����ļ����Ĵ�Сд����(ȱʡ�������Ϊ������ȫ��ͬ���ļ���"),
  N_("    (���ڡ�����ͳ�ơ�ʱ��Ч��"),
  "",
  N_("  --ignore-white-space  (���ڲ���ͳ��ʱʹ��)"),
  N_("    �ڱȽϵ�ʱ�򣬺��Դ������н������ڿո�(�����Ʊ��)��ɵĲ���"),
  N_("    (���ڡ�����ͳ�ơ�ʱ��Ч��"),
  "",
  N_("  --print-lines-info  (���ڵ���ʱʹ��) "),
  N_("    ���ÿ���ļ�����ȽϺ�Ĳ������ͳ���ͳ����Ϣ�����У�,��Ҫ���ڷ������� "),
  N_("    ע�ⲻҪ��Դ��ʹ����ʹ�ã��������д�ӡʱ�����ľ���"),
  "",
  N_("  --print-files-info"),
  N_("    ʹ��������أ���ÿ���ļ�����ȽϺ�ͳ���Ժ�������ļ�����ͳ�ƵĽ����Ϣ"),
  N_("    ȱʡ����ӡÿ���ļ�����Ϣ��ֻ��ӡ���Ľ��"),
  "",
  N_("  --force-parse-all (���ڲ���ͳ��ʱʹ��) "),
  N_("    �Կ�����ͬ���ļ������з�������ȱʡ����ȫ��ͬ���ļ���ʹ�ñȽϹ���"),
  N_("    ���ڡ�����ͳ�ơ�ʱ��Ч��������ͳ��(count-only)ʱ�����ļ���Ҫ����"),
  "",
  N_("  --for-program-reading"),
  N_("    �ı������ʽ���Ը�ʽ���ı���ʽ������������������ȡ�����Ϣ"),
  N_("    ���ɵ������������diffcount������Ҫ��ȡͳ�ƽ����ʱ��ʹ��"),
  N_("    ��ѡ������� --print-lines-info ѡ��"),
  "",
  N_("  -v  --version  "),
  N_("    �����ǰ�İ汾��Ϣ"),
  "",
  N_("  --help "),
  N_("    ���ȱʡ������Ϣ"),
  "", 
  N_("  --cn-help "),
  N_("    ������İ�����Ϣ"),
  
  N_("  --langs "),
  N_("	  ���֧�ֵı�������б�"),
  "",
  N_("���ڴ����е�˵��:"),
  N_("  ���������Ч������ʹ��\"�ǿշ�ע����\"(NBNC)�ĸ���"),
  N_("  �����д����������ʱ��,ֻʹ��NBNC����"),
  "",
  N_("Report bugs to <qianhaiyuan@hikvision.com>."),
  0
};


static char const * const option_help_msgid[] = {
  N_("Compare and diff two packages or only couting one code package"),
  "",
  N_("OPTIONS: "),
  N_(" --svn-post-commit "),
  N_("     svn post commit diff count"), 
  N_(" --process "),
  N_("     show the svn count process"), 
  N_(" --update "),
  N_("     repair svn codecount"), 
  N_(" -e (used with --svn-post-commit)"),
  N_("     network device ,like:eth0"), 
  N_(" -p (used with --svn-post-commit)"),
  N_("     repository's path"), 
  N_(" -r (used with--svn-post-commit)"),
  N_("     count svn revision"), 
  N_(" --end-revision (used with --svn-post-commit)"),
  N_("     end revision ,start with count revision"), 
  N_(" -c --count-only  "),
  N_("     Only counting one code package"),
  N_(" --ignore-filename-case    "),
  N_("     Ignore the difference of file name case "),
  N_(" --ignore-white-space    "),
  N_("     Ignore the white SPACE and TAB  changes in code lines "),
  N_(" --print-lines-info "),
  N_("     Print detailed information of every diffed lines "),
  N_("     Slow, Debug use only "),
  N_(" --print-files-info  "),
  N_("     Print every diffed file result information."),
  N_(" --force-parse-all "),
  N_("     Parsing all diffed files(default: Compare same file)"), 
  N_(" --for-program-reading"),
  N_("    Change result output style for Third-party program reading "),
  N_("    Will disable --print-lines-info automaticly "),
  N_(" -v  --version "),
  N_("     Output current version."),
  N_(" --help "),
  N_("     Show this page"),
  N_(" --cn-help "),
  N_("    Show Chinese help page"),
  N_(" --langs "),
  N_("	  Show supported languages list"),
   "",
  N_("Report bugs to <qianhaiyuan@hikvision.com>."),
  0
};




static void usage (void)
{
  char const * const *p;
  printf (_("\nUsage: %s [OPTION]... (Baseline) Target\n\n"), program_name);
  for (p = option_help_msgid;  *p;  p++)
    {
      if (!**p)
	putchar ('\n');
      else
	{
	  char const *msg = _(*p);
	  char const *nl;
	  while ((nl = strchr (msg, '\n')))
	    {
	      int msglen = nl + 1 - msg;
	      printf ("  %.*s", msglen, msg);
	      msg = nl + 1;
	    }

	  printf ("  %s\n" + 2 * (*msg != ' ' && *msg != '-'), msg);
	}
    }
}



static void usage_cn (void)
{
  char const * const *p;
  printf (_("\nʹ�÷���: %s [����ѡ��]... (���ߴ����/�ļ�) Ŀ������/�ļ�\n\n"), program_name);
  for (p = option_help_msgid_cn;  *p;  p++)
    {
      if (!**p)
	putchar ('\n');
      else
	{
	  char const *msg = _(*p);
	  char const *nl;
	  while ((nl = strchr (msg, '\n')))
	    {
	      int msglen = nl + 1 - msg;
	      printf ("  %.*s", msglen, msg);
	      msg = nl + 1;
	    }

	  printf ("  %s\n" + 2 * (*msg != ' ' && *msg != '-'), msg);
	}
    }
}




static void print_langs_list()
{
	int it_language = 0;
	printf("Diffcount support languages list below:\n");
	printf("LANG\tLCMT\tBCMT1S\tBCMT1E\tBCMT2S\tBCMT2E\tSTRSB\tEXTS\t\tRATE\n");
	while ( languages[it_language].id != -1)
	{
		printf("%s",languages[it_language].language_name);
		printf("\t%s",languages[it_language].line_comment);
		printf("\t%s",languages[it_language].block_comment_on);
		printf("\t%s",languages[it_language].block_comment_off);
		printf("\t%s",languages[it_language].block_comment_on_alt);
		printf("\t%s",languages[it_language].block_comment_off_alt);
		printf("\t%s",languages[it_language].string_symbol);
		printf("\t%s",languages[it_language].file_extensions);
		printf("\t\t%.2f\n",languages[it_language].standard_c_rate);
		it_language ++;
	}
}






/*  Output the final counting result in console (command line mode)  */
static void print_diffcount_result(int cmd_counting_only, char * left ,char * right, 
								      struct diffcount_result * result,
								      int lang_number, float total_standard_c_nbnc_lines)
{
	int i;

	if (cmd_counting_only)
	{
		if (!for_program_reading)
		{
			printf("\nCounting package [%s] result:\n\n",right);
			printf("LANG\tTOTAL\tBLK\tCMT\tNBNC\tRATE\n");
			printf("-----------------------------------------------------------------------\n");
		}
		else if (print_files_info && !for_program_reading)
		{
			printf("$$$$$$$\n"); /* ���������Ҫ��ȡ��ϸ�嵥��7��$�����굥�ͽ���ķָ��*/	
		}
		
		for (i=0; i<lang_number; i++)
		{
			printf("%s",result[i].language_name);
			printf("\t%d",result[i].total_added_lines);
			printf("\t%d",result[i].changed_blank_lines);
			printf("\t%d",result[i].changed_comment_lines);
			printf("\t%d",result[i].changed_nbnc_lines);
			printf("\t%.2f\n",result[i].standard_c_rate);
		}
	}
	else 
	{
		if (svn_post_commit_counting)
		{
			printf("LANG\tADD\tMOD\tDEL\tA&M\tBLK\tCMT\tNBNC\n");
			printf("-------------------------------------------------------------\n");
		}
		else if (!for_program_reading)
		{
			printf("\nDiffcount [%s] and [%s] result:\n\n",left,right);
			printf("LANG\tADD\tMOD\tDEL\tA&M\tBLK\tCMT\tNBNC\tRATE\n");
			printf("-----------------------------------------------------------------------\n");
		}
		else if(print_files_info && !for_program_reading)
		{
			printf("$$$$$$$\n"); /* ���������Ҫ��ȡ��ϸ�嵥��7��$�����굥�ͽ���ķָ��*/
		}
		for (i=0; i<lang_number; i++)
		{
			printf("%s",result[i].language_name);
			printf("\t%d",result[i].total_added_lines);
			printf("\t%d",result[i].total_modified_lines);
			printf("\t%d",result[i].total_deleted_lines);
			printf("\t%d" ,result[i].total_added_lines + result[i].total_modified_lines);
			printf("\t%d",result[i].changed_blank_lines);
			printf("\t%d",result[i].changed_comment_lines);
			printf("\t%d",result[i].changed_nbnc_lines);
			if(!svn_post_commit_counting)
				printf("\t%.2f\n",result[i].standard_c_rate);
			else
				printf("\n");

		}
	}	
	

	if(svn_post_commit_counting)
	{
		printf("-------------------------------------------------------------\n");
	}
	else if (!for_program_reading)
	{
		printf("-----------------------------------------------------------------------\n");
		printf("  Convert all NBNC lines to standard C \n"); 
		printf("      Total: %.2f  (standard C lines)\n\n",total_standard_c_nbnc_lines); 	
	}		


}


bool connect_mysql(MYSQL *p_mysql_conn,mysql_connection *connect_info,bool b_init)
{
	if(b_init)
	{
		if(mysql_init(p_mysql_conn) == NULL)
		{
			printf("mysql init fail\n");
			return false;
		}
	}
	int  timeout_  =  8;
	mysql_options(p_mysql_conn,MYSQL_OPT_CONNECT_TIMEOUT,(const char *)&timeout_);
	mysql_options(p_mysql_conn,MYSQL_OPT_READ_TIMEOUT,(const char *)&timeout_);
	mysql_options(p_mysql_conn,MYSQL_OPT_WRITE_TIMEOUT,(const char *)&timeout_);
	if(mysql_real_connect(p_mysql_conn, connect_info->host, connect_info->user, connect_info->passwd,"svnadmin", connect_info->port, NULL, 0) == NULL)
	{
		SVN_COUNT_DEBUG( "mysql_conn���Ӵ��� : %s\n",mysql_error(p_mysql_conn)); 
		return false;
	}
	mysql_query(p_mysql_conn, "set names utf8");
	return true;
}

bool getdata_mysql(MYSQL *p_mysql_conn,char * execsql,char *getstr,int getlen)
{
	MYSQL_RES *result;
	MYSQL_ROW row;
	int n_count = 5;
start:
	if (mysql_query(p_mysql_conn, execsql)!=0)	
	{ 
		char *Err = (char *)mysql_error(p_mysql_conn);
		if(strstr(Err,"MySQL server has gone away") && n_count>0)
		{
			mysql_close(p_mysql_conn);
			//��ʼ�����ݿ�����
			n_count -- ;
			if(!connect_mysql(p_mysql_conn,&svn_count_connection,false))
			{
				printf("init mysql error\n");
				return false;
			}
			goto start;
		}
		SVN_COUNT_DEBUG("myql_error %s \n",  Err);
		return false;
	}
	
	if(!(result=mysql_use_result(p_mysql_conn)))
	{
		SVN_COUNT_DEBUG( " select error no result \n",""); 
		return false;
	}

	if(row=mysql_fetch_row(result))
	{
		//SVN_MULTISITE_DEBUG( " sqlsel: %s\n",row[0]); 
		if(!row[0])
		{
			goto err_end_fun;
		}
		if(strlen(row[0]) > getlen || strlen(row[0]) == 0)
		{
			goto err_end_fun;
		}

		strncpy(getstr,row[0],getlen);
	}
	else
		goto err_end_fun;
	
	mysql_free_result(result);
	return true;

err_end_fun:
	mysql_free_result(result);
	return false;

}


static int get_line(char **buffer,char **line_str)
{
	char *lpstop = NULL;

	if(*buffer == NULL)
		return COUNT_ERR;
	lpstop = strstr(*buffer,"\n");
	if(NULL != lpstop )
	{
		*line_str = *buffer;
		*buffer = lpstop+1;
		*lpstop = '\0';
	}
	else
	{
		*line_str = *buffer;
		*buffer = NULL;
	}
	return COUNT_OK; 
}
static int get_exec_popen_str(char **buffer,char *execsql)
{
	FILE* fp = NULL;
	if(NULL==(fp=popen(execsql,"r")))
    {
        pclose(fp);
		return COUNT_ERR;
    }
	*buffer = (char *)malloc(4096);
	memset(*buffer,0,4096);
	int bufsize = 4096;
	int len = 0;
	
	while(1)
	{
		if(!fgets(*buffer+len, bufsize-len , fp))
			break;
		len = strlen(*buffer);
		if(len >= bufsize-1)
		{
			bufsize *= 2;
			*buffer = (char *)realloc(*buffer,bufsize);
			//printf("bufsize:%d",bufsize);
		}
	}
	pclose(fp);
	return COUNT_OK;
}

static int write_file_str(char *filepath,char *out_str)
{
	FILE *fd = NULL;
	if (!(fd = fopen( filepath, "w+")))
	{
		return 0;
	}	
	fwrite(out_str ,strlen(out_str),1,fd );
	fclose(fd);
	return 1;
}

static int
svn_count_do_get_command(char * execsql,char * buf,int buf_length)
{
	FILE* fp = NULL;
	
	SVN_COUNT_DEBUG("svn command :\n %s \n",execsql);
	if(NULL==(fp=popen(execsql,"r")))
	{
		pclose(fp);
		SVN_COUNT_DEBUG( "Error: popen: %s\n ",   strerror(errno)); 
		return COUNT_ERR;
	}
	int n = 0;
	int len = 0;
	memset(buf,0,buf_length);
	while(n<20 && 4000-len>0 &&  fgets(buf+len, 4000-len , fp))
	{
		len = strlen(buf);
		n++;
	}
	pclose(fp);
	fp = NULL;
	SVN_COUNT_DEBUG("svn command result(len:%d):\n %s \n", strlen(buf),buf);
	return COUNT_OK;
}


static void rand_str(int in_len,char *OutStr) 
{ 
	char Str[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	
	int len = strlen(Str);
	memset(OutStr,0,strlen(OutStr));
	srand((unsigned int)time(NULL) + rand());    //��ʼ�������������
	int i = 0;
	for (; i  < in_len; i++) 
	{
		OutStr[i] = Str[rand()%len];		
	} 
	OutStr[i] = '\0';
}


static int get_multisite_master_repos_id(MYSQL * lpmysql_conn,char *lprepos_id,char *lprepos) 
{
	if(NULL == lpmysql_conn || NULL == lprepos_id || NULL == lprepos)
		return 0;

	char execsql[1024] = {0};
	MYSQL_RES *result;
	MYSQL_ROW row;
	int nerr = -1;
	
	sprintf(execsql,"select master_repos_id from svn_multisite_info where svn_multisite_id=(select svn_multisite_id from svn_multisite_replicate where repos_id=%s)",lprepos_id);

	if (mysql_query(lpmysql_conn, execsql)!=0)
	{
		SVN_COUNT_DEBUG("mysql_error: %s\n",mysql_error(lpmysql_conn));
		return 0;
		//show_out << chg_time << "	"<< repos<< "	error : myql_error (" <<mysql_error(lpmysql_conn)<<") \n"; 
		//goto select_fail_end;
	}
	result = mysql_store_result(lpmysql_conn);
	if ((row = mysql_fetch_row(result)))
	{
		if(row[0] && strcmp(row[0],lprepos_id))
		{
			SVN_COUNT_DEBUG("repos \"%s\" is a multisite repository[slave] ,use repos_id with master[repos_id:%s].\n",lprepos,row[0]);
			strncpy(lprepos_id,row[0],11);
			nerr = 2;
		}
		else if(row[0])
		{
			SVN_COUNT_DEBUG("repos \"%s\"[\"%s\"] is a multisite repository[master].\n",lprepos,row[0]);
			nerr = 1;
		}
	}
	mysql_free_result(result); 
	return nerr;
	
}

int svn_post_commit_count(char *repos_path,char * revision)
{
	char *left_path = (char*)malloc(256); 
	char *right_path= (char*)malloc(256); 
	char *replace_partent= (char*)malloc(256); 
	struct diffcount_result *result;
	long time = getCurrentTime();
	int exit_status = EXIT_SUCCESS;
	int lang_number = 0;
	float total_standard_c_nbnc_lines = 0;
	memset(left_path ,0,256);
	memset(right_path,0,256);
	memset(replace_partent,0,256);
	char execsql[4096] = {0};
	char rev_codecount[16] = {0};
	int len = 0;
	char *changed_buffer = NULL;
	int file_mod[4] = {0,0,0,0};
	
	svn_info.revision = revision;
	char *author = NULL;
	char *svnlook_info = NULL;
	char *project = NULL;
	char *codereuse = NULL;
	char *date = NULL;
	char *component = NULL;
	char *cru_id = NULL;
	bool h_only = true;
	SVN_COUNT_DEBUG("repos_path : %s \n",repos_path);

	/* ����ͳ�ƿ�ʼ���λ */
	sprintf(execsql,"insert into code_count_process(repos_id,revision,update_time) values(%s,%s,now());",svn_info.repos_id,revision);
	SVN_COUNT_DEBUG("execsql : %s \n",execsql);
	svn_count_mysql_query(&mysql_conn,execsql);

	/*��ִ��ǰ������pre-svncountִ�У���ʵ���������
	memset(execsql,0,4096);
	sprintf(execsql,"/data1/conf/pre-svncount \"%s\" \"%s\" \"%s\"",repos_path,revision,svn_info.repos_id);
	SVN_COUNT_DEBUG("execsql : %s \n",execsql);
	system(execsql);*/

	/* ��ȡ�汾author���޸�ʱ��*/

	sprintf(execsql,"/usr/local/bin/svnlook info \"%s\" -r %s",repos_path,revision);
	SVN_COUNT_DEBUG("execsql : %s \n",execsql);

	if(get_exec_popen_str(&svnlook_info,execsql) == COUNT_ERR )
	{
		printf("get author error\n");
		free(left_path);
		free(right_path);
		free(replace_partent);
		return COUNT_ERR;
	}
	
	if(strlen(svnlook_info) == 0)
	{
		free(left_path);		
		free(right_path);
		free(replace_partent);
		return COUNT_ERR;
	}
	
	author = svnlook_info;
	if(strncmp(author,"cm_",3) ==0 || strncmp(author,"ci_",3) ==0)
		author = author + 3;
	
	char *lp_svnlook_info = svnlook_info;
	char *lp_svnlook_info_end = svnlook_info+strlen(svnlook_info);
	
	while(*lp_svnlook_info != '\n' && lp_svnlook_info < lp_svnlook_info_end )
		lp_svnlook_info++;
	
	if(lp_svnlook_info == lp_svnlook_info_end)
	{
		free(left_path);
		free(right_path);
		return COUNT_ERR;
	}
	
	*lp_svnlook_info = '\0';
	svn_info.date= lp_svnlook_info+1;
	
	while(*lp_svnlook_info != ' ' )
		lp_svnlook_info++;
	*lp_svnlook_info = '\0';

	while(*lp_svnlook_info != '\n' && lp_svnlook_info < lp_svnlook_info_end )
		lp_svnlook_info++;

	lp_svnlook_info++;

	while(*lp_svnlook_info != '\n' && lp_svnlook_info < lp_svnlook_info_end )
		lp_svnlook_info++;
	
	*lp_svnlook_info = '\0';
	svn_info.log = lp_svnlook_info+1;	
	
	SVN_COUNT_DEBUG("author : %s   date: %s \n",author,svn_info.date );

	//��ȡProject ��CodeReuseֵ
	sprintf(execsql,"/usr/local/bin/svnlook propget --revprop '%s' hik:project -r %s",repos_path,revision);
	SVN_COUNT_DEBUG("execsql : %s \n",execsql);

	if(get_exec_popen_str(&project,execsql) == COUNT_ERR )
	{
		printf("get project error\n");
		free(left_path);
		free(right_path);
		free(svnlook_info);
		free(replace_partent);
		return COUNT_ERR;
	}

	sprintf(execsql,"/usr/local/bin/svnlook propget --revprop '%s' hik:codereuse -r %s",repos_path,revision);

	if(get_exec_popen_str(&codereuse,execsql) == COUNT_ERR )
	{
		printf("get project error\n");
		free(left_path);
		free(right_path);
		free(svnlook_info);
		free(replace_partent);
		if(strlen(project) > 0) free(project);
		return COUNT_ERR;
	}

	SVN_COUNT_DEBUG("project : %s  codereuse : %s \n",project,codereuse);

	sprintf(execsql,"/usr/local/bin/svnlook propget --revprop '%s' hik:component -r %s",repos_path,revision);

	if(get_exec_popen_str(&component,execsql) == COUNT_ERR )
	{
		printf("get component error\n");
		free(left_path);
		free(right_path);
		free(svnlook_info);
		free(replace_partent);
		if(strlen(project) > 0) free(project);
		if(strlen(component) > 0) free(component);
		return COUNT_ERR;
	}

	SVN_COUNT_DEBUG("project : %s  codereuse : %s \n",project,codereuse);

	//��ѯ�û�ID
	sprintf(execsql,"select user_id from svn_user where user_name=\"%s\"",author);
	SVN_COUNT_DEBUG("execsql : %s \n",execsql);
	
	if(!getdata_mysql(&mysql_conn,execsql,svn_info.user_id,11))
	{
		free(left_path);
		free(right_path);
		free(svnlook_info);
		free(replace_partent);
		if(strlen(project) > 0) free(project);
		if(strlen (codereuse) >0 ) free(codereuse);
		if(strlen(component) > 0) free(component);
		SVN_COUNT_DEBUG("get user_id error : %s \n",svn_info.user_id);
		return COUNT_OK; 
	}
	SVN_COUNT_DEBUG("user_id : %s \n",svn_info.user_id);

	//��ȡ��ǰ�汾�ļ��޸��б�
	sprintf(execsql,"/usr/local/bin/svnlook changed '%s' -r %s",repos_path,revision);
	SVN_COUNT_DEBUG("execsql : %s \n",execsql);

	if(get_exec_popen_str(&changed_buffer,execsql) == COUNT_ERR || NULL == changed_buffer)
	{
		free(left_path);
		free(right_path);
		free(svnlook_info);
		free(replace_partent);
		if(strlen(project) > 0) free(project);
		if(strlen (codereuse) >0 ) free(codereuse);
		if(strlen(component) > 0) free(component);
		printf("something wrong!!\n");
		return COUNT_ERR;
	}
	if(*changed_buffer == 'A')
		file_mod[0]++;
	else if (*changed_buffer == 'D')
		file_mod[1]++;
	else
		file_mod[2]++;
	char * lpfind = strstr(changed_buffer,"\nA");
	while(lpfind != NULL)
	{
		file_mod[0]++;
		lpfind = strstr(lpfind+2,"\nA");
	}

	lpfind = strstr(changed_buffer,"\nU");
	while(lpfind != NULL)
	{
		file_mod[2]++;
		lpfind = strstr(lpfind+2,"\nU");
	}

	lpfind = strstr(changed_buffer,"\n_U");
	while(lpfind != NULL)
	{
		file_mod[2]++;
		lpfind = strstr(lpfind+2,"\n_U");
	}

	lpfind = strstr(changed_buffer,"\nD");
	while(lpfind != NULL)
	{
		file_mod[1]++;
		lpfind = strstr(lpfind+2,"\nD");
	}

	SVN_COUNT_DEBUG("file_mod count: add(%d) del(%d) chg(%d) \n",file_mod[0],file_mod[1],file_mod[2]);
	// ����ʹ��svnlook history ��cat ��ȡ���ļ��͵�ǰ�ļ����ļ�����ʵ��
	char *lprightstr = changed_buffer;	
	char *lplinestr = NULL;
	if(file_mod[0] >= 150 && file_mod[2] == 0)
	{
		//���������ļ�����150 ��ÿ�Σ�ֱ�ӹ��˱����ύ��ɾ��ͳ������
		SVN_COUNT_DEBUG("audit import that file add more than 150 files: counts(%d) \n",file_mod[0]);

		strcpy(execsql,"insert into audit_code_import(repos_id,user_id,revesion,counts,time,repos_name,server_ip,log_message) value (");
		sprintf(execsql+strlen(execsql),"%s,%s,%s,%d,'%s','%s','%s','",svn_info.repos_id,svn_info.user_id,revision,file_mod[0],svn_info.date,svn_info.repos_name,svn_info.server_ip);

		if(strlen(svn_info.log) >0)
		{
			char *lpexecsql = execsql+strlen(execsql);
			int n = 254;
			while(*(svn_info.log) && n > 0)
			{
				if(*(svn_info.log) == '\'')
				{
					*lpexecsql = '\\';
					lpexecsql++;
				}

				*lpexecsql = *(svn_info.log);
				svn_info.log++;
				lpexecsql++;
				n--;
			}
			if(*(lpexecsql-1) == '\n')
			{
				*(lpexecsql-1) = '\'';
				*(lpexecsql) = ')';
				*(lpexecsql+1) = '\0';
			}
			else
			{
				*(lpexecsql) = '\'';
				*(lpexecsql+1) = ')';
				*(lpexecsql+2) = '\0';
			}
		}
		else
		{
			strcat(execsql,"')");
		}

		char execsql_utf8[8192] = {0};
		if(gbk2utf8(&execsql_utf8[0],&execsql[0],8192 ) >0)
		{
			svn_count_mysql_query(&mysql_conn,execsql_utf8);
		}
		goto end_fun;
	}
	file_mod[0] = 0;
	file_mod[1] = 0;
	file_mod[2] = 0;
	
	while(get_line(&lprightstr,&lplinestr) == COUNT_OK)
	{
		int change_type = 0;
		
		if(NULL == lplinestr )
		{
			break;
		}		
		
		SVN_COUNT_DEBUG("lplinestr : %s \n",lplinestr);

		if(strlen(lplinestr) <= 4 || strlen(lplinestr) > 512)
		{
			continue;
		}
		svn_info.svn_url_path = lplinestr+4;

		SVN_COUNT_DEBUG("svn_url_path : %s \n",svn_info.svn_url_path);

		/* �ļ�״̬ʶ����Ҫ��ΪADU�����ǲ���merge�Ļ���ʾ_U�ȣ�����Ҫɾ��ǰ���_*/
		while(*lplinestr == '_')
			lplinestr++;
		
		switch(*lplinestr)
		{
			case 'A':
				{
					//����ļ��Ƿ�ΪReplace ������Replace ����Ŀ¼��Replace ����
					if(NULL != replace_partent  && strlen(replace_partent)>0 && strncmp(svn_info.svn_url_path,replace_partent,strlen(replace_partent)) ==0)
					{
						change_type = REPLACE;
						break;
					}
					change_type = ADD;
					break;
				}				
			case 'U':
				change_type = MOD;
				break;
			case 'D':
				{
					//����ļ��Ƿ�ΪReplace	
					
					if(NULL != lprightstr)
					{
						char *str = lprightstr;
						char *str_new_line = NULL;
						
						if(get_line(&str,&str_new_line) == COUNT_OK)
						{
							
							if( strlen(str_new_line) <= 4 || strlen(str_new_line) > 512)
							{
								lprightstr = str;
								change_type = DEL;
							}
							else
							{
								SVN_COUNT_DEBUG("Replace check : %s \n",str_new_line);
								//ʧ�ܵĻ�����Ҫ�Ѹո�ȡ��ɾ���Ļس��ӻ�ȥ
								//�ɹ��Ļ�����Ҫ��ָ��slprightstr ָ���µ���
								char *cmp_str = str_new_line+4;
								
								while(*str_new_line == '_')
									str_new_line++;
								if(*str_new_line == 'A')
								{
									if(strcmp(svn_info.svn_url_path,cmp_str) == 0)
									{
										SVN_COUNT_DEBUG("Find Replace \n","");
										lprightstr = str;
										change_type = REPLACE;
										memset(replace_partent,0,256);
										strncpy(replace_partent,svn_info.svn_url_path,256);
									}
								}
								if(NULL != str && change_type == 0)
								{
									*(str-1) = '\n';
								}
							}
						}
					}
					if(change_type == 0)
						change_type = DEL;
					break;
				}
			default:
				
				break;
		}

		//������ļ��У�ֱ�ӷ���
		if(*(lplinestr+strlen(lplinestr)) == '/')
		{
			//printf("not a file \n");
			continue;
		}
		
		/* ����ʶ��*/
		/* ���û��ʶ�����ԣ����һЩ������ȫ�ļ���ƥ�� */
		if(!(current_language = get_current_language(lplinestr)))
		{
			//printf("not a language file \n");
			if(!(current_language = get_current_language_with_filename(lplinestr)))
			{
				continue;
			}
			
			SVN_COUNT_DEBUG("filename ext patten : %s \n",current_language->default_extensions);
		}

		char str_rand[9] = {0};
		rand_str(8,str_rand);
		sprintf(right_path,"/tmp/new_%s_%s_%s%s",str_rand,svn_info.repos_id,revision,current_language->default_extensions);
		SVN_COUNT_DEBUG("rightpack : %s \n",right_path);
		sprintf(left_path,"/tmp/old_%s_%s_%s%s",str_rand,svn_info.repos_id,revision,current_language->default_extensions);

		if(change_type == ADD || change_type == MOD || change_type == REPLACE)
		{
			sprintf(execsql,"/usr/local/bin/svnlook cat '%s' -r %s '%s' > \"%s\"",repos_path,revision,svn_info.svn_url_path,right_path);
			SVN_COUNT_DEBUG("execsql : %s \n",execsql);
			system(execsql);
		}

		svn_info.commit_level = CODE_COUNT_NORMAL;
			
		if(change_type == ADD )
		{
			file_mod[0]++;
			/* �ļ���ӣ�ֱ��ͳ��*/
			exit_status = compare_files((struct comparison *) 0,"", right_path);
			remove(right_path);		
		}
		else if(change_type == DEL)
		{
			file_mod[1]++;
			char *buffer_lines = NULL;
			char revision_del[12] = {0};
			/* ɾ�����ļ�û��ֱ�ӻ�ȡ�ļ�����ֱ�Ӵ���һ�汾�л�ȡ���÷�������ȱ��
			����ļ��Ƿ�֧������ɾ���ģ��ᵼ���ļ�����ȡʧ��
			����TortoiseSVN��Ҳ������ͬbug���ʲ����޸������ָ��ʵͣ��Ҷ�ʵ��Ӱ���С*/
			sprintf(revision_del,"%d",atoi(revision)-1);
			sprintf(execsql,"/usr/local/bin/svnlook cat '%s' -r %s '%s' | wc -l",repos_path,revision_del,svn_info.svn_url_path);
			if(get_exec_popen_str(&buffer_lines,execsql) == COUNT_ERR)
			{
				//��������
				continue;
			}
			SVN_COUNT_DEBUG("buffer_lines : %s \n",buffer_lines);
			if(atoi(buffer_lines) == 0)
			{
				free(buffer_lines);
				continue;
			}
			/* ����ѯ��������ֱ�Ӳ��뵽current_language->total_deleted_lines
			����ǰ̨��ӡ�ļ���ϸ��Ϣ*/
			current_language->total_deleted_lines += atoi(buffer_lines);
			if(print_files_info)
			{
				printf("%s",current_language->language_name);
				printf("\t%d",0);
				printf("\t%d",0);
				printf("\t%d",atoi(buffer_lines));
				printf("\t%d" ,0 );
				printf("\t%d",0);
				printf("\t%d",0);
				printf("\t%d",0);
				printf("\t%s","DEL");
				printf("\t%s",svn_info.svn_url_path);
				printf("\t%s\n","");
			}
			insert_code_count(svn_info.repos_id,svn_info.revision,svn_info.svn_url_path,svn_info.user_id,svn_info.date,current_language->language_name,"DEL",0,0,atoi(buffer_lines),0,0,0);
			free(buffer_lines);
			buffer_lines = NULL;
		}
		else if ( change_type == MOD )
		{
			file_mod[2]++;
			char *buffer_history = NULL;
			char *repos_path_old = NULL;
			char *revision_old = NULL;
			
			//��ѯ��һ�汾·���Ͱ汾��
			sprintf(execsql,"/usr/local/bin/svnlook history '%s' -r %s -l 2 '%s'| grep \"^[ 0-9]\"",repos_path,revision,svn_info.svn_url_path);
			SVN_COUNT_DEBUG("execsql : %s \n",execsql);
			if(get_exec_popen_str(&buffer_history,execsql) == COUNT_ERR)
			{
				//��������
				continue;
			}
			if(!buffer_history )
			{
				continue;
			}
			if(strlen(buffer_history) == 0  || !strstr(buffer_history,"\n") )
			{
				free(buffer_history);
				continue;
			}
			SVN_COUNT_DEBUG("buffer_history : %s \n",buffer_history);

			//�Ӳ�ѯ�Ľ���н�����һ�汾�ļ�·���Ͱ汾
			repos_path_old = buffer_history + strlen(buffer_history);
			char *buffer_history_end = repos_path_old;
			*repos_path_old = '\0';
			repos_path_old--;
			
			while(*repos_path_old == '\n')
			{
				*repos_path_old = '\0';
				repos_path_old--;
			}
			
			while(*repos_path_old != '\n' && repos_path_old > buffer_history)
				repos_path_old--;

			if(repos_path_old == buffer_history)
				continue;
			
			repos_path_old++;

			while(*repos_path_old == ' ')
				repos_path_old++;
			
			revision_old = repos_path_old;

			while(*repos_path_old != ' ' && *repos_path_old < buffer_history_end)
				repos_path_old++;
			*repos_path_old = '\0';

			if(repos_path_old == buffer_history_end)
				continue;
			
			repos_path_old++;
			
			while(*repos_path_old == ' ')
				repos_path_old++;

			if(!(repos_path_old && revision_old && strlen(repos_path_old) >= 1 && strlen(revision_old) >=1))
				continue;

			SVN_COUNT_DEBUG("leftpack : %s \n",left_path);

			//��ȡ��һ�汾�ļ���
			SVN_COUNT_DEBUG("repos_path_old : %s  revision_old : %s \n",repos_path_old,revision_old);
			sprintf(execsql,"/usr/local/bin/svnlook cat '%s' -r %s '%s' >'%s'",repos_path,revision_old,repos_path_old,left_path);
			SVN_COUNT_DEBUG("execsql : %s  \n",execsql);
			system(execsql);
			
			//�Ա������ļ�
			exit_status = diffcount_compare_packages (left_path,right_path);

			remove(right_path);
			remove(left_path);
			free(buffer_history);

		}
		else if(change_type = REPLACE)
		{
			file_mod[3]++;
			svn_info.commit_level = CODE_COUNT_REPLACE;
			//��ȡ��һ�汾�ļ���
			//SVN_COUNT_DEBUG("Repalce file \n","");
			sprintf(execsql,"/usr/local/bin/svnlook cat '%s' -r %d '%s' >'%s'",repos_path,atoi(revision)-1,svn_info.svn_url_path,left_path);
			SVN_COUNT_DEBUG("execsql: %s \n",execsql);

			int ret = system(execsql);
			//�Ա������ļ�
			SVN_COUNT_DEBUG("system return: %d \n",ret);
			//��SVN����û�����ʱ���ã����ܳ��ַ��ؽ��Ϊ-1����������������������е�
			//û���ҳ���������ԭ��
			if(ret==-1)
			{
				ret = access(left_path,F_OK);
			}
			
			if(ret==0)
			{
				file_mod[2]++;
				exit_status = diffcount_compare_packages (left_path,right_path);
			}
			else
			{
				file_mod[0]++;
				exit_status = compare_files((struct comparison *) 0,"", right_path);
			}

			remove(right_path);
			remove(left_path);
		}
		
	}

	memset(execsql,0,4096);
	if(file_mod[0] >= 150 && file_mod[2] == 0)
	{
		//���������ļ�����150 ��ÿ�Σ�ֱ�ӹ��˱����ύ��ɾ��ͳ������
		strcpy(execsql,"delete from code_count where repos_id=");
		strcat(execsql,svn_info.repos_id);
		strcat(execsql," and revision=");
		strcat(execsql,svn_info.revision);				
		svn_count_mysql_query(&mysql_conn,execsql);
		
		SVN_COUNT_DEBUG("audit import that file add more than 150 files: counts(%d) \n",file_mod[0]);

		strcpy(execsql,"insert into audit_code_import(repos_id,user_id,revesion,counts,time,repos_name,server_ip,log_message) value (");
		sprintf(execsql+strlen(execsql),"%s,%s,%s,%d,'%s','%s','%s','",svn_info.repos_id,svn_info.user_id,revision,file_mod[0],svn_info.date,svn_info.repos_name,svn_info.server_ip);

		if(strlen(svn_info.log) >0)
		{
			char *lpexecsql = execsql+strlen(execsql);
			int n = 254;
			while(*(svn_info.log) && n > 0)
			{
				if(*(svn_info.log) == '\'')
				{
					*lpexecsql = '\\';
					lpexecsql++;
				}

				*lpexecsql = *(svn_info.log);
				svn_info.log++;
				lpexecsql++;
				n--;
			}
			if(*(lpexecsql-1) == '\n')
			{
				*(lpexecsql-1) = '\'';
				*(lpexecsql) = ')';
				*(lpexecsql+1) = '\0';
			}
			else
			{
				*(lpexecsql) = '\'';
				*(lpexecsql+1) = ')';
				*(lpexecsql+2) = '\0';
			}
		}
		else
		{
			strcat(execsql,"')");
		}

		char execsql_utf8[8192] = {0};
		if(gbk2utf8(&execsql_utf8[0],&execsql[0],8192 ) >0)
		{
			svn_count_mysql_query(&mysql_conn,execsql_utf8);
		}		
	}
	else
	{
		if(file_mod[0] >= 50 && file_mod[2] <= 5 )
		{
			sprintf(execsql,"update code_count set commit_level=\"%s\" ",(file_mod[0] >= 150)?(CODE_COUNT_IMPORT_II):(CODE_COUNT_IMPORT_I));			
		}
		else if (file_mod[0] >= 150)
		{
			sprintf(execsql,"update code_count set commit_level=\"%s\" ",CODE_COUNT_IMPORT_I);
		}
		else
		{
			if(strlen (codereuse) >0 && strlen (codereuse) <3 )
				sprintf(execsql,"select sum(ROUND((changed_nbnc_lines+deleted_lines*0.05)*(100 - %s)/100,2) ) as total_nbnc_code \
					from code_count where repos_id=%s and revision=%s group by repos_id,revision",codereuse, svn_info.repos_id,svn_info.revision);
			else if (strlen(codereuse) == 3)
			{
				strcpy(rev_codecount,"1");
			}
			else
				sprintf(execsql,"select sum(ROUND(changed_nbnc_lines+deleted_lines*0.05)) as total_nbnc_code \
					from code_count where repos_id=%s and revision=%s group by repos_id,revision",svn_info.repos_id,svn_info.revision);
					
			SVN_COUNT_DEBUG("execsql: %s \n",execsql);
			
			if(strlen(execsql )>0 )
			{
				getdata_mysql(&mysql_conn,execsql,rev_codecount,16);
				memset(execsql,0,4096);
				SVN_COUNT_DEBUG("rev_codecount: %s \n",rev_codecount);
				if(atoi(rev_codecount)> 10000)
				{
					sprintf(execsql,"update code_count set commit_level=\"%s\" ",CODE_COUNT_WARNING);
					SVN_COUNT_DEBUG("revision's codecount is more than 10000, make commit level to %s \n",CODE_COUNT_WARNING);
				}
			}
			
			
		}

		//���������Ŀ��ţ����޸��ύ��¼�������Ŀ��ţ�ͬ�����������ʹ��ϵͳ�������㣬Ĭ�ϴ���������Ϊ0%
		if (strlen(project) > 0 && strlen (project) <= 64 && !strstr(project,"\"") && !strstr(project,"'"))
		{
			if(strlen(execsql) > 0)
			{
				strcat(execsql," , project=\"");
				strcat(execsql,project);
				strcat(execsql,"\"");
			}
			else
				sprintf(execsql,"update code_count set project=\"%s\" ",project);
		}
		
		if(strlen (codereuse) >0 && strlen (codereuse) <= 3)
		{
			if(strlen(execsql) > 0)
			{
				strcat(execsql," , codereuse=");
				strcat(execsql, codereuse );
			}
			else
				sprintf(execsql,"update code_count set codereuse=%s ",codereuse);
		}

		if(strlen (component) >0 && strlen (component) <= 128 && !strstr(component,"\"") && !strstr(component,"'"))
		{
			if(strlen(execsql) > 0)
			{
				strcat(execsql," , component=\"");
				strcat(execsql, component );
				strcat(execsql,"\"");
			}
			else
				sprintf(execsql,"update code_count set component=\"%s\" ",component);
		}
		
		if(strlen(execsql) > 0)
		{
			strcat(execsql," where repos_id=");
			strcat(execsql,svn_info.repos_id);
			strcat(execsql," and revision=");
			strcat(execsql,svn_info.revision);	
			SVN_COUNT_DEBUG("execsql: %s \n",execsql);
			svn_count_mysql_query(&mysql_conn,execsql);
		}

		/*���δ������ڿ���pre-commit�Ĺ���������ʵ��ҵ����
		memset(execsql,0,4096);
		sprintf(execsql,"/usr/local/bin/svnlook log -r \"%s\" \"%s\" | grep -E -o \"#[_0-9a-zA-Z-]+-[0-9]+#\" | head -1 | sed 's/#//g'",revision,repos_path);
		SVN_COUNT_DEBUG("execsql : %s \n",execsql);

		get_exec_popen_str(&cru_id,execsql);
		if(cru_id != NULL && strlen(cru_id) > 0 && strlen(cru_id) < 128)
		{
			len = strlen(cru_id)-1;
			while(*(cru_id+len) == '\r' || *(cru_id+len) == '\n')
			{
				*(cru_id+len) = '\0';
				len --; 
				if(len <= 0)
					break;
				
			}
			memset(execsql,0,4096);
			sprintf(execsql,"curl --connect-timeout 3 -m 10 \"http://cm.opensource.com/webservice/codereview/cru-pre-commit.php?create_ex=1&review_id=%s&repos_id=%s&rev=%s\" -k",cru_id,svn_info.repos_id,revision);
			SVN_COUNT_DEBUG("execsql : %s \n",execsql);
			system(execsql);
		}*/

	}
end_fun:
	/* ����ͳ�ƿ�ʼ���λ */
	memset(execsql,0,4096);
	sprintf(execsql,"delete from code_count_process where repos_id=%s and revision = %s;",svn_info.repos_id,revision);
	SVN_COUNT_DEBUG("execsql : %s \n",execsql);
	svn_count_mysql_query(&mysql_conn,execsql);
	
	/*����ͳ����ɺ��post-svncountִ�У���ʵ���������
	memset(execsql,0,4096);
	sprintf(execsql,"/data1/conf/post-svncount \"%s\" \"%s\" \"%s\"",repos_path,revision,svn_info.repos_id);
	SVN_COUNT_DEBUG("execsql : %s \n",execsql);
	system(execsql);*/
	
	for_program_reading = true;
	diffcount_establish_result(&result,&lang_number,&total_standard_c_nbnc_lines);
	print_diffcount_result(false,left_path, right_path,
					 result,lang_number,total_standard_c_nbnc_lines);
	
	/*for (int i=0; i<lang_number; i++)
	{
		//�����ļ��޸�״̬�����ȫ����.h�޸ģ��Զ�����
		printf("%s",result[i].language_name);
		printf("\t%d",result[i].total_added_lines);
		printf("\t%d",result[i].changed_blank_lines);
		printf("\t%d",result[i].changed_comment_lines);
		printf("\t%d",result[i].changed_nbnc_lines);
		printf("\t%.2f\n",result[i].standard_c_rate);
		if(result[i].default_extensions)
		{
			h_only = false; 
			break;
		}
	}*/
	
	free(result);
	free(svnlook_info);
	free(left_path);
	free(right_path);
	free(changed_buffer);
	free(replace_partent);

	if(cru_id != NULL && strlen(cru_id) > 0) free(cru_id);	
	if(strlen(project) > 0) free(project);
	if(strlen (codereuse) >0 ) free(codereuse);
	if(strlen(component) > 0) free(component);
	time = getCurrentTime() - time;
	printf("time:%ld\n" ,time);
	return COUNT_OK;
}


int main (int argc, char **argv)
{
  int c;
  int prev = -1;
  bool cmd_counting_only = false;
    
  char * leftpack; 
  char * rightpack;
  char repos_path[256] = {0};
  char revision[12] = {0};
  int start_revision = 0;
  int end_revision = 0;
  memset(server_ip,0,32);
  int exit_status;

  struct diffcount_result *result;
  
  int lang_number = 0;
  float total_standard_c_nbnc_lines = 0;

  /*init diffcount */
  diffcount_init();
  
  /* Program name ---> 'diffcount' */
  program_name = argv[0];
  svn_post_commit_counting = false;
  svn_count_debug_on = false;
  svn_count_process = false;
  svn_count_update = false;

  /* Decode the options.  */
int err = 0;
  while ((c = getopt_long (argc, argv, shortopts, longopts, 0)) != -1)
  {

    switch (c)
	{
		case 0:
	 	 break;

		 case 'v':
		  	  printf ("\n\tDiffcount Utility  %s\n%s\n",
			  version_string, copyright_string);
			  check_stdout ();
		  return EXIT_SUCCESS;

		  case 'c':
		  	  trace2("Get Option: counting_only \n");
		  	  cmd_counting_only = true;
		  	  break;
		  case 'd':
			  svn_count_debug_on = true;
			  break;
          case 'p':
		  	if(optarg && strlen(optarg) <256 && strlen(optarg) > 2)
	              strcpy(repos_path,optarg);
			else
				  err = 1;
		  	  trace2("Get Option: repos path \n");
		  	  break;
          case 'r':
		  	if(optarg && strlen(optarg) <12 && strlen(optarg) >= 1)
		  	{
		  		start_revision = atoi(optarg);
	            strcpy(revision,optarg);
		  	}
			else
				  err = 1;		  	
		  	  trace2("Get Option: revision \n");
		  	  break;
		  case 'e':
		  	if(optarg && strlen(optarg) <=16 && strlen(optarg) >= 1)
	              strcpy(server_ip,optarg);
			else
				  err = 1;		  	
		  	  trace2("Get Option: server_ip \n");
		  	  break;
		 case IGNORE_CASE_OPTION:
		 	  trace2("Get Option: Ignore file name case \n");
		 	  diffcount_set_ignore_case(true);
			  break;
		 case SVN_END_REVISION_OPTION:
		 	if(optarg && strlen(optarg) <12 && strlen(optarg) >= 1)
  				  end_revision = atoi(optarg);
			else
				  err = 1;		  	
		  	  trace2("Get Option: end_revision \n");
		  	  break;
		 	break;
			
   	     case PRINT_LINES_INFO_OPTION:
   	     	  trace2("Get Option: Print Lines detailed information \n");
			  if (!for_program_reading)
   	     	  	print_lines_info = true;
   	     	  break;
		 case SVN_COUNT_PROCESS:
		 	  svn_count_process = true;
		 	  break;
		 	
		 case SVN_POST_COMMIT_OPTION:
		 	  svn_post_commit_counting = true;
			  diffcount_set_ignore_white_space(true);
			  break;

		 case SVN_COUNT_UPDATE:
		 	  svn_count_update = true;
			  break;
			  
   	     case PRINT_FILES_INFO_OPTION:
   	     	  trace2("Get Option: Print Files detailed information \n");
   	     	  	print_files_info = true;
   	     	  break;

   	     case FORCE_PARSE_ALL_OPTION:
   	     	  trace2("Get Option: Force parsing all files \n");
   	     	  diffcount_set_parse_all(false);
   	     	  break;

		 case FOR_PROGRAM_READING_OPTION:
			  trace2("Get Option: For program reading \n");
			  diffcount_set_program_reading(true);
			  break;

		 case IGNORE_WHITE_SPACE_OPTION:
			  trace2("Get Option: ignore white spaces \n");
			  diffcount_set_ignore_white_space(true);
			  break;
    
		 case HELP_OPTION:
			  usage();
			  check_stdout ();
			  return EXIT_SUCCESS;

		 case HELP_OPTION_CN:
			  usage_cn ();
			  check_stdout ();
			  return EXIT_SUCCESS;

		  case LANGS_OPTION:
			  print_langs_list();
			  check_stdout ();
			  return EXIT_SUCCESS;

		default:
		  try_help (0, 0);
	}
      prev = c;
    }

	/*printf("svn_post_commit_counting:%s \n",svn_post_commit_counting?"true":"false");
	printf("repos_path:%s \n",repos_path);
	printf("revision:%s \n",revision);*/

	if(err ==1)
	{
		usage();//try_help ("DIFFCOUNT: missing operand after `%s'", argv[argc - 1]);
		exit(0);
	}

  if(svn_post_commit_counting)
	{
		  if(strlen(repos_path) == 0 || strlen(revision) == 0)
		  {
			  usage();
			  exit(0);
		  }
		  if(end_revision != 0 && end_revision < start_revision)
		  {
		  		usage();
			    exit(0);
		  }
		  if(strlen(server_ip) == 0)
		  	strcpy(server_ip,"eth0");
	}
  else if (cmd_counting_only) /* counting_only : only one filename parameter */
  {
  	if (argc - optind != 1)
  	{
		  if (argc - optind < 1)
			try_help ("DIFFCOUNT: missing operand after `%s'", argv[argc - 1]);
	  	else
			try_help ("DIFFCOUNT: extra operand `%s'", argv[optind + 1]);
  	}
  }
  else /* Normal Status */
  {
	if (argc - optind != 2)
  	{
		  if (argc - optind < 2)
			try_help ("DIFFCOUNT: missing operand after `%s'", argv[argc - 1]);
	  	else
			try_help ("DIFFCOUNT: extra operand `%s'", argv[optind + 2]);
  	}
  }

  
  /* Get left file name / right file name */
  if(svn_post_commit_counting)
  {
  		if (!svn_count_debug_on && !svn_count_process && !svn_count_update )
  		{
	  		int pid;
			if(pid=fork())
				exit(0);
			else if(pid< 0)
				exit(1);
			
			setsid();
			chdir("/");
			close(0);
		    close(1);
		    close(2);			
			signal(SIGCHLD, SIG_IGN);
  		}
		
		//svn post commit ����Ƚ�
		char execsql[512]={0};
		char uuid[64] = {0};
		memset(&svn_info,0,sizeof(svn_info_struct));
		svn_info.repos_id = (char *)malloc(12);
		svn_info.user_id = (char *)malloc(12);
		
		//��ʼ�����ݿ�����
		if(!connect_mysql(&mysql_conn,&svn_count_connection,true))
		{
			printf("init mysql error\n");
			return COUNT_ERR; 
		}
		
		//��ȨIP		
		svn_count_do_get_command("cat /var/cmcenter/uuid | grep -E -o \"[a-f0-9]{8}-([a-f0-9]{4}-){3}[a-f0-9]{12}\" | tr -d \"\\n\" ",&uuid[0],63);

		if(strlen(uuid) != 36 )
		{
			printf("uuid error: %s  %d",uuid,strlen(uuid));
			return COUNT_ERR;
		}

		sprintf(execsql,"select server_ip from server where uuid=\"%s\"",uuid);
		memset(server_ip,0,32);
		getdata_mysql(&mysql_conn,execsql,&server_ip[0],31);

		if(strlen(server_ip) == 0  )
		{
			printf("�޷���ȡ������IP�������޷�����!\n");
	        return COUNT_ERR;
		}
		svn_info.server_ip = &server_ip[0];

		svn_info.repos_name = repos_path + strlen(repos_path)-1;
		if(*(svn_info.repos_name) == '/')
			*(svn_info.repos_name) = '\0';
		while(*(svn_info.repos_name) != '/')
			svn_info.repos_name--;
		svn_info.repos_name++;
		SVN_COUNT_DEBUG("reposname : %s \n",svn_info.repos_name);
		
		//��ѯ�汾��ID
		sprintf(execsql,"select repos_id from repos where repos=\"%s\" and server_id=(select id from server where server_ip=\"%s\")",svn_info.repos_name,svn_info.server_ip);
		SVN_COUNT_DEBUG("execsql : %s \n",execsql);

		if(!getdata_mysql(&mysql_conn,execsql,svn_info.repos_id,11))
		{
			printf("get repos_id error : %s \n",svn_info.repos_id);
			return COUNT_ERR; 
		}

		/*
		SVN�ֲ�ʽ�⣬����Ƿֲ�ʽ�⣬��Ҫʹ�������repos id
		get_multisite_master_repos_id(&mysql_conn,svn_info.repos_id,svn_info.repos_name);*/
		
		SVN_COUNT_DEBUG("repos_id : %s \n",svn_info.repos_id);
		if(end_revision == 0)
			svn_post_commit_count(repos_path, revision);
		else
		{
			//�����ʹ�ö�汾��������ʹ�ð汾����һ��ѯ�ķ�ʽ�ԱȰ汾���첢ͳ�Ʋ�ѯ���
			while(start_revision <= end_revision)
			{
				sprintf(revision,"%d",start_revision);
				memset(svn_info.user_id,0,12);
				if(svn_post_commit_count(repos_path, revision) == COUNT_ERR)
				{
					break;
				}
				start_revision ++;
			}
		}

		free(svn_info.repos_id);
		free(svn_info.user_id);
		exit(0);
		
  }
  else if (cmd_counting_only)
  {
  	if (print_files_info && (!for_program_reading))
	{
		printf("\nLANG\tTOTAL\tBLK\tCMT\tNBNC\tTARGET FILE\n");
  	}
  	leftpack = NULL;
   	rightpack = argv[optind];
   	exit_status = diffcount_count_package (rightpack);
  }
  else
  {
  	/* Get the left and right to be counted... */
	
  	if (print_files_info && (!for_program_reading))
	{
		printf("\nLANG\tADD\tMOD\tDEL\tA&M\tBLK\tCMT\tNBNC\tSTATE\tBASELINE FILE\tTARGET FILE\n");
  	}
  	leftpack = argv[optind];
  	rightpack = argv[optind + 1];
  	exit_status = diffcount_compare_packages (leftpack,rightpack);
  }
  

 	/*--- ����������Ľ��-----*/
  diffcount_establish_result(&result,&lang_number,&total_standard_c_nbnc_lines);
  print_diffcount_result(cmd_counting_only,leftpack, rightpack,
  						 result,lang_number,total_standard_c_nbnc_lines);
  free(result);

  
  check_stdout ();
  

  exit (exit_status);
  return exit_status;
}



   
   
