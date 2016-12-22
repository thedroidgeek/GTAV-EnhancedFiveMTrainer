﻿/*
 * Enhanced FiveM Trainer by 5-H
 * (C) Sami Alaoui (5-H) 2015
 * https://facebook.com/TheDroidGeek
 */

 /*
  * The trainer menu code was based on the Enhanced Native Trainer project.
  * https://github.com/gtav-ent/GTAV-EnhancedNativeTrainer
  * (C) Rob Pridham and fellow contributors 2015
  *
  * The player head display code was took from the following commit:
  * http://tohjo.eu/UnTraDe/citizenmp/commit/1b2549f7d9c6bda8b808d67dd70b49686fbc9f66
  */


#define TRAINER_VERSION "1.8"
#define MAX_PLAYERS 32

#include "io.h"
#include "config_io.h"
#include "menu_functions.h"
#include "script.h"


struct playerinfo {
	std::string name;
	Ped ped;
	int blip;
	unsigned int head;
};

Player playerId;

playerinfo playerdb[MAX_PLAYERS];

std::vector<int> playerIdForMenuEntries;


Vehicle ownedveh;
bool ownedvehlocked = false;

bool playerWasDisconnected = true;

bool featurePlayerBlips, featurePlayerHeadDisplay, featurePlayerNotifications, featureShowVoiceChatSpeaker, featureShowDeathCutscene;


bool onconfirm_playerteleport_menu(MenuItem<int> choice);

bool onconfirm_animation_menu(MenuItem<int> choice);

bool onconfirm_misc_menu(MenuItem<int> choice);

bool onconfirm_settings_menu(MenuItem<int> choice);

bool onconfirm_vehicle_menu(MenuItem<int> choice);

bool onconfirm_exit_menu(MenuItem<int> choice);


Hash $(char* string) { return GAMEPLAY::GET_HASH_KEY(string); }


std::string killActionFromWeaponHash(Hash weaponHash)
{
	if (weaponHash != NULL)
	{
		if (weaponHash == $("WEAPON_RUN_OVER_BY_CAR") ||
			weaponHash == $("WEAPON_RAMMED_BY_CAR"))
			return "flattened";

		if (weaponHash == $("WEAPON_CROWBAR") ||
			weaponHash == $("WEAPON_BAT") ||
			weaponHash == $("WEAPON_HAMMER") ||
			weaponHash == $("WEAPON_GOLFCLUB") ||
			weaponHash == $("WEAPON_NIGHTSTICK") ||
			weaponHash == $("WEAPON_KNUCKLE"))
			return "whacked";

		if (weaponHash == $("WEAPON_DAGGER") ||
			weaponHash == $("WEAPON_KNIFE"))
			return "stabbed";

		if (weaponHash == $("WEAPON_SNSPISTOL") ||
			weaponHash == $("WEAPON_HEAVYPISTOL") ||
			weaponHash == $("WEAPON_VINTAGEPISTOL") ||
			weaponHash == $("WEAPON_PISTOL") ||
			weaponHash == $("WEAPON_APPISTOL") ||
			weaponHash == $("WEAPON_COMBATPISTOL") ||
			weaponHash == $("WEAPON_SNSPISTOL"))
			return "shot";

		if (weaponHash == $("WEAPON_GRENADELAUNCHER") ||
			weaponHash == $("WEAPON_HOMINGLAUNCHER") ||
			weaponHash == $("WEAPON_STICKYBOMB") ||
			weaponHash == $("WEAPON_PROXMINE") ||
			weaponHash == $("WEAPON_RPG") ||
			weaponHash == $("WEAPON_EXPLOSION") ||
			weaponHash == $("VEHICLE_WEAPON_TANK"))
			return "bombed";
		
		if (weaponHash == $("WEAPON_MICROSMG") ||
			weaponHash == $("WEAPON_SMG") ||
			weaponHash == $("WEAPON_ASSAULTSMG") ||
			weaponHash == $("WEAPON_MG") ||
			weaponHash == $("WEAPON_COMBATMG") ||
			weaponHash == $("WEAPON_COMBATPDW") ||
			weaponHash == $("WEAPON_MINIGUN"))
			return "sprayed";

		if (weaponHash == $("WEAPON_ASSAULTRIFLE") ||
			weaponHash == $("WEAPON_CARBINERIFLE") ||
			weaponHash == $("WEAPON_ADVANCEDRIFLE") ||
			weaponHash == $("WEAPON_BULLPUPRIFLE") ||
			weaponHash == $("WEAPON_SPECIALCARBINE") ||
			weaponHash == $("WEAPON_GUSENBERG"))
			return "rifled";

		if (weaponHash == $("WEAPON_MARKSMANRIFLE") ||
			weaponHash == $("WEAPON_SNIPERRIFLE") ||
			weaponHash == $("WEAPON_HEAVYSNIPER") ||
			weaponHash == $("WEAPON_ASSAULTSNIPER") ||
			weaponHash == $("WEAPON_REMOTESNIPER"))
			return "sniped";

		if (weaponHash == $("WEAPON_BULLPUPSHOTGUN") ||
			weaponHash == $("WEAPON_ASSAULTSHOTGUN") ||
			weaponHash == $("WEAPON_PUMPSHOTGUN") ||
			weaponHash == $("WEAPON_HEAVYSHOTGUN") ||
			weaponHash == $("WEAPON_SAWNOFFSHOTGUN"))
			return "shotgunned";

		if (weaponHash == $("WEAPON_HATCHET") ||
			weaponHash == $("WEAPON_MACHETE"))
			return "eviscerated";

		if (weaponHash == $("WEAPON_MOLOTOV"))
			return "torched";
	}
	return "murdered";
}


