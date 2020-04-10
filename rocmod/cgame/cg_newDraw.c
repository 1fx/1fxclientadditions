/*
===========================================================================
Copyright (C) 2000 - 2013, Raven Software, Inc.
Copyright (C) 2001 - 2013, Activision, Inc.
Copyright (C) 2003 - 2007, ROCmod contributors
Copyright (C) 2015 - 2020, Ane-Jouke Schat (Boe!Man)
Copyright (C) 2015 - 2020, Jordi Epema (Henkie)

This file is part of the 1fx. Client Additions source code.

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see <http://www.gnu.org/licenses/>.
===========================================================================
*/
// cg_newdraw.c --

#include "cg_local.h"
#include "../game/bg_public.h"
#include "../ui/ui_shared.h"

extern displayContextDef_t cgDC;

static float healthColors[4][4] =
{
  { 1.0f, 0.69f, 0.0f, 1.0f } ,		// normal
  { 1.0f, 0.2f, 0.2f, 1.0f },		// low health
  { 0.5f, 0.5f, 0.5f, 1.0f},		// weapon firing
  { 1.0f, 1.0f, 1.0f, 1.0f } 		// health > 100
};

static void CG_DrawPlayerLocation( rectDef_t *rect, float scale, vec4_t color, int textStyle  )
{
	clientInfo_t *ci = &cgs.clientinfo[cg.snap->ps.clientNum];
	const char   *s;

	if (!ci)
	{
		return;
	}

	s = CG_ConfigString(CS_LOCATIONS + ci->location);

	if (!s || !*s)
	{
		s = "unknown";
	}

    CG_DrawText(rect->x, rect->y + rect->h, cgs.media.hudFont, scale, color, s, 0, 0 );
}

//==============================================================================
//	CG_DrawPlayerSniperBullet
//==============================================================================
static void CG_DrawPlayerSniperBullet ( rectDef_t* rect, qhandle_t shader, int bullet )
{
	// Dont draw the bullet if the player doesnt have enough ammo
	if ( bullet > cg.predictedPlayerState.clip[ATTACK_NORMAL][WP_MSG90A1] )
	{
		return;
	}

	CG_DrawStretchPic ( rect->x, rect->y, rect->w, rect->h, 0, 0, 1, 1, NULL, shader );
}

//==============================================================================
//	CG_DrawPlayerSniperMagnification
//==============================================================================
static void CG_DrawPlayerSniperMagnification ( rectDef_t *rect, qhandle_t font, float scale, vec4_t color )
{
	const char* mag;

	mag = weaponData[cg.predictedPlayerState.weapon].zoom[cg.predictedPlayerState.zoomFov].name;

	// Center the text
	CG_DrawText (rect->x, rect->y, font, scale, color, mag, 0, 0 );
}

//==============================================================================
//	CG_DrawPlayerWeaponImage
//==============================================================================
static void CG_DrawPlayerWeaponImage(rectDef_t *rect, vec4_t color, int flags )
{
	float x, y, w, h;
	int item;

	static weaponDraw_t weaponlist[25] = {
		{ WP_KNIFE, 0.0f, -8.0f, 0, (128 * 0.67f), (64 * 0.67f) },
		{ WP_M1911A1_PISTOL, 9.0f, 0.0f, 0, (128 * 0.33f), (64 * 0.33f) },
		{ WP_USSOCOM_PISTOL, 10.0f, 0.0f, 0, (128 * 0.33f), (64 * 0.33f) },
		{ WP_SILVER_TALON, 4.0f, 0.0f, 0, (128 * 0.33f), (64 * 0.33f) },
		{ WP_M590_SHOTGUN, 17.0f, -10.0f, 0, (128 * 0.67f), (64 * 0.67f) },
		{ WP_MICRO_UZI_SUBMACHINEGUN, 6.0f, 0.0f, 0, (128 * 0.33f), (64 * 0.33f) },
		{ WP_M3A1_SUBMACHINEGUN, 1.0f, 0.0f, 0, (128 * 0.45f), (64 * 0.45f) },
		{ WP_MP5, 1.0f, -6.0f, 0, (128 * 0.67f), (64 * 0.67f) },
		{ WP_USAS_12_SHOTGUN, 0.0f, -1.0f, 0, (128 * 0.67f), (64 * 0.67f) },
		{ WP_M4_ASSAULT_RIFLE, 3.0f, -9.0f, 0, (128 * 0.67f), (64 * 0.67f) },
		{ WP_AK74_ASSAULT_RIFLE, 0.0f, -14.0f, 0, (128 * 0.67f), (64 * 0.67f) },
		{ WP_SIG551, -1.0f, -6.0f, 0, (128 * 0.67f), (64 * 0.67f) },
#ifdef SMKWEAPONS
		{ WP_OICW, 0.0f, -4.0f, 0, (128 * 0.45f), (64 * 0.45f) },
#endif
		{ WP_MSG90A1, 0.0f, -11.0f, 0, (128 * 0.67f), (64 * 0.67f) },
		{ WP_M60_MACHINEGUN, 0.0f, -9.0f, 0, (128 * 0.67f), (64 * 0.67f) },
		{ WP_MM1_GRENADE_LAUNCHER, 0.0f, -3.0f, 0, (128 * 0.45f), (64 * 0.45f) },
		{ WP_RPG7_LAUNCHER, 0.0f, -9.0f, 0, (128 * 0.67f), (64 * 0.67f) },
		{ WP_M84_GRENADE, 0.0f, 0.0f, 0, (64 * 0.33f), (64 * 0.33f) },
		{ WP_SMOHG92_GRENADE, 0.0f, 0.0f, 0, (64 * 0.33f), (64 * 0.33f) },
		{ WP_ANM14_GRENADE, 0.0f, 0.0f, 0, (64 * 0.33f), (64 * 0.33f) },
		{ WP_M15_GRENADE, 0.0f, 0.0f, 0, (64 * 0.33f), (64 * 0.33f) },
		{ WP_M67_GRENADE, 0.0f, 0.0f, 0, (64 * 0.33f), (64 * 0.33f) },
		{ WP_F1_GRENADE, 0.0f, 0.0f, 0, (64 * 0.33f), (64 * 0.33f) },
		{ WP_L2A2_GRENADE, 0.0f, 0.0f, 0, (64 * 0.33f), (64 * 0.33f) },
		{ WP_MDN11_GRENADE, 0.0f, 0.0f, 0, (64 * 0.33f), (64 * 0.33f) },
	};

	if ( cg.weaponSelect <= WP_NONE )
		return;

	item = cg.weaponSelect - 1;
	w = weaponlist[item].width;
	h = weaponlist[item].height;
	x = rect->x - w + weaponlist[item].xadjust;
	y = rect->y + weaponlist[item].yadjust;

	cgs.teamData.redShade[3] = 1.0f;
	cgs.teamData.blueShade[3] = 1.0f;
	if ( flags & CG_SHOW_ONBLUETEAM )
	{
		trap_R_SetColor ( cgs.teamData.blueShade );
	}
	else if ( flags & CG_SHOW_ONREDTEAM )
	{
		trap_R_SetColor ( cgs.teamData.redShade );
	}
	else
	{
		trap_R_SetColor ( color );
	}

	// Draw the icon
	CG_DrawPic ( x, y, w, h, cg_weapons[cg.weaponSelect].weaponIcon );

	// Back to no color
	trap_R_SetColor ( NULL );
}

//==============================================================================
//	CG_DrawPlayerWeaponName
//==============================================================================
static void CG_DrawPlayerWeaponName ( rectDef_t *rect, qhandle_t font, float scale, vec4_t color  )
{
	float		width;
	const char*	name;

	if( cg.weaponSelect > WP_NONE )
	{
		// Equipped weapon
		name = weaponData[cg.weaponSelect].classname;
	}
	else
	{
		// No weapon equipped.
		name = "NONE";
	}

	// Get the width of the text so we can center it
	width = trap_R_GetTextWidth ( name, font, scale, 0 );

	// Center the text
	CG_DrawText (rect->x + (rect->w - width) / 2, rect->y + rect->h, font, scale, color, name, 0, 0 );
}

