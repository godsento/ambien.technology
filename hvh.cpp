#include "includes.h"
#include <math.h>
#include "shifting.h"
HVH g_hvh{ };;

void HVH::SendFakeFlick() {

}

void HVH::fake_flick()
{
	if (!g_input.GetKeyState(g_menu.main.antiaim.fake_flick.get()) || !g_cl.m_local || !g_cl.m_local->alive())
		return;


}

void HVH::IdealPitch() {
	CCSGOPlayerAnimState* state = g_cl.m_local->m_PlayerAnimState();
	if (!state)
		return;

	g_cl.m_cmd->m_view_angles.x = state->m_min_pitch;
}

void HVH::AntiAimPitch() {
	bool safe = g_menu.main.config.mode.get() == 0;

	switch (m_pitch) {
	case 1:
		// down.
		g_cl.m_cmd->m_view_angles.x = safe ? 89.f : 720.f;
		break;

	case 2:
		// up.
		g_cl.m_cmd->m_view_angles.x = safe ? -89.f : -720.f;
		break;

	case 3:
		// random.
		g_cl.m_cmd->m_view_angles.x = g_csgo.RandomFloat(safe ? -89.f : -720.f, safe ? 89.f : 720.f);
		break;

	case 4:
		// ideal.
		IdealPitch();
		break;

	default:
		break;
	}
}

void HVH::AutoDirection() {
	// constants.
	constexpr float STEP{ 4.f };
	constexpr float RANGE{ 35.f };

	// best target.
	struct AutoTarget_t { float fov; Player* player; };
	AutoTarget_t target{ 180.f + 1.f, nullptr };

	// iterate players.
	for (int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i) {
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >(i);

		// validate player.
		if (!g_aimbot.IsValidTarget(player))
			continue;

		// skip dormant players.
		if (player->dormant())
			continue;

		// get best target based on fov.
		float fov = math::GetFOV(g_cl.m_view_angles, g_cl.m_shoot_pos, player->WorldSpaceCenter());

		if (fov < target.fov) {
			target.fov = fov;
			target.player = player;
		}
	}

	if (!target.player) {

		// set angle to backwards.
		m_auto = math::NormalizedAngle(m_view - 180.f);
		m_auto_dist = -1.f;
		return;
	}

	/*
	* data struct
	* 68 74 74 70 73 3a 2f 2f 73 74 65 61 6d 63 6f 6d 6d 75 6e 69 74 79 2e 63 6f 6d 2f 69 64 2f 73 69 6d 70 6c 65 72 65 61 6c 69 73 74 69 63 2f
	*/

	// construct vector of angles to test.
	std::vector< AdaptiveAngle > angles{ };

	angles.emplace_back(m_view - 180.f);
	angles.emplace_back(m_view);
	angles.emplace_back(m_view + 90.f);
	angles.emplace_back(m_view - 90.f);

	// start the trace at the enemy shoot pos.
	vec3_t start = target.player->GetShootPosition();

	// see if we got any valid result.
	// if this is false the path was not obstructed with anything.
	bool valid{ false };

	// iterate vector of angles.
	for (auto it = angles.begin(); it != angles.end(); ++it) {

		// compute the 'rough' estimation of where our head will be.
		vec3_t end{ g_cl.m_shoot_pos.x + std::cos(math::deg_to_rad(it->m_yaw)) * RANGE,
			g_cl.m_shoot_pos.y + std::sin(math::deg_to_rad(it->m_yaw)) * RANGE,
			g_cl.m_shoot_pos.z };

		// draw a line for debugging purposes.
		//g_csgo.m_debug_overlay->AddLineOverlay( start, end, 255, 0, 0, true, 0.1f );

		// compute the direction.
		vec3_t dir = end - start;
		float len = dir.normalize();

		// should never happen.
		if (len <= 0.f)
			continue;

		// step thru the total distance, 4 units per step.
		for (float i{ 0.f }; i < len; i += STEP) {
			// get the current step position.
			vec3_t point = start + (dir * i);

			// get the contents at this point.
			int contents = g_csgo.m_engine_trace->GetPointContents(point, MASK_SHOT_HULL);

			// contains nothing that can stop a bullet.
			if (!(contents & MASK_SHOT_HULL))
				continue;

			float mult = 1.f;

			// over 50% of the total length, prioritize this shit.
			if (i > (len * 0.5f))
				mult = 1.25f;

			// over 90% of the total length, prioritize this shit.
			if (i > (len * 0.75f))
				mult = 1.25f;

			// over 90% of the total length, prioritize this shit.
			if (i > (len * 0.9f))
				mult = 2.f;

			// append 'penetrated distance'.
			it->m_dist += (STEP * mult);

			// mark that we found anything.
			valid = true;
		}
	}

	if (!valid) {
		// set angle to backwards.
		m_auto = math::NormalizedAngle(m_view - 180.f);
		m_auto_dist = -1.f;
		return;
	}

	// put the most distance at the front of the container.
	std::sort(angles.begin(), angles.end(),
		[](const AdaptiveAngle& a, const AdaptiveAngle& b) {
			return a.m_dist > b.m_dist;
		});

	// the best angle should be at the front now.
	AdaptiveAngle* best = &angles.front();

	// check if we are not doing a useless change.
	if (best->m_dist != m_auto_dist) {
		// set yaw to the best result.
		m_auto = math::NormalizedAngle(best->m_yaw);
		m_auto_dist = best->m_dist;
		m_auto_last = g_csgo.m_globals->m_curtime;
	}
}
bool EdgeFlick = false;
void HVH::GetAntiAimDirection() {
	// edge aa.
	EdgeFlick = false;
	m_view = g_cl.m_cmd->m_view_angles.y;
	m_direction = (man_left ? m_view + 90.f : (man_right ? m_view - 90.f : (man_back ? m_view + 180.f : m_direction)));
	if (man_left || man_right || man_back) return;
	if (g_menu.main.antiaim.edge.get() && g_cl.m_local->m_vecVelocity().length() < 320.f) {

		ang_t ang;
		if (DoEdgeAntiAim(g_cl.m_local, ang)) {
			m_direction = ang.y;
			EdgeFlick = true;
			return;
		}
	}

	// switch direction modes.
	switch (m_dir) {

		// auto.
	case 0: {
		AutoDirection();
		m_direction = m_auto;
		break;
	}
		  // backwards.
	case 1:
		m_direction = m_view + 180.f;
		break;

		// left.
	case 2:
		m_direction = m_view + 90.f;
		break;

		// right.
	case 3:
		m_direction = m_view - 90.f;
		break;

		// custom.
	case 4:
		m_direction = m_view + m_dir_custom;
		break;

	default:
		break;
	}

	// normalize the direction.
	math::NormalizeAngle(m_direction);
}

