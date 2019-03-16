/* format and counting routines for DIFFCOUNT.

   DIFF part comes from GNU DIFF
   Copyright (C) 1988, 1989, 1991, 1992, 1993, 1994, 1995, 1998, 2001,
   2002 Free Software Foundation, Inc.

   This file is part of DIFFCOUNT.
   Count Part developed by C.YANG. 2006 

   DIFFCOUNT is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation;  

   This file is part of HikSvnCount.
   SVN Count Part developed by Haiyuan.Qian 2013
   Software Configuration Management of HANGZHOU HIKVISION DIGITAL TECHNOLOGY CO.,LTD. 
   email:qianhaiyuan@hikvision.com

   HikSvnCount is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; 
   
   */

#include "diffcount.h"


static void process_diff_common_lines (lin, lin);
static void process_diff_hunk (struct change *);

/* Next line number to be printed in the two input files.  */
static lin next0, next1;
/* Print the edit-script SCRIPT as a sdiff style output.  */


void process_diff_script (struct change *script)
{
  next0 = next1 = - files[0].prefix_lines;
  process_script (script, find_change, process_diff_hunk);
  process_diff_common_lines (files[0].valid_lines, files[1].valid_lines);
}



/* diff�Ľ����һ����0��β���ַ�������
  \����β���Ʊ���Ϳո�ȫ��ȥ��
  �µĳ���ͨ��len����
*/
static char * strip_white_in_array( char * array, int * len)
{
	int i = 0;
	int array_len = *len;
	char * array_pointer = array;

	if (array_len == 0) 
		  return 0;
	
	/* Strip ��ͷ�����е��Ʊ�� */
	while ((i < array_len) && 
		((array[i] == ' ')
		||(array[i] == '\n')
		||(array[i] == '\r')
		||(array[i] == '\t')))
	{
		i++;
		array_pointer++;
	}

	if (i < array_len)
	{
		array_len = array_len - i;
	}
	else
	{
	  /* �����ǿ��� */
	  *len = 0;
	  return 0;
	}

	/* strip ����β���Ʊ�� */
    i = array_len -1;
	while ( (i >= 0) && 
		    ((array_pointer[i] == ' ')
			||(array_pointer[i] == '\n')
			||(array_pointer[i] == '\r')
			||(array_pointer[i] == '\t')))
	{  
	  i--;
	}
	if ( i< 0 )
	{
		*len = 0;
		return 0;
	}
	else
	{
		array_len = i+1;
	}
	
    *len = array_len;
	return array_pointer;
}




/* ��diff����е��ַ���(���ַ������ſ�ס��)ȥ��,�����ַ�����סע�ͷ���Ӱ��*/
void get_strip_string_from_array( char * strip_array, char *array, int * len, char * string_symbol, char * string_symbol_alt , char * string_escaped, char *line_comment_string,int *block_comment_inline, char *block_commenton_string, char *block_commentoff_string, int *Code_line)
{
	int i = 0;
	char * strip_pointer = strip_array;
	char * array_pointer = array;
	int strip_len = 0;
	int array_len = *len;
	char symbol = * string_symbol;
	char * escaped_pointer = NULL;
	int escaped = 0;
	int char_process = 0;
	int comment = block_comment;

//ȫ�ļ����ַ������ţ���ҪӦ��һЩ���Դ��ڶ����ı������ OK
//���⣬�ڿ�ע�ͷ�����Ŀ����ַ������账��ֱ�Ӻ��Ը÷���  OK
//����ת�������ַ������Ų�����Ϊ�ַ������ţ�ע����\\\������������ת��ת�Ʒ�����Ĳ����Լ���OK
//��ע�ͷ����ڵ���ע�ͷ�������ɾ��������Ӱ��ͳ��ok

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
				for(;len_str > 0; len_str --)
				{
					*strip_pointer = * array_pointer;
					strip_pointer ++;
					strip_len ++;
					i++;
					array_pointer ++;
				}
				//strip_len += len_str;
				*len = strip_len;
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
				for(;len_str > 0; len_str --)
				{
					*strip_pointer = * array_pointer;
					strip_pointer ++;
					strip_len ++;
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
				for(;len_str > 0; len_str --)
				{
					*strip_pointer = * array_pointer;
					strip_pointer ++;
					strip_len ++;
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
					while((*escaped_pointer == *string_escaped) && (escaped_pointer >= strip_array ))
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
			else
			{
				*strip_pointer = 'c';
				strip_pointer ++;
				strip_len ++;	
			}
			//���ָó����������﷨���󣬲��������ͳ���Ƿ���ȷ
				
		}
		
		else if ((*array_pointer != symbol ) && (!string_process))
		{
			if ((*Code_line ==0) && (!comment)  &&  (*array_pointer != ' ')
			&& (*array_pointer != '\n')
			&& (*array_pointer != '\r')
			&& (*array_pointer != '\t'))
			{
				//Ϊ��Ч��
				*Code_line = 1;
			}
			*strip_pointer = *array_pointer;
			//printf("== strip_pointer:%c\n",*strip_pointer);
			strip_pointer ++;
			strip_len ++;	
		}
		else if (*array_pointer == symbol ) 
		{
			//�����ַ����ַ����ж�ת����ţ�ת����ſ��ܴ��ڶ�����ж�����ת���������
			if(array_len != len)
			{
				escaped_pointer = array_pointer-1;
				while((*escaped_pointer == *string_escaped) && (escaped_pointer >= strip_array ))
				{
					escaped = !escaped;
					escaped_pointer --;
				}
			}
			if((!comment) && (!escaped))
				string_process = !string_process;
			
		}
		else
		{
			*strip_pointer = 'x';
			strip_pointer ++;
			strip_len ++;	
		}
		
		i++;
		array_pointer++;
		escaped = 0;
	}
	
    *len = strip_len;
}




