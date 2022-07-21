#include "includes.h"

Visuals g_visuals{ };/*
void Visuals::UpdateSkyBox() {
	std::vector< IMaterial* > world, props;

	// iterate material handles.
	for (uint16_t h{ g_csgo.m_material_system->FirstMaterial() }; h != g_csgo.m_material_system->InvalidMaterial(); h = g_csgo.m_material_system->NextMaterial(h)) {
		// get material from handle.
		IMaterial* mat = g_csgo.m_material_system->GetMaterial(h);
		if (!mat)
			continue;

		// store world materials.
		if (FNV1a::get(mat->GetTextureGroupName()) == HASH("World textures"))
			world.push_back(mat);

		// store props.
		else if (FNV1a::get(mat->GetTextureGroupName()) == HASH("StaticProp textures"))
			props.push_back(mat);
	}
	for (const auto& w : world)
		w->ColorModulate(1.f, 1.f, 1.f);
}*/
void Visuals::ModulateWorld() {
	float night_amt = g_menu.main.visuals.night_amt.get() / 100;
	std::vector< IMaterial* > world, props;
	IMaterial* Material = g_csgo.m_material_system->FindMaterial(XOR("lights/white"), "Model textures");
	if (Material) Material->ColorModulate(g_menu.main.players.chams_skybox_col.get());
	static auto sv_skyname = g_csgo.m_cvar->FindVar(HASH("sv_skyname"));

	// iterate material handles.
	for (uint16_t h{ g_csgo.m_material_system->FirstMaterial() }; h != g_csgo.m_material_system->InvalidMaterial(); h = g_csgo.m_material_system->NextMaterial(h)) {
		// get material from handle.
		IMaterial* mat = g_csgo.m_material_system->GetMaterial(h);
		if (!mat)
			continue;

		// store world materials.
		if (FNV1a::get(mat->GetTextureGroupName()) == HASH("World textures"))
			world.push_back(mat);

		// store props.
		else if (FNV1a::get(mat->GetTextureGroupName()) == HASH("StaticProp textures"))
			props.push_back(mat);
	}

	// transparent props.
	if (g_menu.main.visuals.transparent_props.get()) {

		// IsUsingStaticPropDebugModes my nigga
		if (g_csgo.r_DrawSpecificStaticProp->GetInt() != 0) {
			g_csgo.r_DrawSpecificStaticProp->SetValue(0);
		}

		for (const auto& p : props)
			p->AlphaModulate(0.85f);
	}

	// disable transparent props.
	else {

		// restore r_DrawSpecificStaticProp.
		if (g_csgo.r_DrawSpecificStaticProp->GetInt() != -1) {
			g_csgo.r_DrawSpecificStaticProp->SetValue(-1);
		}

		for (const auto& p : props)
			p->AlphaModulate(1.0f);
	}

	// night.
	if (g_menu.main.visuals.world.get() == 1) {
		for (const auto& w : world)
			w->ColorModulate(night_amt, night_amt, night_amt);

		// IsUsingStaticPropDebugModes my nigga
		if (g_csgo.r_DrawSpecificStaticProp->GetInt() != 0) {
			g_csgo.r_DrawSpecificStaticProp->SetValue(0);
		}

		for (const auto& p : props)
			p->ColorModulate(night_amt, night_amt, night_amt);

		sv_skyname->SetValue(XOR("sky_csgo_night02b"));
	}

	else if (g_menu.main.visuals.world.get() == 3) {
		Color nigger = g_menu.main.visuals.ambient_color.get();

		for (const auto& w : world)
			w->ColorModulate(nigger.r() / 255.f, nigger.g() / 255.f, nigger.b() / 255.f);

		// IsUsingStaticPropDebugModes my black friend
		if (g_csgo.r_DrawSpecificStaticProp->GetInt() != 0) {
			g_csgo.r_DrawSpecificStaticProp->SetValue(0);
		}

		sv_skyname->SetValue(XOR("sky_csgo_night02b"));

		for (const auto& p : props)
			p->ColorModulate(0.5f, 0.5f, 0.5f);
	}

	// disable night.
	else {
		for (const auto& w : world)
			w->ColorModulate(1.f, 1.f, 1.f);

		// restore r_DrawSpecificStaticProp.
		if (g_csgo.r_DrawSpecificStaticProp->GetInt() != -1) {
			g_csgo.r_DrawSpecificStaticProp->SetValue(-1);
		}

		sv_skyname->SetValue(XOR("sky_day02_05"));

		for (const auto& p : props)
			p->ColorModulate(1.f, 1.f, 1.f);
	}

}

void DrawPlate(std::string text, std::string sub_text, int x, int y, bool centered, int barpos, float barpercent, Color barclr, int alpha) {
	int dst = false;
	float a = alpha / 255.f;
	int bar_size = 8;
	render::FontSize_t txts = render::bold.size(text);
	bar_size += txts.m_width;

	if (sub_text != "") {
		dst = true;
		render::FontSize_t mini_txts = render::norm.size(sub_text);
		bar_size += 2 + mini_txts.m_width;
	}

	int x_pos = x;
	if (centered)
		x_pos = floor(x - (bar_size / 2));


	render::rect_filled(x_pos, y, bar_size, 16, Color(0, 0, 0, 150 * a));

	render::bold.string(x_pos + 4, y + 2, Color(255, 255, 255, 255 * a), text);

	if (dst)
		render::norm.string(x_pos + txts.m_width + 6, y + 2, Color(200, 200, 200, 255 * a), sub_text);


	if (barpos != 0) {
		if (barpos == 1) {
			render::rect_filled(x_pos, y + 16, bar_size, 1, Color(0, 0, 0, 255 * a));

			//x + (backsize / 2 - barpercent / 2 * backsize), y - 1, x + (backsize / 2 - barpercent / 2 * backsize) + barpercent * backsize, y


			render::rect_filled(x_pos + (bar_size / 2 - barpercent / 2 * bar_size), y + 16, barpercent * bar_size, 1, Color(barclr.r(), barclr.g(), barclr.b(), 255 * a));
		}
		else if (barpos == 2) {
			render::rect_filled(x_pos, y - 1, bar_size, 1, Color(0, 0, 0, 255 * a));
			render::rect_filled(x_pos + (bar_size / 2 - barpercent / 2 * bar_size), y - 1, barpercent * bar_size, 1, Color(barclr.r(), barclr.g(), barclr.b(), 255 * a));
		}
	}

}

void Visuals::RacistRectangle() {
	//if (!g_menu.main.visuals.racist_rectangle.get()) return;

	if (!g_cl.m_processing) return;

	// face
	render::rect_filled(200, 200, 200, 200, Color(30, 30, 30, 255));
	// left eye
	render::rect_filled(235, 235, 30, 30, Color(255, 255, 255, 255));
	// right eye
	render::rect_filled(335, 235, 30, 30, Color(255, 255, 255, 255));
	// mouth
	render::rect_filled(235, 345, 130, 20, Color(255, 255, 255, 255));
}
bool active;
float start_time;
float stop_time;

float EaseShit(float t, float b, float c, float d) {
	if ((t /= d / 2) < 1) return (c / 2) * t * t * t + b;
	return (c / 2) * ((t -= 2) * t * t + 2) + b;
}
void Visuals::ThirdpersonThink() {
	ang_t                          offset;
	vec3_t                         origin, forward;
	static CTraceFilterSimple_game filter{ };
	CGameTrace                     tr;

	// for whatever reason overrideview also gets called from the main menu.
	if (!g_csgo.m_engine->IsInGame())
		return;

	// check if we have a local player and he is alive.
	bool alive = g_cl.m_local && g_cl.m_local->alive();
	if (m_thirdperson) {
		active = true;
		stop_time = g_csgo.m_globals->m_curtime;
	}
	else if (!m_thirdperson) {
		// we stopped
		start_time = g_csgo.m_globals->m_curtime;
	}
	// camera should be in thirdperson.
	if (active) {

		// if alive and not in thirdperson already switch to thirdperson.
		if (alive && !g_csgo.m_input->CAM_IsThirdPerson())
			g_csgo.m_input->CAM_ToThirdPerson();

		// if dead and spectating in firstperson switch to thirdperson.
		else if (g_cl.m_local->m_iObserverMode() == 4) {

			// if in thirdperson, switch to firstperson.
			// we need to disable thirdperson to spectate properly.
			if (g_csgo.m_input->CAM_IsThirdPerson()) {
				g_csgo.m_input->CAM_ToFirstPerson();
				g_csgo.m_input->m_camera_offset.z = 0.f;
			}

			g_cl.m_local->m_iObserverMode() = 5;
		}
	}

	// camera should be in firstperson.
	else if (g_csgo.m_input->CAM_IsThirdPerson()) {
		g_csgo.m_input->CAM_ToFirstPerson();
		g_csgo.m_input->m_camera_offset.z = 0.f;
	}

	// if after all of this we are still in thirdperson.
	if (g_csgo.m_input->CAM_IsThirdPerson()) {
		// get camera angles.
		g_csgo.m_engine->GetViewAngles(offset);

		// get our viewangle's forward directional vector.
		math::AngleVectors(offset, &forward);

		// cam_idealdist convar.
		float value;
		if (!m_thirdperson) {
			// we stopping ok
			float wtf = EaseShit((g_csgo.m_globals->m_curtime - stop_time), 0, g_menu.main.visuals.thirdperson_dist.get(), .3);
			value = g_menu.main.visuals.thirdperson_dist.get() - wtf;
			if (wtf >= (g_menu.main.visuals.thirdperson_dist.get() - 16)) {
				active = false; stop_time = 0;
			}
		}
		else if (m_thirdperson) {
			value = EaseShit((g_csgo.m_globals->m_curtime - start_time), 0, g_menu.main.visuals.thirdperson_dist.get(), .3);
			if (value > g_menu.main.visuals.thirdperson_dist.get()) value = g_menu.main.visuals.thirdperson_dist.get();
		}

		offset.z = std::clamp(value, 1.f, 300.f);

		// start pos.
		origin = g_cl.m_shoot_pos;

		// setup trace filter and trace.
		filter.SetPassEntity(g_cl.m_local);

		g_csgo.m_engine_trace->TraceRay(
			Ray(origin, origin - (forward * offset.z), { -16.f, -16.f, -16.f }, { 16.f, 16.f, 16.f }),
			MASK_NPCWORLDSTATIC,
			(ITraceFilter*)&filter,
			&tr
		);

		// adapt distance to travel time.
		math::clamp(tr.m_fraction, 0.f, 1.f);
		offset.z *= tr.m_fraction;

		// override camera angles.
		g_csgo.m_input->m_camera_offset = { offset.x, offset.y, offset.z };
	}
}

void Visuals::ImpactData()
{
	if (!g_cl.m_processing) return;


}

