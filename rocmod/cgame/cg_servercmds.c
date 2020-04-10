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
// cg_servercmds.c -- reliably sequenced text commands sent by the server
// these are processed at snapshot transition time, so there will definately
// be a valid snapshot this frame

#include "cg_local.h"
#include "../ui/menudef.h"
#if !defined(CL_LIGHT_H_INC)
	#include "cg_lights.h"
#endif
// #CL_ADD
#include "../ui/1fx_local.h"
// #END CL_ADD

/*
=======================
CG_AddChatText

Adds chat text to the chat chat buffer
=======================
*/
static void CG_AddChatText ( int client, const char *str )
{
	int		len;
	char	*p;
	char	*ls;
	int		lastcolor;
	int		chatHeight;
	float   w;
	float maxwidth;

	if ( (cg.overlayWidth && cg.overlayHeight < 380) || (cg.snap->ps.pm_type == PM_INTERMISSION && cg_drawBestStats.integer && cg.bestStats) )
	{
		maxwidth = 390;
	}
	else
	{
		maxwidth = 560;
	}

	if ( client >= 0 )
	{
		cgs.clientinfo[client].mLastChatTime = cg.time;
	}

	// Grab the users chat height settings
	chatHeight = cg_chatHeight.integer;
	if ( chatHeight > CHAT_HEIGHT )
	{
		chatHeight = CHAT_HEIGHT;
	}

	// Chats disabled?
	if ( chatHeight <= 0 || cg_chatTime.integer <= 0 )
	{
		cgs.chatPos = cgs.chatLastPos = 0;
		return;
	}

	len = 0;

	lastcolor = COLOR_WHITE;

	// Next position to write chat text too in the circular chat buffer
	p = cgs.chatText[cgs.chatPos % chatHeight];
	*p = 0;

	ls = NULL;
	w  = 0;

	while (*str)
	{
		float cw = trap_R_GetTextWidth ( va("%c",*str), cgs.media.hudFont, 0.43f, 0 );

		if ( w > maxwidth )
		{
			w = 0;

			if (ls)
			{
				str -= (p - ls);
				str++;
				p -= (p - ls);
			}

			*p = 0;

			cgs.chatTime[cgs.chatPos % chatHeight] = cg.time;

			cgs.chatPos++;
			p = cgs.chatText[ cgs.chatPos % chatHeight ];
			*p = 0;
			*p++ = Q_COLOR_ESCAPE;
			*p++ = lastcolor;
			len = 0;
			ls = NULL;
		}

		if ( Q_IsColorString( str ) )
		{
			*p++ = *str++;
			lastcolor = *str;
			*p++ = *str++;
			continue;
		}

		if (*str == ' ')
		{
			ls = p;
		}

		*p++ = *str++;
		len++;
		w += cw;
	}
	*p = 0;

	cgs.chatTime[ cgs.chatPos % chatHeight] = cg.time;
	cgs.chatPos++;

	if (cgs.chatPos - cgs.chatLastPos > chatHeight)
	{
		cgs.chatLastPos = cgs.chatPos - chatHeight;
	}
}

/*
=================
CG_ParseScores
=================
*/
static void CG_ParseScores( void )
{
	int		i;

	cg.scoreBoardSpectators[0] = '\0';

	cg.numScores = atoi( CG_Argv( 1 ) );
	if ( cg.numScores > MAX_CLIENTS )
	{
		cg.numScores = MAX_CLIENTS;
	}

	cg.teamScores[0] = atoi( CG_Argv( 2 ) );
	cg.teamScores[1] = atoi( CG_Argv( 3 ) );

	memset( cg.scores, 0, sizeof( cg.scores ) );
	for ( i = 0 ; i < cg.numScores ; i++ )
	{
		cg.scores[i].client = atoi( CG_Argv( i * 9 + 4 ) );
		cg.scores[i].score = atoi( CG_Argv( i * 9 + 5 ) );
		cg.scores[i].kills = atoi( CG_Argv( i * 9 + 6 ) );
		cg.scores[i].deaths = atoi( CG_Argv( i * 9 + 7 ) );
		cg.scores[i].ping = atoi( CG_Argv( i * 9 + 8 ) );
		cg.scores[i].time = atoi( CG_Argv( i * 9 + 9 ) );
		cgs.clientinfo[ cg.scores[i].client ].ghost = atoi( CG_Argv( i * 9 + 10 ) );
		cgs.clientinfo[ cg.scores[i].client ].gametypeitems = atoi( CG_Argv( i * 9 + 11 ) );
		cg.scores[i].teamkillDamage = atoi( CG_Argv( i * 9 + 12 ) );

		if ( cg.scores[i].client < 0 || cg.scores[i].client >= MAX_CLIENTS )
		{
			cg.scores[i].client = 0;
		}

		if ( cg.scores[i].ping < 0 )
		{
			cg.scores[i].time = 0;
		}

		cgs.clientinfo[ cg.scores[i].client ].score = cg.scores[i].score;
		cg.scores[i].team = cgs.clientinfo[cg.scores[i].client].team;

		if ( cg.scores[i].team == TEAM_SPECTATOR )
		{
			if ( cg.scoreBoardSpectators[0] )
			{
				strcat ( cg.scoreBoardSpectators, ", " );
			}

			strcat ( cg.scoreBoardSpectators, va("^7%s^7 (%d)", cgs.clientinfo[cg.scores[i].client].name, cg.scores[i].ping ) );
		}
	}
}