//=============================
// FUNCTION THAT UPDATES STUFF
//=============================

void updateStuff()
{
	if (NETWORK::NETWORK_IS_SESSION_STARTED())
	{
		if (playerWasDisconnected) {
			NETWORK::NETWORK_SET_FRIENDLY_FIRE_OPTION(1);
			playerId = PLAYER::PLAYER_ID();
			PED::SET_CAN_ATTACK_FRIENDLY(playerId, 1, 1);
		}

		std::string voice_status_msg = "Currently Talking:";
		bool isVoiceChatRunning = false;

		for (int i = 0; i < MAX_PLAYERS; i++)
		{
			if (trainer_switch_pressed()) { // To increase chances of trainer switch key capture,
				set_menu_showing(true);     // since this is a consuming function.
				return;
			}
			if (featureShowVoiceChatSpeaker && isVoiceChatRunning)
				update_status_text();

			if (featurePlayerNotifications || featureShowDeathCutscene)
			{
				Ped playerPed = PLAYER::PLAYER_PED_ID();
				if (ENTITY::IS_ENTITY_DEAD(playerPed) && ENTITY::DOES_ENTITY_EXIST(playerPed))
				{
					if (featurePlayerNotifications)
					{
						std::string msg = "You died.";
						Hash weaponHash;
						Entity e = NETWORK::NETWORK_GET_ENTITY_KILLER_OF_PLAYER(playerId, &weaponHash);
						if (PED::IS_PED_A_PLAYER(e)) {
							Player killer = NETWORK::_0x6C0E2E0125610278(e); // _NETWORK_GET_PLAYER_FROM_PED
							std::string kname = PLAYER::GET_PLAYER_NAME(killer);
							if (kname != "") {
								if (kname == PLAYER::GET_PLAYER_NAME(playerId)) {
									msg = "You commited suicide.";
								}
								else {
									msg = "<C>" + kname + "</C> " + killActionFromWeaponHash(weaponHash) + " you.";
								}
							}
						}
						show_notification((char*)msg.c_str());
					}

					if (featureShowDeathCutscene)
					{
						AUDIO::PLAY_SOUND_FRONTEND(-1, "Bed", "WastedSounds", 1);
						GRAPHICS::_START_SCREEN_EFFECT("DeathFailOut", 0, 0);

						int scaleform = GRAPHICS::REQUEST_SCALEFORM_MOVIE("MP_BIG_MESSAGE_FREEMODE");

						while (!GRAPHICS::HAS_SCALEFORM_MOVIE_LOADED(scaleform))
							WAIT(0);

						GRAPHICS::_PUSH_SCALEFORM_MOVIE_FUNCTION(scaleform, "SHOW_SHARD_WASTED_MP_MESSAGE");
						GRAPHICS::_BEGIN_TEXT_COMPONENT("STRING");
						UI::_ADD_TEXT_COMPONENT_ITEM_STRING("RESPAWN_W");
						GRAPHICS::_END_TEXT_COMPONENT();
						GRAPHICS::_POP_SCALEFORM_MOVIE_FUNCTION_VOID();

						WAIT(1000);

						AUDIO::PLAY_SOUND_FRONTEND(-1, "TextHit", "WastedSounds", 1);
						while (ENTITY::IS_ENTITY_DEAD(PLAYER::PLAYER_PED_ID())) {
							GRAPHICS::DRAW_SCALEFORM_MOVIE_FULLSCREEN(scaleform, 255, 255, 255, 255);
							WAIT(0);
						}
						GRAPHICS::_STOP_SCREEN_EFFECT("DeathFailOut");
					}
				}
			}


			if (NETWORK::NETWORK_IS_PLAYER_CONNECTED(i))
			{
				std::string name = (char*)PLAYER::GET_PLAYER_NAME(i);

				if (featureShowVoiceChatSpeaker && NETWORK::NETWORK_IS_PLAYER_TALKING(i))
				{
					if (!isVoiceChatRunning)
						isVoiceChatRunning = true;
					voice_status_msg += "~b~~n~" + name;
				}

				if (i != playerId)
				{
					Ped pedId = PLAYER::GET_PLAYER_PED(i);
					unsigned int headDisplayId = UI::_0xBFEFE3321A3F5015(pedId, "", 0, 0, "", 0); // _CREATE_PED_HEAD_DISPLAY

					if (featurePlayerNotifications && ENTITY::IS_ENTITY_DEAD(pedId) && ENTITY::DOES_ENTITY_EXIST(pedId))
					{
						std::string msg = "<C>" + name + "</C> died.";
						Hash weaponHash;
						Entity e = NETWORK::NETWORK_GET_ENTITY_KILLER_OF_PLAYER(i, &weaponHash);
						if (PED::IS_PED_A_PLAYER(e)) {
							Player killer = NETWORK::_0x6C0E2E0125610278(e); // _NETWORK_GET_PLAYER_FROM_PED
							std::string kname = PLAYER::GET_PLAYER_NAME(killer);
							if (kname != "") {
								if (kname == name) {
									msg = "<C>" + name + "</C> commited suicide.";
								}
								else if (kname == PLAYER::GET_PLAYER_NAME(playerId)) {
									msg = "You " + killActionFromWeaponHash(weaponHash) + " <C>" + name + "</C>";
								}
								else {
									msg = "<C>" + kname + "</C> " + killActionFromWeaponHash(weaponHash) + " <C>" + name + "</C>";
								}
							}
						}
						show_notification((char*)msg.c_str());
					}

					if (UI::_0x4E929E7A5796FD26(headDisplayId))
					{
						playerdb[i].head = headDisplayId;
						if (featurePlayerHeadDisplay)
							UI::_0xDEA2B8283BAA3944(headDisplayId, (char*)name.c_str()); // _SET_HEAD_DISPLAY_STRING
						UI::_0x63BB75ABEDC1F6A0(headDisplayId, 0, 1); // _SET_HEAD_DISPLAY_FLAG
					}

					if (playerWasDisconnected || !UI::DOES_BLIP_EXIST(playerdb[i].blip))
					{
						if (featurePlayerBlips)
						{
							playerdb[i].blip = UI::ADD_BLIP_FOR_ENTITY(pedId);
							UI::SET_BLIP_COLOUR(playerdb[i].blip, 0);
							UI::SET_BLIP_SCALE(playerdb[i].blip, 0.8);
							UI::SET_BLIP_NAME_TO_PLAYER_NAME(playerdb[i].blip, i);
							UI::SET_BLIP_CATEGORY(playerdb[i].blip, 7);
							UI::_0x5FBCA48327B914DF(playerdb[i].blip, 1); // _SET_BLIP_SHOW_HEADING_INDICATOR
						}

						if (playerWasDisconnected || playerdb[i].name != name) // Making sure the player wasn't already here and only changed his ped (ex. skin change)
						{
							if (featurePlayerNotifications)
							{
								std::string msg = "~b~<C>" + name + "</C>~s~ joined.";
								show_notification((char*)msg.c_str());
							}

							PED::SET_CAN_ATTACK_FRIENDLY(i, 1, 1);
							playerdb[i].name = name;
						}
						playerdb[i].ped = pedId;
					}

					if (featurePlayerBlips) {

						int sprite = 1;

						if (PED::IS_PED_IN_ANY_VEHICLE(playerdb[i].ped, 0))
						{
							Vehicle v = PED::GET_VEHICLE_PED_IS_IN(playerdb[i].ped, false);

							switch (VEHICLE::GET_VEHICLE_CLASS(v)) {
							case 8:
							case 13: sprite = 226; break; //Bike
							case 14: sprite = 410; break; //Boat
							case 15: sprite = 422; break; //Helicopter
							case 16: sprite = 423; break; //Airplane
							case 19: sprite = 421; break; //Military
							default: sprite = 225; break; //Car
							}
						}

						if (UI::GET_BLIP_SPRITE(playerdb[i].blip) != sprite) {
							UI::SET_BLIP_SPRITE(playerdb[i].blip, sprite);
							UI::SET_BLIP_NAME_TO_PLAYER_NAME(playerdb[i].blip, i); // Blip name sometimes gets overriden by sprite name
						}
					}
				}
			}
			else if (playerdb[i].name != "")
			{
				if (featurePlayerNotifications)
				{
					std::string msg = "~b~<C>" + playerdb[i].name + "</C>~s~ left.";
					show_notification((char*)msg.c_str());
				}

				if (UI::_0x4E929E7A5796FD26(playerdb[i].head))
					UI::_0xDEA2B8283BAA3944(playerdb[i].head, ""); // _SET_HEAD_DISPLAY_STRING
				if (UI::DOES_BLIP_EXIST(playerdb[i].blip))
					UI::REMOVE_BLIP(&playerdb[i].blip);

				playerdb[i].name = "";
			}
		}
		playerWasDisconnected = false;

		if(isVoiceChatRunning)
			set_status_text(voice_status_msg);
	}
	else
	{
		playerWasDisconnected = true;

		if (trainer_switch_pressed())
			set_menu_showing(true);
	}
}