void Visuals::Hitmarker() {
	static auto cross = g_csgo.m_cvar->FindVar(HASH("weapon_debug_spread_show"));
	cross->SetValue(g_menu.main.visuals.force_xhair.get() && !g_cl.m_local->m_bIsScoped() ? 3 : 0);
	if (!g_menu.main.misc.hitmarker.get())
		return;

	if (g_csgo.m_globals->m_curtime > m_hit_end)
		return;

	if (m_hit_duration <= 0.f)
		return;

	float complete = (g_csgo.m_globals->m_curtime - m_hit_start) / m_hit_duration;
	int x = g_cl.m_width / 2,
		y = g_cl.m_height / 2,
		alpha = (1.f - complete) * 240;

	constexpr int line{ 6 };

	render::line(x - line, y - line, x - (line / 4), y - (line / 4), { 200, 200, 200, alpha });
	render::line(x - line, y + line, x - (line / 4), y + (line / 4), { 200, 200, 200, alpha });
	render::line(x + line, y + line, x + (line / 4), y + (line / 4), { 200, 200, 200, alpha });
	render::line(x + line, y - line, x + (line / 4), y - (line / 4), { 200, 200, 200, alpha });
}

void Visuals::NoSmoke() {
	if (!smoke1)
		smoke1 = g_csgo.m_material_system->FindMaterial(XOR("particle/vistasmokev1/vistasmokev1_fire"), XOR("Other textures"));

	if (!smoke2)
		smoke2 = g_csgo.m_material_system->FindMaterial(XOR("particle/vistasmokev1/vistasmokev1_smokegrenade"), XOR("Other textures"));

	if (!smoke3)
		smoke3 = g_csgo.m_material_system->FindMaterial(XOR("particle/vistasmokev1/vistasmokev1_emods"), XOR("Other textures"));

	if (!smoke4)
		smoke4 = g_csgo.m_material_system->FindMaterial(XOR("particle/vistasmokev1/vistasmokev1_emods_impactdust"), XOR("Other textures"));

	if (g_menu.main.visuals.removals.get(1)) {
		if (!smoke1->GetFlag(MATERIAL_VAR_NO_DRAW))
			smoke1->SetFlag(MATERIAL_VAR_NO_DRAW, true);

		if (!smoke2->GetFlag(MATERIAL_VAR_NO_DRAW))
			smoke2->SetFlag(MATERIAL_VAR_NO_DRAW, true);

		if (!smoke3->GetFlag(MATERIAL_VAR_NO_DRAW))
			smoke3->SetFlag(MATERIAL_VAR_NO_DRAW, true);

		if (!smoke4->GetFlag(MATERIAL_VAR_NO_DRAW))
			smoke4->SetFlag(MATERIAL_VAR_NO_DRAW, true);
	}

	else {
		if (smoke1->GetFlag(MATERIAL_VAR_NO_DRAW))
			smoke1->SetFlag(MATERIAL_VAR_NO_DRAW, false);

		if (smoke2->GetFlag(MATERIAL_VAR_NO_DRAW))
			smoke2->SetFlag(MATERIAL_VAR_NO_DRAW, false);

		if (smoke3->GetFlag(MATERIAL_VAR_NO_DRAW))
			smoke3->SetFlag(MATERIAL_VAR_NO_DRAW, false);

		if (smoke4->GetFlag(MATERIAL_VAR_NO_DRAW))
			smoke4->SetFlag(MATERIAL_VAR_NO_DRAW, false);
	}
}
void Polygon(int count, std::vector< Vertex > vertices, const Color& col) {
	static int texture = g_csgo.m_surface->CreateNewTextureID(true);

	g_csgo.m_surface->DrawSetTextureRGBA(texture, &colors::white, 1, 1);
	g_csgo.m_surface->DrawSetColor(col);
	g_csgo.m_surface->DrawSetTexture(texture);

	g_csgo.m_surface->DrawTexturedPolygon(vertices.size(), vertices.data());
}
void FilledTriangle(const vec2_t& pos1, const vec2_t& pos2, const vec2_t& pos3, const Color& col) {
	Vertex o;
	Vertex t;
	Vertex tt;
	o.init(pos1);
	t.init(pos2);
	tt.init(pos3);
	std::vector< Vertex > vertices{};
	vertices.push_back(o);
	vertices.push_back(t);
	vertices.push_back(tt);
	Polygon(3, vertices, col);
}
void Line(vec2_t v0, vec2_t v1, Color color) {
	g_csgo.m_surface->DrawSetColor(color);
	g_csgo.m_surface->DrawLine(v0.x, v0.y, v1.x, v1.y);
}
void WorldCircle(vec3_t origin, float radius, Color color, Color colorFill) {
	std::vector<vec3_t> Points3D;
	float step = static_cast<float>(M_PI) * 2.0f / 256;

	for (float a = 0; a < (M_PI * 2.0f); a += step) {
		vec3_t start(radius * cosf(a) + origin.x, radius * sinf(a) + origin.y, origin.z);
		vec3_t end(radius * cosf(a + step) + origin.x, radius * sinf(a + step) + origin.y, origin.z);

		vec2_t out, out1, pos3d;

		if (render::WorldToScreen(end, out1) && render::WorldToScreen(start, out)) {
			if (colorFill.a() && render::WorldToScreen(origin, pos3d)) {
				FilledTriangle(out, out1, pos3d, colorFill);
			}

			Line(out, out1, color);
		}
	}
}
extern CCSGOPlayerAnimState* Fake_STATE;
Color LerpRGB(Color a, Color b, float t)
{
	return Color
	(
		a.r() + (b.r() - a.r()) * t,
		a.g() + (b.g() - a.g()) * t,
		a.b() + (b.b() - a.b()) * t,
		a.a() + (b.a() - a.a()) * t
	);
}
extern vec3_t position;
extern vec3_t OriginExtr;
extern vec3_t BehindVector;
void arc(int x, int y, int radius, int radius_inner, int start_angle, int end_angle, int segments, Color color)
{
	g_csgo.m_surface->DrawSetColor(color);

	segments = 360 / segments;

	for (float i = start_angle; i < start_angle + end_angle; i = i + segments)
	{

		float rad = i * math::pi / 180;
		float rad2 = (i + segments) * math::pi / 180;

		float rad_cos = cos(rad);
		float rad_sin = sin(rad);

		float rad2_cos = cos(rad2);
		float rad2_sin = sin(rad2);

		float x1_inner = x + rad_cos * radius_inner;
		float y1_inner = y + rad_sin * radius_inner;

		float x1_outer = x + rad_cos * radius;
		float y1_outer = y + rad_sin * radius;

		float x2_inner = x + rad2_cos * radius_inner;
		float y2_inner = y + rad2_sin * radius_inner;

		float x2_outer = x + rad2_cos * radius;
		float y2_outer = y + rad2_sin * radius;

		Vertex verts[3] = {
			Vertex(vec2_t{ x1_outer, y1_outer }),
			Vertex(vec2_t{ x2_outer, y2_outer }),
			Vertex(vec2_t{ x1_inner, y1_inner })
		};

		Vertex verts2[3] = {
			Vertex(vec2_t{ x1_inner, y1_inner }),
			Vertex(vec2_t{ x2_outer, y2_outer }),
			Vertex(vec2_t{ x2_inner, y2_inner })
		};

		if (verts)
			g_csgo.m_surface->DrawTexturedPolygon(3, verts);

		if (verts2)
			g_csgo.m_surface->DrawTexturedPolygon(3, verts2);
	}
}
extern float m_flNextBodyUpdate;
#include <sstream>

template <typename T>
std::string to_string_with_precision(const T a_value, const int n = 6)
{
	std::ostringstream out;
	out.precision(n);
	out << std::fixed << a_value;
	return out.str();
}
void Visuals::hotkeys()
{
	Color menu_col =  g_gui.m_color;
	if (!g_cl.m_processing) return;
	CCSGOPlayerAnimState* state = g_cl.m_local->m_PlayerAnimState();
	if (!state) return;
	/*WorldCircle(OriginExtr, 4, Color(255, 255, 255, 255), Color(255, 255, 255, 255));
	WorldCircle(BehindVector, 4, Color(0, 0, 255, 255), Color(0, 0, 255, 255));*/
	if (g_input.GetKeyState(g_menu.main.misc.autopeek.get())) {
		WorldCircle(g_movement.quickpeekstartpos, 15, Color(255, 0, 0, 255), Color(255, 0, 0, 255));

	}
	struct Hotkeys_t { Color color; std::string text; };
	std::vector< Hotkeys_t > hotkeys{ };

	if (g_aimbot.m_fake_latency)
	{
		Hotkeys_t data;
		data.color = LerpRGB(0xff0000ff, 0xff15c27b, std::clamp((g_csgo.m_cl->m_net_channel->GetLatency(INetChannel::FLOW_INCOMING)*1000.f) / g_menu.main.misc.fake_latency_amt.get(), 0.f, 1.f));
		data.text = "PING";

		hotkeys.push_back(data);
	}

	if ((g_menu.main.antiaim.lag_enable.get()) && (g_cl.m_local->m_vecVelocity().length_2d() > 270.f || g_cl.m_lagcomp) && !g_input.GetKeyState(g_menu.main.antiaim.lag_exploit.get()))
	{
		Hotkeys_t data;

		data.color = g_cl.m_lagcomp ? 0xff15c27b : 0xff0000ff;
		data.text = "LC";

		hotkeys.push_back(data);
	}
	bool dmg = (g_aimbot.damage_override && g_menu.main.aimbot.dmg_override_mode.get() == 1) || (g_menu.main.aimbot.dmg_override_mode.get() == 0 && g_input.GetKeyState(g_menu.main.aimbot.dmg_override.get()));

	if (dmg) {
		Hotkeys_t data;

		data.color = { 210,210,210,255 };
		data.text = "DMG";

		hotkeys.push_back(data);

	}

	if (g_menu.main.antiaim.enable.get()) {

		Hotkeys_t data;
		float change = std::abs(math::NormalizedAngle(g_cl.m_local->m_flLowerBodyYawTarget() - g_cl.m_angle.y));
		data.color = (g_hvh.m_desync || (g_input.GetKeyState(g_menu.main.misc.fakewalk.get()) && g_menu.main.misc.fakewalk_mode.get() == 1)) ? 0xff15c27b : std::abs(math::AngleDiff(g_cl.m_angle.y, g_cl.m_local->m_flLowerBodyYawTarget())) > 35.f ? 0xff15c27b : 0xff0000ff;
		data.text = (g_hvh.m_desync || (g_input.GetKeyState(g_menu.main.misc.fakewalk.get()) && g_menu.main.misc.fakewalk_mode.get() == 1)) ? "FAKE" : "LBY";
		hotkeys.push_back(data);
	}
	if (g_input.GetKeyState(g_menu.main.aimbot.baim_key.get()))
	{
		Hotkeys_t data;

		data.color = { 210,210,210,255 };
		data.text = "SALT";

		hotkeys.push_back(data);
	}
	if (g_tickshift.m_double_tap) {
		Hotkeys_t data;

		data.color = LerpRGB(0xff0000ff, 0xff15c27b, g_tickshift.m_charged_ticks/14.f);
		data.text = "HIGH CHOLESTEROL";

		hotkeys.push_back(data);
	}
	if (hotkeys.empty())
		return;

	if (hotkeys.empty())
		return;
	float add;

	Color black = colors::black;
	black.a() = 180;

	for (size_t i{ }; i < hotkeys.size(); ++i) {
		auto& indicator = hotkeys[i];
		if (indicator.text == "SUGAR")
		{
			add = (g_cl.m_body - g_csgo.m_globals->m_curtime) / 1.1;
			math::clamp(add, 0.f, 1.f);
			render::FontSize_t IndSize = render::indicator.size(indicator.text);
			//arc(20 + IndSize.m_width + 12, g_cl.m_height - 90 - (30 * i) + 24, 9, 4, 0, 360, 60, black);
			//arc(20 + IndSize.m_width + 12, g_cl.m_height - 90 - (30 * i) + 24, 8, 5, 0, 360 * (add), 60, indicator.color);
		}
		if (indicator.text == "BLOOD PRESSURE SPIKE")
		{
			CCSGOPlayerAnimState* state = g_cl.m_local->m_PlayerAnimState();
			if (state)
			{
				render::FontSize_t IndSize = render::indicator.size(indicator.text);
				float add = std::abs(math::AngleDiff(state->m_goal_feet_yaw, Fake_STATE->m_goal_feet_yaw)) / 180;
				math::clamp(add, 0.f, 1.f);
				indicator.color = LerpRGB(0xff0000ff, 0xff15c27b, add);
			}
		}
		render::indicator.string(20, g_cl.m_height - 80 - (30 * i), indicator.color, indicator.text);
	}
}

