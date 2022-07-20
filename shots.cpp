#include "includes.h"
#include <urlmon.h>
#include <mmsystem.h>

#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "Winmm.lib")
#pragma comment( lib, "Iphlpapi.lib" )

Shots g_shots{ };

void Shots::OnFrameStage()
{
	if (!g_cl.m_processing || m_shots.empty()) {
		if (!m_shots.empty())
			m_shots.clear();

		return;
	}

	for (auto it = m_shots.begin(); it != m_shots.end();) {
		if (it->m_time + 1.f < g_csgo.m_globals->m_curtime)
			it = m_shots.erase(it);
		else
			it = next(it);
	}

	for (auto it = m_shots.begin(); it != m_shots.end();) {
		if (it->m_matched && !it->m_hurt && it->m_confirmed) {
			// not in nospread mode, see if the shot missed due to spread.
			Player* target = it->m_target;
			if (!target) {
				it = m_shots.erase(it);
				continue;
			}

			// not gonna bother anymore.
			if (!target->alive()) {
				it = m_shots.erase(it);
				continue;
			}

			AimPlayer* data = &g_aimbot.m_players[target->index() - 1];
			if (!data) {
				it = m_shots.erase(it);
				continue;
			}

			// this record was deleted already.
			if (!it->m_record->m_bones) {
				g_notify.add(XOR("shot missed due to invalid target\n"));
				it = m_shots.erase(it);
				continue;
			}

			// write historical matrix of the time that we shot
			// into the games bone cache, so we can trace against it.
			//it->m_record->cache( );

			// start position of trace is where we took the shot.
			vec3_t start = it->m_pos;

			// the impact pos contains the spread from the server
			// which is generated with the server seed, so this is where the bullet
			// actually went, compute the direction of this from where the shot landed
			// and from where we actually took the shot.
			vec3_t dir = (it->m_server_pos - start).normalized();

			// get end pos by extending direction forward.
			// todo; to do this properly should save the weapon range at the moment of the shot, cba..
			vec3_t end = start + dir * start.dist_to(it->m_record->m_pred_origin) * 6.6f;

			const vec3_t backup_origin = data->m_player->m_vecOrigin();
			const vec3_t backup_abs_origin = data->m_player->GetAbsOrigin();
			const ang_t backup_abs_angles = data->m_player->GetAbsAngles();
			const vec3_t backup_obb_mins = data->m_player->m_vecMins();
			const vec3_t backup_obb_maxs = data->m_player->m_vecMaxs();
			const auto backup_cache = data->m_player->m_BoneCache2();

			auto restore = [&]() -> void {
				data->m_player->m_vecOrigin() = backup_origin;
				data->m_player->SetAbsOrigin(backup_abs_origin);
				data->m_player->SetAbsAngles(backup_abs_angles);
				data->m_player->m_vecMins() = backup_obb_mins;
				data->m_player->m_vecMaxs() = backup_obb_maxs;
				data->m_player->m_BoneCache2() = backup_cache;
			};


			data->m_player->m_vecOrigin() = it->m_record->m_pred_origin;
			data->m_player->SetAbsOrigin(it->m_record->m_pred_origin);
			data->m_player->SetAbsAngles(it->m_record->m_abs_ang);
			data->m_player->m_vecMins() = it->m_record->m_mins;
			data->m_player->m_vecMaxs() = it->m_record->m_maxs;
			data->m_player->m_BoneCache2() = reinterpret_cast<matrix3x4_t**>(it->m_record->m_bones);

			if (!g_aimbot.CanHit(start, end, it->m_record, it->m_hitbox, true, it->m_record->m_bones)) {
				g_notify.add(XOR("missed shot due to spread\n"));

				restore();
				it = m_shots.erase(it);
				continue;
			}

			restore();

			size_t mode = it->m_record->m_mode;

			player_info_t info;
			if (!g_csgo.m_engine->GetPlayerInfo(it->m_record->m_player->index(), &info))
				return;

			// if we miss a shot on body update.
			// we can chose to stop shooting at them.
			if (mode == Resolver::Modes::RESOLVE_BODY) {
				data->m_body_index++;
				g_cl.print(tfm::format(XOR("miss registered | target: %s | backtrack: %i | rmode: %s | index: %i\n"), info.m_name, game::TIME_TO_TICKS(it->m_record->m_sim_time - it->m_record->m_old_sim_time), "update", data->m_body_index));
			}

			else if (mode == Resolver::Modes::RESOLVE_LM)
			{
				data->m_lm_index++;
				g_cl.print(tfm::format(XOR("miss registered | target: %s | backtrack: %i | rmode: %s | index: %i\n"), info.m_name, game::TIME_TO_TICKS(it->m_record->m_sim_time - it->m_record->m_old_sim_time), "lastmove", data->m_lm_index));
			}

			else if (mode == Resolver::Modes::RESOLVE_STAND || mode == Resolver::Modes::RESOLVE_FREESTAND) {
				data->m_freestand_index++;
				g_cl.print(tfm::format(XOR("miss registered | target: %s | backtrack: %i | rmode: %s | index: %i\n"), info.m_name, game::TIME_TO_TICKS(it->m_record->m_sim_time - it->m_record->m_old_sim_time), "standing", data->m_freestand_index));
			}

			else if (mode == Resolver::Modes::RESOLVE_WALK)
			{
				if (it->m_record->m_anim_velocity.length() < 40.f)
					data->m_move_index++;

				g_cl.print(tfm::format(XOR("miss registered | target: %s | backtrack: %i | rmode: %s\n"), info.m_name, game::TIME_TO_TICKS(it->m_record->m_sim_time - it->m_record->m_old_sim_time), "moving"));
			}
			else if (mode == Resolver::Modes::RESOLVE_AIR)
			{
				g_cl.print(tfm::format(XOR("miss registered | target: %s | backtrack: %i | rmode: %s\n"), info.m_name, game::TIME_TO_TICKS(it->m_record->m_sim_time - it->m_record->m_old_sim_time), "air"));
			}
			else if (mode == Resolver::Modes::RESOLVE_FAKEFLICK)
			{
				data->m_ff_index++;
				g_cl.print(tfm::format(XOR("miss registered | target: %s | backtrack: %i | rmode: %s | index: %i\n"), info.m_name, game::TIME_TO_TICKS(it->m_record->m_sim_time - it->m_record->m_old_sim_time), "fakeflick", data->m_ff_index));
			}


			// let's not increment this if this is a shot record.
			++data->m_missed_shots;
			g_notify.add(XOR("missed shot due to resolver\n"));

			// we processed this shot, let's delete it.
			it = m_shots.erase(it);
		}
		else {
			it = next(it);
		}
	}
}

