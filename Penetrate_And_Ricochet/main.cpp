#include "BulletManager.h"
//#include "utilities/IConsole.h"
//NoGore is unsupported in xNVSE


IDebugLog		gLog("Pene.log");
PluginHandle	g_pluginHandle = kPluginHandle_Invalid;
NVSEMessagingInterface* g_messagingInterface{};
NVSEInterface* g_nvseInterface{};
NVSEEventManagerInterface* g_eventInterface{};
_InventoryRefGetForID InventoryRefGetForID;
_InventoryRefCreate InventoryRefCreate;
static bool Kill_PC_MF = false;

static CallDetour overwrite_pj_impact{};
static CallDetour overwrite_ReloadMuzzleFlash{};

static CallDetour overwrite_impact_sound1{};
static CallDetour overwrite_impact_sound2{};

static CallDetour overwrite_load_muzzle_flash{};//9BAD9A
static CallDetour ReloadPJMuzzleFlash {};//9BD0C9
static CallDetour Load_TempEffectParticle{};
static UINT32 proj_destroy_vtfun;
static UINT32 get_muzzle_vtfun;


namespace MuzzleFlashKiller {
	static bool kill_mf = false;
	static float cur_muzzle_duration = 0;
	static std::chrono::steady_clock::time_point time_point{};
};

bool NVSEPlugin_Query(const NVSEInterface* nvse, PluginInfo* info)
{
	_MESSAGE("query");

	// fill out the info structure
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "Penetrate_And_Ricochet";
	info->version = 114;

	// version checks
	if (nvse->nvseVersion < PACKED_NVSE_VERSION)
	{
		_ERROR("NVSE version too old (got %08X expected at least %08X)", nvse->nvseVersion, PACKED_NVSE_VERSION);
		return false;
	}

	if (!nvse->isEditor)
	{
		if (nvse->runtimeVersion < RUNTIME_VERSION_1_4_0_525)
		{
			_ERROR("incorrect runtime version (got %08X need at least %08X)", nvse->runtimeVersion, RUNTIME_VERSION_1_4_0_525);
			return false;
		}

		if (nvse->isNogore)
		{
			_ERROR("NoGore is not supported");
			return false;
		}
	}
	else
	{
		if (nvse->editorVersion < CS_VERSION_1_4_0_518)
		{
			_ERROR("incorrect editor version (got %08X need at least %08X)", nvse->editorVersion, CS_VERSION_1_4_0_518);
			return false;
		}
	}

	// version checks pass
	// any version compatibility checks should be done here
	return true;
}

namespace HookMisslePJ {

#define BulletMng BulletManager::bullet_manager_instance()


