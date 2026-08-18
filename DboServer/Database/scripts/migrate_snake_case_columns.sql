-- =====================================================================
-- migrate_snake_case_columns.sql
--
-- PURPOSE:
--   Migrates an EXISTING dbo_acc / dbo_char / dbo_log database (created
--   from the old dbo_acc.sql / dbo_char.sql / dbo_log.sql dumps) to the
--   new column naming convention now used by those schema files and by
--   the server code:
--     - snake_case, all lowercase column names
--     - every table's own surrogate primary key is simply named `id`
--     - foreign keys are named `<referenced table, singular>_id`
--       (e.g. `account_id`, `char_id`, `guild_id`, `item_id`)
--     - Hungarian-notation type prefixes are removed (e.g. `wKey` -> `key_code`)
--     - numbered-suffix columns get an underscore before the digit
--       (e.g. `ProductId1` -> `product_id_1`, `WinnerCharID1` -> `winner_char_id_1`)
--
--   This only renames columns - no data is modified and no columns are
--   added/dropped. Table names are unchanged.
--
-- REQUIREMENTS:
--   Uses `ALTER TABLE ... CHANGE COLUMN old new column_definition`, which
--   is supported by every MySQL/MariaDB version (unlike the newer bare
--   `RENAME COLUMN old TO new` syntax, which requires MySQL 8.0.4+ /
--   MariaDB 10.5.2+ and is NOT available on older servers). Because
--   CHANGE COLUMN requires restating the column's type/attributes, each
--   clause below reproduces the exact original definition from the old
--   dbo_*.sql dumps (type, charset/collation, NULL-ability, DEFAULT,
--   AUTO_INCREMENT, COMMENT) so nothing about the column changes except
--   its name.
--
-- USAGE:
--   1. BACK UP the target databases first (e.g. mysqldump).
--   2. Run this file once against the databases you want migrated:
--        mysql -u <user> -p < migrate_snake_case_columns.sql
--   3. Deploy the server binaries built against the updated schema
--      (queries in the repository layer now use the new column names).
--      Do not run old server binaries against a migrated database, or
--      vice versa - the column names must match the code you run.
--
--   Running this twice is safe as a no-op ONLY if it hasn't been run
--   yet; running it again after it already succeeded will fail because
--   the old column names no longer exist (that's expected/harmless -
--   it just means the migration already happened).
-- =====================================================================

-- ---------------------------------------------------------------------
-- dbo_acc
-- ---------------------------------------------------------------------
USE dbo_acc;

ALTER TABLE accounts
  CHANGE COLUMN AccountID id int(10) unsigned NOT NULL AUTO_INCREMENT,
  CHANGE COLUMN Username username varchar(16) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL,
  CHANGE COLUMN Password_hash password_hash varchar(64) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL COMMENT 'password in sha3-256',
  CHANGE COLUMN isGm is_gm tinyint(1) NOT NULL DEFAULT '0' COMMENT '0 = normal user / 1 = game master or people with who can login in testing phase',
  CHANGE COLUMN lastServerFarmId last_server_farm_id tinyint(3) unsigned NOT NULL DEFAULT '255' COMMENT 'default: INVALID_SERVERFARMID ( 255 )',
  CHANGE COLUMN PremiumSlots premium_slots tinyint(1) unsigned NOT NULL DEFAULT '4',
  CHANGE COLUMN EventCoins event_coins int(10) unsigned DEFAULT '0' COMMENT 'coins used to play on HLS event machine',
  CHANGE COLUMN WaguCoins wagu_coins int(10) unsigned DEFAULT '0' COMMENT 'coins used to play on HLS slot machine';

ALTER TABLE accounts_banned
  CHANGE COLUMN GM_AccId gm_account_id int(16) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN Banned_AccId banned_account_id int(16) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN DateTime date_time timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  CHANGE COLUMN Reason reason varchar(1024) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL,
  CHANGE COLUMN Duration duration tinyint(3) unsigned NOT NULL DEFAULT '1' COMMENT 'Value in days. 255 = permanent',
  CHANGE COLUMN Active active tinyint(1) NOT NULL DEFAULT '1' COMMENT '1 = true, 0 = false';

ALTER TABLE cashshop_storage
  CHANGE COLUMN ProductId id int(20) unsigned NOT NULL AUTO_INCREMENT,
  CHANGE COLUMN AccountID account_id int(10) unsigned NOT NULL,
  CHANGE COLUMN HLSitemTblidx hls_item_tblidx int(10) unsigned NOT NULL,
  CHANGE COLUMN StackCount stack_count tinyint(3) unsigned NOT NULL,
  CHANGE COLUMN giftCharId gift_char_id int(10) unsigned DEFAULT NULL,
  CHANGE COLUMN IsRead is_read tinyint(1) NOT NULL DEFAULT '0',
  CHANGE COLUMN SenderName sender_name varchar(16) CHARACTER SET latin1 COLLATE latin1_swedish_ci DEFAULT NULL,
  CHANGE COLUMN isMoved is_moved tinyint(1) NOT NULL DEFAULT '0' COMMENT 'BOOL. Did the player move the item to his inventory',
  CHANGE COLUMN Buyer buyer_account_id int(10) unsigned DEFAULT '0' COMMENT 'account id from the buyer',
  CHANGE COLUMN ItemID item_id bigint(20) unsigned DEFAULT '0';

