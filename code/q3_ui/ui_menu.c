/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
/*
=======================================================================

MAIN MENU

=======================================================================
*/


#include "ui_local.h"


#define ID_SINGLEPLAYER			10
#define ID_MULTIPLAYER			11
#define ID_SETUP				12
#define ID_DEMOS				13
//#define ID_CINEMATICS			14
#define ID_CHALLENGES                   14
#define ID_TEAMARENA		15
#define ID_MODS					16
#define ID_EXIT					17
#define ID_AIMING_MODE			18
#define ID_THIRD_PERSON			19
#define ID_GYROSCOPE			20
#define ID_VOICECHAT			21
#define ID_CARDBOARDVR			22

#define MAIN_BANNER_MODEL				"models/mapobjects/banner/banner5.md3"
#define MAIN_MENU_VERTICAL_SPACING		34


typedef struct {
	menuframework_s	menu;

	menutext_s		singleplayer;
	menutext_s		multiplayer;
	menutext_s		setup;
	menutext_s		demos;
	//menutext_s		cinematics;
        menutext_s              challenges;
	menutext_s		teamArena;
	menutext_s		mods;
	menutext_s		exit;
	menulist_s		aimingmode; // Hack for lazy Android users, who are too ignorant change game settings, but quick to downvote
	menuradiobutton_s	firstperson; // Another hack
	menuradiobutton_s	gyroscope; // Another hack
	menulist_s		voicechat; // Another hack
	menuradiobutton_s	cardboardVR; // Another hack
	qhandle_t		cardboardVRicon; // This hack has it's own icon

	qhandle_t		bannerModel;
} mainmenu_t;


static mainmenu_t s_main;

typedef struct {
	menuframework_s menu;	
	char errorMessage[4096];
} errorMessage_t;

static errorMessage_t s_errorMessage;

static const char *s_main_aimingmode_items[] = {
	"shoot button",
	"tap to shoot",
	"floating crosshair",
	"shoot under finger",
	"aim under finger",
	NULL
};


static const char *main_voicechat_items[] = {
# ifdef USE_VOIP
	"off", "shake and talk", "listen only", NULL
# else
	"experimental", NULL
# endif
};

/*
=================
MainMenu_ExitAction
=================
*/
/*static void MainMenu_ExitAction( qboolean result ) {
	if( !result ) {
		return;
	}
	UI_PopMenu();
	//UI_CreditMenu();
        trap_Cmd_ExecuteText( EXEC_APPEND, "quit\n" );
}*/



/*
=================
Main_MenuEvent
=================
*/
void Main_MenuEvent (void* ptr, int event) {
	if( event != QM_ACTIVATED ) {
		return;
	}

	switch( ((menucommon_s*)ptr)->id ) {
	case ID_SINGLEPLAYER:
		UI_SPLevelMenu();
		break;

	case ID_MULTIPLAYER:
            if(ui_setupchecked.integer)
		UI_ArenaServersMenu();
            else
                UI_FirstConnectMenu();
		break;

	case ID_SETUP:
		UI_SetupMenu();
		break;

	case ID_DEMOS:
		UI_DemosMenu();
		break;

	/*case ID_CINEMATICS:
		UI_CinematicsMenu();
		break;*/

            case ID_CHALLENGES:
                UI_Challenges();
                break;

	case ID_MODS:
		UI_ModsMenu();
		break;

	case ID_TEAMARENA:
		trap_Cvar_Set( "fs_game", "missionpack");
		trap_Cmd_ExecuteText( EXEC_APPEND, "vid_restart;" );
		break;

	case ID_EXIT:
		//UI_ConfirmMenu( "EXIT GAME?", 0, MainMenu_ExitAction );
                UI_CreditMenu();
		break;

	case ID_AIMING_MODE:
		trap_Cvar_SetValue( "cg_touchscreenControls", s_main.aimingmode.curvalue );
		if ( s_main.aimingmode.curvalue == TOUCHSCREEN_FLOATING_CROSSHAIR && trap_Cvar_VariableValue( "cg_thirdPersonConfigOptionInSettings" ) == 0 )
			trap_Cvar_SetValue( "cg_drawGun", 0 );
		break;

	case ID_THIRD_PERSON:
		trap_Cvar_SetValue( "cg_thirdPersonConfigOptionInSettings", !s_main.firstperson.curvalue );
		trap_Cvar_SetValue( "cg_thirdperson", !s_main.firstperson.curvalue );
		break;

	case ID_GYROSCOPE:
		trap_Cvar_SetValue( "in_gyroscope", s_main.gyroscope.curvalue );
		break;

	case ID_VOICECHAT:
		trap_Cvar_SetValue( "cl_voip", s_main.voicechat.curvalue );
		break;

	case ID_CARDBOARDVR:
		trap_Cvar_SetValue( "r_cardboardStereo", s_main.cardboardVR.curvalue );
		break;
	}
}