/*
=================
CG_ParseExtraScores

Extra scores used by old versions of the mod
=================
*/
static void CG_ParseExtraScores( void )
{
	int		i;

	cg.numScores = atoi( CG_Argv( 1 ) );
	if ( cg.numScores > MAX_CLIENTS )
	{
		cg.numScores = MAX_CLIENTS;
	}

	for ( i = 0 ; i < cg.numScores ; i++ )
	{
		cg.scores[i].adminref = atoi( CG_Argv( i * 5 + 2 ) );
		cg.scores[i].hits = atoi( CG_Argv( i * 5 + 3 ) );
		cg.scores[i].misses = atoi( CG_Argv( i * 5 + 4 ) );
		cg.scores[i].headshots = atoi( CG_Argv( i * 5 + 5 ) );
		cg.scores[i].flagcaps = atoi( CG_Argv( i * 5 + 6 ) );
		cg.scores[i].killspree = 0;
		cg.scores[i].meleekills = 0;
		cg.scores[i].nadekills = 0;
	}
}

/*
=================
CG_ParseExtraScores2

Extensible extra scores used with >1.6 versions of the mod
=================
*/
static void CG_ParseExtraScores2( void )
{
	int		i;
	int		statcount;

	statcount = atoi( CG_Argv( 1 ) );

	cg.numScores = atoi( CG_Argv( 2 ) );
	if ( cg.numScores > MAX_CLIENTS )
	{
		cg.numScores = MAX_CLIENTS;
	}

	for ( i = 0 ; i < cg.numScores ; i++ )
	{
		if (statcount > 0 ) cg.scores[i].adminref = atoi( CG_Argv( i * statcount + 3 ) );
		if (statcount > 1 ) cg.scores[i].hits = atoi( CG_Argv( i * statcount + 4 ) );
		if (statcount > 2 ) cg.scores[i].misses = atoi( CG_Argv( i * statcount + 5 ) );
		if (statcount > 3 ) cg.scores[i].headshots = atoi( CG_Argv( i * statcount + 6 ) );
		if (statcount > 4 ) cg.scores[i].flagcaps = atoi( CG_Argv( i * statcount + 7 ) );
		if (statcount > 5 ) cg.scores[i].killspree = atoi( CG_Argv( i * statcount + 8 ) );
		if (statcount > 6 ) cg.scores[i].meleekills = atoi( CG_Argv( i * statcount + 9 ) );
		if (statcount > 7 ) cg.scores[i].nadekills = atoi( CG_Argv( i * statcount + 10 ) );
	}
}

/*
=================
CG_ParseExtraScores3

Extensible extra scores used with >1.7 versions of the mod
=================
*/
static void CG_ParseExtraScores3( void )
{
	int		i;
	int		statcount;

	cgs.levelTimeLimit = atoi( CG_Argv( 1 ) ) * 60000;
	statcount = atoi( CG_Argv( 2 ) );

	cg.numScores = atoi( CG_Argv( 3 ) );
	if ( cg.numScores > MAX_CLIENTS )
	{
		cg.numScores = MAX_CLIENTS;
	}

	for ( i = 0 ; i < cg.numScores ; i++ )
	{
		if (statcount > 0 ) cg.scores[i].adminref = atoi( CG_Argv( i * statcount + 4 ) );
		if (statcount > 1 ) cg.scores[i].hits = atoi( CG_Argv( i * statcount + 5 ) );
		if (statcount > 2 ) cg.scores[i].misses = atoi( CG_Argv( i * statcount + 6 ) );
		if (statcount > 3 ) cg.scores[i].headshots = atoi( CG_Argv( i * statcount + 7 ) );
		if (statcount > 4 ) cg.scores[i].flagcaps = atoi( CG_Argv( i * statcount + 8 ) );
		if (statcount > 5 ) cg.scores[i].killspree = atoi( CG_Argv( i * statcount + 9 ) );
		if (statcount > 6 ) cg.scores[i].meleekills = atoi( CG_Argv( i * statcount + 10 ) );
		if (statcount > 7 ) cg.scores[i].nadekills = atoi( CG_Argv( i * statcount + 11 ) );
		if (statcount > 8 ) cg.scores[i].clan = atoi( CG_Argv( i * statcount + 12 ) );
		if (statcount > 9 ) cg.scores[i].flagdefends = atoi( CG_Argv( i * statcount + 13 ) );
	}
}

/*
=================
CG_ParseExtraTeamInfo
=================
*/
static void CG_ParseExtraTeamInfo( void )
{
	int		i;
	int		members;
	int		clientNum;

	members = atoi( CG_Argv( 1 ) );
	if ( members > MAX_CLIENTS )
	{
		members = MAX_CLIENTS;
	}

	for ( i = 0 ; i < members ; i++ )
	{
		clientNum = atoi( CG_Argv( i * 4 + 2 ) );
		cgs.clientinfo[clientNum].health = atoi( CG_Argv( i * 4 + 3 ) );
		cgs.clientinfo[clientNum].armor = atoi( CG_Argv( i * 4 + 4 ) );
		cgs.clientinfo[clientNum].weapon = atoi( CG_Argv( i * 4 + 5 ) );
	}
}

/*
=================
CG_ParseExtraTeamInfo2
=================
*/
static void CG_ParseExtraTeamInfo2( void )
{
	int			i;
	int			members;
	int			clientNum;
	const char	*entry;

	members = atoi( CG_Argv( 1 ) );
	if ( members > MAX_CLIENTS )
	{
		members = MAX_CLIENTS;
	}

	for ( i = 0 ; i < members ; i++ )
	{
		entry = CG_Argv( i + 2 );
		clientNum = _atoi( &entry );
		if ( entry[-1] == 'h' ) {
			cgs.clientinfo[clientNum].health = _atoi( &entry );
		}
		if ( entry[-1] == 'a' ) {
			cgs.clientinfo[clientNum].armor = _atoi( &entry );
		}
		if ( entry[-1] == 'w' ) {
			cgs.clientinfo[clientNum].weapon = _atoi( &entry );
		}
		if ( entry[-1] == 'l' ) {
			cgs.clientinfo[clientNum].location = _atoi( &entry );
		}
		if ( entry[-1] == 'g' ) {
			cgs.clientinfo[clientNum].ghost = _atoi( &entry );
		}
	}
}