ALTER TABLE event_reward
  CHANGE COLUMN AccountID account_id int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN rewardTblidx reward_tblidx int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN CharID char_id int(15) unsigned DEFAULT '0',
  CHANGE COLUMN CharName char_name varchar(15) CHARACTER SET latin1 COLLATE latin1_swedish_ci DEFAULT NULL;

ALTER TABLE shortcuts
  CHANGE COLUMN AccountID account_id int(10) unsigned NOT NULL,
  CHANGE COLUMN ActionID action_id int(10) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN wKey key_code int(10) unsigned NOT NULL DEFAULT '0';

-- ---------------------------------------------------------------------
-- dbo_char
-- ---------------------------------------------------------------------
USE dbo_char;

ALTER TABLE auctionhouse
  CHANGE COLUMN CharID char_id int(10) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN TabType tab_type tinyint(3) unsigned NOT NULL DEFAULT '255',
  CHANGE COLUMN ItemName item_name varchar(33) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci DEFAULT NULL,
  CHANGE COLUMN Seller seller_name varchar(17) CHARACTER SET latin1 COLLATE latin1_swedish_ci DEFAULT NULL,
  CHANGE COLUMN ItemID item_id bigint(20) unsigned NOT NULL,
  CHANGE COLUMN TimeStart time_start int(10) unsigned NOT NULL,
  CHANGE COLUMN TimeEnd time_end int(10) unsigned DEFAULT NULL COMMENT 'time in seconds',
  CHANGE COLUMN ItemLevel item_level tinyint(3) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN NeedClass need_class int(10) unsigned NOT NULL,
  CHANGE COLUMN ItemType item_type tinyint(3) unsigned NOT NULL DEFAULT '0';

ALTER TABLE bannword
  CHANGE COLUMN bannword word varchar(50) CHARACTER SET latin1 COLLATE latin1_swedish_ci DEFAULT NULL;

ALTER TABLE bind
  CHANGE COLUMN CharID char_id int(10) unsigned NOT NULL,
  CHANGE COLUMN WorldID world_id int(10) unsigned NOT NULL DEFAULT '1',
  CHANGE COLUMN BindObjectTblIdx bind_object_tblidx int(10) unsigned NOT NULL DEFAULT '4294967295',
  CHANGE COLUMN LocX loc_x float(11,6) NOT NULL,
  CHANGE COLUMN LocY loc_y float(11,6) NOT NULL,
  CHANGE COLUMN LocZ loc_z float(11,6) NOT NULL,
  CHANGE COLUMN DirX dir_x float(11,6) NOT NULL,
  CHANGE COLUMN DirY dir_y float(11,6) NOT NULL,
  CHANGE COLUMN DirZ dir_z float(11,6) NOT NULL,
  CHANGE COLUMN Type type tinyint(1) NOT NULL DEFAULT '1';

ALTER TABLE buffs
  CHANGE COLUMN CharID char_id int(10) unsigned NOT NULL,
  CHANGE COLUMN SourceTblidx source_tblidx int(10) unsigned NOT NULL,
  CHANGE COLUMN SourceType source_type tinyint(3) unsigned NOT NULL DEFAULT '255' COMMENT '0 skill and 1 item',
  CHANGE COLUMN BuffIndex buff_index tinyint(3) unsigned NOT NULL DEFAULT '255',
  CHANGE COLUMN BuffGroup buff_group tinyint(3) unsigned NOT NULL DEFAULT '255',
  CHANGE COLUMN InitialDuration initial_duration int(10) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN TimeRemaining time_remaining int(10) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN effectValue1 effect_value_1 double(10,2) DEFAULT NULL,
  CHANGE COLUMN effectValue2 effect_value_2 double(10,2) DEFAULT NULL,
  CHANGE COLUMN Argument1_0 argument_1_0 int(10) unsigned DEFAULT NULL COMMENT 'commonConfigTblidx',
  CHANGE COLUMN Argument1_1 argument_1_1 int(10) unsigned DEFAULT NULL COMMENT 'dwRemainTime',
  CHANGE COLUMN Argument1_2 argument_1_2 int(10) unsigned DEFAULT NULL COMMENT 'dwRemainValue',
  CHANGE COLUMN Argument2_0 argument_2_0 int(10) unsigned DEFAULT NULL COMMENT 'commonConfigTblidx',
  CHANGE COLUMN Argument2_1 argument_2_1 int(10) unsigned DEFAULT NULL COMMENT 'dwRemainTime',
  CHANGE COLUMN Argument2_2 argument_2_2 int(10) unsigned DEFAULT NULL COMMENT 'dwRemainValue';