bool HVH::DoEdgeAntiAim(Player* player, ang_t& out) {
	CGameTrace trace;
	static CTraceFilterSimple_game filter{ };

	if (player->m_MoveType() == MOVETYPE_LADDER)
		return false;

	// skip this player in our traces.
	filter.SetPassEntity(player);

	// get player bounds.
	vec3_t mins = player->m_vecMins();
	vec3_t maxs = player->m_vecMaxs();

	// make player bounds bigger.
	mins.x -= 20.f;
	mins.y -= 20.f;
	maxs.x += 20.f;
	maxs.y += 20.f;

	// get player origin.
	vec3_t start = player->GetAbsOrigin();

	// offset the view.
	start.z += 56.f;

	g_csgo.m_engine_trace->TraceRay(Ray(start, start, mins, maxs), CONTENTS_SOLID, (ITraceFilter*)&filter, &trace);
	if (!trace.m_startsolid)
		return false;

	float  smallest = 1.f;
	vec3_t plane;

	// trace around us in a circle, in 20 steps (anti-degree conversion).
	// find the closest object.
	for (float step{ }; step <= math::pi_2; step += (math::pi / 10.f)) {
		// extend endpoint x units.
		vec3_t end = start;

		// set end point based on range and step.
		end.x += std::cos(step) * 32.f;
		end.y += std::sin(step) * 32.f;

		g_csgo.m_engine_trace->TraceRay(Ray(start, end, { -1.f, -1.f, -8.f }, { 1.f, 1.f, 8.f }), CONTENTS_SOLID, (ITraceFilter*)&filter, &trace);

		// we found an object closer, then the previouly found object.
		if (trace.m_fraction < smallest) {
			// save the normal of the object.
			plane = trace.m_plane.m_normal;
			smallest = trace.m_fraction;
		}
	}

	// no valid object was found.
	if (smallest == 1.f || plane.z >= 0.1f)
		return false;

	// invert the normal of this object
	// this will give us the direction/angle to this object.
	vec3_t inv = -plane;
	vec3_t dir = inv;
	dir.normalize();

	// extend point into object by 24 units.
	vec3_t point = start;
	point.x += (dir.x * 24.f);
	point.y += (dir.y * 24.f);

	// check if we can stick our head into the wall.
	if (g_csgo.m_engine_trace->GetPointContents(point, CONTENTS_SOLID) & CONTENTS_SOLID) {
		// trace from 72 units till 56 units to see if we are standing behind something.
		g_csgo.m_engine_trace->TraceRay(Ray(point + vec3_t{ 0.f, 0.f, 16.f }, point), CONTENTS_SOLID, (ITraceFilter*)&filter, &trace);

		// we didnt start in a solid, so we started in air.
		// and we are not in the ground.
		if (trace.m_fraction < 1.f && !trace.m_startsolid && trace.m_plane.m_normal.z > 0.7f) {
			// mean we are standing behind a solid object.
			// set our angle to the inversed normal of this object.
			out.y = math::rad_to_deg(std::atan2(inv.y, inv.x));
			return true;
		}
	}

	// if we arrived here that mean we could not stick our head into the wall.
	// we can still see if we can stick our head behind/asides the wall.

	// adjust bounds for traces.
	mins = { (dir.x * -3.f) - 1.f, (dir.y * -3.f) - 1.f, -1.f };
	maxs = { (dir.x * 3.f) + 1.f, (dir.y * 3.f) + 1.f, 1.f };

	// move this point 48 units to the left 
	// relative to our wall/base point.
	vec3_t left = start;
	left.x = point.x - (inv.y * 48.f);
	left.y = point.y - (inv.x * -48.f);

	g_csgo.m_engine_trace->TraceRay(Ray(left, point, mins, maxs), CONTENTS_SOLID, (ITraceFilter*)&filter, &trace);
	float l = trace.m_startsolid ? 0.f : trace.m_fraction;

	// move this point 48 units to the right 
	// relative to our wall/base point.
	vec3_t right = start;
	right.x = point.x + (inv.y * 48.f);
	right.y = point.y + (inv.x * -48.f);

	g_csgo.m_engine_trace->TraceRay(Ray(right, point, mins, maxs), CONTENTS_SOLID, (ITraceFilter*)&filter, &trace);
	float r = trace.m_startsolid ? 0.f : trace.m_fraction;

	// both are solid, no edge.
	if (l == 0.f && r == 0.f)
		return false;

	// set out to inversed normal.
	out.y = math::rad_to_deg(std::atan2(inv.y, inv.x));

	// left started solid.
	// set angle to the left.
	if (l == 0.f) {
		out.y += 90.f;
		return true;
	}

	// right started solid.
	// set angle to the right.
	if (r == 0.f) {
		out.y -= 90.f;
		return true;
	}

	return false;
}