void Shots::OnShotFire(Player* target, float damage, int bullets, int hitbox, LagRecord* record) {

	// iterate all bullets in this shot.
	for (int i{ }; i < bullets; ++i) {
		// setup new shot data.
		ShotRecord shot;
		shot.m_target = target;
		shot.m_record = record;
		shot.m_hitbox = hitbox;
		shot.m_time = game::TICKS_TO_TIME(g_cl.m_local->m_nTickBase());
		shot.m_lat = g_cl.m_latency;
		shot.m_damage = damage;
		shot.m_matched = false;
		shot.m_hurt = false;
		shot.m_confirmed = false;
		shot.m_pos = g_cl.m_local->GetShootPosition();

		// we are not shooting manually.
		// and this is the first bullet, only do this once.
		if (target && i == 0) {
			// increment total shots on this player.
			AimPlayer* data = &g_aimbot.m_players[target->index() - 1];
			if (data)
				++data->m_shots;

			auto matrix = record->m_bones;

			if (matrix)
				shot.m_matrix = matrix;

		}

		// add to tracks.
		m_shots.push_front(shot);
	}

	// no need to keep an insane amount of shots.
	while (m_shots.size() > 128)
		m_shots.pop_back();
}

void Shots::OnImpact(IGameEvent* evt) {
	int        attacker;
	vec3_t     pos, dir, start, end;
	float      time;
	CGameTrace trace;

	// screw this.
	if (!evt || !g_cl.m_local)
		return;

	// get attacker, if its not us, screw it.
	attacker = g_csgo.m_engine->GetPlayerForUserID(evt->m_keys->FindKey(HASH("userid"))->GetInt());


	// decode impact coordinates and convert to vec3.
	pos = {
		evt->m_keys->FindKey(HASH("x"))->GetFloat(),
		evt->m_keys->FindKey(HASH("y"))->GetFloat(),
		evt->m_keys->FindKey(HASH("z"))->GetFloat()
	};

	// get prediction time at this point.
	time = game::TICKS_TO_TIME(g_cl.m_local->m_nTickBase());

	// add to visual impacts if we have features that rely on it enabled.
	// todo - dex; need to match shots for this to have proper GetShootPosition, don't really care to do it anymore.
	Player* ent = g_csgo.m_entlist->GetClientEntity<Player*>(attacker);
	if (attacker != g_csgo.m_engine->GetLocalPlayer() && ent->enemy(g_cl.m_local)) {
		if (g_menu.main.visuals.impact_beams.get() && !ent->dormant()) {
			m_vis_impacts.push_back({ pos, ent->GetShootPosition(), g_cl.m_local->m_nTickBase() });
			return;
		}
	}
	if (g_menu.main.visuals.impact_beams.get() && attacker == g_csgo.m_engine->GetLocalPlayer())
		m_vis_impacts.push_back({ pos, g_cl.m_local->GetShootPosition(), g_cl.m_local->m_nTickBase() });

	// we did not take a shot yet.
	if (m_shots.empty())
		return;

	struct ShotMatch_t { float delta; ShotRecord* shot; };
	ShotMatch_t match;
	match.delta = std::numeric_limits< float >::max();
	match.shot = nullptr;

	// iterate all shots.
	for (auto& s : m_shots) {

		// this shot was already matched
		// with a 'bullet_impact' event.
		if (s.m_matched)
			continue;

		// add the latency to the time when we shot.
		// to predict when we would receive this event.
		float predicted = s.m_time + s.m_lat;

		// get the delta between the current time
		// and the predicted arrival time of the shot.
		float delta = std::abs(time - predicted);

		// fuck this.
		if (delta > 1.f)
			continue;

		// store this shot as being the best for now.
		if (delta < match.delta) {
			match.delta = delta;
			match.shot = &s;
		}
	}

	// no valid shotrecord was found.
	ShotRecord* shot = match.shot;
	if (!shot)
		return;

	// this shot was matched.
	shot->m_matched = true;
	shot->m_server_pos = pos;

	// create new impact instance that we can match with a player hurt.
	ImpactRecord impact;
	impact.m_shot = shot;
	impact.m_tick = g_cl.m_local->m_nTickBase();
	impact.m_pos = pos;

	// add to track.
	m_impacts.push_front(impact);

	// no need to keep an insane amount of impacts.
	while (m_impacts.size() > 16)
		m_impacts.pop_back();
}