ALTER TABLE characters
  CHANGE COLUMN CharID id int(10) unsigned NOT NULL,
  CHANGE COLUMN CharName char_name varchar(16) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL,
  CHANGE COLUMN AccountID account_id int(10) unsigned NOT NULL,
  CHANGE COLUMN Level level tinyint(3) unsigned NOT NULL DEFAULT '1',
  CHANGE COLUMN Exp exp int(10) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN Race race tinyint(1) unsigned DEFAULT NULL,
  CHANGE COLUMN Class class tinyint(2) unsigned DEFAULT NULL,
  CHANGE COLUMN Gender gender tinyint(1) unsigned DEFAULT NULL,
  CHANGE COLUMN Face face tinyint(2) unsigned DEFAULT NULL,
  CHANGE COLUMN Adult adult tinyint(1) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN Hair hair tinyint(2) unsigned NOT NULL,
  CHANGE COLUMN HairColor hair_color tinyint(2) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN SkinColor skin_color tinyint(2) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN Blood blood tinyint(2) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN CurLocX cur_loc_x float(11,6) NOT NULL DEFAULT '78.900002',
  CHANGE COLUMN CurLocY cur_loc_y float(11,6) NOT NULL DEFAULT '46.950001',
  CHANGE COLUMN CurLocZ cur_loc_z float(11,6) NOT NULL DEFAULT '168.350006',
  CHANGE COLUMN CurDirX cur_dir_x float(11,6) NOT NULL DEFAULT '0.950000',
  CHANGE COLUMN CurDirY cur_dir_y float(11,6) NOT NULL DEFAULT '0.000000',
  CHANGE COLUMN CurDirZ cur_dir_z float(11,6) NOT NULL DEFAULT '0.300000',
  CHANGE COLUMN WorldID world_id int(10) unsigned NOT NULL DEFAULT '1',
  CHANGE COLUMN WorldTable world_table int(10) unsigned NOT NULL DEFAULT '1',
  CHANGE COLUMN MapInfoIndex map_info_index int(10) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN Money money int(10) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN MoneyBank money_bank int(10) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN TutorialFlag tutorial_flag tinyint(1) NOT NULL DEFAULT '0' COMMENT '0 = start tutorial / 1 = dont start tutorial',
  CHANGE COLUMN TutorialHint tutorial_hint int(10) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN NameChange name_change tinyint(1) NOT NULL DEFAULT '0',
  CHANGE COLUMN Reputation reputation int(10) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN MudosaPoint mudosa_point int(10) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN SpPoint sp_point int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'skill points',
  CHANGE COLUMN GameMaster game_master tinyint(1) NOT NULL DEFAULT '0',
  CHANGE COLUMN GuildID guild_id int(10) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN GuildName guild_name varchar(16) CHARACTER SET latin1 COLLATE latin1_swedish_ci DEFAULT NULL,
  CHANGE COLUMN CurLP cur_lp int(10) NOT NULL DEFAULT '15000',
  CHANGE COLUMN CurEP cur_ep smallint(5) unsigned NOT NULL DEFAULT '15000',
  CHANGE COLUMN CurRP cur_rp smallint(5) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN CurAP cur_ap int(10) NOT NULL DEFAULT '450000',
  CHANGE COLUMN MailIsAway mail_is_away tinyint(1) NOT NULL DEFAULT '0',
  CHANGE COLUMN SrvFarmID srv_farm_id int(3) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN DelCharTime del_char_time bigint(20) unsigned DEFAULT NULL COMMENT 'time(0) + 43200 = 12 hours',
  CHANGE COLUMN Hoipoi_NormalStart hoipoi_normal_start tinyint(1) NOT NULL DEFAULT '0',
  CHANGE COLUMN Hoipoi_SpecialStart hoipoi_special_start tinyint(1) NOT NULL DEFAULT '0',
  CHANGE COLUMN Hoipoi_Type hoipoi_type tinyint(3) unsigned NOT NULL DEFAULT '255',
  CHANGE COLUMN Hoipoi_MixLevel hoipoi_mix_level tinyint(3) unsigned NOT NULL DEFAULT '1',
  CHANGE COLUMN Hoipoi_MixExp hoipoi_mix_exp int(10) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN Title title int(10) unsigned NOT NULL DEFAULT '4294967295',
  CHANGE COLUMN Mascot mascot int(10) unsigned NOT NULL DEFAULT '4294967295',
  CHANGE COLUMN RpBall rp_ball tinyint(1) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN Netpy netpy int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Netpy are points the user receives while staying online',
  CHANGE COLUMN WaguPoint wagu_point int(10) unsigned NOT NULL DEFAULT '0' COMMENT '',
  CHANGE COLUMN IP ip varchar(16) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL DEFAULT '0.0.0.0' COMMENT 'the last IP in the char',
  CHANGE COLUMN AirState air_state tinyint(1) unsigned NOT NULL DEFAULT '0' COMMENT '0 = off and 1 = on',
  CHANGE COLUMN InvisibleCostume invisible_costume tinyint(1) NOT NULL DEFAULT '0' COMMENT '0 = false 1 = true',
  CHANGE COLUMN PlayTime play_time bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT 'play time in seconds',
  CHANGE COLUMN SuperiorEffectType superior_effect_type tinyint(3) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN CreateTime create_time bigint(15) unsigned NOT NULL COMMENT 'time(0)',
  CHANGE COLUMN IsOnline is_online tinyint(1) unsigned NOT NULL DEFAULT '0';