void Visuals::think() {
	// don't run anything if our local player isn't valid.
	if (!g_cl.m_local)
		return;

	if (g_menu.main.visuals.removals.get(4)
		&& g_cl.m_local->alive()
		&& g_cl.m_local->GetActiveWeapon()
		&& g_cl.m_local->GetActiveWeapon()->GetWpnData()->m_weapon_type == CSWeaponType::WEAPONTYPE_SNIPER_RIFLE
		&& g_cl.m_local->m_bIsScoped()) {

		// rebuild the original scope lines.
		int w = g_cl.m_width,
			h = g_cl.m_height,
			x = w / 2,
			y = h / 2,
			size = g_csgo.cl_crosshair_sniper_width->GetInt();

		// Here We Use The Euclidean distance To Get The Polar-Rectangular Conversion Formula.
		if (size > 1) {
			x -= (size / 2);
			y -= (size / 2);
		}

		// draw our lines.
		render::rect_filled(0, y, w, size, colors::black);
		render::rect_filled(x, 0, size, h, colors::black);
	}

	// draw esp on ents.
	for (int i{ 1 }; i <= g_csgo.m_entlist->GetHighestEntityIndex(); ++i) {
		Entity* ent = g_csgo.m_entlist->GetClientEntity(i);
		if (!ent)
			continue;

		draw(ent);
	}

	// draw everything else.
	hotkeys();
	SpreadCrosshair();
	//StatusIndicators( );
	Spectators();
	PenetrationCrosshair();
	Hitmarker();
	DrawPlantedC4();
	Indicators();
	if (g_hvh.man_left || g_hvh.man_right || g_hvh.man_back) {
		// manual aa
		int x, y;
		x = g_cl.m_width / 2;
		y = g_cl.m_height / 2;
		int x1, y1, x2, y2;
		Color wtf = g_gui.m_color;
		wtf.a() = 127;
		if (g_hvh.man_left) {
			render::line(vec2_t(x - 50, y), vec2_t(x - 35, y - 9), wtf);
			render::line(vec2_t(x - 50, y), vec2_t(x - 35, y + 9), wtf);
			render::line(vec2_t(x - 35, y - 9), vec2_t(x - 35, y + 11), wtf);

			render::line(vec2_t(x - 52, y), vec2_t(x - 34, y - 11), g_gui.m_color);
			render::line(vec2_t(x - 52, y), vec2_t(x - 34, y + 11), g_gui.m_color);
			render::line(vec2_t(x - 34, y - 10), vec2_t(x - 34, y + 12), g_gui.m_color);
		}
		else if (g_hvh.man_right) {
			render::line(vec2_t(x + 50, y), vec2_t(x + 35, y - 9), wtf);
			render::line(vec2_t(x + 50, y), vec2_t(x + 35, y + 9), wtf);
			render::line(vec2_t(x + 35, y - 9), vec2_t(x + 35, y + 11), wtf);

			render::line(vec2_t(x + 52, y), vec2_t(x + 34, y - 11), g_gui.m_color);
			render::line(vec2_t(x + 52, y), vec2_t(x + 34, y + 11), g_gui.m_color);
			render::line(vec2_t(x + 34, y - 10), vec2_t(x + 34, y + 12), g_gui.m_color);
		}
	}
	static auto cl_foot_contact_shadows = g_csgo.m_cvar->FindVar(HASH("cl_foot_contact_shadows"));
	if (g_cl.m_processing && g_menu.main.players.chams_local_skeleton.get() && m_thirdperson)
	{
		cl_foot_contact_shadows->SetValue(0);
		DrawSkeleton(g_cl.m_local, 255);
	}
	else
	{
		cl_foot_contact_shadows->SetValue(1);
	}
	//RacistRectangle();
}

void Visuals::Spectators() {
	if (!g_menu.main.visuals.spectators.get())
		return;

	std::vector< std::string > spectators{ };
	int h = render::norm.m_size.m_height;

	for (int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i) {
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >(i);
		if (!player)
			continue;

		if (player->m_bIsLocalPlayer())
			continue;

		if (player->dormant())
			continue;

		if (player->m_lifeState() == LIFE_ALIVE)
			continue;

		if (player->GetObserverTarget() != g_cl.m_local)
			continue;

		player_info_t info;
		if (!g_csgo.m_engine->GetPlayerInfo(i, &info))
			continue;

		spectators.push_back(std::string(info.m_name).substr(0, 24));
	}

	size_t total_size = spectators.size() * (h - 1);

	for (size_t i{ }; i < spectators.size(); ++i) {
		const std::string& name = spectators[i];

		render::norm.string(g_cl.m_width - 3, 15 + (i * (h - 1)), { 255, 255, 255, 255 }, name, render::ALIGN_RIGHT);
	}
}

void Visuals::StatusIndicators() {
}

void Visuals::SpreadCrosshair() {
	// dont do if dead.
	if (!g_cl.m_processing)
		return;

	if (!g_menu.main.visuals.spread_xhair.get())
		return;

	// get active weapon.
	Weapon* weapon = g_cl.m_local->GetActiveWeapon();
	if (!weapon)
		return;

	WeaponInfo* data = weapon->GetWpnData();
	if (!data)
		return;

	// do not do this on: bomb, knife and nades.
	CSWeaponType type = data->m_weapon_type;
	if (type == WEAPONTYPE_KNIFE || type == WEAPONTYPE_C4 || type == WEAPONTYPE_GRENADE)
		return;


	int cross_x = g_cl.m_height / 2, cross_y = g_cl.m_width / 2; //g_cl.m_local->GetFOV()

	float recoil_step = g_cl.m_height / g_cl.m_local->GetFOV();

	cross_x -= (int)(g_cl.m_local->m_aimPunchAngle().y * recoil_step);
	cross_y += (int)(g_cl.m_local->m_aimPunchAngle().x * recoil_step);


	//		weapon->update_accuracy_penalty();
	float inaccuracy = weapon->GetInaccuracy();
	float spread = weapon->GetSpread();

	float cone = inaccuracy * spread;
	cone *= g_cl.m_height * 0.7f;
	cone *= 90.f / g_cl.m_local->GetFOV();

	for (int seed{ }; seed < 256; ++seed) {
		//g_csgo.RandomSeed(g_csgo.RandomInt(0, 255) + 1);
		float rand_a = g_csgo.RandomFloat(0.f, 1.0f);
		float pi_rand_a = g_csgo.RandomFloat(0.f, 2.0f * M_PI);
		float rand_b = g_csgo.RandomFloat(0.0f, 1.0f);
		float pi_rand_b = g_csgo.RandomFloat(0.f, 2.f * M_PI);

		float spread_x = cos(pi_rand_a) * (rand_a * inaccuracy) + cos(pi_rand_b) * (rand_b * spread);
		float spread_y = sin(pi_rand_a) * (rand_a * inaccuracy) + sin(pi_rand_b) * (rand_b * spread);

		float max_x = cos(pi_rand_a) * cone + cos(pi_rand_b) * cone;
		float max_y = sin(pi_rand_a) * cone + sin(pi_rand_b) * cone;

		float step = g_cl.m_height / g_cl.m_local->GetFOV() * 90.f;
		int screen_spread_x = (int)(spread_x * step * 0.7f);
		int screen_spread_y = (int)(spread_y * step * 0.7f);

		float percentage = (rand_a * inaccuracy + rand_b * spread) / (inaccuracy + spread);


		int alpha = 255 * (0.4f + percentage * 0.6f) * (0.1f + percentage * 0.9f);
		render::rect_filled(cross_x + screen_spread_x, cross_y + screen_spread_y, 2, 2, { 255, 255, 255, 30 });
		//	draw_filled_rect(cross_x + screen_spread_x, cross_y + screen_spread_y, 1, 1,
		//		clr_t(255, 255, 255, 255 * (0.4f + percentage * 0.6f)) * (0.1f + percentage * 0.9f));
	}


	//// calc radius.
	//float radius = ((weapon->GetInaccuracy() + weapon->GetSpread()) * 320.f) / (std::tan(math::deg_to_rad(g_cl.m_local->GetFOV()) * 0.5f) + FLT_EPSILON);

	//// scale by screen size.
	//radius *= g_cl.m_height * (1.f / 480.f);

	//// get color.
	//Color col = { 20, 20, 20, 20 };

	//int segements = std::max(16, (int)std::round(radius * 0.75f));
	//render::circle(g_cl.m_width / 2, g_cl.m_height / 2, radius, segements, col);
}

void Visuals::PenetrationCrosshair() {
	int   x, y;
	bool  valid_player_hit;
	Color final_color;

	if (!g_menu.main.visuals.pen_crosshair.get() || !g_cl.m_processing)
		return;

	x = g_cl.m_width / 2;
	y = g_cl.m_height / 2;


	/*valid_player_hit = (g_cl.m_pen_data.m_target && g_cl.m_pen_data.m_target->enemy(g_cl.m_local));
	if (valid_player_hit)
		final_color = colors::transparent_yellow;*/

	if (g_cl.m_pen_data.m_pen)
		final_color = colors::transparent_green;

	else
		final_color = colors::transparent_red;

	// todo - dex; use fmt library to get damage string here?
	//             draw damage string?

	// draw small square in center of screen.
	//if (g_cl.m_pen_data.m_pen) {
	render::rect_filled(x - 1, y, 1, 1, { final_color });
	render::rect_filled(x, y, 1, 1, { final_color });
	render::rect_filled(x + 1, y, 1, 1, { final_color });
	render::rect_filled(x, y + 1, 1, 1, { final_color });
	render::rect_filled(x, y - 1, 1, 1, { final_color });
	//}
}

