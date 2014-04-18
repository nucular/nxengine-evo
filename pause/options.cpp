
#include "../nx.h"
#include "options.h"
#include "dialog.h"
#include "message.h"
using namespace Options;
#include "../map.h"
#include "../settings.h"
#include "../input.h"
#include "../game.h"
#include "../sound/sound.h"
#include "../common/misc.h"

#include "../graphics/graphics.h"



std::vector<void*> optionstack;

void DialogDismissed();
static void EnterMainMenu();
void LeavingMainMenu();
void _res_get(ODItem *item);
void _res_change(ODItem *item, int dir);
void _fullscreen_get(ODItem *item);
void _fullscreen_change(ODItem *item, int dir);

void _debug_change(ODItem *item, int dir);
void _debug_get(ODItem *item);
void _save_change(ODItem *item, int dir);
void _save_get(ODItem *item);
void _sound_change(ODItem *item, int dir);
void _sound_get(ODItem *item);
void _music_change(ODItem *item, int dir);
void _music_get(ODItem *item);
static void EnterControlsMenu(ODItem *item, int dir);
static void _upd_control(ODItem *item);
static void _edit_control(ODItem *item, int dir);
static void _finish_control_edit(Message *msg);


#define SLIDE_SPEED				32
#define SLIDE_DIST				(3 * SLIDE_SPEED)

static struct
{
	Dialog *dlg, *subdlg;
	Dialog *dismiss_on_focus;
	int mm_cursel;
	bool InMainMenu;
	int xoffset;
	
	int32_t remapping_key, new_sdl_key;
} opt;


bool options_init(int retmode)
{
	memset(&opt, 0, sizeof(opt));
	Options::init_objects();
	opt.dlg = new Dialog;
	
	opt.xoffset = SLIDE_DIST;
	opt.dlg->offset(-SLIDE_DIST, 0);
	
	EnterMainMenu();
	opt.dlg->ondismiss = DialogDismissed;
	opt.dlg->ShowFull();
	
	inputs[F3KEY] = 0;
	sound(SND_MENU_MOVE);
	return 0;
}

void options_close()
{
	Options::close_objects();
	for (int i=0;i<optionstack.size();i++)
	{
	  FocusHolder *fh = (FocusHolder *)optionstack.at(i);
	  delete fh;
	}
	optionstack.clear();
	settings_save();
}

/*
void c------------------------------() {}
*/

void options_tick()
{
int i;
FocusHolder *fh;

	if (justpushed(F3KEY))
	{
		game.pause(0);
		return;
	}
	
	Graphics::ClearScreen(BLACK);
	Options::run_and_draw_objects();
	
	fh = (FocusHolder *)optionstack.at(optionstack.size() - 1);
	if (fh)
	{
		fh->RunInput();
		if (game.paused != GP_OPTIONS) return;
		
		fh = (FocusHolder *)optionstack.at(optionstack.size() - 1);
		if (fh == opt.dismiss_on_focus && fh)
		{
			opt.dismiss_on_focus = NULL;
			delete fh;
//			optionstack.erase(optionstack.begin()+(optionstack.size() - 1));
		}
	}
	
	for(i=0;i<optionstack.size();i++)
	{
		fh = (FocusHolder *)optionstack.at(i);
		fh->Draw();
	}
	
	if (opt.xoffset > 0)
	{
		opt.dlg->offset(SLIDE_SPEED, 0);
		opt.xoffset -= SLIDE_SPEED;
	}
}


/*
void c------------------------------() {}
*/

void DialogDismissed()
{
	if (opt.InMainMenu)
	{
		memset(inputs, 0, sizeof(inputs));
		game.pause(false);
	}
	else
	{
		EnterMainMenu();
	}
}


/*
void c------------------------------() {}
*/

static void EnterMainMenu()
{
Dialog *dlg = opt.dlg;

	dlg->Clear();
	
	dlg->AddItem("Resolution: ", _res_change, _res_get);
	dlg->AddItem("Fullscreen ", _fullscreen_change, _fullscreen_get);
	dlg->AddItem("Controls", EnterControlsMenu);
	
	dlg->AddSeparator();
	
	dlg->AddItem("Enable Debug Keys", _debug_change, _debug_get);
	dlg->AddItem("Save Slots: ", _save_change, _save_get);
	
	dlg->AddSeparator();
	
	dlg->AddItem("Music: ", _music_change, _music_get);
	dlg->AddItem("Sound: ", _sound_change, _sound_get);
	
	dlg->AddSeparator();
	dlg->AddDismissalItem();
	
	dlg->SetSelection(opt.mm_cursel);
	dlg->onclear = LeavingMainMenu;
	opt.InMainMenu = true;
}

void LeavingMainMenu()
{
	opt.mm_cursel = opt.dlg->GetSelection();
	opt.dlg->onclear = NULL;
	opt.InMainMenu = false;
}

void _res_get(ODItem *item)
{
	const char **reslist = Graphics::GetResolutions();
	
	if (settings->resolution < 0 || \
		settings->resolution >= count_string_list(reslist))
	{
		item->suffix[0] = 0;
	}
	else
	{
		strcpy(item->suffix, reslist[settings->resolution]);
	}
}