static void add_node_to_diffed_list(enum changes line_status, enum line_comment_type line_comment)
{
	struct diffed_line_node * link = (struct diffed_line_node *)malloc(sizeof(struct diffed_line_node));
	link->next = NULL;
	link->line_comment = line_comment;
	link->line_status = line_status;

	if (line_node_head == NULL)
	{
		line_node_head = line_node_tail = link;
	}
	else 
	{
		line_node_tail->next = link;
		line_node_tail = link;
	}
}

/* Debug print Add list processing....... */
static void print_debug_line(enum changes line_status,enum line_comment_type  line_comment,
								char *string, int length)
{
	char * ppp = string;
	int len = length;
	char sep[20];
	switch (line_status)
	{
		case CHANGED: 
			strcpy(sep," [|]");
			break;
		case OLD:
			strcpy(sep," [-]");
			break;
		case NEW:
			strcpy(sep," [+]");
			break;
		case UNCHANGED:
			strcpy(sep," [@]");
			break;
		defaut:
			break;
	}

	switch (line_comment)
	{
		case BlockCommentOn:
			strcat(sep,"[BlockCommentOn] ");
			break;
		case BlockCommentOff:
			strcat(sep,"[BlockCommentOff] ");
			break;
		case LineComment:
			strcat(sep,"[LineComment] ");
			break;
		case NormalLine:
			strcat(sep,"[NormalLine] ");
			break;
		case BlankLine:
			strcat(sep,"[BlankLine ] ");
			break;
		case NormalWithComment:
			strcat(sep,"[NormalWithComment] ");
			break;
		case NormalWithBlockOn:
			strcat(sep,"[NormalWithBlockOn] ");
			break;		
		case BlockOffWithNormal:
			strcat(sep,"[BlockOffWithNormal] ");
			break;		
		default:
			break;
	}
	
	printf("%s",sep);

	while (ppp < string+len)
	{
		register unsigned char c = *ppp++;
		printf("%c",c);
	}

	printf("\n");
}


/* construct diff list from diff result 
   (counting process read this list and do analyzing )
*/