//=================
// MENU PROCESSING
//=================

int activeLineIndexPlayer = 0;

bool shouldRefreshPlayerMenu = true;

void process_player_menu(bool(*onConfirmation)(MenuItem<int> value), bool warningMsg)
{
	shouldRefreshPlayerMenu = false;

	if (NETWORK::NETWORK_IS_SESSION_STARTED())
	{
		std::vector<StandardOrToggleMenuDef> lines_v;

		playerIdForMenuEntries.clear();
		for (int i = 0; i < MAX_PLAYERS; i++)
		{
			if (playerdb[i].name != "")
			{
				std::string linetxt = "[ID: " + std::to_string(i) + "] " + playerdb[i].name;
				playerIdForMenuEntries.push_back(i);
				lines_v.push_back({ linetxt, NULL, NULL, true });
			}
		}

		const int lineCount = (int)lines_v.size();

		if (lineCount == 0)
		{
			show_notification("Do you feel forever alone ?");
			return;
		}

		std::string caption = std::to_string(lineCount) + " CONNECTED PLAYER" + (lineCount == 1 ? "" : "S");

		if (warningMsg)
			show_notification("Please don't abuse this feature and annoy other players.");

		bool result = draw_menu_from_struct_def(&lines_v[0], lineCount, &activeLineIndexPlayer, caption, onConfirmation);

		if (result)
			shouldRefreshPlayerMenu = true;
	}
	else
	{
		show_notification("Not in a network session.");
	}
}


