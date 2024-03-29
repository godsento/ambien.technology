#include "includes.h"

extern matrix3x4_t fake_matrix[128];

inline bool isHand(std::string& modelName) {
	if ((modelName.find("arms") != std::string::npos || modelName.find("v_models") != std::string::npos) && !(modelName.find("uid") != std::string::npos || modelName.find("stattrack") != std::string::npos)) {
		return true;
	}

	return false;
}

inline bool isWeapon(std::string& modelName) {
	if ((modelName.find("v_") != std::string::npos || modelName.find("uid") != std::string::npos || modelName.find("stattrack") != std::string::npos) && modelName.find("arms") == std::string::npos) {
		return true;
	}

	return false;
}


void Hooks::DrawModelExecute(uintptr_t ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* bone) {
	Chams::model_type_t type = g_chams.GetModelType(info);

	if (isWeapon(std::string(info.m_model->m_name)) && g_menu.main.players.chams_weapon.get()) {
		Color C = g_menu.main.players.chams_weapon_col.get();
		g_chams.SetAlpha(1);
		g_chams.SetupMaterial(g_chams.m_materials[g_menu.main.players.chams_weapon_mat.get()], g_menu.main.players.chams_weapon_col.get(), false);
		g_hooks.m_model_render.GetOldMethod< Hooks::DrawModelExecute_t >(IVModelRender::DRAWMODELEXECUTE)(this, ctx, state, info, bone);
		g_csgo.m_studio_render->ForcedMaterialOverride(nullptr);
		g_csgo.m_render_view->SetColorModulation(colors::white);
		g_csgo.m_render_view->SetBlend(1.f);
		return;
	}
	else if (isHand(std::string(info.m_model->m_name)) && g_menu.main.players.chams_arms.get()) {
		Color C = g_menu.main.players.chams_arms_col.get();
		g_chams.SetAlpha(1);
		g_chams.SetupMaterial(g_chams.m_materials[g_menu.main.players.chams_arms_mat.get()], g_menu.main.players.chams_arms_col.get(), false);
		g_hooks.m_model_render.GetOldMethod< Hooks::DrawModelExecute_t >(IVModelRender::DRAWMODELEXECUTE)(this, ctx, state, info, bone);
		g_csgo.m_studio_render->ForcedMaterialOverride(nullptr);
		g_csgo.m_render_view->SetColorModulation(colors::white);
		g_csgo.m_render_view->SetBlend(1.f);
		return;
	}

	if (type == Chams::model_type_t::player) {
		if (!g_chams.m_running && !g_csgo.m_studio_render->m_pForcedMaterial) {
			Player* player = g_csgo.m_entlist->GetClientEntity< Player* >(info.m_index);
			if (player && (player->m_bIsLocalPlayer())) {
				if (g_menu.main.players.chams_fake.get() && !g_csgo.m_gamerules->m_bFreezePeriod() && ((g_input.GetKeyState(g_menu.main.misc.fakewalk.get()) && g_cl.m_local->GetGroundEntity() && g_menu.main.misc.fakewalk_mode.get() == 1) || g_hvh.m_desync || (g_menu.main.antiaim.boxhack_breaker.get()))) {
					if (g_menu.main.players.chams_fake_mat.get() == 4)
						g_chams.SetupMaterial(g_chams.m_materials[4], g_menu.main.players.chams_fake_col.get(), true);
					else
						g_chams.SetupMaterial(g_chams.m_materials[g_menu.main.players.chams_fake_mat.get()], g_menu.main.players.chams_fake_col.get(), true);

					g_chams.SetAlpha(g_menu.main.players.chams_fake_blend.get() / 100.f);

					Weapon* weapon = player->GetActiveWeapon();
					if (!weapon)
						return;

					WeaponInfo* data = weapon->GetWpnData();
					if (!data)
						return;

						for (auto& i : fake_matrix) {
							i[0][3] += g_cl.m_local->GetRenderOrigin().x; //-V807
							i[1][3] += g_cl.m_local->GetRenderOrigin().y;
							i[2][3] += g_cl.m_local->GetRenderOrigin().z;
						}
					g_hooks.m_model_render.GetOldMethod< Hooks::DrawModelExecute_t >(IVModelRender::DRAWMODELEXECUTE)(this, ctx, state, info, fake_matrix);

					g_csgo.m_studio_render->ForcedMaterialOverride(nullptr);
					g_csgo.m_render_view->SetColorModulation(colors::white);
					g_csgo.m_render_view->SetBlend(1.f);

						for (auto& i : fake_matrix) {
							i[0][3] -= g_cl.m_local->GetRenderOrigin().x; //-V807
							i[1][3] -= g_cl.m_local->GetRenderOrigin().y;
							i[2][3] -= g_cl.m_local->GetRenderOrigin().z;
						}
				}

				if (g_menu.main.players.chams_local_scope.get() && player->m_bIsScoped()) {
					g_chams.SetAlpha((g_menu.main.players.chams_local_blend.get() / 100.f) * (g_menu.main.players.chams_local_scope_blend.get() / 100.f));
				}
				else if (g_menu.main.players.chams_local.get()) {
					// override blend.
					g_chams.SetAlpha(g_menu.main.players.chams_local_blend.get() / 100.f);

					// set material and color.
					if (g_menu.main.players.chams_local_mat.get() == 4)
						g_chams.SetupMaterial(g_chams.m_materials[4], g_menu.main.players.chams_local_col.get(), false);
					else
						g_chams.SetupMaterial(g_chams.m_materials[g_menu.main.players.chams_local_mat.get()], g_menu.main.players.chams_local_col.get(), false);
				}

				if (g_menu.main.players.chams_local_mat.get() == 5) {
					g_chams.SetAlpha(0.f);
				}

				g_hooks.m_model_render.GetOldMethod< Hooks::DrawModelExecute_t >(IVModelRender::DRAWMODELEXECUTE)(this, ctx, state, info, bone);
				g_csgo.m_studio_render->ForcedMaterialOverride(nullptr);
				g_csgo.m_render_view->SetColorModulation(colors::white);
				g_csgo.m_render_view->SetBlend(1.f);
				return;
			}
		}
	}
	// do chams.
	if (g_chams.DrawModel(ctx, state, info, bone)) {
		g_hooks.m_model_render.GetOldMethod< Hooks::DrawModelExecute_t >(IVModelRender::DRAWMODELEXECUTE)(this, ctx, state, info, bone);
	}
}