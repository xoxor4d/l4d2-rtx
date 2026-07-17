#pragma once

namespace l4d2
{
	// --------------
	// game variables

	

	// --------------
	// game functions


	// --------------
	// game asm offsets

	// - server
	extern uint32_t hk_addr__scene_ent_on_start_event;
	extern uint32_t hk_addr__scene_ent_on_finish_event;

	// - engine
	extern uint32_t hk_addr__on_map_load;
	extern uint32_t hk_addr__on_host_disconnect;
	extern uint32_t hk_addr__on_host_change_level;
	extern uint32_t nop_addr__cdispinfo_render;
	extern uint32_t hk_addr__pre_recursive_world_node;
	extern uint32_t jmp_addr__cullnode01;
	extern uint32_t retn_addr__cullnode_cull;
	extern uint32_t retn_addr__cullnode_skip;
	extern uint32_t nop_addr_cullnode_backface_check01;
	extern uint32_t nop_addr_cullnode_backface_check02;
	extern uint32_t nop_addr_drawleaf_backface_check;
	extern uint32_t nop_addr_draw_opaque_bmodel_backface_check;
	
	// - client
	extern uint32_t hk_addr__cviewrenderer_renderview;
	extern uint32_t hk_addr__skyboxview_draw_internal;
	
	extern uint32_t hk_addr__01;

	extern uint32_t retn_addr__01;

	extern uint32_t jmp_addr__01;

	extern uint32_t nop_addr__01;

	// ---

	void init_game_addresses();
	void main();
}
