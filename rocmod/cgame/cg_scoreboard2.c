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
// cg_scoreboard -- draw the scoreboard on top of the game screen
#include "cg_local.h"


extern	int		sb_lineHeight;
extern	int		sb_maxClients;
extern	float	sb_nameFontScale;
extern	float	sb_numberFontScale;
extern	float	sb_readyFontScale;

/*
=================
CG_DrawClientScoreIM
=================
*/
static void CG_DrawClientScoreIM( float x, float y, float w, score_t* score, float* color )
{
	clientInfo_t*	ci;
	vec4_t			dataColor;
	vec4_t			nameColor;
	const char*		s;
	float			f;
	float			h;
	float			acc;
	int				accm, accn;
	vec4_t			colorBg = {0, 0, 0, 0.9f};
	vec4_t			colorSp = {1.0f, 1.0f, 1.0f, 0.05f};
	vec4_t			colorP = {1, 1, 1, 1};
	qhandle_t		spreemedal;
	int				spreestart;

	nameColor[3] = dataColor[3] = 1.0f;
	
	// Validate the score
	if ( score->client < 0 || score->client >= cgs.maxclients ) 
	{
		Com_Printf( "Bad score->client: %i\n", score->client );
		return;
	}

	// Convienience	
	ci = &cgs.clientinfo[score->client];

//	CG_DrawPic ( x - 5, y, w, sb_lineHeight, cgs.media.scoreboardLine );

//	color[3] = 0.6f;
//	CG_DrawPic ( x - 15, y, 10, sb_lineHeight, cgs.media.scoreboardLine );
//	CG_FillRect( x - 15, y, 10, sb_lineHeight, color );
//	color[3] = 1.0f;
	CG_FillRect( x - 5, y + 2, w, sb_lineHeight - 2, colorBg );

	// highlight your position
	if ( score->client == cg.snap->ps.clientNum ) 
	{
		vec4_t hcolor;

		if ( score->clan )
		{
			hcolor[0] = cgs.teamData.clanShade[0];
			hcolor[1] = cgs.teamData.clanShade[1];
			hcolor[2] = cgs.teamData.clanShade[2];
			hcolor[3] = 0.1f;
		}
		else
		{
			hcolor[0] = 1.0f;
			hcolor[1] = 1.0f;
			hcolor[2] = 1.0f;
			hcolor[3] = 0.1f;
		}

		CG_FillRect( x - 5, y, w, sb_lineHeight, hcolor );

		VectorSet ( nameColor, 1.0f, 1.0f, 1.0f );
		VectorSet ( dataColor, 0.5f, 0.5f, 0.5f );
	}
	else if ( (cg.snap->ps.pm_type == PM_DEAD) && score->client == cg.snap->ps.persistant[PERS_ATTACKER] )
	{
		vec4_t hcolor;

		if ( score->clan )
		{
			hcolor[0] = cgs.teamData.clanShade[0];
			hcolor[1] = cgs.teamData.clanShade[1];
			hcolor[2] = cgs.teamData.clanShade[2];
			hcolor[3] = 0.1f;
		}
		else
		{
			hcolor[0] = 1.0f;
			hcolor[1] = 1.0f;
			hcolor[2] = 1.0f;
			hcolor[3] = 0.1f;
		}

		CG_FillRect( x - 5, y, w, sb_lineHeight, hcolor );

		VectorCopy ( color, nameColor );
		VectorSet ( dataColor, 0.5f, 0.5f, 0.5f );
	}
	else
	{
		vec4_t hcolor;

		if ( score->clan )
		{
			hcolor[0] = cgs.teamData.clanShade[0];
			hcolor[1] = cgs.teamData.clanShade[1];
			hcolor[2] = cgs.teamData.clanShade[2];
			hcolor[3] = .10f;

			CG_FillRect( x - 5, y, w, sb_lineHeight, hcolor );
		}

		VectorCopy ( color, nameColor );
		VectorSet ( dataColor, 0.3f, 0.3f, 0.3f );

		if ( ci->ghost )
		{
			VectorScale ( nameColor, 0.6f, nameColor );
		}
	}

	CG_DrawText( x, y + 2, cgs.media.hudFont, sb_nameFontScale, nameColor, ci->name, 24, 0 );

	s = va("%i", score->score );
	f = trap_R_GetTextWidth ( s, cgs.media.hudFont, sb_nameFontScale, 0 );
	CG_DrawText( x + w - 10 - f, y + 2, cgs.media.hudFont, sb_nameFontScale, nameColor, s, 0, 0 );

	h = trap_R_GetTextHeight("W", cgs.media.hudFont, sb_readyFontScale, 0 );

	s = va("%i/%i", score->kills, score->deaths);
	f = trap_R_GetTextWidth ( s, cgs.media.hudFont, sb_readyFontScale, 0 );
	CG_DrawText( x + w - 10 - f, y + sb_lineHeight - h - 1, cgs.media.hudFont, sb_readyFontScale, dataColor, s, 0, 0 );

	h = trap_R_GetTextHeight("W", cgs.media.hudFont, sb_readyFontScale * 0.85f, 0 );

//	CG_DrawText( x + 120, y + (sb_lineHeight * 0.45f), cgs.media.hudFont, sb_readyFontScale * 0.85f, dataColor, va("hs: %i", score->headshots ), 0, 0 );

	if ( (score->hits + score->misses) > 0 ) {
		acc = score->hits;							//
		acc /= ( score->hits + score->misses );		// why do
		acc *= 100;									// I have
		accm = (int) acc;							// to spell
		acc -= accm;								// this out?
		acc *= 10;									//
		accn = (int) acc;							//
	}
	else
	{
		accm = 0;
		accn = 0;
	}

//	CG_DrawText( x, y + (sb_lineHeight * 0.45f), cgs.media.hudFont, sb_readyFontScale * 0.85f, dataColor, va("id: %i", score->client ), 0, 0 );
//	CG_DrawText( x, y + (sb_lineHeight * 0.7f), cgs.media.hudFont, sb_readyFontScale * 0.85f, dataColor, va("ping: %i", score->ping ), 0, 0 );
//	CG_DrawText( x + 60, y + (sb_lineHeight * 0.45f), cgs.media.hudFont, sb_readyFontScale * 0.85f, dataColor, va("time: %i", score->time ), 0, 0 );

	CG_DrawText( x, y + sb_lineHeight - h - 1, cgs.media.hudFont, sb_readyFontScale * 0.85f, dataColor, va("time: %i", score->time ), 0, 0 );

	if ( score->teamkillDamage && score->adminref < 2 && exitReady.string[ score->client ] == '0' )
	{
		CG_DrawText( x + 40, y + sb_lineHeight - h - 1, cgs.media.hudFont, sb_readyFontScale * 0.85f, dataColor, va("%i.%i%% / ^$%i%%", accm, accn, score->teamkillDamage ), 0, 0 );
	}
	else
	{
		CG_DrawText( x + 40, y + sb_lineHeight - h - 1, cgs.media.hudFont, sb_readyFontScale * 0.85f, dataColor, va("%i.%i%%", accm, accn ), 0, 0 );
	}

	if ( g_anonymousAdmin.integer ) {
		dataColor[3] = 0.25f;
	}

	if ( score->adminref == 1 ) {
		VectorSet ( dataColor, 0.5f, 1.0f, 0.5f );
		CG_DrawText( x - 12, y + (sb_lineHeight - h) / 2, cgs.media.hudFont, sb_readyFontScale * 0.85f, dataColor, "R", 0, DT_OUTLINE );
	}
	else if ( score->adminref == 2 ) {
		VectorSet ( dataColor, 0.0f, 1.0f, 1.0f );
		CG_DrawText( x - 12, y + (sb_lineHeight - h) / 2, cgs.media.hudFont, sb_readyFontScale * 0.85f, dataColor, "A", 0, DT_OUTLINE );
	}
	else if ( score->adminref == 3 ) {
		VectorSet ( dataColor, 1.0f, 1.0f, 1.0f );
		CG_DrawText( x - 12, y + (sb_lineHeight - h) / 2, cgs.media.hudFont, sb_readyFontScale * 0.85f, dataColor, "A", 0, DT_OUTLINE );
	}
	else if ( score->adminref == -1 ) {
		VectorSet ( dataColor, 0.3f, 0.3f, 0.3f );
		CG_DrawText( x - 12, y + (sb_lineHeight - h) / 2, cgs.media.hudFont, sb_readyFontScale * 0.85f, dataColor, "A", 0, DT_OUTLINE );
	}

	spreestart = w;
	spreestart *= 0.525f;
	spreemedal = trap_R_RegisterShaderNoMip( "gfx/menus/misc/gold_star.png" );
	CG_FillRect( x + spreestart, y + 2 + (sb_lineHeight / 2), (sb_lineHeight - 7) / 2, (sb_lineHeight - 7) / 2, colorSp );
	if ( score->killspree >= KILL_SPREE_1 ) {
		colorP[1] = 1.0f;
		colorP[2] = 1.0f;
		trap_R_SetColor ( colorP );
		CG_DrawPic( x + spreestart, y + 2 + (sb_lineHeight / 2), (sb_lineHeight - 7) / 2, (sb_lineHeight - 7) / 2, spreemedal );
	}
	CG_FillRect( x + spreestart + 2 + ((sb_lineHeight - 8) / 2), y + 2 + (sb_lineHeight / 2), (sb_lineHeight - 7) / 2, (sb_lineHeight - 7) / 2, colorSp );
	if ( score->killspree >= KILL_SPREE_2 ) {
		colorP[1] = 0.75f;
		colorP[2] = 0.75f;
		trap_R_SetColor ( colorP );
		CG_DrawPic( x + spreestart + 2 + ((sb_lineHeight - 8) / 2), y + 2 + (sb_lineHeight / 2), (sb_lineHeight - 7) / 2, (sb_lineHeight - 7) / 2, spreemedal );
	}
	CG_FillRect( x + spreestart + 4 + ((sb_lineHeight - 8) / 2) * 2, y + 2 + (sb_lineHeight / 2), (sb_lineHeight - 7) / 2, (sb_lineHeight - 7) / 2, colorSp );
	if ( score->killspree >= KILL_SPREE_3 ) {
		colorP[1] = 0.5f;
		colorP[2] = 0.5f;
		trap_R_SetColor ( colorP );
		CG_DrawPic( x + spreestart + 4 + ((sb_lineHeight - 8) / 2) * 2, y + 2 + (sb_lineHeight / 2), (sb_lineHeight - 7) / 2, (sb_lineHeight - 7) / 2, spreemedal );
	}
	CG_FillRect( x + spreestart + 6 + ((sb_lineHeight - 8) / 2) * 3, y + 2 + (sb_lineHeight / 2), (sb_lineHeight - 7) / 2, (sb_lineHeight - 7) / 2, colorSp );
	if ( score->killspree >= KILL_SPREE_4 ) {
		colorP[1] = 0.25f;
		colorP[2] = 0.25f;
		trap_R_SetColor ( colorP );
		CG_DrawPic( x + spreestart + 6 + ((sb_lineHeight - 8) / 2) * 3, y + 2 + (sb_lineHeight / 2), (sb_lineHeight - 7) / 2, (sb_lineHeight - 7) / 2, spreemedal );
	}
	CG_FillRect( x + spreestart + 8 + ((sb_lineHeight - 8) / 2) * 4, y + 2 + (sb_lineHeight / 2), (sb_lineHeight - 7) / 2, (sb_lineHeight - 7) / 2, colorSp );
	if ( score->killspree >= KILL_SPREE_5 ) {
		colorP[1] = 0.1f;
		colorP[2] = 0.1f;
		trap_R_SetColor ( colorP );
		CG_DrawPic( x + spreestart + 8 + ((sb_lineHeight - 8) / 2) * 4, y + 2 + (sb_lineHeight / 2), (sb_lineHeight - 7) / 2, (sb_lineHeight - 7) / 2, spreemedal );
	}
	colorP[1] = 1.0f;
	colorP[2] = 1.0f;
	trap_R_SetColor ( colorP );

//	if ( cg.snap->ps.stats[ STAT_CLIENTS_READY ] & ( 1 << score->client ) )
	if ( exitReady.string[ score->client ] == '1' )
	{
		vec4_t deadColor;
		deadColor[0] = 1.0f;
		deadColor[1] = 1.0f;
		deadColor[2] = 1.0f;
		deadColor[3] = 1.0f;
		spreestart = w;
		spreestart *= 0.525f;
		f = trap_R_GetTextWidth ( "READY", cgs.media.hudFont, sb_readyFontScale, 0 );
		h = trap_R_GetTextHeight("W", cgs.media.hudFont, sb_readyFontScale, 0 );
		CG_DrawText( x + spreestart - f - 3, y + (sb_lineHeight - h) - 1, cgs.media.hudFont, sb_readyFontScale, deadColor, "READY", 0, DT_OUTLINE );
	}
}

