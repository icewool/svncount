#------------------------------------------------------
#  This file is part of HikSvnCount.
#  SVN Count Part developed by Haiyuan.Qian 2013
#  Software Configuration Management of HANGZHOU HIKVISION DIGITAL TECHNOLOGY CO.,LTD. 
#  email:qianhaiyuan@hikvision.com
#
#  HikSvnCount is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; 
#------------------------------------------------------

ʹ�÷�����
һ����post-commit�����������䣺
SVNDIFFCOUNT=/_cmcenter/svncount/diffcount 
$SVNDIFFCOUNT --svn-post-commit -p "$REPOS" -r "$REV" --process
--process��ʾ���ȣ�������ӣ����̨ͳ�ƣ�ǰ̨��������ʾ��������post-commit�ű��У�����ͳ��ռ��post-commit���е�ʱ��

��������ͳ��
diffcount --svn-post-commit -p "$REPOS" -r "$REV" [--end-revision "$rev_end"] --process
��ʵ�ֶ����ͬ�汾��ͳ��

������������ͳ�Ƽ�����
ʹ��--help�����ȡ����ͳ�Ʒ�ʽ