/*
================
CG_ParseServerinfo

This is called explicitly when the gamestate is first received,
and whenever the server updates any serverinfo flagged cvars
================
*/
void CG_ParseServerinfo( void )
{
	// #CL_ADD
	const char	*info;
	const char	*sysInfo;
	char		*mapname;

	info = CG_ConfigString( CS_SERVERINFO );
	sysInfo = CG_ConfigString( CS_SYSTEMINFO );
	// #END CL_ADD
	cgs.gametype = BG_FindGametype ( Info_ValueForKey( info, "g_gametype" ) );
	cgs.gametypeData = &bg_gametypeData[cgs.gametype];
	cgs.dmflags = atoi( Info_ValueForKey( info, "dmflags" ) );
	cgs.teamflags = atoi( Info_ValueForKey( info, "teamflags" ) );
	cgs.scorelimit = atoi( Info_ValueForKey( info, "scorelimit" ) );
	cgs.timelimit = atoi( Info_ValueForKey( info, "timelimit" ) );
	cgs.maxclients = atoi( Info_ValueForKey( info, "sv_maxclients" ) );
	cgs.friendlyFire = atoi( Info_ValueForKey( info, "g_friendlyFire" ) ) ? qtrue : qfalse;
	cgs.punkbuster = atoi( Info_ValueForKey( info, "sv_punkbster" ) ) ? qtrue : qfalse;

	// #CL_ADD
	// Boe!Man 11/8/15: Check if third person is allowed by the server.
	char *s = Info_ValueForKey(info, "g_allowThirdPerson");

	if(*s){
		cgs.allowThirdPerson = atoi(s);
	}else{
		cgs.allowThirdPerson = 0;
	}

	// Boe!Man 1/2/16: Check for the recoil and inaccuracy ratio.
	s = Info_ValueForKey( sysInfo, "g_recoilRatio" );
	if(*s)
	{
		trap_Cvar_Set("cg_recoilRatio", s);
		trap_Cvar_Update(&cg_recoilRatio);
	}

	s = Info_ValueForKey( sysInfo, "g_inaccuracyRatio" );
	if(*s)
	{
		trap_Cvar_Set("cg_inaccuracyRatio", s);
		trap_Cvar_Update(&cg_inaccuracyRatio);
	}

	// Also determine what gametype the server is running.
	// We use this to recommend 3rd person in H&S/H&Z.
	s = Info_ValueForKey(info, "current_gametype");
	if(*s){
		cgs.current_gametype = atoi(s);
	}else{
		cgs.current_gametype = GT_NONE;
	}

	// Set correct gametype name in the about screen.
	// This is also used for determining H&S/H&Z in the UI code.
	if(cgs.current_gametype == GT_HS){
		trap_Cvar_Set ( "ui_about_gametypename", "Hide&Seek" );
	}else if(cgs.current_gametype == GT_HZ){
		trap_Cvar_Set ( "ui_about_gametypename", "Humans&Zombies" );
	}else{
		trap_Cvar_Set ( "ui_about_gametypename", cgs.gametypeData->displayName );
	}

	mapname = Info_ValueForKey( info, "mapname" );
	Com_sprintf( cgs.mapname, sizeof( cgs.mapname ), "maps/%s.bsp", mapname );

	trap_Cvar_Set ( "ui_about_gametype", va("%i", cgs.gametype ) );
	//trap_Cvar_Set ( "ui_about_gametypename", cgs.gametypeData->displayName );
	// #END CL_ADD
	trap_Cvar_Set ( "ui_about_scorelimit", va("%i", cgs.scorelimit ) );
	trap_Cvar_Set ( "ui_about_timelimit", va("%i", cgs.timelimit ) );
	trap_Cvar_Set ( "ui_about_friendlyfire", va("%i", cgs.friendlyFire) );
	trap_Cvar_Set ( "ui_about_maxclients", va("%i", cgs.maxclients ) );
	trap_Cvar_Set ( "ui_about_dmflags", va("%i", cgs.dmflags ) );
	trap_Cvar_Set ( "ui_about_mapname", mapname );
	trap_Cvar_Set ( "ui_about_hostname", Info_ValueForKey( info, "sv_hostname" ) );
	trap_Cvar_Set ( "ui_about_needpass", Info_ValueForKey( info, "g_needpass" ) );
	trap_Cvar_Set ( "ui_about_botminplayers", Info_ValueForKey ( info, "bot_minplayers" ) );

	// 1.03 CHANGE - (unofficial) fix for disabled weapons showing up in outfitting
	trap_Cvar_Set ( "ui_info_availableweapons", Info_ValueForKey ( info, "g_available" ) );
	trap_Cvar_Set ( "ui_info_teamgame", va("%i", cgs.gametypeData->teams ? 1 : 0 ) );

	BG_SetAvailableOutfitting ( Info_ValueForKey ( info, "g_availableWeapons" ) );

	if ( cgs.gametypeData->teams )
	{
		trap_Cvar_Set ( "ui_info_redteam", CG_ConfigString ( CS_GAMETYPE_REDTEAM ) );
		trap_Cvar_Set ( "ui_info_blueteam", CG_ConfigString ( CS_GAMETYPE_BLUETEAM ) );
	}
	else
	{
		trap_Cvar_Set ( "ui_info_redteam", "" );
		trap_Cvar_Set ( "ui_info_blueteam", "" );
	}

	info = CG_ConfigString( CS_TERRAINS + 1 );
	if ( !info || !*info )
	{
		cg.mInRMG = qfalse;
	}
	else
	{
		cg.mInRMG = qtrue;
	}
}

/*
==================
CG_ParseWarmup
==================
*/
static void CG_ParseWarmup( void )
{
	const char	*info;
	int			warmup;

	info = CG_ConfigString( CS_WARMUP );

	warmup = atoi( info );
	cg.warmupCount = -1;

	cg.warmup = warmup;
}

static void CG_ParseGametypeTimer ( void )
{
	cgs.gametypeTimerTime = atoi( CG_ConfigString( CS_GAMETYPE_TIMER ) );
}