//==============================================================================
//	CG_DrawPlayerWeaponName
//==============================================================================
static void CG_DrawPlayerAltWeaponName ( rectDef_t *rect, qhandle_t font, float scale, vec4_t color )
{
	float		width;
	const char*	name;

	if( cg.weaponSelect > WP_NONE )
	{
		// Equipped weapon
		name = weaponData[cg.weaponSelect].attack[ATTACK_ALTERNATE].name;
	}
	else
	{
		// No weapon equipped.
		name = "NONE";
	}

	// Get the width of the text so we can center it
	width = trap_R_GetTextWidth ( name, font, scale, 0 );

	// Center the text
	CG_DrawText (rect->x + (rect->w - width) / 2, rect->y + rect->h, font, scale, color, name, 0, 0 );
}

//==============================================================================
//	CG_DrawPlayerWeaponAmmo
//==============================================================================
static void CG_DrawPlayerWeaponAmmo(rectDef_t *rect, qhandle_t font, float scale, vec4_t color )
{
	const char *ammo;
	int			value;
	float		width;

	// If the client doenst have a selected weapon then dont draw the ammo
	if ( cg.weaponSelect <= WP_NONE )
		return;

	// Retrieve the ammo value
	value = cg.predictedPlayerState.ammo[weaponData[cg.weaponSelect].attack[ATTACK_NORMAL].ammoIndex];

	// If the ammo value is crazy then dont display it
	if ( value <= -1 )
		return;

	// Build the ammo string padded with zeros
	ammo = va ( "%03d", value );

	// Retrive the width of the ammo string so it can be centered
	width = trap_R_GetTextWidth (ammo, font, scale, 0 );

	// Draw the ammo string centered
	if ( cg_drawEnhancedHUD.integer )
	{
		CG_DrawText (rect->x, rect->y, font, scale, color, ammo, 0, DT_OUTLINE );
	}
	else
	{
		CG_DrawText (rect->x + (rect->w - width) / 2, rect->y + rect->h, font, scale, color, ammo, 0, 0 );
	}
}

//==============================================================================
//	CG_DrawPlayerAltWeaponAmmo
//==============================================================================
static void CG_DrawPlayerAltWeaponAmmo(rectDef_t *rect, qhandle_t font, float scale, vec4_t color )
{
	const char *ammo;
	int			value;
	float		width;

	// If the client doenst have a selected weapon then dont draw the ammo
	if ( cg.weaponSelect <= WP_NONE )
		return;

	// If the selected weapon doesn't have an altclip, don't draw.
	if(weaponData[cg.weaponSelect].attack[ATTACK_ALTERNATE].clipSize<=0)
		return;

	// Retrieve the ammo value
	value  = cg.predictedPlayerState.ammo[weaponData[cg.weaponSelect].attack[ATTACK_ALTERNATE].ammoIndex];
	value += cg.predictedPlayerState.clip[ATTACK_ALTERNATE][cg.weaponSelect];

	// If the ammo value is crazy then dont display it
	if ( value <= -1 )
		return;

	// Build the ammo string padded with zeros
	ammo = va ( "%03d", value );

	// Retrive the width of the ammo string so it can be centered
	width = trap_R_GetTextWidth (ammo, font, scale, 0 );

	// Draw the ammo string centered
	if ( cg_drawEnhancedHUD.integer )
	{
		CG_DrawText (rect->x, rect->y, font, scale, color, ammo, 0, DT_OUTLINE );
	}
	else
	{
		CG_DrawText (rect->x + (rect->w - width) / 2, rect->y + rect->h, font, scale, color, ammo, 0, 0 );
	}
}

//==============================================================================
//	CG_DrawPlayerAltWeaponAmmoIcon
//==============================================================================
static void CG_DrawPlayerAltWeaponAmmoIcon(rectDef_t *rect, vec4_t color )
{
	vec4_t fade;

	// If the client doenst have a selected weapon then dont draw the ammo icon
	if ( cg.weaponSelect <= WP_NONE )
		return;

	// If there is no loaded icon then dont draw it
	if ( !cg_weapons[cg.weaponSelect].attack[ATTACK_ALTERNATE].ammoIcon )
	{
		return;
	}

	// Fade out the icon if there isnt anything in the alt clip
	fade[0] = color[0];
	fade[1] = color[1];
	fade[2] = color[2];
	fade[3] = color[3] * (cg.predictedPlayerState.clip[ATTACK_ALTERNATE][cg.weaponSelect]?1:0.20f);

	trap_R_SetColor ( fade );

	// Draw the icon
	CG_DrawPic ( rect->x, rect->y, rect->w, rect->h, cg_weapons[cg.weaponSelect].attack[ATTACK_ALTERNATE].ammoIcon );

	// Back to no color
	trap_R_SetColor ( NULL );
}

//==============================================================================
//	CG_DrawPlayerWeaponClip
//==============================================================================
static void CG_DrawPlayerWeaponClip (rectDef_t *rect, qhandle_t font, float scale, vec4_t color )
{
	const char *clip;
	int			value;
	float		width;
	static int	shade;

	// If the client doenst have a selected weapon then dont draw the ammo
	if ( cg.weaponSelect <= WP_NONE )
		return;

	// Retrieve the ammo value
	value = cg.predictedPlayerState.clip[ATTACK_NORMAL][cg.weaponSelect];

	// If the ammo value is crazy then dont display it
	if ( value <= -1 )
		return;

	if ( value <= (weaponData[cg.weaponSelect].attack[ATTACK_NORMAL].clipSize / 6) )
	{
		VectorSet ( color, 1.0f, 0.2f, 0.2f );
	}

	// Build the clip string padded with zeros
	clip = va ( "%02d", value );

	// Retrive the width of the clip string so it can be centered
	width = trap_R_GetTextWidth (clip, font, scale, 0 );

	// Draw the clip string centered
	if ( cg_drawEnhancedHUD.integer )
	{
		CG_DrawText (rect->x, rect->y, font, scale, color, clip, 0, DT_OUTLINE );
	}
	else
	{
		CG_DrawText (rect->x + (rect->w - width) / 2, rect->y + rect->h, font, scale, color, clip, 0, 0 );
	}
}

//==============================================================================
//	CG_DrawPlayerWeaponMode
//==============================================================================
static void CG_DrawPlayerWeaponMode ( rectDef_t *rect, qhandle_t font, float scale, vec4_t color )
{
	float		width;
	const char*	mode;
	int			curMode;

	// Use the current fire mode for this weapon, if its undefined then find what it will
	// be once its set
	curMode = cg.predictedPlayerState.firemode[cg.weaponSelect];
	if ( curMode == WP_FIREMODE_NONE )
	{
		curMode = BG_FindFireMode ( cg.weaponSelect, ATTACK_NORMAL, WP_FIREMODE_AUTO );
	}

	// TODO: Retrieve the real mode
	if ( cg_drawEnhancedHUD.integer )
	{
		switch ( curMode )
		{
			case WP_FIREMODE_NONE:
				mode = "";
				break;
			case WP_FIREMODE_AUTO:
				mode = "auto";
				break;
			case WP_FIREMODE_BURST:
				mode = "burst";
				break;
			case WP_FIREMODE_SINGLE:
				mode = "single";
				break;
#ifdef SMKWEAPONS
			case WP_FIREMODE_20MM:	//added 1/13/05 by pimpy
				mode = "20mm";
				break;
#endif
			default:
				// This shouldn't happen.
				assert(0);
				mode = "";
				break;
		}
	}
	else
	{
		switch ( curMode )
		{
			case WP_FIREMODE_NONE:
				mode = "";
				break;
			case WP_FIREMODE_AUTO:
				mode = "AUTO";
				break;
			case WP_FIREMODE_BURST:
				mode = "BURST";
				break;
			case WP_FIREMODE_SINGLE:
				mode = "SINGLE";
				break;
#ifdef SMKWEAPONS
			case WP_FIREMODE_20MM:	//added 1/13/05 by pimpy
				mode = "20mm";
				break;
#endif
			default:
				// This shouldn't happen.
				assert(0);
				mode = "";
				break;
		}
	}

	// Get the width of the text so we can center it
	width = trap_R_GetTextWidth ( mode, font, scale, 0 );

	// Center the text
	if ( cg_drawEnhancedHUD.integer )
	{
		CG_DrawText (rect->x, rect->y, font, scale, color, mode, 0, DT_OUTLINE );
	}
	else
	{
		CG_DrawText (rect->x + (rect->w - width) / 2, rect->y + rect->h, font, scale, color, mode, 0, 0 );
	}
}