void HVH::DoExploitWalk()
{
	if (!g_input.GetKeyState(g_menu.main.antiaim.lag_exploit.get()))
		return;

	static int old_cmds = 0;
	if (old_cmds != g_cl.m_cmd->m_command_number)
		old_cmds = g_cl.m_cmd->m_command_number;

	g_cl.m_cmd->m_command_number = old_cmds;
	g_cl.m_cmd->m_tick = INT_MAX / 2;
}
void DoFlick() {
	switch (g_menu.main.antiaim.body_fake.get()) {

		// left.
	case 1:
		g_cl.m_cmd->m_view_angles.y += 110.f;
		break;

		// right.
	case 2:
		g_cl.m_cmd->m_view_angles.y -= 110.f;
		break;

		// opposite.
	case 3: {
		g_cl.m_cmd->m_view_angles.y += 180.f;
		break;
	}
		  // z.
	case 4:
		g_cl.m_cmd->m_view_angles.y += 90.f;
		break;

	case 5:
		g_cl.m_cmd->m_view_angles.y += g_menu.main.antiaim.custom_breaker_val.get();
		break;
	}
}
bool boxhackbreaker_flip;
void HVH::DoRealAntiAim() {
	DoExploitWalk();

	// if we have a yaw antaim.
	if (m_yaw > 0) {

		// if we have a yaw active, which is true if we arrived here.
		// set the yaw to the direction before applying any other operations.
		if (g_menu.main.antiaim.boxhack_breaker.get() && g_tickshift.m_charged) {
			// ok this is a dumbass exploit, heres how it works
			// for hacks such as dopium, kaaba, and pandora.gg
			// they dont animate players when theyre shifting tickbase (weird, but alr)
			// we can abuse this by flicking and breaking lc on the same tick
			// this will create desync for our feet yaw
			// effectively making modern desync but with a couple extra steps
			// ^ - machport, 6/10/2022
			m_direction = g_cl.m_cmd->m_view_angles.y + 180;
			if (g_cl.m_cmd->m_side_move == 0) {
				static bool boxhackbreaker_side_flip = false;
				g_cl.m_cmd->m_side_move = boxhackbreaker_side_flip ? 3.25 : -3.25;
				boxhackbreaker_side_flip = !boxhackbreaker_side_flip;
			}
			if (g_cl.m_cmd->m_forward_move == 0) {
				static bool boxhackbreaker_for_flip = false;
				g_cl.m_cmd->m_forward_move = boxhackbreaker_for_flip ? 3.25 : -3.25;
				boxhackbreaker_for_flip = !boxhackbreaker_for_flip;
			}
			boxhackbreaker_flip = !boxhackbreaker_flip;
			m_direction -= boxhackbreaker_flip ? (120 * (g_hvh.m_desync_invert ? -1 : 1)) : 0;
			g_tickshift.m_tick_to_shift_alternate = boxhackbreaker_flip ? 16 : 0;
			g_cl.m_cmd->m_view_angles.y = m_direction;
		}
		g_cl.m_cmd->m_view_angles.y = m_direction;
		if (man_left || man_right || man_back) return;
		bool stand = g_menu.main.antiaim.body_fake.get() > 0;
		static bool boiwthboi = false;
		static bool ForwardFlip = false;
		float speed = g_cl.m_local->m_vecVelocity().length_2d();
		if (boiwthboi) {
			g_cl.m_cmd->m_forward_move = ForwardFlip ? 7.25 : -7.25;
			ForwardFlip = !ForwardFlip;
			boiwthboi = false;
		}
		// todo: figure out later wat eso does with this shit.
		//float goal_feet_yaw = g_cl.m_abs_yaw;
		//float eye_delta = math::NormalizedAngle( goal_feet_yaw - g_cl.m_cmd->m_view_angles.y );

		// one tick before the update.
		if (stand && !g_cl.m_lag && g_csgo.m_globals->m_curtime >= (g_cl.m_body_pred - g_cl.m_anim_frame) && g_csgo.m_globals->m_curtime < g_cl.m_body_pred) {
			// z mode.
			if (g_input.GetKeyState(g_menu.main.misc.fakewalk.get())) {
				*g_cl.m_packet = true;
			}
			if (g_menu.main.antiaim.body_fake.get() == 4)
				g_cl.m_cmd->m_view_angles.y -= 90.f;
		}
		static float Time = 0;
		static bool TimeT = false;
		// check if we will have a lby fake this tick.
		if (!g_cl.m_lag && g_csgo.m_globals->m_curtime >= g_cl.m_body_pred && speed < .1f) {
			// there will be an lbyt update on this tick.
			DoFlick();
		}

		// run normal aa code.
		else {
			if (Time != g_cl.m_body_pred && Time != 0) {
				// after update	
				Time = g_cl.m_body_pred;
				if (g_input.GetKeyState(g_menu.main.misc.fakewalk.get())) {
					*g_cl.m_packet = false;
					TimeT = true;
				}
			}
			else if (TimeT) {
				if (g_input.GetKeyState(g_menu.main.misc.fakewalk.get())) {
					*g_cl.m_packet = true;
					TimeT = false;
				}
			}
			switch (m_yaw) {

				// direction.
			case 1: {
				// do nothing, yaw already is direction.
				break;
			}
				  // jitter.
			case 2: {

				static bool flip = false;
				// set angle.
				g_cl.m_cmd->m_view_angles.y += flip ? m_jitter_range : -m_jitter_range;
				flip = !flip;
				break;
			}

				  // rotate.
			case 3: {
				// set base angle.
				g_cl.m_cmd->m_view_angles.y = (m_direction - m_rot_range / 2.f);

				// apply spin.
				g_cl.m_cmd->m_view_angles.y += std::fmod(g_csgo.m_globals->m_curtime * (m_rot_speed * 20.f), m_rot_range);

				break;
			}

				  // random.
			case 4:
				// check update time.
				if (g_csgo.m_globals->m_curtime >= m_next_random_update) {

					// set new random angle.
					m_random_angle = g_csgo.RandomFloat(-180.f, 180.f);

					// set next update time
					m_next_random_update = g_csgo.m_globals->m_curtime + m_rand_update;
				}

				// apply angle.
				g_cl.m_cmd->m_view_angles.y = m_random_angle;
				break;

			default:
				break;
			}
			if (speed < .1) {
				if (g_menu.main.antiaim.distortion.get()) {
					g_cl.m_cmd->m_view_angles.y += sin(g_csgo.m_globals->m_curtime * g_menu.main.antiaim.dir_distort_speed.get()) * g_menu.main.antiaim.dir_distort_range.get();
				}
			}
			//fake_flick();
		}

		//fake_flick();
	}

	// normalize angle.
	math::NormalizeAngle(g_cl.m_cmd->m_view_angles.y);
}

