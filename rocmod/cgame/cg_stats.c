/*
===========================================================================
Copyright (C) 2003, Caleb Gingles

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

#include "cg_local.h"

#include "../ui/ui_shared.h"


#define FADE_INTERVAL	500

/* 
=================
CG_DrawStatLine
=================
*/
void CG_DrawStatLine( int time, char *title, char *name, int team, char *score, int x, int y, int w, int h, char *medalfile )
{
	float		width;
	float		fade;
	int			value;
	vec4_t		colorP = {1, 1, 1, 1};
	vec4_t		colorP2 = {1, 1, 1, 1};
	vec4_t		colorBg = {0, 0, 0, 0.9f};
	vec4_t		colorW = {1, 1, 1, 1};
	vec4_t		colorG = {169.0f/255.0f, 194.0f/255.0f, 116.0f/255.0f, 1};
	qhandle_t	medal;

	value = cg.bestStatsStart + (time * FADE_INTERVAL);
	if ( cg.time < value )
	{
		return;
	}
	value = cg.time - value;
	if ( value < 0 )
	{
		value = 0;
	}
	fade = value;
	fade /= 1000;
	if ( fade > 1.0f )
	{
		fade = 1.0f;
	}
	colorP[3] = fade;
	colorBg[3] = fade * 0.9f;
	colorW[3] = fade;
	cgs.teamData.redShade[3] = fade;
	cgs.teamData.blueShade[3] = fade;
	colorG[3] = fade;

	medal = trap_R_RegisterShaderNoMip( medalfile );

	CG_FillRect( x, y, w, h, colorBg );

	trap_R_SetColor ( colorP );
	CG_DrawPic( x, y, h / 2, h, medal );
	trap_R_SetColor ( colorP2 );

	CG_DrawText ( x + 22, y + 3, cgs.media.hudFont, 0.4f, colorW, title, 0, 0 );
	if ( team == TEAM_RED )
	{
		CG_DrawText ( x + 22, y + 20, cgs.media.hudFont, 0.35f, cgs.teamData.redShade, name, 24, 0 );
	}
	else if ( team == TEAM_BLUE )
	{
		CG_DrawText ( x + 22, y + 20, cgs.media.hudFont, 0.35f, cgs.teamData.blueShade, name, 24, 0 );
	}
	else
	{
		CG_DrawText ( x + 22, y + 20, cgs.media.hudFont, 0.35f, colorG, name, 24, 0 );
	}
	width = trap_R_GetTextWidth ( score, cgs.media.hudFont, 0.5f, 0 );
	CG_DrawText ( x + w - width - 2, y + 2, cgs.media.hudFont, 0.5f, colorW, score, 0, 0 );
}