void Visuals::draw(Entity* ent) {
	if (ent->IsPlayer()) {
		Player* player = ent->as< Player* >();

		// dont draw dead players.
		if (!player->alive())
			return;

		if (player->m_bIsLocalPlayer())
			return;

		// draw player esp.
		DrawPlayer(player);
	}
	else if (g_menu.main.visuals.items.get() && ent->IsBaseCombatWeapon() && !ent->dormant()) {

		DrawItem(ent->as< Weapon* >());

	}
	else if (g_menu.main.visuals.proj.get())
		DrawProjectile(ent->as< Weapon* >());

	if (g_menu.main.visuals.planted_c4.get() && !ent->dormant() && (ent->is(HASH("CHostage")) || ent->is(HASH("CC4"))))
		DrawBombAndHostage(ent->as< Weapon* >());
}
float EntSpawns[2048];

void Visuals::DrawBombAndHostage(Weapon* ent) {

	Entity* owner = g_csgo.m_entlist->GetClientEntityFromHandle(ent->m_hOwnerEntity());
	if (owner)
		return;

	vec2_t screen;
	vec3_t origin = ent->GetAbsOrigin();
	if (!render::WorldToScreen(origin, screen))
		return;

	if (ent->is(HASH("CHostage"))) {

		render::bold.string(screen.x, screen.y - 6, colors::light_blue, XOR("HOSTAGE"), render::ALIGN_CENTER);

	}

	else if (ent->is(HASH("CC4")))
		render::bold.string(screen.x, screen.y - 6, colors::light_red, XOR("BOMB"), render::ALIGN_CENTER);

}

void Visuals::DrawProjectile(Weapon* ent) {
	vec2_t screen;
	vec3_t origin = ent->GetAbsOrigin();
	if (!render::WorldToScreen(origin, screen))
		return;

	if (!g_csgo.m_engine->IsInGame() && !g_csgo.m_engine->IsConnected())
		return;

	if (!g_menu.main.visuals.proj.get())
		return;

	Color col = colors::white;

	auto dist_world = g_cl.m_local->m_vecOrigin().dist_to(origin);
	int dist = g_cl.m_local->m_vecOrigin().dist_to(origin) / 12;

	if (dist_world > 250.f) {
		col.a() *= std::clamp((750.f - (dist_world - 400.f)) / 750.f, 0.f, 1.f);
	}

	if (dist < 175) {
		// draw decoy.
		//if (ent->is(HASH("CDecoyProjectile"))) {
		//	render::bold.string(screen.x, screen.y, Color(255, 255, 255, 200), XOR("decoy"), render::ALIGN_CENTER);
		//}

		// draw molotov.
		if (ent->is(HASH("CMolotovProjectile")) && !ent->dormant()) {
			if (dist <= 15)
				render::circle(screen.x, screen.y - 10, 20, 360, Color(255, 0, 0, 50));

			render::circle(screen.x, screen.y - 10, 20, 360, Color(26, 26, 30, 150));
			render::circle_outline(screen.x, screen.y - 10, 20, 360, Color(26, 26, 30, 255));
			render::undefeatedd.string(screen.x - 8, screen.y - 23, Color(255, 255, 255, 200), XOR("l"));
		}

		else if (ent->is(HASH("CBaseCSGrenadeProjectile"))) {
			const model_t* model = ent->GetModel();

			if (model) {
				// grab modelname.
				std::string name{ ent->GetModel()->m_name };

				if (name.find(XOR("flashbang")) != std::string::npos) {
					render::circle(screen.x, screen.y - 10, 20, 360, Color(26, 26, 30, 150));
					render::circle_outline(screen.x, screen.y - 10, 20, 360, Color(26, 26, 30, 255));
					render::undefeatedd.string(screen.x - 5, screen.y - 23, Color(255, 255, 255, 255), XOR("i"));
				}

				else if (name.find(XOR("fraggrenade")) != std::string::npos) {
					if (dist <= 20)
						render::circle(screen.x, screen.y - 10, 20, 360, Color(255, 0, 0, 50));

					render::circle(screen.x, screen.y - 10, 20, 360, Color(26, 26, 30, 150));
					render::circle_outline(screen.x, screen.y - 10, 20, 360, Color(26, 26, 30, 255));
					render::undefeatedd.string(screen.x - 8, screen.y - 23, Color(255, 255, 255, 255), XOR("j"));
				}
			}
		}

		else if (ent->is(HASH("CInferno"))) {
			const double spawn_time = *(float*)(uintptr_t(ent) + 0x20);
			const double reltime = ((spawn_time + 7.031) - g_csgo.m_globals->m_curtime);
			const double factor = reltime / 7.031;

			if (factor <= 0) return;

			Color mollycol = colors::red;
			mollycol.a() = col.a();

			if (reltime <= 1.f) {
				mollycol.a() = mollycol.a() * (reltime / 1.f);
			}

			if (g_menu.main.visuals.proj_range.get(1))
			{
				// setup our vectors.
				std::vector<vec2_t> mollypoints, finalp;
				vec3_t n_origin = ent->m_vecOrigin();

				//// get molotov bounds (current radius).
				//ent->GetRenderBounds(mins, maxs);

				math::GetRadius2d(&mollypoints, n_origin, 100, 5);


				int* m_fireXDelta = reinterpret_cast<int*> ((DWORD)ent + g_entoffsets.m_fireXDelta);
				int* m_fireYDelta = reinterpret_cast<int*> ((DWORD)ent + g_entoffsets.m_fireYDelta);
				int* m_fireZDelta = reinterpret_cast<int*> ((DWORD)ent + g_entoffsets.m_fireZDelta);

				for (int fires = 0; fires < ent->m_fireCount(); fires++) {

					math::GetRadius2d(&mollypoints, vec3_t(n_origin.x + m_fireXDelta[fires + 1], n_origin.y + m_fireYDelta[fires + 1], n_origin.z + m_fireZDelta[fires + 1]), 70, 5);
				}

				/*;*/
				if (math::GiftWrap(mollypoints, &finalp)) {

					for (int i = 0; i < finalp.size(); i++) {
						vec2_t start_p = finalp[i];


						vec2_t end_p = finalp[0];
						if (i != finalp.size() - 1)
							end_p = finalp[i + 1];
						render::line(start_p, end_p, mollycol);
					}

				}


				//// render the smoke range circle.
				//render::WorldCircleOutline(origin, vec3_t(maxs - mins).length_2d() * 0.5, 1.f, mollycol);
			}

			//DrawPlate(std::string text, std::string sub_text, int x, int y, bool centered, int barpos, float barpercent, Color barclr, int alpha)
			if (dist <= 15)
				render::circle(screen.x, screen.y - 10, 20, 360, Color(255, 0, 0, 50));

			render::circle(screen.x, screen.y - 10, 20, 360, Color(26, 26, 30, 150));
			render::circle_outline(screen.x, screen.y - 10, 20, 360, Color(26, 26, 30, 255));
			render::undefeatedd.string(screen.x - 8, screen.y - 23, Color(255, 255, 255, 200), XOR("l"));
		}

		else if (ent->is(HASH("CSmokeGrenadeProjectile"))) {
			Weapon* pSmokeEffect = reinterpret_cast<Weapon*>(ent);
			const float spawn_time = game::TICKS_TO_TIME(pSmokeEffect->m_nSmokeEffectTickBegin());
			const double reltime = ((spawn_time + 17.441) - g_csgo.m_globals->m_curtime);
			const double factor = reltime / 17.441;
			if (factor <= 0) return;

			Color smokecol = colors::light_blue;
			smokecol.a() = col.a();

			if (reltime <= 1.f) {
				smokecol.a() = smokecol.a() * (reltime / 1.f);
			}

			if (g_menu.main.visuals.proj_range.get(0))
			{
				float radius = 144.f;
				auto time_since_explosion = g_csgo.m_globals->m_interval * (g_csgo.m_globals->m_tick_count - ent->m_nSmokeEffectTickBegin());

				// animation.
				if (0.3f > time_since_explosion)
					radius = radius * 0.6f + (radius * (time_since_explosion / .3f)) * 0.4f;

				if (1.0f > (17.441 - time_since_explosion))
					radius = radius * (((17.441 - time_since_explosion) / 1.0f) * 0.3f + 0.7f);

				// render the smoke range circle.
				render::WorldCircleOutline(origin, radius, 1.f, smokecol);
			}

			render::circle(screen.x, screen.y - 10, 20, 360, Color(26, 26, 30, 150));
			render::circle_outline(screen.x, screen.y - 10, 20, 360, Color(26, 26, 30, 255));
			render::undefeatedd.string(screen.x - 6, screen.y - 23, Color(255, 255, 255, 200), XOR("k"));
			render::undefeatedd.semi_filled_text(screen.x - 6, screen.y - 23, Color(255, 255, 255, 255), XOR("k"), render::ALIGN_LEFT, factor, true);

		}
	}
}

void Visuals::DrawItem(Weapon* item) {
	// we only want to draw shit without owner.
	Entity* owner = g_csgo.m_entlist->GetClientEntityFromHandle(item->m_hOwnerEntity());
	if (owner)
		return;

	// is the fucker even on the screen?
	vec2_t screen;
	vec3_t origin = item->GetAbsOrigin();
	if (!render::WorldToScreen(origin, screen))
		return;

	WeaponInfo* data = item->GetWpnData();
	if (!data)
		return;

	Color col = g_menu.main.visuals.item_color.get();
	col.a() = 0xb4;

	// render bomb in green.

	// if not bomb
	// normal item, get its name.

	if (!item->is(HASH("CC4"))) {
		std::string name{ item->GetLocalizedName() };

		// smallfonts needs uppercase.
		std::transform(name.begin(), name.end(), name.begin(), ::toupper);

		render::esp_small.string(screen.x, screen.y, col, name, render::ALIGN_CENTER);
	}

	if (!g_menu.main.visuals.ammo.get())
		return;

	// nades do not have ammo.
	if (data->m_weapon_type == WEAPONTYPE_GRENADE || data->m_weapon_type == WEAPONTYPE_KNIFE)
		return;

	if (item->m_iItemDefinitionIndex() == 0 || item->m_iItemDefinitionIndex() == C4)
		return;

	std::string ammo = tfm::format(XOR("(%i/%i)"), item->m_iClip1(), item->m_iPrimaryReserveAmmoCount());
	render::esp_small.string(screen.x, screen.y - render::esp_small.m_size.m_height - 1, col, ammo, render::ALIGN_CENTER);
}