//==============================================================================
//	CG_DrawPlayerWeaponList
//==============================================================================
static void CG_DrawPlayerWeaponList(rectDef_t *rect, qhandle_t font, vec4_t color, int flags )
{
	float		x, y, w, h;
	float		xadjust, yadjust, wadjust;
	int			item;
	int			weaponToDraw;
	int			category; //pimpy
	const char*	mode;	  //pimpy
	int			curMode;  //pipmy
	const char *ammo;
	int			value;
	float		width;
	float		listwidth;
	float		listscale;
	float		yscale;

	vec4_t colorGrey = {0.50f,0.50f,0.50f,0.80f};

	static weaponDraw_t weaponlist[25] = {
		{ WP_KNIFE, 0.0f, 0.0f, 1.0f, (128 * 0.35f), (64 * 0.35f) },
		{ WP_M1911A1_PISTOL, 0.0f, 0.0f, -8.0f, (128 * 0.35f), (64 * 0.35f) },
		{ WP_USSOCOM_PISTOL, 0.0f, 0.0f, -8.0f, (128 * 0.35f), (64 * 0.35f) },
		{ WP_SILVER_TALON, 0.0f, 0.0f, -3.0f, (128 * 0.35f), (64 * 0.35f) },
		{ WP_M590_SHOTGUN, 0.0f, -5.0f, -11.0f, (128 * 0.50f), (64 * 0.50f) },
		{ WP_MICRO_UZI_SUBMACHINEGUN, 0.0f, 1.0f, -6.0f, (128 * 0.35f), (64 * 0.35f) },
		{ WP_M3A1_SUBMACHINEGUN, 0.0f, 3.0f, 0.0f, (128 * 0.45f), (64 * 0.45f) },
		{ WP_MP5, 0.0f, -2.0f, 0.0f, (128 * 0.50f), (64 * 0.50f) },
		{ WP_USAS_12_SHOTGUN, 0.0f, 1.0f, 1.0f, (128 * 0.50f), (64 * 0.50f) },
		{ WP_M4_ASSAULT_RIFLE, 0.0f, -5.0f, 0.0f, (128 * 0.50f), (64 * 0.50f) },
		{ WP_AK74_ASSAULT_RIFLE, 0.0f, -8.0f, 1.0f, (128 * 0.50f), (64 * 0.50f) },
		{ WP_SIG551, 0.0f, 0.0f, 1.0f, (128 * 0.50f), (64 * 0.50f) },
#ifdef SMKWEAPONS
		{ WP_OICW, 0.0f, 0.0f, 0.0f, (128 * 0.45f), (64 * 0.45f) },
#endif
		{ WP_MSG90A1, 0.0f, -6.0f, 0.0f, (128 * 0.50f), (64 * 0.50f) },
		{ WP_M60_MACHINEGUN, 0.0f, -3.0f, 0.0f, (128 * 0.50f), (64 * 0.50f) },
		{ WP_MM1_GRENADE_LAUNCHER, 0.0f, 0.0f, 0.0f, (128 * 0.45f), (64 * 0.45f) },
		{ WP_RPG7_LAUNCHER, 0.0f, -6.0f, 0.0f, (128 * 0.50f), (64 * 0.50f) },
		{ WP_M84_GRENADE, 0.0f, 0.0f, -5.0f, (64 * 0.35f), (64 * 0.35f) },
		{ WP_SMOHG92_GRENADE, 0.0f, 0.0f, -3.0f, (64 * 0.35f), (64 * 0.35f) },
		{ WP_ANM14_GRENADE, 0.0f, 0.0f, -4.0f, (64 * 0.35f), (64 * 0.35f) },
		{ WP_M15_GRENADE, 0.0f, 0.0f, -4.0f, (64 * 0.35f), (64 * 0.35f) },
		{ WP_M67_GRENADE, 0.0f, 0.0f, -4.0f, (64 * 0.35f), (64 * 0.35f) },
		{ WP_F1_GRENADE, 0.0f, 0.0f, -4.0f, (64 * 0.35f), (64 * 0.35f) },
		{ WP_L2A2_GRENADE, 0.0f, 0.0f, -4.0f, (64 * 0.35f), (64 * 0.35f) },
		{ WP_MDN11_GRENADE, 0.0f, 0.0f, -4.0f, (64 * 0.35f), (64 * 0.35f) },
	};

//	CG_FillRect ( rect->x + rect->w, rect->y, 50, rect->h, color ); // pimpy the 50
														//was drawing a stupid white box
	if ( cg.weaponSelect <= WP_NONE )
		return;

	listwidth = 0;
	for (weaponToDraw = WP_NONE + 1; weaponToDraw < WP_NUM_WEAPONS; weaponToDraw++ )
	{
		if ( (cg.snap->ps.stats[STAT_WEAPONS] & ( 1 << weaponToDraw )) )
		{
			w = weaponlist[weaponToDraw - 1].width;

			// Retrieve the ammo value
			value = cg.predictedPlayerState.ammo[weaponData[weaponToDraw].attack[ATTACK_NORMAL].ammoIndex];

			// If the ammo value is crazy then dont display it
			if ( value > -1 )
			{
				// Build the ammo string padded with zeros
				ammo = va ( "%03d", value );

				// Retrive the width of the ammo string so it can be centered
				width = trap_R_GetTextWidth (ammo, font, 0.25f, 0 );
			}

			listwidth += w + weaponlist[weaponToDraw - 1].xadjust + weaponlist[weaponToDraw - 1].wadjust + width + 2;
		}
	}
	listscale = rect->w / listwidth;
	yscale = (rect->h - (rect->h * listscale)) / 2;
	if (listscale > 1.0f)
	{
		listscale = 1.0f;
		yscale = 0.0f;
	}

	x = rect->x;
	y = rect->y;
	for (weaponToDraw = WP_NONE + 1; weaponToDraw < WP_NUM_WEAPONS; weaponToDraw++ )
	{
		if ( (cg.snap->ps.stats[STAT_WEAPONS] & ( 1 << weaponToDraw )) )
		{
			vec4_t	weaponcolor;

			w = weaponlist[weaponToDraw - 1].width * listscale;
			h = weaponlist[weaponToDraw - 1].height * listscale;
			wadjust = weaponlist[weaponToDraw - 1].wadjust * listscale;
			xadjust = weaponlist[weaponToDraw - 1].xadjust * listscale;
			yadjust = weaponlist[weaponToDraw - 1].yadjust * listscale;
			cgs.teamData.redShade[3] = 1.0f;
			cgs.teamData.blueShade[3] = 1.0f;
			if ( weaponToDraw == cg.weaponSelect )
			{
				if ( flags & CG_SHOW_ONBLUETEAM )
				{
					VectorCopy( cgs.teamData.blueShade, weaponcolor );
				}
				else if ( flags & CG_SHOW_ONREDTEAM )
				{
					VectorCopy( cgs.teamData.redShade, weaponcolor );
				}
				else
				{
					VectorCopy( color, weaponcolor );
				}
			}
			else
			{
				VectorCopy( colorGrey, weaponcolor );
			}

			if ( cg.predictedPlayerState.clip[ATTACK_NORMAL][weaponToDraw] > 0 )
			{
				weaponcolor[3] = 1.0f;
			}
			else
			{
				weaponcolor[3] = 0.4f;
			}

			trap_R_SetColor ( weaponcolor );

			// Draw the icon
			CG_DrawPic ( x + xadjust, y + yadjust + yscale, w, h, cg_weapons[weaponToDraw].weaponIcon );

			// Back to no color
			trap_R_SetColor ( NULL );

			// Retrieve the ammo value
			value = cg.predictedPlayerState.clip[ATTACK_NORMAL][weaponToDraw];

			// If the ammo value is crazy then dont display it
			if ( value >= 0 )
			{
				vec4_t ammocolor;

				VectorCopy( colorWhite, ammocolor );
				ammocolor[3] = 1.0f;
				if ( weaponToDraw == cg.weaponSelect && value <= (weaponData[weaponToDraw].attack[ATTACK_NORMAL].clipSize / 6) )
				{
					VectorSet ( ammocolor, 1.0f, 0.2f, 0.2f );
				}
				else if ( weaponToDraw != cg.weaponSelect && value == 0 )
				{
					ammocolor[3] = 0.5f;
				}

				// Build the clip string padded with zeros
				ammo = va ( "%02d", value );

				CG_DrawText (x + w + xadjust + wadjust, y + yscale + 2 * listscale, font, 0.38f * listscale, ammocolor, ammo, 0, 0 ); // 01-23-05 pimpy from 0.25f
			}

			// Retrieve the ammo value
			value = cg.predictedPlayerState.ammo[weaponData[weaponToDraw].attack[ATTACK_NORMAL].ammoIndex];

			// If the ammo value is crazy then dont display it
			if ( value >= 0 )
			{
				vec4_t ammocolor;

				VectorCopy( colorWhite, ammocolor );
				ammocolor[3] = 1.0f;
				if ( weaponToDraw != cg.weaponSelect && value == 0 )
				{
					ammocolor[3] = 0.5f;
				}

				// Build the ammo string padded with zeros
				ammo = va ( "%03d", value );

				// Retrive the width of the ammo string so we can add it to the list length
				width = trap_R_GetTextWidth (ammo, font, 0.25f * listscale, 0 );

				CG_DrawText (x + w + xadjust + wadjust, y + yscale + 13 * listscale, font, 0.25f * listscale, ammocolor, ammo, 0, 0 );
			}

			// If the selected weapon doesn't have an altclip, don't draw. 01-23-05 pimpy
			category = weaponData[weaponToDraw].category;
			if ( weaponData[weaponToDraw].attack[ATTACK_ALTERNATE].clipSize>=1 && ( CAT_ASSAULT == category ) )
			{

				// Retrieve the ammo value //alt ammo
				value  = cg.predictedPlayerState.ammo [weaponData[weaponToDraw].attack[ATTACK_ALTERNATE].ammoIndex];
				value += cg.predictedPlayerState.clip[ATTACK_ALTERNATE][weaponToDraw];

				// If the ammo value is crazy then dont display it
				if ( value >= 1 )
				{
					// Build the ammo string padded with zeros
					ammo = va ( "%03d", value );

					CG_DrawText (x + xadjust + wadjust + 9 * listscale, y + yscale + 15 * listscale, font, 0.25f * listscale, colorWhite, ammo, 0, 0 );
				}
			}
			// Use the current fire mode for this weapon, if its undefined then find what it will
			// be once its set
			curMode = cg.predictedPlayerState.firemode[cg.weaponSelect];
			if ( curMode == WP_FIREMODE_NONE )
			{
				curMode = BG_FindFireMode ( cg.weaponSelect, ATTACK_NORMAL, WP_FIREMODE_AUTO );
			}

			switch ( curMode )
			{
				case WP_FIREMODE_NONE:
					mode = "";
					break;
				case WP_FIREMODE_AUTO:
					mode = "auto";
					break;
				case WP_FIREMODE_BURST:
					mode = "burst";
					break;
				case WP_FIREMODE_SINGLE:
					mode = "single";
					break;
#ifdef SMKWEAPONS
				case WP_FIREMODE_20MM:
					mode = "20mm";
					break;
#endif
			}
			// Get the width of the text so we can center it
			if ( weaponToDraw == cg.weaponSelect && ( CAT_GRENADE != category ) && ( CAT_KNIFE != category ) )
			{
				CG_DrawText (x + w + xadjust + wadjust - 32 * listscale, y + yscale + 10 * listscale, font, 0.25f * listscale, colorWhite, mode, 0, DT_OUTLINE );
			}//end pimpy

			x += w + xadjust + wadjust + width + 2 * listscale;
		}
	}
}