/*
===============
MainMenu_Cache
===============
*/
void MainMenu_Cache( void ) {
	s_main.bannerModel = trap_R_RegisterModel( MAIN_BANNER_MODEL );
	s_main.cardboardVRicon = trap_R_RegisterShaderNoMip( "gfx/2d/cardboard-vr-icon" );
}

sfxHandle_t ErrorMessage_Key(int key)
{
	trap_Cvar_Set( "com_errorMessage", "" );
	UI_MainMenu();
	return (menu_null_sound);
}

/*
===============
Main_MenuDraw
TTimo: this function is common to the main menu and errorMessage menu
===============
*/

static void Main_MenuDraw( void ) {
	refdef_t		refdef;
	refEntity_t		ent;
	vec3_t			origin;
	vec3_t			angles;
	float			adjust;
	float			x, y, w, h;
	vec4_t			color = {0.2, 0.2, 1.0, 1};

	// setup the refdef

	memset( &refdef, 0, sizeof( refdef ) );

	refdef.rdflags = RDF_NOWORLDMODEL;

	AxisClear( refdef.viewaxis );

	x = 0;
	y = 0;
	w = 640;
	h = 120;
	UI_AdjustFrom640( &x, &y, &w, &h );
	refdef.x = x;
	refdef.y = y;
	refdef.width = w;
	refdef.height = h;

	adjust = 0; // JDC: Kenneth asked me to stop this 1.0 * sin( (float)uis.realtime / 1000 );
	refdef.fov_x = 60 + adjust;
	refdef.fov_y = 19.6875 + adjust;

	refdef.time = uis.realtime;

	origin[0] = 300;
	origin[1] = 0;
	origin[2] = -32;

	trap_R_ClearScene();

	// add the model

	if (!s_main.cardboardVR.curvalue)
	{
		memset( &ent, 0, sizeof(ent) );

		adjust = 5.0 * sin( (float)uis.realtime / 5000 );
		VectorSet( angles, 0, 180 + adjust, 0 );
		AnglesToAxis( angles, ent.axis );
		ent.hModel = s_main.bannerModel;
		VectorCopy( origin, ent.origin );
		VectorCopy( origin, ent.lightingOrigin );
		ent.renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;
		VectorCopy( ent.origin, ent.oldorigin );

		trap_R_AddRefEntityToScene( &ent );

		trap_R_RenderScene( &refdef );
	}
	
	if (strlen(s_errorMessage.errorMessage))
	{
		UI_DrawProportionalString_AutoWrapped( 320, 192, 600, 20, s_errorMessage.errorMessage, UI_CENTER|UI_SMALLFONT|UI_DROPSHADOW, menu_text_color );
	}
	else
	{
		// standard menu drawing
		Menu_Draw( &s_main.menu );		
	}

	//UI_SetColor( (s_main.cardboardVR.generic.flags & QMF_HASMOUSEFOCUS) ? color_white : text_color_normal );
	UI_SetColor( Menu_ItemAtCursor(&s_main.menu) == &s_main.cardboardVR.generic ? color_white : text_color_normal );
	UI_DrawHandlePic( s_main.cardboardVR.generic.x - 70, s_main.cardboardVR.generic.y - SMALLCHAR_HEIGHT / 4 - 1, 32, 32, s_main.cardboardVRicon );

		UI_DrawProportionalString( 320, 372, "", UI_CENTER|UI_SMALLFONT, color );
		UI_DrawString( 320, 390, "OpenArena(c) 2005-2013 OpenArena Team", UI_CENTER|UI_SMALLFONT, color );
		UI_DrawString( 320, 414, "OpenArena comes with ABSOLUTELY NO WARRANTY; this is free software", UI_CENTER|UI_SMALLFONT, color );
		UI_DrawString( 320, 428, "and you are welcome to redistribute it under certain conditions;", UI_CENTER|UI_SMALLFONT, color );
		UI_DrawString( 320, 444, "read COPYING for details.", UI_CENTER|UI_SMALLFONT, color );
                
                //Draw version.
                UI_DrawString( 640-40, 480-14, "^7OAX", UI_SMALLFONT, color );
                if((int)trap_Cvar_VariableValue("protocol")!=71)
                    UI_DrawString( 0, 480-14, va("^7Protocol: %i",(int)trap_Cvar_VariableValue("protocol")), UI_SMALLFONT, color);
}


