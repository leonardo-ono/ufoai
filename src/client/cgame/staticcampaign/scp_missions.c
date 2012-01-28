/**
 * @file scp_missions.c
 * @brief Singleplayer static campaign mission code
 */

/*
 Copyright (C) 2002-2011 UFO: Alien Invasion.

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

#include "scp_missions.h"
#include "../campaign/cp_time.h"
#include "../campaign/cp_missions.h"
#include "scp_shared.h"
#include "../../../common/binaryexpressionparser.h"

static qboolean SCP_StageSetDone (const char *name)
{
	setState_t *set;
	int i;

	for (i = 0, set = &scd->set[scd->testStage->first]; i < scd->testStage->num; i++, set++)
		if (Q_streq(name, set->def->name))
			return set->done >= set->def->quota;

	/* didn't find set */
	return qfalse;
}

static void SCP_CampaignActivateStageSets (stage_t *stage)
{
	setState_t *set;
	int i;

	scd->testStage = stage;
	for (i = 0, set = &scd->set[stage->first]; i < stage->num; i++, set++)
		if (!set->active && !set->done && !set->num) {
			const date_t zero = {0, 0};
			/* check needed sets */
			if (set->def->needed[0] && !BEP_Evaluate(set->def->needed, SCP_StageSetDone))
				continue;

			/* activate it */
			set->active = qtrue;
			set->start = Date_Add(ccs.date, set->def->delay);
			set->event = Date_Add(set->start, Date_Random(zero, set->def->frame));
		}
}

static stageState_t *SCP_CampaignActivateStage (const char *name)
{
	stage_t *stage;
	stageState_t *state;
	int i, j;

	for (i = 0, stage = scd->stages; i < scd->numStages; i++, stage++) {
		if (Q_streq(stage->name, name)) {
			/* add it to the list */
			state = &scd->stage[i];
			state->active = qtrue;
			state->def = stage;
			state->start = ccs.date;

			/* add stage sets */
			for (j = stage->first; j < stage->first + stage->num; j++) {
				OBJZERO(scd->set[j]);
				scd->set[j].stage = &stage[j];
				scd->set[j].def = &scd->stageSets[j];
			}

			/* activate stage sets */
			SCP_CampaignActivateStageSets(stage);
			return state;
		}
	}

	Com_Printf("SCP_CampaignActivateStage: stage '%s' not found.\n", name);
	return NULL;
}

static void SCP_CampaignEndStage (const char *name)
{
	stageState_t *state;
	int i;

	for (i = 0, state = scd->stage; i < scd->numStages; i++, state++)
		if (Q_streq(state->def->name, name)) {
			state->active = qfalse;
			return;
		}

	Com_Printf("SCP_CampaignEndStage: stage '%s' not found.\n", name);
}

static void SCP_CampaignExecute (setState_t *set)
{
	/* handle stages, execute commands */
	if (set->def->nextstage[0] != '\0')
		SCP_CampaignActivateStage(set->def->nextstage);

	if (set->def->endstage[0] != '\0')
		SCP_CampaignEndStage(set->def->endstage);

	if (set->def->cmds[0] != '\0')
		Cbuf_AddText(set->def->cmds);

	/* activate new sets in old stage */
	SCP_CampaignActivateStageSets(set->stage);
}

static void SCP_CampaignAddMission (setState_t *set)
{
	actMis_t *mis;

	/* add mission */
	if (scd->numMissions >= MAX_ACTMISSIONS) {
		Com_Printf("SCP_CampaignAddMission: Too many active missions!\n");
		return;
	}
	mis = &scd->mission[scd->numMissions++];
	OBJZERO(*mis);

	/* set relevant info */
	mis->def = &scd->missions[set->def->missions[(int) (set->def->numMissions * frand())]];
	mis->cause = set;

	/* prepare next event (if any) */
	set->num++;
	if (set->def->number && set->num >= set->def->number) {
		set->active = qfalse;
	} else {
		const date_t minTime = {0, 0};
		set->event = Date_Add(ccs.date, Date_Random(minTime, set->def->frame));
	}

	mission_t * mission = CP_CreateNewMission(INTERESTCATEGORY_TERROR_ATTACK, qtrue);
	mission->mapDef = Com_GetMapDefinitionByID(mis->def->mapDef);
	if (!mission->mapDef) {
		Com_Printf("SCP_CampaignAddMission: Could not get the mapdef '%s'\n", mis->def->mapDef);
		CP_MissionRemove(mission);
		return;
	}
	Vector2Copy(mis->def->pos, mission->pos);
	mission->posAssigned = qtrue;
	CP_TerrorMissionStart(mission);
	mission->finalDate = mis->expire;
}

static void SCP_CampaignRemoveMission (actMis_t *mis)
{
	int i, num;

	num = mis - scd->mission;
	if (num >= scd->numMissions) {
		Com_Printf("SCP_CampaignRemoveMission: Can't remove mission.\n");
		return;
	}

	scd->numMissions--;
	for (i = num; i < scd->numMissions; i++)
		scd->mission[i] = scd->mission[i + 1];
}

void SCP_SpawnNewMissions (void)
{
	stageState_t *stage;
	actMis_t *mis;
	int i, j;

	/* check campaign events */
	for (i = 0, stage = scd->stage; i < scd->numStages; i++, stage++) {
		setState_t *set;
		if (stage->active) {
			for (j = 0, set = &scd->set[stage->def->first]; j < stage->def->num; j++, set++)
				if (set->active && set->event.day && Date_LaterThan(&ccs.date, &set->event)) {
					if (set->def->numMissions) {
						SCP_CampaignAddMission(set);
					} else {
						SCP_CampaignExecute(set);
					}
				}
		}
	}

	/* let missions expire */
	for (i = 0, mis = scd->mission; i < scd->numMissions; i++, mis++) {
		if (mis->expire.day && Date_LaterThan(&ccs.date, &mis->expire)) {
			SCP_CampaignRemoveMission(mis);
		}
	}
}

void SCP_CampaignActivateFirstStage (void)
{
	SCP_CampaignActivateStage("intro");
}