//==============================================================================
//	CG_DrawUseIcon
//==============================================================================
void CG_DrawUseIcon ( rectDef_t* rect )
{
	// If not in a use zone then dont bother
	if ( !(cg.predictedPlayerState.pm_flags & PMF_CAN_USE ) )
	{
		return;
	}

	CG_DrawStretchPic ( rect->x, rect->y, rect->w, rect->h,0, 0, 1, 1, NULL,
						cgs.gameIcons [ cg.predictedPlayerState.stats[STAT_USEICON] ] );

	if ( cg.predictedPlayerState.stats[STAT_USETIME] )
	{
		float w = 98.0f * (float)cg.predictedPlayerState.stats[STAT_USETIME] / (float)cg.predictedPlayerState.stats[STAT_USETIME_MAX];
		CG_DrawRect ( rect->x + rect->h + 10, rect->y + 9, 100, rect->h - 18, 1, colorWhite );
		CG_FillRect ( rect->x + rect->h + 11, rect->y + 10, w, rect->h - 20, colorRed );
		CG_DrawRect ( rect->x + rect->h + 11, rect->y + 10, w, rect->h - 20, 1, colorBlack );
	}
}

//==============================================================================
//	CG_DrawPlayerHudBackground
//==============================================================================
void CG_DrawPlayerHudBackground ( rectDef_t* rect, vec4_t color, int flags )
{
	if ( flags & CG_SHOW_ONBLUETEAM )
	{
		VectorCopy( cgs.teamData.blueShade, color );
	}
	else if ( flags & CG_SHOW_ONREDTEAM )
	{
		VectorCopy( cgs.teamData.redShade, color );
	}
	color[3] = 0.6f;

	trap_R_SetColor ( color );
	CG_DrawPic ( rect->x, rect->y, rect->w, rect->h, cgs.media.hudcolShader );

	color[0] = 1.0f;
	color[1] = 1.0f;
	color[2] = 1.0f;
	color[3] = 0.5f;
	trap_R_SetColor ( color );
	CG_DrawPic ( rect->x, rect->y, rect->w, rect->h, cgs.media.hudwpShader );

	trap_R_SetColor ( NULL );
}

//==============================================================================
//	CG_DrawPlayerGametypeItems
//==============================================================================
void CG_DrawPlayerGametypeItems ( rectDef_t* rect )
{
	float x;
	int	  i;

	// If not in a use zone then dont bother
	if ( cg.predictedPlayerState.pm_flags & PMF_CAN_USE )
	{
		return;
	}

	x = rect->x;

	for ( i = 0; i < MAX_GAMETYPE_ITEMS; i ++ )
	{
		// Doesnt have this mission item
		if ( !(cg.predictedPlayerState.stats[STAT_GAMETYPE_ITEMS] & (1<<i)) )
		{
			continue;
		}

		if ( !cg_items[i+MODELINDEX_GAMETYPE_ITEM].icon )
		{
			continue;
		}

		CG_DrawPic ( x, rect->y, rect->w, rect->h, cg_items[i+MODELINDEX_GAMETYPE_ITEM].icon );

		x += 2;
	}
}

//==============================================================================
//	CG_DrawPlayerArmor
//==============================================================================
void CG_DrawPlayerArmor ( rectDef_t* rect )
{
	float x;
	float w;

	// Draw the icon
	trap_R_SetColor ( NULL );

	w = rect->w;
	x = rect->x;

	for ( ; w >= rect->h; w -= rect->h, x += rect->h )
	{
		CG_DrawPic ( x, rect->y - rect->h, rect->h, rect->h, cgs.media.armorShader );
	}

	if ( w )
	{
		CG_DrawStretchPic ( x, rect->y - rect->h, w, rect->h,
							0, 0, w/rect->h, 1.0f, NULL, cgs.media.armorShader );
	}
}

//==============================================================================
//	CG_DrawPlayerHealth
//==============================================================================
void CG_DrawPlayerHealth ( rectDef_t* rect, int flags )
{
	cgs.teamData.redShade[3] = 1.0f;
	cgs.teamData.blueShade[3] = 1.0f;
	if ( flags & CG_SHOW_ONBLUETEAM )
	{
		CG_FillRect ( rect->x, rect->y, rect->w * CG_GetValue( CG_PLAYER_HEALTH ), rect->h, cgs.teamData.blueShade );
	}
	else if ( flags & CG_SHOW_ONREDTEAM )
	{
		CG_FillRect ( rect->x, rect->y, rect->w * CG_GetValue( CG_PLAYER_HEALTH ), rect->h, cgs.teamData.redShade );
	}
}