ALTER TABLE dojos
  CHANGE COLUMN GuildId guild_id int(10) unsigned NOT NULL,
  CHANGE COLUMN DojoTblidx dojo_tblidx int(10) unsigned NOT NULL,
  CHANGE COLUMN Level level tinyint(1) unsigned NOT NULL DEFAULT '1',
  CHANGE COLUMN PeaceStatus peace_status tinyint(1) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN PeacePoints peace_points int(15) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN GuildName guild_name varchar(16) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL,
  CHANGE COLUMN LeaderName leader_name varchar(16) CHARACTER SET latin1 COLLATE latin1_swedish_ci DEFAULT NULL,
  CHANGE COLUMN Notice notice varchar(64) CHARACTER SET latin1 COLLATE latin1_swedish_ci DEFAULT NULL,
  CHANGE COLUMN ChallengeGuildId challenge_guild_id int(10) unsigned NOT NULL DEFAULT '4294967295',
  CHANGE COLUMN SeedCharName seed_char_name varchar(16) CHARACTER SET latin1 COLLATE latin1_swedish_ci DEFAULT NULL;

ALTER TABLE friendlist
  CHANGE COLUMN user_id char_id int(10) unsigned NOT NULL,
  CHANGE COLUMN friend_id friend_char_id int(10) unsigned NOT NULL;

ALTER TABLE guilds
  CHANGE COLUMN GuildID id int(10) unsigned NOT NULL,
  CHANGE COLUMN GuildName guild_name varchar(16) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL,
  CHANGE COLUMN GuildMaster master_char_id int(10) unsigned NOT NULL,
  CHANGE COLUMN GuildSecondMaster second_master_char_id int(10) unsigned NOT NULL DEFAULT '4294967295',
  CHANGE COLUMN GuildSecondMaster2 second_master_2_char_id int(10) unsigned NOT NULL DEFAULT '4294967295',
  CHANGE COLUMN GuildSecondMaster3 second_master_3_char_id int(10) unsigned NOT NULL DEFAULT '4294967295',
  CHANGE COLUMN GuildSecondMaster4 second_master_4_char_id int(10) unsigned NOT NULL DEFAULT '4294967295',
  CHANGE COLUMN GuildReputation guild_reputation int(10) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN GuildPointEver guild_point_ever int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'max guild points ever received',
  CHANGE COLUMN FunctionFlag function_flag bigint(15) unsigned NOT NULL DEFAULT '7',
  CHANGE COLUMN GuildDisbandTime guild_disband_time int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN MarkInColor mark_in_color tinyint(3) unsigned NOT NULL DEFAULT '255',
  CHANGE COLUMN MarkInLine mark_in_line tinyint(3) unsigned NOT NULL DEFAULT '255',
  CHANGE COLUMN MarkMain mark_main tinyint(3) unsigned NOT NULL DEFAULT '255',
  CHANGE COLUMN MarkMainColor mark_main_color tinyint(3) unsigned NOT NULL DEFAULT '255',
  CHANGE COLUMN MarkOutColor mark_out_color tinyint(3) unsigned NOT NULL DEFAULT '255',
  CHANGE COLUMN MarkOutLine mark_out_line tinyint(3) unsigned NOT NULL DEFAULT '255',
  CHANGE COLUMN NoticeBy notice_by varchar(16) CHARACTER SET latin1 COLLATE latin1_swedish_ci DEFAULT NULL,
  CHANGE COLUMN GuildNotice guild_notice varchar(257) CHARACTER SET latin1 COLLATE latin1_swedish_ci DEFAULT NULL,
  CHANGE COLUMN DojoColor dojo_color tinyint(3) unsigned NOT NULL DEFAULT '255',
  CHANGE COLUMN GuildColor guild_color tinyint(3) unsigned NOT NULL DEFAULT '255',
  CHANGE COLUMN DogiType dogi_type tinyint(3) unsigned NOT NULL DEFAULT '255',
  CHANGE COLUMN Zeni zeni int(10) unsigned DEFAULT '0' COMMENT 'Zeni inside guild bank';

ALTER TABLE guild_members
  CHANGE COLUMN GuildID guild_id int(14) unsigned NOT NULL,
  CHANGE COLUMN CharID char_id int(14) unsigned NOT NULL;

ALTER TABLE hoipoi_recipe
  CHANGE COLUMN CharID char_id int(10) unsigned NOT NULL,
  CHANGE COLUMN RecipeTblidx recipe_tblidx int(10) unsigned NOT NULL,
  CHANGE COLUMN RecipeType recipe_type tinyint(3) unsigned NOT NULL DEFAULT '255';

ALTER TABLE htb_skills
  CHANGE COLUMN owner_id char_id int(10) unsigned NOT NULL,
  CHANGE COLUMN SlotID slot_id smallint(3) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN TimeRemaining time_remaining int(10) unsigned NOT NULL DEFAULT '0';