static void CG_ParseGametypeMessage ( void )
{
	char  temp[1024];
	char* comma;

	strcpy ( temp, CG_ConfigString( CS_GAMETYPE_MESSAGE ) );
	comma = strchr ( temp, ',' );
	if ( !comma )
	{
		return;
	}

	*(comma++) = '\0';

	cgs.gametypeMessageTime = atoi ( temp );

	// Silent gametype message
	if ( *comma == '@' )
	{
		strcpy ( cgs.gametypeMessage, comma + 1 );
	}
	else
	{
		strcpy ( cgs.gametypeMessage, comma );
		Com_Printf ( "@%s\n", cgs.gametypeMessage );
	}
}

/*
================
CG_ParseVoteTime
================
*/
static void CG_ParseVoteTime ( void )
{
	char  temp[1024];
	char* comma;
	const char *str;

	str = CG_ConfigString( CS_VOTE_TIME );

	strcpy ( temp, str );
	comma = strchr ( temp, ',' );
	if ( !comma )
	{
		cgs.voteTime = cgs.voteDuration = 0;
		return;
	}
	*comma = 0;

	cgs.voteTime = atoi(str);
	cgs.voteDuration = atoi(comma+1);
	cgs.voteModified = qtrue;
}

/*
================
CG_SetConfigValues

Called on load to set the initial values from configure strings
================
*/
void CG_SetConfigValues( void )
{
	int i;

	cgs.levelStartTime	  = atoi( CG_ConfigString( CS_LEVEL_START_TIME ) );
	cg.warmup			  = atoi( CG_ConfigString( CS_WARMUP ) );
	cgs.pickupsDisabled   = atoi( CG_ConfigString( CS_PICKUPSDISABLED ) );
	cgs.gameID			  = atoi( CG_ConfigString( CS_GAME_ID ) );

	trap_Cvar_Set ( "ui_info_pickupsdisabled", va("%i", cgs.pickupsDisabled ) );

	CG_ParseGametypeTimer ( );
	CG_ParseGametypeMessage ( );
	CG_ParseVoteTime ( );

	for ( i = 0; i < MAX_HUDICONS; i ++ )
	{
		cgs.hudIcons[i] = atoi ( CG_ConfigString ( CS_HUDICONS + i ) );
	}
}

/*
=====================
CG_ShaderStateChanged
=====================
*/
void CG_ShaderStateChanged(void) {
	char originalShader[MAX_QPATH];
	char newShader[MAX_QPATH];
	char timeOffset[16];
	const char *o;
	char *n,*t;

	o = CG_ConfigString( CS_SHADERSTATE );
	while (o && *o) {
		n = strstr(o, "=");
		if (n && *n) {
			strncpy(originalShader, o, n-o);
			originalShader[n-o] = 0;
			n++;
			t = strstr(n, ":");
			if (t && *t) {
				strncpy(newShader, n, t-n);
				newShader[t-n] = 0;
			} else {
				break;
			}
			t++;
			o = strstr(t, "@");
			if (o) {
				strncpy(timeOffset, t, o-t);
				timeOffset[o-t] = 0;
				o++;
				trap_R_RemapShader( originalShader, newShader, timeOffset );
			}
		} else {
			break;
		}
	}
}

/*
================
CG_ConfigStringModified
================
*/
static void CG_ConfigStringModified( void )
{
	const char	*str;
	int			num;

	num = atoi( CG_Argv( 1 ) );

	// get the gamestate from the client system, which will have the
	// new configstring already integrated
	trap_GetGameState( &cgs.gameState );

	// look up the individual string that was modified
	str = CG_ConfigString( num );

	// do something with it if necessary
	if ( num == CS_MUSIC )
	{
		CG_StartMusic( qtrue );
	}
	else if ( num == CS_SERVERINFO )
	{
		CG_ParseServerinfo();
	}
	else if ( num == CS_WARMUP )
	{
		CG_ParseWarmup();
	}
	else if ( num == CS_GAMETYPE_TIMER )
	{
		CG_ParseGametypeTimer ( );
	}
	else if ( num == CS_GAMETYPE_MESSAGE )
	{
		CG_ParseGametypeMessage ( );
	}
	else if ( num == CS_LEVEL_START_TIME )
	{
		cgs.levelStartTime = atoi( str );
	}
	else if ( num == CS_VOTE_TIME )
	{
		CG_ParseVoteTime();
	}
	else if ( num == CS_VOTE_NEEDED )
	{
		cgs.voteNeeded = atoi( str );
		cgs.voteModified = qtrue;
	}
	else if ( num == CS_VOTE_YES )
	{
		cgs.voteYes = atoi( str );
		cgs.voteModified = qtrue;
	}
	else if ( num == CS_VOTE_NO )
	{
		cgs.voteNo = atoi( str );
		cgs.voteModified = qtrue;
	}
	else if ( num == CS_VOTE_STRING )
	{
		Q_strncpyz( cgs.voteString, str, sizeof( cgs.voteString ) );
		trap_S_StartLocalSound( cgs.media.talkSound, CHAN_ANNOUNCER );
	}
	else if ( num == CS_INTERMISSION )
	{
		cg.intermissionStarted = atoi( str );
	}
	else if ( num >= CS_MODELS && num < CS_MODELS+MAX_MODELS )
	{
		cgs.gameModels[ num-CS_MODELS ] = trap_R_RegisterModel( str );
	}
	else if ( num >= CS_SOUNDS && num < CS_SOUNDS+MAX_MODELS )
	{
		if ( str[0] != '*' )
		{
			// player specific sounds don't register here
			cgs.gameSounds[ num-CS_SOUNDS] = trap_S_RegisterSound( str );
		}
	}
	else if ( num >= CS_PLAYERS && num < CS_PLAYERS+MAX_CLIENTS )
	{
		CG_NewClientInfo( num - CS_PLAYERS );
	}
	else if ( num == CS_SHADERSTATE )
	{
		CG_ShaderStateChanged();
	}
	else if ( num >= CS_LIGHT_STYLES && num < CS_LIGHT_STYLES + (MAX_LIGHT_STYLES * 3))
	{
		CG_SetLightstyle(num - CS_LIGHT_STYLES);
	}
	else if ( num >= CS_ICONS && num < CS_ICONS + MAX_ICONS )
	{
		cgs.gameIcons[ num - CS_ICONS ] = trap_R_RegisterShaderNoMip ( str );
	}
	else if ( num >= CS_HUDICONS && num < CS_HUDICONS + MAX_HUDICONS )
	{
		cgs.hudIcons[ num - CS_HUDICONS ] = atoi ( str );
	}
}