//==============================================================================
//	CG_DrawPlayerArmorText
//==============================================================================
void CG_DrawPlayerArmorText ( rectDef_t *rect, qhandle_t font, float scale, vec4_t color  )
{
	float width;
	const char *value;
	playerState_t	*ps;

	ps = &cg.snap->ps;
	value = va ( "%i", ps->stats[STAT_ARMOR] );

	// Get the width of the text so we can right-justify it
	width = trap_R_GetTextWidth ( value, font, scale, 0 );

	if ( cg_drawEnhancedHUD.integer == 1 && !cg_drawNumericHealth.integer ) // 1-26-05 pimpy
	{
		// Right justify the text
		CG_DrawText ( rect->x + (rect->w - width), rect->y, font, scale, color, value, 0, DT_OUTLINE );
	}
	else
	{
		// Left justify the text.
		CG_DrawText ( rect->x, rect->y, font, scale, color, value, 0, DT_OUTLINE );
	}
}

//==============================================================================
//	CG_DrawPlayerHealthText
//==============================================================================
void CG_DrawPlayerHealthText ( rectDef_t *rect, qhandle_t font, float scale, vec4_t color  )
{
	float width;
	const char *value;
	playerState_t	*ps;

	ps = &cg.snap->ps;
	value = va ( "%i", ps->stats[STAT_HEALTH] );

	// Get the width of the text so we can right-justify it
	width = trap_R_GetTextWidth ( value, font, scale, 0 );

	if ( cg_drawEnhancedHUD.integer == 1 && !cg_drawNumericHealth.integer ) // 1-26-05 pimpy
	{
		// Right justify the text
		CG_DrawText ( rect->x + (rect->w - width), rect->y, font, scale, color, value, 0, DT_OUTLINE );
	}
	else
	{
		// Left justify the text.
		CG_DrawText ( rect->x, rect->y, font, scale, color, value, 0, DT_OUTLINE );
	}
}

//==============================================================================
//	CG_DrawClock
//==============================================================================
static void CG_DrawClock ( rectDef_t *rect, qhandle_t font, float scale  )
{
	float			width;
	qtime_t			currtime;
	char			time[16];

	trap_RealTime (&currtime);

	if (currtime.tm_hour > 12)
	{
		Com_sprintf(time, 16, "%2i:%02ipm", currtime.tm_hour - 12, currtime.tm_min);
	}
	else
	{
		if ( currtime.tm_hour == 0 )
		{
			currtime.tm_hour = 12;
		}
		Com_sprintf(time, 16, "%2i:%02iam", currtime.tm_hour, currtime.tm_min);
	}

	// Get the width of the text so we can right-justify it
	width = trap_R_GetTextWidth ( time, font, scale, 0 );

	// Center the text
	CG_DrawText ( rect->x - width, rect->y, font, scale, g_color_table[ColorIndex(COLOR_GREEN)], time, 0, DT_OUTLINE );
}

//==============================================================================
//	CG_DrawTimeLeft
//==============================================================================

static void CG_DrawTimeLeft ( rectDef_t *rect, qhandle_t font, float scale  )
{
	float			width;
	float			height;
	char			*text;
	char			*time;
	int				mins;
	int				tens;
	int				seconds;

//	if ( !cgs.levelTimeLimit ) {
//		return;
//	}

	seconds = (cgs.levelTimeLimit - (cg.time - cgs.levelStartTime)) / 1000;
	mins = seconds / 60;
	seconds -= mins * 60;
	tens = seconds / 10;
	seconds -= tens * 10;

	text = "Time Left";

	if ( cgs.levelTimeLimit && seconds >= 0 ) {
		time = va("%i:%i%i", mins, tens, seconds);
	}
	else {
		time = "--:-- ";
	}

	width = trap_R_GetTextWidth ( text, font, 0.3, 0 );
	height = trap_R_GetTextHeight ( text, font, 0.3, 0 );
	CG_DrawText ( rect->x - width, rect->y, font, 0.3, g_color_table[ColorIndex(COLOR_GREEN)], text, 0, DT_OUTLINE );
	width = trap_R_GetTextWidth ( time, font, scale, 0 );
	CG_DrawText ( rect->x - width, rect->y + height, font, scale, g_color_table[ColorIndex(COLOR_GREEN)], time, 0, DT_OUTLINE );
}

//==============================================================================
//	CG_DrawInMatch
//==============================================================================

static void CG_DrawInMatch ( rectDef_t *rect, qhandle_t font, float scale  )
{
	float			width;
	float			height;
	char			*text;

	text = "Match Mode";

	width = trap_R_GetTextWidth ( text, font, scale, 0 );
	height = trap_R_GetTextHeight ( text, font, scale, 0 );
	CG_DrawText ( rect->x - width, rect->y, font, scale, g_color_table[ColorIndex(COLOR_RED)], text, 0, DT_OUTLINE );
}

static void CG_DrawPlayerScore( rectDef_t *rect, qhandle_t font, float scale, vec4_t color )
{
	const char* s;

	s = va("%03d",cg.snap->ps.persistant[PERS_SCORE]);

	CG_DrawText(rect->x + (rect->w - trap_R_GetTextWidth(s,font,scale,0)) / 2, rect->y + rect->h, font, scale, color, s, 0, 0 );
}

static void CG_DrawRedScore(rectDef_t *rect, qhandle_t font, float scale, vec4_t color )
{
	int  value;
	char num[16];

	if ( cgs.scores1 == SCORE_NOT_PRESENT )
	{
		Com_sprintf (num, sizeof(num), "-");
	}
	else
	{
		Com_sprintf (num, sizeof(num), "%i", cgs.scores1);
	}

	value = trap_R_GetTextWidth (num, font, scale, 0);

	CG_DrawText (rect->x + rect->w - value, rect->y + rect->h, font, scale, color, num, 0, 0 );
}

static void CG_DrawBlueScore(rectDef_t *rect, qhandle_t font, float scale, vec4_t color )
{
	int  value;
	char num[16];

	if ( cgs.scores2 == SCORE_NOT_PRESENT )
	{
		Com_sprintf (num, sizeof(num), "-");
	}
	else
	{
		Com_sprintf (num, sizeof(num), "%i", cgs.scores2);
	}

	value = trap_R_GetTextWidth(num, font, scale, 0 );
	CG_DrawText(rect->x + rect->w - value, rect->y + rect->h, font, scale, color, num, 0, 0 );
}

#define MSECS_PER_FRAME (1000.0 / 60.0)
#define HUD_FADE_SPEED 0.05

float TimeAdjustHUDVal(float mainVal, float dstVal, float factor)
{
	mainVal = mainVal * (1.0 - factor) + dstVal * factor;

	//once we get close enough, just finish up
	if(fabs(mainVal - dstVal) < .01)
	{	// if we don't have this, the last few pixels take FOREVER
		mainVal = dstVal;
	}

	return mainVal;
}

void HandleHUDSegment(float *floorVal, float *ceilVal, float dstVal, float factor)
{
	if(dstVal < *floorVal)
	{
		*floorVal = dstVal;
	}
	else if(dstVal > *floorVal)
	{
		*floorVal = TimeAdjustHUDVal(*floorVal, dstVal, factor);
	}

	if(dstVal > *ceilVal)
	{
		*ceilVal = dstVal;
	}
	else if(dstVal < *ceilVal)
	{
		*ceilVal = TimeAdjustHUDVal(*ceilVal, dstVal, factor);
	}
}