static void construct_diffed_list(enum changes line_status,char const* const* line)
{
    register char const *text_pointer = line[0];
    register char const *text_limit = line[1];
    char * trim_array;
	char * trim_string_array;
	char * free_array_pointer = 0;

	//printf("\nline:%s\n",line[0]);

    int  text_length = text_limit - text_pointer;
	int  trim_text_length;
    int found_pattern = -1;

    char * line_comment_string = current_language->line_comment;
	char * block_commenton_string = current_language->block_comment_on;
    char * block_commentoff_string = current_language->block_comment_off;
    char * block_commenton_alt_string = current_language->block_comment_on_alt;
    char * block_commentoff_alt_string = current_language->block_comment_off_alt;
	char * string_escaped = current_language->string_escaped;
	char * string_symbol_alt = current_language->string_symbol_alt;
	/*add string symbol for exclude some comment symbol */
	char * string_symbol = current_language->string_symbol;
	int block_comment_inline = NormalLine;
	int Code_line = 0;

	int block_comment_len = strlen(block_commentoff_string);
	int block_comment_alt_len = strlen(block_commentoff_alt_string);

    int line_comment_result,block_commenton_result,block_commentoff_result,
    						block_commenton_alt_result,block_commentoff_alt_result,
    						string_symbol_result;

    enum line_comment_type  final_finding_comment_mode;

    if (line)
    {
     	trim_array = strip_white_in_array(text_pointer,&text_length);
		trim_text_length = text_length;
    	if (trim_array)
    	{
    		/*������ַ��������ַ��������޳�*/
			/*string_symbol_result = find_string_in_array(trim_array,string_symbol,text_length);
			printf("\-------string_symbol_result:%d\n",string_symbol_result);
			if (string_symbol_result != -1)
			{*/
				free_array_pointer = trim_string_array = (char *)xmalloc(text_length);
				/*int block_end = -1;
				if(block_comment)
				{
					block_end = find_string_in_array(trim_string_array,block_commentoff_string,text_length);
					if(block_end == -1 && strlen(block_commentoff_alt_string) > 0)
						block_end = find_string_in_array(trim_string_array,block_commentoff_alt_string,text_length);
				}
				printf("\nblock_end:%d\n",block_end);	*/
				get_strip_string_from_array(trim_string_array,trim_array,&trim_text_length,string_symbol,string_symbol_alt,string_escaped,line_comment_string,&block_comment_inline,block_commenton_string,block_commentoff_string,&Code_line);
			/*}
			else
			{
				trim_string_array = trim_array;
			}*/

			/*���޳�����ַ�����ƥ�����ע�ͷ���*/
			line_comment_result = find_string_in_array(trim_string_array,line_comment_string,trim_text_length);
			block_commenton_result = find_string_in_array(trim_string_array,block_commenton_string,trim_text_length);
			block_commentoff_result = find_string_in_array(trim_string_array,block_commentoff_string,trim_text_length);
			block_commenton_alt_result = find_string_in_array(trim_string_array,block_commenton_alt_string,trim_text_length);
			block_commentoff_alt_result = find_string_in_array(trim_string_array,block_commentoff_alt_string,trim_text_length);
			
			/*printf("\n------------free_array_pointer[%d]:%s\n",trim_text_length,trim_string_array);
			printf("--------------line_comment_result:%d\n",line_comment_result);
			printf("--------------block_commenton_result[%s]:%d\n",block_commenton_string,block_commenton_result);
			printf("--------------block_commentoff_result[%s]:%d\n",block_commentoff_string,block_commentoff_result);
			*/
			//SVN_COUNT_DEBUG("#### l_c[%d] b_on[%d] b_off[%d] block_comment_inline[%d] block_comment[%d] Code_line[%d] string_process[%d]\n",line_comment_result,block_commenton_result,block_commentoff_result,block_comment_inline, block_comment,Code_line,string_process);
			
			/* û���κ�ע�ͷ��ŵģ��ж�Ϊһ��������
			  notes: ��ν�����п����Ǵ����У�Ҳ�����ǿ�ע�͵�����*/
			if ( (line_comment_result == -1) && (block_commenton_result == -1) 
				&&(block_commentoff_result == -1) && (block_commenton_alt_result == -1) 
				&& (block_commentoff_alt_result == -1))
							final_finding_comment_mode = NormalLine;	
			else
				
			/* ����ע�ͷ��Ŵ�ͷ�ģ������ж�Ϊע���Ҹ��в���ע�����ı� */
    		if (line_comment_result ==  0 && (!block_comment))
    		  			final_finding_comment_mode = LineComment;
			else

			/* ������ע�ͷ��ţ��Ҳ���ͷ,����û������ע�ͷ��ŵ�*/
    		if ((line_comment_result > 0)
    			&& (block_commenton_result == -1) && (block_commentoff_result == -1) 
    			&& (block_commenton_alt_result == -1) && (block_commentoff_alt_result == -1))
    					final_finding_comment_mode = NormalWithComment;	
			else
				
			/* �Կ�ע�Ϳ�ʼ����ͷ������û�п�ע�ͽ������ж�Ϊע�Ϳ�ʼ*/
    		if ((block_commenton_result == 0) && (block_commentoff_result == -1))
    			{
    					final_finding_comment_mode = BlockCommentOn;
						block_comment = 1;
    			}
			else
			/* �Կ�ѡ��ע�Ϳ�ʼ����ͷ������û�п�ѡ��ע�ͽ������ж�Ϊע�Ϳ�ʼ*/
    		if ((block_commenton_alt_result == 0) && (block_commentoff_alt_result == -1))
    			{
    					final_finding_comment_mode = BlockCommentOn;	
						block_comment = 1;
    			}
			else
				
			/*���ڿ�ע�ͷ��ſ�ʼ�󣬲�����ͷ������û�п�ע�ͷ��Ž������ж�ΪNormalWithBlockOn*/
			if ((block_commenton_result > 0) && (block_commentoff_result == -1))
				{
						final_finding_comment_mode = NormalWithBlockOn;
						block_comment = 1;
				}
    		else
    		/*���ڿ�ѡ��ע�ͷ��ſ�ʼ�󣬲�����ͷ������û�п�ѡ��ע�ͷ��Ž������ж�ΪNormalWithBlockOn*/
			if ((block_commenton_alt_result > 0) && (block_commentoff_alt_result == -1))
				{
						final_finding_comment_mode = NormalWithBlockOn;
						block_comment = 1;
				}
    		else


			/* ���ڿ�ע�ͽ������ţ����Ҳ����ڿ�ע�Ϳ�ʼ���ŵ����	*/
    		if ((block_commenton_result == -1) && (block_commentoff_result != -1))
    		{
    			/*���֮��������*/
    			if ((text_length - block_commentoff_result) > block_comment_len)
    				{	
    					final_finding_comment_mode = BlockOffWithNormal;
						block_comment = 0;
					}
    			else
    				{
    					final_finding_comment_mode = BlockCommentOff;				
						block_comment = 0;
					}
    		}
    		
    		else
    		/* ���ڿ�ѡ��ע�ͽ������ţ����Ҳ����ڿ�ѡ��ע�Ϳ�ʼ���ŵ����*/
    		if ((block_commenton_alt_result == -1) && (block_commentoff_alt_result != -1))
    		{
    			/*���֮��������*/
    			if ((text_length - block_commentoff_alt_result) > block_comment_alt_len)
    				{
    					final_finding_comment_mode = BlockOffWithNormal;				
						block_comment = 0;
    				}
    			else
    				{
    					final_finding_comment_mode = BlockCommentOff;				
						block_comment = 0;
    				}
    		}
    		else

			/*ͬʱ���ڿ�ע�Ϳ�ʼ�Ϳ�ע�ͽ��������*/
    		if ((block_commenton_result != -1) && (block_commentoff_result != -1 ))
    		{
    			/*�Կ�ע����ʼ��ͷ�����Կ�ע�ͽ���Ϊ�����ģ��϶�Ϊ��ע��*/
    			if ((block_commenton_result == 0 ) && 
    				((text_length - block_commentoff_result) == block_comment_len)  && (!block_comment) && (!Code_line))
    					final_finding_comment_mode = LineComment;
    			else 
				/*��ע����ʼ������ǰ�ߵģ���ע�Ϳ�ʼ��ע�ͽ���֮ǰ
    				  ��ʶΪNormalWithComment*/
				if ((block_commenton_result < block_commentoff_result) && 
    				((text_length - block_commentoff_result) == block_comment_len) && (!block_comment))
    				  final_finding_comment_mode = NormalWithComment;
				//
				else 
				{
					//printf("#################block_comment_inline[%d] block_comment[%d] BlockCommentOn[%d]\n",block_comment_inline, block_comment,BlockCommentOn);
					if(block_comment_inline == BlockCommentOn)
					{
						//�����ʱ����һ�����������У����Ըô����У�ʵ�ʴ�����ͳ�ƻ���һ
						if(block_comment)
						{
							final_finding_comment_mode = NormalLine;
							//printf("#################NormalLine[%d] \n",final_finding_comment_mode );
						}
						else
						{
							if(Code_line)
								final_finding_comment_mode = NormalWithBlockOn;
							else
								final_finding_comment_mode = BlockCommentOn;
							//printf("#################NormalWithBlockOn[%d] \n",final_finding_comment_mode );
							block_comment = 1;
						}
					}
					else
					{
						if(block_comment)
						{
							if(Code_line)
								final_finding_comment_mode = BlockOffWithNormal;
							else
								final_finding_comment_mode = BlockCommentOff;
							//printf("#################BlockOffWithNormal[%d] \n",final_finding_comment_mode );
							block_comment = 0;
						}
						else
						{
							if(Code_line)
								final_finding_comment_mode = NormalWithComment;
							else
								final_finding_comment_mode = LineComment;
						}
					}

				}
				
				/* ����鿪ʼע�ͷ����ڿ����ע�ͷ���ߣ�������Ƚϸ���:
				   1. aaaaaaaaaa *)  (* bbbb
				   2. aaaaaaaaaa*) (* bbbbbbb *)
				   3. aaaaaaaaaa*) (* bbbbbbb *) kkkkkkkk....
				   ���϶���һ����ע�͵Ľ�����Ŀǰ�ȼ򻯴���������������ʽר�Ŵ���
				
				else
					{
					 	final_finding_comment_mode = BlockCommentOff;
						block_comment = 0;
					}*/
				
    		}

    		else
    		/*ͬʱ���ڿ�ѡ��ע�Ϳ�ʼ�Ϳ�ѡ��ע�ͽ��������*/
    		if ((block_commenton_alt_result != -1) && (block_commentoff_alt_result != -1 ))
    		{
    			/*�Կ�ע����ʼ��ͷ�����Կ�ע�ͽ���Ϊ�����ģ��϶�Ϊ��ע��*/
    			if ((block_commenton_alt_result == 0 ) && 
    				((text_length - block_commentoff_alt_result) == block_comment_alt_len))
    					final_finding_comment_mode = LineComment;
				else 
    			if (block_commenton_alt_result < block_commentoff_alt_result)
    				final_finding_comment_mode = NormalWithComment;		
				else
					{
					 final_finding_comment_mode = BlockCommentOff;
					 block_comment = 0;
					}
    		}
    		
			/*�������Ľ�����������Ա�*/
    		add_node_to_diffed_list(line_status, final_finding_comment_mode);	

			if (print_lines_info)
    			/* Debug print the add list processing... */
				print_debug_line (line_status,final_finding_comment_mode,trim_array,text_length);

			/* free xalloc space */
			if (free_array_pointer)
				free(free_array_pointer);
    	}
    	else 
    	{
			/* all are blanks ,add blank line to list*/
    		final_finding_comment_mode = BlankLine;
			add_node_to_diffed_list(line_status,final_finding_comment_mode); 	
    		
    		if (print_lines_info)
    			/* Debug print the add list processing... */
				print_debug_line (line_status,final_finding_comment_mode,"",0);
    	}
    }
    
}
	  