/* 
=================
CG_DrawBestStats
=================
*/
void CG_DrawBestStats( void )
{
	char	*string;
	int		i;
	int		y;
	int		x;
	int		bestheadshots;
	int		bestkills;
	int		bestflagcaps;
	int		bestflagdefends;
	int		bestvalue;
	int		bestkillrate;
	int		bestkillspree;
	int		bestnadekills;
	int		bestmeleekills;
	float	bestaccuracy;
	float	bestratio;
	int		accm;
	int		accn;
	int		ratiom;
	int		ration;
	char	*headshotname;
	char	*killsname;
	char	*flagcapsname;
	char	*flagdefendsname;
	char	*accuracyname;
	char	*rationame;
	char	*valuename;
	char	*killratename;
	char	*killspreename;
	char	*nadekillsname;
	char	*meleekillsname;
	int		headshotteam;
	int		killsteam;
	int		flagcapsteam;
	int		flagdefendsteam;
	int		accuracyteam;
	int		ratioteam;
	int		valueteam;
	int		killrateteam;
	int		killspreeteam;
	int		nadekillsteam;
	int		meleekillsteam;
	int		starttime;

	x = 435;
	y = 45;
	starttime = 0;

	accm = 0;
	accn = 0;
	ratiom = 0;
	ration = 0;
	bestheadshots = 0;
	headshotname = "none";
	headshotteam = 0;
	bestkills = 0;
	killsname = "none";
	killsteam = 0;
	bestflagcaps = 0;
	flagcapsname = "none";
	flagcapsteam = 0;
	bestflagdefends = 0;
	flagdefendsname = "none";
	flagdefendsteam = 0;
	bestaccuracy = 0.0f;
	accuracyname = "none";
	accuracyteam = 0;
	bestratio = 0.0f;
	rationame = "none";
	ratioteam = 0;
	bestvalue = 0;
	valuename = "none";
	valueteam = 0;
	bestkillrate = 0;
	killratename = "none";
	killrateteam = 0;
	bestkillspree = 0;
	killspreename = "none";
	killspreeteam = 0;
	bestnadekills = 0;
	nadekillsname = "none";
	nadekillsteam = 0;
	bestmeleekills = 0;
	meleekillsname = "none";
	meleekillsteam = 0;

	for ( i = 0; i < cg.numScores; i++ )
	{
		if ( cg.bestKills != -1 && cg.scores[i].client == cg.bestKills )
		{
			bestkills = cg.scores[ i ].kills;
			killsname = cgs.clientinfo[ cg.bestKills ].name;
			killsteam = cgs.clientinfo[ cg.bestKills ].team;
		}
		if ( cg.bestHeadshots != -1 && cg.scores[i].client == cg.bestHeadshots )
		{
			bestheadshots = cg.scores[ i ].headshots;
			headshotname = cgs.clientinfo[ cg.bestHeadshots ].name;
			headshotteam = cgs.clientinfo[ cg.bestHeadshots ].team;
		}
		if ( cg.bestFlagCaps != -1 && cg.scores[i].client == cg.bestFlagCaps )
		{
			bestflagcaps = cg.scores[ i ].flagcaps;
			flagcapsname = cgs.clientinfo[ cg.bestFlagCaps ].name;
			flagcapsteam = cgs.clientinfo[ cg.bestFlagCaps ].team;
		}
		if ( cg.bestFlagDefends != -1 && cg.scores[i].client == cg.bestFlagDefends )
		{
			bestflagdefends = cg.scores[ i ].flagdefends;
			flagdefendsname = cgs.clientinfo[ cg.bestFlagDefends ].name;
			flagdefendsteam = cgs.clientinfo[ cg.bestFlagDefends ].team;
		}
		if ( cg.bestAccuracy != -1 && cg.scores[i].client == cg.bestAccuracy )
		{
			if ( cg.scores[ i ].misses )
			{
				bestaccuracy = cg.scores[ i ].hits;
				bestaccuracy /= ( cg.scores[ i ].hits + cg.scores[ i ].misses );
				bestaccuracy *= 100;
				accm = (int) bestaccuracy;
				bestaccuracy -= accm;
				bestaccuracy *= 10;
				accn = (int) bestaccuracy;
			}
			else
			{
				accm = 100;
			}
			accuracyname = cgs.clientinfo[ cg.bestAccuracy ].name;
			accuracyteam = cgs.clientinfo[ cg.bestAccuracy ].team;
		}
		if ( cg.bestRatio != -1 && cg.scores[i].client == cg.bestRatio )
		{
			bestratio = cg.scores[ i ].kills;
			if ( cg.scores[ i ].deaths )
			{
				bestratio /= cg.scores[ i ].deaths;
			}
			ratiom = (int) bestratio;
			bestratio -= ratiom;
			bestratio *= 10;
			ration = (int) bestratio;
			rationame = cgs.clientinfo[ cg.bestRatio ].name;
			ratioteam = cgs.clientinfo[ cg.bestRatio ].team;
		}
		if ( cg.bestValue != -1 && cg.scores[i].client == cg.bestValue )
		{
			bestvalue = (cg.scores[ i ].score / 2) + cg.scores[ i ].kills - cg.scores[ i ].deaths + cg.scores[ i ].headshots;
			if ( cg.scores[ i ].misses )
			{
				bestvalue *= cg.scores[ i ].hits;
				bestvalue /= ( cg.scores[ i ].hits + cg.scores[ i ].misses );
			}
			valuename = cgs.clientinfo[ cg.bestValue ].name;
			valueteam = cgs.clientinfo[ cg.bestValue ].team;
		}
		if ( cg.bestKillRate != -1 && cg.scores[i].client == cg.bestKillRate )
		{
			if ( cg.scores[ i ].time )
			{
				bestkillrate = cg.scores[ i ].kills * (60 / cg.scores[ i ].time);
				killratename = cgs.clientinfo[ cg.bestKillRate ].name;
				killrateteam = cgs.clientinfo[ cg.bestKillRate ].team;
			}
		}
		if ( cg.bestKillSpree != -1 && cg.scores[i].client == cg.bestKillSpree )
		{
			bestkillspree = cg.scores[ i ].killspree;
			killspreename = cgs.clientinfo[ cg.bestKillSpree ].name;
			killspreeteam = cgs.clientinfo[ cg.bestKillSpree ].team;
		}
		if ( cg.bestMeleeKills != -1 && cg.scores[i].client == cg.bestMeleeKills )
		{
			bestmeleekills = cg.scores[ i ].meleekills;
			meleekillsname = cgs.clientinfo[ cg.bestMeleeKills ].name;
			meleekillsteam = cgs.clientinfo[ cg.bestMeleeKills ].team;
		}
		if ( cg.bestNadeKills != -1 && cg.scores[i].client == cg.bestNadeKills )
		{
			bestnadekills = cg.scores[ i ].nadekills;
			nadekillsname = cgs.clientinfo[ cg.bestNadeKills ].name;
			nadekillsteam = cgs.clientinfo[ cg.bestNadeKills ].team;
		}
	}

	CG_DrawText ( x + 62,  y - 14, cgs.media.hudFont, 0.38f, colorLtGrey, "Best Players", 0, DT_OUTLINE );

	CG_DrawStatLine( starttime, "Overall Most Skilled", valuename, valueteam, va("%i", bestvalue), x, y, 200, 35, "gfx/menus/misc/ar_medal_untouched1.png");
	y += 37;
	starttime++;
	CG_DrawStatLine( starttime, "Most Kills", killsname, killsteam, va("%i", bestkills), x, y, 200, 35, "gfx/menus/misc/ar_medal_kill_all.png");
	y += 37;
	starttime++;
	CG_DrawStatLine( starttime, "Most Headshots", headshotname, headshotteam, va("%i", bestheadshots), x, y, 200, 35, "gfx/menus/misc/ar_medal_sniper1.png");
	y += 37;
	starttime++;
	CG_DrawStatLine( starttime, "Highest Ratio", rationame, ratioteam, va("%i.%i", ratiom, ration), x, y, 200, 35, "gfx/menus/misc/ar_medal_damage1.png");
	y += 37;
	starttime++;
	CG_DrawStatLine( starttime, "Highest Accuracy", accuracyname, accuracyteam, va("%i.%i%%", accm, accn), x, y, 200, 35, "gfx/menus/misc/ar_medal_sharpshooter1.png");
	y += 37;
	starttime++;
	if ( BG_RelatedGametype( cgs.gametype, "ctf" ) )
	{
		CG_DrawStatLine( starttime, "Most Flag Caps", flagcapsname, flagcapsteam, va("%i", bestflagcaps), x, y, 200, 35, "gfx/menus/misc/ar_medal_osprey.png");
		y += 37;
		starttime++;
		CG_DrawStatLine( starttime, "Most Flag Defends", flagdefendsname, flagdefendsteam, va("%i", bestflagdefends), x, y, 200, 35, "gfx/menus/misc/ar_medal_osprey.png");
		y += 37;
		starttime++;
	}
	else if ( BG_RelatedGametype( cgs.gametype, "inf" ) )
	{
		CG_DrawStatLine( starttime, "Most Briefcase Caps", flagcapsname, flagcapsteam, va("%i", bestflagcaps), x, y, 200, 35, "gfx/menus/misc/ar_medal_osprey.png");
		y += 37;
		starttime++;
		CG_DrawStatLine( starttime, "Most Briefcase Defends", flagdefendsname, flagdefendsteam, va("%i", bestflagdefends), x, y, 200, 35, "gfx/menus/misc/ar_medal_osprey.png");
		y += 37;
		starttime++;
	}
	CG_DrawStatLine( starttime, "Highest Kill Rate", killratename, killrateteam, va("%i", bestkillrate), x, y, 200, 35, "gfx/menus/misc/ar_medal_mission50.png");
	y += 37;
	starttime++;
	CG_DrawStatLine( starttime, "Highest Killing Spree", killspreename, killspreeteam, va("%i", bestkillspree), x, y, 200, 35, "gfx/menus/misc/ar_medal_mission10.png");
	y += 37;
	starttime++;
	CG_DrawStatLine( starttime, "Most Nade Kills", nadekillsname, nadekillsteam, va("%i", bestnadekills), x, y, 200, 35, "gfx/menus/misc/ar_medal_grenade1.png");
	y += 37;
	starttime++;
	CG_DrawStatLine( starttime, "Most Melee Kills", meleekillsname, meleekillsteam, va("%i", bestmeleekills), x, y, 200, 35, "gfx/menus/misc/ar_medal_knife1.png");

//	CG_DrawPic ( 385, y, w, sb_lineHeight, cgs.media.scoreboardLine );

	if ( !cgs.screenshotTaken &&
		(cg_autoScreenshot.integer == 2 || (inMatch.integer && cg_autoScreenshot.integer == 1)) &&
		cg.time >= (cg.bestStatsStart + (11 * FADE_INTERVAL) + 500) )
	{
		qtime_t currtime;

		trap_RealTime (&currtime);
		trap_SendConsoleCommand( va("screenshot match-%i%02i%02i-%02i%02i", currtime.tm_year + 1900, currtime.tm_mon + 1, currtime.tm_mday, currtime.tm_hour, currtime.tm_min) );
		cgs.screenshotTaken = qtrue;
	}

}
