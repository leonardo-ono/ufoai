/*
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
/* cl_market.c -- single player market stuff */

#include "client.h"
#include "cl_global.h"

#define MAX_BUYLIST		32

/* ================================ ITEMS / WEAPONS ====================== */

byte	buyList[MAX_BUYLIST];
int		buyListLength;

/*
======================
CL_BuySelectCmd
======================
*/
static void CL_BuySelectCmd( void )
{
	int num;

	if ( Cmd_Argc() < 2 )
	{
		Com_Printf( "Usage: buy_select <num>\n" );
		return;
	}

	num = atoi( Cmd_Argv( 1 ) );
	if ( num >= buyListLength )
		return;

	Cbuf_AddText( va( "buyselect%i\n", num ) );
	CL_ItemDescription( buyList[num] );
}

#define MAX_AIRCRAFT_STORAGE 8
/*
======================
AIR_GetStorageSupplyCount

set storage and supply to the values of aircraft
in use - and the value of aircraft available for
buying
======================
*/
static void AIR_GetStorageSupplyCount( char *aircraft, int *storage, int *supply )
{
	base_t* base;
	aircraft_t* air;
	int i, j;

	*supply = MAX_AIRCRAFT_STORAGE;

	for ( i = 0, base = gd.bases; i < gd.numBases; i++, base++ )
	{
		if ( ! base->founded ) continue;
		for ( j = 0, air = base->aircraft; j < base->numAircraftInBase; j++, air++ )
		{
			if ( !Q_strncmp( air->id, aircraft, MAX_VAR ) )
			{
				*storage++;
			}
		}
	}
	if ( *storage < MAX_AIRCRAFT_STORAGE )
		*supply -= *storage;
	else
		*supply = 0;
}

/*
======================
CL_BuyType
======================
*/
static void CL_BuyType( void )
{
	objDef_t	*od;
	aircraft_t	*air;
	technology_t	*tech;
	int		i, j = 0, num, storage, supply;
	char	str[MAX_VAR];

	if ( Cmd_Argc() < 2 )
	{
		Com_Printf( "Usage: buy_type <category>\n" );
		return;
	}
	num = atoi( Cmd_Argv( 1 ) );

	CL_UpdateCredits( ccs.credits );

	if ( num < NUM_BUYTYPES ) /* 'normal' items */
	{
		/* get item list */
		for ( i = 0, j = 0, od = csi.ods; i < csi.numODs; i++, od++ )
		{
			tech = (technology_t*)od->tech;
			if ( !tech || RS_Collected_(tech) || RS_IsResearched_ptr(tech) ) {
				if ( od->buytype == num && (ccs.eCampaign.num[i] || ccs.eMarket.num[i]) )
				{
					Q_strncpyz( str, va("mn_item%i", j), MAX_VAR );
					Cvar_Set( str, _(od->name) );

					Q_strncpyz( str, va("mn_storage%i", j), MAX_VAR );
					Cvar_SetValue( str, ccs.eCampaign.num[i] );

					Q_strncpyz( str, va("mn_supply%i", j), MAX_VAR );
					Cvar_SetValue( str, ccs.eMarket.num[i] );

					Q_strncpyz( str, va("mn_price%i", j), MAX_VAR );
					Cvar_Set( str, va( "%i c", od->price ) );

					buyList[j] = i;
					j++;
				}
			} /* is researched OR collected */
		}
	}
	else if ( num == NUM_BUYTYPES ) /* aircraft */
	{
		for ( i = 0, j = 0, air = aircraft; i < numAircraft; i++, air++ )
		{
			AIR_GetStorageSupplyCount( air->id, &storage, &supply );
			Q_strncpyz( str, va("mn_item%i", j), MAX_VAR );
			Cvar_Set( str, _(air->name) );

			Q_strncpyz( str, va("mn_storage%i", j), MAX_VAR );
			Cvar_SetValue( str, storage );

			Q_strncpyz( str, va("mn_supply%i", j), MAX_VAR );
			Cvar_SetValue( str, supply );

			Q_strncpyz( str, va("mn_price%i", j), MAX_VAR );
			Cvar_Set( str, va( "%i c", air->price ) );

			buyList[j] = i;
			j++;
		}
	}

	buyListLength = j;

	/* FIXME: This list needs to be scrollable - so a hardcoded end is bad */
	for ( i = 0; j < 28; j++ )
	{
		Cvar_Set( va( "mn_item%i", j ), "" );
		Cvar_Set( va( "mn_storage%i", j ), "" );
		Cvar_Set( va( "mn_supply%i", j ), "" );
		Cvar_Set( va( "mn_price%i", j ), "" );
	}

	/* select first item */
	if ( buyListLength )
	{
		Cbuf_AddText( "buyselect0\n" );
		CL_ItemDescription( buyList[0] );
	} else {
		/* reset description */
		Cvar_Set( "mn_itemname", "" );
		Cvar_Set( "mn_item", "" );
		Cvar_Set( "mn_weapon", "" );
		Cvar_Set( "mn_ammo", "" );
		menuText[TEXT_STANDARD] = NULL;
	}
}


