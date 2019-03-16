## SVN 代码统计主表
CREATE TABLE `code_count` (
  `repos_id` int(11) NOT NULL,
  `revision` int(11) NOT NULL,
  `path` varchar(330) NOT NULL,
  `user_id` int(11) NOT NULL,
  `date` date NOT NULL,
  `diff_state` enum('DEL','NEW','MOD') CHARACTER SET latin1 NOT NULL,
  `added_lines` int(11) DEFAULT '0',
  `modified_lines` int(11) DEFAULT '0',
  `deleted_lines` int(11) DEFAULT '0',
  `changed_blank_lines` int(11) DEFAULT '0',
  `changed_comment_lines` int(11) DEFAULT '0',
  `changed_nbnc_lines` int(11) DEFAULT '0',
  `language_type` varchar(32) DEFAULT NULL,
  `commit_level` enum('Replace','ImportII','ImportI','Warning','Normal') NOT NULL DEFAULT 'Normal' COMMENT '提交等级，Normal为正常，ImportI可能是代码导入，ImportII是大批量代码导入，Replace为特殊的分支文件替换，Warning为存在警告的文件',
  `project` varchar(64) DEFAULT NULL,
  `codereuse` int(11) DEFAULT NULL,
  `component` varchar(128) DEFAULT NULL,
  `reviewed` int(1) DEFAULT '0',
  PRIMARY KEY (`repos_id`,`revision`,`path`),
  KEY `index_user` (`user_id`) USING BTREE,
  KEY `index_date` (`date`) USING BTREE,
  KEY `index_path` (`path`) USING BTREE,
  KEY `index_repos_id` (`repos_id`) USING BTREE,
  KEY `index_project` (`project`) USING BTREE
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

## SVN 代码统计进程表
CREATE TABLE `code_count_process` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `repos_id` int(11) NOT NULL,
  `revision` int(11) NOT NULL,
  `update_time` datetime NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `unuque_index_key` (`repos_id`,`revision`) USING BTREE
) ENGINE=MyISAM AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

## SVN 代码统计异常信息表
CREATE TABLE `code_count_error` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `repos_id` int(11) NOT NULL,
  `revision` int(11) NOT NULL,
  `update_time` datetime NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `unuque_index_key` (`repos_id`,`revision`,`update_time`) USING BTREE
) ENGINE=MyISAM AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

## SVN 版本库表
CREATE TABLE `repos` (
  `repos_id` int(11) NOT NULL AUTO_INCREMENT,
  `server_id` int(11) NOT NULL,
  `repos` varchar(64) DEFAULT NULL,
  `repos_path` varchar(255) DEFAULT NULL,
  `svnadmin` varchar(64) DEFAULT NULL,
  `deptcode` varchar(40) DEFAULT NULL,
  `use_info` varchar(255) DEFAULT NULL,
  `remark_info` varchar(255) DEFAULT NULL,
  `creater` varchar(32) DEFAULT NULL,
  `repos_workspace` varchar(255) DEFAULT NULL,
  `update_time` datetime DEFAULT NULL,
  PRIMARY KEY (`repos_id`) USING BTREE,
  UNIQUE KEY `index_repos_server` (`server_id`,`repos`),
  KEY `index_repos` (`repos`)
) ENGINE=MyISAM AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

## SVN 服务器表
CREATE TABLE `server` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `server_ip` varchar(64) NOT NULL,
  `server_name` varchar(64) NOT NULL,
  `server_type` int(11) NOT NULL,
  `server_ports` int(11) NOT NULL,
  `use_info` varchar(64) DEFAULT NULL,
  `remark_info` varchar(255) DEFAULT NULL,
  `uuid` varchar(36) DEFAULT NULL
  PRIMARY KEY (`id`),
  UNIQUE KEY `index_server_ip` (`server_ip`),
  KEY `index_server_name` (`server_name`)
) ENGINE=MyISAM AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

## SVN用户表
CREATE TABLE `svn_user` (
  `user_id` int(11) NOT NULL AUTO_INCREMENT,
  `user_name` varchar(32) NOT NULL,
  `full_name` varchar(32) DEFAULT NULL,
  `staff_no` varchar(16) DEFAULT NULL,
  `email` varchar(64) DEFAULT NULL,
  `getpwd_date` datetime DEFAULT NULL,
  `getpwd_times` int(11) DEFAULT '0',
  `creation_date` datetime DEFAULT NULL,
  `disable` bit(1) DEFAULT b'0',
  `disable_date` datetime DEFAULT NULL,
  `remark` varchar(256) DEFAULT NULL,
  `deptcode` varchar(32) DEFAULT NULL,
  PRIMARY KEY (`user_id`) USING BTREE,
  UNIQUE KEY `index_user_name` (`user_name`),
  KEY `index_full_name` (`full_name`),
  KEY `index_disable` (`disable`),
  KEY `index_staff_no` (`staff_no`),
  KEY `index_deptcode` (`deptcode`) USING BTREE
) ENGINE=MyISAM AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

## 导入审查表
CREATE TABLE `audit_code_import` (
  `repos_id` int(11) NOT NULL,
  `user_id` int(11) NOT NULL,
  `revesion` int(11) NOT NULL,
  `counts` int(11) NOT NULL,
  `time` date NOT NULL,
  PRIMARY KEY (`repos_id`,`revesion`),
  KEY `index_user_id` (`user_id`) USING BTREE
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
