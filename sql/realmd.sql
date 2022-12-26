/*
Navicat MySQL Data Transfer

Source Server         : localhost_3306
Source Server Version : 50553
Source Host           : localhost:3306
Source Database       : realmd

Target Server Type    : MYSQL
Target Server Version : 50553
File Encoding         : 65001

Date: 2022-12-18 03:23:20
*/

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for accchange_log
-- ----------------------------
DROP TABLE IF EXISTS `accchange_log`;
CREATE TABLE `accchange_log` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `old_acc` int(11) NOT NULL,
  `new_acc` int(11) NOT NULL,
  `date` char(25) NOT NULL,
  `char` int(11) DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=2 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of accchange_log
-- ----------------------------

-- ----------------------------
-- Table structure for account
-- ----------------------------
DROP TABLE IF EXISTS `account`;
CREATE TABLE `account` (
  `account_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `username` varchar(16) CHARACTER SET latin1 COLLATE latin1_general_ci NOT NULL,
  `email` varchar(50) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `pass_hash` char(40) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `token_key` varchar(100) COLLATE utf8_unicode_ci DEFAULT '',
  `join_date` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `registration_ip` varchar(16) COLLATE utf8_unicode_ci NOT NULL DEFAULT '0.0.0.0',
  `expansion_id` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `account_state_id` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `locale_id` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `failed_logins` int(10) unsigned NOT NULL DEFAULT '0',
  `last_ip` varchar(16) COLLATE utf8_unicode_ci NOT NULL DEFAULT '0.0.0.0',
  `last_local_ip` varchar(16) COLLATE utf8_unicode_ci NOT NULL DEFAULT '0.0.0.0',
  `last_login` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  `online` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `opcodes_disabled` int(10) unsigned NOT NULL DEFAULT '0',
  `account_flags` int(10) unsigned NOT NULL DEFAULT '0',
  `client_os_version_id` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `recruiter` int(10) unsigned NOT NULL DEFAULT '0',
  `current_realm` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`account_id`),
  UNIQUE KEY `username` (`username`),
  KEY `FK_account_state_id` (`account_state_id`),
  KEY `FK_client_os_version_id` (`client_os_version_id`),
  KEY `FK_expansion_id` (`expansion_id`),
  KEY `online` (`online`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- ----------------------------
-- Records of account
-- ----------------------------
INSERT INTO `account` VALUES ('1', 'ADMIN', '', '8301316d0d8448a34fa6d0c6bf1cbfa2b4a1a93a', '', '2022-06-20 07:00:45', '0.0.0.0', '1', '1', '0', '0', '127.0.0.1', '127.0.0.1', '2022-12-18 03:21:02', '0', '0', '0', '1', '0', '0');
INSERT INTO `account` VALUES ('2', 'TEST', '', '3d0d99423e31fcc67a6745ec89d70d700344bc76', '', '2022-12-15 16:28:39', '0.0.0.0', '0', '1', '0', '0', '127.0.0.1', '127.0.0.1', '2022-12-18 03:21:24', '1', '0', '0', '1', '0', '0');

-- ----------------------------
-- Table structure for account_access
-- ----------------------------
DROP TABLE IF EXISTS `account_access`;
CREATE TABLE `account_access` (
  `account_id` int(10) unsigned NOT NULL,
  `realm_id` int(10) unsigned NOT NULL,
  `gmlevel` int(10) unsigned NOT NULL DEFAULT '1',
  PRIMARY KEY (`account_id`,`realm_id`),
  KEY `FK_account_permissions_realm_id` (`realm_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of account_access
-- ----------------------------
INSERT INTO `account_access` VALUES ('1', '1', '6');
INSERT INTO `account_access` VALUES ('2', '1', '3');

-- ----------------------------
-- Table structure for account_friends
-- ----------------------------
DROP TABLE IF EXISTS `account_friends`;
CREATE TABLE `account_friends` (
  `id` int(11) unsigned NOT NULL DEFAULT '0',
  `friend_id` int(11) unsigned NOT NULL DEFAULT '0',
  `bind_date` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT 'Bring Date',
  `expire_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00' COMMENT 'Expire Date',
  PRIMARY KEY (`id`,`friend_id`),
  KEY `id` (`id`),
  KEY `friend_id` (`friend_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 PACK_KEYS=0 COMMENT='Stores accounts for Refer-a-Friend System.';

-- ----------------------------
-- Records of account_friends
-- ----------------------------

-- ----------------------------
-- Table structure for account_login
-- ----------------------------
DROP TABLE IF EXISTS `account_login`;
CREATE TABLE `account_login` (
  `account_id` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Account Id',
  `login_date` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `ip` varchar(30) COLLATE utf8_unicode_ci NOT NULL,
  `local_ip` varchar(30) COLLATE utf8_unicode_ci NOT NULL,
  `LocId` int(10) unsigned NOT NULL DEFAULT '0',
  KEY `account_id` (`account_id`),
  KEY `login_date` (`login_date`),
  KEY `ip` (`ip`),
  KEY `local_ip` (`local_ip`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- ----------------------------
-- Records of account_login
-- ----------------------------
INSERT INTO `account_login` VALUES ('1', '2022-06-20 07:01:16', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-20 09:27:55', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-20 09:27:58', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-20 09:28:44', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-20 09:32:53', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-20 09:35:31', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-20 09:50:21', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-20 09:54:21', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-20 09:56:30', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-20 10:11:35', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-20 10:15:31', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-20 10:52:44', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-20 11:06:57', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-20 11:24:09', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-20 11:45:37', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-20 11:53:41', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-20 12:28:16', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-20 12:31:31', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-20 13:16:49', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-20 14:02:15', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-20 14:05:46', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-20 14:15:31', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-20 14:22:44', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-20 14:34:09', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-20 14:36:00', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-20 14:37:05', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-20 14:44:01', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-20 14:46:15', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-20 15:03:18', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-20 15:23:24', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-20 16:01:40', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-21 03:00:17', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-21 03:01:23', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-21 04:24:37', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-21 04:25:49', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-21 04:29:24', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-21 04:32:00', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-21 04:42:59', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-21 04:50:10', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-21 04:51:25', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-21 04:52:54', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-21 04:54:09', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-21 04:55:42', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-21 04:57:11', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-21 05:05:51', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-21 05:16:35', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-21 05:24:06', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-21 05:28:56', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-21 05:30:46', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-21 05:41:32', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-21 05:48:18', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-21 06:02:55', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-21 06:05:06', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-21 06:06:24', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-21 06:08:12', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-21 06:11:42', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-21 06:37:07', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-21 06:39:43', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-06-21 06:53:17', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-11-01 22:05:34', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-11-04 10:07:33', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-12-14 14:34:09', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-12-14 15:26:43', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-12-15 16:29:02', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('2', '2022-12-15 16:29:24', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('2', '2022-12-15 16:29:28', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-12-16 11:39:34', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-12-16 11:39:49', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-12-16 11:39:57', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-12-16 11:40:13', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-12-16 11:46:05', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-12-16 12:36:26', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-12-16 13:44:59', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-12-16 13:47:48', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-12-16 14:13:59', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-12-16 14:58:23', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-12-16 15:29:41', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-12-16 17:51:09', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-12-16 17:53:46', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-12-16 17:57:31', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-12-17 10:15:54', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-12-17 10:21:12', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-12-17 10:32:45', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-12-17 10:58:16', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('2', '2022-12-17 10:59:50', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-12-17 11:00:04', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-12-17 11:01:09', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-12-17 11:03:30', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('2', '2022-12-17 11:03:43', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-12-17 11:03:52', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-12-18 00:04:59', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-12-18 00:33:01', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('2', '2022-12-18 02:19:24', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('2', '2022-12-18 02:20:11', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-12-18 02:24:41', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-12-18 02:53:16', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-12-18 02:53:38', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-12-18 02:53:45', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('2', '2022-12-18 02:53:56', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('2', '2022-12-18 02:54:01', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('2', '2022-12-18 02:54:12', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-12-18 02:57:10', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-12-18 02:57:59', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-12-18 03:01:40', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('2', '2022-12-18 03:02:08', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('2', '2022-12-18 03:02:12', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-12-18 03:07:53', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('2', '2022-12-18 03:08:04', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('2', '2022-12-18 03:08:21', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('2', '2022-12-18 03:08:39', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('2', '2022-12-18 03:08:42', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('2', '2022-12-18 03:09:01', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('2', '2022-12-18 03:11:07', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('2', '2022-12-18 03:12:33', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('2', '2022-12-18 03:15:48', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('2', '2022-12-18 03:20:54', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-12-18 03:21:03', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-12-18 03:21:12', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('1', '2022-12-18 03:21:16', '127.0.0.1', '127.0.0.1', '183');
INSERT INTO `account_login` VALUES ('2', '2022-12-18 03:21:24', '127.0.0.1', '127.0.0.1', '183');

-- ----------------------------
-- Table structure for account_punishment
-- ----------------------------
DROP TABLE IF EXISTS `account_punishment`;
CREATE TABLE `account_punishment` (
  `account_id` int(10) unsigned NOT NULL,
  `punishment_type_id` tinyint(3) unsigned NOT NULL,
  `punishment_date` int(11) NOT NULL,
  `expiration_date` int(11) NOT NULL,
  `punished_by` varchar(45) NOT NULL,
  `reason` varchar(100) NOT NULL DEFAULT 'no reason',
  `active` tinyint(4) NOT NULL DEFAULT '1',
  PRIMARY KEY (`account_id`,`punishment_type_id`,`punishment_date`),
  KEY `FK_punishment_type_id` (`punishment_type_id`),
  KEY `punishment_date` (`punishment_date`),
  KEY `expiration_date` (`expiration_date`),
  KEY `account_id` (`account_id`),
  KEY `punishment_date_2` (`punishment_date`,`expiration_date`),
  KEY `punishment_type_id` (`punishment_type_id`,`punishment_date`,`expiration_date`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of account_punishment
-- ----------------------------

-- ----------------------------
-- Table structure for account_session
-- ----------------------------
DROP TABLE IF EXISTS `account_session`;
CREATE TABLE `account_session` (
  `account_id` int(10) unsigned NOT NULL,
  `session_key` varchar(80) NOT NULL DEFAULT '',
  `v` varchar(80) NOT NULL DEFAULT '',
  `s` varchar(80) NOT NULL DEFAULT '',
  PRIMARY KEY (`account_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of account_session
-- ----------------------------
INSERT INTO `account_session` VALUES ('1', 'B928351257227A4D1E014224248D862CEB9036FBEDAB5B5D950FEB9B223EAB32E87947B0B5C82AE0', '0', '0');
INSERT INTO `account_session` VALUES ('2', 'FD85C9522CECB76F0911116597817F56AD087B10F5F2C68EDB86C566734A4E7C5E6313469FAC62AE', '0', '0');

-- ----------------------------
-- Table structure for account_state
-- ----------------------------
DROP TABLE IF EXISTS `account_state`;
CREATE TABLE `account_state` (
  `account_state_id` tinyint(3) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(16) NOT NULL,
  PRIMARY KEY (`account_state_id`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of account_state
-- ----------------------------
INSERT INTO `account_state` VALUES ('1', 'Active');
INSERT INTO `account_state` VALUES ('2', 'IP locked');
INSERT INTO `account_state` VALUES ('3', 'Frozen');

-- ----------------------------
-- Table structure for account_support
-- ----------------------------
DROP TABLE IF EXISTS `account_support`;
CREATE TABLE `account_support` (
  `account_id` int(10) unsigned NOT NULL,
  `support_points` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`account_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of account_support
-- ----------------------------
INSERT INTO `account_support` VALUES ('1', '0');
INSERT INTO `account_support` VALUES ('2', '0');

-- ----------------------------
-- Table structure for client_os_version
-- ----------------------------
DROP TABLE IF EXISTS `client_os_version`;
CREATE TABLE `client_os_version` (
  `client_os_version_id` tinyint(3) unsigned NOT NULL,
  `name` varchar(45) NOT NULL,
  PRIMARY KEY (`client_os_version_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of client_os_version
-- ----------------------------
INSERT INTO `client_os_version` VALUES ('0', 'Unknown');
INSERT INTO `client_os_version` VALUES ('1', 'Microsoft Windows');
INSERT INTO `client_os_version` VALUES ('2', 'Apple Macintosh OSX');
INSERT INTO `client_os_version` VALUES ('3', 'Custom WoW Chat Client');

-- ----------------------------
-- Table structure for email_banned
-- ----------------------------
DROP TABLE IF EXISTS `email_banned`;
CREATE TABLE `email_banned` (
  `email` varchar(50) NOT NULL,
  `ban_date` int(11) NOT NULL,
  `banned_by` varchar(16) NOT NULL,
  `ban_reason` varchar(100) NOT NULL,
  PRIMARY KEY (`email`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC;

-- ----------------------------
-- Records of email_banned
-- ----------------------------

-- ----------------------------
-- Table structure for email_log
-- ----------------------------
DROP TABLE IF EXISTS `email_log`;
CREATE TABLE `email_log` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `old_email` char(50) NOT NULL,
  `new_email` char(50) NOT NULL,
  `date` char(25) NOT NULL,
  `acc` int(11) DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of email_log
-- ----------------------------

-- ----------------------------
-- Table structure for expansion
-- ----------------------------
DROP TABLE IF EXISTS `expansion`;
CREATE TABLE `expansion` (
  `expansion_id` tinyint(3) unsigned NOT NULL,
  `name` varchar(45) NOT NULL,
  PRIMARY KEY (`expansion_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of expansion
-- ----------------------------
INSERT INTO `expansion` VALUES ('0', 'World of Warcraft');
INSERT INTO `expansion` VALUES ('1', 'World of Warcraft: The Burning Crusade');
INSERT INTO `expansion` VALUES ('2', 'World of Warcraft: Wrath of the Lich King');
INSERT INTO `expansion` VALUES ('3', 'World of Warcraft: Cataclysm');

-- ----------------------------
-- Table structure for ip2nation
-- ----------------------------
DROP TABLE IF EXISTS `ip2nation`;
CREATE TABLE `ip2nation` (
  `ip` int(11) unsigned NOT NULL DEFAULT '0',
  `country` char(2) NOT NULL DEFAULT '',
  KEY `ip` (`ip`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of ip2nation
-- ----------------------------

-- ----------------------------
-- Table structure for ip2nationcountries
-- ----------------------------
DROP TABLE IF EXISTS `ip2nationcountries`;
CREATE TABLE `ip2nationcountries` (
  `code` varchar(4) NOT NULL DEFAULT '',
  `iso_code_2` varchar(2) NOT NULL DEFAULT '',
  `iso_code_3` varchar(3) DEFAULT '',
  `iso_country` varchar(255) NOT NULL DEFAULT '',
  `country` varchar(255) NOT NULL DEFAULT '',
  `lat` float NOT NULL DEFAULT '0',
  `lon` float NOT NULL DEFAULT '0',
  PRIMARY KEY (`code`),
  KEY `code` (`code`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of ip2nationcountries
-- ----------------------------

-- ----------------------------
-- Table structure for ip_banned
-- ----------------------------
DROP TABLE IF EXISTS `ip_banned`;
CREATE TABLE `ip_banned` (
  `ip` char(16) NOT NULL,
  `punishment_date` int(11) NOT NULL,
  `expiration_date` int(11) NOT NULL,
  `punished_by` varchar(16) NOT NULL,
  `ban_reason` varchar(100) NOT NULL,
  `active` decimal(1,0) NOT NULL DEFAULT '1',
  PRIMARY KEY (`ip`,`punishment_date`),
  KEY `expiration_date` (`expiration_date`),
  KEY `punishment_date` (`punishment_date`),
  KEY `active` (`active`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Banned IPs';

-- ----------------------------
-- Records of ip_banned
-- ----------------------------

-- ----------------------------
-- Table structure for locale
-- ----------------------------
DROP TABLE IF EXISTS `locale`;
CREATE TABLE `locale` (
  `locale_id` tinyint(3) unsigned NOT NULL,
  `name` varchar(45) NOT NULL,
  PRIMARY KEY (`locale_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of locale
-- ----------------------------
INSERT INTO `locale` VALUES ('0', 'enUS');
INSERT INTO `locale` VALUES ('1', 'koKR');
INSERT INTO `locale` VALUES ('2', 'frFR');
INSERT INTO `locale` VALUES ('3', 'deDE');
INSERT INTO `locale` VALUES ('4', 'zhCN');
INSERT INTO `locale` VALUES ('5', 'zhTW');
INSERT INTO `locale` VALUES ('6', 'esES');
INSERT INTO `locale` VALUES ('7', 'esMX');
INSERT INTO `locale` VALUES ('8', 'ruRU');

-- ----------------------------
-- Table structure for migrations
-- ----------------------------
DROP TABLE IF EXISTS `migrations`;
CREATE TABLE `migrations` (
  `id` int(10) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- ----------------------------
-- Records of migrations
-- ----------------------------

-- ----------------------------
-- Table structure for pattern_banned
-- ----------------------------
DROP TABLE IF EXISTS `pattern_banned`;
CREATE TABLE `pattern_banned` (
  `pattern_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `ip_pattern` varchar(255) NOT NULL,
  `local_ip_pattern` varchar(255) NOT NULL,
  `comment` char(100) NOT NULL,
  PRIMARY KEY (`pattern_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of pattern_banned
-- ----------------------------

-- ----------------------------
-- Table structure for punishment_type
-- ----------------------------
DROP TABLE IF EXISTS `punishment_type`;
CREATE TABLE `punishment_type` (
  `punishment_type_id` tinyint(3) unsigned NOT NULL,
  `name` char(30) NOT NULL,
  PRIMARY KEY (`punishment_type_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of punishment_type
-- ----------------------------
INSERT INTO `punishment_type` VALUES ('1', 'Account mute');
INSERT INTO `punishment_type` VALUES ('2', 'Account ban');

-- ----------------------------
-- Table structure for realms
-- ----------------------------
DROP TABLE IF EXISTS `realms`;
CREATE TABLE `realms` (
  `realm_id` int(10) unsigned NOT NULL,
  `name` varchar(32) NOT NULL,
  `ip_address` varchar(16) NOT NULL DEFAULT '127.0.0.1',
  `port` int(10) unsigned NOT NULL DEFAULT '8085',
  `icon` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `flags` tinyint(3) unsigned NOT NULL DEFAULT '6' COMMENT 'Supported masks: 0x1 (invalid, not show in realm list), 0x2 (offline, set by core), 0x4 (show version and build), 0x20 (recommended), 0x40 (new players)',
  `timezone` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `required_gmlevel` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `population` float unsigned NOT NULL DEFAULT '0',
  `allowed_builds` varchar(64) NOT NULL DEFAULT '8606',
  `current_realm` int(10) unsigned NOT NULL DEFAULT '8085',
  UNIQUE KEY `name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Realm System';

-- ----------------------------
-- Records of realms
-- ----------------------------
INSERT INTO `realms` VALUES ('1', 'Hellfire', '127.0.0.1', '8085', '1', '4', '1', '1', '0.02', '8606 ', '8085');
INSERT INTO `realms` VALUES ('2', 'HellfireDev', '127.0.0.1', '8085', '1', '4', '1', '5', '0.02', '8606 ', '8085');

-- ----------------------------
-- Table structure for realm_characters
-- ----------------------------
DROP TABLE IF EXISTS `realm_characters`;
CREATE TABLE `realm_characters` (
  `realm_id` int(10) unsigned NOT NULL,
  `account_id` int(10) unsigned NOT NULL,
  `characters_count` tinyint(3) unsigned NOT NULL DEFAULT '0',
  KEY `FK_realm_characters_realm_id` (`realm_id`),
  KEY `FK_realm_characters_account_id` (`account_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- ----------------------------
-- Records of realm_characters
-- ----------------------------
INSERT INTO `realm_characters` VALUES ('1', '1', '4');
INSERT INTO `realm_characters` VALUES ('1', '2', '1');

-- ----------------------------
-- Table structure for unqueue_account
-- ----------------------------
DROP TABLE IF EXISTS `unqueue_account`;
CREATE TABLE `unqueue_account` (
  `accid` int(11) unsigned NOT NULL,
  `add_time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `comment` text,
  PRIMARY KEY (`accid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC;

-- ----------------------------
-- Records of unqueue_account
-- ----------------------------
DROP TRIGGER IF EXISTS `account_creation`;
DELIMITER ;;
CREATE TRIGGER `account_creation` AFTER INSERT ON `account` FOR EACH ROW BEGIN
    replace INTO `account_permissions` (`account_id`, `realm_id`)
        SELECT NEW.`account_id`, `realm_id` FROM realms;
    replace INTO `account_support` (`account_id`)
        VALUES (NEW.`account_id`);
    replace INTO `account_session` (`account_id`)
        VALUES (NEW.`account_id`);
    replace INTO `realm_characters` (`account_id`, `realm_id`)
        SELECT NEW.`account_id`, `realm_id` FROM realms;
END
;;
DELIMITER ;
DROP TRIGGER IF EXISTS `email_upd_check`;
DELIMITER ;;
CREATE TRIGGER `email_upd_check` BEFORE UPDATE ON `account` FOR EACH ROW BEGIN
	IF NEW.email != OLD.email THEN
		INSERT INTO email_log VALUES ('XXX', OLD.email, NEW.email, NOW(), NEW.account_id);
	END IF;
  END
;;
DELIMITER ;