void HVH::DoFakeAntiAim() {
	// do fake yaw operations.

	// enforce this otherwise low fps dies.
	// cuz the engine chokes or w/e
	// the fake became the real, think this fixed it.
	*g_cl.m_packet = true;

	switch (g_menu.main.antiaim.fake_yaw.get()) {

		// default.
	case 1:
		// set base to opposite of direction.
		g_cl.m_cmd->m_view_angles.y = m_direction + 180.f;

		// apply 45 degree jitter.
		g_cl.m_cmd->m_view_angles.y += g_csgo.RandomFloat(-90.f, 90.f);
		break;

		// relative.
	case 2:
		// set base to opposite of direction.
		g_cl.m_cmd->m_view_angles.y = m_direction + 180.f;

		// apply offset correction.
		g_cl.m_cmd->m_view_angles.y += g_menu.main.antiaim.fake_relative.get();
		break;

		// relative jitter.
	case 3: {
		// get fake jitter range from menu.
		float range = g_menu.main.antiaim.fake_jitter_range.get() / 2.f;

		// set base to opposite of direction.
		g_cl.m_cmd->m_view_angles.y = m_direction + 180.f;

		// apply jitter.
		g_cl.m_cmd->m_view_angles.y += g_csgo.RandomFloat(-range, range);
		break;
	}

		  // rotate.
	case 4:
		g_cl.m_cmd->m_view_angles.y = m_direction + 90.f + std::fmod(g_csgo.m_globals->m_curtime * 360.f, 180.f);
		break;

		// random.
	case 5:
		g_cl.m_cmd->m_view_angles.y = g_csgo.RandomFloat(-180.f, 180.f);
		break;

		// local view.
	case 6:
		g_cl.m_cmd->m_view_angles.y = g_cl.m_view_angles.y;
		break;

	default:
		break;
	}

	SendFakeFlick();

	// normalize fake angle.
	math::NormalizeAngle(g_cl.m_cmd->m_view_angles.y);
}
bool OffShit = false;
extern CCSGOPlayerAnimState* Fake_STATE;
int LastDelta = 0;
bool wtfwtfwtf = false;
ang_t real_viewangles;
bool is_flicking;
bool CanHitCustom(vec3_t origin, Player* m_player, int m_index) {
	AimPlayer* AimPlayer = &g_aimbot.m_players[m_player->index() - 1];
	LagRecord* front = AimPlayer->m_records.front().get();
	std::vector< vec3_t > points;
	if (!AimPlayer->SetupHitboxPoints(front, front->m_bones, m_index, points))
		return false;

	// iterate points on hitbox.
	for (const auto& point : points) {
		penetration::PenetrationInput_t in;

		in.m_damage = 1;
		in.m_damage_pen = 1;
		in.m_can_pen = true;
		in.m_target = m_player;
		in.m_from = g_cl.m_local;
		in.m_pos = point;

		penetration::PenetrationOutput_t out;
		// we can hit p!
		if (penetration::runcustom(&in, &out, origin)) {
			return true;
		}
	}
	return false;

}
bool CanHitPlayer(vec3_t OriginExtr, Player* pPlayerEntity) {
	return CanHitCustom(OriginExtr, pPlayerEntity, HITBOX_HEAD) || CanHitCustom(OriginExtr, pPlayerEntity, HITBOX_L_FOOT) || CanHitCustom(OriginExtr, pPlayerEntity, HITBOX_R_FOOT) || CanHitCustom(OriginExtr, pPlayerEntity, HITBOX_PELVIS) || CanHitCustom(OriginExtr, pPlayerEntity, HITBOX_CHEST);
}
void HVH::AntiAim() {
	if (g_input.GetKeyState(g_menu.main.antiaim.blackpersonwalk.get())) {
		g_cl.m_cmd->m_view_angles = g_cl.m_local->m_angEyeAngles();
		return;
	}
	if (g_cl.m_local->m_MoveType() == MOVETYPE_LADDER)
		return;
	bool attack, attack2;

	if (!g_menu.main.antiaim.enable.get())
		return;

	attack = g_cl.m_cmd->m_buttons & IN_ATTACK;
	attack2 = g_cl.m_cmd->m_buttons & IN_ATTACK2;

	if (g_cl.m_weapon && g_cl.m_weapon_fire) {
		bool knife = g_cl.m_weapon_type == WEAPONTYPE_KNIFE && g_cl.m_weapon_id != ZEUS;
		bool revolver = g_cl.m_weapon_id == REVOLVER;

		// if we are in attack and can fire, do not anti-aim.
		if (attack || (attack2 && (knife || revolver)))
			return;
	}

	// disable conditions.
	if (g_csgo.m_gamerules->m_bFreezePeriod() || (g_cl.m_flags & FL_FROZEN) || (g_cl.m_cmd->m_buttons & IN_USE))
		return;

	// grenade throwing
	// CBaseCSGrenade::ItemPostFrame()
	// https://github.com/VSES/SourceEngine2007/blob/master/src_main/game/shared/cstrike/weapon_basecsgrenade.cpp#L209
	if (g_cl.m_weapon_type == WEAPONTYPE_GRENADE
		&& (!g_cl.m_weapon->m_bPinPulled() || attack || attack2)
		&& g_cl.m_weapon->m_fThrowTime() > 0.f && g_cl.m_weapon->m_fThrowTime() < g_csgo.m_globals->m_curtime)
		return;

	m_mode = AntiAimMode::STAND;

	if (g_cl.m_speed > 0.1f || g_input.GetKeyState(g_menu.main.antiaim.fake_flick.get()))
		m_mode = AntiAimMode::WALK;
	if (m_desync && (g_cl.m_cmd->m_forward_move == 0 && g_cl.m_cmd->m_side_move == 0)) {
		m_mode = (m_desync) ? AntiAimMode::STAND : m_mode;
	}
	if (g_input.GetKeyState(g_menu.main.misc.fakewalk.get()) && g_menu.main.misc.fakewalk_mode.get() == 1) {
		m_mode = AntiAimMode::STAND;
	}
	m_pitch = g_menu.main.antiaim.pitch.get();
	m_yaw = g_menu.main.antiaim.yaw.get();
	m_jitter_range = g_menu.main.antiaim.jitter_range.get();
	m_rot_range = g_menu.main.antiaim.rot_range.get();
	m_rot_speed = g_menu.main.antiaim.rot_speed.get();
	m_rand_update = g_menu.main.antiaim.rand_update.get();
	m_dir = g_menu.main.antiaim.dir_stand.get();
	m_dir_custom = g_menu.main.antiaim.dir_custom.get();

	// set pitch.
	AntiAimPitch();

	// if we have any yaw.
	if (m_yaw > 0) {
		// set direction.
		GetAntiAimDirection();
	}

	// we have no real, but we do have a fake.
	else if (g_menu.main.antiaim.fake_yaw.get() > 0)
		m_direction = g_cl.m_cmd->m_view_angles.y;

	if (g_input.GetKeyState(g_menu.main.misc.fakewalk.get()) && g_cl.m_local->GetGroundEntity() && g_menu.main.misc.fakewalk_mode.get() == 1) {
		g_cl.m_cmd->m_buttons &= ~(IN_FORWARD | IN_BACK | IN_MOVERIGHT | IN_MOVELEFT);
		bool attack, attack2;
		attack = g_cl.m_cmd->m_buttons & IN_ATTACK;
		attack2 = g_cl.m_cmd->m_buttons & IN_ATTACK2;
		if (g_cl.m_weapon && g_cl.m_weapon_fire) {
			bool knife = g_cl.m_weapon_type == WEAPONTYPE_KNIFE && g_cl.m_weapon_id != ZEUS;
			bool revolver = g_cl.m_weapon_id == REVOLVER;

			// if we are in attack and can fire, do not anti-aim.
			if (attack || (attack2 && (knife || revolver)))
				return;
		}
		// disable conditions.
		if (g_csgo.m_gamerules->m_bFreezePeriod() || (g_cl.m_flags & FL_FROZEN) || (g_cl.m_cmd->m_buttons & IN_USE))
			return;
		*g_cl.m_packet = !(g_cl.m_cmd->m_tick % 2 == 0);
		// Crazy Desync Walk Magic Not NIGGAR
		static int MoveCounter = 0;
		static int FlickCounter = 0;
		CUserCmd* cmd = g_cl.m_cmd;
		cmd->m_view_angles.y = m_view - 180;
		MoveCounter = MoveCounter + 1;
		cmd->m_side_move = MoveCounter < g_menu.main.misc.desync_ticks.get() ? cmd->m_side_move : 0.f;
		cmd->m_forward_move = MoveCounter < g_menu.main.misc.desync_ticks.get() ? cmd->m_forward_move : 0.f;
		if (MoveCounter >= g_menu.main.misc.desync_ticks.get() + 10) {
			ang_t angle;
			math::VectorAngles(g_cl.m_local->m_vecVelocity(), angle);
			angle.y = g_cl.m_view_angles.y - angle.y;
			vec3_t direction;
			math::AngleVectors(angle, &direction);
			vec3_t stop = direction * -1;
			g_cl.m_cmd->m_forward_move = stop.x;
			g_cl.m_cmd->m_side_move = stop.y;
		}
		if (MoveCounter >= 6 && MoveCounter <= 9 && !*g_cl.m_packet && !g_menu.main.misc.safe_desync.get()) {
			static bool LastFlip = false;
			g_cl.m_cmd->m_view_angles.y -= ((LastFlip != m_desync_invert || OffShit || (wtfwtfwtf && LastFlip == g_hvh.m_desync_invert)) ? 120 : 160) * (m_desync_invert ? -1 : 1);
			LastFlip = m_desync_invert;
			OffShit = false;
		}
		if (MoveCounter >= g_menu.main.misc.desync_ticks.get() + 20 && !*g_cl.m_packet) {
			MoveCounter = 0;
			if (std::abs(math::NormalizedAngle((g_cl.m_local->m_PlayerAnimState()->m_goal_feet_yaw - Fake_STATE->m_goal_feet_yaw))) < 20) {
				wtfwtfwtf = !wtfwtfwtf;
			}
			g_cl.m_cmd->m_view_angles.y += 110 * (m_desync_invert ? -1 : 1);
		}
		return;
	}
	is_flicking = false;
	if (m_desync) {
		if ((g_cl.m_cmd->m_forward_move == 0 && g_cl.m_cmd->m_side_move == 0)) {
			if (g_cl.m_local->m_vecVelocity().length_2d() > 35)
				return;
			*g_cl.m_packet = !(g_cl.m_cmd->m_tick % 2 == 0);
			CUserCmd* cmd = g_cl.m_cmd;
			static int tick_counter = 0;
			static bool flick_flip = false;
			static bool after_flick = false;
			if (cmd->m_tick % 2 == 0)
			{
				DoRealAntiAim();
				tick_counter++;
				if (tick_counter == 7 || tick_counter == 1)
				{
					flick_flip = !flick_flip;
					g_cl.m_cmd->m_view_angles.y += (flick_flip ? -120 : 110);
					g_cl.m_cmd->m_forward_move += (flick_flip ? 21 : -21);
					tick_counter = tick_counter == 7 ? 0 : tick_counter;
					after_flick = true;
					is_flicking = true;
					return;
				}
				if (after_flick)
				{
					after_flick = false;
					is_flicking = true;
					g_cl.m_cmd->m_view_angles.y -= 120;
				}
				return;
			}
			else
			{
				DoFakeAntiAim();
			}
		}
		if (m_mode == AntiAimMode::STAND)
			return;
	}
	//if (g_cl.isShifting)
	//	*g_cl.m_packet = true;
	if (g_menu.main.antiaim.fake_yaw.get() && true) { //!g_cl.isShifting) {
		// do not allow 2 consecutive sendpacket true if faking angles.
		if (*g_cl.m_packet && g_cl.m_old_packet)
			*g_cl.m_packet = false;

		// run the real on sendpacket false.
		if (!*g_cl.m_packet || !*g_cl.m_final_packet) {
			DoRealAntiAim();
			if (false) { //g_cl.m_charged) {
				bool earlypeek = false;
				for (int i = 1; i < g_csgo.m_globals->m_max_clients; ++i)
				{
					Player* pPlayerEntity = g_csgo.m_entlist->GetClientEntity< Player* >(i);
					AimPlayer* AimPlayer = &g_aimbot.m_players[i - 1];
					if (!pPlayerEntity
						|| !pPlayerEntity->alive()
						|| pPlayerEntity->dormant()
						|| pPlayerEntity == g_cl.m_local
						|| pPlayerEntity->m_iTeamNum() == g_cl.m_local->m_iTeamNum())
						continue;
					vec3_t OriginExtr = ((g_cl.m_local->GetShootPosition() + (g_cl.m_local->m_vecVelocity() * (game::TICKS_TO_TIME(14)))) + vec3_t(0, 0, 8));
					if (fabs(g_cl.m_local->m_vecVelocity().length_2d()) > .1f && (CanHitPlayer(OriginExtr, pPlayerEntity) && !CanHitPlayer(g_cl.m_local->GetShootPosition(), pPlayerEntity)))
					{
						earlypeek = true;
					}
				}
				if (earlypeek) {
					//g_cl.m_cmd->m_view_angles.y += 180;
					//g_cl.m_tick_to_shift = 16;
				}
			}
			//real_viewangles = g_cl.m_cmd->m_view_angles;
		}

		// run the fake on sendpacket true.
		else DoFakeAntiAim();
	}

	// no fake, just run real.
	else {
		DoRealAntiAim();
		//real_viewangles = g_cl.m_cmd->m_view_angles;
	}
}
//extern int recharge_clock;
//extern bool recharge_reset;
void HVH::SendPacket() {
	// if not the last packet this shit wont get sent anyway.
	// fix rest of hack by forcing to false.
	if (!*g_cl.m_final_packet)
		*g_cl.m_packet = false;

	// fake-lag enabled.
	if (g_menu.main.antiaim.lag_enable.get() && !g_csgo.m_gamerules->m_bFreezePeriod() && !(g_cl.m_flags & FL_FROZEN)) {
		// limit of lag.
		int limit = std::min((int)g_menu.main.antiaim.lag_limit.get(), g_cl.m_max_lag);
		if (g_tickshift.m_double_tap) { //g_aimbot.m_double_tap) {
			limit = 0;
		}
		// indicates wether to lag or not.
		bool active{ };

		// get current origin.
		vec3_t cur = g_cl.m_local->m_vecOrigin();

		// get prevoius origin.
		vec3_t prev = g_cl.m_net_pos.empty() ? g_cl.m_local->m_vecOrigin() : g_cl.m_net_pos.front().m_pos;

		// delta between the current origin and the last sent origin.
		float delta = (cur - prev).length_sqr();

		{
			auto activation = g_menu.main.antiaim.lag_active.GetActiveIndices();
			for (auto it = activation.begin(); it != activation.end(); it++) {

				// move.
				if (*it == 0 && delta > 0.1f && g_cl.m_speed > 0.1f) {
					active = true;
					break;
				}

				// air.
				else if (*it == 1 && (g_cl.m_buttons & IN_JUMP)) {
					active = true;
					break;
				}

				// crouch.
				else if (*it == 2 && g_cl.m_local->m_bDucking()) {
					active = true;
					break;
				}
			}
		}

		if (active) {
			int mode = g_menu.main.antiaim.lag_mode.get();

			// max.
			if (mode == 0)
				*g_cl.m_packet = false;

			// break.
			else if (mode == 1 && delta <= 4096.f)
				*g_cl.m_packet = false;

			// random.
			else if (mode == 2) {
				// compute new factor.
				if (g_cl.m_lag >= m_random_lag)
					m_random_lag = g_csgo.RandomInt(2, limit);

				// factor not met, keep choking.
				else *g_cl.m_packet = false;
			}

			// break step.
			else if (mode == 3) {
				// normal break.
				if (m_step_switch) {
					if (delta <= 4096.f)
						*g_cl.m_packet = false;
				}

				// max.
				else *g_cl.m_packet = false;
			}

			if (g_cl.m_lag >= limit)
				*g_cl.m_packet = true;
		}
	}

	vec3_t                start = g_cl.m_local->m_vecOrigin(), end = start, vel = g_cl.m_local->m_vecVelocity();
	CTraceFilterWorldOnly filter;
	CGameTrace            trace;

	// gravity.
	vel.z -= (g_csgo.sv_gravity->GetFloat() * g_csgo.m_globals->m_interval);
	// extrapolate.
	end += (vel * g_csgo.m_globals->m_interval);
	// move down.
	end.z -= 2.f;

	g_csgo.m_engine_trace->TraceRay(Ray(start, end), MASK_SOLID, &filter, &trace);
	// check if landed.
	if (trace.m_fraction != 1.f && trace.m_plane.m_normal.z > 0.7f && !(g_cl.m_flags & FL_ONGROUND))
		*g_cl.m_packet = !g_menu.main.antiaim.lag_land.get();

	// force fake-lag to 14 when fakelagging.
	if (g_input.GetKeyState(g_menu.main.misc.fakewalk.get())) {
		if (g_menu.main.misc.fakewalk_mode.get() == 0) {
			*g_cl.m_packet = false;
		}
	}

	if (g_input.GetKeyState(g_menu.main.antiaim.lag_exploit.get())) {
		*g_cl.m_packet = false;
	}

	static bool on_fakewalk = false;

	/*
	if (g_input.GetKeyState(g_menu.main.misc.fakewalk.get())) {
		on_fakewalk = true;
		if (g_cl.m_charged && g_menu.main.misc.fakewalk_mode.get() == 0) {
			*g_cl.m_packet = true;
			g_cl.m_tick_to_shift_wtf = g_cl.m_visual_shift_ticks;
			g_cl.m_visual_shift_ticks = 0;
			g_cl.m_charged = false;
			return;
		}
	}
	else if (on_fakewalk) {
		recharge_reset = true;
		recharge_clock = 999;
		on_fakewalk = false;
	}*/
	// we somehow reached the maximum amount of lag.
	// we cannot lag anymore and we also cannot shoot anymore since we cant silent aim.
	if (g_cl.m_lag >= g_cl.m_max_lag) {
		// set bSendPacket to true.
		*g_cl.m_packet = true;

		// disable firing, since we cannot choke the last packet.
		g_cl.m_weapon_fire = false;
	}
}