	static void __forceinline PrintProj(MissileProjectile* prj)
	{
		if (!prj) return;
		gLog.Message("=======================================");
		gLog.FormattedMessage("prjid %x,ref_flag %x", prj->refID, prj->flags);
		gLog.FormattedMessage("impactdatalist size: %u, hasimpact: %u, projFlags: %u", prj->impactDataList.Count(), prj->hasImpacted, prj->projFlags);
		gLog.FormattedMessage("power: %.3f, speedMult: %.3f, BaseProjRange: %.3f, lifeTime: %.3f, hitDamage: %.3f", prj->power, prj->speedMult, prj->BaseProjRange, prj->lifeTime, prj->hitDamage);
		gLog.FormattedMessage("alpha: %.3f, detonationTime: %.3f, blinkTimer: %.3f, angMomentumZ: %.3f, angMomentumX: %.3f", prj->alpha, prj->detonationTime, prj->blinkTimer, prj->angMomentumZ, prj->angMomentumX);
		gLog.FormattedMessage("sourceWpID %x,WpHealthPerc %.3f,sourceRefID %x,liveGrenadeTarid %x", (prj->sourceWeap ? prj->sourceWeap->refID : 0xffffffff), prj->wpnHealthPerc, (prj->sourceRef ? prj->sourceRef->refID : 0xffffffff), (prj->liveGrenadeTarget ? prj->liveGrenadeTarget->refID : 0xffffffff));
		gLog.FormattedMessage("vector104 x- %.3f,y- %.3f,z- %.3f", prj->UnitVector.x, prj->UnitVector.y, prj->UnitVector.z);
		gLog.FormattedMessage("distTravelled %.3f,NiRefObject_muiRefCount: %u", prj->distTravelled, (prj->object114 ? prj->object114->m_uiRefCount : 0xffffffff));
		gLog.FormattedMessage("byte118: %u ,pad119[0]: %u ,pad119[1]: %u ,pad119[2]: %u", prj->byte118, prj->pad119[0], prj->pad119[1], prj->pad119[2]);
		gLog.FormattedMessage("NiNodeName: %s", (prj->node11C ? prj->node11C->m_pcName : "Invaild"));
		gLog.FormattedMessage("unk120: %x, decalSize: %.3f", prj->unk120, prj->decalSize);
		gLog.FormattedMessage("unk140: %x,", prj->unk140);
		gLog.FormattedMessage("range %.3f", prj->range);
		gLog.FormattedMessage("byte154 %u", prj->byte154);
		gLog.FormattedMessage("pad155[0] %u,pad155[1] %u,pad155[2] %u", prj->pad155[0], prj->pad155[1], prj->pad155[2]);
		gLog.FormattedMessage("lifetime %.3f,flt15C %.3f", prj->lifetime, prj->flt15C);
		gLog.FormattedMessage("prj->unk148 %u", prj->hasPlayedPassPlayerSound);

		gLog.Message("=======================================");
	}



	static bool __fastcall MissileProjectileImpactVF(MissileProjectile* _this) {
		if (!_this) {
			gLog.Message("Projectile is none");
			return false;
		}
		auto list = _this->impactDataList;
		if (list.Count() == 0) {
			gLog.Message("=============== This is Fake Projectile ===============");
			PrintProj(_this);
			gLog.FormattedMessage("Fake Projectile Impact Result %u", _this->eImpactResult);
			gLog.FormattedMessage("Fake Projectile Has Impact Flag %u", _this->hasImpacted);
		}
		else {
			gLog.Message("=============== This is Real Projectile ===============");
			PrintProj(_this);
			gLog.FormattedMessage("Real Projectile Impact Result %u", _this->eImpactResult);
			gLog.FormattedMessage("Real Projectile Has Impact Flag %u", _this->hasImpacted);
			gLog.FormattedMessage("Real Projectile ImpactList,count %u", list.Count());
		}
		bool ret = ThisStdCall<bool>(0x009B8B10, _this);
		if (!_this) {
			gLog.Message("Proj Get Clear After VF Calling");
			return ret;
		}
		if (list.Count() == 0) {
			gLog.Message("List Get Clear After VF Calling");
		}
		else
		{
			gLog.FormattedMessage("List Not Get Clear After VF Calling,count %u", list.Count());
		}

		gLog.FormattedMessage("Current Projectile Impact Result %u", _this->eImpactResult);
		gLog.FormattedMessage("Current Projectile Has Impact Flag %u", _this->hasImpacted);
		return ret;
	}

	enum dealing_fakeproj {
		is_fake_proj = 0,
		not_fake_proj = 1
	};