int activeLineIndexAnim = 0;

void process_anim_menu()
{
	std::string caption = "ANIMATIONS";

	const int lineCount = 23;

	StandardOrToggleMenuDef lines[lineCount] = {
		{ "The Bird", NULL, NULL, true },
		{ "The Bird 2", NULL, NULL, true },
		{ "Up Yours", NULL, NULL, true },
		{ "Dock", NULL, NULL, true },
		{ "Jerk", NULL, NULL, true },
		{ "Knuckle Crunch", NULL, NULL, true },
		{ "Shush", NULL, NULL, true },
		{ "DJ", NULL, NULL, true },
		{ "Rock", NULL, NULL, true },
		{ "Air Guitar", NULL, NULL, true },
		{ "Wave", NULL, NULL, true },
		{ "Salute", NULL, NULL, true },
		{ "Thanks", NULL, NULL, true },
		{ "Thumbs Up", NULL, NULL, true },
		{ "Face Palm", NULL, NULL, true },
		{ "Nose Pick", NULL, NULL, true },
		{ "Slow Clap", NULL, NULL, true },
		{ "Photography", NULL, NULL, true },
		{ "Blow Kiss", NULL, NULL, true },
		{ "Jazz Hands", NULL, NULL, true },
		{ "Surrender", NULL, NULL, true },
		{ "Air Synth", NULL, NULL, true },
		{ "Air Shagging", NULL, NULL, true }
	};

	draw_menu_from_struct_def(lines, lineCount, &activeLineIndexAnim, caption, onconfirm_animation_menu);
}


int activeLineIndexMisc = 0;

void process_misc_menu()
{
	std::string caption = "MISCELLANEOUS";

	const int lineCount = 3;

	StandardOrToggleMenuDef lines[lineCount] = {
		{ "Basic Vehicle Management System", NULL, NULL, false },
		{ "Show Voice Chat Speaker", &featureShowVoiceChatSpeaker, NULL, true },
		{ "Show Death Cutscene", &featureShowDeathCutscene, NULL, true },
	};

	draw_menu_from_struct_def(lines, lineCount, &activeLineIndexMisc, caption, onconfirm_misc_menu);
}