/*
======================
CL_BuyItem
======================
*/
static void CL_BuyItem( void )
{
	int num, item;

	if ( Cmd_Argc() < 2 )
	{
		Com_Printf( "Usage: mn_buy <num>\n" );
		return;
	}

	num = atoi( Cmd_Argv( 1 ) );
	if ( num < 0 || num >= buyListLength )
		return;

	item = buyList[num];
	Cbuf_AddText( va( "buyselect%i\n", num ) );
	CL_ItemDescription( item );
	Com_DPrintf("item %i\n", item );

	if ( ccs.credits >= csi.ods[item].price && ccs.eMarket.num[item] )
	{
		Cvar_SetValue( va( "mn_storage%i", num ), ++ccs.eCampaign.num[item] );
		Cvar_SetValue( va( "mn_supply%i", num ),  --ccs.eMarket.num[item] );
		CL_UpdateCredits( ccs.credits-csi.ods[item].price );
	}
	RS_MarkCollected();
	RS_MarkResearchable();
}

/*
======================
CL_SellItem
======================
*/
static void CL_SellItem( void )
{
	int num, item;

	if ( Cmd_Argc() < 2 )
	{
		Com_Printf( "Usage: mn_sell <num>\n" );
		return;
	}

	num = atoi( Cmd_Argv( 1 ) );
	if ( num < 0 || num >= buyListLength )
		return;

	item = buyList[num];
	Cbuf_AddText( va( "buyselect%i\n", num ) );
	CL_ItemDescription( item );

	if ( ccs.eCampaign.num[item] )
	{
		Cvar_SetValue( va( "mn_storage%i", num ), --ccs.eCampaign.num[item] );
		Cvar_SetValue( va( "mn_supply%i", num ),  ++ccs.eMarket.num[item] );
		CL_UpdateCredits( ccs.credits+csi.ods[item].price );
	}
}

/*
======================
CL_BuyAircraft
======================
*/
static void CL_BuyAircraft( void )
{
	int num, aircraftID;

	if ( Cmd_Argc() < 2 )
	{
		Com_Printf( "Usage: mn_buy_aircraft <num>\n" );
		return;
	}

	num = atoi( Cmd_Argv( 1 ) );
	if ( num < 0 || num >= buyListLength )
		return;

	aircraftID = buyList[num];
	Cbuf_AddText( va( "buyselect%i\n", num ) );

/* 	if ( ccs.credits >= csi.ods[item].price && ccs.eMarket.num[item] ) */
/* 	{ */
/* 		Cvar_SetValue( va( "mn_storage%i", num ), ++ccs.eCampaign.num[item] ); */
/* 		Cvar_SetValue( va( "mn_supply%i", num ),  --ccs.eMarket.num[item] ); */
/* 		CL_UpdateCredits( ccs.credits-csi.ods[item].price ); */
/* 	} */
}


/*
======================
CL_SellAircraft

FIXME: This needs work in reassigning the base aircraft array
       or the other functions need to check whether the aircraft
       at current arraypos is valid
======================
*/
static void CL_SellAircraft( void )
{
	int num, aircraftID, i, j;
	base_t*	base;
	aircraft_t*	air;
	qboolean	found = qfalse;

	if ( Cmd_Argc() < 2 )
	{
		Com_Printf( "Usage: mn_sell_aircraft <num>\n" );
		return;
	}

	num = atoi( Cmd_Argv( 1 ) );
	if ( num < 0 || num >= buyListLength )
		return;

	aircraftID = buyList[num];
	if ( aircraftID > numAircraft )
		return;

	for ( i = 0, base = gd.bases; i < gd.numBases; i++, base++ )
	{
		if ( ! base->founded ) continue;
		for ( j = 0, air = base->aircraft; j < base->numAircraftInBase; j++, air++ )
		{
			if ( !Q_strncmp( air->id, aircraft[aircraftID].id, MAX_VAR ) )
			{
				if ( *air->teamSize )
					continue;
				found = qtrue;
				break;
			}
		}
		/* ok, we've found an empty aircraft (no team) in a base */
		/* so now we can sell it */
		if ( found )
		{
			/* FIXME: Do the selling here... */
			/* reassign the aircraft-array in base_t */
			/* maybe a linked list would be the best in base_t */

			/* delete this aircraft in base */
			memset( &base->aircraft[j], 0, sizeof(aircraft_t) );

			/* last entry - we don't have to search for this anymore */
			if ( j == base->numAircraftInBase-1 )
				base->numAircraftInBase--;

			CL_UpdateCredits( ccs.credits + air->price );
			return;
		}
	}
	if ( ! found )
		Com_Printf("...there are no aircraft available (with no team assigned) for selling\n");
}

/*
======================
CL_ResetMarket
======================
*/
void CL_ResetMarket( void )
{
	Cmd_AddCommand( "buy_type", CL_BuyType );
	Cmd_AddCommand( "buy_select", CL_BuySelectCmd );
	Cmd_AddCommand( "mn_buy", CL_BuyItem );
	Cmd_AddCommand( "mn_sell", CL_SellItem );
	Cmd_AddCommand( "mn_buy_aircraft", CL_BuyAircraft );
	Cmd_AddCommand( "mn_sell_aircraft", CL_SellAircraft );
	buyListLength = -1;
}