	static __forceinline dealing_fakeproj Dealing_Fake_Impact(MissileProjectile* _fake) {
		if (const auto& fake_ret = BulletMng.IsDPProj(_fake); fake_ret.Valid() ) {
			if (Projectile::ImpactData* impact_data = _fake->GetImpactDataAlt(); impact_data->refr ) {
				if (TESObjectWEAP* src_weap = _fake->sourceWeap){
					if (BGSImpactData* imp_data = GetImpactDataByMaterial(src_weap, MaterialConverter(impact_data->materialType))){
						NiVector3 re_rot {-_fake->rotX,-_fake->rotY,-_fake->rotZ};
						//CdeclCall<UINT32>(Load_TempEffectParticle_Decal, _fake->parentCell,
						//	imp_data->effectDuration, imp_data->model.nifPath.CStr(), re_rot, *_fake->GetPos(), 1.0, 7, nullptr);
					}
				}
			}
			_fake->hasImpacted = 1;
			_fake->eImpactResult = MissileProjectile::ImpactResult::IR_DESTROY;
			return is_fake_proj;
		}
		return not_fake_proj;
	}

	/* ===== Is_Illegal_Impact =====
	use to check penetrate and ricochet is "illegal" or not
	impact distance too short will regard as "illegal"
	*/
	static __forceinline bool Is_Illegal_Impact(MissileProjectile* _this) {
		bool is_illegal = false;
		if (auto pene_ret = BulletMng.IsPenetrateProj(_this); pene_ret.Valid() ) {
			if (Point3Distance(pene_ret.pene_info->LastHitPos, *_this->GetPos()) < pene_ret.pene_info->ExpectDepth) {
				BulletMng.EraseFromPeneMap(_this);
				is_illegal = true;
			}
		}

		if (auto rico_ret = BulletMng.IsRicoProj(_this); rico_ret.Valid() ) {		// Check Ricochet Impact
			float impact_dist = Point3Distance(rico_ret.rico_info->LastHitPos, *_this->GetPos());
			if (impact_dist < 16) {
				BulletMng.EraseFromRicoMap(_this);
				is_illegal = true;
			}
		}
		if (is_illegal) {	// make illegal projectile will destroy directly in impact dealing
			_this->hasImpacted = 1;
			_this->eImpactResult = MissileProjectile::ImpactResult::IR_DESTROY;
		}
		return is_illegal;
	}

	/*
	===== Spawn Penetrate Proj =====
	2 depth based on impact ref type
	turn off basepj hitscan flag at first( when basepj has this flag on ) and turn on after spawning
	only when projectile hit a vaild ref will create penetrate projectiles

	*/
	static __forceinline MissileProjectile* Do_SpawnProj(MissileProjectile* _this, BGSProjectile* base_pj, NiVector3& new_proj_pos,
		bool use_source,float nDamage = -1, float nSpdM = -1, float nRotX = -1, float nRotZ = -1) {
		bool basepj_hitscan = false;
		if ((base_pj->projFlags & BGSProjectile::kFlags_Hitscan) != 0) { // is hitscan
			basepj_hitscan = true;
			base_pj->projFlags ^= BGSProjectile::kFlags_Hitscan;
		}
		if (nDamage < 0) nDamage = _this->hitDamage;
		if (nSpdM < 0) nSpdM = _this->speedMult;
		if (nRotX < 0) nRotX = _this->rotX;
		if (nRotZ < 0) nRotZ = _this->rotZ;

		Actor* source_ref = nullptr;
		MuzzleFlash* muzzle_flash = nullptr;
		
		if (use_source) {
			source_ref = static_cast<Actor*>(_this->sourceRef);
			muzzle_flash = ((HighProcess*)source_ref->baseProcess)->muzzleFlash;
			muzzle_flash->bEnabled = 0;
			muzzle_flash->flashDuration = 0;
			muzzle_flash->enableTimer = -1;
		}
		
		if (source_ref && source_ref == PlayerCharacter::GetSingleton()) {
			if (!MuzzleFlashKiller::kill_mf) {
				MuzzleFlashKiller::cur_muzzle_duration = ((HighProcess*)source_ref->baseProcess)->muzzleFlash->flashDuration * 1.2;
				MuzzleFlashKiller::time_point = std::chrono::steady_clock::now();
				MuzzleFlashKiller::kill_mf = true; 
			}
		}

		MissileProjectile* newProj = (MissileProjectile*)Projectile::Spawn(base_pj, source_ref, nullptr,
			_this->sourceWeap, new_proj_pos, nRotZ, nRotX,
			0, 0, _this->parentCell, /*ignoreGravity*/ 0);

		if (basepj_hitscan) base_pj->projFlags |= BGSProjectile::kFlags_Hitscan;

		newProj->impactDataList.RemoveAll();
		newProj->hasImpacted = 0;
		newProj->eImpactResult = MissileProjectile::ImpactResult::IR_NONE;
		newProj->lifeTime = _this->lifeTime;
		newProj->sourceRef = _this->sourceRef;
		newProj->speedMult = nSpdM;
		newProj->hitDamage = nDamage;
		newProj->wpnHealthPerc = _this->wpnHealthPerc;
		newProj->UnitVector = _this->UnitVector;
		newProj->byte118 = _this->byte118;
		newProj->distTravelled = _this->distTravelled;
		newProj->BaseProjRange = _this->BaseProjRange;
		newProj->unk140 = 0;
		newProj->projFlags = _this->projFlags;
		if (auto* unk120 = (SpawnPJInfo*)newProj->unk120) {
			unk120->PosX = _this->posX;
			unk120->PosY = _this->posY;
			unk120->PosZ = _this->posZ;
		}

		if (basepj_hitscan) {
			newProj->projFlags ^= Projectile::kProjFlag_Bit03Unk_HitScan;
			newProj->projFlags ^= Projectile::kProjFlag_IsHitScan;
			newProj->projFlags ^= Projectile::kProjFlag_Bit0DUnk_HitScan;
			newProj->projFlags ^= Projectile::kProjFlag_HitScanNonTracer;
		}
		return newProj;
	}

