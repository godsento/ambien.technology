#include "includes.h"

Resolver g_resolver{};;

LagRecord* Resolver::FindIdealRecord(AimPlayer* data) {
	LagRecord* first_valid, * current;

	if (data->m_records.empty())
		return nullptr;

	first_valid = nullptr;

	// iterate records.
	for (const auto& it : data->m_records) {
		if (it->dormant() || it->immune() || !it->valid())
			continue;

		// get current record.
		current = it.get();

		// first record that was valid, store it for later.
		if (!first_valid)
			first_valid = current;

		// try to find a record with a shot, lby update, walking or no anti-aim.
		if (it->m_shot || it->m_mode == Modes::RESOLVE_BODY || it->m_mode == Modes::RESOLVE_WALK || it->m_mode == Modes::RESOLVE_NONE)
			return current;
	}

	// none found above, return the first valid record if possible.
	return (first_valid) ? first_valid : nullptr;
}

LagRecord* Resolver::FindLastRecord(AimPlayer* data) {
	LagRecord* current;

	if (data->m_records.empty())
		return nullptr;


	return data->m_records.back().get();
}

void Resolver::PredictBodyUpdates(Player* player, LagRecord* record) {

	if (!g_cl.m_processing)
		return;

	AimPlayer* data = &g_aimbot.m_players[player->index() - 1];

	if (!data)
		return;

	if (data->m_records.empty())
		return;

	// inform esp that we're about to be the prediction process
	data->m_predict = true;

	//check if the player is walking
	// no need for the extra fakewalk check since we null velocity when they're fakewalking anyway
	if (record->m_anim_velocity.length_2d() > 0.1f && !record->m_fake_walk) {
		// predict the first flick they have to do after they stop moving
		data->m_body_update = player->m_flSimulationTime() + 0.22f;

		// since we are still not in the prediction process, inform the cheat that we arent predicting yet
		// this is only really used to determine if we should draw the lby timer bar on esp, no other real purpose
		data->m_predict = false;

		// stop here
		return;
	}

	if (data->m_body_index > 0)
		return;

	if (data->m_records.size() >= 2) {
		LagRecord* previous = data->m_records[1].get();

		if (previous && previous->valid() && !record->m_retard && !previous->m_retard) {
			// lby updated on this tick
			if (previous->m_body != player->m_flLowerBodyYawTarget() || player->m_flSimulationTime() >= data->m_body_update) {

				// for backtrack
				record->m_resolved = true;

				// inform the cheat of the resolver method
				record->m_mode = RESOLVE_BODY;

				// predict the next body update
				data->m_body_update = player->m_flSimulationTime() + 1.1f;

				// set eyeangles to lby
				record->m_eye_angles.y = record->m_body;
				math::NormalizeAngle(record->m_eye_angles.y);

				player->m_angEyeAngles().y = record->m_eye_angles.y;
			}
		}
	}
}

void Resolver::OnBodyUpdate(Player* player, float value) {
	AimPlayer* data = &g_aimbot.m_players[player->index() - 1];

	// set data.
	data->m_old_body = data->m_body;
	data->m_body = value;
	data->m_update_time = g_csgo.m_globals->m_curtime;
}

float Resolver::GetAwayAngle(LagRecord* record) {
	float  delta{ std::numeric_limits< float >::max() };
	vec3_t pos;
	ang_t  away;

	// other cheats predict you by their own latency.
	// they do this because, then they can put their away angle to exactly
	// where you are on the server at that moment in time.

	// the idea is that you would need to know where they 'saw' you when they created their user-command.
	// lets say you move on your client right now, this would take half of our latency to arrive at the server.
	// the delay between the server and the target client is compensated by themselves already, that is fortunate for us.

	// we have no historical origins.
	// no choice but to use the most recent one.
	//if( g_cl.m_net_pos.empty( ) ) {
	math::VectorAngles(g_cl.m_local->m_vecOrigin() - record->m_pred_origin, away);
	return away.y;
	//}

	// half of our rtt.
	// also known as the one-way delay.
	//float owd = ( g_cl.m_latency / 2.f );

	// since our origins are computed here on the client
	// we have to compensate for the delay between our client and the server
	// therefore the OWD should be subtracted from the target time.
	//float target = record->m_pred_time; //- owd;

	// iterate all.
	//for( const auto &net : g_cl.m_net_pos ) {
		// get the delta between this records time context
		// and the target time.
	//	float dt = std::abs( target - net.m_time );

		// the best origin.
	//	if( dt < delta ) {
	//		delta = dt;
	//		pos   = net.m_pos;
	//	}
	//}

	//math::VectorAngles( pos - record->m_pred_origin, away );
	//return away.y;
}