/*
===============
CG_MapRestart

The server has issued a map_restart, so the next snapshot
is completely new and should not be interpolated to.

A map restart will clear everything, but doesn't
require a reload of all the media
===============
*/
void CG_MapRestart( qboolean gametypeRestart )
{
	int i;

	if ( cg_showmiss.integer )
	{
		Com_Printf( "CG_MapRestart\n" );
	}

	trap_R_ClearDecals ( );
	trap_FX_Reset ( );
	trap_MAT_Reset();

	CG_InitLocalEntities();

	cg.intermissionStarted = qfalse;

	// dont clear votes on gametype restarts
	if ( !gametypeRestart )
	{
		cgs.voteTime = 0;
		cgs.gametypeMessage[0] = '\0';
		cgs.gametypeMessageTime = 0;
	}

	cgs.gameover[0] = '\0';

	cg.mapRestart = qtrue;

	// Make sure the weapon selection menu isnt up
	cg.weaponMenuUp = qfalse;

	cg.gametypeStarted = qfalse;

//	cgs.media.mAutomap = 0;			// make sure to re-upload the auto-map

	CG_StartMusic(qtrue);

	trap_S_ClearLoopingSounds(qtrue);

	trap_Cvar_Set("cg_thirdPerson", "0");


	for ( i = 0 ; i < cgs.maxclients ; i++ )
	{
		clientInfo_t* cl;

		cl = &cgs.clientinfo[ i ];

		if ( !cl->infoValid )
		{
			continue;
		}

		cl->spectator = qfalse;
	}

	cg.bestStats = qfalse;
	cgs.screenshotTaken = qfalse;

}

#define MAX_VOICEFILESIZE	16384
#define MAX_VOICEFILES		8
#define MAX_VOICECHATS		64
#define MAX_VOICESOUNDS		64
#define MAX_CHATSIZE		64
#define MAX_HEADMODELS		64

typedef struct voiceChat_s
{
	char id[64];
	int numSounds;
	sfxHandle_t sounds[MAX_VOICESOUNDS];
	char chats[MAX_VOICESOUNDS][MAX_CHATSIZE];
} voiceChat_t;

typedef struct voiceChatList_s
{
	char name[64];
	int gender;
	int numVoiceChats;
	voiceChat_t voiceChats[MAX_VOICECHATS];
} voiceChatList_t;

typedef struct headModelVoiceChat_s
{
	char headmodel[64];
	int voiceChatNum;
} headModelVoiceChat_t;

voiceChatList_t voiceChatLists[MAX_VOICEFILES];
headModelVoiceChat_t headModelVoiceChat[MAX_HEADMODELS];

/*
=================
CG_ParseVoiceChats
=================
*/
int CG_ParseVoiceChats( const char *filename, voiceChatList_t *voiceChatList, int maxVoiceChats ) {
	int	len, i;
	fileHandle_t f;
	char buf[MAX_VOICEFILESIZE];
	const char **p, *ptr;
	char *token;
	voiceChat_t *voiceChats;
	qboolean compress;

	compress = qtrue;
	if (cg_buildScript.integer) {
		compress = qfalse;
	}

	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( !f ) {
		trap_Print( va( S_COLOR_RED "voice chat file not found: %s\n", filename ) );
		return qfalse;
	}
	if ( len >= MAX_VOICEFILESIZE ) {
		trap_Print( va( S_COLOR_RED "voice chat file too large: %s is %i, max allowed is %i", filename, len, MAX_VOICEFILESIZE ) );
		trap_FS_FCloseFile( f );
		return qfalse;
	}

	trap_FS_Read( buf, len, f );
	buf[len] = 0;
	trap_FS_FCloseFile( f );

	ptr = buf;
	p = &ptr;

	Com_sprintf(voiceChatList->name, sizeof(voiceChatList->name), "%s", filename);
	voiceChats = voiceChatList->voiceChats;
	for ( i = 0; i < maxVoiceChats; i++ ) {
		voiceChats[i].id[0] = 0;
	}
	token = COM_ParseExt(p, qtrue);
	if (!token || token[0] == 0) {
		return qtrue;
	}
	if (!Q_stricmp(token, "female")) {
		voiceChatList->gender = GENDER_FEMALE;
	}
	else if (!Q_stricmp(token, "male")) {
		voiceChatList->gender = GENDER_MALE;
	}
	else if (!Q_stricmp(token, "neuter")) {
		voiceChatList->gender = GENDER_NEUTER;
	}
	else {
		trap_Print( va( S_COLOR_RED "expected gender not found in voice chat file: %s\n", filename ) );
		return qfalse;
	}

	voiceChatList->numVoiceChats = 0;
	while ( 1 ) {
		token = COM_ParseExt(p, qtrue);
		if (!token || token[0] == 0) {
			return qtrue;
		}
		Com_sprintf(voiceChats[voiceChatList->numVoiceChats].id, sizeof( voiceChats[voiceChatList->numVoiceChats].id ), "%s", token);
		token = COM_ParseExt(p, qtrue);
		if (Q_stricmp(token, "{")) {
			trap_Print( va( S_COLOR_RED "expected { found %s in voice chat file: %s\n", token, filename ) );
			return qfalse;
		}
		voiceChats[voiceChatList->numVoiceChats].numSounds = 0;
		while(1) {
			token = COM_ParseExt(p, qtrue);
			if (!token || token[0] == 0) {
				return qtrue;
			}
			if (!Q_stricmp(token, "}"))
				break;
			voiceChats[voiceChatList->numVoiceChats].sounds[voiceChats[voiceChatList->numVoiceChats].numSounds] =
          trap_S_RegisterSound( token );
			token = COM_ParseExt(p, qtrue);
			if (!token || token[0] == 0) {
				return qtrue;
			}
			Com_sprintf(voiceChats[voiceChatList->numVoiceChats].chats[
							voiceChats[voiceChatList->numVoiceChats].numSounds], MAX_CHATSIZE, "%s", token);
			voiceChats[voiceChatList->numVoiceChats].numSounds++;
			if (voiceChats[voiceChatList->numVoiceChats].numSounds >= MAX_VOICESOUNDS)
				break;
		}
		voiceChatList->numVoiceChats++;
		if (voiceChatList->numVoiceChats >= maxVoiceChats)
			return qtrue;
	}
	return qtrue;
}