	// do penetrate proj spawn
	static bool __fastcall Do_PenePJSpawn(MissileProjectile* _this, BGSProjectile* base_pj, TESObjectWEAP* base_weap, const Projectile::ImpactData* imp_data) {
		TESObjectREFR* imp_ref = imp_data->refr;
		if (!imp_ref) {
			BulletMng.EraseFromPeneMap(_this);
			return false;
		}
		UINT8 converted_mType = MaterialConverter(imp_data->materialType);
		if (BulletMng.NotInPeneMap(_this) && BulletMng.NotInRicoMap(_this)) {	// first hit, Not pene or rico
			auto new_pj_state = NewPenePJStats(_this, imp_ref, (Actor*)_this->sourceRef, base_weap, base_pj, imp_data->hitLocation, converted_mType);// first hit
			if (!new_pj_state.Pene) return false;

			NiVector3 new_pos = NewPosBasedDepth_Pene(_this, new_pj_state.penetrate_depth);
			NiVector3 DP_Pos;
			CalcMidPoint3(*_this->GetPos(),new_pos,DP_Pos);
			BulletMng.MarkAsDP(_this, DP_Pos);
			float peneRotX = _this->rotX, peneRotZ = _this->rotZ;
			CalcPenetrateRot(peneRotX, peneRotZ, new_pj_state.newAPScore);
			if (MissileProjectile* new_proj = Do_SpawnProj(_this, base_pj, new_pos, true,new_pj_state.newDmg, new_pj_state.newSpeedMult, peneRotX, peneRotZ)) {
				BulletMng.GoInPeneMap(new_proj, imp_ref, imp_data->pos, converted_mType, new_pj_state.penetrate_depth, new_pj_state.newAPScore, 1);
			}
		}
		else if (!BulletMng.NotInRicoMap(_this)) {							// ricochet hit.
			return false;
		}
		else if (IsFlagOn(MultiPenetrate)) {								// allow penetrate projectile do penetrate
			if (auto pene_ret = BulletMng.IsPenetrateProj(_this); pene_ret.Valid()) {
				auto& _pene_info = pene_ret.pene_info;
				auto new_pj_state = NewPenePJStats(_this, imp_ref, (Actor*)_this->sourceRef, base_weap, base_pj, imp_data->hitLocation, converted_mType, _pene_info->Pene_Times, _pene_info->APScore);// penetrate hit
				if (!new_pj_state.Pene) {
					BulletMng.EraseFromPeneMap(_this);
					return false;
				}

				NiVector3 new_pos = NewPosBasedDepth_Pene(_this, new_pj_state.penetrate_depth);
				NiVector3 DP_Pos;
				CalcMidPoint3(*_this->GetPos(), new_pos, DP_Pos);
				BulletMng.MarkAsDP(_this, DP_Pos);
				float peneRotX = _this->rotX, peneRotZ = _this->rotZ;
				CalcPenetrateRot(peneRotX, peneRotZ, new_pj_state.newAPScore);
				
				if (MissileProjectile* new_proj = Do_SpawnProj(_this, base_pj, new_pos,true, new_pj_state.newDmg, new_pj_state.newSpeedMult, peneRotX, peneRotZ)) {
					BulletMng.GoInPeneMap(new_proj, imp_ref, imp_data->pos, converted_mType, new_pj_state.penetrate_depth, new_pj_state.newAPScore, _pene_info->Pene_Times + 1);
				}
			}
			BulletMng.EraseFromPeneMap(_this);
		}
		else {
			BulletMng.EraseFromPeneMap(_this);
			return false;
		}
		return true;
	}