void Visuals::OffScreen(Player* player, int alpha) {
	vec3_t view_origin, target_pos, delta;
	vec2_t screen_pos, offscreen_pos;
	float  leeway_x, leeway_y, radius, offscreen_rotation;
	bool   is_on_screen;
	Vertex verts[3], verts_outline[3];
	Color  color;

	// todo - dex; move this?
	static auto get_offscreen_data = [](const vec3_t& delta, float radius, vec2_t& out_offscreen_pos, float& out_rotation) {
		ang_t  view_angles(g_csgo.m_view_render->m_view.m_angles);
		vec3_t fwd, right, up(0.f, 0.f, 1.f);
		float  front, side, yaw_rad, sa, ca;

		// get viewport angles forward directional vector.
		math::AngleVectors(view_angles, &fwd);

		// convert viewangles forward directional vector to a unit vector.
		fwd.z = 0.f;
		fwd.normalize();

		// calculate front / side positions.
		right = up.cross(fwd);
		front = delta.dot(fwd);
		side = delta.dot(right);

		// setup offscreen position.
		out_offscreen_pos.x = radius * -side;
		out_offscreen_pos.y = radius * -front;

		// get the rotation ( yaw, 0 - 360 ).
		out_rotation = math::rad_to_deg(std::atan2(out_offscreen_pos.x, out_offscreen_pos.y) + math::pi);

		// get needed sine / cosine values.
		yaw_rad = math::deg_to_rad(-out_rotation);
		sa = std::sin(yaw_rad);
		ca = std::cos(yaw_rad);

		// rotate offscreen position around.
		out_offscreen_pos.x = (int)((g_cl.m_width / 2.f) + (radius * sa));
		out_offscreen_pos.y = (int)((g_cl.m_height / 2.f) - (radius * ca));
	};

	if (!g_menu.main.players.offscreen.get())
		return;

	if (!g_cl.m_processing || !g_cl.m_local->enemy(player))
		return;

	// get the player's center screen position.
	target_pos = player->WorldSpaceCenter();
	is_on_screen = render::WorldToScreen(target_pos, screen_pos);

	// give some extra room for screen position to be off screen.
	leeway_x = g_cl.m_width / 18.f;
	leeway_y = g_cl.m_height / 18.f;

	// origin is not on the screen at all, get offscreen position data and start rendering.


	// get viewport origin.
	view_origin = g_csgo.m_view_render->m_view.m_origin;

	// get direction to target.
	delta = (target_pos - view_origin).normalized();

	// note - dex; this is the 'YRES' macro from the source sdk.
	radius = 200.f * (g_cl.m_height / 480.f);

	// get the data we need for rendering.
	get_offscreen_data(delta, radius, offscreen_pos, offscreen_rotation);

	// bring rotation back into range... before rotating verts, sine and cosine needs this value inverted.
	// note - dex; reference: 
	// https://github.com/VSES/SourceEngine2007/blob/43a5c90a5ada1e69ca044595383be67f40b33c61/src_main/game/client/tf/tf_hud_damageindicator.cpp#L182
	offscreen_rotation = -offscreen_rotation;

	// setup vertices for the triangle.
	verts[0] = { offscreen_pos.x, offscreen_pos.y };        // 0,  0
	verts[1] = { offscreen_pos.x - 12.f, offscreen_pos.y + 24.f }; // -1, 1
	verts[2] = { offscreen_pos.x + 12.f, offscreen_pos.y + 24.f }; // 1,  1

	// setup verts for the triangle's outline.
	verts_outline[0] = { verts[0].m_pos.x - 1.f, verts[0].m_pos.y - 1.f };
	verts_outline[1] = { verts[1].m_pos.x - 1.f, verts[1].m_pos.y + 1.f };
	verts_outline[2] = { verts[2].m_pos.x + 1.f, verts[2].m_pos.y + 1.f };

	// rotate all vertices to point towards our target.
	verts[0] = render::RotateVertex(offscreen_pos, verts[0], offscreen_rotation);
	verts[1] = render::RotateVertex(offscreen_pos, verts[1], offscreen_rotation);
	verts[2] = render::RotateVertex(offscreen_pos, verts[2], offscreen_rotation);
	// verts_outline[ 0 ] = render::RotateVertex( offscreen_pos, verts_outline[ 0 ], offscreen_rotation );
	// verts_outline[ 1 ] = render::RotateVertex( offscreen_pos, verts_outline[ 1 ], offscreen_rotation );
	// verts_outline[ 2 ] = render::RotateVertex( offscreen_pos, verts_outline[ 2 ], offscreen_rotation );

	// todo - dex; finish this, i want it.
	// auto &damage_data = m_offscreen_damage[ player->index( ) ];
	// 
	// // the local player was damaged by another player recently.
	// if( damage_data.m_time > 0.f ) {
	//     // // only a small amount of time left, start fading into white again.
	//     // if( damage_data.m_time < 1.f ) {
	//     //     // calculate step needed to reach 255 in 1 second.
	//     //     // float step = UINT8_MAX / ( 1000.f * g_csgo.m_globals->m_frametime );
	//     //     float step = ( 1.f / g_csgo.m_globals->m_frametime ) / UINT8_MAX;
	//     //     
	//     //     // increment the new value for the color.
	//     //     // if( damage_data.m_color_step < 255.f )
	//     //         damage_data.m_color_step += step;
	//     // 
	//     //     math::clamp( damage_data.m_color_step, 0.f, 255.f );
	//     // 
	//     //     damage_data.m_color.g( ) = (uint8_t)damage_data.m_color_step;
	//     //     damage_data.m_color.b( ) = (uint8_t)damage_data.m_color_step;
	//     // }
	//     // 
	//     // g_cl.print( "%f %f %u %u %u\n", damage_data.m_time, damage_data.m_color_step, damage_data.m_color.r( ), damage_data.m_color.g( ), damage_data.m_color.b( ) );
	//     
	//     // decrement timer.
	//     damage_data.m_time -= g_csgo.m_globals->m_frametime;
	// }
	// 
	// else
	//     damage_data.m_color = colors::white;

	// render!
	color = g_menu.main.players.offscreen_color.get(); // damage_data.m_color;

	if (player->dormant())
		color.a() = (alpha == 255) ? alpha : alpha / 2;

	g_csgo.m_surface->DrawSetColor(color);
	g_csgo.m_surface->DrawTexturedPolygon(3, verts);

	// g_csgo.m_surface->DrawSetColor( colors::black );
	// g_csgo.m_surface->DrawTexturedPolyLine( 3, verts_outline );

}

bool GetBBox(Player* ent, float mbox[6])
{
	const vec3_t min = ent->m_vecMins();
	const vec3_t max = ent->m_vecMaxs();

	std::array< vec3_t, 8 > points = {

		vec3_t(min.x, min.y, min.z),
		vec3_t(min.x, max.y, min.z),
		vec3_t(max.x, max.y, min.z),
		vec3_t(max.x, min.y, min.z),
		vec3_t(max.x, max.y, max.z),
		vec3_t(min.x, max.y, max.z),
		vec3_t(min.x, min.y, max.z),
		vec3_t(max.x, min.y, max.z)

	};

	const auto& transformed = ent->coord_frame();

	auto left = std::numeric_limits< float >::max();
	auto top = std::numeric_limits< float >::max();
	auto right = std::numeric_limits< float >::lowest();
	auto bottom = std::numeric_limits< float >::lowest();

	std::array< vec2_t, 8 > screen;

	for (std::size_t i = 0; i < 8; i++) {

		if (!render::WorldToScreen(math::vector_transform(points.at(i), transformed), screen.at(i)))
			return false;

		left = std::min(left, screen.at(i).x);
		top = std::min(top, screen.at(i).y);
		right = std::max(right, screen.at(i).x);
		bottom = std::max(bottom, screen.at(i).y);

	}

	mbox[0] = left;
	mbox[1] = top;
	mbox[2] = right;
	mbox[3] = bottom;
	mbox[4] = right - left;
	mbox[5] = bottom - top;

	return true;
}

std::string RelWepName(std::string name) {
	if (name == "p2000")
		return "p2k";
	else if (name == "glock-18")
		return "glonk";
	else if (name == "r8 revolver")
		return "r8";
	else if (name == "ssg 08")
		return "scout";
	else if (name == "desert eagle")
		return "deagle";
	else if (name == "scar-20")
		return "scar";
	else if (name == "dual berettas")
		return "dualies";
	return name;
};