/*
=================
CG_LoadVoiceChats
=================
*/
void CG_LoadVoiceChats( void )
{
	int size;

	size = trap_MemoryRemaining();
	CG_ParseVoiceChats( "scripts/female1.voice", &voiceChatLists[0], MAX_VOICECHATS );
	CG_ParseVoiceChats( "scripts/male1.voice", &voiceChatLists[1], MAX_VOICECHATS );
}

/*
=================
CG_GetVoiceChat
=================
*/
int CG_GetVoiceChat( voiceChatList_t *voiceChatList, const char *id, sfxHandle_t *snd, char **chat)
{
	int i, rnd;

	for ( i = 0; i < voiceChatList->numVoiceChats; i++ )
	{
		if ( !Q_stricmp( id, voiceChatList->voiceChats[i].id ) )
		{
			rnd   = random() * voiceChatList->voiceChats[i].numSounds;
			*snd  = voiceChatList->voiceChats[i].sounds[rnd];
			*chat = voiceChatList->voiceChats[i].chats[rnd];
			return qtrue;
		}
	}

	return qfalse;
}

/*
=================
CG_VoiceChatListForClient
=================
*/
voiceChatList_t *CG_VoiceChatListForClient( int clientNum )
{
	clientInfo_t *ci;

	ci = &cgs.clientinfo[ clientNum ];

	switch ( ci->gender )
	{
		case GENDER_FEMALE:
			return &voiceChatLists[0];

		case GENDER_MALE:
			return &voiceChatLists[1];
	}

	// just return the male voice chat list since there are more male characters
	return &voiceChatLists[1];
}

#define MAX_VOICECHATBUFFER		32

typedef struct bufferedVoiceChat_s
{
	int clientNum;
	sfxHandle_t snd;
	int voiceOnly;
	char cmd[MAX_SAY_TEXT];
	char message[MAX_SAY_TEXT];
} bufferedVoiceChat_t;

bufferedVoiceChat_t voiceChatBuffer[MAX_VOICECHATBUFFER];

/*
=================
CG_PlayVoiceChat
=================
*/
void CG_PlayVoiceChat( bufferedVoiceChat_t *vchat )
{
	// if we are going into the intermission, don't start any voices
	if ( cg.intermissionStarted )
	{
		return;
	}

	if ( cg_voiceRadio.integer )
	{
		trap_S_StartLocalSound( vchat->snd, CHAN_VOICE);
	}

	if (!vchat->voiceOnly && !cg_noVoiceText.integer)
	{
		CG_AddChatText ( vchat->clientNum, vchat->message );
	}

	voiceChatBuffer[cg.voiceChatBufferOut].snd = 0;
}

/*
=====================
CG_PlayBufferedVoieChats
=====================
*/
void CG_PlayBufferedVoiceChats( void )
{
	if ( cg.voiceChatTime < cg.time )
	{
		if (cg.voiceChatBufferOut != cg.voiceChatBufferIn && voiceChatBuffer[cg.voiceChatBufferOut].snd)
		{
			//
			CG_PlayVoiceChat(&voiceChatBuffer[cg.voiceChatBufferOut]);
			//
			cg.voiceChatBufferOut = (cg.voiceChatBufferOut + 1) % MAX_VOICECHATBUFFER;
			cg.voiceChatTime = cg.time + 1000;
		}
	}
}

/*
=====================
CG_AddBufferedVoiceChat
=====================
*/
void CG_AddBufferedVoiceChat( bufferedVoiceChat_t *vchat )
{
	// if we are going into the intermission, don't start any voices
	if ( cg.intermissionStarted )
	{
		return;
	}

	memcpy(&voiceChatBuffer[cg.voiceChatBufferIn], vchat, sizeof(bufferedVoiceChat_t));
	cg.voiceChatBufferIn = (cg.voiceChatBufferIn + 1) % MAX_VOICECHATBUFFER;
	if (cg.voiceChatBufferIn == cg.voiceChatBufferOut)
	{
		CG_PlayVoiceChat( &voiceChatBuffer[cg.voiceChatBufferOut] );
		cg.voiceChatBufferOut++;
	}
}

/*
=================
CG_VoiceChatLocal
=================
*/
void CG_VoiceChatLocal( qboolean voiceOnly, int clientNum, const char* chatprefix, const char *cmd )
{
	char				*chat;
	voiceChatList_t		*voiceChatList;
	sfxHandle_t			snd;
	bufferedVoiceChat_t vchat;

	// if we are going into the intermission, don't start any voices
	if ( cg.intermissionStarted )
	{
		return;
	}

	// Get the voice chat info for the speaking client
	voiceChatList = CG_VoiceChatListForClient( clientNum );
	if ( !CG_GetVoiceChat( voiceChatList, cmd, &snd, &chat ) )
	{
		return;
	}

	vchat.clientNum = clientNum;
	vchat.snd = snd;
	vchat.voiceOnly = voiceOnly;
	Q_strncpyz(vchat.cmd, cmd, sizeof(vchat.cmd));

	Com_sprintf ( vchat.message, sizeof(vchat.message), "%s%s", chatprefix, chat );

	CG_AddBufferedVoiceChat(&vchat);
}