void Resolver::MatchShot(AimPlayer* data, LagRecord* record) {
	// do not attempt to do this in nospread mode.
	if (g_menu.main.config.mode.get() == 1)
		return;

	float shoot_time = -1.f;

	Weapon* weapon = data->m_player->GetActiveWeapon();

	if (!weapon)
		return;

	// this record has a shot on it.
	// nigger from supremacy, lastshottime is properly networked
	// fuck yall
	if (data->m_shot_time != weapon->m_fLastShotTime()) {
		if (record->m_lag <= 1)
			record->m_shot = true;

		// more then 1 choke, cant hit pitch, apply prev pitch.
		else if (data->m_records.size() >= 2) {
			LagRecord* previous = data->m_records[1].get();

			if (previous && !previous->dormant())
				record->m_eye_angles.x = previous->m_eye_angles.x;
		}
	}
}

void Resolver::SetMode(LagRecord* record) {
	// the resolver has 3 modes to chose from.
	// these modes will vary more under the hood depending on what data we have about the player
	// and what kind of hack vs. hack we are playing (mm/nospread).

	float speed = record->m_anim_velocity.length();
	AimPlayer* data = &g_aimbot.m_players[record->m_player->index() - 1];
	
	bool retard = false;
	if (data->m_records.size() >= 2) {
		LagRecord* previous = data->m_records[1].get();

		if (previous && !previous->dormant())
			retard = previous->m_retard;
	}
	// @doc: on-ground, running and not fake-walking.
	if ((record->m_flags & FL_ONGROUND) && speed > 0.1f && !record->m_fake_walk && !record->m_retard && !retard)
		record->m_mode = Modes::RESOLVE_WALK;

	// @doc: on-ground, and ( idle or fake-walking ).
	if ((record->m_flags & FL_ONGROUND) && (speed <= 0.1f || record->m_fake_walk || record->m_retard || retard))
		record->m_mode = Modes::RESOLVE_STAND;

	// @doc: in-air.
	else if (!(record->m_flags & FL_ONGROUND))
		record->m_mode = Modes::RESOLVE_AIR;
}

void Resolver::ResolveAngles(Player* player, LagRecord* record) {
	AimPlayer* data = &g_aimbot.m_players[player->index() - 1];

	// mark this record if it contains a shot.
	MatchShot(data, record);

	// next up mark this record with a resolver mode that will be used.
	SetMode(record);

	// we arrived here we can do the acutal resolve.
	if (record->m_mode == Modes::RESOLVE_WALK)
		ResolveWalk(data, record);

	else if (record->m_mode == Modes::RESOLVE_STAND)
		ResolveStand(data, record);

	else if (record->m_mode == Modes::RESOLVE_AIR)
		ResolveAir(data, record);

	// normalize the eye angles, doesn't really matter but its clean.
	math::NormalizeAngle(record->m_eye_angles.y);
}

void Resolver::ResolveWalk(AimPlayer* data, LagRecord* record) {

	// apply lby to eyeangles.
	record->m_eye_angles.y = record->m_body;

	// reset stand and body index.		

	if (record->m_anim_velocity.length() > 20.f) { // niggers
		data->m_lm_index = 0;
		data->m_freestand_index = 0;
		data->m_ff_index = 0;

		if (record->m_anim_velocity.length() > 80.f || record->m_lag > 4) {
			data->m_move_index = 0;
			data->m_body_index = 0;
		}
	}

	// copy the last record that this player was walking
	// we need it later on because it gives us crucial data.
	std::memcpy(&data->m_walk_record, record, sizeof(LagRecord));
}