int activeLineIndexVeh = 0;

void process_vehicle_menu()
{
	std::string caption = "VEHICLE MANAGEMENT";

	const int lineCount = 5;

	StandardOrToggleMenuDef lines[lineCount] = {
		{ "Set Current Vehicle As Owned", NULL, NULL, true },
		{ "Clear Owned Vehicle", NULL, NULL, true },
		{ "Lock Owned Vehicle", NULL, NULL, true },
		{ "Unlock Owned Vehicle", NULL, NULL, true },
		{ "Attempt To Retrieve Owned Vehicle", NULL, NULL, true }
	};

	draw_menu_from_struct_def(lines, lineCount, &activeLineIndexVeh, caption, onconfirm_vehicle_menu);
}


int activeLineIndexSettings = 0;

void process_settings_menu()
{
	std::string caption = "TRAINER SETTINGS";

	const int lineCount = 3;

	StandardOrToggleMenuDef lines[lineCount] = {
		{ "Player Blips", &featurePlayerBlips, NULL, true },
		{ "Player Head Display", &featurePlayerHeadDisplay, NULL, true },
		{ "Player Notifications", &featurePlayerNotifications, NULL, true }
	};

	draw_menu_from_struct_def(lines, lineCount, &activeLineIndexSettings, caption, onconfirm_settings_menu);
}

int activeLineIndexExitMenu;

void process_exit_menu()
{
	activeLineIndexExitMenu = 1;

	std::string caption = "Are you sure you want to exit ?";

	const int lineCount = 2;

	StandardOrToggleMenuDef lines[lineCount] = {
		{ "Yes, get me outta here", NULL, NULL, true },
		{ "No, let me play some", NULL, NULL, true }
	};

	draw_menu_from_struct_def(lines, lineCount, &activeLineIndexExitMenu, caption, onconfirm_exit_menu);
}


//====================
// MENU CONFIRMATIONS
//====================

bool onconfirm_playerteleport_menu(MenuItem<int> choice)
{
	Player targetId = playerIdForMenuEntries[activeLineIndexPlayer];
	playerinfo target = playerdb[targetId];

	if (!NETWORK::NETWORK_IS_PLAYER_CONNECTED(targetId))
		show_notification("Player has disconnected.");

	else if (ENTITY::DOES_ENTITY_EXIST(target.ped))
	{
		Entity us = PLAYER::PLAYER_PED_ID();
		if (PED::IS_PED_IN_ANY_VEHICLE(us, 0))
		{
			Vehicle v = PED::GET_VEHICLE_PED_IS_USING(us);
			if (VEHICLE::GET_PED_IN_VEHICLE_SEAT(v, -1) == us)
				us = v;
		}

		Vector3 targetpos = ENTITY::GET_ENTITY_COORDS(target.ped, 0);

		targetpos.x += 3.0; targetpos.z += 3.0;

		ENTITY::SET_ENTITY_COORDS_NO_OFFSET(us, targetpos.x, targetpos.y, targetpos.z, 0, 0, 1);

		if (target.name == "androidgeek")
			show_notification("Teleported to the creator of this trainer.");
		else
		{
			std::string msg = "Teleported to ~b~<C>" + target.name + "</C>";
			show_notification((char*)msg.c_str());
		}
	}

	return true;
}