/*
=================
CG_VoiceChat
=================
*/
void CG_VoiceChat( int mode )
{
	char cmd[MAX_QPATH];
	const char *chatprefix;
	qboolean   voiceOnly;
	int		   clientNum;

	voiceOnly  = atoi(CG_Argv(1));
	clientNum  = atoi(CG_Argv(2));
	Com_sprintf ( cmd, MAX_QPATH, CG_Argv(4) );
	chatprefix = CG_Argv(3);

	CG_VoiceChatLocal ( voiceOnly, clientNum, chatprefix, cmd );
}

/*
=================
CG_RemoveChatEscapeChar
=================
*/
static void CG_RemoveChatEscapeChar( char *text ) {
	int i, l;

	l = 0;
	for ( i = 0; text[i]; i++ ) {
		if (text[i] == '\x19')
			continue;
		text[l++] = text[i];
	}
	text[l] = '\0';
}

/*
=================
CG_ServerCommand

The string has been tokenized and can be retrieved with
Cmd_Argc() / Cmd_Argv()
=================
*/
static void CG_ServerCommand( void ) {
	const char	*cmd;
	char		text[MAX_SAY_TEXT];
	char		var[MAX_TOKEN_CHARS];


	cmd = CG_Argv(0);

	if ( !cmd[0] ) {
		// server claimed the command
		return;
	}

	if ( !strcmp( cmd, "cp" ) )
	{
		const char* s;

		s = CG_Argv(1);
		if ( *s == '@' )
		{
			s++;
		}
		else
		{
			Com_Printf ( "@%s", s );
		}

		CG_CenterPrint( s, 0.43f );
		cg.centerPrintPriority = qtrue;
		return;
	}

	if ( !strcmp( cmd, "cs" ) )
	{
		CG_ConfigStringModified();
		return;
	}

	if ( !strcmp( cmd, "print" ) )
	{
		Com_Printf( "%s", CG_Argv(1) );

/*
		cmd = CG_Argv(1);			// yes, this is obviously a hack, but so is the way we hear about
									// votes passing or failing
		if ( !Q_stricmpn( cmd, "vote failed", 11 ) )
		{
			trap_S_StartLocalSound( cgs.media.voteFailed, CHAN_ANNOUNCER );
		}
		else if ( !Q_stricmpn( cmd, "vote passed", 11 ) )
		{
			trap_S_StartLocalSound( cgs.media.votePassed, CHAN_ANNOUNCER );
		}
*/
		return;
	}

	if ( !strcmp( cmd, "chat" ) )
	{
		if ( !cg_teamChatsOnly.integer )
		{
			trap_S_StartLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND );
			Q_strncpyz( text, CG_Argv(2), MAX_SAY_TEXT );
			CG_RemoveChatEscapeChar( text );
			CG_AddChatText ( atoi(CG_Argv(1)), text );
			Com_Printf( "@%s\n", text );
		}
		return;
	}

	if ( !strcmp( cmd, "tchat" ) )
	{
		trap_S_StartLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND );
		Q_strncpyz( text, CG_Argv(2), MAX_SAY_TEXT );
		CG_RemoveChatEscapeChar( text );
		CG_AddChatText ( atoi(CG_Argv(1)), text );
		Com_Printf( "@%s\n", text );
		return;
	}

	if ( !strcmp( cmd, "achat" ) )
	{
		trap_S_StartLocalSound( cgs.media.admintalkSound, CHAN_LOCAL_SOUND );
		Q_strncpyz( text, CG_Argv(2), MAX_SAY_TEXT );
		CG_RemoveChatEscapeChar( text );
		CG_AddChatText ( atoi(CG_Argv(1)), text );
		Com_Printf( "@%s\n", text );
		return;
	}

	if ( !strcmp( cmd, "sound" ) )
	{
		const char* s;
		int team;

		team = atoi( CG_Argv(1) );
		s = CG_Argv(2);

		if ( cg_allowVoiceCommandSounds.integer )
		{
			if ( cg_allowVoiceCommandSounds.integer == 2 && team != cgs.clientinfo[cg.clientNum].team )
			{
				return;
			}
			trap_S_StartLocalSound( trap_S_RegisterSound( s ), CHAN_AUTO );
		}
		return;
	}

	if ( !strcmp( cmd, "verifymod" ) )
	{
		trap_SendClientCommand( va("verified %i", ROCMOD_VER ) );

		return;
	}

	if ( !strcmp( cmd, "playerstats" ) )
	{
		if ( !cg.bestStats )
		{
			cg.bestStats = qtrue;
			cg.bestStatsStart = cg.time + 500;
		}
		cg.bestKills = atoi( CG_Argv(1) );
		cg.bestAccuracy = atoi( CG_Argv(2) );
		cg.bestHeadshots = atoi( CG_Argv(3) );
		cg.bestFlagCaps = atoi( CG_Argv(4) );
		cg.bestRatio = atoi( CG_Argv(5) );
		cg.bestValue = atoi( CG_Argv(6) );
		cg.bestKillRate = atoi( CG_Argv(7) );
		cg.bestKillSpree = atoi( CG_Argv(8) );
		cg.bestNadeKills = atoi( CG_Argv(9) );
		cg.bestMeleeKills = atoi( CG_Argv(10) );
		cg.bestFlagDefends = atoi( CG_Argv(11) );
		return;
	}

	if ( !strcmp( cmd, "vglobal" ) )
	{
		char				*chat;
		voiceChatList_t		*voiceChatList;
		sfxHandle_t			snd;
		int					clientNum;

		if ( !cg_voiceGlobal.integer )
		{
			return;
		}

		clientNum = atoi(CG_Argv(1));

		// Get the voice chat info for the speaking client
		voiceChatList = CG_VoiceChatListForClient( clientNum );
		if ( !CG_GetVoiceChat( voiceChatList, CG_Argv(2), &snd, &chat ) )
		{
			return;
		}

		trap_S_StartSound ( NULL, clientNum, CHAN_AUTO, snd, 240, 1150 );
		return;
	}

	if ( !strcmp( cmd, "vtchat" ) )
	{
		CG_VoiceChat( SAY_TEAM );
		return;
	}

	if ( !strcmp( cmd, "vtell" ) )
	{
		CG_VoiceChat( SAY_TELL );
		return;
	}

	if ( !strcmp( cmd, "scores" ) )
	{
		CG_ParseScores();
		CG_UpdateTeamCountCvars ( );
		return;
	}

	if ( !strcmp( cmd, "scores2" ) )
	{
		CG_ParseExtraScores();
		return;
	}

	if ( !strcmp( cmd, "scores3" ) )
	{
		CG_ParseExtraScores2();
		return;
	}

	if ( !strcmp( cmd, "scores4" ) )
	{
		CG_ParseExtraScores3();
		return;
	}

	if ( !strcmp( cmd, "eti" ) )		// extra - team info (old)
	{
		CG_ParseExtraTeamInfo();
		return;
	}

	if ( !strcmp( cmd, "eti2" ) )		// extra - team info
	{
		CG_ParseExtraTeamInfo2();
		return;
	}

	if ( !strcmp( cmd, "eas" ) )		// extra - add spectator
	{
		int clientNum;

		clientNum = atoi( CG_Argv(1) );
		cgs.clientinfo[clientNum].spectator = qtrue;
		return;
	}

	if ( !strcmp( cmd, "ers" ) )		// extra - remove spectator
	{
		int clientNum;

		clientNum = atoi( CG_Argv(1) );
		cgs.clientinfo[clientNum].spectator = qfalse;
		return;
	}

	if ( !strcmp( cmd, "efack" ) )		// extra features acknowledgement
	{
		int value;

		value = atoi( CG_Argv(1) );
		if ( value == cg.extraFeatures )
		{
			cg.extraFeaturesAckTime = 0;
		}
		return;
	}

	if ( !strcmp( cmd, "cvarcheck" ) )
	{
		char buffer[1024];
		int id;

		id = atoi( CG_Argv(1) );
		trap_Cvar_VariableStringBuffer( CG_Argv(2), buffer, sizeof(buffer) );
		if ( buffer[0] == 0 )
		{
			strcpy( buffer, "unset" );
		}
		trap_SendClientCommand( va("cvarval %i %s %s", id, CG_Argv(2), buffer ) );
		return;
	}