	// do rico proj spawn
	static MissileProjectile* __fastcall Do_RicoPJSpawn(MissileProjectile* _this, BGSProjectile* base_pj, TESObjectWEAP* base_weap, const Projectile::ImpactData* imp_data, float backward_chance = 0) {
		TESObjectREFR* imp_ref = (!imp_data->refr) ? nullptr : imp_data->refr;
		NiVector3 Pos;
		UINT32 converted_mType = MaterialConverter(imp_data->materialType);
		SetPosBasedDepth_Backward(_this, 8, Pos);
		auto new_rc_info = NewRicoPJStats(_this, base_pj, backward_chance);
		if (!new_rc_info.Rico) return nullptr;

		if (BulletMng.NotInRicoMap(_this) && BulletMng.NotInPeneMap(_this)) {	// first hit - not ricochet and not penetrate bullet do ricochet
			if (MissileProjectile* rico_pj = Do_SpawnProj(_this, base_pj, Pos,true, new_rc_info.newDmg, new_rc_info.newSpeedMult, new_rc_info.newRX, new_rc_info.newRZ)) {
				RandomlyPlayRicoSound(rico_pj, *_this->GetPos());
				BulletMng.GoInRicoMap(rico_pj, imp_ref, *_this->GetPos(), 1);
				return rico_pj;
			}
		} 
		else if (IsFlagOn(MultiRicochet)) {	// a ricochet do ricochet
			if (auto rico_ret = BulletMng.IsRicoProj(_this); rico_ret.Valid()) {
				if (converted_mType< 12) {
					if (RandMng.GetRand(0, 100) < BulletMng.MaterialChance[converted_mType]) {	// Ricochet Chance Only Apply To Ricochet
						auto& rico_info = rico_ret.rico_info;
						if (MissileProjectile* rico_pj = Do_SpawnProj(_this, base_pj, Pos,true, new_rc_info.newDmg, new_rc_info.newSpeedMult, new_rc_info.newRX, new_rc_info.newRZ)) {
							BulletMng.GoInRicoMap(rico_pj, imp_ref, *_this->GetPos(), rico_ret.rico_info->Rico_Times + 1);
							return rico_pj;
						}
					}
				}
			}
			BulletMng.EraseFromRicoMap(_this);
		}
		return nullptr;
	}

