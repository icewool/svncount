/*    
   This file is part of Remove Comment.
   Remove Comment developed by Haiyuan.Qian 2018
   Software Configuration Management of HANGZHOU HIKVISION DIGITAL TECHNOLOGY CO.,LTD. 
   email:qianhaiyuan@hikvision.com

   */
#define DIFFCOUNT_CMD
	   
#include "lang.h"
#include <error.h>
#include <getopt.h>
#include <sys/time.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

extern char *malloc ();

void *
xmalloc (size)
    unsigned size;
{
  void *new_mem = (void *) malloc (size);

  if (new_mem == NULL)
    {
      fprintf (stderr, "fatal: memory exhausted (xmalloc of %u bytes).\n",
               size);
      /* 1 means success on VMS, so pick a random number (ASCII `K').  */
      exit (75);
    }

  return new_mem;
}



#  define TOLOWER(c) tolower(c)

/* ��ע�ͷ����*/
static int block_comment = 0;
/* �ַ������*/
static int string_process = 0;      

/* the comment kind of line type of diffed result */
enum line_comment_type
{
   /*Block Comment Begin eg. { (* .... */
   BlockCommentOn,
   /*Block Comment end eg. } *) ......*/
   BlockCommentOff,
   /*Line Comment .. eg // or the combined blockon and off eg {xxx}..*/
   LineComment,
   /*Normal Code line*/
   NormalLine,
   /* Blank Code line */
   BlankLine,
   /*One code line with a comment line */
   NormalWithComment,
   /* one code line with BlockCommentOn */
   NormalWithBlockOn,
   /*Blockoff with one code */
   BlockOffWithNormal,
};

struct language_type * current_language;

long getCurrentTime()  
{  
   struct timeval tv;  
   gettimeofday(&tv,NULL);  
   return tv.tv_sec * 1000 + tv.tv_usec / 1000;  
} 

static char const shortopts[] =
"0123456789abBcC:dD:e:EfF:hHiI:lL:nNp:Pqr:sS:tTuU:vwW:x:X:y";



static void usage (void)
{
  
}



