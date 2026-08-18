/*
Navicat MySQL Data Transfer

Source Server         : db
Source Server Version : 80018
Source Host           : localhost:3306
Source Database       : dbo_acc

Target Server Type    : MYSQL
Target Server Version : 80018
File Encoding         : 65001

Date: 2020-04-21 14:05:53
*/

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for accounts
-- ----------------------------
DROP TABLE IF EXISTS `accounts`;
CREATE TABLE `accounts` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `username` varchar(16) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL,
  `password_hash` varchar(64) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL COMMENT 'password in sha3-256',
  `acc_status` enum('pending','block','active') CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL DEFAULT 'active',
  `email` varchar(80) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL DEFAULT 'test@mail.com',
  `mallpoints` int(10) unsigned NOT NULL DEFAULT '10000000',
  `reg_date` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `last_login` timestamp NULL DEFAULT NULL,
  `reg_ip` varchar(15) CHARACTER SET latin1 COLLATE latin1_swedish_ci DEFAULT NULL,
  `admin` tinyint(2) NOT NULL DEFAULT '0' COMMENT 'value from 0 to 10',
  `is_gm` tinyint(1) NOT NULL DEFAULT '0' COMMENT '0 = normal user / 1 = game master or people with who can login in testing phase',
  `last_server_farm_id` tinyint(3) unsigned NOT NULL DEFAULT '255' COMMENT 'default: INVALID_SERVERFARMID ( 255 )',
  `founder` smallint(1) NOT NULL DEFAULT '0' COMMENT '0 = no founder / 1 = first founder / 2 = second / 3 = third',
  `founder_recv` smallint(1) NOT NULL DEFAULT '0' COMMENT '0 = founder not received / 1 = founder received',
  `last_ip` varchar(16) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL DEFAULT '0.0.0.0',
  `del_char_pw` varchar(64) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL DEFAULT '87cd084d190e436f147322b90e7384f6a8e0676c99d21ef519ea718e51d45f9c',
  `premium_slots` tinyint(1) unsigned NOT NULL DEFAULT '4',
  `event_coins` int(10) unsigned DEFAULT '0' COMMENT 'coins used to play on HLS event machine',
  `wagu_coins` int(10) unsigned DEFAULT '0' COMMENT 'coins used to play on HLS slot machine',
  `web_ip` varchar(15) CHARACTER SET latin1 COLLATE latin1_swedish_ci DEFAULT NULL,
  PRIMARY KEY (`id`,`username`),
  UNIQUE KEY `id` (`id`) USING BTREE,
  UNIQUE KEY `username` (`username`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for accounts_banned
-- ----------------------------
DROP TABLE IF EXISTS `accounts_banned`;
CREATE TABLE `accounts_banned` (
  `id` int(20) unsigned NOT NULL AUTO_INCREMENT,
  `gm_account_id` int(16) unsigned NOT NULL DEFAULT '0',
  `banned_account_id` int(16) unsigned NOT NULL DEFAULT '0',
  `date_time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `reason` varchar(1024) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL,
  `duration` tinyint(3) unsigned NOT NULL DEFAULT '1' COMMENT 'Value in days. 255 = permanent',
  `active` tinyint(1) NOT NULL DEFAULT '1' COMMENT '1 = true, 0 = false',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for cashshop_storage
-- ----------------------------
DROP TABLE IF EXISTS `cashshop_storage`;
CREATE TABLE `cashshop_storage` (
  `id` int(20) unsigned NOT NULL AUTO_INCREMENT,
  `account_id` int(10) unsigned NOT NULL,
  `hls_item_tblidx` int(10) unsigned NOT NULL,
  `stack_count` tinyint(3) unsigned NOT NULL,
  `gift_char_id` int(10) unsigned DEFAULT NULL,
  `is_read` tinyint(1) NOT NULL DEFAULT '0',
  `sender_name` varchar(16) CHARACTER SET latin1 COLLATE latin1_swedish_ci DEFAULT NULL,
  `year` int(4) unsigned NOT NULL,
  `month` tinyint(2) unsigned NOT NULL,
  `day` tinyint(2) unsigned NOT NULL,
  `hour` tinyint(2) unsigned NOT NULL,
  `minute` tinyint(2) unsigned NOT NULL,
  `second` tinyint(2) unsigned NOT NULL,
  `millisecond` int(4) unsigned NOT NULL,
  `is_moved` tinyint(1) NOT NULL DEFAULT '0' COMMENT 'BOOL. Did the player move the item to his inventory',
  `buyer_account_id` int(10) unsigned DEFAULT '0' COMMENT 'account id from the buyer',
  `price` int(10) unsigned DEFAULT '0',
  `item_id` bigint(20) unsigned DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `id` (`id`,`account_id`) USING BTREE,
  KEY `account_id` (`account_id`,`is_moved`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for event_reward
-- ----------------------------
DROP TABLE IF EXISTS `event_reward`;
CREATE TABLE `event_reward` (
  `account_id` int(15) unsigned DEFAULT NULL,
  `reward_tblidx` int(15) unsigned DEFAULT NULL,
  `char_id` int(15) unsigned DEFAULT '0',
  `char_name` varchar(15) CHARACTER SET latin1 COLLATE latin1_swedish_ci DEFAULT NULL,
  UNIQUE KEY `account_id` (`account_id`,`reward_tblidx`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for shortcuts
-- ----------------------------
DROP TABLE IF EXISTS `shortcuts`;
CREATE TABLE `shortcuts` (
  `account_id` int(10) unsigned NOT NULL,
  `action_id` int(10) unsigned NOT NULL DEFAULT '0',
  `key_code` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`account_id`,`action_id`),
  UNIQUE KEY `account_id` (`account_id`,`action_id`) USING BTREE,
  KEY `account_id_2` (`account_id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