bool onconfirm_animation_menu(MenuItem<int> choice)
{
	Ped playerPed = PLAYER::PLAYER_PED_ID();

	char* dict, * anim;

	switch (activeLineIndexAnim)
	{
	case 0:
		dict = "mp_player_intfinger"; anim = "mp_player_int_finger";
		break;
	case 1:
		dict = "mp_player_int_upperfinger"; anim = "mp_player_int_finger_01";
		break;
	case 2:
		dict = "mp_player_int_upperfinger"; anim = "mp_player_int_finger_02";
		break;
	case 3:
		dict = "anim@mp_player_intcelebrationmale@dock"; anim = "dock";
		break;
	case 4:
		dict = "mp_player_intwank"; anim = "mp_player_int_wank";
		break;
	case 5:
		dict = "anim@mp_player_intcelebrationmale@knuckle_crunch"; anim = "knuckle_crunch";
		break;
	case 6:
		dict = "anim@mp_player_intcelebrationmale@shush"; anim = "shush";
		break;
	case 7:
		dict = "anim@mp_player_intcelebrationmale@dj"; anim = "dj";
		break;
	case 8:
		dict = "mp_player_introck"; anim = "mp_player_int_rock";
		break;
	case 9:
		dict = "anim@mp_player_intcelebrationmale@air_guitar"; anim = "air_guitar";
		break;
	case 10:
		dict = "anim@mp_player_intcelebrationmale@wave"; anim = "wave";
		break;
	case 11:
		dict = "mp_player_intsalute"; anim = "mp_player_int_salute";
		break;
	case 12:
		dict = "mp_action"; anim = "thanks_male_06";
		break;
	case 13:
		dict = "anim@mp_player_intcelebrationmale@thumbs_up"; anim = "thumbs_up";
		break;
	case 14:
		dict = "anim@mp_player_intcelebrationmale@face_palm"; anim = "face_palm";
		break;
	case 15:
		dict = "anim@mp_player_intcelebrationmale@nose_pick"; anim = "nose_pick";
		break;
	case 16:
		dict = "anim@mp_player_intcelebrationfemale@slow_clap"; anim = "slow_clap";
		break;
	case 17:
		dict = "anim@mp_player_intcelebrationmale@photography"; anim = "photography";
		break;
	case 18:
		dict = "anim@mp_player_intcelebrationmale@blow_kiss"; anim = "blow_kiss";
		break;
	case 19:
		dict = "anim@mp_player_intcelebrationmale@jazz_hands"; anim = "jazz_hands";
		break;
	case 20:
		dict = "anim@mp_player_intcelebrationmale@surrender"; anim = "surrender";
		break;
	case 21:
		dict = "anim@mp_player_intcelebrationmale@air_synth"; anim = "air_synth";
		break;
	case 22:
		dict = "anim@mp_player_intcelebrationmale@air_shagging"; anim = "air_shagging";
		break;
	}

	STREAMING::REQUEST_ANIM_DICT(dict);
	while (!STREAMING::HAS_ANIM_DICT_LOADED(dict))
		WAIT(0);
	AI::TASK_PLAY_ANIM(playerPed, dict, anim, 1, -1, -1, 48, 0, 0, 0, 0);
	STREAMING::REMOVE_ANIM_DICT(dict);

	return false;
}


bool onconfirm_misc_menu(MenuItem<int> choice)
{
	switch (activeLineIndexMisc)
	{
	case 0:
		process_vehicle_menu();
		break;
	}
	return false;
}