void Resolver::anti_freestand(AimPlayer* data, LagRecord* record) {
	float away = GetAwayAngle(record);

	CGameTrace tr_right;
	CGameTrace tr_left;
	CTraceFilterSimple filter{ };
	filter.SetPassEntity(g_cl.m_local);

	ang_t angDirectionAngle;
	math::VectorAngles(g_cl.m_shoot_pos - record->m_pred_origin, angDirectionAngle);

	angDirectionAngle.y += 180.f;

	vec3_t forward, right, up;
	math::AngleVectors(angDirectionAngle, &forward, &right, &up);

	auto vecStart = g_cl.m_shoot_pos;;
	auto vecEnd = vecStart + forward * (g_cl.m_shoot_pos.dist_to(record->m_pred_origin) * 1.25f);

	vecEnd.z = data->m_player->GetShootPosition().z;


	g_csgo.m_engine_trace->TraceRay(Ray(g_cl.m_shoot_pos, vecEnd + right * 17.f), CONTENTS_SOLID, &filter, &tr_right);

	//	g_csgo.m_debug_overlay->AddLineOverlay(g_cl.m_shoot_pos, vecEnd + right * 16.f, 255, 0, 0, false, 1); // base line
	//	g_csgo.m_debug_overlay->AddLineOverlay(g_cl.m_shoot_pos, tr_right.m_endpos, 255, 255, 0, false, 1); // where it hits

	g_csgo.m_engine_trace->TraceRay(Ray(g_cl.m_shoot_pos, vecEnd - right * 17.f), CONTENTS_SOLID, &filter, &tr_left);

	//	g_csgo.m_debug_overlay->AddLineOverlay(g_cl.m_shoot_pos, vecEnd - right * 16.f, 255, 0, 0, false, 1); // base line
	// g_csgo.m_debug_overlay->AddLineOverlay(g_cl.m_shoot_pos, tr_left.m_endpos, 255, 255, 0, false, 1); // where it hits

	if ((tr_right.m_fraction > 0.97f && tr_left.m_fraction > 0.97f) || (tr_left.m_fraction == tr_right.m_fraction) || std::abs(tr_left.m_fraction - tr_right.m_fraction) <= 0.1f)
		record->m_eye_angles.y = away + 180.f;
	else
		record->m_eye_angles.y = (tr_left.m_fraction < tr_right.m_fraction) ? away - 90.f : away + 90.f;
}

void Resolver::ResolveStand(AimPlayer* data, LagRecord* record) {

	switch (data->m_freestand_index % 3) {
	case 0:
		anti_freestand(data, record);
		break;
	case 1:
		record->m_eye_angles.y = record->m_body + 90.f;
		break;
	case 2:
		record->m_eye_angles.y = record->m_body - 90.f;
		break;
	}

}


void Resolver::ResolveAir(AimPlayer* data, LagRecord* record) {
	
	// try to predict the direction of the player based on his velocity direction.
	// this should be a rough estimation of where he is looking.
	float velyaw = math::rad_to_deg(std::atan2(record->m_velocity.y, record->m_velocity.x));

	switch (data->m_shots % 4) {
	case 0:
		record->m_eye_angles.y = velyaw + 180.f;
		break;

	case 1:
		record->m_eye_angles.y = GetAwayAngle(record) + 180.f;
		break;

	case 2:
		record->m_eye_angles.y = velyaw - 90.f;
		break;

	case 3:
		record->m_eye_angles.y = velyaw + 90.f;
		break;
	}
}

