/*
Navicat MySQL Data Transfer

Source Server         : localhost_3306
Source Server Version : 50553
Source Host           : localhost:3306
Source Database       : characters

Target Server Type    : MYSQL
Target Server Version : 50553
File Encoding         : 65001

Date: 2022-12-17 11:04:32
*/

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for arena_fights
-- ----------------------------
DROP TABLE IF EXISTS `arena_fights`;
CREATE TABLE `arena_fights` (
  `fight_guid` int(10) unsigned NOT NULL,
  `type` char(1) NOT NULL COMMENT '2, 3, 5',
  `winners_id` int(10) unsigned NOT NULL,
  `losers_id` int(10) unsigned NOT NULL,
  `winners_rating` int(10) unsigned NOT NULL,
  `losers_rating` int(10) unsigned NOT NULL,
  `rating_change` int(10) unsigned NOT NULL,
  `timestamp` datetime DEFAULT NULL,
  `length` int(6) unsigned NOT NULL DEFAULT '0' COMMENT 'time in seconds',
  PRIMARY KEY (`fight_guid`),
  KEY `winners_id` (`winners_id`),
  KEY `losers_id` (`losers_id`),
  KEY `timestamp` (`timestamp`),
  KEY `type` (`type`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of arena_fights
-- ----------------------------

-- ----------------------------
-- Table structure for arena_fights_detailed
-- ----------------------------
DROP TABLE IF EXISTS `arena_fights_detailed`;
CREATE TABLE `arena_fights_detailed` (
  `fight_guid` int(10) unsigned NOT NULL,
  `player_guid` int(10) unsigned NOT NULL,
  `team_id` int(10) unsigned NOT NULL,
  `damage_done` int(10) unsigned NOT NULL DEFAULT '0',
  `healing_done` int(10) unsigned NOT NULL DEFAULT '0',
  `kills` char(1) NOT NULL DEFAULT '0',
  `personal_rating` int(10) unsigned NOT NULL,
  `rating_change` int(11) NOT NULL,
  PRIMARY KEY (`fight_guid`,`player_guid`),
  KEY `player_guid` (`player_guid`),
  KEY `team_id` (`team_id`),
  KEY `fight_guid` (`fight_guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of arena_fights_detailed
-- ----------------------------

-- ----------------------------
-- Table structure for arena_team
-- ----------------------------
DROP TABLE IF EXISTS `arena_team`;
CREATE TABLE `arena_team` (
  `arenateamid` int(10) unsigned NOT NULL DEFAULT '0',
  `name` varchar(100) NOT NULL,
  `captainguid` int(10) unsigned NOT NULL DEFAULT '0',
  `type` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `BackgroundColor` int(10) unsigned NOT NULL DEFAULT '0',
  `EmblemStyle` int(10) unsigned NOT NULL DEFAULT '0',
  `EmblemColor` int(10) unsigned NOT NULL DEFAULT '0',
  `BorderStyle` int(10) unsigned NOT NULL DEFAULT '0',
  `BorderColor` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`arenateamid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of arena_team
-- ----------------------------

-- ----------------------------
-- Table structure for arena_team_member
-- ----------------------------
DROP TABLE IF EXISTS `arena_team_member`;
CREATE TABLE `arena_team_member` (
  `arenateamid` int(10) unsigned NOT NULL DEFAULT '0',
  `guid` int(10) unsigned NOT NULL DEFAULT '0',
  `played_week` int(10) unsigned NOT NULL DEFAULT '0',
  `wons_week` int(10) unsigned NOT NULL DEFAULT '0',
  `played_season` int(10) unsigned NOT NULL DEFAULT '0',
  `wons_season` int(10) unsigned NOT NULL DEFAULT '0',
  `personal_rating` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`arenateamid`,`guid`),
  KEY `arenateamid` (`arenateamid`),
  KEY `guid` (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of arena_team_member
-- ----------------------------

-- ----------------------------
-- Table structure for arena_team_stats
-- ----------------------------
DROP TABLE IF EXISTS `arena_team_stats`;
CREATE TABLE `arena_team_stats` (
  `arenateamid` int(10) unsigned NOT NULL DEFAULT '0',
  `rating` int(10) unsigned NOT NULL DEFAULT '0',
  `games` int(10) unsigned NOT NULL DEFAULT '0',
  `wins` int(10) unsigned NOT NULL DEFAULT '0',
  `played` int(10) unsigned NOT NULL DEFAULT '0',
  `wins2` int(10) unsigned NOT NULL DEFAULT '0',
  `rank` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`arenateamid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of arena_team_stats
-- ----------------------------

-- ----------------------------
-- Table structure for auctionhouse
-- ----------------------------
DROP TABLE IF EXISTS `auctionhouse`;
CREATE TABLE `auctionhouse` (
  `id` int(11) unsigned NOT NULL DEFAULT '0',
  `houseid` int(11) unsigned NOT NULL DEFAULT '0',
  `itemguid` int(11) unsigned NOT NULL DEFAULT '0',
  `item_template` int(11) unsigned NOT NULL DEFAULT '0',
  `item_count` int(11) unsigned NOT NULL DEFAULT '0',
  `item_randompropertyid` int(11) NOT NULL DEFAULT '0',
  `itemowner` int(11) unsigned NOT NULL DEFAULT '0',
  `buyoutprice` int(11) NOT NULL DEFAULT '0',
  `time` bigint(40) NOT NULL DEFAULT '0',
  `buyguid` int(11) unsigned NOT NULL DEFAULT '0',
  `lastbid` int(11) NOT NULL DEFAULT '0',
  `startbid` int(11) NOT NULL DEFAULT '0',
  `deposit` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `itemguid` (`itemguid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of auctionhouse
-- ----------------------------

-- ----------------------------
-- Table structure for boss_fights
-- ----------------------------
DROP TABLE IF EXISTS `boss_fights`;
CREATE TABLE `boss_fights` (
  `kill_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `mob_id` int(5) unsigned NOT NULL,
  `instance_id` int(5) unsigned NOT NULL,
  `guild_id` int(5) unsigned zerofill DEFAULT NULL,
  `length` int(5) unsigned NOT NULL,
  `date` datetime NOT NULL,
  PRIMARY KEY (`kill_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of boss_fights
-- ----------------------------

-- ----------------------------
-- Table structure for boss_fights_detailed
-- ----------------------------
DROP TABLE IF EXISTS `boss_fights_detailed`;
CREATE TABLE `boss_fights_detailed` (
  `kill_id` int(10) unsigned NOT NULL,
  `player_guid` int(10) unsigned NOT NULL,
  `damage` int(10) unsigned DEFAULT NULL,
  `healing` int(10) unsigned DEFAULT NULL,
  `deaths` int(2) unsigned DEFAULT NULL,
  PRIMARY KEY (`kill_id`,`player_guid`),
  KEY `player_guid` (`player_guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of boss_fights_detailed
-- ----------------------------

-- ----------------------------
-- Table structure for boss_fights_loot
-- ----------------------------
DROP TABLE IF EXISTS `boss_fights_loot`;
CREATE TABLE `boss_fights_loot` (
  `kill_id` int(10) unsigned NOT NULL,
  `item_id` int(6) unsigned NOT NULL,
  `count` int(2) DEFAULT NULL,
  PRIMARY KEY (`kill_id`,`item_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of boss_fights_loot
-- ----------------------------

-- ----------------------------
-- Table structure for boss_id_locations
-- ----------------------------
DROP TABLE IF EXISTS `boss_id_locations`;
CREATE TABLE `boss_id_locations` (
  `boss_id` int(2) unsigned NOT NULL,
  `boss_location` varchar(40) DEFAULT NULL,
  PRIMARY KEY (`boss_id`),
  CONSTRAINT `boss_id_locations_ibfk_1` FOREIGN KEY (`boss_id`) REFERENCES `boss_id_names` (`boss_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of boss_id_locations
-- ----------------------------

-- ----------------------------
-- Table structure for boss_id_names
-- ----------------------------
DROP TABLE IF EXISTS `boss_id_names`;
CREATE TABLE `boss_id_names` (
  `boss_id` int(5) unsigned NOT NULL,
  `boss_name` varchar(26) NOT NULL,
  PRIMARY KEY (`boss_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of boss_id_names
-- ----------------------------

-- ----------------------------
-- Table structure for bugreport
-- ----------------------------
DROP TABLE IF EXISTS `bugreport`;
CREATE TABLE `bugreport` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `type` varchar(255) NOT NULL DEFAULT '',
  `content` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of bugreport
-- ----------------------------

-- ----------------------------
-- Table structure for characters
-- ----------------------------
DROP TABLE IF EXISTS `characters`;
CREATE TABLE `characters` (
  `guid` int(11) unsigned NOT NULL DEFAULT '0',
  `account` int(11) unsigned NOT NULL DEFAULT '0',
  `data` longtext,
  `name` varchar(12) NOT NULL DEFAULT '',
  `race` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `class` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `gender` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `xp` int(10) unsigned NOT NULL DEFAULT '0',
  `money` int(10) unsigned NOT NULL DEFAULT '0',
  `playerBytes` int(10) unsigned NOT NULL DEFAULT '0',
  `playerBytes2` int(10) unsigned NOT NULL DEFAULT '0',
  `playerFlags` int(10) unsigned NOT NULL DEFAULT '0',
  `level` tinyint(3) unsigned DEFAULT '1',
  `title` bigint(30) unsigned DEFAULT NULL COMMENT 'Character''s title',
  `position_x` float NOT NULL DEFAULT '0',
  `position_y` float NOT NULL DEFAULT '0',
  `position_z` float NOT NULL DEFAULT '0',
  `map` int(11) unsigned NOT NULL DEFAULT '0',
  `instance_id` int(11) unsigned NOT NULL DEFAULT '0',
  `dungeon_difficulty` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `orientation` float NOT NULL DEFAULT '0',
  `taximask` longtext,
  `online` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `cinematic` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `totaltime` int(11) unsigned NOT NULL DEFAULT '0',
  `leveltime` int(11) unsigned NOT NULL DEFAULT '0',
  `logout_time` bigint(20) unsigned NOT NULL DEFAULT '0',
  `is_logout_resting` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `rest_bonus` float NOT NULL DEFAULT '0',
  `resettalents_cost` int(11) unsigned NOT NULL DEFAULT '0',
  `resettalents_time` bigint(20) unsigned NOT NULL DEFAULT '0',
  `trans_x` float NOT NULL DEFAULT '0',
  `trans_y` float NOT NULL DEFAULT '0',
  `trans_z` float NOT NULL DEFAULT '0',
  `trans_o` float NOT NULL DEFAULT '0',
  `transguid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `extra_flags` int(11) unsigned NOT NULL DEFAULT '0',
  `stable_slots` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `at_login` int(11) unsigned NOT NULL DEFAULT '0',
  `zone` int(11) unsigned NOT NULL DEFAULT '0',
  `death_expire_time` bigint(20) unsigned NOT NULL DEFAULT '0',
  `taxi_path` text,
  `arena_pending_points` int(10) unsigned NOT NULL DEFAULT '0',
  `latency` int(11) unsigned NOT NULL DEFAULT '0',
  `grantableLevels` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `changeRaceTo` tinyint(4) DEFAULT '0',
  PRIMARY KEY (`guid`),
  KEY `account` (`account`),
  KEY `online` (`online`),
  KEY `name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of characters
-- ----------------------------
INSERT INTO `characters` VALUES ('2', '1', '2 0 25 0 1080033280 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 6549 125 0 0 100 0 6549 125 1000 0 100 0 60 2 16843010 0 0 0 0 0 0 0 0 0 4104 0 11010 41953 2457 25289 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 286332689 0 0 0 0 0 0 0 0 0 0 0 0 0 1010580540 0 0 0 0 0 0 0 0 0 0 0 0 0 6144 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1153138688 1157234688 1161117696 1053240066 1069547520 52 52 0 1147181932 1150817718 0 0 0 0 0 0 0 0 0 1065353216 0 0 0 0 284 123 504 27 48 1126039552 1110441984 1136820224 0 0 0 0 0 0 0 10931 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1689 10240 728 328 0 173 96 0 1146370896 1151929767 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2048 0 0 50594567 16777221 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 22418 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 22732 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 22419 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 6125 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 22416 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 22422 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 22417 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 22420 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 22423 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 22421 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 23059 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 21205 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 19406 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 19431 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 21710 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 23577 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 23043 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 22811 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 77 1073741824 84 1073741824 78 1073741824 2 1073741824 75 1073741824 81 1073741824 76 1073741824 79 1073741824 82 1073741824 80 1073741824 83 1073741824 89 1073741824 90 1073741824 91 1073741824 85 1073741824 92 1073741824 87 1073741824 88 1073741824 0 0 0 0 0 0 0 0 0 0 8 1073741824 86 1073741824 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 560 494000 26 65537 0 95 19661100 0 44 19661100 0 172 19661100 0 43 19661100 0 162 19661100 0 109 19661100 0 413 65537 0 414 65537 0 415 65537 0 433 65537 0 118 19661100 0 293 65537 0 45 19661100 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 48 2 0 0 1101990461 1100477891 1092070932 5 5 1090823128 1090823128 0 0 0 0 0 0 0 0 237 0 0 33554432 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 8 0 0 0 2147483648 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 370500 998909799 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1083703297 1083703297 1083703297 1083703297 1083703297 1083703297 1083703297 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 4294967295 0 169 84 0 62 70 70 0 14 14 0 0 0 0 0 0 0 0 0 0 0 0 0 14 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 70 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ', 'Develop', '2', '1', '1', '560', '998909799', '50594567', '16777221', '2048', '60', '0', '3067.67', '-3563.84', '131.595', '0', '0', '0', '3.01608', '4194304 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 ', '0', '1', '732', '685', '1670999706', '0', '370500', '0', '0', '0', '0', '0', '0', '0', '4', '0', '0', '139', '0', '', '0', '0', '0', '0');
INSERT INTO `characters` VALUES ('3', '1', '3 0 25 0 1065353216 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 9394 0 0 0 100 0 9394 305 1000 0 100 0 70 4 50332676 0 0 0 0 0 0 0 0 0 134221832 0 22988 24252 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 6425 0 0 0 0 0 0 0 0 0 0 0 0 0 17990 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1156211167 1147822559 1157308344 1053240066 1069547520 55 55 15290 1142387507 1145402163 1130573004 1133778534 0 0 0 0 0 0 0 1065353216 0 0 0 0 92 567 587 39 58 0 1136951296 1139703808 0 0 0 0 0 0 0 4095 0 0 10 0 0 0 0 0 0 1092616192 0 0 0 0 0 0 0 0 0 0 0 3704 10240 779 1180 0 627 1180 0 1143229879 1146113463 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1024 0 0 100664581 33554432 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 34244 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 34358 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 31030 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 49 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 34397 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 34558 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 34188 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 34575 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 34448 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 34370 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 34189 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 34361 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 28830 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 34427 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 34241 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 32837 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 32838 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 34196 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 31405 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 43 1073741824 44 1073741824 45 1073741824 16 1073741824 47 1073741824 53 1073741824 54 1073741824 55 1073741824 48 1073741824 49 1073741824 56 1073741824 57 1073741824 58 1073741824 59 1073741824 46 1073741824 50 1073741824 51 1073741824 52 1073741824 66 1073741824 63 1073741824 60 1073741824 61 1073741824 62 1073741824 64 1073741824 26 1073741824 67 1073741824 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 779700 95 22937950 0 162 22937950 0 98 19661100 0 113 19661100 0 173 22937950 0 38 65537 0 253 65537 0 176 22937950 0 414 65537 0 415 65537 0 66291 24576350 0 43 22937950 0 45 22937950 0 118 22937601 0 633 22937950 0 762 19661100 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 1107757630 1084227584 16 16 1104027330 1104027330 1104027330 0 0 0 0 0 0 0 3 1 0 0 0 512 0 0 0 2 0 0 0 0 0 0 0 0 131072 0 10240 0 65536 0 0 0 0 96 64 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 35700516 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1065353216 1065353216 1065353216 1065353216 1065353216 1065353216 1065353216 0 0 4294966047 458752 28053 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 4294967295 0 0 0 0 0 159 159 0 152 152 0 0 0 0 0 0 0 221 221 0 0 0 0 24 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 70 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ', 'By', '4', '4', '0', '0', '35700516', '100664581', '33554432', '1024', '70', '0', '5403.31', '-3313.98', '1651.65', '1', '0', '0', '5.09852', '100663296 0 0 8 0 0 0 0 0 0 0 0 0 0 0 0 ', '0', '1', '1319', '1319', '1655764835', '0', '0', '0', '0', '0', '0', '0', '0', '0', '4', '0', '0', '616', '0', '', '0', '0', '0', '0');
INSERT INTO `characters` VALUES ('4', '1', '4 0 25 0 1065353216 0 0 0 0 0 0 0 0 0 0 0 16884353 4046454784 0 0 0 0 18723 11726 0 0 100 0 18723 11726 1000 0 100 0 70 35 2049 0 0 0 0 0 0 0 0 0 4104 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 70 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1161527296 1157234688 1157234688 1065353216 1069547520 49 49 0 1134332196 1135839524 0 0 0 0 0 0 0 0 0 1065353216 0 0 0 0 533 539 1551 651 645 1140457472 1140457472 1153138688 1138320428 1138968018 0 0 0 0 0 21085 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2241 3393 10240 523 0 0 529 0 0 1117332626 1117463698 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 10 0 0 151257088 33554436 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 12064 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 6096 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2586 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 11508 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 13347 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 13262 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 99966 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 72 1073741824 0 0 0 0 36 1073741824 70 1073741824 0 0 0 0 71 1073741824 0 0 0 0 0 0 0 0 74 1073741824 0 0 0 0 73 1073741824 0 0 0 0 69 1073741824 0 0 0 0 0 0 0 0 38 1073741824 40 1073741824 42 1073741824 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 779700 95 22937950 0 8 65537 0 6 65537 0 162 22937950 0 136 22937950 0 98 19661100 0 228 22937950 0 415 65537 0 55 22937950 0 756 65537 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2 0 0 0 1103635415 0 0 0 1103635415 1093683118 1093683118 0 1091614802 1091614802 1091614802 1091614802 1091614802 1091614802 25 0 0 2147483648 2684354816 4 0 0 0 0 0 0 1048576 0 0 0 128 0 0 0 0 50331648 8 536870920 0 0 0 0 0 0 0 0 1024 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1065353216 1065353215 1065353215 1065353215 1065353215 1065353215 1065353215 0 0 0 8 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 4294967295 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1125744265 0 70 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ', 'Hellfire', '1', '8', '0', '0', '0', '151257088', '33554436', '10', '70', '0', '-360.746', '-2674.33', '95.726', '1', '0', '0', '1.67583', '2 0 0 8 0 0 0 0 0 0 0 0 0 0 0 0 ', '0', '1', '14634', '14634', '1671245609', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '17', '1655713615', '', '0', '0', '0', '0');

-- ----------------------------
-- Table structure for characters_goldsy
-- ----------------------------
DROP TABLE IF EXISTS `characters_goldsy`;
CREATE TABLE `characters_goldsy` (
  `guid` int(11) unsigned NOT NULL DEFAULT '0',
  `account` int(11) unsigned NOT NULL DEFAULT '0',
  `name` varchar(12) NOT NULL DEFAULT '',
  `money` int(10) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of characters_goldsy
-- ----------------------------

-- ----------------------------
-- Table structure for character_action
-- ----------------------------
DROP TABLE IF EXISTS `character_action`;
CREATE TABLE `character_action` (
  `guid` int(11) unsigned NOT NULL DEFAULT '0',
  `button` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `action` smallint(5) unsigned NOT NULL DEFAULT '0',
  `type` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `misc` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`button`),
  KEY `guid` (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of character_action
-- ----------------------------
INSERT INTO `character_action` VALUES ('2', '72', '6603', '0', '0');
INSERT INTO `character_action` VALUES ('2', '73', '25286', '0', '0');
INSERT INTO `character_action` VALUES ('2', '74', '7373', '0', '0');
INSERT INTO `character_action` VALUES ('2', '75', '41953', '0', '0');
INSERT INTO `character_action` VALUES ('2', '83', '117', '128', '0');
INSERT INTO `character_action` VALUES ('3', '0', '26862', '0', '0');
INSERT INTO `character_action` VALUES ('3', '1', '32684', '0', '0');
INSERT INTO `character_action` VALUES ('3', '2', '26865', '0', '0');
INSERT INTO `character_action` VALUES ('3', '3', '8643', '0', '0');
INSERT INTO `character_action` VALUES ('3', '4', '22988', '0', '0');
INSERT INTO `character_action` VALUES ('3', '72', '11297', '0', '0');
INSERT INTO `character_action` VALUES ('3', '73', '1833', '0', '0');
INSERT INTO `character_action` VALUES ('3', '74', '26884', '0', '0');
INSERT INTO `character_action` VALUES ('3', '75', '1725', '0', '0');
INSERT INTO `character_action` VALUES ('4', '0', '5', '0', '0');
INSERT INTO `character_action` VALUES ('4', '1', '9454', '0', '0');
INSERT INTO `character_action` VALUES ('4', '2', '1302', '0', '0');
INSERT INTO `character_action` VALUES ('4', '3', '10032', '0', '0');
INSERT INTO `character_action` VALUES ('4', '4', '35999', '0', '0');
INSERT INTO `character_action` VALUES ('4', '6', '1953', '0', '0');
INSERT INTO `character_action` VALUES ('4', '10', '159', '128', '0');
INSERT INTO `character_action` VALUES ('4', '11', '2070', '128', '0');

-- ----------------------------
-- Table structure for character_aura
-- ----------------------------
DROP TABLE IF EXISTS `character_aura`;
CREATE TABLE `character_aura` (
  `guid` int(11) unsigned NOT NULL DEFAULT '0',
  `caster_guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `spell` int(11) unsigned NOT NULL DEFAULT '0',
  `effect_index` int(11) unsigned NOT NULL DEFAULT '0',
  `stackcount` int(11) NOT NULL DEFAULT '1',
  `amount` int(11) NOT NULL DEFAULT '0',
  `maxduration` int(11) NOT NULL DEFAULT '0',
  `remaintime` int(11) NOT NULL DEFAULT '0',
  `remaincharges` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`spell`,`effect_index`),
  KEY `guid` (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of character_aura
-- ----------------------------
INSERT INTO `character_aura` VALUES ('2', '2', '11010', '0', '1', '1', '-1', '-1', '-1');
INSERT INTO `character_aura` VALUES ('2', '12', '25289', '0', '1', '232', '120000', '86588', '-1');
INSERT INTO `character_aura` VALUES ('2', '2', '41953', '0', '25', '10', '299000', '116704', '-1');
INSERT INTO `character_aura` VALUES ('2', '2', '41953', '1', '25', '15', '299000', '116704', '-1');
INSERT INTO `character_aura` VALUES ('2', '2', '41953', '2', '25', '20', '299000', '116704', '-1');
INSERT INTO `character_aura` VALUES ('3', '3', '22988', '0', '1', '1400', '-1', '-1', '-1');
INSERT INTO `character_aura` VALUES ('3', '3', '22988', '1', '1', '20', '-1', '-1', '-1');
INSERT INTO `character_aura` VALUES ('3', '3', '22988', '2', '1', '30', '-1', '-1', '-1');
INSERT INTO `character_aura` VALUES ('3', '3', '24252', '0', '1', '1', '-1', '-1', '-1');
INSERT INTO `character_aura` VALUES ('3', '3', '24252', '1', '1', '100', '-1', '-1', '-1');

-- ----------------------------
-- Table structure for character_battleground_data
-- ----------------------------
DROP TABLE IF EXISTS `character_battleground_data`;
CREATE TABLE `character_battleground_data` (
  `guid` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Global Unique Identifier',
  `instance_id` int(11) unsigned NOT NULL DEFAULT '0',
  `team` int(11) unsigned NOT NULL DEFAULT '0',
  `join_x` float NOT NULL DEFAULT '0',
  `join_y` float NOT NULL DEFAULT '0',
  `join_z` float NOT NULL DEFAULT '0',
  `join_o` float NOT NULL DEFAULT '0',
  `join_map` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Player System';

-- ----------------------------
-- Records of character_battleground_data
-- ----------------------------

-- ----------------------------
-- Table structure for character_bgcoord
-- ----------------------------
DROP TABLE IF EXISTS `character_bgcoord`;
CREATE TABLE `character_bgcoord` (
  `guid` int(11) unsigned NOT NULL DEFAULT '0',
  `bgid` int(10) unsigned NOT NULL DEFAULT '0',
  `bgteam` int(10) unsigned NOT NULL DEFAULT '0',
  `bgmap` int(10) unsigned NOT NULL DEFAULT '0',
  `bgx` float NOT NULL DEFAULT '0',
  `bgy` float NOT NULL DEFAULT '0',
  `bgz` float NOT NULL DEFAULT '0',
  `bgo` float NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of character_bgcoord
-- ----------------------------

-- ----------------------------
-- Table structure for character_declinedname
-- ----------------------------
DROP TABLE IF EXISTS `character_declinedname`;
CREATE TABLE `character_declinedname` (
  `guid` int(11) unsigned NOT NULL DEFAULT '0',
  `genitive` varchar(15) NOT NULL DEFAULT '',
  `dative` varchar(15) NOT NULL DEFAULT '',
  `accusative` varchar(15) NOT NULL DEFAULT '',
  `instrumental` varchar(15) NOT NULL DEFAULT '',
  `prepositional` varchar(15) NOT NULL DEFAULT '',
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of character_declinedname
-- ----------------------------

-- ----------------------------
-- Table structure for character_freerespecs
-- ----------------------------
DROP TABLE IF EXISTS `character_freerespecs`;
CREATE TABLE `character_freerespecs` (
  `guid` int(11) unsigned NOT NULL DEFAULT '0',
  `expiration_date` int(11) unsigned NOT NULL DEFAULT '0',
  KEY `guid` (`guid`),
  KEY `expiration_date` (`expiration_date`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of character_freerespecs
-- ----------------------------

-- ----------------------------
-- Table structure for character_gifts
-- ----------------------------
DROP TABLE IF EXISTS `character_gifts`;
CREATE TABLE `character_gifts` (
  `guid` int(20) unsigned NOT NULL DEFAULT '0',
  `item_guid` int(11) unsigned NOT NULL DEFAULT '0',
  `entry` int(20) unsigned NOT NULL DEFAULT '0',
  `flags` int(20) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`item_guid`),
  KEY `guid` (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of character_gifts
-- ----------------------------

-- ----------------------------
-- Table structure for character_homebind
-- ----------------------------
DROP TABLE IF EXISTS `character_homebind`;
CREATE TABLE `character_homebind` (
  `guid` int(11) unsigned NOT NULL DEFAULT '0',
  `map` int(11) unsigned NOT NULL DEFAULT '0',
  `zone` int(11) unsigned NOT NULL DEFAULT '0',
  `position_x` float NOT NULL DEFAULT '0',
  `position_y` float NOT NULL DEFAULT '0',
  `position_z` float NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of character_homebind
-- ----------------------------
INSERT INTO `character_homebind` VALUES ('2', '1', '14', '-618.518', '-4251.67', '38.718');
INSERT INTO `character_homebind` VALUES ('3', '1', '141', '10311.3', '832.463', '1326.41');
INSERT INTO `character_homebind` VALUES ('4', '0', '12', '-8949.95', '-132.493', '83.5312');

-- ----------------------------
-- Table structure for character_instance
-- ----------------------------
DROP TABLE IF EXISTS `character_instance`;
CREATE TABLE `character_instance` (
  `guid` int(11) unsigned NOT NULL DEFAULT '0',
  `instance` int(11) unsigned NOT NULL DEFAULT '0',
  `permanent` tinyint(1) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`instance`),
  KEY `instance` (`instance`),
  KEY `guid` (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of character_instance
-- ----------------------------

-- ----------------------------
-- Table structure for character_inventory
-- ----------------------------
DROP TABLE IF EXISTS `character_inventory`;
CREATE TABLE `character_inventory` (
  `guid` int(11) unsigned NOT NULL DEFAULT '0',
  `bag` int(11) unsigned NOT NULL DEFAULT '0',
  `slot` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `item` int(11) unsigned NOT NULL DEFAULT '0',
  `item_template` int(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`item`),
  KEY `guid` (`guid`),
  KEY `item_template` (`item_template`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of character_inventory
-- ----------------------------
INSERT INTO `character_inventory` VALUES ('2', '0', '3', '2', '6125');
INSERT INTO `character_inventory` VALUES ('2', '0', '23', '8', '6948');
INSERT INTO `character_inventory` VALUES ('3', '0', '3', '16', '49');
INSERT INTO `character_inventory` VALUES ('3', '0', '24', '26', '6948');
INSERT INTO `character_inventory` VALUES ('4', '0', '3', '36', '6096');
INSERT INTO `character_inventory` VALUES ('4', '0', '23', '38', '2070');
INSERT INTO `character_inventory` VALUES ('4', '0', '24', '40', '159');
INSERT INTO `character_inventory` VALUES ('4', '0', '25', '42', '6948');
INSERT INTO `character_inventory` VALUES ('3', '0', '0', '43', '34244');
INSERT INTO `character_inventory` VALUES ('3', '0', '1', '44', '34358');
INSERT INTO `character_inventory` VALUES ('3', '0', '2', '45', '31030');
INSERT INTO `character_inventory` VALUES ('3', '0', '14', '46', '34241');
INSERT INTO `character_inventory` VALUES ('3', '0', '4', '47', '34397');
INSERT INTO `character_inventory` VALUES ('3', '0', '8', '48', '34448');
INSERT INTO `character_inventory` VALUES ('3', '0', '9', '49', '34370');
INSERT INTO `character_inventory` VALUES ('3', '0', '15', '50', '32837');
INSERT INTO `character_inventory` VALUES ('3', '0', '16', '51', '32838');
INSERT INTO `character_inventory` VALUES ('3', '0', '17', '52', '34196');
INSERT INTO `character_inventory` VALUES ('3', '0', '5', '53', '34558');
INSERT INTO `character_inventory` VALUES ('3', '0', '6', '54', '34188');
INSERT INTO `character_inventory` VALUES ('3', '0', '7', '55', '34575');
INSERT INTO `character_inventory` VALUES ('3', '0', '10', '56', '34189');
INSERT INTO `character_inventory` VALUES ('3', '0', '11', '57', '34361');
INSERT INTO `character_inventory` VALUES ('3', '0', '12', '58', '28830');
INSERT INTO `character_inventory` VALUES ('3', '0', '13', '59', '34427');
INSERT INTO `character_inventory` VALUES ('3', '0', '20', '60', '23162');
INSERT INTO `character_inventory` VALUES ('3', '0', '21', '61', '23162');
INSERT INTO `character_inventory` VALUES ('3', '0', '22', '62', '23162');
INSERT INTO `character_inventory` VALUES ('3', '0', '19', '63', '23162');
INSERT INTO `character_inventory` VALUES ('3', '0', '23', '64', '19902');
INSERT INTO `character_inventory` VALUES ('3', '0', '18', '66', '31405');
INSERT INTO `character_inventory` VALUES ('3', '0', '25', '67', '30504');
INSERT INTO `character_inventory` VALUES ('3', '62', '0', '68', '28053');
INSERT INTO `character_inventory` VALUES ('4', '0', '18', '69', '99966');
INSERT INTO `character_inventory` VALUES ('4', '0', '4', '70', '2586');
INSERT INTO `character_inventory` VALUES ('4', '0', '7', '71', '11508');
INSERT INTO `character_inventory` VALUES ('4', '0', '0', '72', '12064');
INSERT INTO `character_inventory` VALUES ('4', '0', '15', '73', '13262');
INSERT INTO `character_inventory` VALUES ('4', '0', '12', '74', '13347');
INSERT INTO `character_inventory` VALUES ('2', '0', '4', '75', '22416');
INSERT INTO `character_inventory` VALUES ('2', '0', '6', '76', '22417');
INSERT INTO `character_inventory` VALUES ('2', '0', '0', '77', '22418');
INSERT INTO `character_inventory` VALUES ('2', '0', '2', '78', '22419');
INSERT INTO `character_inventory` VALUES ('2', '0', '7', '79', '22420');
INSERT INTO `character_inventory` VALUES ('2', '0', '9', '80', '22421');
INSERT INTO `character_inventory` VALUES ('2', '0', '5', '81', '22422');
INSERT INTO `character_inventory` VALUES ('2', '0', '8', '82', '22423');
INSERT INTO `character_inventory` VALUES ('2', '0', '10', '83', '23059');
INSERT INTO `character_inventory` VALUES ('2', '0', '1', '84', '22732');
INSERT INTO `character_inventory` VALUES ('2', '0', '14', '85', '21710');
INSERT INTO `character_inventory` VALUES ('2', '0', '24', '86', '19019');
INSERT INTO `character_inventory` VALUES ('2', '0', '16', '87', '23043');
INSERT INTO `character_inventory` VALUES ('2', '0', '17', '88', '22811');
INSERT INTO `character_inventory` VALUES ('2', '0', '11', '89', '21205');
INSERT INTO `character_inventory` VALUES ('2', '0', '12', '90', '19406');
INSERT INTO `character_inventory` VALUES ('2', '0', '13', '91', '19431');
INSERT INTO `character_inventory` VALUES ('2', '0', '15', '92', '23577');

-- ----------------------------
-- Table structure for character_pet
-- ----------------------------
DROP TABLE IF EXISTS `character_pet`;
CREATE TABLE `character_pet` (
  `id` int(11) unsigned NOT NULL,
  `entry` int(11) unsigned NOT NULL DEFAULT '0',
  `owner` int(11) unsigned NOT NULL DEFAULT '0',
  `modelid` int(11) unsigned DEFAULT '0',
  `CreatedBySpell` int(11) unsigned NOT NULL DEFAULT '0',
  `PetType` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `level` int(11) unsigned NOT NULL DEFAULT '1',
  `exp` int(11) unsigned NOT NULL DEFAULT '0',
  `Reactstate` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `loyaltypoints` int(11) NOT NULL DEFAULT '0',
  `loyalty` int(11) unsigned NOT NULL DEFAULT '0',
  `trainpoint` int(11) NOT NULL DEFAULT '0',
  `name` varchar(100) DEFAULT 'Pet',
  `renamed` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `slot` int(11) unsigned NOT NULL DEFAULT '0',
  `curhealth` int(11) unsigned NOT NULL DEFAULT '1',
  `curmana` int(11) unsigned NOT NULL DEFAULT '0',
  `curhappiness` int(11) unsigned NOT NULL DEFAULT '0',
  `savetime` bigint(20) unsigned NOT NULL DEFAULT '0',
  `resettalents_cost` int(11) unsigned NOT NULL DEFAULT '0',
  `resettalents_time` bigint(20) unsigned NOT NULL DEFAULT '0',
  `abdata` longtext,
  `teachspelldata` longtext,
  PRIMARY KEY (`id`),
  KEY `owner` (`owner`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of character_pet
-- ----------------------------

-- ----------------------------
-- Table structure for character_pet_declinedname
-- ----------------------------
DROP TABLE IF EXISTS `character_pet_declinedname`;
CREATE TABLE `character_pet_declinedname` (
  `id` int(11) unsigned NOT NULL DEFAULT '0',
  `owner` int(11) unsigned NOT NULL DEFAULT '0',
  `genitive` varchar(12) NOT NULL DEFAULT '',
  `dative` varchar(12) NOT NULL DEFAULT '',
  `accusative` varchar(12) NOT NULL DEFAULT '',
  `instrumental` varchar(12) NOT NULL DEFAULT '',
  `prepositional` varchar(12) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  KEY `owner` (`owner`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of character_pet_declinedname
-- ----------------------------

-- ----------------------------
-- Table structure for character_queststatus
-- ----------------------------
DROP TABLE IF EXISTS `character_queststatus`;
CREATE TABLE `character_queststatus` (
  `guid` int(11) unsigned NOT NULL DEFAULT '0',
  `quest` int(11) unsigned NOT NULL DEFAULT '0',
  `status` int(11) unsigned NOT NULL DEFAULT '0',
  `rewarded` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `explored` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `timer` bigint(20) unsigned NOT NULL DEFAULT '0',
  `mobcount1` int(11) unsigned NOT NULL DEFAULT '0',
  `mobcount2` int(11) unsigned NOT NULL DEFAULT '0',
  `mobcount3` int(11) unsigned NOT NULL DEFAULT '0',
  `mobcount4` int(11) unsigned NOT NULL DEFAULT '0',
  `itemcount1` int(11) unsigned NOT NULL DEFAULT '0',
  `itemcount2` int(11) unsigned NOT NULL DEFAULT '0',
  `itemcount3` int(11) unsigned NOT NULL DEFAULT '0',
  `itemcount4` int(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`quest`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of character_queststatus
-- ----------------------------

-- ----------------------------
-- Table structure for character_queststatus_daily
-- ----------------------------
DROP TABLE IF EXISTS `character_queststatus_daily`;
CREATE TABLE `character_queststatus_daily` (
  `guid` int(11) unsigned NOT NULL DEFAULT '0',
  `quest` int(11) unsigned NOT NULL DEFAULT '0',
  `time` bigint(20) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`quest`),
  KEY `guid` (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of character_queststatus_daily
-- ----------------------------

-- ----------------------------
-- Table structure for character_reputation
-- ----------------------------
DROP TABLE IF EXISTS `character_reputation`;
CREATE TABLE `character_reputation` (
  `guid` bigint(10) NOT NULL DEFAULT '0',
  `faction` bigint(10) NOT NULL DEFAULT '0',
  `standing` bigint(10) DEFAULT NULL,
  `flags` bigint(10) DEFAULT NULL,
  PRIMARY KEY (`guid`,`faction`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of character_reputation
-- ----------------------------
INSERT INTO `character_reputation` VALUES ('2', '21', '0', '0');
INSERT INTO `character_reputation` VALUES ('2', '46', '0', '4');
INSERT INTO `character_reputation` VALUES ('2', '47', '0', '6');
INSERT INTO `character_reputation` VALUES ('2', '54', '0', '6');
INSERT INTO `character_reputation` VALUES ('2', '59', '0', '16');
INSERT INTO `character_reputation` VALUES ('2', '67', '0', '25');
INSERT INTO `character_reputation` VALUES ('2', '68', '0', '17');
INSERT INTO `character_reputation` VALUES ('2', '69', '0', '6');
INSERT INTO `character_reputation` VALUES ('2', '70', '0', '2');
INSERT INTO `character_reputation` VALUES ('2', '72', '0', '6');
INSERT INTO `character_reputation` VALUES ('2', '76', '0', '17');
INSERT INTO `character_reputation` VALUES ('2', '81', '0', '17');
INSERT INTO `character_reputation` VALUES ('2', '83', '0', '4');
INSERT INTO `character_reputation` VALUES ('2', '86', '0', '4');
INSERT INTO `character_reputation` VALUES ('2', '87', '0', '2');
INSERT INTO `character_reputation` VALUES ('2', '92', '0', '2');
INSERT INTO `character_reputation` VALUES ('2', '93', '0', '2');
INSERT INTO `character_reputation` VALUES ('2', '169', '0', '8');
INSERT INTO `character_reputation` VALUES ('2', '270', '0', '16');
INSERT INTO `character_reputation` VALUES ('2', '289', '0', '4');
INSERT INTO `character_reputation` VALUES ('2', '349', '0', '0');
INSERT INTO `character_reputation` VALUES ('2', '369', '0', '0');
INSERT INTO `character_reputation` VALUES ('2', '469', '0', '14');
INSERT INTO `character_reputation` VALUES ('2', '470', '0', '0');
INSERT INTO `character_reputation` VALUES ('2', '471', '0', '22');
INSERT INTO `character_reputation` VALUES ('2', '509', '0', '2');
INSERT INTO `character_reputation` VALUES ('2', '510', '0', '16');
INSERT INTO `character_reputation` VALUES ('2', '529', '0', '0');
INSERT INTO `character_reputation` VALUES ('2', '530', '0', '17');
INSERT INTO `character_reputation` VALUES ('2', '549', '0', '4');
INSERT INTO `character_reputation` VALUES ('2', '550', '0', '4');
INSERT INTO `character_reputation` VALUES ('2', '551', '0', '4');
INSERT INTO `character_reputation` VALUES ('2', '569', '0', '4');
INSERT INTO `character_reputation` VALUES ('2', '570', '0', '4');
INSERT INTO `character_reputation` VALUES ('2', '571', '0', '4');
INSERT INTO `character_reputation` VALUES ('2', '574', '0', '4');
INSERT INTO `character_reputation` VALUES ('2', '576', '0', '2');
INSERT INTO `character_reputation` VALUES ('2', '577', '0', '0');
INSERT INTO `character_reputation` VALUES ('2', '589', '0', '6');
INSERT INTO `character_reputation` VALUES ('2', '609', '0', '0');
INSERT INTO `character_reputation` VALUES ('2', '729', '0', '16');
INSERT INTO `character_reputation` VALUES ('2', '730', '0', '2');
INSERT INTO `character_reputation` VALUES ('2', '749', '0', '0');
INSERT INTO `character_reputation` VALUES ('2', '809', '0', '16');
INSERT INTO `character_reputation` VALUES ('2', '889', '0', '16');
INSERT INTO `character_reputation` VALUES ('2', '890', '0', '6');
INSERT INTO `character_reputation` VALUES ('2', '891', '0', '14');
INSERT INTO `character_reputation` VALUES ('2', '892', '0', '24');
INSERT INTO `character_reputation` VALUES ('2', '909', '0', '16');
INSERT INTO `character_reputation` VALUES ('2', '910', '84000', '1');
INSERT INTO `character_reputation` VALUES ('2', '911', '0', '17');
INSERT INTO `character_reputation` VALUES ('2', '922', '0', '16');
INSERT INTO `character_reputation` VALUES ('2', '930', '0', '6');
INSERT INTO `character_reputation` VALUES ('2', '932', '0', '80');
INSERT INTO `character_reputation` VALUES ('2', '933', '0', '16');
INSERT INTO `character_reputation` VALUES ('2', '934', '0', '80');
INSERT INTO `character_reputation` VALUES ('2', '935', '0', '16');
INSERT INTO `character_reputation` VALUES ('2', '936', '0', '28');
INSERT INTO `character_reputation` VALUES ('2', '941', '0', '16');
INSERT INTO `character_reputation` VALUES ('2', '942', '0', '16');
INSERT INTO `character_reputation` VALUES ('2', '946', '0', '2');
INSERT INTO `character_reputation` VALUES ('2', '947', '0', '16');
INSERT INTO `character_reputation` VALUES ('2', '967', '0', '16');
INSERT INTO `character_reputation` VALUES ('2', '970', '0', '0');
INSERT INTO `character_reputation` VALUES ('2', '978', '0', '2');
INSERT INTO `character_reputation` VALUES ('2', '980', '0', '8');
INSERT INTO `character_reputation` VALUES ('2', '989', '0', '16');
INSERT INTO `character_reputation` VALUES ('2', '990', '0', '16');
INSERT INTO `character_reputation` VALUES ('2', '1005', '0', '4');
INSERT INTO `character_reputation` VALUES ('2', '1011', '0', '16');
INSERT INTO `character_reputation` VALUES ('2', '1012', '0', '16');
INSERT INTO `character_reputation` VALUES ('2', '1015', '0', '2');
INSERT INTO `character_reputation` VALUES ('2', '1031', '0', '16');
INSERT INTO `character_reputation` VALUES ('2', '1038', '0', '16');
INSERT INTO `character_reputation` VALUES ('2', '1077', '0', '16');
INSERT INTO `character_reputation` VALUES ('3', '21', '0', '0');
INSERT INTO `character_reputation` VALUES ('3', '46', '0', '4');
INSERT INTO `character_reputation` VALUES ('3', '47', '0', '17');
INSERT INTO `character_reputation` VALUES ('3', '54', '0', '17');
INSERT INTO `character_reputation` VALUES ('3', '59', '0', '16');
INSERT INTO `character_reputation` VALUES ('3', '67', '0', '14');
INSERT INTO `character_reputation` VALUES ('3', '68', '0', '6');
INSERT INTO `character_reputation` VALUES ('3', '69', '0', '17');
INSERT INTO `character_reputation` VALUES ('3', '70', '0', '2');
INSERT INTO `character_reputation` VALUES ('3', '72', '0', '17');
INSERT INTO `character_reputation` VALUES ('3', '76', '0', '6');
INSERT INTO `character_reputation` VALUES ('3', '81', '0', '6');
INSERT INTO `character_reputation` VALUES ('3', '83', '0', '4');
INSERT INTO `character_reputation` VALUES ('3', '86', '0', '4');
INSERT INTO `character_reputation` VALUES ('3', '87', '0', '2');
INSERT INTO `character_reputation` VALUES ('3', '92', '0', '2');
INSERT INTO `character_reputation` VALUES ('3', '93', '0', '2');
INSERT INTO `character_reputation` VALUES ('3', '169', '0', '8');
INSERT INTO `character_reputation` VALUES ('3', '270', '0', '16');
INSERT INTO `character_reputation` VALUES ('3', '289', '0', '4');
INSERT INTO `character_reputation` VALUES ('3', '349', '0', '0');
INSERT INTO `character_reputation` VALUES ('3', '369', '0', '0');
INSERT INTO `character_reputation` VALUES ('3', '469', '0', '25');
INSERT INTO `character_reputation` VALUES ('3', '470', '0', '0');
INSERT INTO `character_reputation` VALUES ('3', '471', '0', '20');
INSERT INTO `character_reputation` VALUES ('3', '509', '0', '16');
INSERT INTO `character_reputation` VALUES ('3', '510', '0', '2');
INSERT INTO `character_reputation` VALUES ('3', '529', '36', '1');
INSERT INTO `character_reputation` VALUES ('3', '530', '0', '6');
INSERT INTO `character_reputation` VALUES ('3', '549', '0', '4');
INSERT INTO `character_reputation` VALUES ('3', '550', '0', '4');
INSERT INTO `character_reputation` VALUES ('3', '551', '0', '4');
INSERT INTO `character_reputation` VALUES ('3', '569', '0', '4');
INSERT INTO `character_reputation` VALUES ('3', '570', '0', '4');
INSERT INTO `character_reputation` VALUES ('3', '571', '0', '4');
INSERT INTO `character_reputation` VALUES ('3', '574', '0', '4');
INSERT INTO `character_reputation` VALUES ('3', '576', '0', '2');
INSERT INTO `character_reputation` VALUES ('3', '577', '0', '0');
INSERT INTO `character_reputation` VALUES ('3', '589', '0', '0');
INSERT INTO `character_reputation` VALUES ('3', '609', '0', '0');
INSERT INTO `character_reputation` VALUES ('3', '729', '0', '2');
INSERT INTO `character_reputation` VALUES ('3', '730', '0', '16');
INSERT INTO `character_reputation` VALUES ('3', '749', '0', '0');
INSERT INTO `character_reputation` VALUES ('3', '809', '0', '16');
INSERT INTO `character_reputation` VALUES ('3', '889', '0', '6');
INSERT INTO `character_reputation` VALUES ('3', '890', '0', '16');
INSERT INTO `character_reputation` VALUES ('3', '891', '0', '24');
INSERT INTO `character_reputation` VALUES ('3', '892', '0', '14');
INSERT INTO `character_reputation` VALUES ('3', '909', '0', '16');
INSERT INTO `character_reputation` VALUES ('3', '910', '0', '2');
INSERT INTO `character_reputation` VALUES ('3', '911', '0', '6');
INSERT INTO `character_reputation` VALUES ('3', '922', '0', '6');
INSERT INTO `character_reputation` VALUES ('3', '930', '0', '17');
INSERT INTO `character_reputation` VALUES ('3', '932', '0', '80');
INSERT INTO `character_reputation` VALUES ('3', '933', '0', '16');
INSERT INTO `character_reputation` VALUES ('3', '934', '0', '80');
INSERT INTO `character_reputation` VALUES ('3', '935', '0', '16');
INSERT INTO `character_reputation` VALUES ('3', '936', '0', '28');
INSERT INTO `character_reputation` VALUES ('3', '941', '0', '6');
INSERT INTO `character_reputation` VALUES ('3', '942', '0', '16');
INSERT INTO `character_reputation` VALUES ('3', '946', '0', '16');
INSERT INTO `character_reputation` VALUES ('3', '947', '0', '2');
INSERT INTO `character_reputation` VALUES ('3', '967', '0', '16');
INSERT INTO `character_reputation` VALUES ('3', '970', '0', '0');
INSERT INTO `character_reputation` VALUES ('3', '978', '0', '16');
INSERT INTO `character_reputation` VALUES ('3', '980', '0', '8');
INSERT INTO `character_reputation` VALUES ('3', '989', '0', '16');
INSERT INTO `character_reputation` VALUES ('3', '990', '0', '16');
INSERT INTO `character_reputation` VALUES ('3', '1005', '0', '4');
INSERT INTO `character_reputation` VALUES ('3', '1011', '0', '16');
INSERT INTO `character_reputation` VALUES ('3', '1012', '0', '16');
INSERT INTO `character_reputation` VALUES ('3', '1015', '0', '2');
INSERT INTO `character_reputation` VALUES ('3', '1031', '0', '16');
INSERT INTO `character_reputation` VALUES ('3', '1038', '0', '16');
INSERT INTO `character_reputation` VALUES ('3', '1077', '0', '16');
INSERT INTO `character_reputation` VALUES ('4', '21', '0', '0');
INSERT INTO `character_reputation` VALUES ('4', '46', '0', '4');
INSERT INTO `character_reputation` VALUES ('4', '47', '0', '17');
INSERT INTO `character_reputation` VALUES ('4', '54', '0', '17');
INSERT INTO `character_reputation` VALUES ('4', '59', '0', '16');
INSERT INTO `character_reputation` VALUES ('4', '67', '0', '14');
INSERT INTO `character_reputation` VALUES ('4', '68', '0', '6');
INSERT INTO `character_reputation` VALUES ('4', '69', '0', '17');
INSERT INTO `character_reputation` VALUES ('4', '70', '0', '2');
INSERT INTO `character_reputation` VALUES ('4', '72', '0', '17');
INSERT INTO `character_reputation` VALUES ('4', '76', '0', '6');
INSERT INTO `character_reputation` VALUES ('4', '81', '0', '6');
INSERT INTO `character_reputation` VALUES ('4', '83', '0', '4');
INSERT INTO `character_reputation` VALUES ('4', '86', '0', '4');
INSERT INTO `character_reputation` VALUES ('4', '87', '0', '2');
INSERT INTO `character_reputation` VALUES ('4', '92', '0', '2');
INSERT INTO `character_reputation` VALUES ('4', '93', '0', '2');
INSERT INTO `character_reputation` VALUES ('4', '169', '0', '8');
INSERT INTO `character_reputation` VALUES ('4', '270', '0', '16');
INSERT INTO `character_reputation` VALUES ('4', '289', '0', '4');
INSERT INTO `character_reputation` VALUES ('4', '349', '0', '0');
INSERT INTO `character_reputation` VALUES ('4', '369', '0', '0');
INSERT INTO `character_reputation` VALUES ('4', '469', '0', '25');
INSERT INTO `character_reputation` VALUES ('4', '470', '0', '0');
INSERT INTO `character_reputation` VALUES ('4', '471', '0', '20');
INSERT INTO `character_reputation` VALUES ('4', '509', '0', '16');
INSERT INTO `character_reputation` VALUES ('4', '510', '0', '2');
INSERT INTO `character_reputation` VALUES ('4', '529', '0', '0');
INSERT INTO `character_reputation` VALUES ('4', '530', '0', '6');
INSERT INTO `character_reputation` VALUES ('4', '549', '0', '4');
INSERT INTO `character_reputation` VALUES ('4', '550', '0', '4');
INSERT INTO `character_reputation` VALUES ('4', '551', '0', '4');
INSERT INTO `character_reputation` VALUES ('4', '569', '0', '4');
INSERT INTO `character_reputation` VALUES ('4', '570', '0', '4');
INSERT INTO `character_reputation` VALUES ('4', '571', '0', '4');
INSERT INTO `character_reputation` VALUES ('4', '574', '0', '4');
INSERT INTO `character_reputation` VALUES ('4', '576', '0', '2');
INSERT INTO `character_reputation` VALUES ('4', '577', '0', '0');
INSERT INTO `character_reputation` VALUES ('4', '589', '0', '0');
INSERT INTO `character_reputation` VALUES ('4', '609', '0', '0');
INSERT INTO `character_reputation` VALUES ('4', '729', '0', '2');
INSERT INTO `character_reputation` VALUES ('4', '730', '0', '16');
INSERT INTO `character_reputation` VALUES ('4', '749', '0', '0');
INSERT INTO `character_reputation` VALUES ('4', '809', '0', '16');
INSERT INTO `character_reputation` VALUES ('4', '889', '0', '6');
INSERT INTO `character_reputation` VALUES ('4', '890', '0', '16');
INSERT INTO `character_reputation` VALUES ('4', '891', '0', '24');
INSERT INTO `character_reputation` VALUES ('4', '892', '0', '14');
INSERT INTO `character_reputation` VALUES ('4', '909', '0', '16');
INSERT INTO `character_reputation` VALUES ('4', '910', '0', '2');
INSERT INTO `character_reputation` VALUES ('4', '911', '0', '6');
INSERT INTO `character_reputation` VALUES ('4', '922', '0', '6');
INSERT INTO `character_reputation` VALUES ('4', '930', '0', '17');
INSERT INTO `character_reputation` VALUES ('4', '932', '0', '80');
INSERT INTO `character_reputation` VALUES ('4', '933', '0', '16');
INSERT INTO `character_reputation` VALUES ('4', '934', '0', '80');
INSERT INTO `character_reputation` VALUES ('4', '935', '0', '16');
INSERT INTO `character_reputation` VALUES ('4', '936', '0', '28');
INSERT INTO `character_reputation` VALUES ('4', '941', '0', '6');
INSERT INTO `character_reputation` VALUES ('4', '942', '0', '16');
INSERT INTO `character_reputation` VALUES ('4', '946', '0', '16');
INSERT INTO `character_reputation` VALUES ('4', '947', '0', '2');
INSERT INTO `character_reputation` VALUES ('4', '967', '0', '16');
INSERT INTO `character_reputation` VALUES ('4', '970', '0', '0');
INSERT INTO `character_reputation` VALUES ('4', '978', '0', '16');
INSERT INTO `character_reputation` VALUES ('4', '980', '0', '8');
INSERT INTO `character_reputation` VALUES ('4', '989', '0', '16');
INSERT INTO `character_reputation` VALUES ('4', '990', '0', '16');
INSERT INTO `character_reputation` VALUES ('4', '1005', '0', '4');
INSERT INTO `character_reputation` VALUES ('4', '1011', '0', '16');
INSERT INTO `character_reputation` VALUES ('4', '1012', '0', '16');
INSERT INTO `character_reputation` VALUES ('4', '1015', '0', '2');
INSERT INTO `character_reputation` VALUES ('4', '1031', '0', '16');
INSERT INTO `character_reputation` VALUES ('4', '1038', '0', '16');
INSERT INTO `character_reputation` VALUES ('4', '1077', '0', '16');

-- ----------------------------
-- Table structure for character_social
-- ----------------------------
DROP TABLE IF EXISTS `character_social`;
CREATE TABLE `character_social` (
  `guid` int(11) unsigned NOT NULL DEFAULT '0',
  `friend` int(11) unsigned NOT NULL DEFAULT '0',
  `flags` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `note` varchar(48) NOT NULL DEFAULT '',
  PRIMARY KEY (`guid`,`friend`,`flags`),
  KEY `guid` (`guid`),
  KEY `friend` (`friend`),
  KEY `guid_flags` (`guid`,`flags`),
  KEY `friend_flags` (`friend`,`flags`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of character_social
-- ----------------------------
INSERT INTO `character_social` VALUES ('3', '4', '1', '');

-- ----------------------------
-- Table structure for character_spell
-- ----------------------------
DROP TABLE IF EXISTS `character_spell`;
CREATE TABLE `character_spell` (
  `guid` int(11) unsigned NOT NULL DEFAULT '0',
  `spell` int(11) unsigned NOT NULL DEFAULT '0',
  `slot` int(11) unsigned NOT NULL DEFAULT '0',
  `active` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `disabled` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`spell`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of character_spell
-- ----------------------------
INSERT INTO `character_spell` VALUES ('2', '71', '137', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '72', '120', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '78', '1', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '81', '2', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '100', '46', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '107', '3', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '196', '4', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '197', '5', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '201', '6', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '203', '7', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '204', '8', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '264', '144', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '284', '50', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '285', '53', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '355', '139', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '522', '9', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '669', '10', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '674', '84', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '676', '123', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '694', '54', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '750', '118', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '772', '47', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '845', '88', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '871', '126', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '1160', '87', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '1161', '93', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '1464', '95', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '1608', '58', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '1671', '127', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '1672', '130', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '1680', '100', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '1715', '49', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '1719', '110', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '2382', '11', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '2457', '12', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '2458', '133', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '2479', '13', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '2565', '122', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '2687', '119', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '3018', '145', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '3050', '14', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '3127', '83', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '3365', '15', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '5242', '86', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '5246', '90', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '5301', '16', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '5308', '92', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '6178', '59', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '6190', '91', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '6192', '89', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '6233', '17', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '6246', '18', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '6247', '19', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '6343', '48', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '6477', '20', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '6478', '21', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '6546', '51', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '6547', '56', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '6548', '63', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '6552', '101', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '6554', '115', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '6572', '121', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '6574', '125', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '6603', '22', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '6673', '85', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '7266', '23', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '7267', '24', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '7355', '25', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '7369', '94', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '7372', '64', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '7373', '76', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '7376', '26', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '7379', '128', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '7381', '27', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '7384', '52', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '7386', '138', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '7400', '60', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '7402', '66', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '7405', '140', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '7887', '61', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '8198', '55', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '8204', '62', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '8205', '67', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '8380', '141', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '8386', '28', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '8737', '29', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '8820', '102', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '9077', '30', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '9078', '31', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '9116', '32', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '9125', '33', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '11549', '96', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '11550', '105', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '11551', '111', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '11554', '99', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '11555', '106', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '11556', '112', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '11564', '65', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '11565', '68', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '11566', '73', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '11567', '77', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '11572', '69', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '11573', '75', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '11574', '82', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '11578', '71', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '11580', '74', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '11581', '79', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '11584', '70', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '11585', '81', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '11596', '142', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '11597', '143', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '11600', '129', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '11601', '131', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '11604', '107', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '11605', '113', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '11608', '103', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '11609', '109', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '12677', '45', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '12678', '124', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '18499', '97', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '20230', '57', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '20252', '134', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '20559', '72', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '20560', '78', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '20569', '117', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '20572', '34', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '20573', '35', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '20574', '36', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '20616', '135', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '20617', '136', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '20658', '98', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '20660', '104', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '20661', '108', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '20662', '114', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '21156', '37', '0', '0');
INSERT INTO `character_spell` VALUES ('2', '21563', '38', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '21651', '39', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '21652', '40', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '22027', '41', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '22810', '42', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '25286', '80', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '25288', '132', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '25289', '116', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '32215', '43', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '41953', '146', '1', '0');
INSERT INTO `character_spell` VALUES ('2', '45927', '44', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '53', '134', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '81', '1', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '201', '50', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '203', '2', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '204', '3', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '264', '87', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '408', '103', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '522', '4', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '668', '5', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '671', '6', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '674', '177', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '703', '92', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '921', '204', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '1180', '7', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '1725', '207', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '1752', '8', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '1757', '136', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '1758', '141', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '1759', '145', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '1760', '149', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '1766', '140', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '1767', '146', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '1768', '155', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '1769', '163', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '1776', '135', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '1777', '143', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '1784', '203', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '1785', '206', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '1786', '214', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '1787', '217', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '1804', '178', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '1833', '99', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '1842', '211', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '1856', '208', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '1857', '215', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '1860', '213', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '1943', '95', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '1966', '142', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '2070', '210', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '2094', '212', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '2098', '9', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '2382', '10', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '2479', '11', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '2567', '12', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '2589', '139', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '2590', '144', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '2591', '147', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '2764', '13', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '2835', '182', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '2836', '209', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '2837', '185', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '2983', '138', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '3018', '88', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '3050', '14', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '3127', '176', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '3365', '15', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '3420', '179', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '3421', '191', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '5171', '90', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '5277', '137', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '5763', '180', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '5938', '174', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '6233', '16', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '6246', '17', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '6247', '18', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '6477', '19', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '6478', '20', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '6603', '21', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '6760', '89', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '6761', '93', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '6762', '97', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '6768', '148', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '6770', '205', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '6774', '111', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '7266', '22', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '7267', '23', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '7355', '24', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '8386', '25', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '8621', '153', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '8623', '104', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '8624', '109', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '8629', '150', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '8631', '96', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '8632', '102', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '8633', '108', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '8637', '154', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '8639', '101', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '8640', '107', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '8643', '117', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '8647', '91', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '8649', '100', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '8650', '106', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '8676', '94', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '8687', '181', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '8691', '184', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '8694', '186', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '8696', '151', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '8721', '152', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '8724', '98', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '8725', '105', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '9077', '26', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '9078', '27', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '9125', '28', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '11197', '113', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '11198', '121', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '11267', '110', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '11268', '116', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '11269', '122', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '11273', '112', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '11274', '118', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '11275', '124', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '11279', '156', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '11280', '160', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '11281', '165', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '11285', '157', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '11286', '168', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '11289', '114', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '11290', '119', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '11293', '158', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '11294', '162', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '11297', '216', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '11299', '115', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '11300', '120', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '11303', '161', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '11305', '164', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '11341', '188', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '11342', '192', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '11343', '197', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '11357', '189', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '11358', '194', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '11400', '193', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '13220', '183', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '13228', '187', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '13229', '190', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '13230', '195', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '13750', '61', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '13791', '70', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '13845', '71', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '13852', '73', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '13863', '69', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '13867', '78', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '13877', '57', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '13958', '86', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '13964', '74', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '14137', '85', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '14142', '82', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '14159', '83', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '14161', '84', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '14164', '81', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '14167', '72', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '14251', '68', '1', '1');
INSERT INTO `character_spell` VALUES ('3', '16092', '29', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '18429', '76', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '20580', '30', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '20582', '31', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '20583', '32', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '20585', '33', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '21009', '34', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '21184', '35', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '21651', '36', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '21652', '37', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '22027', '38', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '22810', '39', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '22988', '224', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '25229', '41', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '25230', '46', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '25255', '42', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '25300', '166', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '25302', '167', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '25347', '196', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '25493', '43', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '26669', '159', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '26679', '127', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '26786', '200', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '26839', '125', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '26861', '169', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '26862', '175', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '26863', '172', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '26865', '128', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '26866', '130', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '26867', '131', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '26884', '133', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '26889', '218', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '26892', '201', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '26925', '44', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '26969', '198', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '27282', '202', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '27283', '199', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '27441', '129', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '27448', '170', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '28894', '47', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '28895', '48', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '28897', '49', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '30920', '75', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '31016', '123', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '31123', '77', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '31224', '219', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '32259', '45', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '32601', '80', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '32645', '126', '0', '0');
INSERT INTO `character_spell` VALUES ('3', '32684', '132', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '33388', '220', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '33391', '221', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '34090', '222', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '34091', '223', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '35553', '79', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '38764', '171', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '38768', '173', '1', '0');
INSERT INTO `character_spell` VALUES ('3', '45927', '40', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '5', '37', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10', '43', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '11', '44', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '66', '45', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '81', '1', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '116', '46', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '118', '47', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '120', '48', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '122', '49', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '130', '50', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '133', '2', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '143', '51', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '145', '52', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '168', '3', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '202', '40', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '203', '4', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '204', '5', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '205', '53', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '227', '6', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '475', '54', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '491', '55', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '522', '7', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '543', '56', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '587', '57', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '597', '58', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '604', '59', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '668', '8', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '700', '60', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '759', '61', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '837', '62', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '857', '63', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '865', '64', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '990', '65', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '1008', '66', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '1090', '67', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '1302', '38', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '1449', '68', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '1459', '69', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '1460', '70', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '1461', '71', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '1463', '72', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '1953', '73', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '2120', '74', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '2121', '75', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '2136', '76', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '2137', '77', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '2138', '78', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '2139', '79', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '2382', '9', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '2479', '10', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '2855', '80', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '2948', '81', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '3050', '11', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '3140', '82', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '3365', '12', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '3552', '83', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '3561', '84', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '3562', '85', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '3565', '86', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '5009', '13', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '5019', '14', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '5143', '87', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '5144', '88', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '5145', '89', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '5405', '90', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '5504', '91', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '5505', '92', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '5506', '93', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '6085', '728', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '6117', '94', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '6127', '95', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '6129', '96', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '6131', '97', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '6136', '98', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '6141', '99', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '6143', '100', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '6233', '15', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '6246', '16', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '6247', '17', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '6477', '18', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '6478', '19', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '6603', '20', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '7266', '21', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '7267', '22', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '7268', '101', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '7269', '102', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '7270', '103', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '7300', '104', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '7301', '105', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '7302', '106', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '7320', '107', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '7321', '108', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '7322', '109', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '7355', '23', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '8386', '24', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '8400', '110', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '8401', '111', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '8402', '112', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '8406', '113', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '8407', '114', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '8408', '115', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '8412', '116', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '8413', '117', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '8416', '118', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '8417', '119', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '8418', '121', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '8419', '120', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '8422', '122', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '8423', '123', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '8427', '124', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '8437', '125', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '8438', '126', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '8439', '127', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '8444', '128', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '8445', '129', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '8446', '130', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '8450', '131', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '8451', '132', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '8455', '133', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '8457', '134', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '8458', '135', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '8461', '136', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '8462', '137', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '8492', '138', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '8494', '139', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '8495', '140', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '9078', '25', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '9125', '26', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '9454', '39', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10032', '41', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10052', '141', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10053', '142', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10054', '143', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10057', '144', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10058', '145', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10059', '146', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10138', '147', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10139', '148', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10140', '149', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10144', '150', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10145', '151', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10148', '152', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10149', '153', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10150', '154', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10151', '155', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10156', '156', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10157', '157', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10159', '158', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10160', '159', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10161', '160', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10165', '161', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10166', '162', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10169', '163', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10170', '164', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10173', '165', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10174', '166', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10177', '167', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10179', '168', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10180', '169', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10181', '170', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10185', '171', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10186', '172', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10187', '173', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10191', '174', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10192', '175', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10193', '176', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10197', '177', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10199', '178', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10201', '179', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10202', '180', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10205', '181', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10206', '182', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10207', '183', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10211', '184', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10212', '185', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10215', '186', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10216', '187', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10219', '188', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10220', '189', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10223', '190', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10225', '191', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10230', '192', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10273', '193', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '10274', '194', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '11113', '693', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '11129', '703', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '11366', '681', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '11368', '700', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '11416', '195', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '11419', '196', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '11426', '716', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '11958', '722', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12042', '735', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12043', '734', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12051', '197', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12341', '678', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12342', '679', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12350', '692', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12351', '675', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12353', '680', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12354', '198', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12360', '691', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12400', '702', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12469', '730', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12472', '714', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12475', '707', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12484', '199', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12485', '200', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12486', '201', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12488', '708', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12490', '709', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12497', '705', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12503', '727', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12505', '682', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12519', '711', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12522', '683', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12523', '684', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12524', '685', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12525', '686', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12526', '687', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12529', '202', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12531', '203', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12536', '204', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12544', '205', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12556', '206', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12557', '207', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12571', '710', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12577', '725', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12579', '208', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12592', '724', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12598', '736', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12605', '732', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12606', '731', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12611', '209', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12654', '210', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12824', '211', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12825', '212', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12826', '213', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12842', '726', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12848', '701', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12873', '677', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12953', '706', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12954', '214', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12957', '215', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '12985', '712', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '13018', '694', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '13019', '695', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '13020', '696', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '13021', '697', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '13031', '717', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '13032', '718', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '13033', '719', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '13043', '676', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '13326', '216', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '13339', '217', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '13340', '218', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '13341', '219', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '13342', '220', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '13374', '221', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '13438', '222', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '13439', '223', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '13441', '224', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '13744', '225', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '13878', '226', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '13912', '227', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '14105', '228', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '14145', '229', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '14514', '230', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '15041', '231', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '15044', '232', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '15053', '723', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '15060', '737', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '15091', '233', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '15122', '234', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '15241', '235', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '15244', '236', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '15288', '237', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '15574', '238', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '15662', '239', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '15735', '240', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '15736', '241', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '15744', '242', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '15784', '243', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '15790', '244', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '15791', '245', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '15849', '246', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '15850', '247', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '16046', '248', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '16067', '249', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '16070', '250', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '16144', '251', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '16327', '252', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '16380', '253', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '16407', '254', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '16412', '255', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '16413', '256', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '16415', '257', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '16454', '258', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '16551', '259', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '16758', '738', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '16766', '704', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '16770', '729', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '16785', '260', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '16876', '261', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '16908', '262', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '17010', '263', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '17011', '264', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '17145', '265', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '17150', '266', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '17195', '267', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '17273', '268', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '17274', '269', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '17276', '270', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '17277', '271', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '17492', '272', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '17740', '273', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '17741', '274', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '18082', '275', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '18083', '276', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '18085', '277', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '18091', '278', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '18092', '279', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '18100', '280', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '18101', '281', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '18105', '282', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '18108', '283', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '18146', '284', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '18189', '285', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '18199', '286', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '18204', '287', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '18276', '288', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '18398', '289', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '18460', '739', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '18464', '740', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '18796', '290', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '18809', '688', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '18833', '291', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '18948', '292', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '18949', '293', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '19260', '294', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '19715', '295', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '20228', '296', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '20229', '297', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '20537', '298', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '20597', '27', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '20598', '28', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '20599', '29', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '20600', '30', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '20623', '299', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '20679', '300', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '20788', '301', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '20795', '302', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '20828', '303', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '20832', '304', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '20864', '31', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '20869', '305', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '20883', '306', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '21127', '307', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '21159', '308', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '21162', '309', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '21651', '32', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '21652', '33', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '21655', '310', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '22027', '34', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '22272', '311', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '22273', '312', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '22424', '313', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '22460', '314', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '22746', '315', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '22782', '316', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '22783', '317', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '22810', '35', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '23028', '318', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '23037', '319', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '23039', '320', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '23113', '321', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '23267', '322', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '23331', '323', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '23603', '324', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '24006', '325', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '24530', '326', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '25022', '327', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '25023', '328', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '25028', '329', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '25049', '330', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '25304', '331', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '25306', '332', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '25345', '333', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '25346', '334', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '25641', '335', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '26373', '336', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '26406', '337', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '26408', '338', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '26409', '339', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '26410', '340', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '26412', '341', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '26414', '342', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '26448', '343', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '26450', '344', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '26451', '345', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '26452', '346', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '26453', '347', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '26454', '348', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '26455', '349', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '26456', '350', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '26470', '351', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '27070', '352', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '27071', '353', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '27072', '354', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '27073', '355', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '27074', '356', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '27075', '357', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '27076', '358', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '27078', '359', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '27079', '360', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '27080', '361', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '27082', '362', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '27085', '363', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '27086', '364', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '27087', '365', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '27088', '366', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '27090', '368', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '27101', '369', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '27103', '370', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '27124', '371', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '27125', '372', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '27126', '373', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '27127', '374', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '27128', '375', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '27130', '376', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '27131', '377', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '27132', '689', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '27133', '698', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '27134', '720', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '27204', '378', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '27514', '379', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '27618', '380', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '27619', '381', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '28148', '382', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '28271', '383', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '28272', '384', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '28323', '385', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '28332', '715', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '28391', '386', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '28401', '387', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '28574', '733', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '28595', '713', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '28609', '388', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '28612', '389', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '28682', '390', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '28734', '391', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '28762', '392', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '28772', '393', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '28863', '394', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '29076', '741', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '29124', '395', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '29164', '396', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '29208', '397', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '29209', '398', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '29210', '399', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '29211', '400', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '29440', '742', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '29443', '401', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '29447', '743', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '29515', '402', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '29525', '403', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '29607', '404', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '29717', '405', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '29718', '406', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '29815', '407', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '29880', '408', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '29883', '409', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '29884', '410', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '29955', '411', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '29956', '412', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '29960', '413', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '29961', '414', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '29966', '415', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '29967', '416', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '29968', '417', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '29975', '418', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '29976', '419', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '29977', '420', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '30092', '421', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '30095', '422', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '30254', '423', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '30449', '424', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '30451', '425', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '30455', '426', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '30482', '427', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '30496', '428', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '30512', '429', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '30516', '430', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '30539', '431', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '30553', '432', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '30554', '433', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '30600', '434', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '30610', '435', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '30628', '436', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '30744', '437', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '30849', '438', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '30935', '439', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '30972', '440', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '30973', '441', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '31256', '442', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '31257', '443', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '31263', '444', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '31378', '445', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '31439', '446', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '31465', '447', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '31570', '744', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '31573', '745', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '31575', '746', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '31583', '747', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '31588', '748', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '31589', '749', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '31596', '448', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '31640', '750', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '31642', '751', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '31643', '449', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '31660', '754', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '31661', '755', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '31669', '759', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '31672', '760', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '31678', '761', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '31680', '752', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '31686', '762', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '31687', '763', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '31742', '450', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '31743', '451', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '31751', '452', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '31999', '453', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '32196', '454', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '32200', '455', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '32224', '456', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '32266', '457', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '32271', '458', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '32366', '459', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '32557', '460', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '32602', '461', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '32603', '462', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '32612', '463', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '32675', '464', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '32677', '465', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '32689', '466', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '32691', '467', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '32796', '468', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '32811', '469', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '32826', '470', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '32920', '471', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '32932', '472', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '32937', '473', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '32990', '474', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '32991', '475', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '32992', '476', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '32993', '477', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '33031', '478', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '33041', '756', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '33042', '757', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '33043', '758', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '33395', '479', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '33405', '721', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '33419', '480', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '33421', '481', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '33462', '482', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '33483', '483', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '33546', '484', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '33548', '485', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '33549', '486', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '33550', '487', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '33552', '488', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '33553', '489', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '33690', '490', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '33691', '491', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '33713', '492', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '33714', '493', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '33717', '494', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '33770', '495', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '33832', '496', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '33833', '497', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '33932', '498', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '33933', '699', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '33938', '690', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '33944', '499', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '33946', '500', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '33975', '501', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '33988', '502', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '33989', '503', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '34165', '504', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '34296', '753', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '34325', '505', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '34349', '506', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '34426', '507', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '34446', '508', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '34447', '509', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '34605', '510', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '34679', '511', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '34714', '512', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '34715', '513', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '34722', '514', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '34752', '515', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '34809', '516', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '34844', '517', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '34880', '518', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '34913', '519', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '34973', '520', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '35012', '521', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '35033', '522', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '35034', '523', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '35039', '524', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '35064', '525', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '35071', '526', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '35072', '527', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '35188', '528', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '35206', '529', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '35250', '530', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '35267', '531', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '35377', '532', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '35581', '764', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '35696', '533', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '35743', '534', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '35744', '535', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '35915', '536', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '35916', '537', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '35917', '538', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '35927', '539', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '35999', '42', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '36021', '540', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '36032', '541', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '36097', '542', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '36109', '543', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '36111', '544', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '36119', '545', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '36147', '546', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '36277', '547', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '36278', '548', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '36339', '549', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '36412', '550', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '36463', '551', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '36506', '552', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '36527', '553', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '36535', '554', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '36544', '555', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '36574', '556', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '36608', '557', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '36718', '558', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '36807', '559', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '36813', '560', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '36879', '561', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '36880', '562', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '36881', '563', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '36883', '564', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '36891', '565', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '36894', '566', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '36908', '567', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '36911', '568', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '36994', '569', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '37014', '570', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '37063', '571', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '37110', '572', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '37265', '573', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '37289', '574', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '37420', '367', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '37470', '575', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '37778', '576', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '37844', '577', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '37921', '578', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '37988', '579', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '37993', '580', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '38064', '581', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '38147', '582', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '38148', '583', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '38151', '584', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '38194', '585', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '38203', '586', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '38263', '587', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '38264', '588', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '38274', '589', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '38275', '590', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '38276', '591', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '38384', '592', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '38391', '593', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '38526', '594', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '38535', '595', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '38536', '596', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '38636', '597', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '38642', '598', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '38643', '599', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '38644', '600', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '38647', '601', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '38648', '602', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '38649', '603', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '38692', '604', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '38697', '605', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '38699', '606', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '38700', '607', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '38703', '608', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '38704', '609', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '38712', '610', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '38808', '611', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '38849', '612', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '38881', '613', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '38930', '614', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '38932', '615', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '38941', '616', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '38967', '617', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '38981', '618', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '39001', '619', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '39038', '620', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '39076', '621', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '39208', '622', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '39235', '623', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '39414', '624', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '39576', '625', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '39893', '626', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '39894', '627', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '39965', '628', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '40432', '629', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '40525', '630', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '40590', '631', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '40822', '632', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '40949', '633', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '41234', '634', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '41253', '635', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '41478', '636', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '42198', '643', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '42208', '637', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '42209', '638', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '42210', '639', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '42211', '640', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '42212', '641', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '42213', '642', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '43245', '644', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '43515', '645', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '43520', '646', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '43525', '647', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '43987', '648', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '43988', '649', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '43993', '650', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '44080', '651', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '44082', '652', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '44178', '653', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '44457', '654', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '44459', '655', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '44461', '656', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '45214', '657', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '45438', '658', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '45461', '659', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '45927', '36', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '46149', '660', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '46151', '661', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '46195', '662', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '46224', '663', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '46580', '664', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '46604', '665', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '46706', '666', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '46744', '667', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '46745', '668', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '46984', '669', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '46989', '670', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '47000', '671', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '49359', '672', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '49360', '673', '1', '0');
INSERT INTO `character_spell` VALUES ('4', '49512', '674', '1', '0');

-- ----------------------------
-- Table structure for character_spell_cooldown
-- ----------------------------
DROP TABLE IF EXISTS `character_spell_cooldown`;
CREATE TABLE `character_spell_cooldown` (
  `guid` int(11) unsigned NOT NULL DEFAULT '0',
  `spell` int(11) unsigned NOT NULL DEFAULT '0',
  `item` int(11) unsigned NOT NULL DEFAULT '0',
  `time` bigint(20) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`spell`,`item`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of character_spell_cooldown
-- ----------------------------

-- ----------------------------
-- Table structure for character_stats_ro
-- ----------------------------
DROP TABLE IF EXISTS `character_stats_ro`;
CREATE TABLE `character_stats_ro` (
  `guid` bigint(8) unsigned NOT NULL,
  `honor` bigint(8) unsigned NOT NULL,
  `honorablekills` bigint(8) unsigned NOT NULL,
  `dailyarenawins` smallint(5) unsigned DEFAULT '0',
  PRIMARY KEY (`guid`),
  KEY `honorablekills` (`honorablekills`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of character_stats_ro
-- ----------------------------
INSERT INTO `character_stats_ro` VALUES ('1', '0', '0', '0');
INSERT INTO `character_stats_ro` VALUES ('2', '0', '0', '0');
INSERT INTO `character_stats_ro` VALUES ('3', '0', '0', '0');
INSERT INTO `character_stats_ro` VALUES ('4', '0', '0', '0');
INSERT INTO `character_stats_ro` VALUES ('5', '0', '0', '0');
INSERT INTO `character_stats_ro` VALUES ('6', '0', '0', '0');
INSERT INTO `character_stats_ro` VALUES ('7', '0', '0', '0');
INSERT INTO `character_stats_ro` VALUES ('8', '0', '0', '0');
INSERT INTO `character_stats_ro` VALUES ('14', '0', '0', '0');
INSERT INTO `character_stats_ro` VALUES ('15', '0', '0', '0');
INSERT INTO `character_stats_ro` VALUES ('17', '0', '0', '0');

-- ----------------------------
-- Table structure for character_tutorial
-- ----------------------------
DROP TABLE IF EXISTS `character_tutorial`;
CREATE TABLE `character_tutorial` (
  `account` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `realmid` int(11) unsigned NOT NULL DEFAULT '0',
  `tut0` int(11) unsigned NOT NULL DEFAULT '0',
  `tut1` int(11) unsigned NOT NULL DEFAULT '0',
  `tut2` int(11) unsigned NOT NULL DEFAULT '0',
  `tut3` int(11) unsigned NOT NULL DEFAULT '0',
  `tut4` int(11) unsigned NOT NULL DEFAULT '0',
  `tut5` int(11) unsigned NOT NULL DEFAULT '0',
  `tut6` int(11) unsigned NOT NULL DEFAULT '0',
  `tut7` int(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`account`,`realmid`),
  KEY `account` (`account`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of character_tutorial
-- ----------------------------
INSERT INTO `character_tutorial` VALUES ('1', '1', '4294967295', '4294967295', '4294967295', '4294967295', '4294967295', '4294967295', '4294967295', '4294967295');
INSERT INTO `character_tutorial` VALUES ('2', '1', '4294967295', '4294967295', '4294967295', '4294967295', '4294967295', '4294967295', '4294967295', '4294967295');

-- ----------------------------
-- Table structure for corpse
-- ----------------------------
DROP TABLE IF EXISTS `corpse`;
CREATE TABLE `corpse` (
  `guid` int(11) unsigned NOT NULL DEFAULT '0',
  `player` int(11) unsigned NOT NULL DEFAULT '0',
  `position_x` float NOT NULL DEFAULT '0',
  `position_y` float NOT NULL DEFAULT '0',
  `position_z` float NOT NULL DEFAULT '0',
  `orientation` float NOT NULL DEFAULT '0',
  `zone` int(11) unsigned NOT NULL DEFAULT '38',
  `map` int(11) unsigned NOT NULL DEFAULT '0',
  `data` longtext,
  `time` bigint(20) unsigned NOT NULL DEFAULT '0',
  `corpse_type` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `instance` int(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`),
  KEY `corpse_type` (`corpse_type`),
  KEY `instance` (`instance`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of corpse
-- ----------------------------

-- ----------------------------
-- Table structure for creature_respawn
-- ----------------------------
DROP TABLE IF EXISTS `creature_respawn`;
CREATE TABLE `creature_respawn` (
  `guid` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Global Unique Identifier',
  `respawntime` bigint(20) NOT NULL DEFAULT '0',
  `instance` mediumint(8) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`instance`),
  KEY `instance` (`instance`),
  KEY `guid` (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC;

-- ----------------------------
-- Records of creature_respawn
-- ----------------------------
INSERT INTO `creature_respawn` VALUES ('1673', '1655702604', '0');
INSERT INTO `creature_respawn` VALUES ('1686', '1655702699', '0');
INSERT INTO `creature_respawn` VALUES ('1704', '1655702667', '0');
INSERT INTO `creature_respawn` VALUES ('1957', '1655702752', '0');
INSERT INTO `creature_respawn` VALUES ('2483', '1655702607', '0');
INSERT INTO `creature_respawn` VALUES ('29198', '1655700299', '0');
INSERT INTO `creature_respawn` VALUES ('44180', '1655712539', '0');
INSERT INTO `creature_respawn` VALUES ('44182', '1655712574', '0');
INSERT INTO `creature_respawn` VALUES ('44215', '1655712642', '0');
INSERT INTO `creature_respawn` VALUES ('44216', '1655712569', '0');
INSERT INTO `creature_respawn` VALUES ('44217', '1655712614', '0');
INSERT INTO `creature_respawn` VALUES ('44359', '1671167813', '0');
INSERT INTO `creature_respawn` VALUES ('45132', '1655712705', '0');
INSERT INTO `creature_respawn` VALUES ('45134', '1655712606', '0');
INSERT INTO `creature_respawn` VALUES ('45444', '1655712626', '0');
INSERT INTO `creature_respawn` VALUES ('45932', '1655712611', '0');
INSERT INTO `creature_respawn` VALUES ('45934', '1655712638', '0');
INSERT INTO `creature_respawn` VALUES ('45936', '1655712562', '0');
INSERT INTO `creature_respawn` VALUES ('46349', '1655712657', '0');
INSERT INTO `creature_respawn` VALUES ('46354', '1655712603', '0');
INSERT INTO `creature_respawn` VALUES ('46381', '1655712584', '0');
INSERT INTO `creature_respawn` VALUES ('46917', '1655712528', '0');
INSERT INTO `creature_respawn` VALUES ('47137', '1655712503', '0');
INSERT INTO `creature_respawn` VALUES ('49282', '1655709046', '0');
INSERT INTO `creature_respawn` VALUES ('51094', '1655712578', '0');
INSERT INTO `creature_respawn` VALUES ('78132', '1655702685', '0');
INSERT INTO `creature_respawn` VALUES ('88730', '1671002537', '0');
INSERT INTO `creature_respawn` VALUES ('88734', '1671002537', '0');
INSERT INTO `creature_respawn` VALUES ('97271', '1655759723', '0');
INSERT INTO `creature_respawn` VALUES ('97272', '1655759762', '0');

-- ----------------------------
-- Table structure for custom_transmogrification
-- ----------------------------
DROP TABLE IF EXISTS `custom_transmogrification`;
CREATE TABLE `custom_transmogrification` (
  `GUID` int(10) unsigned NOT NULL COMMENT 'Item guidLow',
  `FakeEntry` int(10) unsigned NOT NULL COMMENT 'Item entry',
  `Owner` int(10) unsigned NOT NULL COMMENT 'Player guidLow',
  PRIMARY KEY (`GUID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='version 4.0';

-- ----------------------------
-- Records of custom_transmogrification
-- ----------------------------

-- ----------------------------
-- Table structure for deleted_chars
-- ----------------------------
DROP TABLE IF EXISTS `deleted_chars`;
CREATE TABLE `deleted_chars` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `char_guid` int(10) unsigned NOT NULL,
  `oldname` varchar(15) NOT NULL,
  `acc` int(11) NOT NULL,
  `date` datetime NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of deleted_chars
-- ----------------------------

-- ----------------------------
-- Table structure for gameobject_respawn
-- ----------------------------
DROP TABLE IF EXISTS `gameobject_respawn`;
CREATE TABLE `gameobject_respawn` (
  `guid` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Global Unique Identifier',
  `respawntime` bigint(20) NOT NULL DEFAULT '0',
  `instance` mediumint(8) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`instance`),
  KEY `instance` (`instance`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC;

-- ----------------------------
-- Records of gameobject_respawn
-- ----------------------------

-- ----------------------------
-- Table structure for game_event_condition_save
-- ----------------------------
DROP TABLE IF EXISTS `game_event_condition_save`;
CREATE TABLE `game_event_condition_save` (
  `event_id` mediumint(8) unsigned NOT NULL,
  `condition_id` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `done` float DEFAULT '0',
  PRIMARY KEY (`event_id`,`condition_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of game_event_condition_save
-- ----------------------------

-- ----------------------------
-- Table structure for game_event_save
-- ----------------------------
DROP TABLE IF EXISTS `game_event_save`;
CREATE TABLE `game_event_save` (
  `event_id` mediumint(8) unsigned NOT NULL,
  `state` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `next_start` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  PRIMARY KEY (`event_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of game_event_save
-- ----------------------------

-- ----------------------------
-- Table structure for gm_tickets
-- ----------------------------
DROP TABLE IF EXISTS `gm_tickets`;
CREATE TABLE `gm_tickets` (
  `guid` int(10) NOT NULL AUTO_INCREMENT,
  `playerGuid` int(11) unsigned NOT NULL DEFAULT '0',
  `name` varchar(15) NOT NULL,
  `message` text NOT NULL,
  `createtime` int(10) NOT NULL,
  `map` int(11) NOT NULL DEFAULT '0',
  `posX` float NOT NULL DEFAULT '0',
  `posY` float NOT NULL DEFAULT '0',
  `posZ` float NOT NULL DEFAULT '0',
  `timestamp` int(10) NOT NULL DEFAULT '0',
  `assignedto` int(10) NOT NULL DEFAULT '0',
  `closed` int(10) NOT NULL DEFAULT '0',
  `comment` text NOT NULL,
  `response` text NOT NULL,
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of gm_tickets
-- ----------------------------

-- ----------------------------
-- Table structure for groups
-- ----------------------------
DROP TABLE IF EXISTS `groups`;
CREATE TABLE `groups` (
  `leaderGuid` int(11) unsigned NOT NULL,
  `mainTank` int(11) unsigned NOT NULL,
  `mainAssistant` int(11) unsigned NOT NULL,
  `lootMethod` tinyint(4) unsigned NOT NULL,
  `looterGuid` int(11) unsigned NOT NULL,
  `lootThreshold` tinyint(4) unsigned NOT NULL,
  `icon1` int(11) unsigned NOT NULL,
  `icon2` int(11) unsigned NOT NULL,
  `icon3` int(11) unsigned NOT NULL,
  `icon4` int(11) unsigned NOT NULL,
  `icon5` int(11) unsigned NOT NULL,
  `icon6` int(11) unsigned NOT NULL,
  `icon7` int(11) unsigned NOT NULL,
  `icon8` int(11) unsigned NOT NULL,
  `isRaid` tinyint(1) unsigned NOT NULL,
  `difficulty` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`leaderGuid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of groups
-- ----------------------------

-- ----------------------------
-- Table structure for group_instance
-- ----------------------------
DROP TABLE IF EXISTS `group_instance`;
CREATE TABLE `group_instance` (
  `leaderGuid` int(11) unsigned NOT NULL DEFAULT '0',
  `instance` int(11) unsigned NOT NULL DEFAULT '0',
  `permanent` tinyint(1) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`leaderGuid`,`instance`),
  KEY `instance` (`instance`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of group_instance
-- ----------------------------

-- ----------------------------
-- Table structure for group_member
-- ----------------------------
DROP TABLE IF EXISTS `group_member`;
CREATE TABLE `group_member` (
  `leaderGuid` int(11) unsigned NOT NULL,
  `memberGuid` int(11) unsigned NOT NULL,
  `assistant` tinyint(1) unsigned NOT NULL,
  `subgroup` smallint(6) unsigned NOT NULL,
  PRIMARY KEY (`leaderGuid`,`memberGuid`),
  KEY `memberGuid` (`memberGuid`),
  KEY `leaderGuid` (`leaderGuid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of group_member
-- ----------------------------

-- ----------------------------
-- Table structure for group_saved_loot
-- ----------------------------
DROP TABLE IF EXISTS `group_saved_loot`;
CREATE TABLE `group_saved_loot` (
  `creatureId` int(11) NOT NULL DEFAULT '0',
  `instanceId` int(11) NOT NULL DEFAULT '0',
  `itemId` int(11) NOT NULL DEFAULT '0',
  `itemCount` int(11) DEFAULT NULL,
  `summoned` tinyint(1) DEFAULT '0',
  `position_x` float DEFAULT NULL,
  `position_y` float DEFAULT NULL,
  `position_z` float DEFAULT NULL,
  `playerGuids` varchar(1024) DEFAULT NULL,
  PRIMARY KEY (`creatureId`,`instanceId`,`itemId`),
  KEY `summoned` (`summoned`),
  KEY `creatureId` (`creatureId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of group_saved_loot
-- ----------------------------

-- ----------------------------
-- Table structure for guild
-- ----------------------------
DROP TABLE IF EXISTS `guild`;
CREATE TABLE `guild` (
  `guildid` int(6) unsigned NOT NULL DEFAULT '0',
  `name` varchar(255) NOT NULL DEFAULT '',
  `leaderguid` int(6) unsigned NOT NULL DEFAULT '0',
  `EmblemStyle` int(5) NOT NULL DEFAULT '0',
  `EmblemColor` int(5) NOT NULL DEFAULT '0',
  `BorderStyle` int(5) NOT NULL DEFAULT '0',
  `BorderColor` int(5) NOT NULL DEFAULT '0',
  `BackgroundColor` int(5) NOT NULL DEFAULT '0',
  `info` text NOT NULL,
  `motd` varchar(255) NOT NULL DEFAULT '',
  `createdate` datetime DEFAULT NULL,
  `BankMoney` bigint(20) NOT NULL DEFAULT '0',
  `flags` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guildid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of guild
-- ----------------------------

-- ----------------------------
-- Table structure for guild_announce_cooldown
-- ----------------------------
DROP TABLE IF EXISTS `guild_announce_cooldown`;
CREATE TABLE `guild_announce_cooldown` (
  `guild_id` int(10) unsigned NOT NULL,
  `cooldown_end` bigint(20) unsigned NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of guild_announce_cooldown
-- ----------------------------

-- ----------------------------
-- Table structure for guild_bank_eventlog
-- ----------------------------
DROP TABLE IF EXISTS `guild_bank_eventlog`;
CREATE TABLE `guild_bank_eventlog` (
  `guildid` int(11) unsigned NOT NULL DEFAULT '0',
  `LogGuid` int(11) unsigned NOT NULL DEFAULT '0',
  `LogEntry` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `TabId` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `PlayerGuid` int(11) unsigned NOT NULL DEFAULT '0',
  `ItemOrMoney` int(11) unsigned NOT NULL DEFAULT '0',
  `ItemStackCount` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `DestTabId` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `TimeStamp` bigint(20) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guildid`,`LogGuid`),
  KEY `PlayerGuid` (`PlayerGuid`),
  KEY `LogGuid` (`LogGuid`),
  KEY `TimeStamp` (`TimeStamp`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of guild_bank_eventlog
-- ----------------------------

-- ----------------------------
-- Table structure for guild_bank_item
-- ----------------------------
DROP TABLE IF EXISTS `guild_bank_item`;
CREATE TABLE `guild_bank_item` (
  `guildid` int(11) unsigned NOT NULL DEFAULT '0',
  `TabId` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `SlotId` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `item_guid` int(11) unsigned NOT NULL DEFAULT '0',
  `item_entry` int(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guildid`,`TabId`,`SlotId`),
  KEY `guildid` (`guildid`),
  KEY `item_guid` (`item_guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of guild_bank_item
-- ----------------------------

-- ----------------------------
-- Table structure for guild_bank_right
-- ----------------------------
DROP TABLE IF EXISTS `guild_bank_right`;
CREATE TABLE `guild_bank_right` (
  `guildid` int(11) unsigned NOT NULL DEFAULT '0',
  `TabId` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `rid` int(11) unsigned NOT NULL DEFAULT '0',
  `gbright` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `SlotPerDay` int(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guildid`,`TabId`,`rid`),
  KEY `guildid` (`guildid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of guild_bank_right
-- ----------------------------

-- ----------------------------
-- Table structure for guild_bank_tab
-- ----------------------------
DROP TABLE IF EXISTS `guild_bank_tab`;
CREATE TABLE `guild_bank_tab` (
  `guildid` int(11) unsigned NOT NULL DEFAULT '0',
  `TabId` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `TabName` varchar(100) NOT NULL DEFAULT '',
  `TabIcon` varchar(100) NOT NULL DEFAULT '',
  `TabText` text,
  PRIMARY KEY (`guildid`,`TabId`),
  KEY `guildid` (`guildid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of guild_bank_tab
-- ----------------------------

-- ----------------------------
-- Table structure for guild_eventlog
-- ----------------------------
DROP TABLE IF EXISTS `guild_eventlog`;
CREATE TABLE `guild_eventlog` (
  `guildid` int(11) NOT NULL,
  `LogGuid` int(11) NOT NULL,
  `EventType` tinyint(1) NOT NULL,
  `PlayerGuid1` int(11) NOT NULL,
  `PlayerGuid2` int(11) NOT NULL,
  `NewRank` tinyint(2) NOT NULL,
  `TimeStamp` bigint(20) NOT NULL,
  PRIMARY KEY (`guildid`,`LogGuid`),
  KEY `PlayerGuid1` (`PlayerGuid1`),
  KEY `PlayerGuid2` (`PlayerGuid2`),
  KEY `LogGuid` (`LogGuid`),
  KEY `TimeStamp` (`TimeStamp`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of guild_eventlog
-- ----------------------------

-- ----------------------------
-- Table structure for guild_member
-- ----------------------------
DROP TABLE IF EXISTS `guild_member`;
CREATE TABLE `guild_member` (
  `guildid` int(6) unsigned NOT NULL DEFAULT '0',
  `guid` int(11) unsigned NOT NULL DEFAULT '0',
  `rank` tinyint(2) unsigned NOT NULL DEFAULT '0',
  `pnote` varchar(255) NOT NULL DEFAULT '',
  `offnote` varchar(255) NOT NULL DEFAULT '',
  `BankResetTimeMoney` int(11) unsigned NOT NULL DEFAULT '0',
  `BankRemMoney` int(11) unsigned NOT NULL DEFAULT '0',
  `BankResetTimeTab0` int(11) unsigned NOT NULL DEFAULT '0',
  `BankRemSlotsTab0` int(11) unsigned NOT NULL DEFAULT '0',
  `BankResetTimeTab1` int(11) unsigned NOT NULL DEFAULT '0',
  `BankRemSlotsTab1` int(11) unsigned NOT NULL DEFAULT '0',
  `BankResetTimeTab2` int(11) unsigned NOT NULL DEFAULT '0',
  `BankRemSlotsTab2` int(11) unsigned NOT NULL DEFAULT '0',
  `BankResetTimeTab3` int(11) unsigned NOT NULL DEFAULT '0',
  `BankRemSlotsTab3` int(11) unsigned NOT NULL DEFAULT '0',
  `BankResetTimeTab4` int(11) unsigned NOT NULL DEFAULT '0',
  `BankRemSlotsTab4` int(11) unsigned NOT NULL DEFAULT '0',
  `BankResetTimeTab5` int(11) unsigned NOT NULL DEFAULT '0',
  `BankRemSlotsTab5` int(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`),
  KEY `guildid` (`guildid`),
  KEY `guildid_rank` (`guildid`,`rank`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of guild_member
-- ----------------------------

-- ----------------------------
-- Table structure for guild_rank
-- ----------------------------
DROP TABLE IF EXISTS `guild_rank`;
CREATE TABLE `guild_rank` (
  `guildid` int(6) unsigned NOT NULL DEFAULT '0',
  `rid` int(11) unsigned NOT NULL,
  `rname` varchar(255) NOT NULL DEFAULT '',
  `rights` int(3) unsigned NOT NULL DEFAULT '0',
  `BankMoneyPerDay` int(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guildid`,`rid`),
  KEY `rid` (`rid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of guild_rank
-- ----------------------------

-- ----------------------------
-- Table structure for hidden_rating
-- ----------------------------
DROP TABLE IF EXISTS `hidden_rating`;
CREATE TABLE `hidden_rating` (
  `guid` int(11) unsigned NOT NULL,
  `rating2` int(10) unsigned NOT NULL DEFAULT '1500',
  `rating3` int(10) unsigned NOT NULL DEFAULT '1500',
  `rating5` int(10) unsigned NOT NULL DEFAULT '1500',
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of hidden_rating
-- ----------------------------

-- ----------------------------
-- Table structure for instance
-- ----------------------------
DROP TABLE IF EXISTS `instance`;
CREATE TABLE `instance` (
  `id` int(11) unsigned NOT NULL DEFAULT '0',
  `map` int(11) unsigned NOT NULL DEFAULT '0',
  `resettime` bigint(40) NOT NULL DEFAULT '0',
  `difficulty` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `data` longtext,
  PRIMARY KEY (`id`),
  KEY `map` (`map`),
  KEY `resettime` (`resettime`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of instance
-- ----------------------------

-- ----------------------------
-- Table structure for instance_reset
-- ----------------------------
DROP TABLE IF EXISTS `instance_reset`;
CREATE TABLE `instance_reset` (
  `mapid` int(11) unsigned NOT NULL DEFAULT '0',
  `resettime` bigint(40) NOT NULL DEFAULT '0',
  PRIMARY KEY (`mapid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of instance_reset
-- ----------------------------
INSERT INTO `instance_reset` VALUES ('169', '1671249600');
INSERT INTO `instance_reset` VALUES ('249', '1671595200');
INSERT INTO `instance_reset` VALUES ('269', '1671249600');
INSERT INTO `instance_reset` VALUES ('309', '1671422400');
INSERT INTO `instance_reset` VALUES ('409', '1671336000');
INSERT INTO `instance_reset` VALUES ('469', '1671336000');
INSERT INTO `instance_reset` VALUES ('509', '1671422400');
INSERT INTO `instance_reset` VALUES ('531', '1671336000');
INSERT INTO `instance_reset` VALUES ('532', '1671336000');
INSERT INTO `instance_reset` VALUES ('533', '1671336000');
INSERT INTO `instance_reset` VALUES ('534', '1671336000');
INSERT INTO `instance_reset` VALUES ('540', '1671249600');
INSERT INTO `instance_reset` VALUES ('542', '1671249600');
INSERT INTO `instance_reset` VALUES ('543', '1671249600');
INSERT INTO `instance_reset` VALUES ('544', '1671336000');
INSERT INTO `instance_reset` VALUES ('545', '1671249600');
INSERT INTO `instance_reset` VALUES ('546', '1671249600');
INSERT INTO `instance_reset` VALUES ('547', '1671249600');
INSERT INTO `instance_reset` VALUES ('548', '1671336000');
INSERT INTO `instance_reset` VALUES ('550', '1671336000');
INSERT INTO `instance_reset` VALUES ('552', '1671249600');
INSERT INTO `instance_reset` VALUES ('553', '1671249600');
INSERT INTO `instance_reset` VALUES ('554', '1671249600');
INSERT INTO `instance_reset` VALUES ('555', '1671249600');
INSERT INTO `instance_reset` VALUES ('556', '1671249600');
INSERT INTO `instance_reset` VALUES ('557', '1671249600');
INSERT INTO `instance_reset` VALUES ('558', '1671249600');
INSERT INTO `instance_reset` VALUES ('560', '1671249600');
INSERT INTO `instance_reset` VALUES ('564', '1671336000');
INSERT INTO `instance_reset` VALUES ('565', '1671336000');
INSERT INTO `instance_reset` VALUES ('568', '1671422400');
INSERT INTO `instance_reset` VALUES ('580', '1671336000');
INSERT INTO `instance_reset` VALUES ('585', '1671249600');

-- ----------------------------
-- Table structure for item_instance
-- ----------------------------
DROP TABLE IF EXISTS `item_instance`;
CREATE TABLE `item_instance` (
  `guid` int(11) unsigned NOT NULL DEFAULT '0',
  `owner_guid` int(11) unsigned NOT NULL DEFAULT '0',
  `data` longtext,
  PRIMARY KEY (`guid`),
  KEY `owner_guid` (`owner_guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of item_instance
-- ----------------------------
INSERT INTO `item_instance` VALUES ('2', '2', '2 1073741824 3 6125 1065353216 0 2 0 2 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ');
INSERT INTO `item_instance` VALUES ('8', '2', '8 1073741824 3 6948 1065353216 0 2 0 2 0 0 0 0 0 1 0 0 0 0 0 0 65 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ');
INSERT INTO `item_instance` VALUES ('16', '3', '16 1073741824 3 49 1065353216 0 3 0 3 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ');
INSERT INTO `item_instance` VALUES ('26', '3', '26 1073741824 3 6948 1065353216 0 3 0 3 0 0 0 0 0 1 0 0 0 0 0 0 65 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ');
INSERT INTO `item_instance` VALUES ('36', '4', '36 1073741824 3 6096 1065353216 0 4 0 4 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ');
INSERT INTO `item_instance` VALUES ('38', '4', '38 1073741824 3 2070 1065353216 0 4 0 4 0 0 0 0 0 4 0 4294967295 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ');
INSERT INTO `item_instance` VALUES ('40', '4', '40 1073741824 3 159 1065353216 0 4 0 4 0 0 0 0 0 2 0 4294967295 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ');
INSERT INTO `item_instance` VALUES ('42', '4', '42 1073741824 3 6948 1065353216 0 4 0 4 0 0 0 0 0 1 0 0 0 0 0 0 65 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ');
INSERT INTO `item_instance` VALUES ('43', '3', '43 1073741824 3 34244 1065353216 0 3 0 3 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 70 70 ');
INSERT INTO `item_instance` VALUES ('44', '3', '44 1073741824 3 34358 1065353216 0 3 0 3 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ');
INSERT INTO `item_instance` VALUES ('45', '3', '45 1073741824 3 31030 1065353216 0 3 0 3 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 70 70 ');
INSERT INTO `item_instance` VALUES ('46', '3', '46 1073741824 3 34241 1065353216 0 3 0 3 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ');
INSERT INTO `item_instance` VALUES ('47', '3', '47 1073741824 3 34397 1065353216 0 3 0 3 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 120 120 ');
INSERT INTO `item_instance` VALUES ('48', '3', '48 1073741824 3 34448 1065353216 0 3 0 3 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 40 40 ');
INSERT INTO `item_instance` VALUES ('49', '3', '49 1073741824 3 34370 1065353216 0 3 0 3 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 40 40 ');
INSERT INTO `item_instance` VALUES ('50', '3', '50 1073741824 3 32837 1065353216 0 3 0 3 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 125 125 ');
INSERT INTO `item_instance` VALUES ('51', '3', '51 1073741824 3 32838 1065353216 0 3 0 3 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 124 125 ');
INSERT INTO `item_instance` VALUES ('52', '3', '52 1073741824 3 34196 1065353216 0 3 0 3 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 90 90 ');
INSERT INTO `item_instance` VALUES ('53', '3', '53 1073741824 3 34558 1065353216 0 3 0 3 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 40 40 ');
INSERT INTO `item_instance` VALUES ('54', '3', '54 1073741824 3 34188 1065353216 0 3 0 3 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 90 90 ');
INSERT INTO `item_instance` VALUES ('55', '3', '55 1073741824 3 34575 1065353216 0 3 0 3 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 60 60 ');
INSERT INTO `item_instance` VALUES ('56', '3', '56 1073741824 3 34189 1065353216 0 3 0 3 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ');
INSERT INTO `item_instance` VALUES ('57', '3', '57 1073741824 3 34361 1065353216 0 3 0 3 0 0 0 0 0 1 0 0 0 0 0 0 524289 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ');
INSERT INTO `item_instance` VALUES ('58', '3', '58 1073741824 3 28830 1065353216 0 3 0 3 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ');
INSERT INTO `item_instance` VALUES ('59', '3', '59 1073741824 3 34427 1065353216 0 3 0 3 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ');
INSERT INTO `item_instance` VALUES ('60', '3', '60 1073741824 7 23162 1065353216 0 3 0 3 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 36 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ');
INSERT INTO `item_instance` VALUES ('61', '3', '61 1073741824 7 23162 1065353216 0 3 0 3 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 36 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ');
INSERT INTO `item_instance` VALUES ('62', '3', '62 1073741824 7 23162 1065353216 0 3 0 3 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 36 0 68 1073741824 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ');
INSERT INTO `item_instance` VALUES ('63', '3', '63 1073741824 7 23162 1065353216 0 3 0 3 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 36 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ');
INSERT INTO `item_instance` VALUES ('64', '3', '64 1073741824 3 19902 1065353216 0 3 0 3 0 0 0 0 0 1 0 0 0 0 0 0 65 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ');
INSERT INTO `item_instance` VALUES ('66', '3', '66 1073741824 3 31405 1065353216 0 3 0 3 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ');
INSERT INTO `item_instance` VALUES ('67', '3', '67 1073741824 3 30504 1065353216 0 3 0 3 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 40 40 ');
INSERT INTO `item_instance` VALUES ('68', '3', '68 1073741824 3 28053 1065353216 0 3 0 62 1073741824 0 0 0 0 200 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ');
INSERT INTO `item_instance` VALUES ('69', '4', '69 1073741824 3 99966 1065353216 0 4 0 4 0 0 0 0 0 1 0 0 0 0 0 0 134217729 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ');
INSERT INTO `item_instance` VALUES ('70', '4', '70 1073741824 3 2586 1065353216 0 4 0 4 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 34 35 ');
INSERT INTO `item_instance` VALUES ('71', '4', '71 1073741824 3 11508 1065353216 0 4 0 4 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 16 16 ');
INSERT INTO `item_instance` VALUES ('72', '4', '72 1073741824 3 12064 1065353216 0 4 0 4 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 20 20 ');
INSERT INTO `item_instance` VALUES ('73', '4', '73 1073741824 3 13262 1065353216 0 4 0 4 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 145 145 ');
INSERT INTO `item_instance` VALUES ('74', '4', '74 1073741824 3 13347 1065353216 0 4 0 4 0 0 0 0 0 1 0 0 0 0 0 0 2049 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ');
INSERT INTO `item_instance` VALUES ('75', '2', '75 1073741824 3 22416 1065353216 0 2 0 2 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 165 165 ');
INSERT INTO `item_instance` VALUES ('76', '2', '76 1073741824 3 22417 1065353216 0 2 0 2 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 120 120 ');
INSERT INTO `item_instance` VALUES ('77', '2', '77 1073741824 3 22418 1065353216 0 2 0 2 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 100 100 ');
INSERT INTO `item_instance` VALUES ('78', '2', '78 1073741824 3 22419 1065353216 0 2 0 2 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 100 100 ');
INSERT INTO `item_instance` VALUES ('79', '2', '79 1073741824 3 22420 1065353216 0 2 0 2 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 75 75 ');
INSERT INTO `item_instance` VALUES ('80', '2', '80 1073741824 3 22421 1065353216 0 2 0 2 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 55 55 ');
INSERT INTO `item_instance` VALUES ('81', '2', '81 1073741824 3 22422 1065353216 0 2 0 2 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 55 55 ');
INSERT INTO `item_instance` VALUES ('82', '2', '82 1073741824 3 22423 1065353216 0 2 0 2 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 55 55 ');
INSERT INTO `item_instance` VALUES ('83', '2', '83 1073741824 3 23059 1065353216 0 2 0 2 0 0 0 0 0 1 0 0 0 0 0 0 524289 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ');
INSERT INTO `item_instance` VALUES ('84', '2', '84 1073741824 3 22732 1065353216 0 2 0 2 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ');
INSERT INTO `item_instance` VALUES ('85', '2', '85 1073741824 3 21710 1065353216 0 2 0 2 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ');
INSERT INTO `item_instance` VALUES ('86', '2', '86 1073741824 3 19019 1065353216 0 2 0 2 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 125 125 ');
INSERT INTO `item_instance` VALUES ('87', '2', '87 1073741824 3 23043 1065353216 0 2 0 2 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 120 120 ');
INSERT INTO `item_instance` VALUES ('88', '2', '88 1073741824 3 22811 1065353216 0 2 0 2 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 90 90 ');
INSERT INTO `item_instance` VALUES ('89', '2', '89 1073741824 3 21205 1065353216 0 2 0 2 0 0 0 0 0 1 0 0 0 0 0 0 32769 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ');
INSERT INTO `item_instance` VALUES ('90', '2', '90 1073741824 3 19406 1065353216 0 2 0 2 0 0 0 0 0 1 0 0 0 0 0 0 524289 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ');
INSERT INTO `item_instance` VALUES ('91', '2', '91 1073741824 3 19431 1065353216 0 2 0 2 0 0 0 0 0 1 0 0 0 0 0 0 524289 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ');
INSERT INTO `item_instance` VALUES ('92', '2', '92 1073741824 3 23577 1065353216 0 2 0 2 0 0 0 0 0 1 0 0 0 0 0 0 524289 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 105 105 ');

-- ----------------------------
-- Table structure for item_text
-- ----------------------------
DROP TABLE IF EXISTS `item_text`;
CREATE TABLE `item_text` (
  `id` int(11) unsigned NOT NULL DEFAULT '0',
  `text` longtext,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of item_text
-- ----------------------------

-- ----------------------------
-- Table structure for mail
-- ----------------------------
DROP TABLE IF EXISTS `mail`;
CREATE TABLE `mail` (
  `id` int(11) unsigned NOT NULL DEFAULT '0',
  `messageType` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `stationery` tinyint(3) NOT NULL DEFAULT '41',
  `mailTemplateId` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `sender` int(11) unsigned NOT NULL DEFAULT '0',
  `receiver` int(11) unsigned NOT NULL DEFAULT '0',
  `subject` longtext,
  `itemTextId` int(11) unsigned NOT NULL DEFAULT '0',
  `has_items` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `expire_time` bigint(40) NOT NULL DEFAULT '0',
  `deliver_time` bigint(40) NOT NULL DEFAULT '0',
  `money` int(11) unsigned NOT NULL DEFAULT '0',
  `cod` int(11) unsigned NOT NULL DEFAULT '0',
  `checked` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `receiver` (`receiver`),
  KEY `expire_time` (`expire_time`),
  KEY `deliver_time` (`deliver_time`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of mail
-- ----------------------------

-- ----------------------------
-- Table structure for mail_external
-- ----------------------------
DROP TABLE IF EXISTS `mail_external`;
CREATE TABLE `mail_external` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `receiver` bigint(20) unsigned NOT NULL,
  `subject` varchar(200) DEFAULT 'Support Message',
  `message` varchar(500) DEFAULT 'Support Message',
  `money` bigint(20) unsigned NOT NULL DEFAULT '0',
  `item` bigint(20) unsigned NOT NULL DEFAULT '0',
  `item_count` bigint(20) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `receiver` (`receiver`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of mail_external
-- ----------------------------

-- ----------------------------
-- Table structure for mail_items
-- ----------------------------
DROP TABLE IF EXISTS `mail_items`;
CREATE TABLE `mail_items` (
  `mail_id` int(11) NOT NULL DEFAULT '0',
  `item_guid` int(11) NOT NULL DEFAULT '0',
  `item_template` int(11) NOT NULL DEFAULT '0',
  `receiver` int(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`item_guid`),
  KEY `receiver` (`receiver`),
  KEY `idx_mail_id` (`mail_id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of mail_items
-- ----------------------------

-- ----------------------------
-- Table structure for map_template
-- ----------------------------
DROP TABLE IF EXISTS `map_template`;
CREATE TABLE `map_template` (
  `entry` int(3) unsigned NOT NULL COMMENT 'MapID',
  `visibility` float unsigned DEFAULT '533' COMMENT 'VisibilityRadius',
  `pathfinding` smallint(1) unsigned DEFAULT '6' COMMENT 'PathFinding Prioririty\r\n    F_ALWAYS_DISABLED = 0,\r\n    F_LOW_PRIORITY    = 1,\r\n    F_MID_PRIORITY    = 2,\r\n    F_HIGH_PRIORITY   = 3,\r\n\r\n    F_ALWAYS_ENABLED = 6//CB_TRESHOLD_MAX +1,',
  `lineofsight` smallint(1) unsigned DEFAULT '6' COMMENT 'LineOfSight Prioririty',
  `ainotifyperiod` smallint(4) unsigned DEFAULT '1000' COMMENT 'Interval between AI notifications about relocation',
  `viewupdatedistance` smallint(2) unsigned DEFAULT '10' COMMENT 'ViewUpdate minimal distance',
  PRIMARY KEY (`entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of map_template
-- ----------------------------
INSERT INTO `map_template` VALUES ('0', '110', '6', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('1', '110', '6', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('13', '120', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('25', '120', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('29', '120', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('33', '120', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('34', '120', '6', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('36', '120', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('43', '120', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('47', '120', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('48', '120', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('70', '120', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('90', '120', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('109', '120', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('129', '120', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('169', '120', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('189', '120', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('209', '120', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('229', '120', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('230', '120', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('249', '120', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('269', '120', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('289', '120', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('309', '120', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('329', '120', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('349', '120', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('369', '120', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('389', '120', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('409', '200', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('429', '120', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('449', '120', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('450', '120', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('451', '120', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('489', '250', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('509', '120', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('530', '100', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('531', '160', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('532', '130', '3', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('533', '160', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('534', '380', '3', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('540', '160', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('542', '160', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('543', '160', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('544', '160', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('545', '160', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('546', '160', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('547', '160', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('548', '160', '3', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('550', '200', '3', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('552', '160', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('553', '160', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('554', '160', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('555', '160', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('556', '160', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('557', '160', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('558', '160', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('559', '250', '6', '6', '500', '1');
INSERT INTO `map_template` VALUES ('560', '160', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('562', '250', '6', '6', '500', '1');
INSERT INTO `map_template` VALUES ('564', '250', '3', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('565', '160', '3', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('566', '160', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('568', '160', '3', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('572', '250', '6', '6', '500', '5');
INSERT INTO `map_template` VALUES ('580', '200', '6', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('585', '160', '2', '6', '1000', '5');
INSERT INTO `map_template` VALUES ('598', '160', '2', '6', '1000', '5');

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
-- Table structure for opcodes_cooldown
-- ----------------------------
DROP TABLE IF EXISTS `opcodes_cooldown`;
CREATE TABLE `opcodes_cooldown` (
  `opcode` varchar(20) NOT NULL COMMENT 'Opcode name',
  `cooldown` int(4) unsigned DEFAULT '1000' COMMENT 'Opcode cooldown',
  PRIMARY KEY (`opcode`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of opcodes_cooldown
-- ----------------------------
INSERT INTO `opcodes_cooldown` VALUES ('CMSG_ADD_FRIEND   ', '1000');
INSERT INTO `opcodes_cooldown` VALUES ('CMSG_CHANNEL_LIST', '1000');
INSERT INTO `opcodes_cooldown` VALUES ('CMSG_GAMEOBJ_USE', '500');
INSERT INTO `opcodes_cooldown` VALUES ('CMSG_GROUP_CHANGE_SU', '1000');
INSERT INTO `opcodes_cooldown` VALUES ('CMSG_GROUP_INVITE   ', '1000');
INSERT INTO `opcodes_cooldown` VALUES ('CMSG_GUILD_BANK_SWAP', '1000');
INSERT INTO `opcodes_cooldown` VALUES ('CMSG_INSPECT', '2000');
INSERT INTO `opcodes_cooldown` VALUES ('CMSG_WHO', '2000');
INSERT INTO `opcodes_cooldown` VALUES ('CMSG_WHOIS', '1000');

-- ----------------------------
-- Table structure for petition
-- ----------------------------
DROP TABLE IF EXISTS `petition`;
CREATE TABLE `petition` (
  `ownerguid` int(10) unsigned NOT NULL,
  `petitionguid` int(10) unsigned DEFAULT '0',
  `name` varchar(255) NOT NULL DEFAULT '',
  `type` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`ownerguid`,`type`),
  UNIQUE KEY `ownerguid_petitionguid` (`ownerguid`,`petitionguid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of petition
-- ----------------------------

-- ----------------------------
-- Table structure for petition_sign
-- ----------------------------
DROP TABLE IF EXISTS `petition_sign`;
CREATE TABLE `petition_sign` (
  `ownerguid` int(10) unsigned NOT NULL,
  `petitionguid` int(11) unsigned NOT NULL DEFAULT '0',
  `playerguid` int(11) unsigned NOT NULL DEFAULT '0',
  `player_account` int(11) unsigned NOT NULL DEFAULT '0',
  `type` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`petitionguid`,`playerguid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of petition_sign
-- ----------------------------

-- ----------------------------
-- Table structure for pet_aura
-- ----------------------------
DROP TABLE IF EXISTS `pet_aura`;
CREATE TABLE `pet_aura` (
  `guid` int(11) unsigned NOT NULL DEFAULT '0',
  `caster_guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `spell` int(11) unsigned NOT NULL DEFAULT '0',
  `effect_index` int(11) unsigned NOT NULL DEFAULT '0',
  `stackcount` int(11) NOT NULL DEFAULT '1',
  `amount` int(11) NOT NULL DEFAULT '0',
  `maxduration` int(11) NOT NULL DEFAULT '0',
  `remaintime` int(11) NOT NULL DEFAULT '0',
  `remaincharges` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`spell`,`effect_index`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of pet_aura
-- ----------------------------

-- ----------------------------
-- Table structure for pet_spell
-- ----------------------------
DROP TABLE IF EXISTS `pet_spell`;
CREATE TABLE `pet_spell` (
  `guid` int(11) unsigned NOT NULL DEFAULT '0',
  `spell` int(11) unsigned NOT NULL DEFAULT '0',
  `slot` int(11) unsigned NOT NULL DEFAULT '0',
  `active` int(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`spell`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of pet_spell
-- ----------------------------

-- ----------------------------
-- Table structure for pet_spell_cooldown
-- ----------------------------
DROP TABLE IF EXISTS `pet_spell_cooldown`;
CREATE TABLE `pet_spell_cooldown` (
  `guid` int(11) unsigned NOT NULL DEFAULT '0',
  `spell` int(11) unsigned NOT NULL DEFAULT '0',
  `time` bigint(20) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`spell`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of pet_spell_cooldown
-- ----------------------------

-- ----------------------------
-- Table structure for playerbot
-- ----------------------------
DROP TABLE IF EXISTS `playerbot`;
CREATE TABLE `playerbot` (
  `char_guid` bigint(20) unsigned NOT NULL,
  `chance` int(10) unsigned NOT NULL DEFAULT '10',
  `comment` varchar(255) DEFAULT NULL,
  `ai` varchar(50) DEFAULT NULL,
  PRIMARY KEY (`char_guid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of playerbot
-- ----------------------------

-- ----------------------------
-- Table structure for premium
-- ----------------------------
DROP TABLE IF EXISTS `premium`;
CREATE TABLE `premium` (
  `AccountId` int(11) unsigned NOT NULL,
  `active` int(11) unsigned NOT NULL DEFAULT '1'
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of premium
-- ----------------------------

-- ----------------------------
-- Table structure for reserved_name
-- ----------------------------
DROP TABLE IF EXISTS `reserved_name`;
CREATE TABLE `reserved_name` (
  `name` varchar(12) NOT NULL DEFAULT '',
  PRIMARY KEY (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Player Reserved Names';

-- ----------------------------
-- Records of reserved_name
-- ----------------------------

-- ----------------------------
-- Table structure for saved_variables
-- ----------------------------
DROP TABLE IF EXISTS `saved_variables`;
CREATE TABLE `saved_variables` (
  `NextArenaPointDistributionTime` bigint(40) unsigned NOT NULL DEFAULT '0',
  `HeroicQuest` int(5) unsigned NOT NULL DEFAULT '0',
  `NormalQuest` int(5) unsigned NOT NULL DEFAULT '0',
  `CookingQuest` int(5) unsigned NOT NULL DEFAULT '0',
  `FishingQuest` int(5) unsigned NOT NULL DEFAULT '0',
  `PVPAlliance` int(5) unsigned NOT NULL DEFAULT '0',
  `PVPHorde` int(5) unsigned NOT NULL DEFAULT '0',
  `LastCharacterGuid` int(11) unsigned NOT NULL DEFAULT '1'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of saved_variables
-- ----------------------------
INSERT INTO `saved_variables` VALUES ('1', '1', '1', '1', '1', '1', '1', '18');

-- ----------------------------
-- Table structure for spell_disabled
-- ----------------------------
DROP TABLE IF EXISTS `spell_disabled`;
CREATE TABLE `spell_disabled` (
  `entry` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Spell entry',
  `disable_mask` int(8) unsigned NOT NULL DEFAULT '0',
  `comment` varchar(64) NOT NULL DEFAULT '',
  PRIMARY KEY (`entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Disabled Spell System';

-- ----------------------------
-- Records of spell_disabled
-- ----------------------------
INSERT INTO `spell_disabled` VALUES ('1852', '7', 'Silenced (GM Tool) Spell bugged');
INSERT INTO `spell_disabled` VALUES ('46642', '7', '5k gold');

-- ----------------------------
-- Table structure for ticket_history
-- ----------------------------
DROP TABLE IF EXISTS `ticket_history`;
CREATE TABLE `ticket_history` (
  `guid` int(10) NOT NULL AUTO_INCREMENT,
  `playerGuid` int(11) unsigned NOT NULL DEFAULT '0',
  `name` varchar(15) NOT NULL,
  `message` text NOT NULL,
  `createtime` int(10) NOT NULL,
  `timestamp` int(10) NOT NULL DEFAULT '0',
  `closed` int(10) NOT NULL DEFAULT '0',
  `comment` text NOT NULL,
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of ticket_history
-- ----------------------------

-- ----------------------------
-- Table structure for titles_to_award
-- ----------------------------
DROP TABLE IF EXISTS `titles_to_award`;
CREATE TABLE `titles_to_award` (
  `guid` int(10) unsigned NOT NULL,
  `mask` int(14) unsigned NOT NULL,
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of titles_to_award
-- ----------------------------

-- ----------------------------
-- Table structure for uptime
-- ----------------------------
DROP TABLE IF EXISTS `uptime`;
CREATE TABLE `uptime` (
  `starttime` bigint(20) unsigned NOT NULL DEFAULT '0',
  `startstring` varchar(64) NOT NULL DEFAULT '',
  `uptime` bigint(20) unsigned NOT NULL DEFAULT '0',
  `maxplayers` smallint(5) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`starttime`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Uptime system';

-- ----------------------------
-- Records of uptime
-- ----------------------------
INSERT INTO `uptime` VALUES ('1655679625', '2022-06-20 02:00:25', '0', '0');
INSERT INTO `uptime` VALUES ('1655679861', '2022-06-20 02:04:21', '0', '0');
INSERT INTO `uptime` VALUES ('1655688440', '2022-06-20 04:27:20', '1200', '1');
INSERT INTO `uptime` VALUES ('1655689818', '2022-06-20 04:50:18', '0', '0');
INSERT INTO `uptime` VALUES ('1655690061', '2022-06-20 04:54:21', '0', '0');
INSERT INTO `uptime` VALUES ('1655690190', '2022-06-20 04:56:30', '600', '1');
INSERT INTO `uptime` VALUES ('1655691094', '2022-06-20 05:11:34', '0', '0');
INSERT INTO `uptime` VALUES ('1655691330', '2022-06-20 05:15:30', '1800', '1');
INSERT INTO `uptime` VALUES ('1655693561', '2022-06-20 05:52:41', '0', '0');
INSERT INTO `uptime` VALUES ('1655694416', '2022-06-20 06:06:56', '0', '0');
INSERT INTO `uptime` VALUES ('1655695448', '2022-06-20 06:24:08', '0', '0');
INSERT INTO `uptime` VALUES ('1655696736', '2022-06-20 06:45:36', '0', '0');
INSERT INTO `uptime` VALUES ('1655697220', '2022-06-20 06:53:40', '1801', '1');
INSERT INTO `uptime` VALUES ('1655699295', '2022-06-20 07:28:15', '0', '0');
INSERT INTO `uptime` VALUES ('1655699490', '2022-06-20 07:31:30', '0', '0');
INSERT INTO `uptime` VALUES ('1655702208', '2022-06-20 08:16:48', '2400', '1');
INSERT INTO `uptime` VALUES ('1655704778', '2022-06-20 08:59:38', '0', '0');
INSERT INTO `uptime` VALUES ('1655705145', '2022-06-20 09:05:45', '0', '0');
INSERT INTO `uptime` VALUES ('1655705730', '2022-06-20 09:15:30', '0', '0');
INSERT INTO `uptime` VALUES ('1655706163', '2022-06-20 09:22:43', '600', '1');
INSERT INTO `uptime` VALUES ('1655706848', '2022-06-20 09:34:08', '0', '0');
INSERT INTO `uptime` VALUES ('1655706959', '2022-06-20 09:35:59', '0', '0');
INSERT INTO `uptime` VALUES ('1655707021', '2022-06-20 09:37:01', '0', '0');
INSERT INTO `uptime` VALUES ('1655707440', '2022-06-20 09:44:00', '600', '1');
INSERT INTO `uptime` VALUES ('1655708598', '2022-06-20 10:03:18', '600', '1');
INSERT INTO `uptime` VALUES ('1655709746', '2022-06-20 10:22:26', '0', '0');
INSERT INTO `uptime` VALUES ('1655709789', '2022-06-20 10:23:09', '600', '1');
INSERT INTO `uptime` VALUES ('1655712100', '2022-06-20 11:01:40', '1800', '1');
INSERT INTO `uptime` VALUES ('1655714386', '2022-06-20 11:39:46', '0', '0');
INSERT INTO `uptime` VALUES ('1655714397', '2022-06-20 11:39:57', '0', '0');
INSERT INTO `uptime` VALUES ('1655751572', '2022-06-20 21:59:32', '2400', '1');
INSERT INTO `uptime` VALUES ('1655756649', '2022-06-20 23:24:09', '0', '0');
INSERT INTO `uptime` VALUES ('1655756674', '2022-06-20 23:24:34', '0', '0');
INSERT INTO `uptime` VALUES ('1655756745', '2022-06-20 23:25:45', '0', '0');
INSERT INTO `uptime` VALUES ('1655756964', '2022-06-20 23:29:24', '600', '1');
INSERT INTO `uptime` VALUES ('1655757683', '2022-06-20 23:41:23', '0', '0');
INSERT INTO `uptime` VALUES ('1655758209', '2022-06-20 23:50:09', '0', '0');
INSERT INTO `uptime` VALUES ('1655758283', '2022-06-20 23:51:23', '0', '0');
INSERT INTO `uptime` VALUES ('1655758373', '2022-06-20 23:52:53', '0', '0');
INSERT INTO `uptime` VALUES ('1655758447', '2022-06-20 23:54:07', '0', '0');
INSERT INTO `uptime` VALUES ('1655758531', '2022-06-20 23:55:31', '0', '0');
INSERT INTO `uptime` VALUES ('1655758631', '2022-06-20 23:57:11', '0', '0');
INSERT INTO `uptime` VALUES ('1655759142', '2022-06-21 00:05:42', '600', '1');
INSERT INTO `uptime` VALUES ('1655759781', '2022-06-21 00:16:21', '0', '0');
INSERT INTO `uptime` VALUES ('1655759795', '2022-06-21 00:16:35', '0', '0');
INSERT INTO `uptime` VALUES ('1655760246', '2022-06-21 00:24:06', '0', '0');
INSERT INTO `uptime` VALUES ('1655760536', '2022-06-21 00:28:56', '0', '0');
INSERT INTO `uptime` VALUES ('1655760645', '2022-06-21 00:30:45', '0', '0');
INSERT INTO `uptime` VALUES ('1655761276', '2022-06-21 00:41:16', '0', '0');
INSERT INTO `uptime` VALUES ('1655761697', '2022-06-21 00:48:17', '600', '1');
INSERT INTO `uptime` VALUES ('1655762575', '2022-06-21 01:02:55', '0', '0');
INSERT INTO `uptime` VALUES ('1655762705', '2022-06-21 01:05:05', '0', '0');
INSERT INTO `uptime` VALUES ('1655762784', '2022-06-21 01:06:24', '0', '0');
INSERT INTO `uptime` VALUES ('1655762891', '2022-06-21 01:08:11', '601', '1');
INSERT INTO `uptime` VALUES ('1655764599', '2022-06-21 01:36:39', '600', '1');
INSERT INTO `uptime` VALUES ('1655765585', '2022-06-21 01:53:05', '0', '0');
INSERT INTO `uptime` VALUES ('1667311516', '2022-11-01 16:05:16', '0', '0');
INSERT INTO `uptime` VALUES ('1667527602', '2022-11-04 10:06:42', '756', '1');
INSERT INTO `uptime` VALUES ('1670993238', '2022-12-14 12:47:18', '0', '0');
INSERT INTO `uptime` VALUES ('1670999642', '2022-12-14 14:34:02', '2284', '2');
INSERT INTO `uptime` VALUES ('1671002801', '2022-12-14 15:26:41', '0', '0');
INSERT INTO `uptime` VALUES ('1671092904', '2022-12-15 16:28:24', '18805', '2');
INSERT INTO `uptime` VALUES ('1671161955', '2022-12-16 11:39:15', '0', '0');
INSERT INTO `uptime` VALUES ('1671162356', '2022-12-16 11:45:56', '2277', '2');
INSERT INTO `uptime` VALUES ('1671165364', '2022-12-16 12:36:04', '3801', '1');
INSERT INTO `uptime` VALUES ('1671169479', '2022-12-16 13:44:39', '0', '0');
INSERT INTO `uptime` VALUES ('1671169653', '2022-12-16 13:47:33', '762', '1');
INSERT INTO `uptime` VALUES ('1671171233', '2022-12-16 14:13:53', '0', '0');
INSERT INTO `uptime` VALUES ('1671173898', '2022-12-16 14:58:18', '1521', '1');
INSERT INTO `uptime` VALUES ('1671175775', '2022-12-16 15:29:35', '5306', '1');
INSERT INTO `uptime` VALUES ('1671184112', '2022-12-16 17:48:32', '0', '0');
INSERT INTO `uptime` VALUES ('1671184636', '2022-12-16 17:57:16', '0', '0');
INSERT INTO `uptime` VALUES ('1671242689', '2022-12-17 10:04:49', '0', '0');
INSERT INTO `uptime` VALUES ('1671243128', '2022-12-17 10:12:08', '754', '1');
INSERT INTO `uptime` VALUES ('1671244358', '2022-12-17 10:32:38', '760', '1');
INSERT INTO `uptime` VALUES ('1671245883', '2022-12-17 10:58:03', '0', '0');
INSERT INTO `uptime` VALUES ('1671245983', '2022-12-17 10:59:43', '0', '0');

-- ----------------------------
-- View structure for arena_fights_detailed_view
-- ----------------------------
DROP VIEW IF EXISTS `arena_fights_detailed_view`;
CREATE ALGORITHM=UNDEFINED DEFINER=`elyrion`@`%` SQL SECURITY DEFINER VIEW `arena_fights_detailed_view` AS (select `d`.`fight_guid` AS `fight_guid`,`t`.`type` AS `type`,`c`.`name` AS `nick`,`w`.`name` AS `arena_team_name`,`d`.`team_id` AS `team_id`,`d`.`damage_done` AS `damage_done`,`d`.`healing_done` AS `healing_done`,`d`.`kills` AS `kills`,`d`.`personal_rating` AS `personal_rating`,`d`.`rating_change` AS `rating_change`,`t`.`timestamp` AS `timestamp`,`t`.`length` AS `length` from (((`arena_fights_detailed` `d` join `arena_fights` `t` on((`d`.`fight_guid` = `t`.`fight_guid`))) left join `arena_team` `w` on((`w`.`arenateamid` = `d`.`team_id`))) left join `characters` `c` on((`d`.`player_guid` = `c`.`guid`))) where (`t`.`timestamp` < (sysdate() - interval 12 hour)) order by `t`.`timestamp` desc) ;

-- ----------------------------
-- View structure for arena_fights_view
-- ----------------------------
DROP VIEW IF EXISTS `arena_fights_view`;
CREATE ALGORITHM=UNDEFINED DEFINER=`elyrion`@`%` SQL SECURITY DEFINER VIEW `arena_fights_view` AS (select `f`.`fight_guid` AS `fight_guid`,`f`.`type` AS `type`,`w`.`name` AS `winners_name`,`f`.`winners_id` AS `winners_id`,`f`.`winners_rating` AS `winners_rating`,`l`.`name` AS `losers_name`,`f`.`losers_id` AS `losers_id`,`f`.`losers_rating` AS `losers_rating`,`f`.`rating_change` AS `rating_change`,`f`.`timestamp` AS `timestamp`,`f`.`length` AS `length` from ((`arena_fights` `f` join `arena_team` `w` on((`f`.`winners_id` = `w`.`arenateamid`))) left join `arena_team` `l` on((`f`.`losers_id` = `l`.`arenateamid`))) where (`f`.`timestamp` < (sysdate() - interval 12 hour)) order by `f`.`timestamp` desc) ;

-- ----------------------------
-- View structure for boss_fights_detailed_view
-- ----------------------------
DROP VIEW IF EXISTS `boss_fights_detailed_view`;
CREATE ALGORITHM=UNDEFINED DEFINER=`elyrion`@`%` SQL SECURITY DEFINER VIEW `boss_fights_detailed_view` AS (select `b`.`kill_id` AS `kill_id`,`c`.`name` AS `player_name`,`b`.`player_guid` AS `player_guid`,`b`.`healing` AS `healing`,`b`.`damage` AS `damage`,`b`.`deaths` AS `deaths` from (`boss_fights_detailed` `b` left join `characters` `c` on((`c`.`guid` = `b`.`player_guid`)))) ;

-- ----------------------------
-- View structure for boss_fights_loot_view
-- ----------------------------
DROP VIEW IF EXISTS `boss_fights_loot_view`;
CREATE ALGORITHM=UNDEFINED DEFINER=`elyrion`@`%` SQL SECURITY DEFINER VIEW `boss_fights_loot_view` AS (select `l`.`kill_id` AS `kill_id`,`l`.`item_id` AS `item_id`,`t`.`name` AS `name`,`l`.`count` AS `count` from (`characters`.`boss_fights_loot` `l` left join `mangos`.`item_template` `t` on((`t`.`entry` = `l`.`item_id`)))) ;

-- ----------------------------
-- View structure for boss_fights_view
-- ----------------------------
DROP VIEW IF EXISTS `boss_fights_view`;
CREATE ALGORITHM=UNDEFINED DEFINER=`elyrion`@`%` SQL SECURITY DEFINER VIEW `boss_fights_view` AS (select `f`.`kill_id` AS `kill_id`,`n`.`boss_name` AS `boss_name`,`f`.`mob_id` AS `mob_id`,`l`.`boss_location` AS `location`,`f`.`instance_id` AS `instance_id`,`g`.`name` AS `name`,`f`.`guild_id` AS `guild_id`,`f`.`length` AS `length`,`f`.`date` AS `date` from (((`boss_fights` `f` join `boss_id_names` `n` on((`f`.`mob_id` = `n`.`boss_id`))) left join `guild` `g` on((`f`.`guild_id` = `g`.`guildid`))) join `boss_id_locations` `l` on((`n`.`boss_id` = `l`.`boss_id`)))) ;

-- ----------------------------
-- Procedure structure for PreventCharDelete
-- ----------------------------
DROP PROCEDURE IF EXISTS `PreventCharDelete`;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `PreventCharDelete`(IN charguid INT UNSIGNED)
BEGIN
    INSERT INTO deleted_chars VALUES ('XXX', charguid, (SELECT name FROM characters WHERE guid = charguid), (SELECT account FROM characters WHERE guid = charguid), CAST(NOW() AS DATETIME));
    UPDATE characters SET account = 1 WHERE guid = charguid;
    UPDATE characters SET name = CONCAT('DEL', name, 'DEL') WHERE guid = charguid;
    DELETE FROM character_social WHERE guid = charguid OR friend = charguid;
    DELETE FROM mail WHERE receiver = charguid;
    DELETE FROM mail_items WHERE receiver = charguid;
END
;;
DELIMITER ;
DROP TRIGGER IF EXISTS `accchange_upd_check`;
DELIMITER ;;
CREATE TRIGGER `accchange_upd_check` BEFORE UPDATE ON `characters` FOR EACH ROW BEGIN
	IF NEW.account != OLD.account THEN
		INSERT INTO realmd.accchange_log VALUES ('XXX', OLD.account, NEW.account, NOW(), NEW.guid);
	END IF;
  END
;;
DELIMITER ;
DROP TRIGGER IF EXISTS `ticket_insert_check`;
DELIMITER ;;
CREATE TRIGGER `ticket_insert_check` BEFORE INSERT ON `gm_tickets` FOR EACH ROW BEGIN
    INSERT INTO ticket_history VALUES ('XXX', NEW.playerGuid, NEW.name, NEW.message, NEW.createtime, NEW.timestamp, NEW.closed, NEW.comment);
  END
;;
DELIMITER ;
DROP TRIGGER IF EXISTS `ticket_upd_check`;
DELIMITER ;;
CREATE TRIGGER `ticket_upd_check` BEFORE UPDATE ON `gm_tickets` FOR EACH ROW BEGIN
    UPDATE ticket_history
    SET message = NEW.message, name = NEW.name, timestamp = NEW.timestamp, closed = NEW.closed, comment = NEW.comment
    WHERE playerGuid = NEW.playerGuid AND createtime = NEW.createtime;
  END
;;
DELIMITER ;