void Visuals::DrawPlayer(Player* player) {
	constexpr float MAX_DORMANT_TIME = 10.f;
	constexpr float DORMANT_FADE_TIME = MAX_DORMANT_TIME / 2.f;

	player_info_t info;
	Color		  color;

	vec3_t origin, mins, maxs;
	vec2_t bottom, top;

	// get player index.
	int index = player->index();

	// get reference to array variable.
	float& opacity = m_opacities[index - 1];
	bool& draw = m_draw[index - 1];

	// opacity should reach 1 in 300 milliseconds.
	constexpr int frequency = 1.f / 0.3f;

	// the increment / decrement per frame.
	float step = frequency * g_csgo.m_globals->m_frametime;

	// is player enemy.
	bool enemy = player->enemy(g_cl.m_local);
	if (!enemy)
		return;

	bool dormant = player->dormant();

	if (g_menu.main.visuals.enemy_radar.get() && enemy && !dormant)
		player->m_bSpotted() = true;

	// we can draw this player again.
	if (!dormant)
		draw = true;

	if (!draw)
		return;

	// if non-dormant	-> increment
	// if dormant		-> decrement
	dormant ? opacity -= step : opacity += step;

	// is dormant esp enabled for this player.
	bool dormant_esp = enemy;

	// clamp the opacity.
	math::clamp(opacity, 0.f, 1.f);
	if (!opacity && !dormant_esp)
		return;

	// stay for x seconds max.
	float dt = g_csgo.m_globals->m_curtime - player->m_flSimulationTime();
	if (dormant && dt > MAX_DORMANT_TIME)
		return;

	float boundingb[6];
	bool drawbox = true;
	drawbox = GetBBox(player, boundingb);

	int center_top = boundingb[0] + (boundingb[4] / 2);

	// calculate alpha channels.
	int alpha = (int)(255.f * opacity);
	int low_alpha = (int)(179.f * opacity);

	if (dormant && dormant_esp) {
		alpha = 112;
		low_alpha = 80;

		// fade.
		if (dt > DORMANT_FADE_TIME) {
			// for how long have we been fading?
			float faded = (dt - DORMANT_FADE_TIME);
			float scale = 1.f - (faded / DORMANT_FADE_TIME);

			alpha *= scale;
			low_alpha *= scale;
		}
	}

	// override alpha.
	color.a() = alpha;

	// get player info.
	if (!g_csgo.m_engine->GetPlayerInfo(index, &info))
		return;

	// run offscreen ESP.
	if ((!render::OnScreen(vec2_t(boundingb[0], boundingb[1])) && !render::OnScreen(vec2_t(boundingb[2], boundingb[3]))) || !drawbox) {
		OffScreen(player, alpha);
		return;
	}
	Rect		  box;

	if (!GetPlayerBoxRect(player, box)) {
		// OffScreen( player );
		return;
	}

	int bottom_y = boundingb[3];

	// DebugAimbotPoints( player );

	if (g_menu.main.players.skeleton.get() && !dormant)
		DrawSkeleton(player, alpha);


	bool bone_esp = (enemy && g_menu.main.players.skeleton.get());
	if (bone_esp)
		DrawSkeleton(player, alpha);

	// is box esp enabled for this player.
	bool box_esp = (enemy && g_menu.main.players.box.get());

	// render box if specified.
	if (box_esp)
		render::rect_outlined(box.x, box.y, box.w, box.h, dormant ? Color(70, 70, 70, 196) : g_menu.main.players.box_enemy.get(), {10, 10, 10, low_alpha});

	// is name esp enabled for this player.
	bool name_esp = (enemy && g_menu.main.players.name.get());

	// draw name.
	if (name_esp) {
		// fix retards with their namechange meme 
		// the point of this is overflowing unicode compares with hardcoded buffers, good hvh strat
		std::string name{ std::string(info.m_name).substr(0, 24) };

		Color clr = { 255,255,255,low_alpha };

		render::esp.string(box.x + box.w / 2, box.y - render::esp.m_size.m_height, clr, name, render::ALIGN_CENTER);
	}

	if (enemy) {
		AimPlayer* data = &g_aimbot.m_players[player->index() - 1];


		// make sure everything is valid.
		if (data && data->m_records.size()) {
			// grab lag record.
			LagRecord* current = data->m_records.front().get();



			if (current && current->valid()) {

				std::string resolver_text = "none";

				switch (current->m_mode)
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
				}

				render::esp.string(box.x + box.w / 2, box.y - render::esp.m_size.m_height*2, Color(220, 220, 220, low_alpha), resolver_text, render::ALIGN_CENTER);
			}
		}
	}

	// is health esp enabled for this player.
	bool health_esp = (enemy && g_menu.main.players.health.get());

	if (health_esp) {
		int y = box.y + 1;
		int h = box.h - 2;

		// retarded servers that go above 100 hp..
		int hp = std::min(100, player->m_iHealth());

		// calculate hp bar color.
		int r = std::min((510 * (100 - hp)) / 100, 255);
		int g = std::min((510 * hp) / 100, 255);

		// get hp bar height.
		int fill = (int)std::round(hp * h / 100.f);

		// render background.
		render::rect_filled(box.x - 6, y - 1, 4, h + 2, { 10, 10, 10, low_alpha });

		// render actual bar.
		render::rect(box.x - 5, y + h - fill, 2, fill, { r, g, 0, alpha });

		// if hp is below max, draw a string.
		if (hp < 100)
			render::esp_small.string(box.x - 5, y + (h - fill) - 5, { 255, 255, 255, low_alpha }, std::to_string(hp), render::ALIGN_CENTER);
	}


	// draw flags.
	{
		std::vector< std::pair< std::string, Color > > flags;

		auto items = g_menu.main.players.flags_enemy.GetActiveIndices();
		if (!enemy) return;

		for (auto it = items.begin(); it != items.end(); ++it) {

			// money.
			if (*it == 0)
			{
				flags.push_back({ tfm::format(XOR("$%i"), player->m_iAccount()), {  155, 210, 100, low_alpha } });
			}

			// armor.
			if (*it == 1) {
				// helmet and kevlar.
				if (player->m_bHasHelmet() && player->m_ArmorValue() > 0)
					flags.push_back({ XOR("HK"), { 255, 255, 255, low_alpha } });

				// only kevlar.
				else if (player->m_ArmorValue() > 0)
					flags.push_back({ XOR("K"), { 255, 255, 255, low_alpha } });
			}

			// scoped.
			if (*it == 2 && player->m_bIsScoped())
				flags.push_back({ XOR("ZOOM"), { 0, 175, 255, low_alpha } });

			// flashed.
			if (*it == 3 && player->m_flFlashBangTime() > 0.f)
				flags.push_back({ XOR("BLIND"), { 0, 175, 255, low_alpha } });

			// reload.
			if (*it == 4) {
				// get ptr to layer 1.
				C_AnimationLayer* layer1 = &player->m_AnimOverlay()[1];

				// check if reload animation is going on.
				if (layer1->m_weight != 0.f && player->GetSequenceActivity(layer1->m_sequence) == 967 /* ACT_CSGO_RELOAD */)
					flags.push_back({ XOR("R"), { 0, 175, 255, low_alpha } });
			}

			// bomb.
			if (*it == 5 && player->HasC4())
				flags.push_back({ XOR("B"), { 255, 0, 0, low_alpha } });



		}
		/*
		// NOTE FROM NITRO TO DEX -> stop removing my iterator loops, i do it so i dont have to check the size of the vector
		// with range loops u do that to do that.
		AimPlayer* data = &g_aimbot.m_players[player->index() - 1];
		if (data && data->m_records.size()) {
			//flags.push_back({ std::to_string(data->m_update_time), {255, 255, 255, low_alpha} });
			// ok, lets see if theyre crazy dopium flick :scream:
			if (data->m_records.front() && data->m_records.front().get()->m_retard && data->m_records.front().get()->valid()) {
				// ok yes we do have a side. hmm
				flags.push_back({ XOR("RETARD"), { 255, 0, 0, low_alpha} });

			}
		}*/

		int  offset{ 0 };

		// draw lby update bar.
		if (enemy && g_menu.main.players.lby_update.get()) {
			AimPlayer* data = &g_aimbot.m_players[player->index() - 1];




			// make sure everything is valid.
			if (data && data->m_records.size()) {
				// grab lag record.
				LagRecord* current = data->m_records.front().get();

				if (current) {
					if (!(current->m_velocity.length_2d() > 0.1 && !current->m_fake_walk) && data->m_body_index <= 3) {
						// calculate box width
						float cycle = std::clamp<float>(data->m_body_update - current->m_anim_time, 0.f, 1.1f);
						float width = (box.w * cycle) / 1.1f;

						if (width > 0.f) {
							// draw.
							render::rect_filled(box.x, box.y + box.h + 2, box.w, 4, { 10, 10, 10, low_alpha });

							Color clr = g_menu.main.players.lby_update_color.get();
							clr.a() = alpha;
							render::rect(box.x + 1, box.y + box.h + 3, width, 2, clr);

							// move down the offset to make room for the next bar.
							offset += 5;
						}
					}
				}
			}
		}

		// iterate flags.
		for (size_t i{ }; i < flags.size(); ++i) {
			// get flag job (pair).
			const auto& f = flags[i];

			int offset = i * (render::esp_small.m_size.m_height + 1);

			// draw flag.
			render::esp_small.string(box.x + box.w + 2, box.y + offset, f.second, f.first);
		}

	}

	// draw bottom bars.
	int  offset{ 0 };

	// draw lby update bar.
	if (enemy && g_menu.main.players.lby_update.get()) {
		AimPlayer* data = &g_aimbot.m_players[player->index() - 1];

		// make sure everything is valid.
		if (data && data->m_moved && data->m_records.size()) {
			// grab lag record.
			LagRecord* current = data->m_records.front().get();

			if (current) {
				if (!(current->m_velocity.length_2d() > 0.1 && !current->m_fake_walk) && data->m_body_index <= 3) {
					// calculate box width
					float cycle = std::clamp<float>(data->m_body_update - current->m_anim_time, 0.f, 1.0f);
					float width = (box.w * cycle) / 1.1f;

					if (width > 0.f) {
						// draw.
						render::rect_filled(box.x, box.y + box.h + 2, box.w, 4, { 10, 10, 10, low_alpha });

						Color clr = g_menu.main.players.lby_update_color.get();
						clr.a() = alpha;
						render::rect(box.x + 1, box.y + box.h + 3, width, 2, clr);

						// move down the offset to make room for the next bar.
						offset += 5;
					}
				}
			}
		}
	}
	// draw weapon.
	if ((enemy && g_menu.main.players.weapon.get())) {
		Weapon* weapon = player->GetActiveWeapon();
		if (weapon) {
			WeaponInfo* data = weapon->GetWpnData();
			if (data) {
				int bar;
				float scale;

				// the maxclip1 in the weaponinfo
				int max = data->m_max_clip1;
				int current = weapon->m_iClip1();

				C_AnimationLayer* layer1 = &player->m_AnimOverlay()[1];

				// set reload state.
				bool reload = (layer1->m_weight != 0.f) && (player->GetSequenceActivity(layer1->m_sequence) == 967);

				// ammo bar.
				if (max != -1 && g_menu.main.players.ammo.get()) {
					// check for reload.
					if (reload)
						scale = layer1->m_cycle;

					// not reloading.
					// make the division of 2 ints produce a float instead of another int.
					else
						scale = (float)current / max;

					// relative to bar.
					bar = (int)std::round((box.w - 2) * scale);

					// draw.
					render::rect_filled(box.x, box.y + box.h + 2 + offset, box.w, 4, { 10, 10, 10, low_alpha });

					Color clr = g_menu.main.players.ammo_color.get();
					clr.a() = alpha;
					render::rect(box.x + 1, box.y + box.h + 3 + offset, bar, 2, clr);

					// less then a 5th of the bullets left.
					if (current <= (int)std::round(max / 5) && !reload)
						render::esp_small.string(box.x + bar, box.y + box.h + offset, { 255, 255, 255, low_alpha }, std::to_string(current), render::ALIGN_CENTER);

					offset += 6;
				}

				// text.
				if (g_menu.main.players.weapon_mode.get() == 0) {
					// construct std::string instance of localized weapon name.
					std::string name{ weapon->GetLocalizedName() };

					// smallfonts needs upper case.
					std::transform(name.begin(), name.end(), name.begin(), ::toupper);

					render::esp_small.string(box.x + box.w / 2, box.y + box.h + offset, { 255, 255, 255, low_alpha }, name, render::ALIGN_CENTER);
				}

				// icons.
				else if (g_menu.main.players.weapon_mode.get() == 1) {
					// icons are super fat..
					// move them back up.
					offset -= 5;

					std::string icon = tfm::format(XOR("%c"), m_weapon_icons[weapon->m_iItemDefinitionIndex()]);
					render::cs.string(box.x + box.w / 2, box.y + box.h + offset, { 255, 255, 255, low_alpha }, icon, render::ALIGN_CENTER);
				}
			}
		}
	}
}

void DrawBombESP(int x, int y, float time, int damage) {
	if (time < 0)
		return;

	std::string time_str, damage_str;
	int start_w = x - 50;
	float bar_percent = time / 40.f;
	Color bar_clr = Color(120, 255, 0, 170);

	time_str = tfm::format(XOR("%.2f"), time);


	damage_str = XOR("Bomb planted"); // L8R ON ADD 
	if (g_cl.m_local->alive() && damage > 0)
		damage_str += tfm::format(XOR(" - %i dmg"), damage);

	if (time < 10.f)
		bar_clr = Color(252, 155, 52, 170);
	if (time < 5.f)
		bar_clr = Color(255, 30, 30, 170);


	render::bold.string(x, y - 22, { 255, 255, 255, 255 }, damage_str, render::ALIGN_CENTER);
	render::rect_filled(start_w, y - 10, 100, 16, { 0, 0, 0, 100 });
	render::rect_filled(start_w + 2, y - 8, 96 * bar_percent, 12, bar_clr);
	render::bold.string(x, y - 9, { 255, 255, 255, 255 }, time_str, render::ALIGN_CENTER);


}

