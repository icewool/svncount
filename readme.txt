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

使用方法：
一、在post-commit中添加运行语句：
SVNDIFFCOUNT=/_cmcenter/svncount/diffcount 
$SVNDIFFCOUNT --svn-post-commit -p "$REPOS" -r "$REV" --process
--process显示进度，如果不加，则后台统计，前台不会有显示，可用于post-commit脚本中，减少统计占用post-commit运行的时间

二、批量统计
diffcount --svn-post-commit -p "$REPOS" -r "$REV" [--end-revision "$rev_end"] --process
来实现多个不同版本的统计

三、本地数据统计及其他
使用--help命令获取更多统计方式; 

特别备注：数据库不是必须的，如果不需要导入数据库，则可以使用本地统计功能；SVN的统计模式，必须依赖数据库
编程语言：本程序的编程语言可配置，按照src/lang.h文件内的提示，增加文件即可

号外：devops招聘，有意加入海康威视 devops团队的可发送简历至：qianhaiyuan@hikvision.com