void Shots::OnHurt(IGameEvent* evt) {
	int         attacker, victim, group, hp;
	float       damage;
	std::string name;

	if (!evt || !g_cl.m_local)
		return;

	attacker = g_csgo.m_engine->GetPlayerForUserID(evt->m_keys->FindKey(HASH("attacker"))->GetInt());
	victim = g_csgo.m_engine->GetPlayerForUserID(evt->m_keys->FindKey(HASH("userid"))->GetInt());

	// skip invalid player indexes.
	// should never happen? world entity could be attacker, or a nade that hits you.
	if (attacker < 1 || attacker > 64 || victim < 1 || victim > 64)
		return;

	// we were not the attacker or we hurt ourselves.
	else if (attacker != g_csgo.m_engine->GetLocalPlayer() || victim == g_csgo.m_engine->GetLocalPlayer())
		return;

	// get hitgroup.
	// players that get naded ( DMG_BLAST ) or stabbed seem to be put as HITGROUP_GENERIC.
	group = evt->m_keys->FindKey(HASH("hitgroup"))->GetInt();

	// invalid hitgroups ( note - dex; HITGROUP_GEAR isn't really invalid, seems to be set for hands and stuff? ).
	if (group == HITGROUP_GEAR)
		return;

	// get the player that was hurt.
	Player* target = g_csgo.m_entlist->GetClientEntity< Player* >(victim);
	if (!target)
		return;

	// get player info.
	player_info_t info;
	if (!g_csgo.m_engine->GetPlayerInfo(victim, &info))
		return;

	// get player name;
	name = std::string(info.m_name).substr(0, 24);

	// get damage reported by the server.
	damage = (float)evt->m_keys->FindKey(HASH("dmg_health"))->GetInt();

	// get remaining hp.
	hp = evt->m_keys->FindKey(HASH("health"))->GetInt();

	// setup headshot marker
	if (group == HITGROUP_HEAD)
		iHeadshot = true;
	else
		iHeadshot = false;

	// hitmarker.
	if (g_menu.main.misc.hitmarker.get()) {
		g_visuals.m_hit_duration = 2.f;
		g_visuals.m_hit_start = g_csgo.m_globals->m_curtime;
		g_visuals.m_hit_end = g_visuals.m_hit_start + g_visuals.m_hit_duration;

		// bind to draw
		iHitDmg = damage;

		// get interpolated origin.
		iPlayerOrigin = target->GetAbsOrigin();

		// get hitbox bounds.
		// hehe boy
		target->ComputeHitboxSurroundingBox(&iPlayermins, &iPlayermaxs);

		// correct x and y coordinates.
		iPlayermins = { iPlayerOrigin.x, iPlayerOrigin.y, iPlayermins.z };
		iPlayermaxs = { iPlayerOrigin.x, iPlayerOrigin.y, iPlayermaxs.z + 8.f };

		g_csgo.m_sound->EmitAmbientSound(XOR("buttons/arena_switch_press_02.wav"), 1.f);

	}

	// print this shit.
	if (g_menu.main.misc.notifications.get(1)) {
		std::string out = tfm::format(XOR("hit %s in the %s for %i damage (%i health remaining)\n"), name, m_groups[group], (int)damage, hp);
		g_notify.add(out);
	}



	// if we hit a player, mark vis impacts.
	if (!m_vis_impacts.empty()) {
		for (auto& i : m_vis_impacts) {
			if (i.m_tickbase == g_csgo.m_globals->m_tick_count)
				i.m_hit_player = true;
		}
	}

	struct ShotMatch_t { float delta; ShotRecord* shot; };
	ShotMatch_t match;
	match.delta = std::numeric_limits< float >::max();
	match.shot = nullptr;

	// iterate all shots.
	for (auto& s : m_shots) {

		// this shot was already matched
		// with a 'player_hurt' event.
		if (s.m_hurt)
			continue;

		// add the latency to the time when we shot.
		// to predict when we would receive this event.
		float predicted = s.m_time + s.m_lat;

		// get the delta between the current time
		// and the predicted arrival time of the shot.
		float delta = std::abs(g_csgo.m_globals->m_curtime - predicted);

		// fuck this.
		if (delta > 1.f)
			continue;

		// store this shot as being the best for now.
		if (delta < match.delta) {
			match.delta = delta;
			match.shot = &s;
		}

		player_info_t info;
		if (!g_csgo.m_engine->GetPlayerInfo(match.shot->m_record->m_player->index(), &info))
			return;

		std::string resolver_text;

		switch (match.shot->m_record->m_mode)
		{
		case g_resolver.RESOLVE_LM:
			resolver_text = "lastmove";
			break;
		case g_resolver.RESOLVE_STAND:
			resolver_text = "stand";
			break;
		case g_resolver.RESOLVE_BODY:
			resolver_text = "update";
			break;
		case g_resolver.RESOLVE_WALK:
			resolver_text = "moving";
			break;
		case g_resolver.RESOLVE_AIR:
			resolver_text = "air";
			break;
		default:
			resolver_text = "none";
		}

		g_cl.print(tfm::format(XOR("hit registered | target: %s | backtrack: %i | rmode: %s\n"), info.m_name, game::TIME_TO_TICKS(match.shot->m_record->m_sim_time - match.shot->m_record->m_old_sim_time), resolver_text));
	}

	// no valid shotrecord was found.
	ShotRecord* shot = match.shot;
	if (!shot)
		return;

	shot->m_hurt = true;


	// no impacts to match.
	if (m_impacts.empty())
		return;

	ImpactRecord* impact{ nullptr };



	// iterate stored impacts.
	for (auto& i : m_impacts) {

		// this impact doesnt match with our current hit.
		if (i.m_tick != g_cl.m_local->m_nTickBase())
			continue;

		// wrong player.
		if (i.m_shot->m_target != target)
			continue;

		// shit fond.
		impact = &i;
		break;
	}

	// no impact matched.
	if (!impact)
		return;

	// setup new data for hit track and push to hit track.
	HitRecord hit;
	hit.m_impact = impact;
	hit.m_group = group;
	hit.m_damage = damage;

	m_hits.push_front(hit);

	while (m_hits.size() > 16)
		m_hits.pop_back();
}