ALTER TABLE items
  CHANGE COLUMN owner_id char_id int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'character id',
  CHANGE COLUMN NeedToIdentify need_to_identify tinyint(1) NOT NULL DEFAULT '0',
  CHANGE COLUMN BattleAttribute battle_attribute tinyint(1) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN Maker maker varchar(16) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL DEFAULT '',
  CHANGE COLUMN OptionTblidx option_tblidx int(10) unsigned NOT NULL DEFAULT '4294967295' COMMENT 'item_option_data tblidx',
  CHANGE COLUMN OptionTblidx2 option_tblidx_2 int(10) unsigned NOT NULL DEFAULT '4294967295' COMMENT 'item_option_data tblidx',
  CHANGE COLUMN OptionRandomId option_random_id smallint(5) unsigned NOT NULL DEFAULT '65535' COMMENT 'item_enchant tblidx',
  CHANGE COLUMN OptionRandomVal option_random_val int(10) unsigned NOT NULL DEFAULT '4294967295',
  CHANGE COLUMN OptionRandomId2 option_random_id_2 smallint(5) unsigned NOT NULL DEFAULT '65535',
  CHANGE COLUMN OptionRandomVal2 option_random_val_2 int(10) unsigned NOT NULL DEFAULT '4294967295',
  CHANGE COLUMN OptionRandomId3 option_random_id_3 smallint(5) unsigned NOT NULL DEFAULT '65535',
  CHANGE COLUMN OptionRandomVal3 option_random_val_3 int(10) unsigned NOT NULL DEFAULT '4294967295',
  CHANGE COLUMN OptionRandomId4 option_random_id_4 smallint(5) unsigned NOT NULL DEFAULT '65535',
  CHANGE COLUMN OptionRandomVal4 option_random_val_4 int(10) unsigned NOT NULL DEFAULT '4294967295',
  CHANGE COLUMN OptionRandomId5 option_random_id_5 smallint(5) unsigned NOT NULL DEFAULT '65535',
  CHANGE COLUMN OptionRandomVal5 option_random_val_5 int(10) unsigned NOT NULL DEFAULT '4294967295',
  CHANGE COLUMN OptionRandomId6 option_random_id_6 smallint(5) unsigned NOT NULL DEFAULT '65535',
  CHANGE COLUMN OptionRandomVal6 option_random_val_6 int(10) unsigned NOT NULL DEFAULT '4294967295',
  CHANGE COLUMN OptionRandomId7 option_random_id_7 smallint(5) unsigned NOT NULL DEFAULT '65535',
  CHANGE COLUMN OptionRandomVal7 option_random_val_7 int(10) unsigned NOT NULL DEFAULT '4294967295',
  CHANGE COLUMN OptionRandomId8 option_random_id_8 smallint(5) unsigned NOT NULL DEFAULT '65535',
  CHANGE COLUMN OptionRandomVal8 option_random_val_8 int(10) unsigned NOT NULL DEFAULT '4294967295',
  CHANGE COLUMN UseStartTime use_start_time bigint(15) unsigned NOT NULL DEFAULT '0' COMMENT 'max duration time (3600 = 60 minutes)',
  CHANGE COLUMN UseEndTime use_end_time bigint(15) unsigned NOT NULL DEFAULT '0' COMMENT 'current duration time (3600 = 60 minutes)',
  CHANGE COLUMN RestrictState restrict_state tinyint(3) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN DurationType duration_type tinyint(3) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN AccountID account_id int(10) unsigned DEFAULT '0',
  CHANGE COLUMN GuildID guild_id int(10) unsigned DEFAULT '0';

ALTER TABLE items_cd
  CHANGE COLUMN CharID char_id int(10) unsigned NOT NULL,
  CHANGE COLUMN GroupIndex group_index tinyint(2) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN CoolTime cool_time int(10) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN TimeRemaining time_remaining int(10) unsigned NOT NULL DEFAULT '0';

ALTER TABLE mail
  CHANGE COLUMN CharID char_id int(10) unsigned NOT NULL,
  CHANGE COLUMN SenderType sender_type tinyint(1) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN MailType mail_type tinyint(1) unsigned NOT NULL DEFAULT '1',
  CHANGE COLUMN TextSize text_size tinyint(3) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN Text text varchar(127) CHARACTER SET utf8 COLLATE utf8_general_ci DEFAULT NULL,
  CHANGE COLUMN Zenny zenny int(10) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN itemId item_id bigint(20) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN TargetName target_name varchar(16) CHARACTER SET latin1 COLLATE latin1_swedish_ci DEFAULT NULL,
  CHANGE COLUMN FromName from_name varchar(16) CHARACTER SET latin1 COLLATE latin1_swedish_ci DEFAULT NULL,
  CHANGE COLUMN IsAccept is_accept tinyint(1) NOT NULL DEFAULT '0',
  CHANGE COLUMN IsLock is_lock tinyint(1) NOT NULL DEFAULT '0',
  CHANGE COLUMN IsRead is_read tinyint(1) NOT NULL DEFAULT '0',
  CHANGE COLUMN CreateTime create_time bigint(20) unsigned DEFAULT NULL,
  CHANGE COLUMN EndTime end_time bigint(20) unsigned DEFAULT NULL,
  CHANGE COLUMN RemainDay remain_day tinyint(2) unsigned NOT NULL DEFAULT '1';

ALTER TABLE mascots
  CHANGE COLUMN CharID char_id int(15) unsigned NOT NULL,
  CHANGE COLUMN SlotID slot_id tinyint(3) unsigned NOT NULL DEFAULT '255' COMMENT 'index',
  CHANGE COLUMN MascotTblidx mascot_tblidx int(10) unsigned NOT NULL,
  CHANGE COLUMN CurVP cur_vp int(10) unsigned NOT NULL DEFAULT '100',
  CHANGE COLUMN MaxVP max_vp int(10) unsigned NOT NULL DEFAULT '100',
  CHANGE COLUMN CurExp cur_exp int(10) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN skillTblidx0 skill_tblidx_0 int(10) unsigned NOT NULL DEFAULT '4294967295',
  CHANGE COLUMN skillTblidx1 skill_tblidx_1 int(10) unsigned NOT NULL DEFAULT '4294967295',
  CHANGE COLUMN skillTblidx2 skill_tblidx_2 int(10) unsigned NOT NULL DEFAULT '4294967295',
  CHANGE COLUMN skillTblidx3 skill_tblidx_3 int(10) unsigned NOT NULL DEFAULT '4294967295';