	/*
	static void __forceinline PrintProjPos(MissileProjectile* _this,bool isBefore) {
		if (isBefore)gLog.FormattedMessage("Before Proj Pos { %.2f, %.2f, %.2f}", _this->posX, _this->posY, _this->posZ);
		else gLog.FormattedMessage("After Proj Pos { %.2f, %.2f, %.2f}", _this->posX, _this->posY, _this->posZ);
	}
	*/

	static void __forceinline EraseProj(Projectile* _proj) {
		BulletMng.EraseFromPeneMap(_proj);
		BulletMng.EraseFromRicoMap(_proj);
		BulletMng.EraseFromDPMap(_proj);
	}

	// impact - call pene spawn
	static bool __fastcall PJ_Impact_Hook_9B8BD8(MissileProjectile* _this, void* edx) {
		if (!_this || Is_Illegal_Impact(_this) ||
			PJIsExplosion(_this) || !_this->sourceRef || !IS_ACTOR(_this->sourceRef) ) {
			return ThisStdCall(overwrite_pj_impact.GetOverwrittenAddr(), _this);
		}
		if (const auto* bs_proc = ((Actor*)_this->sourceRef)->baseProcess; bs_proc->processLevel){
			return ThisStdCall(overwrite_pj_impact.GetOverwrittenAddr(), _this);
		}

		Actor* _actor = static_cast<Actor*>(_this->sourceRef);
		auto* base_pj = static_cast<BGSProjectile*>(_this->baseForm);
		auto* base_wp = static_cast<TESObjectWEAP*>(_this->sourceWeap);
		if (!base_pj || !base_wp) {
			return ThisStdCall(overwrite_pj_impact.GetOverwrittenAddr(), _this);
		}

		auto* imp_data = _this->GetImpactDataAlt();
		if (!imp_data) { 
			return ThisStdCall(overwrite_pj_impact.GetOverwrittenAddr(), _this);
		}

		float BackwardChcance = BulletMng.GetBackwardChance(_this);

		
			
		if (!Do_PenePJSpawn(_this, base_pj, base_wp, imp_data)) {
			if (IsFlagOn(Ricochet)) Do_RicoPJSpawn(_this, base_pj, base_wp, imp_data, BackwardChcance);
		}
		else {
		
		}

		return ThisStdCall(overwrite_pj_impact.GetOverwrittenAddr(), (Projectile*)_this);
	}

	static Projectile* __fastcall MissileProjectileDestroyVF(MissileProjectile* _this, void* edx, char flag) {
		BulletMng.EraseFromPeneMap(_this);
		BulletMng.EraseFromRicoMap(_this);
		BulletMng.EraseFromDPMap(_this);
		return ThisStdCall<Projectile*>(proj_destroy_vtfun, _this, flag);
	}



	static TESSound* __fastcall GetImpactDataSound1_Hook(BGSImpactData* _this, MissileProjectile* _impact_pj, TESObjectREFR* imp_ref, UInt32 hit_material) {
		auto* tes_sound = ThisStdCall<TESSound*>(overwrite_impact_sound1.GetOverwrittenAddr(), _this);
		if (tes_sound && _impact_pj && IS_TYPE(_impact_pj, MissileProjectile)) {
			
			if (const auto& pene_ret = BulletMng.IsPenetrateProj(_impact_pj); pene_ret.Valid())
				if (const auto& pene_info = pene_ret.pene_info; pene_info->impact_ref == imp_ref || pene_info->lastHitMaterial == MaterialConverter(hit_material))
					if (Point3Distance(pene_info->LastHitPos, *_impact_pj->GetPos()) < pene_info->ExpectDepth * 3)
						return nullptr;
		}
		return tes_sound;
	}