float CG_GetValue(int ownerDraw)
{
	centity_t	*cent;
	playerState_t	*ps;

	cent = CG_GetEntity ( cg.snap->ps.clientNum );
	ps = &cg.snap->ps;

	switch (ownerDraw)
	{
		case CG_PLAYER_WEAPON_AMMO:
			if ( cent->currentState.weapon )
			{
				return ps->ammo[weaponData[cent->currentState.weapon].attack[ATTACK_NORMAL].ammoIndex];
			}
			break;

		case CG_PLAYER_SCORE:
			return cg.snap->ps.persistant[PERS_SCORE];

		case CG_PLAYER_HEALTH:
			if ( cg_drawEnhancedHUD.integer )
			{
				return ((float) ps->stats[STAT_HEALTH] * 0.01f);
			}
			else
			{
				return ((float) ps->stats[STAT_HEALTH] * 0.01f) + 0.01f;
			}

		case CG_PLAYER_HEALTH_FADE:
		{
			static float healthEdge = 0.0;
			static float healthFull = 0.0;

			float curHealth    = ps->stats[STAT_HEALTH] * 0.01;
			float adjustFactor = (cg.frametime / MSECS_PER_FRAME) * HUD_FADE_SPEED;

			HandleHUDSegment(&healthEdge, &healthFull, curHealth, adjustFactor);

			if ( healthFull < 0 )
				healthFull = 0;

			return healthFull;
		}

		case CG_PLAYER_ARMOR_FADE:
		{
			static float armorEdge = 0.0;
			static float armorFull = 0.0;

			float curArmor     = ps->stats[STAT_ARMOR] * 0.01;
			float adjustFactor = (cg.frametime / MSECS_PER_FRAME) * HUD_FADE_SPEED;

			HandleHUDSegment(&armorEdge, &armorFull, curArmor, adjustFactor);

			if ( armorFull < 0 )
				armorFull = 0;

			return armorFull;
		}

		case CG_PLAYER_ARMOR:
			return ((float) ps->stats[STAT_ARMOR] * 0.01f);

		case CG_RED_SCORE:
			return cgs.scores1;
			break;

		case CG_BLUE_SCORE:
			return cgs.scores2;
			break;

		default:
			break;
	}

	return -1;
}

qboolean CG_OwnerDrawDisabled(int flags, const char* param )
{
	return qfalse;
}

qboolean CG_OwnerDrawVisible(int flags, const char* param )
{
	qboolean visible = qtrue;

	while ( visible )
	{
		// Player on the red team?
		if ( flags & CG_SHOW_ONREDTEAM )
		{
			visible = cg.predictedPlayerState.persistant[PERS_TEAM]==TEAM_RED?qtrue:qfalse;
			flags &= ~(CG_SHOW_ONREDTEAM);
			continue;
		}
		// Player on the blue team?
		else if ( flags & CG_SHOW_ONBLUETEAM )
		{
			visible = cg.predictedPlayerState.persistant[PERS_TEAM]==TEAM_BLUE?qtrue:qfalse;
			flags &= ~(CG_SHOW_ONBLUETEAM);
		}
		else if ( flags & CG_SHOW_HUD_NIGHTVISION )
		{
			visible = ((cg.predictedPlayerState.pm_flags & PMF_GOGGLES_ON) && cg.predictedPlayerState.stats[STAT_GOGGLES] == GOGGLES_NIGHTVISION && !(cg.predictedPlayerState.pm_flags&PMF_ZOOMED))? qtrue : qfalse;
			flags &= ~(CG_SHOW_HUD_NIGHTVISION);
		}
		else if ( flags & CG_SHOW_HUD_THERMAL )
		{
			visible = ((cg.predictedPlayerState.pm_flags & PMF_GOGGLES_ON) && cg.predictedPlayerState.stats[STAT_GOGGLES] == GOGGLES_INFRARED )? qtrue : qfalse;
			flags &= ~(CG_SHOW_HUD_THERMAL);
		}
		// Only draw the sniper scope when zoomed
		else if ( flags & CG_SHOW_HUD_SNIPERSCOPE )
		{
			visible = (cg.predictedPlayerState.pm_flags&PMF_ZOOMED)?qtrue:qfalse;
			flags &= ~(CG_SHOW_HUD_SNIPERSCOPE);
		}
		// Only draw the sniper clip when zoomed
		else if ( flags & CG_SHOW_HUD_SNIPERCLIP )
		{
			if ( cg.predictedPlayerState.weapon == WP_MSG90A1 )
			{
				visible = (cg.predictedPlayerState.pm_flags&PMF_ZOOMED)?qtrue:qfalse;
				flags &= ~(CG_SHOW_HUD_SNIPERCLIP);
			}
			else
			{
				visible = qfalse;
			}
		}
		// Draw the health as long as we arent zoomed
		else if ( flags & CG_SHOW_HUD_HEALTH )
		{
			visible = cg.showScores ? qfalse : qtrue;
			flags &= ~(CG_SHOW_HUD_HEALTH);
		}
		// Timer
		else if ( flags & CG_SHOW_HUD_TIMER )
		{
			if ( cg.showScores )
			{
				visible = qfalse;
			}
			else if ( !cg_drawTimer.integer )
			{
				visible = qfalse;
			}

			flags &= ~(CG_SHOW_HUD_TIMER);
		}
		// Clock
		else if ( flags & CG_SHOW_HUD_CLOCK )
		{
			if ( cg.showScores )
			{
				visible = qfalse;
			}
			else if ( !cg_drawClock.integer )
			{
				visible = qfalse;
			}

			flags &= ~(CG_SHOW_HUD_CLOCK);
		}
		// Time Left
		else if ( flags & CG_SHOW_HUD_TIMELEFT )
		{
			if ( cg.showScores )
			{
				visible = qfalse;
			}
			else if ( !cg_drawTimeLeft.integer )
			{
				visible = qfalse;
			}

			flags &= ~(CG_SHOW_HUD_TIMELEFT);
		}
		// Match
		else if ( flags & CG_SHOW_HUD_INMATCH )
		{
			if ( cg.showScores )
			{
				visible = qfalse;
			}
			else if ( !inMatch.integer )
			{
				visible = qfalse;
			}

			flags &= ~(CG_SHOW_HUD_INMATCH);
		}
		// Should the alternate weapon information be shown?
		else if (flags & (CG_SHOW_PLAYER_ALT_WEAPONINFO|CG_HIDE_PLAYER_ALT_WEAPONINFO) )
		{
			if ( cg.predictedPlayerState.weapon == WP_MSG90A1 )
			{
				if ( cg.predictedPlayerState.pm_flags & PMF_ZOOMED )
				{
					visible = qfalse;
				}
			}

			if ( cg.showScores )
			{
				visible = qfalse;
			}
			else
			{
				if ( !BG_WeaponHasAlternateAmmo ( cg.weaponSelect ) )
				{
					visible = qfalse;
				}

				// Dont show it if we dont have ammo
				if ( !cg.predictedPlayerState.ammo[ weaponData[cg.weaponSelect].attack[ATTACK_ALTERNATE].ammoIndex ] &&
					 !cg.predictedPlayerState.clip[ATTACK_ALTERNATE][ cg.weaponSelect ] )
				{
					visible = qfalse;
				}

				// INvert the visible flag for hidden
				if ( flags & CG_HIDE_PLAYER_ALT_WEAPONINFO )
				{
					if ( cg.predictedPlayerState.weapon == WP_MSG90A1 && (cg.predictedPlayerState.pm_flags & PMF_ZOOMED ) )
					{
						visible = qfalse;
					}
					else
					{
						visible = !visible;
					}
				}
			}

			flags &= ~(CG_SHOW_PLAYER_ALT_WEAPONINFO|CG_HIDE_PLAYER_ALT_WEAPONINFO);
		}
		// Draw the weapon info when we arent zoomed
		else if ( flags & CG_SHOW_HUD_WEAPONINFO )
		{
			qboolean zoomed = qfalse;
			if ( cg.predictedPlayerState.weapon == WP_MSG90A1 && (cg.predictedPlayerState.pm_flags & PMF_ZOOMED ) )
			{
				zoomed = qtrue;
			}
			visible = (cg.showScores||zoomed)?qfalse:qtrue;
			flags &= ~(CG_SHOW_HUD_WEAPONINFO);
		}
		else if (flags & CG_SHOW_ANYTEAMGAME)
		{
			if( !cgs.gametypeData->teams )
			{
				visible = qfalse;
			}

			flags &= ~(CG_SHOW_ANYTEAMGAME);
		}
		else if (flags & CG_SHOW_ANYNONTEAMGAME)
		{
			if( cgs.gametypeData->teams )
			{
				visible = qfalse;
			}

			flags &= ~(CG_SHOW_ANYNONTEAMGAME);
		}
		else
		{
			break;
		}
	}

	return visible;
}