void Shots::OnFire(IGameEvent* evt)
{
	int attacker;

	// screw this.
	if (!evt || !g_cl.m_local)
		return;


	// get attacker, if its not us, screw it.
	attacker = g_csgo.m_engine->GetPlayerForUserID(evt->m_keys->FindKey(HASH("userid"))->GetInt());
	if (attacker != g_csgo.m_engine->GetLocalPlayer())
		return;

	struct ShotMatch_t { float delta; ShotRecord* shot; };
	ShotMatch_t match;
	match.delta = std::numeric_limits< float >::max();
	match.shot = nullptr;

	// iterate all shots.
	for (auto& s : m_shots) {


		// this shot was already matched
		// with a 'weapon_fire' event.
		if (s.m_confirmed)
			continue;

		// add the latency to the time when we shot.
		// to predict when we would receive this event.
		float predicted = s.m_time + s.m_lat;

		// get the delta between the current time
		// and the predicted arrival time of the shot.
		float delta = std::abs(g_csgo.m_globals->m_curtime - predicted);

		// fuck this.
		if (delta > 1.f)
			continue;

		// store this shot as being the best for now.
		if (delta < match.delta) {
			match.delta = delta;
			match.shot = &s;
		}
	}

	// no valid shotrecord was found.
	ShotRecord* shot = match.shot;
	if (!shot)
		return;

	shot->m_confirmed = true;
}