/*
=================
CG_TeamScoreboardIM
=================
*/
static int CG_TeamScoreboardIM( float x, float y, float w, team_t team ) 
{
	int				i;
	int				skipped;
	float			color[4];
	int				count;
	clientInfo_t	*ci;
	const char*		s;
	int				players;
	qboolean		drawnClient;
	vec4_t			colorBg = {0, 0, 0, 0.9f};

	// Do we make sure the current client is drawn?
	drawnClient = qtrue;
	if ( cg.scores [ cg.snap->ps.clientNum ].team == team )
	{
		drawnClient = qfalse;
	}

	CG_FillRect( x - 5, y, w, 15, colorBg );

	cgs.teamData.redShade[3] = 1.0;
	cgs.teamData.blueShade[3] = 1.0;
	// Determine the color for this team
	switch ( team )
	{
		case TEAM_RED:
			VectorCopy4 ( cgs.teamData.redShade, color );
			break;

		case TEAM_BLUE:
			VectorCopy4 ( cgs.teamData.blueShade, color );
			break;

		case TEAM_FREE:
			VectorCopy4 ( g_color_table[ColorIndex(COLOR_GREEN)], color );
			break;

		case TEAM_SPECTATOR:
		default:
			VectorCopy4 ( colorWhite, color );
			break;
	}

	// Draw as many clients as we can for this team
	for ( skipped = -1, count = 0, i = 0 ; i < cg.numScores && count < sb_maxClients ; i++ ) 
	{
		score_t*	score;

		score = &cg.scores[i];
		ci    = &cgs.clientinfo[ score->client ];

		if ( team != score->team ) 
		{
			continue;
		}

		if ( count == sb_maxClients - 1 && !drawnClient )
		{
			if ( score->client != cg.snap->ps.clientNum )
			{
				skipped = i;
				continue;
			}
			
			drawnClient = qtrue; 
		}

		CG_DrawClientScoreIM( x, y + 15 + sb_lineHeight * count, w, score, color );

		count++;
	}

	if ( skipped != -1 &&  count < sb_maxClients )
	{
		CG_DrawClientScoreIM( x, y + 15 + sb_lineHeight * count, w, &cg.scores[skipped], color );

		count++;
	}

	s = "";
	switch ( team )
	{
		case TEAM_RED:
			s = cgs.teamData.redName;
			players = ui_info_redcount.integer;
			break;

		case TEAM_BLUE:
			s = cgs.teamData.blueName;
			players = ui_info_bluecount.integer;
			break;

		case TEAM_FREE:
			s = "PLAYERS";
			players = ui_info_freecount.integer;
			break;

		default:
		case TEAM_SPECTATOR:
			s = "SPECTATORS";
			players = ui_info_speccount.integer;
			break;
	}

	// Use the same team color here, but alpha it a bit.
	color[3] = 0.6f;

	// Draw the header information for this team
//	CG_DrawPic ( x - 15, y, 10, 15, cgs.media.scoreboardHeader );
//	CG_FillRect( x - 15, y, 10, 15, color );
//	CG_DrawPic ( x - 5, y, w, 15, cgs.media.scoreboardHeader );
	CG_FillRect( x - 5, y, w, 15, color );
	CG_DrawText ( x, y, cgs.media.hudFont, 0.40f, colorWhite, s, 0, 0 );
	CG_DrawText ( x + 100, y + 3, cgs.media.hudFont, 0.30f, colorWhite, va("players: %d", players), 0, 0 );

	// Draw the totals if this is the red or blue team
	if ( (team == TEAM_RED || team == TEAM_BLUE))
	{
		const char* s;
		float		f;

		s = va("%d",(cg.teamScores[team-TEAM_RED]));
		f = trap_R_GetTextWidth ( s, cgs.media.hudFont, 0.43f, 0 );
		CG_DrawText ( x + w - 10 - f, y, cgs.media.hudFont, 0.43f, colorWhite, s, 0, DT_OUTLINE );
	}

//	if ( count )
//	{
////		CG_DrawPic ( x - 15, y + 15 + sb_lineHeight * count, w + 10, sb_lineHeight, cgs.media.scoreboardFooter );
//		CG_DrawPic ( x - 5, y + 15 + sb_lineHeight * count, w, sb_lineHeight, cgs.media.scoreboardFooter );
//	}
	
	y = count * sb_lineHeight + y + 15;

	if ( y > cg.scoreBoardBottom )
	{
		cg.scoreBoardBottom = y;
	}

	return y;
}