static void usage_cn (void)
{
  
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

/* -- ���ַ������н���ģʽƥ�� 
   -- ע���ַ����鿿���Ƚ��ж�λ��û��'\0'���� 
   -- ʹ��KMPģʽƥ���㷨         
*/
int find_string_in_array(char *S, char *T, int s_array_len)
{
	int next[50]; 
	int T_len = strlen(T);
	int S_len = s_array_len;
	int next_i=0;
	int next_j=-1;
	int kmp_i=0;
	int kmp_j=0;
	
	if (*T == '\0') 
			return -1;
	if (S_len == 0)
			return -1;
	
	/* -- Get Next Array ---*/
	next[0] = -1; 
	while(next_i < T_len)
	{
		if((next_j == -1) || (T[next_i] == T[next_j])) 
		{
			next_i++;
			next_j++;
			next[next_i] = next_j;
		}
		else 
		{ 
			next_j = next[next_j]; 
		} 
		
	}
	
	
	/*-- Kmp Search string  --*/	
	while((kmp_i < S_len) && (kmp_j < T_len)) 
	{ 
		if((kmp_j == -1) || (S[kmp_i] == T[kmp_j])) 
		{ 
			kmp_i++; 
			kmp_j++; 
		} 
		else 
		{ 
			kmp_j = next[kmp_j]; 
		} 
	} 
	
	if(kmp_j >= T_len) 
	{ 
		return (kmp_i-T_len); 
	} 
	else 
	{ 
		return -1; 
	} 
}


int find_string(char * S, char * T)
{
	return find_string_in_array(S,T,strlen(S));
}


struct language_type * get_current_language(char * filename)
{
    
	char * afterdot;
	char file_ext [20];
	int  it_filename = strlen(filename);
	int it_language = 0;
	char * p;
	/*�������lost+foundĿ¼��һЩ��ֵĳ��ļ�����Խ�������*/
	int skip_too_long = 15;
	
	/* get file extension name */
	afterdot = filename+strlen(filename);

	while (it_filename)
	{
		if ( *afterdot == '.' )
		{
			afterdot ++;
			break;
		}
		if (skip_too_long == 0)
		{
			it_filename = 0;
			break;
		}
		afterdot --;
		it_filename --;
		skip_too_long --;
	}	

	if (it_filename == 0)
		return NULL;

 	/* use ";_ext_name_;" pattern */
	strcpy(file_ext,";");
	strcat(file_ext,afterdot);
	strcat(file_ext,";");

	/*lower case */
	for (p=file_ext;p<file_ext+strlen(file_ext);p++)
		*p = TOLOWER(*p);

	/*find language type based on file extension name */
	while ( languages[it_language].id != -1)
	{
		if (find_string(languages[it_language].file_extensions,file_ext) != -1)
		{
			return &(languages[it_language]);
		}
		it_language ++;
	}

	return NULL;
}





/* ��diff����е��ַ���(���ַ������ſ�ס��)ȥ��,�����ַ�����סע�ͷ���Ӱ��*/
void get_strip_string_from_array(char* new_array, char *array, int * len, char * string_symbol, char * string_symbol_alt , char * string_escaped, char *line_comment_string,int *block_comment_inline, char *block_commenton_string, char *block_commentoff_string)
{
	int i = 0;
	char * new_pointer = new_array;
	char * array_pointer = array;
	int strip_len = 0;
	int array_len = *len;
	//char symbol = * string_symbol;
	char * escaped_pointer = NULL;
	int escaped = 0;
	int char_process = 0;
	int comment = block_comment;

//ȫ�ļ����ַ������ţ���ҪӦ��һЩ���Դ��ڶ����ı������ OK
//���⣬�ڿ�ע�ͷ�����Ŀ����ַ������账��ֱ�Ӻ��Ը÷���  OK
//����ת�������ַ������Ų�����Ϊ�ַ������ţ�ע����\\\������������ת��ת�Ʒ�����Ĳ����Լ���OK
//��ע�ͷ����ڵ���ע�ͷ�������ɾ��������Ӱ��ͳ��

	//printf("comment[%d]\n",comment);

	while (i < array_len)
	{
		//printf("================comment[%d]  array_pointer[%c] block_commentoff_string[%c] \n",comment,*array_pointer,*block_commentoff_string);
		//�жϳ�����ע�ͷ�
		if((!comment) && (!string_process) && (!char_process) && (*array_pointer == *line_comment_string) )
		{
			//ֱ�ӽ��������Բ������ж�
			int len_str = strlen(line_comment_string);
			if((array_len - i >= len_str) && (strncmp(array_pointer,line_comment_string,len_str) == 0))
			{
				*len = strip_len;
				*new_pointer = '\n';
				new_pointer ++;
				*new_pointer = '\0';
				return ;
			}
			
		}
		//�жϳ��ֿ�ע�ͷ���ʼ
		if((!comment) && (!string_process) && (!char_process) && (*array_pointer == *block_commenton_string))
		{
			int len_str = strlen(block_commenton_string);
			if((array_len - i >= len_str) && (strncmp(array_pointer,block_commenton_string,len_str) == 0))
			{
				//printf("BlockCommentOn\n");
				block_comment = 1;
				for(;len_str > 0; len_str --)
				{
					i++;
					array_pointer ++;
				}
				comment = 1;
				*block_comment_inline = BlockCommentOn;
				continue;
			}			
		}
		

		//�жϳ��ֿ�ע�ͽ�������
		if(comment && (!string_process) && (!char_process) && (*array_pointer == *block_commentoff_string))
		{
			int len_str = strlen(block_commentoff_string);
			//printf("BlockCommentOff to pd\n");
			if((array_len - i >= len_str) && (strncmp(array_pointer,block_commentoff_string,len_str) == 0))
			{				
				//printf("BlockCommentOff\n");
				block_comment = 0;
				for(;len_str > 0; len_str --)
				{
					i++;
					array_pointer ++;
				}
				comment = 0;
				*block_comment_inline = BlockCommentOff;
				continue;
			}
		}

		//�жϵ����ŵȸ�ʽ�ַ������ַ�ֱ�ӹ��ˣ����ַ����ֳ���:��ע�ͣ����ַ���
		//ϵͳ�ڴ��������ַ�����ʽʱ�����ȼ����ڱ�׼�ַ�����ʽ����Ҫ������C��C++��Java�ȳ������������ַ������ǵ��ֽ�����
		if(((*array_pointer == *string_symbol_alt ) && (!string_process) && (!comment)) || char_process)
		{
			//printf("string_symbol_char\n");
			if (*array_pointer == *string_symbol_alt ) 
			{
				//�����ַ����ַ����ж�ת����ţ�ת����ſ��ܴ��ڶ�����ж�����ת���������
				if(array_len != len)
				{
					escaped_pointer = array_pointer-1;
					while((*escaped_pointer == *string_escaped) && (escaped_pointer >= new_pointer ))
					{
						escaped = !escaped;
						escaped_pointer --;
					}
				}
				if((!comment) && (!escaped))
				{
					char_process = !char_process;
				}
				
			}
		}
		else if (*array_pointer == *string_symbol ) 
		{
			//�����ַ����ַ����ж�ת����ţ�ת����ſ��ܴ��ڶ�����ж�����ת���������
			if(array_len != len)
			{
				escaped_pointer = array_pointer-1;
				while((*escaped_pointer == *string_escaped) && (escaped_pointer >= new_pointer ))
				{
					escaped = !escaped;
					escaped_pointer --;
				}
			}
			if((!comment) && (!escaped))
				string_process = !string_process;
			
		}
		//ֻ������ע������
		if ( !comment)
		{
			*new_pointer = * array_pointer;
			new_pointer ++;
			strip_len ++;
		}
		
		i++;
		array_pointer++;
		escaped = 0;
	}
	*new_pointer = '\0';
	
    *len = strip_len;
}



static int construct_line(char* file_path, char *new_file_path)
{
	if(NULL == file_path || NULL == new_file_path)
	{
		return;
	}
	
	/* ����ʶ��*/
	/* ���û��ʶ�����ԣ����һЩ������ȫ�ļ���ƥ�� */
	if(!(current_language = get_current_language(file_path)))
	{
		//û��ƥ�䵽���룬ֱ�ӿ����ļ�
		printf("No match lang.\n");
		return 1; 
	}
	
    char * trim_array;
	char * new_string_array;
	int new_length = 0;
	char * line_comment_string = current_language->line_comment;
	char * block_commenton_string = current_language->block_comment_on;
    char * block_commentoff_string = current_language->block_comment_off;
    char * block_commenton_alt_string = current_language->block_comment_on_alt;
    char * block_commentoff_alt_string = current_language->block_comment_off_alt;
	char * string_escaped = current_language->string_escaped;
	char * string_symbol_alt = current_language->string_symbol_alt;
	char * string_symbol = current_language->string_symbol;

	enum line_comment_type  final_finding_comment_mode;
	int block_comment_inline = NormalLine;
	
	FILE *fp;
    char line_array[4096];
    fp = fopen(file_path, "r");
    
    FILE *fp_new;
    if ((fp_new=fopen(new_file_path,"w+"))==NULL)
    {  
        printf("Open fp_new Failed.\n");  
        return -1;  
    }
	
    while(fgets(line_array, sizeof(line_array), fp))
    {
		if(NULL == line_array || strlen(line_array) == 0)
		{
			continue;
		}
		int text_length = strlen(line_array);
		new_length = text_length;
		new_string_array = (char *)xmalloc(text_length+1);
		memset(new_string_array,0,text_length+1);
		get_strip_string_from_array(new_string_array,line_array,&new_length,string_symbol,string_symbol_alt,string_escaped,line_comment_string,&block_comment_inline,block_commenton_string,block_commentoff_string);
		//printf("Newline: %s \n", new_string_array);
		fprintf(fp_new,"%s",new_string_array);
		free(new_string_array);
		new_string_array = NULL;
    }
    fclose(fp);//�ر��ļ���
    fclose(fp_new); 
    block_comment = 0;
     
    return 0;
	//�������ļ�
    
}
	  

int main (int argc, char **argv)
{

	int exit_code = construct_line("/_cmcenter/test/test.c","/_cmcenter/test/test-no-comment.c");
	return exit_code;
}



   
   