ALTER TABLE portals
  CHANGE COLUMN CharID char_id int(10) unsigned NOT NULL;

ALTER TABLE questitems
  CHANGE COLUMN CharID char_id int(10) unsigned NOT NULL;

ALTER TABLE quests
  CHANGE COLUMN CharID char_id int(10) unsigned NOT NULL,
  CHANGE COLUMN QuestID quest_id int(10) unsigned NOT NULL,
  CHANGE COLUMN tcQuestInfo tc_quest_info tinyint(3) unsigned NOT NULL DEFAULT '1',
  CHANGE COLUMN taQuestInfo ta_quest_info tinyint(3) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN tgExcCGroup tg_exc_c_group tinyint(3) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN tcPreId tc_pre_id tinyint(3) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN tcCurId tc_cur_id tinyint(3) unsigned NOT NULL DEFAULT '255',
  CHANGE COLUMN tcId tc_id tinyint(3) unsigned NOT NULL DEFAULT '255',
  CHANGE COLUMN taId ta_id tinyint(3) unsigned NOT NULL DEFAULT '255',
  CHANGE COLUMN evtUserData evt_user_data int(10) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN evtUserData2 evt_user_data_2 int(10) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN evtUserData3 evt_user_data_3 int(10) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN evtUserData4 evt_user_data_4 int(10) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN tcTimeInfo tc_time_info tinyint(3) unsigned NOT NULL DEFAULT '255',
  CHANGE COLUMN taTimeInfo ta_time_info tinyint(3) unsigned NOT NULL DEFAULT '255',
  CHANGE COLUMN TimeLeft time_left int(10) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN QState q_state smallint(5) unsigned NOT NULL DEFAULT '0';

ALTER TABLE quickslot
  CHANGE COLUMN CharID char_id int(10) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN Tblidx tblidx int(10) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN Slot slot int(10) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN Type type tinyint(3) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN Item item_id bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT 'item unique id';

ALTER TABLE quick_teleport
  CHANGE COLUMN CharID char_id int(10) unsigned NOT NULL,
  CHANGE COLUMN SlotNum slot_num tinyint(3) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN WorldTblidx world_tblidx int(10) unsigned NOT NULL,
  CHANGE COLUMN LocX loc_x float(11,6) NOT NULL,
  CHANGE COLUMN LocY loc_y float(11,6) NOT NULL,
  CHANGE COLUMN LocZ loc_z float(11,6) NOT NULL,
  CHANGE COLUMN MapNameTblidx map_name_tblidx int(10) unsigned NOT NULL;

ALTER TABLE rank_battle
  CHANGE COLUMN CharID char_id int(10) unsigned NOT NULL,
  CHANGE COLUMN Win win int(10) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN Draw draw int(10) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN Lose lose int(10) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN StraightKOWin straight_ko_win int(10) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN MaxStraightKOWin max_straight_ko_win int(10) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN MaxStraightWin max_straight_win int(10) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN StraightWin straight_win int(10) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN Points points float(10,0) NOT NULL DEFAULT '0';

ALTER TABLE skills
  CHANGE COLUMN owner_id char_id int(10) unsigned NOT NULL,
  CHANGE COLUMN RpBonusAuto rp_bonus_auto tinyint(1) NOT NULL DEFAULT '0',
  CHANGE COLUMN RpBonusType rp_bonus_type tinyint(3) unsigned NOT NULL DEFAULT '255',
  CHANGE COLUMN SlotID slot_id tinyint(3) unsigned NOT NULL DEFAULT '0' COMMENT 'skillIndex',
  CHANGE COLUMN TimeRemaining time_remaining int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Skill CD Time',
  CHANGE COLUMN Exp exp int(10) unsigned NOT NULL DEFAULT '0';

ALTER TABLE titles
  CHANGE COLUMN CharID char_id int(10) unsigned NOT NULL,
  CHANGE COLUMN TitleTblidx title_tblidx int(10) unsigned NOT NULL;

ALTER TABLE warfog
  CHANGE COLUMN CharID char_id int(10) unsigned NOT NULL,
  CHANGE COLUMN WarFog war_fog int(10) unsigned NOT NULL;

-- ---------------------------------------------------------------------
-- dbo_log
-- ---------------------------------------------------------------------
USE dbo_log;

ALTER TABLE auctionhouse_log
  CHANGE COLUMN Seller seller_char_id int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN Buyer buyer_char_id int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN Price price int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemTblidx item_tblidx int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemID item_id bigint(30) unsigned DEFAULT NULL;

ALTER TABLE auth_login_log
  CHANGE COLUMN AccountID account_id int(15) unsigned NOT NULL,
  CHANGE COLUMN IP ip varchar(255) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL;

