/**
 * @file m_node_container.h
 */

/*
Copyright (C) 2002-2010 UFO: Alien Invasion.

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

#ifndef CLIENT_MENU_M_NODE_CONTAINER_H
#define CLIENT_MENU_M_NODE_CONTAINER_H

#include "../../../game/inventory.h"

/** @brief One unit in the containers is 25x25. */
#define C_UNIT				25

/* prototypes */
struct base_s;
struct uiBehaviour_s;
struct uiNode_s;

extern struct inventory_s *ui_inventory;

void UI_RegisterContainerNode(struct uiBehaviour_s *behaviour);
void UI_DrawItem(struct uiNode_s *node, const vec3_t org, const struct item_s *item, int x, int y, const vec3_t scale, const vec4_t color);
void UI_ContainerNodeUpdateEquipment(inventory_t *inv, equipDef_t *ed);

/**
 * @brief extradata for container widget
 * @note everything is not used by all container's type (grid, single, scrolled)
 */
typedef struct containerExtraData_s {
	/* for all containers */
	invDef_t *container;	/**< The container linked to this node. */

	int lastSelectedId;		/**< id oject the object type selected */
	struct uiAction_s *onSelect;	/**< call when we select an item */

	/* for scrolled container */

	int filterEquipType;	/**< A filter */

	int columns;
	qboolean displayWeapon;
	qboolean displayAmmo;
	qboolean displayUnavailableItem;
	qboolean displayAmmoOfWeapon;
	qboolean displayUnavailableAmmoOfWeapon;
	qboolean displayAvailableOnTop;

	int scrollCur;			/**< Index of first item that is displayed. */
	int scrollNum;			/**< Number of items that are displayed. */
	int scrollTotalNum;		/**< Total number of displayable items. */
	struct uiAction_s *onViewChange;	/**< called when view change (scrollCur, scrollNum, scrollTotalNum) */
} containerExtraData_t;


#endif