bool onconfirm_vehicle_menu(MenuItem<int> choice)
{
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	switch (activeLineIndexVeh)
	{
	case 0:
		if (ownedveh != NULL && ENTITY::DOES_ENTITY_EXIST(ownedveh)) {
			show_notification("You already have an owned vehicle, please clear it first.");
		}
		else if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, 1))
		{
			ownedveh = PED::GET_VEHICLE_PED_IS_USING(playerPed);
			if (VEHICLE::GET_PED_IN_VEHICLE_SEAT(ownedveh, -1) == playerPed) {
				ENTITY::SET_ENTITY_AS_MISSION_ENTITY(ownedveh, 1, 1);
				AUDIO::SET_VEHICLE_RADIO_LOUD(ownedveh, 1);
				ownedvehlocked = false;
				show_notification("Your current vehicle has been set as owned.");
			}
			else {
				ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&ownedveh);
				show_notification("You can't own a vehicle you're not driving.");
			}
		}
		else
			show_notification("Not in a vehicle.");
		break;

	case 1:
		if (ownedveh != NULL && ENTITY::DOES_ENTITY_EXIST(ownedveh))
		{
			VEHICLE::SET_VEHICLE_DOORS_LOCKED_FOR_ALL_PLAYERS(ownedveh, 0);
			VEHICLE::SET_VEHICLE_ALARM(ownedveh, 0);
			ownedveh = NULL;
			show_notification("Cleared previously owned vehicle.");
		}
		else
			show_notification("Owned vehicle not found.");
		break;

	case 2:
		if (ownedveh != NULL && ENTITY::DOES_ENTITY_EXIST(ownedveh))
		{
			if (!ownedvehlocked)
			{
				VEHICLE::SET_VEHICLE_DOORS_LOCKED_FOR_ALL_PLAYERS(ownedveh, 1);
				VEHICLE::SET_VEHICLE_DOORS_LOCKED_FOR_PLAYER(ownedveh, playerId, 0);
				VEHICLE::SET_VEHICLE_ALARM(ownedveh, 1);
				ownedvehlocked = true;
				if (VEHICLE::IS_VEHICLE_SEAT_FREE(ownedveh, -1)) {
					if (VEHICLE::IS_VEHICLE_A_CONVERTIBLE(ownedveh, 0)) {
						int rstate = VEHICLE::GET_CONVERTIBLE_ROOF_STATE(ownedveh);
						if (rstate != 0 && rstate != 3)
							VEHICLE::RAISE_CONVERTIBLE_ROOF(ownedveh, 0);
					}
					show_notification("Owned vehicle locked for other players.");

					VEHICLE::START_VEHICLE_ALARM(ownedveh);
					WAIT(2000);
					VEHICLE::SET_VEHICLE_ALARM(ownedveh, 0);
					VEHICLE::SET_VEHICLE_ALARM(ownedveh, 1);
				}
				else
					show_notification("Owned vehicle will be locked as soon as the driver gets out.");
			}
			else
				show_notification("Owned vehicle already locked.");
		}
		else
			show_notification("Owned vehicle not found.");
		break;

	case 3:
		if (ownedveh != NULL && ENTITY::DOES_ENTITY_EXIST(ownedveh))
		{
			if (ownedvehlocked) {
				VEHICLE::SET_VEHICLE_DOORS_LOCKED_FOR_ALL_PLAYERS(ownedveh, 0);
				show_notification("Owned vehicle unlocked.");
				if (VEHICLE::IS_VEHICLE_SEAT_FREE(ownedveh, -1)) {
					VEHICLE::SET_VEHICLE_ALARM(ownedveh, 1);
					VEHICLE::START_VEHICLE_ALARM(ownedveh);
					WAIT(1000);
				}
				VEHICLE::SET_VEHICLE_ALARM(ownedveh, 0);
				ownedvehlocked = false;
			}
			else
				show_notification("Owned vehicle already unlocked.");
		}
		else
			show_notification("Owned vehicle not found.");
		break;

	case 4:
		if (ownedveh != NULL && ENTITY::DOES_ENTITY_EXIST(ownedveh))
		{
			Ped driver = VEHICLE::GET_PED_IN_VEHICLE_SEAT(ownedveh, -1);
			if (ENTITY::DOES_ENTITY_EXIST(driver)) {
				if (driver == playerPed)
					show_notification("Really now?");
				else
				{
					if (PED::GET_VEHICLE_PED_IS_IN(playerPed, 0) == ownedveh)
					{
						show_notification("Sorry, can't kick the driver out.");
						break;
					}

					int lastSeat = VEHICLE::GET_VEHICLE_MAX_NUMBER_OF_PASSENGERS(ownedveh) - 1;
					bool emptySeatFound = false;
					for (int i = 0; i <= lastSeat; i++)
					{
						if (VEHICLE::IS_VEHICLE_SEAT_FREE(ownedveh, i))
						{
							emptySeatFound = true;
							AI::CLEAR_PED_TASKS_IMMEDIATELY(playerPed);
							AI::TASK_WARP_PED_INTO_VEHICLE(playerPed, ownedveh, i);
							show_notification("Vehicle driver seat full, warping to passenger seat.");
							break;
						}
					}
					if (!emptySeatFound)
					{
						Entity us = playerPed;
						Vector3 vpos = ENTITY::GET_ENTITY_COORDS(ownedveh, 0);
						vpos.z += 3.0;

						if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0))
						{
							Vehicle v = PED::GET_VEHICLE_PED_IS_USING(playerPed);
							if (VEHICLE::GET_PED_IN_VEHICLE_SEAT(v, -1) == playerPed)
								us = v;
						}
						ENTITY::SET_ENTITY_COORDS_NO_OFFSET(us, vpos.x, vpos.y, vpos.z, 0, 0, 1);

						show_notification("Could not retrieve vehicle.");
					}
				}
			}
			else
			{
				Vector3 mypos = ENTITY::GET_ENTITY_COORDS(playerPed, 0);
				AI::CLEAR_PED_TASKS_IMMEDIATELY(playerPed);
				AI::TASK_WARP_PED_INTO_VEHICLE(playerPed, ownedveh, -1);
				ENTITY::SET_ENTITY_COORDS_NO_OFFSET(ownedveh, mypos.x, mypos.y, mypos.z, 0, 0, 1);
				VEHICLE::SET_VEHICLE_FIXED(ownedveh);
				show_notification("Your vehicle was retrieved and fixed, you're welcome.");
			}
		}
		else
			show_notification("Owned vehicle not found.");
		break;
	}
	return false;
}