/*
=================
CG_DrawNormalScoreboardIM

Draws a scoreboard that has no teams
=================
*/
qboolean CG_DrawNormalScoreboardIM ( float y )
{
	cg.scoreBoardBottom = 0;

	// DRaw the game timer and the game type
	CG_DrawText ( 40,  y - 14, cgs.media.hudFont, 0.38f, colorLtGrey, "Game Time:", 0, DT_OUTLINE );
	CG_DrawTimer ( 110,  y - 14, cgs.media.hudFont, 0.38f, colorLtGrey, DT_OUTLINE, cg.time - cgs.levelStartTime );
	CG_DrawText ( 40,  y - 30, cgs.media.hudFont, 0.38f, colorLtGrey, cgs.gametypeData->displayName, 0, DT_OUTLINE );

	if ( ui_info_speccount.integer )
	{
		CG_FillRect ( 0, 470, 640, 10, colorBlack );
		CG_DrawText ( 5, 470, cgs.media.hudFont, 0.30f, colorWhite, va("SPECTATORS:   %s", cg.scoreBoardSpectators), 0, 0 );
	}

	if ( ui_info_freecount.integer > 13 )
	{
		sb_maxClients	   = 17;
		sb_lineHeight	   = 20;
		sb_nameFontScale   = 0.30f;
		sb_readyFontScale  = 0.30f;
		sb_numberFontScale = trap_R_GetTextHeight ( "W", cgs.media.hudFont, sb_nameFontScale, 0 );
	} 
	else
	{
		sb_maxClients	   = 13;
		sb_lineHeight	   = 25;
		sb_nameFontScale   = 0.35f;
		sb_readyFontScale  = 0.30f;
		sb_numberFontScale = trap_R_GetTextHeight ( "W", cgs.media.hudFont, sb_nameFontScale, 0 ) + 4;
	}

	CG_TeamScoreboardIM ( 20, y, 400, TEAM_FREE );

	return qtrue;
}