// old res


	/*
	if (data->comp && data->m_ff_index == 0) {
		record->m_mode = Modes::RESOLVE_FAKEFLICK;
		// this is gonna be so retarded
		float away = GetAwayAngle(record);
		if (record->m_anim_velocity.length_2d() > 0.1f) {
			// micromoving :scream:
			float delta = math::NormalizedAngle(record->m_body - away);
			float deltaabs = std::abs(delta);
			if (deltaabs > 65 && deltaabs < 110) {
				// we have the side flick?
				if (delta < 0) {
					// possibly right side
					data->real_side = 1;
				}
				else {
					// possibly left side
					data->real_side = 2;
				}
			}
		}
		if (data->real_side != 0) {
			// HURAYY WE HAVE A SIDE LETS GO
			record->m_eye_angles.y = away - (data->real_side == 1 ? 90 : -90);
			return; // fuck the og resolver lets headshot this nigga
		}
	}
	else { data->real_side = 0; }*/

	/*
	// for no-spread call a seperate resolver.
	if (g_menu.main.config.mode.get() == 1) {
		StandNS(data, record);
		return;
	}*/


	/*
	// set our resolver mode
	record->m_mode = Modes::RESOLVE_FREESTAND;
	switch (data->m_freestand_index%2) {
		case 0: {
			// try and anti freestand?
			if (FindBestAngle(record, data)) {
				// great. we have freestand data.
				// function in the if statement already set angles so lets not do anything rn
				break;
			}
			else {
				// we have no freestand data. fuck.
				handle_no_freestand(record, data);
			}
			break;
		}
		case 1: {
			// ok we missed. we either just missed anti freestand (probably feet?)
			// lets just hit this anyways.
			handle_no_freestand(record, data);
			break;
		}
		default: {
			break;
		}
	}*/

/*
bool has_979(AimPlayer* data)
{
	C_AnimationLayer layers[15];
	data->m_player->GetAnimLayers(layers);
	for (int i = 0; i < 15; i++)
	{
		const int activity = data->m_player->GetSequenceActivity(layers[i].m_sequence);
		if (activity == 979)
		{
			return true;
		}
	}
	return false;
}

void handle_lby_breaker(LagRecord* record, AimPlayer* data) {

}
void handle_sideways(LagRecord* record, AimPlayer* data) {

	switch (data->m_freestand_index % 2) {
	case 0:
	{
		record->m_eye_angles.y = record->m_body - 119;
		// 2nd shot
		break;
	}
	case 1:
	{
		record->m_eye_angles.y = record->m_body + 119;
		// third shot
		break;
	}
	default:
	{
		// nigga how
		break;
	}
	}
}
void handle_no_freestand(LagRecord* record, AimPlayer* data) {

	float away = g_resolver.GetAwayAngle(record);
	bool adjust = has_979(data);
	if (std::abs(math::AngleDiff(away, record->m_body)) < 60) { // todo: make this better.
		// we have 2 possibilites, they are backwards with no lby breaker (probably not, but ive seen some players do this). or they are sideways.
		// lets try sideways first
		if (!adjust) {
			// ok, if they are sideways its only 119
			// lets try left & right ok?
			handle_sideways(record,data);

		}
		else {
			// they are def lby breaking. dont use this angle
			// lets try some shit ok?
			// we know that theyre lby is close to the back angle, this could mean one of 3 things
			// 1. they are forwards breaking backwards (odd but could be possible)
			// 2. they are they are distortioning.
			// 3. they are fail breaking.
			// lets take the distortion approach
			record->m_eye_angles.y = record->m_body + std::sin(data->m_player->m_flSimulationTime() * 4) * 90; // p boss code
		}
	}
	else {
		// we literally have no clue what theyre doing :sob:
		// lets try atleast
		if (!adjust) {
			// they could be sideways?
			handle_sideways(record, data);
		}
		else {
			// they are probably doing 1 of 2 things.
			// 1. distortion.
			// 2. 180 break (LOL)
			// lets try 180 first
			switch (data->m_freestand_index % 2) {
				case 0: {
					record->m_eye_angles.y = record->m_body + 160; // lets maybe not try 180, ill have to test if this is better than + 180;
					break;
				}
				case 1: {
					record->m_eye_angles.y = record->m_body + std::sin(data->m_player->m_flSimulationTime() * 5) * 180; // p boss code yet again.
					break;
				}
				default: {
					// literally breaking math......
					break;
				}
			}
		}
	}
}*/