void Visuals::DrawPlantedC4() {
	bool        is_visible;
	float       explode_time_diff, dist, range_damage;
	vec3_t      dst, to_target;
	int         final_damage;
	std::string time_str, damage_str;
	Color       damage_color;
	vec2_t      screen_pos;

	static auto scale_damage = [](float damage, int armor_value) {
		float new_damage, armor;

		if (armor_value > 0) {
			new_damage = damage * 0.5f;
			armor = (damage - new_damage) * 0.5f;

			if (armor > (float)armor_value) {
				armor = (float)armor_value * 2.f;
				new_damage = damage - armor;
			}

			damage = new_damage;
		}

		return std::max(0, (int)std::floor(damage));
	};

	// store menu vars for later.
	if (!g_menu.main.visuals.planted_c4.get())
		return;

	// bomb not currently active, do nothing.
	if (!m_c4_planted)
		return;

	// calculate bomb damage.
	// references:
	//     https://github.com/VSES/SourceEngine2007/blob/43a5c90a5ada1e69ca044595383be67f40b33c61/se2007/game/shared/cstrike/weapon_c4.cpp#L271
	//     https://github.com/VSES/SourceEngine2007/blob/43a5c90a5ada1e69ca044595383be67f40b33c61/se2007/game/shared/cstrike/weapon_c4.cpp#L437
	//     https://github.com/ValveSoftware/source-sdk-2013/blob/master/sp/src/game/shared/sdk/sdk_gamerules.cpp#L173
	{
		// get our distance to the bomb.
		// todo - dex; is dst right? might need to reverse CBasePlayer::BodyTarget...
		dst = g_cl.m_local->WorldSpaceCenter();
		to_target = m_planted_c4_explosion_origin - dst;
		dist = to_target.length();

		// calculate the bomb damage based on our distance to the C4's explosion.
		range_damage = m_planted_c4_damage * std::exp((dist * dist) / ((m_planted_c4_radius_scaled * -2.f) * m_planted_c4_radius_scaled));

		// now finally, scale the damage based on our armor (if we have any).
		final_damage = scale_damage(range_damage, g_cl.m_local->m_ArmorValue());
	}

	// m_flC4Blow is set to gpGlobals->curtime + m_flTimerLength inside CPlantedC4.
	explode_time_diff = m_planted_c4_explode_time - g_csgo.m_globals->m_curtime;

	// get formatted strings for bomb.
	time_str = tfm::format(XOR("%.2f"), explode_time_diff);
	damage_str = tfm::format(XOR("%i"), final_damage);

	// get damage color.
	damage_color = (final_damage < g_cl.m_local->m_iHealth()) ? colors::white : colors::red;

	// finally do all of our rendering.
	is_visible = render::WorldToScreen(m_planted_c4_explosion_origin, screen_pos);


	if (render::OnScreen(screen_pos)) {
		// 3D
		DrawBombESP(screen_pos.x, screen_pos.y, explode_time_diff, final_damage);
	}
	else {
		DrawBombESP(g_cl.m_width / 2, 100, explode_time_diff, final_damage);
		// 2D
	}




}

bool Visuals::GetPlayerBoxRect(Player* player, Rect& box) {
	//matrix3x4_t& tran_frame = player->coord_frame();

	//const vec3_t min = player->m_vecMins();
	//const vec3_t max = player->m_vecMaxs();

	//vec3_t screen_boxes[8];

	//vec3_t points[] = {
	//	vec3_t(min.x, min.y, min.z),
	//	vec3_t(min.x, max.y, min.z),
	//	vec3_t(max.x, max.y, min.z),
	//	vec3_t(max.x, min.y, min.z),
	//	vec3_t(max.x, max.y, max.z),
	//	vec3_t(min.x, max.y, max.z),
	//	vec3_t(min.x, min.y, max.z),
	//	vec3_t(max.x, min.y, max.z)
	//};

	//for (int i = 0; i <= 7; i++)
	//	if (!render::world_to_screen(math::vector_transform(points[i], tran_frame), screen_boxes[i]))
	//		return false;

	//vec3_t box_array[] = {
	//	screen_boxes[3], // fl
	//	screen_boxes[5], // br
	//	screen_boxes[0], // bl
	//	screen_boxes[4], // fr
	//	screen_boxes[2], // fr
	//	screen_boxes[1], // br
	//	screen_boxes[6], // bl
	//	screen_boxes[7] // fl
	//};

	//float left = screen_boxes[3].x, bottom = screen_boxes[3].y, right = screen_boxes[3].x, top = screen_boxes[3].y;

	//for (int i = 0; i <= 7; i++) {
	//	if (left > box_array[i].x)
	//		left = box_array[i].x;

	//	if (bottom < box_array[i].y)
	//		bottom = box_array[i].y;

	//	if (right < box_array[i].x)
	//		right = box_array[i].x;

	//	if (top > box_array[i].y)
	//		top = box_array[i].y;
	//}

	//box.x = static_cast<int>(left);
	//box.y = static_cast<int>(top);
	//box.w = static_cast<int>(right) - static_cast<int>(left);
	//box.h = static_cast<int>(bottom) - static_cast<int>(top);

	//return true;

	vec3_t origin, mins, maxs;
	vec2_t bottom, top;

	// get interpolated origin.
	origin = player->GetAbsOrigin();

	// get hitbox bounds.
	player->ComputeHitboxSurroundingBox(&mins, &maxs);

	// correct x and y coordinates.
	mins = { origin.x, origin.y, mins.z };
	maxs = { origin.x, origin.y, maxs.z + 8.f };

	if (!render::WorldToScreen(mins, bottom) || !render::WorldToScreen(maxs, top))
		return false;

	box.h = bottom.y - top.y;
	box.w = box.h / 2.f;
	box.x = bottom.x - (box.w / 2.f);
	box.y = bottom.y - box.h;

	return true;
}

void Visuals::DrawHistorySkeleton(Player* player, int opacity) {
	const model_t* model;
	studiohdr_t* hdr;
	mstudiobone_t* bone;
	AimPlayer* data;
	LagRecord* record;
	int           parent;
	vec3_t        bone_pos, parent_pos;
	vec2_t        bone_pos_screen, parent_pos_screen;

	if (!g_menu.main.misc.fake_latency.get())
		return;

	// get player's model.
	model = player->GetModel();
	if (!model)
		return;

	// get studio model.
	hdr = g_csgo.m_model_info->GetStudioModel(model);
	if (!hdr)
		return;

	data = &g_aimbot.m_players[player->index() - 1];
	if (!data)
		return;

	record = g_resolver.FindLastRecord(data);
	if (!record)
		return;

	for (int i{ }; i < hdr->m_num_bones; ++i) {
		// get bone.
		bone = hdr->GetBone(i);
		if (!bone || !(bone->m_flags & BONE_USED_BY_HITBOX))
			continue;

		// get parent bone.
		parent = bone->m_parent;
		if (parent == -1)
			continue;

		// resolve main bone and parent bone positions.
		record->m_bones->get_bone(bone_pos, i);
		record->m_bones->get_bone(parent_pos, parent);

		Color clr = player->enemy(g_cl.m_local) ? g_menu.main.players.skeleton_enemy.get() : Color(0, 0, 0);
		clr.a() = opacity;

		// world to screen both the bone parent bone then draw.
		if (render::WorldToScreen(bone_pos, bone_pos_screen) && render::WorldToScreen(parent_pos, parent_pos_screen))
			render::line(bone_pos_screen.x, bone_pos_screen.y, parent_pos_screen.x, parent_pos_screen.y, clr);
	}
}

void Visuals::DrawSkeleton(Player* player, int opacity) {
	const model_t* model;
	studiohdr_t* hdr;
	mstudiobone_t* bone;
	int           parent;
	BoneArray     matrix[128];
	vec3_t        bone_pos, parent_pos;
	vec2_t        bone_pos_screen, parent_pos_screen;

	// get player's model.
	model = player->GetModel();
	if (!model)
		return;

	// get studio model.
	hdr = g_csgo.m_model_info->GetStudioModel(model);
	if (!hdr)
		return;

	// get bone matrix.
	if (!player->SetupBones(matrix, 128, BONE_USED_BY_ANYTHING, g_csgo.m_globals->m_curtime))
		return;

	for (int i{ }; i < hdr->m_num_bones; ++i) {
		// get bone.
		bone = hdr->GetBone(i);
		if (!bone || !(bone->m_flags & BONE_USED_BY_HITBOX))
			continue;

		// get parent bone.
		parent = bone->m_parent;
		if (parent == -1)
			continue;

		// resolve main bone and parent bone positions.
		matrix->get_bone(bone_pos, i);
		matrix->get_bone(parent_pos, parent);

		Color clr = g_menu.main.players.skeleton_enemy.get();
		clr.a() = opacity;

		// world to screen both the bone parent bone then draw.
		if (render::WorldToScreen(bone_pos, bone_pos_screen) && render::WorldToScreen(parent_pos, parent_pos_screen))
			render::line(bone_pos_screen.x, bone_pos_screen.y, parent_pos_screen.x, parent_pos_screen.y, clr);
	}
}

void Visuals::RenderGlow() {
	Color   color;
	Player* player;

	if (!g_cl.m_local)
		return;

	if (!g_csgo.m_glow->m_object_definitions.Count())
		return;

	float blend = g_menu.main.players.glow_blend.get() / 100.f;

	for (int i{ }; i < g_csgo.m_glow->m_object_definitions.Count(); ++i) {
		GlowObjectDefinition_t* obj = &g_csgo.m_glow->m_object_definitions[i];

		// skip non-players.
		if (!obj->m_entity || !obj->m_entity->IsPlayer())
			continue;

		// get player ptr.
		player = obj->m_entity->as< Player* >();

		if (player->m_bIsLocalPlayer())
			continue;

		// get reference to array variable.
		float& opacity = m_opacities[player->index() - 1];

		bool enemy = player->enemy(g_cl.m_local);

		if (enemy && !g_menu.main.players.glow.get(0))
			continue;

		if (!enemy)
			continue;

		color = g_menu.main.players.glow_enemy.get();

		obj->m_render_occluded = true;
		obj->m_render_unoccluded = false;
		obj->m_render_full_bloom = false;
		obj->m_color = { (float)color.r() / 255.f, (float)color.g() / 255.f, (float)color.b() / 255.f };
		obj->m_alpha = opacity * blend;
	}
}


void Visuals::AddMatrix(Player* player, matrix3x4_t* bones) {
	
}

void Visuals::override_material(bool ignoreZ, bool use_env, Color& color, IMaterial* material) {
	material->SetFlag(MATERIAL_VAR_IGNOREZ, ignoreZ);
	material->IncrementReferenceCount();

	bool found;
	auto var = material->FindVar("$envmaptint", &found);

	if (found)
		var->set_vec_value(color.r(), color.g(), color.b());

	g_csgo.m_studio_render->ForcedMaterialOverride(material);
}

void Visuals::on_post_screen_effects() {
	if (!g_cl.m_processing)
		return;
	static ConVar* strength = g_csgo.m_cvar->FindVar(HASH("mat_motion_blur_strength"));
	static ConVar* enabled = g_csgo.m_cvar->FindVar(HASH("mat_motion_blur_enabled"));
	strength->SetValue(g_menu.main.misc.motion_blur.get());
	enabled->SetValue(strength->GetInt() != 0);
}