static void CG_Draw1stPlace(rectDef_t *rect, qhandle_t font, float scale, vec4_t color )
{
	if (cgs.scores1 != SCORE_NOT_PRESENT)
	{
		CG_DrawText(rect->x, rect->y, font, scale, color, va("%2i", cgs.scores1),0, 0 );
	}
}

static void CG_Draw2ndPlace(rectDef_t *rect, qhandle_t font, float scale, vec4_t color )
{
	if (cgs.scores2 != SCORE_NOT_PRESENT)
	{
		CG_DrawText(rect->x, rect->y, font, scale, color, va("%2i", cgs.scores2),0, 0 );
	}
}

// Draw an entire row of weapons for the visual weapon selection menu

static void CG_DrawWeaponList(rectDef_t *rect, qhandle_t font, float scale, vec4_t color )
{
	static vec4_t	colorNormal		= { 0.5f, 0.5f, 0.5f, 0.8f };
	static vec4_t	colorShadow		= { 0.0f, 0.0f, 0.0f, 0.25f };
	static vec4_t	colorSelected	= { 204.0f / 255.0f, 222.0f / 255.0f, 159.0f / 255.0f, 0.925f };
	static vec4_t	colorDisabled   = { 0.4f, 0.4f, 0.4f, 0.8f };

	centity_t		*cent;
	playerState_t	*ps;
	qhandle_t		icon;
	int				weaponToDraw;
	int			x,y;
	int			w,h;
	int			i,wid;
	int			category, activeCategory;
	char		numStr[3];
	int			category_x[CAT_MAX];
	gitem_t		*item;
	const char	*s;


	vec4_t		colorDraw;

	rect->x = 20;
	rect->y = 65;
	rect->h = 320;

	// are we showing weapon menu?
	if (!cg.weaponMenuUp)
	{
		return;
	}

	cent = CG_GetEntity ( cg.snap->ps.clientNum );
	ps   = &cg.snap->ps;

	// If the menu was up during follow then turn it off
	if ( (ps->pm_flags & PMF_FOLLOW) || ps->pm_type == PM_SPECTATOR )
	{
		cg.weaponMenuUp = qfalse;
		return;
	}


	h = rect->h / CAT_MAX;
	w = h*2;  // we know that the weapon icons are a 2:1 aspect ratio

	activeCategory = -1;

	// start at left edge for all weapon categories
	for (i=CAT_NONE; i<CAT_MAX; i++)
	{
		category_x[i] = rect->x + w;
	}

	for (weaponToDraw = WP_NONE + 1; weaponToDraw < WP_NUM_WEAPONS; weaponToDraw++ )
	{
		category = weaponData[weaponToDraw].category;

		x = category_x[category];
		y = rect->y + (category-1) * h;

		// found a weapon in this catagory
		if (weaponToDraw == cg.weaponMenuSelect)
		{
			// this is the currently selected weapon, so draw in green
			Vector4Copy(colorSelected, colorDraw);
			activeCategory = category;
		}
		else if ( (ps->stats[STAT_WEAPONS] & ( 1 << weaponToDraw )) )
		{
			int ammoCount = 0;

			ammoCount += ps->ammo[weaponData[weaponToDraw].attack[ATTACK_NORMAL].ammoIndex];
			ammoCount += ps->clip[ATTACK_NORMAL][weaponToDraw];
			ammoCount += ps->ammo[weaponData[weaponToDraw].attack[ATTACK_ALTERNATE].ammoIndex];
			ammoCount += ps->clip[ATTACK_ALTERNATE][weaponToDraw];

			if ( !ammoCount )
			{
				if (CAT_GRENADE <= category)
				{
					// no grenades means no grenades
					continue;
				}
				else
				{
					// weapon has no ammo so show it disabled
					Vector4Copy(colorDisabled, colorDraw );
				}
			}
			else
			{
				// draw in normal color
				Vector4Copy(colorNormal, colorDraw);
			}
		}
		else
		{	// don't have it, so don't draw
			continue;
		}

		icon = cg_weapons[ weaponToDraw ].weaponIcon;
		if (icon)
		{
			// draw weapon icon
			if (CAT_GRENADE <= category )
			{
				// draw square icons
				trap_R_SetColor( colorShadow );
				CG_DrawPic( x+2, y+2, h * 0.90f, h * 0.90f, icon );

				trap_R_SetColor( colorDraw );
				CG_DrawPic( x, y, h * 0.90f, h * 0.90f, icon );

				category_x[category] += h;
			}
			else
			{
				// draw rectangular icons
				trap_R_SetColor( colorShadow );
				CG_DrawPic( x+2, y+2, w * 0.90f, h * 0.90f, icon );

				trap_R_SetColor( colorDraw );
				CG_DrawPic( x, y, w * 0.90f, h * 0.90f, icon );

				category_x[category] += w;
			}
		}

		trap_R_SetColor( NULL );
	}

	// show weapon category numbers
	for (i = CAT_NONE + 1; i < CAT_MAX; i++)
	{
		y = rect->y + (i - 1) * h;
		Com_sprintf(numStr, sizeof(numStr),"%d", i );
		wid = trap_R_GetTextWidth (numStr, font, 1.25f, 0 );
		if (i == activeCategory)
		{
			Vector4Copy(colorSelected, colorDraw);
		}
		else
		{
			Vector4Copy(colorNormal, colorDraw);
		}

		colorDraw[3] = 1.0f;

		CG_DrawText (rect->x + (w - wid) / 2 + 2, y, font, 0.6f, colorDraw, numStr, 0, DT_DROPSHADOW );
	}

	item = BG_FindWeaponItem ( cg.weaponMenuSelect );
	if ( item )
	{
		// Draw the selected weapons name
		s = va("Weapon: %s", item->pickup_name);
		w = trap_R_GetTextWidth ( s, cgs.media.hudFont, 0.43f, 0 );
		CG_DrawText ( 640 - rect->x - w, rect->y, cgs.media.hudFont, 0.43f, colorSelected, s, 0, 0  );

		// Draw the selected weapons ammo count
		if ( BG_WeaponHasAlternateAmmo ( cg.weaponMenuSelect ) )
		{
			s = va("Ammo: %i / %i", cg.predictedPlayerState.clip[ATTACK_NORMAL][cg.weaponMenuSelect] + cg.predictedPlayerState.ammo[weaponData[cg.weaponMenuSelect].attack[ATTACK_NORMAL].ammoIndex],
									cg.predictedPlayerState.clip[ATTACK_ALTERNATE][cg.weaponMenuSelect] + cg.predictedPlayerState.ammo[weaponData[cg.weaponMenuSelect].attack[ATTACK_ALTERNATE].ammoIndex]);
		}
		else
		{
			s = va("Ammo: %i", cg.predictedPlayerState.clip[ATTACK_NORMAL][cg.weaponMenuSelect] + cg.predictedPlayerState.ammo[weaponData[cg.weaponMenuSelect].attack[ATTACK_NORMAL].ammoIndex]);
		}

		w = trap_R_GetTextWidth ( s, cgs.media.hudFont, 0.43f, 0 );
		CG_DrawText ( 640 - rect->x - w, rect->y + 15, cgs.media.hudFont, 0.43f, colorSelected, s, 0, 0  );
	}
}

const char *CG_GetGameStatusText()
{
	const char *s = "";

	if ( !cgs.gametypeData->teams )
	{
		if (cg.snap->ps.persistant[PERS_TEAM] != TEAM_SPECTATOR )
		{
			s = va("%s place with %i",CG_PlaceString( cg.snap->ps.persistant[PERS_RANK] + 1 ),cg.snap->ps.persistant[PERS_SCORE] );
		}
	}
	else
	{
		if ( cg.teamScores[0] == cg.teamScores[1] )
		{
			s = va("Teams are tied at %i", cg.teamScores[0] );
		}
		else if ( cg.teamScores[0] >= cg.teamScores[1] )
		{
			s = va("%s^7 leads %s^7, %i to %i", cgs.teamData.redName, cgs.teamData.blueName, cg.teamScores[0], cg.teamScores[1] );
		}
		else
		{
			s = va("%s^7 leads %s^7, %i to %i", cgs.teamData.blueName, cgs.teamData.redName, cg.teamScores[1], cg.teamScores[0] );
		}
	}

	return s;
}