/* process lines common to both files in side-by-side format.  */
static void process_diff_common_lines (lin limit0, lin limit1)
{
  lin i0 = next0, i1 = next1;

  if (i0 != limit0 || i1 != limit1)
    {
	  while (i0 != limit0 && i1 != limit1)
	  {
        construct_diffed_list(UNCHANGED,&files[1].linbuf[i1]);
         i0++;i1++;
	  }
  }
  next0 = limit0;
  next1 = limit1;
}



/* process a hunk of an sdiff diff.
   This is a contiguous portion of a complete edit script,
   describing changes in consecutive lines.  */

static void process_diff_hunk (struct change *hunk)
{
  lin first0, last0, first1, last1;
  register lin i, j;
  
  /* Determine range of line numbers involved in each file.  */
  enum changes changes =
    analyze_hunk (hunk, &first0, &last0, &first1, &last1);


  if (!changes)
    return;
  
  /* Print out lines up to this change.  */

  process_diff_common_lines (first0, first1);

  /*``xxx  |  xxx '' lines */
  if (changes == CHANGED)
    {
      for (i = first0, j = first1;  i <= last0 && j <= last1;  i++, j++)
     {
		construct_diffed_list(CHANGED,&files[1].linbuf[j]);
     }
	  
      changes = (i <= last0 ? OLD : 0) + (j <= last1 ? NEW : 0);
      next0 = first0 = i;
      next1 = first1 = j;
    }

  /*  ``     >  xxx '' lines */
  if (changes & NEW)
    {
      for (j = first1; j <= last1; ++j)
      {
    	construct_diffed_list(NEW,&files[1].linbuf[j]);
      }
      next1 = j;
    }

  /*  ``xxx  <     '' lines */
  if (changes & OLD)
    {
      for (i = first0; i <= last0; ++i)
      {
     	construct_diffed_list(OLD,&files[0].linbuf[i]);
      }
      next0 = i;
    }
}