/*
=================
CG_DrawTeamScoreboardIM

Draw the normal in-game scoreboard
=================
*/
qboolean CG_DrawTeamScoreboardIM( float y ) 
{
	qboolean redFirst = qfalse;

	cg.scoreBoardBottom = 0;

	// Draw the game timer and the game type
	CG_DrawText ( 40,  y - 14, cgs.media.hudFont, 0.38f, colorLtGrey, "Game Time:", 0, DT_OUTLINE );
	CG_DrawTimer ( 110,  y - 14, cgs.media.hudFont, 0.38f, colorLtGrey, DT_OUTLINE, cg.time - cgs.levelStartTime );
	CG_DrawText ( 40,  y - 30, cgs.media.hudFont, 0.38f, colorLtGrey, cgs.gametypeData->displayName, 0, DT_OUTLINE );

	if ( ui_info_speccount.integer )
	{
		CG_FillRect ( 0, 470, 640, 10, colorBlack );
		CG_DrawText ( 5, 470, cgs.media.hudFont, 0.30f, colorWhite, va("SPECTATORS:   %s", cg.scoreBoardSpectators), 0, 0 );
	}

	if ( ui_info_redcount.integer > 13 || ui_info_bluecount.integer > 13 ) 
	{
		sb_maxClients	   = 32;
		sb_lineHeight	   = 20;
		sb_nameFontScale   = 0.30f;
		sb_readyFontScale  = 0.30f;
		sb_numberFontScale = trap_R_GetTextHeight ( "W", cgs.media.hudFont, sb_nameFontScale, 0 );
	} 
	else
	{
		sb_maxClients	   = 13;
		sb_lineHeight	   = 25;
		sb_nameFontScale   = 0.35f;
		sb_readyFontScale  = 0.30f;
		sb_numberFontScale = trap_R_GetTextHeight ( "W", cgs.media.hudFont, sb_nameFontScale, 0 );
	}

	// If there are more scores than the scoreboard can show then show the 
	// players team first rather than the winning team
	if ( cgs.clientinfo[cg.clientNum].team == TEAM_SPECTATOR )
	{
		if ( cg.teamScores[0] >= cg.teamScores[1] )
		{
			redFirst = qtrue;
		}
	}
	else if ( cgs.clientinfo[cg.clientNum].team == TEAM_RED )
	{
		redFirst = qtrue;
	}

	if ( redFirst ) 
	{
		CG_TeamScoreboardIM( 20, y, 200, TEAM_RED );
		CG_TeamScoreboardIM( 235, y, 200, TEAM_BLUE );
	}
	else
	{
		CG_TeamScoreboardIM( 235, y, 200, TEAM_RED );
		CG_TeamScoreboardIM( 20, y, 200, TEAM_BLUE );
	}

	return qtrue;
}