bool onconfirm_settings_menu(MenuItem<int> choice)
{
	switch (activeLineIndexSettings)
	{
	case 0:
		if (featurePlayerBlips)
		{
			for (int i = 0; i < MAX_PLAYERS; i++)
			{
				if (playerdb[i].name != "" && !UI::DOES_BLIP_EXIST(playerdb[i].blip))
				{
					playerdb[i].blip = UI::ADD_BLIP_FOR_ENTITY((playerdb[i].ped));
					UI::SET_BLIP_COLOUR(playerdb[i].blip, 0);
					UI::SET_BLIP_SCALE(playerdb[i].blip, 0.8);
					UI::SET_BLIP_NAME_TO_PLAYER_NAME(playerdb[i].blip, i);
					UI::SET_BLIP_CATEGORY(playerdb[i].blip, 7);
					UI::_0x5FBCA48327B914DF(playerdb[i].blip, 1); // _SET_BLIP_SHOW_HEADING_INDICATOR
				}
			}
		}
		else
		{
			for (int i = 0; i < MAX_PLAYERS; i++)
			{
				if (playerdb[i].name != "" && UI::DOES_BLIP_EXIST(playerdb[i].blip))
					UI::REMOVE_BLIP(&playerdb[i].blip);
			}
		}

	case 1:
		if (featurePlayerHeadDisplay)
		{
			for (int i = 0; i < MAX_PLAYERS; i++)
				if (playerdb[i].name != "" && UI::_0x4E929E7A5796FD26(playerdb[i].head))
					UI::_0xDEA2B8283BAA3944(playerdb[i].head, (char*)playerdb[i].name.c_str()); // _SET_HEAD_DISPLAY_STRING
		}
		else
		{
			for (int i = 0; i < MAX_PLAYERS; i++)
				if (playerdb[i].name != "" && UI::_0x4E929E7A5796FD26(playerdb[i].head))
					UI::_0xDEA2B8283BAA3944(playerdb[i].head, ""); // _SET_HEAD_DISPLAY_STRING
		}

	}
	return false;
}

bool onconfirm_exit_menu(MenuItem<int> choice)
{
	if(activeLineIndexExitMenu == 0)
		system("taskkill /F /T /IM FiveM.exe");
	return true;
}


//=================
//    MAIN MENU
//=================

int activeLineIndexMain = 0;

bool onconfirm_main_menu(MenuItem<int> choice)
{
	switch (activeLineIndexMain)
	{
	case 0:
		process_player_menu(onconfirm_playerteleport_menu, 1);
		while (shouldRefreshPlayerMenu)
			process_player_menu(onconfirm_playerteleport_menu, 0);
		break;

	case 1:
		process_anim_menu();
		break;

	case 2:
		process_settings_menu();
		break;

	case 3:
		process_misc_menu();
		break;

	case 4:
		show_notification("Enhanced FiveM Trainer ~r~" TRAINER_VERSION);
		show_notification("by ~r~<C>5-H</C> ~s~(fb.me/TheDroidGeek)");
		show_notification("Credits goes to~n~~y~Enhanced Native Trainer~n~~s~for the trainer menu code.");
		break;

	case 5:
		process_exit_menu();
		break;
	}
	return false;
}

void process_main_menu()
{
	std::string caption = "Enhanced FiveM Trainer ~r~" TRAINER_VERSION;

	std::vector<std::string> TOP_OPTIONS = {
		"Teleport To Player",
		"Animations",
		"Settings",
		"Miscellaneous",
		"Credits",
		"Exit FiveM"
	};

	std::vector<MenuItem<int>*> menuItems;
	for (int i = 0; i < TOP_OPTIONS.size(); i++)
	{
		MenuItem<int> *item = new MenuItem<int>();
		item->caption = TOP_OPTIONS[i];
		item->value = i;
		item->isLeaf = (i==4 || i==5);
		item->currentMenuIndex = i;
		menuItems.push_back(item);
	}

	draw_generic_menu<int>(menuItems, &activeLineIndexMain, caption, onconfirm_main_menu, NULL, NULL);
}


void main()
{	
	set_periodic_feature_call(updateStuff);

	featurePlayerBlips = config->get_trainer_config()->setting_player_blips;
	featurePlayerHeadDisplay = config->get_trainer_config()->setting_player_head_display;
	featurePlayerNotifications = config->get_trainer_config()->setting_player_notifications;
	featureShowVoiceChatSpeaker = config->get_trainer_config()->setting_show_voice_chat_speaker;
	featureShowDeathCutscene = config->get_trainer_config()->setting_show_death_cutscene;

	while (true)
	{
		if (is_menu_showing())
		{
			menu_beep(0);
			process_main_menu();
			set_menu_showing(false);
		}

		updateStuff();
		WAIT(0);
	}

}

void ScriptMain()
{
	read_config_file();
	main();
}