/*	// Disabled for now, the generic form is too dangerous
	if ( !strcmp( cmd, "cvarset" ) )
	{
		char	buffer1[1024];
		char	buffer2[1024];

		Q_strncpyz( buffer1, CG_Argv(1), sizeof(buffer1) );
		Q_strncpyz( buffer2, CG_Argv(2), sizeof(buffer2) );
		Com_Printf("Request received to change \"%s\" to \"%s\"\n", buffer1, buffer2);
		trap_Cvar_Set( buffer1, buffer2 );
		return;
	}
*/

	if ( !strcmp( cmd, "setrate" ) )
	{
		int value;

		value = atoi( CG_Argv(1) );
		if ( value < 2500 )
		{
			return;
		}

		trap_Cvar_Set( "rate", va("%i", value) );
		return;
	}

	if ( !strcmp( cmd, "scan" ) )
	{
		int id;
		int	len;
		fileHandle_t	f;
		int	remaining;
		int readsize;
		unsigned char buf[4096];

		id = atoi( CG_Argv(1) );
		len = trap_FS_FOpenFile( CG_Argv(2), &f, FS_READ );
		if ( !f ) {
			trap_SendClientCommand( va("result %i 0", id) );
			return;
		}

		md5_reset();
		remaining = len;
		while ( remaining ) {
			readsize = (remaining > sizeof(buf)) ? sizeof(buf) : remaining;
			trap_FS_Read( buf, readsize, f );
			md5_process( buf, readsize );
			remaining -= readsize;
		}
		md5_finish();

		trap_FS_FCloseFile( f );

		trap_SendClientCommand( va("result %i %s", id, md5_output) );
		return;
	}

	if ( !strcmp( cmd, "map_restart" ) )
	{
		CG_MapRestart( qfalse );
		return;
	}

	if ( Q_stricmp (cmd, "remapShader") == 0 )
	{
		if (trap_Argc() == 4)
		{
			trap_R_RemapShader(CG_Argv(1), CG_Argv(2), CG_Argv(3));
		}
	}

	// loaddeferred can be both a servercmd and a consolecmd
	if ( !strcmp( cmd, "loaddeferred" ) )
	{
		CG_LoadDeferredPlayers();
		return;
	}

	// clientLevelShot is sent before taking a special screenshot for
	// the menu system during development
	if ( !strcmp( cmd, "clientLevelShot" ) )
	{
		cg.levelShot = qtrue;
		return;
	}

	// #CL_ADD
	// Boe!Man 11/4/15: Let the server know we use client additions.
	if ( !strcmp( cmd, "ca_verify" ) )
	{
		trap_SendClientCommand(va("ca_verified %s", _1FX_CLADD_VER));
		return;
	}
	// #END CL_ADD

	Com_Printf( "Unknown client game command: %s\n", cmd );
}


/*
====================
CG_ExecuteNewServerCommands

Execute all of the server commands that were received along
with this this snapshot.
====================
*/
void CG_ExecuteNewServerCommands( int latestSequence ) {
	while ( cgs.serverCommandSequence < latestSequence ) {
		if ( trap_GetServerCommand( ++cgs.serverCommandSequence ) ) {
			CG_ServerCommand();
		}
	}
}