	static TESSound* __fastcall GetImpactDataSound2_Hook(BGSImpactData* _this, MissileProjectile* _impact_pj, TESObjectREFR* imp_ref, UInt32 hit_material) {
		// Do Chcek
		auto* tes_sound = ThisStdCall<TESSound*>(overwrite_impact_sound2.GetOverwrittenAddr(), _this);
		if (tes_sound && _impact_pj && IS_TYPE(_impact_pj, MissileProjectile)){

			if (const auto& pene_ret = BulletMng.IsPenetrateProj(_impact_pj); pene_ret.Valid())
				if (const auto& pene_info = pene_ret.pene_info; pene_info->impact_ref == imp_ref || pene_info->lastHitMaterial == MaterialConverter(hit_material))
					if (Point3Distance(pene_info->LastHitPos, *_impact_pj->GetPos()) < pene_info->ExpectDepth * 3)
						return nullptr;
		}
		return tes_sound;
	}

	static __declspec(naked) void Caller_9C2B69() {
		__asm {
			mov edx, ebp

			push ebp
			mov ebp, esp

			push[edx + 0x18]				// hitmaterial
			push[edx + 0x8]				// impact_refr
			mov edx, [edx - 0x2B8]			// Projectile *
			call GetImpactDataSound1_Hook

			mov esp, ebp
			pop ebp
			ret
		}
	}

	static __declspec(naked) void Caller_9C2BED() {
		__asm {
			mov edx, ebp

			push ebp
			mov ebp, esp

			push[edx + 0x18]				// hitmaterial
			push[edx + 0x8]					// impact_refr
			mov edx, [edx - 0x2B8]			// Projectile *
			call GetImpactDataSound2_Hook

			mov esp, ebp
			pop ebp
			ret
		}
	}

	static UINT32 __cdecl Load_TempEffectParticle_Hook(TESObjectCELL* parent_cell, float lifetime,
		const char* impact_model_path, NiVector3 impact_rot, NiVector3 ImpactPos, float scale,
		int unk_arg, NiNode* unk_node) {
		
		Projectile* _proj;
		__asm mov _proj, ecx	// Get you mother fucker!
		
		float dmg_ratio = 1.0;
		if (IsFlagOn(ImpactScaler) && _proj){
			if (_proj->hitDamage <= impact_scaler_dmg_minline) dmg_ratio = impact_scaler_min;
			else if (_proj->hitDamage >= impact_scaler_dmg_maxline) dmg_ratio = impact_scaler_max;
			else {
				if (_proj->hitDamage < impact_scaler_dmg_baseline)
					dmg_ratio = 1 + (_proj->hitDamage - impact_scaler_dmg_baseline) / impact_scaler_dmg_dmin_base;
				else 
					dmg_ratio = 1 + (_proj->hitDamage - impact_scaler_dmg_baseline) / impact_scaler_dmg_dmax_base;
			}
			
			if (dmg_ratio < impact_scaler_min) dmg_ratio = impact_scaler_min;
			else if (dmg_ratio > impact_scaler_max) dmg_ratio = impact_scaler_max;
		}
		

		UINT32 ret = CdeclCall<UINT32>(Load_TempEffectParticle.GetOverwrittenAddr(), parent_cell,
			lifetime, impact_model_path, impact_rot, ImpactPos, (scale*dmg_ratio)
			, unk_arg , unk_node); 
		if (ret && IsFlagOn(ExtraDecal)){
			if (IS_TYPE(_proj, MissileProjectile)){
				if (const auto& fake_ret = BulletMng.IsDPProj(_proj); fake_ret.Valid()) {
					NiVector3 rev_rot{ -(impact_rot.x),-(impact_rot.y),-(impact_rot.z) };
					CdeclCall<UINT32>(Load_TempEffectParticle.GetOverwrittenAddr(), parent_cell,
						lifetime, impact_model_path, rev_rot, fake_ret.dp_info->new_pos, (scale*dmg_ratio)
						, unk_arg , unk_node);
				}
			}
		}
		return ret;
	}