void Visuals::DrawHitboxMatrix(LagRecord* record, Color col, float time) {
	const model_t* model;
	studiohdr_t* hdr;
	mstudiohitboxset_t* set;
	mstudiobbox_t* bbox;
	vec3_t             mins, maxs, origin;
	ang_t			   angle;

	model = record->m_player->GetModel();
	if (!model)
		return;

	hdr = g_csgo.m_model_info->GetStudioModel(model);
	if (!hdr)
		return;

	set = hdr->GetHitboxSet(record->m_player->m_nHitboxSet());
	if (!set)
		return;

	for (int i{ }; i < set->m_hitboxes; ++i) {
		bbox = set->GetHitbox(i);
		if (!bbox)
			continue;

		// bbox.
		if (bbox->m_radius <= 0.f) {
			// https://developer.valvesoftware.com/wiki/Rotation_Tutorial

			// convert rotation angle to a matrix.
			matrix3x4_t rot_matrix;
			g_csgo.AngleMatrix(bbox->m_angle, rot_matrix);

			// apply the rotation to the entity input space (local).
			matrix3x4_t matrix;
			math::ConcatTransforms(record->m_bones[bbox->m_bone], rot_matrix, matrix);

			// extract the compound rotation as an angle.
			ang_t bbox_angle;
			math::MatrixAngles(matrix, bbox_angle);

			// extract hitbox origin.
			vec3_t origin = matrix.GetOrigin();

			// draw box.
			//g_csgo.m_debug_overlay->AddBoxOverlay( origin, bbox->m_mins, bbox->m_maxs, bbox_angle, col.r( ), col.g( ), col.b( ), 0, time );
		}

		// capsule.
		else {
			// NOTE; the angle for capsules is always 0.f, 0.f, 0.f.

			// create a rotation matrix.
			matrix3x4_t matrix;
			g_csgo.AngleMatrix(bbox->m_angle, matrix);

			// apply the rotation matrix to the entity output space (world).
			math::ConcatTransforms(record->m_bones[bbox->m_bone], matrix, matrix);

			// get world positions from new matrix.
			math::VectorTransform(bbox->m_mins, matrix, mins);
			math::VectorTransform(bbox->m_maxs, matrix, maxs);

			//g_csgo.m_debug_overlay->AddCapsuleOverlay( mins, maxs, bbox->m_radius, col.r( ), col.g( ), col.b( ), col.a( ), time, 0, 0 );
		}
	}
}

void Visuals::DrawBeams() {
	size_t     impact_count;
	float      curtime, dist;
	bool       is_final_impact;
	vec3_t     va_fwd, start, dir, end;
	BeamInfo_t beam_info;
	Beam_t* beam;

	if (!g_cl.m_local)
		return;

	if (!g_menu.main.visuals.impact_beams.get())
		return;

	auto vis_impacts = &g_shots.m_vis_impacts;

	// the local player is dead, clear impacts.
	if (!g_cl.m_processing) {
		if (!vis_impacts->empty())
			vis_impacts->clear();
	}

	else {
		impact_count = vis_impacts->size();
		if (!impact_count)
			return;

		curtime = game::TICKS_TO_TIME(g_cl.m_local->m_nTickBase());

		for (size_t i{ impact_count }; i-- > 0; ) {
			auto impact = &vis_impacts->operator[ ](i);
			if (!impact)
				continue;

			// impact is too old, erase it.
			if (std::abs(curtime - game::TICKS_TO_TIME(impact->m_tickbase)) > g_menu.main.visuals.impact_beams_time.get()) {
				vis_impacts->erase(vis_impacts->begin() + i);

				continue;
			}

			// already rendering this impact, skip over it.
			if (impact->m_ignore)
				continue;

			// is this the final impact?
			// last impact in the vector, it's the final impact.
			if (i == (impact_count - 1))
				is_final_impact = true;

			// the current impact's tickbase is different than the next, it's the final impact.
			else if ((i + 1) < impact_count && impact->m_tickbase != vis_impacts->operator[ ](i + 1).m_tickbase)
				is_final_impact = true;

			else
				is_final_impact = false;

			// is this the final impact?
			// is_final_impact = ( ( i == ( impact_count - 1 ) ) || ( impact->m_tickbase != vis_impacts->at( i + 1 ).m_tickbase ) );

			if (is_final_impact) {
				// calculate start and end position for beam.
				start = impact->m_shoot_pos;

				dir = (impact->m_impact_pos - start).normalized();
				dist = (impact->m_impact_pos - start).length();

				end = start + (dir * dist);

				// setup beam info.
				// note - dex; possible beam models: sprites/physbeam.vmt | sprites/white.vmt
				beam_info.m_vecStart = start;
				beam_info.m_vecEnd = end;
				beam_info.m_nModelIndex = g_csgo.m_model_info->GetModelIndex(XOR("sprites/white.vmt"));
				beam_info.m_pszModelName = XOR("sprites/white.vmt");
				beam_info.m_flHaloScale = 0.f;
				beam_info.m_flLife = g_menu.main.visuals.impact_beams_time.get();
				beam_info.m_flWidth = .6f;
				beam_info.m_flEndWidth = .75f;
				beam_info.m_flFadeLength = 3.0f;
				beam_info.m_flAmplitude = 0.f;//beam 'jitter'.
				beam_info.m_flBrightness = 255.f;
				beam_info.m_flSpeed = 1.f;  // seems to control how fast the 'scrolling' of beam is... once fully spawned.
				beam_info.m_nStartFrame = 1;
				beam_info.m_flFrameRate = 60;
				beam_info.m_nSegments = 4;     // controls how much of the beam is 'split up', usually makes m_flAmplitude and m_flSpeed much more noticeable.
				beam_info.m_bRenderable = true;  // must be true or you won't see the beam.
				beam_info.m_nFlags = 0x00000040 | 0x00000004 | 0x00000001 | 0x00008000;

				beam_info.m_flRed = g_menu.main.visuals.impact_beams_color.get().r();
				beam_info.m_flGreen = g_menu.main.visuals.impact_beams_color.get().g();
				beam_info.m_flBlue = g_menu.main.visuals.impact_beams_color.get().b();

				// attempt to render the beam.
				beam = game::CreateGenericBeam(beam_info);
				if (beam) {
					g_csgo.m_beams->DrawBeam(beam);

					// we only want to render a beam for this impact once.
					impact->m_ignore = true;
				}
			}
		}
	}
}

void Visuals::DebugAimbotPoints(Player* player) {
	std::vector< vec3_t > p2{ };

	AimPlayer* data = &g_aimbot.m_players.at(player->index() - 1);
	if (!data || data->m_records.empty())
		return;

	LagRecord* front = data->m_records.front().get();
	if (!front || front->dormant())
		return;

	// get bone matrix.
	BoneArray matrix[128];
	if (!g_bones.setup(player, matrix, front))
		return;

	data->SetupHitboxes(front, false);
	if (data->m_hitboxes.empty())
		return;

	for (const auto& it : data->m_hitboxes) {
		std::vector< vec3_t > p1{ };

		if (!data->SetupHitboxPoints(front, matrix, it.m_index, p1))
			continue;

		for (auto& p : p1)
			p2.push_back(p);
	}

	if (p2.empty())
		return;

	for (auto& p : p2) {
		vec2_t screen;

		if (render::WorldToScreen(p, screen))
			render::rect_filled(screen.x, screen.y, 2, 2, { 0, 255, 255, 255 });
	}
}

void Visuals::Indicators()
{
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_local->alive())
		return;

	if (!g_menu.main.visuals.keybinds.get())
		return;

	Color color = g_gui.m_color;
	struct Indicator_t { std::string text; std::string mode; std::string info; };
	std::vector< Indicator_t > indicators{ };

	if (g_input.GetKeyState(g_menu.main.aimbot.baim_key.get()))
	{
		Indicator_t ind{};
		ind.text = XOR("Force Body Aim");
		ind.mode = XOR("[Holding]");
		indicators.push_back(ind);
	}

	if (g_input.GetKeyState(g_menu.main.aimbot.baim_key.get()))
	{
		Indicator_t ind{};
		ind.text = XOR("Force Body Aim");
		ind.mode = XOR("[Holding]");
		indicators.push_back(ind);
	}
	if (g_visuals.m_thirdperson)
	{
		Indicator_t ind{};
		ind.text = XOR("Third person");
		ind.mode = XOR("[Toggled]");
		indicators.push_back(ind);
	}

	if (g_tickshift.m_double_tap) {
		Indicator_t ind{};
		ind.text = XOR("Double Tap");
		ind.mode = XOR("[Toggled]");
		indicators.push_back(ind);

	}

      /* if (g_menu.main.antiaim.fake_flick.get()) {
		Indicator_t ind{};
		ind.text = XOR("Fake Flick");
		ind.mode = XOR("[Toggled]");
		indicators.push_back(ind);

	}*/


	if (g_input.GetKeyState(g_menu.main.antiaim.lag_exploit.get())) {
		Indicator_t ind{};
		ind.text = XOR("Lag Exploit");
		ind.mode = XOR("[Hold]");
		indicators.push_back(ind);

	}


	//if (g_input.GetKeyState(g_menu.main.aimbot.min_key.get()))
	//{
	//    Indicator_t ind{};
	//    ind.text = XOR("Damage Override");
	//    ind.mode = XOR("[Holding]");
	//    indicators.push_back(ind);
	//}

	if (g_aimbot.m_fake_latency)
	{
		Indicator_t ind{};
		ind.text = XOR("Fake Latency");
		ind.mode = XOR("[Toggled]");
		indicators.push_back(ind);
	}

	if (g_input.GetKeyState(g_menu.main.misc.fakewalk.get()))
	{
		Indicator_t ind{};
		ind.text = XOR("Fake Walk");
		ind.mode = XOR("[Holding]");
		indicators.push_back(ind);
	}
	Color icolor = g_gui.m_color;
	render::rect_outlined(10, g_cl.m_height / 2 + 10, 150, 17, { icolor.r(),icolor.g(),icolor.b(), 90 }, { icolor.r(),icolor.g(),icolor.b(), 20 });
	render::gradient(10, g_cl.m_height / 2 + 10, 150, 17, { icolor.r(),icolor.g(),icolor.b(), 90 }, { icolor.r(),icolor.g(),icolor.b(), 20 }, true);

	render::norm.string(85, g_cl.m_height / 2 + 12, colors::white, XOR("Bindings"), render::ALIGN_CENTER);

	if (indicators.empty())
		return;

	render::rect_filled(10, g_cl.m_height / 2 + 27, 150, (indicators.size() * 15), { icolor.r(),icolor.g(),icolor.b(), 5 });

	for (size_t i{ }; i < indicators.size(); ++i) {
		auto& indicator = indicators[i];
		auto size = render::norm.size(indicator.text);

		render::norm.string(12, (g_cl.m_height / 2 + 27) + (i * 15), colors::white, indicator.text, render::ALIGN_LEFT);
		render::norm.string(135, (g_cl.m_height / 2 + 27) + (i * 15), color, indicator.mode, render::ALIGN_CENTER);
	}
}