ALTER TABLE budokai
  CHANGE COLUMN SeasonCount season_count int(6) unsigned NOT NULL DEFAULT '0' COMMENT 'Amount of budokais. 0 = 1, 10 = 11 ...',
  CHANGE COLUMN DefaultOpenTime default_open_time int(15) unsigned DEFAULT '0',
  CHANGE COLUMN RankPointInitialized rank_point_initialized bit(1) DEFAULT b'0',
  CHANGE COLUMN StateData_State state_data_state tinyint(3) unsigned DEFAULT '0' COMMENT 'Budokai State',
  CHANGE COLUMN StateData_NextStepTime state_data_next_step_time int(15) unsigned DEFAULT '0' COMMENT 'budokai next step time',
  CHANGE COLUMN IndividualStateData_State individual_state_data_state tinyint(3) unsigned DEFAULT '0' COMMENT 'Solo match state',
  CHANGE COLUMN IndividualStateData_NextStepTime individual_state_data_next_step_time int(15) unsigned DEFAULT '0' COMMENT 'solo match next step time',
  CHANGE COLUMN TeamStateData_State team_state_data_state tinyint(3) unsigned DEFAULT '0' COMMENT 'team match state',
  CHANGE COLUMN TeamStateData_NextStepTime team_state_data_next_step_time int(15) unsigned DEFAULT '0' COMMENT 'Team match next state time';

ALTER TABLE budokai_winners
  CHANGE COLUMN BudokaiNumber budokai_number int(10) unsigned DEFAULT '0',
  CHANGE COLUMN Type type tinyint(1) DEFAULT '0' COMMENT '0 = junior, 1 = adult',
  CHANGE COLUMN MatchType match_type tinyint(1) DEFAULT '0' COMMENT '0 = individual, 1 = team',
  CHANGE COLUMN WinnerCharID1 winner_char_id_1 int(15) unsigned DEFAULT '0',
  CHANGE COLUMN WinnerCharID2 winner_char_id_2 int(15) unsigned DEFAULT '0',
  CHANGE COLUMN WinnerCharID3 winner_char_id_3 int(15) unsigned DEFAULT '0',
  CHANGE COLUMN WinnerCharID4 winner_char_id_4 int(15) unsigned DEFAULT '0',
  CHANGE COLUMN WinnerCharID5 winner_char_id_5 int(15) unsigned DEFAULT '0',
  CHANGE COLUMN Date date timestamp NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP;

ALTER TABLE change_char_name
  CHANGE COLUMN CharID char_id int(15) unsigned NOT NULL,
  CHANGE COLUMN Name name varchar(17) CHARACTER SET utf8 COLLATE utf8_general_ci DEFAULT NULL,
  CHANGE COLUMN newName new_name varchar(17) CHARACTER SET utf8 COLLATE utf8_general_ci DEFAULT NULL;

ALTER TABLE character_delete_log
  CHANGE COLUMN AccountID account_id int(15) unsigned NOT NULL,
  CHANGE COLUMN CharID char_id int(15) NOT NULL;

ALTER TABLE dynamic_field_count
  CHANGE COLUMN serverIndex server_index tinyint(3) unsigned NOT NULL DEFAULT '0';

ALTER TABLE founder_log
  CHANGE COLUMN Username username varchar(20) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL;

ALTER TABLE gm_log
  CHANGE COLUMN CharID char_id int(15) unsigned NOT NULL,
  CHANGE COLUMN LogType log_type int(3) unsigned DEFAULT NULL,
  CHANGE COLUMN String string text CHARACTER SET utf8 COLLATE utf8_general_ci;

ALTER TABLE guild_name_change_log
  CHANGE COLUMN `key` id int(10) unsigned NOT NULL AUTO_INCREMENT,
  CHANGE COLUMN GuildID guild_id int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN CurrentName current_name varchar(25) CHARACTER SET utf8 COLLATE utf8_general_ci DEFAULT NULL,
  CHANGE COLUMN NewName new_name varchar(25) CHARACTER SET utf8 COLLATE utf8_general_ci DEFAULT NULL;

ALTER TABLE item_upgrade_log
  CHANGE COLUMN charId char_id int(15) unsigned NOT NULL,
  CHANGE COLUMN IsSuccess is_success bit(1) NOT NULL,
  CHANGE COLUMN itemId item_id bigint(20) unsigned NOT NULL,
  CHANGE COLUMN itemTblidx item_tblidx int(15) unsigned NOT NULL,
  CHANGE COLUMN newGrade new_grade int(3) NOT NULL,
  CHANGE COLUMN StoneItemId stone_item_id bigint(20) unsigned NOT NULL,
  CHANGE COLUMN StoneItemTblidx stone_item_tblidx int(15) unsigned NOT NULL,
  CHANGE COLUMN CoreItemUse core_item_use bit(1) NOT NULL,
  CHANGE COLUMN coreItemId core_item_id bigint(20) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN coreItemTblidx core_item_tblidx int(15) unsigned NOT NULL;

