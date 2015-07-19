/*
Part of the Enhanced Native Trainer project.
https://github.com/gtav-ent/GTAV-EnhancedNativeTrainer
(C) Rob Pridham and fellow contributors 2015
*/

#pragma warning(disable : 4192) //ignore automatically excluding while importing type library warnings

#import <msxml6.dll> //read the GitHub project readme regarding what you need to make this work

#include "keyboard.h"

/**A class to hold the current key bindings.*/
class SettingsConfig
{
public:
	//these are the defaults which may be overridden by the XML config

	int toggle_trainer_menu = VK_F3;

	int key_menu_up = VK_NUMPAD8;
	int key_menu_down = VK_NUMPAD2;
	int key_menu_left = VK_NUMPAD4;
	int key_menu_right = VK_NUMPAD6;
	int key_menu_confirm = VK_NUMPAD5;
	int key_menu_back = VK_NUMPAD0;

	bool setting_player_blips = 1;
	bool setting_player_head_display = 1;
	bool setting_player_blip_cone = 0;
	bool setting_player_notifications = 1;
	bool setting_show_speaking_players = 1;

	/**Change the key binding and settings using a function string and key string.*/
	void set_param(char* function, char* value);
};

/**A class to hold all the user settings.*/
class TrainerConfig
{
public:
	TrainerConfig();
	SettingsConfig* get_trainer_config() { return settingsConfig; }

private:
	SettingsConfig* settingsConfig;
};

/**The current user config.*/
extern TrainerConfig* config;

/**Read the user config in from an XML file.*/
void read_config_file();

/**Get the current config object.*/
inline TrainerConfig* get_config() { return config;  }