static void CG_DrawGameStatus(rectDef_t *rect, qhandle_t font, float scale, vec4_t color )
{
	CG_DrawText (rect->x, rect->y + rect->h, font, scale, color, CG_GetGameStatusText(), 0 , 0 );
}

const char *CG_GameTypeString()
{
	return cgs.gametypeData->displayName;
}

static void CG_DrawGameType(rectDef_t *rect, qhandle_t font, float scale, vec4_t color )
{
	CG_DrawText (rect->x, rect->y + rect->h, font, scale, color, CG_GameTypeString(), 0, 0  );
}

void CG_OwnerDraw ( float x, float y, float w, float h, float text_x, float text_y, int ownerDraw, int ownerDrawFlags, int align, float special, qhandle_t font, float scale, vec4_t color, qhandle_t shader, int textStyle, const char* param )
{
	rectDef_t rect;

	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;

	switch (ownerDraw)
	{
		case CG_HUD_CLOCK:
			CG_DrawClock ( &rect, font, scale );
			break;

		case CG_HUD_TIMELEFT:
			CG_DrawTimeLeft ( &rect, font, scale );
			break;

		case CG_HUD_INMATCH:
			CG_DrawInMatch ( &rect, font, scale );
			break;

		case CG_USE_ICON:
			CG_DrawUseIcon ( &rect );
			break;

		case CG_PLAYER_GAMETYPE_ITEMS:
			CG_DrawPlayerGametypeItems ( &rect );
			break;

		case CG_PLAYER_HUD:
			CG_DrawPlayerHudBackground ( &rect, color, ownerDrawFlags );
			break;

		case CG_PLAYER_ARMOR:
			CG_DrawPlayerArmor ( &rect );
			break;

		case CG_PLAYER_HEALTH:
			CG_DrawPlayerHealth ( &rect, ownerDrawFlags );
			break;

		case CG_PLAYER_ARMOR_TEXT:
			CG_DrawPlayerArmorText ( &rect, font, scale, color );
			break;

		case CG_PLAYER_HEALTH_TEXT:
			CG_DrawPlayerHealthText ( &rect, font, scale, color );
			break;

		case CG_PLAYER_WEAPON_LIST:
			CG_DrawPlayerWeaponList ( &rect, font, color, ownerDrawFlags );
			break;

		case CG_PLAYER_WEAPON_IMAGE:
			CG_DrawPlayerWeaponImage ( &rect, color, ownerDrawFlags );
			break;

		case CG_PLAYER_WEAPON_AMMO:
			CG_DrawPlayerWeaponAmmo(&rect, font, scale, color );
			break;

		case CG_PLAYER_WEAPON_CLIP:
			CG_DrawPlayerWeaponClip(&rect, font, scale, color );
			break;

		case CG_PLAYER_WEAPON_NAME:
			CG_DrawPlayerWeaponName(&rect, font, scale, color );
			break;

		case CG_PLAYER_ALT_WEAPON_NAME:
			CG_DrawPlayerAltWeaponName(&rect, font, scale, color );
			break;

		case CG_PLAYER_ALT_WEAPON_AMMO:
			CG_DrawPlayerAltWeaponAmmo(&rect, font, scale, color );
			break;

		case CG_PLAYER_ALT_WEAPON_AMMOICON:
			CG_DrawPlayerAltWeaponAmmoIcon ( &rect, color );
			break;

		case CG_PLAYER_WEAPON_MODE:
			CG_DrawPlayerWeaponMode(&rect, font, scale, color );
			break;

		case CG_PLAYER_SCORE:
			CG_DrawPlayerScore(&rect, font, scale, color );
			break;

		case CG_GAME_TYPE:
			CG_DrawGameType(&rect, font, scale, color );
			break;

		case CG_RED_SCORE:
			CG_DrawRedScore(&rect, font, scale, color );
			break;

		case CG_BLUE_SCORE:
			CG_DrawBlueScore(&rect, font, scale, color );
			break;

		case CG_GAME_STATUS:
			CG_DrawGameStatus(&rect, font, scale, color );
			break;

		case CG_1STPLACE:
			CG_Draw1stPlace(&rect, font, scale, color );
			break;

		case CG_2NDPLACE:
			CG_Draw2ndPlace(&rect, font, scale, color );
			break;

		case CG_WEAPON_LIST:
			CG_DrawWeaponList(&rect, font, scale, color );
			break;

		case CG_PLAYER_LOCATION:
			CG_DrawPlayerLocation(&rect, scale, color, textStyle);
			break;

		case CG_PLAYER_SNIPER_BULLET_1:
		case CG_PLAYER_SNIPER_BULLET_2:
		case CG_PLAYER_SNIPER_BULLET_3:
		case CG_PLAYER_SNIPER_BULLET_4:
		case CG_PLAYER_SNIPER_BULLET_5:
		case CG_PLAYER_SNIPER_BULLET_6:
			CG_DrawPlayerSniperBullet ( &rect, shader, ownerDraw - CG_PLAYER_SNIPER_BULLET_1 + 1);
			break;

		case CG_PLAYER_SNIPER_MAGNIFICATION:
			CG_DrawPlayerSniperMagnification ( &rect, font, scale, color );
			break;

		default:
			break;
	}
}

void CG_MouseEvent(int x, int y)
{
	int n;

	if ( (cg.predictedPlayerState.pm_type == PM_NORMAL || cg.predictedPlayerState.pm_type == PM_SPECTATOR) && cg.showScores == qfalse)
	{
	    trap_Key_SetCatcher(0);
		return;
	}

	cgs.cursorX+= x;
	if (cgs.cursorX < 0)
	{
		cgs.cursorX = 0;
	}
	else if (cgs.cursorX > 640)
	{
		cgs.cursorX = 640;
	}

	cgs.cursorY += y;
	if (cgs.cursorY < 0)
	{
		cgs.cursorY = 0;
	}
	else if (cgs.cursorY > 480)
	{
		cgs.cursorY = 480;
	}

	n = Display_CursorType(cgs.cursorX, cgs.cursorY);
	cgs.activeCursor = 0;
	if (n == CURSOR_ARROW)
	{
		cgs.activeCursor = cgs.media.cursor;
	}
	else if (n == CURSOR_SIZER)
	{
		cgs.activeCursor = cgs.media.cursor;
	}

	if (cgs.capturedItem)
	{
		Display_MouseMove(cgs.capturedItem, x, y);
	}
	else
	{
		Display_MouseMove(NULL, cgs.cursorX, cgs.cursorY);
	}
}

/*
==================
CG_EventHandling
==================
 type 0 - no event handling
      1 - team menu
      2 - hud editor

*/
void CG_EventHandling(int type)
{
	cgs.eventHandling = type;
}


void CG_KeyEvent(int key, qboolean down)
{
	if (!down)
	{
		return;
	}

	if ( cg.predictedPlayerState.pm_type == PM_NORMAL || (cg.predictedPlayerState.pm_type == PM_SPECTATOR && cg.showScores == qfalse))
	{
		CG_EventHandling(CGAME_EVENT_NONE);
		trap_Key_SetCatcher(0);
		return;
	}

	Display_HandleKey(key, down, cgs.cursorX, cgs.cursorY);

	if (cgs.capturedItem)
	{
		cgs.capturedItem = NULL;
	}
	else
	{
		if (key == K_MOUSE2 && down)
		{
			cgs.capturedItem = Display_CaptureItem(cgs.cursorX, cgs.cursorY);
		}
	}
}

void CG_RunMenuScript(const char **args)
{
}


void CG_GetTeamColor(vec4_t *color)
{
	if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED) {
		VectorCopy( cgs.teamData.redShade, *color );
		(*color)[3] = 0.25f;
//		(*color)[0] = 1.0f;
//		(*color)[1] = (*color)[2] = 0.0f;
	} else if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE) {
		VectorCopy( cgs.teamData.blueShade, *color );
		(*color)[3] = 0.25f;
//		(*color)[0] = (*color)[1] = 0.0f;
//		(*color)[2] = 1.0f;
	} else {
		(*color)[0] = (*color)[2] = 0.0f;
		(*color)[1] = 0.17f;
		(*color)[3] = 0.25f;
	}
}