	static void __fastcall Kill_Muzzle_Flash(MuzzleFlash* _mf,void* edx,Actor* _actor) {
		gLog.Message("Loaded MF");
		ThisStdCall(overwrite_load_muzzle_flash.GetOverwrittenAddr(),_mf,_actor);
	}

	static MuzzleFlash* __fastcall Get_Muzzle_Flash(HighProcess* _hproc) {
		auto* _mf = ThisStdCall<MuzzleFlash*>(0x8D9300, _hproc);
		if (_mf && MuzzleFlashKiller::kill_mf)
		{
			_mf->bEnabled = 0;
			_mf->flashDuration = 0; 
			_mf->bUpdateLight = 0;
			_mf->enableTimer = -1;
		}
		
		return _mf;
	}


	static inline void InstallHook()
	{
		//INIT_Bullet_Flag();
		overwrite_pj_impact.WriteRelCall(0x9B8BD8, UINT32(PJ_Impact_Hook_9B8BD8));
		proj_destroy_vtfun = DetourVtable(0x108FA54, UInt32(MissileProjectileDestroyVF));

		get_muzzle_vtfun = DetourVtable(0x1087F1C, UInt32(Get_Muzzle_Flash)); 

		overwrite_impact_sound1.WriteRelCall(0x9C2B69, UINT32(Caller_9C2B69));
		overwrite_impact_sound2.WriteRelCall(0x9C2BED, UINT32(Caller_9C2BED));

		Load_TempEffectParticle.WriteRelCall(0x9C2AC3, UINT32(Load_TempEffectParticle_Hook));
	}
}


// This is a message handler for nvse events
// With this, plugins can listen to messages such as whenever the game loads
void MessageHandler(NVSEMessagingInterface::Message* msg)
{
	switch (msg->type)
	{
	case NVSEMessagingInterface::kMessage_DeferredInit:
		HookMisslePJ::InstallHook();
		HookMisslePJ::ReadGenericConfig();
		break;
	case NVSEMessagingInterface::kMessage_MainGameLoop:
		{
			if (MuzzleFlashKiller::kill_mf)
			{
				auto* _player = PlayerCharacter::GetSingleton();
				if (auto* _bp = _player->baseProcess;!_bp->processLevel ){
					MuzzleFlash* _p_mf = ((HighProcess*)_bp)->muzzleFlash;
					_p_mf->enableTimer = -1;
					_p_mf->bEnabled = 0;
					_p_mf->bUpdateLight = 0;
					_p_mf->flashDuration = 0;
				}
				using _sec = std::chrono::duration<float>;
				auto c_time_point = std::chrono::steady_clock::now();
				auto d_time = c_time_point - MuzzleFlashKiller::time_point;
				auto d_sec = std::chrono::duration_cast<_sec>(d_time).count();
				if (d_sec >= MuzzleFlashKiller::cur_muzzle_duration)
					MuzzleFlashKiller::kill_mf = false;
			}
		}
		break;
	}
}

bool NVSEPlugin_Load(NVSEInterface* nvse)
{
	_MESSAGE("MissileHook load");
	g_pluginHandle = nvse->GetPluginHandle();
	// save the NVSE interface in case we need it later
	g_nvseInterface = nvse;
	NVSEDataInterface* nvseData = (NVSEDataInterface*)nvse->QueryInterface(kInterface_Data);
	InventoryRefGetForID = (_InventoryRefGetForID)nvseData->GetFunc(NVSEDataInterface::kNVSEData_InventoryReferenceGetForRefID);
	InventoryRefCreate = (_InventoryRefCreate)nvseData->GetFunc(NVSEDataInterface::kNVSEData_InventoryReferenceCreateEntry);

	// register to receive messages from NVSE

	if (!nvse->isEditor)
	{
		g_messagingInterface = static_cast<NVSEMessagingInterface*>(nvse->QueryInterface(kInterface_Messaging));
		g_messagingInterface->RegisterListener(g_pluginHandle, "NVSE", MessageHandler);
	}
	return true;
}