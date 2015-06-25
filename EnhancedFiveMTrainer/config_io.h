/*
Part of the Enhanced Native Trainer project.
https://github.com/gtav-ent/GTAV-EnhancedNativeTrainer
(C) Rob Pridham and fellow contributors 2015
*/

#include <stdio.h>
#include <tchar.h>
#include <windows.h>

#pragma warning(disable : 4192) //ignore automatically excluding while importing type library warnings

#import <msxml6.dll> //read the GitHub project readme regarding what you need to make this work

#include "keyboard.h"

/**A class to hold the current key bindings.*/
class KeyInputConfig
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

	/**Change the key binding using a function string and key string.*/
	void set_key(char* function, char* keyName);
};

/**A class to hold all the user settings.*/
class TrainerConfig
{
public:
	TrainerConfig();
	KeyInputConfig* get_key_config() { return keyConfig;  }

private:
	KeyInputConfig* keyConfig;
};

/**The current user config.*/
extern TrainerConfig* config;

/**Read the user config in from an XML file.*/
void read_config_file();

/**Get the current config object.*/
inline TrainerConfig* get_config() { return config;  }