/*
===============
UI_TeamArenaExists
===============
*/
static qboolean UI_TeamArenaExists( void ) {
	int		numdirs;
	char	dirlist[2048];
	char	*dirptr;
  char  *descptr;
	int		i;
	int		dirlen;

	numdirs = trap_FS_GetFileList( "$modlist", "", dirlist, sizeof(dirlist) );
	dirptr  = dirlist;
	for( i = 0; i < numdirs; i++ ) {
		dirlen = strlen( dirptr ) + 1;
    descptr = dirptr + dirlen;
		if (Q_stricmp(dirptr, "missionpack") == 0) {
			return qtrue;
		}
    dirptr += dirlen + strlen(descptr) + 1;
	}
	return qfalse;
}


/*
===============
UI_MainMenu

The main menu only comes up when not in a game,
so make sure that the attract loop server is down
and that local cinematics are killed
===============
*/
void UI_MainMenu( void ) {
	int		y;
	qboolean teamArena = qfalse;
	int		style = UI_CENTER | UI_DROPSHADOW;

	trap_Cvar_Set( "sv_killserver", "1" );
        trap_Cvar_SetValue( "handicap", 100 ); //Reset handicap during server change, it must be ser per game

	memset( &s_main, 0 ,sizeof(mainmenu_t) );
	memset( &s_errorMessage, 0 ,sizeof(errorMessage_t) );

	// com_errorMessage would need that too
	MainMenu_Cache();
	
	trap_Cvar_VariableStringBuffer( "com_errorMessage", s_errorMessage.errorMessage, sizeof(s_errorMessage.errorMessage) );
	if (strlen(s_errorMessage.errorMessage))
	{	
		s_errorMessage.menu.draw = Main_MenuDraw;
		s_errorMessage.menu.key = ErrorMessage_Key;
		s_errorMessage.menu.fullscreen = qtrue;
		s_errorMessage.menu.wrapAround = qtrue;
		s_errorMessage.menu.showlogo = qtrue;		

		trap_Key_SetCatcher( KEYCATCH_UI );
		uis.menusp = 0;
		UI_PushMenu ( &s_errorMessage.menu );
		
		return;
	}

	s_main.menu.draw = Main_MenuDraw;
	s_main.menu.fullscreen = qtrue;
	s_main.menu.wrapAround = qtrue;
	s_main.menu.showlogo = qtrue;

	y = 134;
	s_main.singleplayer.generic.type		= MTYPE_PTEXT;
	s_main.singleplayer.generic.flags		= QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_main.singleplayer.generic.x			= 320;
	s_main.singleplayer.generic.y			= y;
	s_main.singleplayer.generic.id			= ID_SINGLEPLAYER;
	s_main.singleplayer.generic.callback	= Main_MenuEvent; 
	s_main.singleplayer.string				= "SINGLE PLAYER";
	s_main.singleplayer.color				= color_red;
	s_main.singleplayer.style				= style;

	y += MAIN_MENU_VERTICAL_SPACING;
	s_main.multiplayer.generic.type			= MTYPE_PTEXT;
	s_main.multiplayer.generic.flags		= QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_main.multiplayer.generic.x			= 320;
	s_main.multiplayer.generic.y			= y;
	s_main.multiplayer.generic.id			= ID_MULTIPLAYER;
	s_main.multiplayer.generic.callback		= Main_MenuEvent; 
	s_main.multiplayer.string				= "MULTIPLAYER";
	s_main.multiplayer.color				= color_red;
	s_main.multiplayer.style				= style;

	y += MAIN_MENU_VERTICAL_SPACING;
	s_main.setup.generic.type				= MTYPE_PTEXT;
	s_main.setup.generic.flags				= QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_main.setup.generic.x					= 320;
	s_main.setup.generic.y					= y;
	s_main.setup.generic.id					= ID_SETUP;
	s_main.setup.generic.callback			= Main_MenuEvent; 
	s_main.setup.string						= "SETTINGS"; //"SETUP"; // Well, "Settings" is more used nowadays
	s_main.setup.color						= color_red;
	s_main.setup.style						= style;

	y += MAIN_MENU_VERTICAL_SPACING;
	s_main.demos.generic.type				= MTYPE_PTEXT;
	s_main.demos.generic.flags				= QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_main.demos.generic.x					= 320;
	s_main.demos.generic.y					= y;
	s_main.demos.generic.id					= ID_DEMOS;
	s_main.demos.generic.callback			= Main_MenuEvent; 
	s_main.demos.string						= "DEMOS";
	s_main.demos.color						= color_red;
	s_main.demos.style						= style;

	/*y += MAIN_MENU_VERTICAL_SPACING;
	s_main.cinematics.generic.type			= MTYPE_PTEXT;
	s_main.cinematics.generic.flags			= QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_main.cinematics.generic.x				= 320;
	s_main.cinematics.generic.y				= y;
	s_main.cinematics.generic.id			= ID_CINEMATICS;
	s_main.cinematics.generic.callback		= Main_MenuEvent; 
	s_main.cinematics.string				= "CINEMATICS";
	s_main.cinematics.color					= color_red;
	s_main.cinematics.style					= style;*/

        y += MAIN_MENU_VERTICAL_SPACING;
	s_main.challenges.generic.type			= MTYPE_PTEXT;
	s_main.challenges.generic.flags			= QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_main.challenges.generic.x				= 320;
	s_main.challenges.generic.y				= y;
	s_main.challenges.generic.id			= ID_CHALLENGES;
	s_main.challenges.generic.callback		= Main_MenuEvent;
	s_main.challenges.string				= "STATISTICS";
	s_main.challenges.color					= color_red;
	s_main.challenges.style					= style;

	if (UI_TeamArenaExists()) {
		teamArena = qtrue;
		y += MAIN_MENU_VERTICAL_SPACING;
		s_main.teamArena.generic.type			= MTYPE_PTEXT;
		s_main.teamArena.generic.flags			= QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
		s_main.teamArena.generic.x				= 320;
		s_main.teamArena.generic.y				= y;
		s_main.teamArena.generic.id				= ID_TEAMARENA;
		s_main.teamArena.generic.callback		= Main_MenuEvent; 
		s_main.teamArena.string					= "MISSION PACK";
		s_main.teamArena.color					= color_red;
		s_main.teamArena.style					= style;
	}

	y += MAIN_MENU_VERTICAL_SPACING;
	s_main.mods.generic.type			= MTYPE_PTEXT;
	s_main.mods.generic.flags			= QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_main.mods.generic.x				= 320;
	s_main.mods.generic.y				= y;
	s_main.mods.generic.id				= ID_MODS;
	s_main.mods.generic.callback		= Main_MenuEvent; 
	s_main.mods.string					= "MODS";
	s_main.mods.color					= color_red;
	s_main.mods.style					= style;

	y += MAIN_MENU_VERTICAL_SPACING;
	s_main.exit.generic.type				= MTYPE_PTEXT;
	s_main.exit.generic.flags				= QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_main.exit.generic.x					= 320;
	s_main.exit.generic.y					= y;
	s_main.exit.generic.id					= ID_EXIT;
	s_main.exit.generic.callback			= Main_MenuEvent; 
	s_main.exit.string						= "EXIT";
	s_main.exit.color						= color_red;
	s_main.exit.style						= style;

	s_main.aimingmode.generic.type		= MTYPE_SPINCONTROL;
	s_main.aimingmode.generic.flags		= QMF_SMALLFONT;
	s_main.aimingmode.generic.x			= 220;
	s_main.aimingmode.generic.y			= 15;
	s_main.aimingmode.generic.name		= "touchscreen controls:";
	s_main.aimingmode.generic.id		= ID_AIMING_MODE;
	s_main.aimingmode.generic.callback	= Main_MenuEvent;
	s_main.aimingmode.curvalue			= UI_ClampCvar( TOUCHSCREEN_FIRE_BUTTON, TOUCHSCREEN_AIM_UNDER_FINGER, trap_Cvar_VariableValue( "cg_touchscreenControls" ) );
	s_main.aimingmode.itemnames			= s_main_aimingmode_items;

	s_main.firstperson.generic.type		= MTYPE_RADIOBUTTON;
	s_main.firstperson.generic.flags	= QMF_SMALLFONT;
	s_main.firstperson.generic.x		= SCREEN_WIDTH - 60;
	s_main.firstperson.generic.y		= 15; // + SMALLCHAR_HEIGHT + 2;
	s_main.firstperson.generic.name		= "first-person view:";
	s_main.firstperson.generic.id		= ID_THIRD_PERSON;
	s_main.firstperson.generic.callback	= Main_MenuEvent;
	s_main.firstperson.curvalue			= !trap_Cvar_VariableValue( "cg_thirdPersonConfigOptionInSettings" );

	s_main.gyroscope.generic.type		= MTYPE_RADIOBUTTON;
	s_main.gyroscope.generic.flags		= QMF_SMALLFONT;
	s_main.gyroscope.generic.x			= SCREEN_WIDTH - 60;
	s_main.gyroscope.generic.y			= 15 + SMALLCHAR_HEIGHT + 2;
	s_main.gyroscope.generic.name		= "gyroscope:";
	s_main.gyroscope.generic.id			= ID_GYROSCOPE;
	s_main.gyroscope.generic.callback	= Main_MenuEvent;
	s_main.gyroscope.curvalue			= trap_Cvar_VariableValue( "in_gyroscope" );

	s_main.voicechat.generic.type			= MTYPE_SPINCONTROL;
	s_main.voicechat.generic.name			= "voice chat:";
	s_main.voicechat.generic.flags			= QMF_SMALLFONT;
	s_main.voicechat.generic.callback		= Main_MenuEvent;
	s_main.voicechat.generic.id				= ID_VOICECHAT;
	s_main.voicechat.generic.x				= 140;
	s_main.voicechat.generic.y				= 15 + SMALLCHAR_HEIGHT + 2;
	s_main.voicechat.itemnames				= main_voicechat_items;
	s_main.voicechat.curvalue				= trap_Cvar_VariableValue( "cl_voip" );

	s_main.cardboardVR.generic.type		= MTYPE_RADIOBUTTON;
	s_main.cardboardVR.generic.flags	= QMF_SMALLFONT;
	s_main.cardboardVR.generic.x		= 120;
	s_main.cardboardVR.generic.y		= 360;
	s_main.cardboardVR.generic.name		= "     vr:";
	s_main.cardboardVR.generic.id		= ID_CARDBOARDVR;
	s_main.cardboardVR.generic.callback	= Main_MenuEvent;
	s_main.cardboardVR.curvalue			= trap_Cvar_VariableValue( "r_cardboardStereo" );

	Menu_AddItem( &s_main.menu,	&s_main.singleplayer );
	Menu_AddItem( &s_main.menu,	&s_main.multiplayer );
	Menu_AddItem( &s_main.menu,	&s_main.setup );
	Menu_AddItem( &s_main.menu,	&s_main.demos );
	//Menu_AddItem( &s_main.menu,	&s_main.cinematics );
        Menu_AddItem( &s_main.menu,	&s_main.challenges );
	if (teamArena) {
		Menu_AddItem( &s_main.menu,	&s_main.teamArena );
	}
	Menu_AddItem( &s_main.menu,	&s_main.mods );
	Menu_AddItem( &s_main.menu,	&s_main.exit );
	Menu_AddItem( &s_main.menu,	&s_main.aimingmode );
	Menu_AddItem( &s_main.menu,	&s_main.firstperson );
	Menu_AddItem( &s_main.menu,	&s_main.gyroscope );
	Menu_AddItem( &s_main.menu,	&s_main.voicechat );
	Menu_AddItem( &s_main.menu,	&s_main.cardboardVR );

	trap_Key_SetCatcher( KEYCATCH_UI );
	uis.menusp = 0;
	UI_PushMenu ( &s_main.menu );

}