ALTER TABLE mail_deleted
  CHANGE COLUMN CharID char_id int(10) unsigned NOT NULL,
  CHANGE COLUMN SenderType sender_type tinyint(1) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN MailType mail_type tinyint(1) unsigned NOT NULL DEFAULT '1',
  CHANGE COLUMN TextSize text_size tinyint(3) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN Text text varchar(127) CHARACTER SET utf8 COLLATE utf8_general_ci DEFAULT NULL,
  CHANGE COLUMN Zenny zenny int(10) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN itemId item_id bigint(20) unsigned NOT NULL DEFAULT '0',
  CHANGE COLUMN TargetName target_name varchar(16) CHARACTER SET latin1 COLLATE latin1_swedish_ci DEFAULT NULL,
  CHANGE COLUMN FromName from_name varchar(16) CHARACTER SET latin1 COLLATE latin1_swedish_ci DEFAULT NULL,
  CHANGE COLUMN IsAccept is_accept tinyint(1) NOT NULL DEFAULT '0',
  CHANGE COLUMN IsLock is_lock tinyint(1) NOT NULL DEFAULT '0',
  CHANGE COLUMN IsRead is_read tinyint(1) NOT NULL DEFAULT '0',
  CHANGE COLUMN CreateTime create_time bigint(20) unsigned DEFAULT NULL,
  CHANGE COLUMN EndTime end_time bigint(20) unsigned DEFAULT NULL,
  CHANGE COLUMN RemainDay remain_day tinyint(2) unsigned NOT NULL DEFAULT '1';

ALTER TABLE mute_log
  CHANGE COLUMN CharID char_id int(15) unsigned NOT NULL,
  CHANGE COLUMN GmAccountID gm_account_id int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN DurationInMinutes duration_in_minutes int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN Reason reason varchar(255) CHARACTER SET latin1 COLLATE latin1_swedish_ci DEFAULT NULL,
  CHANGE COLUMN muteUntil mute_until bigint(30) unsigned DEFAULT NULL;

ALTER TABLE privateshoplogs
  CHANGE COLUMN `key` id int(15) unsigned NOT NULL AUTO_INCREMENT,
  CHANGE COLUMN SellerCharID seller_char_id int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN BuyerCharID buyer_char_id int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN Zeni zeni int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemCount item_count int(3) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemID_1 item_id_1 bigint(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemTblidx_1 item_tblidx_1 int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemID_2 item_id_2 bigint(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemTblidx_2 item_tblidx_2 int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemID_3 item_id_3 bigint(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemTblidx_3 item_tblidx_3 int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemID_4 item_id_4 bigint(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemTblidx_4 item_tblidx_4 int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemID_5 item_id_5 bigint(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemTblidx_5 item_tblidx_5 int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemID_6 item_id_6 bigint(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemTblidx_6 item_tblidx_6 int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemID_7 item_id_7 bigint(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemTblidx_7 item_tblidx_7 int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemID_8 item_id_8 bigint(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemTblidx_8 item_tblidx_8 int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemID_9 item_id_9 bigint(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemTblidx_9 item_tblidx_9 int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemID_10 item_id_10 bigint(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemTblidx_10 item_tblidx_10 int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemID_11 item_id_11 bigint(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemTblidx_11 item_tblidx_11 int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemID_12 item_id_12 bigint(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemTblidx_12 item_tblidx_12 int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN HasIssues has_issues int(1) DEFAULT NULL,
  CHANGE COLUMN IssueReason issue_reason varchar(512) CHARACTER SET utf8 COLLATE utf8_general_ci DEFAULT NULL;

ALTER TABLE slot_machine_log
  CHANGE COLUMN accountid account_id int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN charid char_id int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN extractCount extract_count int(3) unsigned DEFAULT NULL,
  CHANGE COLUMN currentPoints current_points int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN newPoints new_points int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ProductId1 product_id_1 int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ProductId2 product_id_2 int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ProductId3 product_id_3 int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ProductId4 product_id_4 int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ProductId5 product_id_5 int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ProductId6 product_id_6 int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ProductId7 product_id_7 int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ProductId8 product_id_8 int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ProductId9 product_id_9 int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ProductId10 product_id_10 int(15) unsigned DEFAULT NULL;

ALTER TABLE tradelogs
  CHANGE COLUMN `key` id int(15) unsigned NOT NULL AUTO_INCREMENT,
  CHANGE COLUMN CharID char_id int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN TargetCharID target_char_id int(15) DEFAULT NULL,
  CHANGE COLUMN Zeni zeni int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemCount item_count int(3) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemID_1 item_id_1 bigint(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemTblidx_1 item_tblidx_1 int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemID_2 item_id_2 bigint(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemTblidx_2 item_tblidx_2 int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemID_3 item_id_3 bigint(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemTblidx_3 item_tblidx_3 int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemID_4 item_id_4 bigint(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemTblidx_4 item_tblidx_4 int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemID_5 item_id_5 bigint(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemTblidx_5 item_tblidx_5 int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemID_6 item_id_6 bigint(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemTblidx_6 item_tblidx_6 int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemID_7 item_id_7 bigint(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemTblidx_7 item_tblidx_7 int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemID_8 item_id_8 bigint(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemTblidx_8 item_tblidx_8 int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemID_9 item_id_9 bigint(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemTblidx_9 item_tblidx_9 int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemID_10 item_id_10 bigint(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemTblidx_10 item_tblidx_10 int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemID_11 item_id_11 bigint(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemTblidx_11 item_tblidx_11 int(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemID_12 item_id_12 bigint(15) unsigned DEFAULT NULL,
  CHANGE COLUMN ItemTblidx_12 item_tblidx_12 int(15) unsigned DEFAULT NULL;