void _res_change(ODItem *item, int dir)
{
const char **reslist = Graphics::GetResolutions();
int numres = count_string_list(reslist);
int newres;

	sound(SND_DOOR);
	
	newres = (settings->resolution + dir);
	if (newres >= numres) newres = 1;
	if (newres < 1) newres = (numres - 1);
	
	if (!Graphics::SetResolution(newres, true))
	{
		settings->resolution = newres;
		Graphics::SetFullscreen(false);
		Graphics::SetFullscreen(settings->fullscreen);

	}
	else
	{
		new Message("Resolution change failed");
		sound(SND_GUN_CLICK);
	}
}

void _fullscreen_get(ODItem *item)
{
	static const char *strs[] = { "", " =" };
	strcpy(item->suffix, strs[settings->fullscreen]);
}


void _fullscreen_change(ODItem *item, int dir)
{
	settings->fullscreen ^= 1;
	sound(SND_MENU_SELECT);
	Graphics::SetFullscreen(settings->fullscreen);
}


void _debug_change(ODItem *item, int dir)
{
	settings->enable_debug_keys ^= 1;
	sound(SND_MENU_SELECT);
}

void _debug_get(ODItem *item)
{
	static const char *strs[] = { "", " =" };
	strcpy(item->suffix, strs[settings->enable_debug_keys]);
}


void _save_change(ODItem *item, int dir)
{
	settings->multisave ^= 1;
	sound(SND_MENU_MOVE);
}

void _save_get(ODItem *item)
{
	static const char *strs[] = { "1", "5" };
	strcpy(item->suffix, strs[settings->multisave]);
}


void _sound_change(ODItem *item, int dir)
{
	settings->sound_enabled ^= 1;
	sound(SND_MENU_SELECT);
}

void _sound_get(ODItem *item)
{
	static const char *strs[] = { "Off", "On" };
	strcpy(item->suffix, strs[settings->sound_enabled]);
}



void _music_change(ODItem *item, int dir)
{
	music_set_enabled((settings->music_enabled + 1) % 3);
	sound(SND_MENU_SELECT);
}

void _music_get(ODItem *item)
{
	static const char *strs[] = { "Off", "On", "Boss Only" };
	strcpy(item->suffix, strs[settings->music_enabled]);
}


/*
void c------------------------------() {}
*/

static void EnterControlsMenu(ODItem *item, int dir)
{
Dialog *dlg = opt.dlg;

	dlg->Clear();
	sound(SND_MENU_MOVE);
	
	dlg->AddItem("Left", _edit_control, _upd_control, LEFTKEY);
	dlg->AddItem("Right", _edit_control, _upd_control, RIGHTKEY);
	dlg->AddItem("Up", _edit_control, _upd_control, UPKEY);
	dlg->AddItem("Down", _edit_control, _upd_control, DOWNKEY);
	
	dlg->AddSeparator();
	
	dlg->AddItem("Jump", _edit_control, _upd_control, JUMPKEY);
	dlg->AddItem("Fire", _edit_control, _upd_control,  FIREKEY);
	dlg->AddItem("Wpn Prev", _edit_control, _upd_control, PREVWPNKEY);
	dlg->AddItem("Wpn Next", _edit_control, _upd_control, NEXTWPNKEY);
	dlg->AddItem("Inventory", _edit_control, _upd_control, INVENTORYKEY);
	dlg->AddItem("Map", _edit_control, _upd_control, MAPSYSTEMKEY);
	
	dlg->AddSeparator();
	dlg->AddDismissalItem();
}

static void _upd_control(ODItem *item)
{
	in_action action = input_get_mapping(item->id);
	int keysym = action.key;
	const char *keyname = SDL_GetKeyName((SDL_Keycode)keysym);
	
	maxcpy(item->righttext, keyname, sizeof(item->righttext) - 1);
}

static void _edit_control(ODItem *item, int dir)
{
Message *msg;

	opt.remapping_key = item->id;
	opt.new_sdl_key = -1;
	
	msg = new Message("Press new key for:", input_get_name(opt.remapping_key));
	msg->rawKeyReturn = &opt.new_sdl_key;
	msg->on_dismiss = _finish_control_edit;
	
	sound(SND_DOOR);
}

static void _finish_control_edit(Message *msg)
{
int inputno = opt.remapping_key;
int32_t new_sdl_key = opt.new_sdl_key;
int i;
in_action action = input_get_mapping(inputno);
	if (action.key == new_sdl_key)
	{
		sound(SND_MENU_MOVE);
		return;
	}
	
	// check if key is already in use
	for(i=0;i<INPUT_COUNT;i++)
	{
	    action = input_get_mapping(i);
		if (i != inputno && action.key == new_sdl_key)
		{
			new Message("Key already in use by:", input_get_name(i));
			sound(SND_GUN_CLICK);
			return;
		}
	}
	
	input_remap(inputno, new_sdl_key);
	sound(SND_MENU_SELECT);
	opt.dlg->Refresh();
}














