#include "BulletManager.h"
#include "backward.hpp"
#include <string>
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
static CallDetour ReloadPJMuzzleFlash{};//9BD0C9



static UINT32 proj_destroy_vtfun;
static UINT32 get_muzzle_vtfun;
static UINT32 set_muzzle_vtfun;

using namespace PenetrateAndRicochet;
using namespace PenetrateAndRicochet::ExtraDecalAndImpactEffect;
//namespace MuzzleFlashKiller {
//	static bool kill_mf = false;
//	static float cur_muzzle_duration = 0;
//	static std::chrono::steady_clock::time_point time_point{};
//};

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

namespace PenetrateAndRicochet {

#define BulletMng BulletManager::bullet_manager_instance()


	static void __forceinline PrintProj(MissileProjectile* prj)
	{
		if (!prj) return;
		__WriteLog("=======================================");
		__WriteLog("prjid %x,ref_flag %x", prj->refID, prj->flags);
		__WriteLog("impactdatalist size: %u, hasimpact: %u, projFlags: %u", prj->impactDataList.Count(), prj->hasImpacted, prj->projFlags);
		__WriteLog("power: %.3f, speedMult: %.3f, BaseProjRange: %.3f, lifeTime: %.3f, hitDamage: %.3f", prj->power, prj->speedMult, prj->BaseProjRange, prj->lifeTime, prj->hitDamage);
		__WriteLog("alpha: %.3f, detonationTime: %.3f, blinkTimer: %.3f, angMomentumZ: %.3f, angMomentumX: %.3f", prj->alpha, prj->detonationTime, prj->blinkTimer, prj->angMomentumZ, prj->angMomentumX);
		__WriteLog("sourceWpID %x,WpHealthPerc %.3f,sourceRefID %x,liveGrenadeTarid %x", (prj->sourceWeap ? prj->sourceWeap->refID : 0xffffffff), prj->wpnHealthPerc, (prj->sourceRef ? prj->sourceRef->refID : 0xffffffff), (prj->liveGrenadeTarget ? prj->liveGrenadeTarget->refID : 0xffffffff));
		__WriteLog("vector104 x- %.3f,y- %.3f,z- %.3f", prj->UnitVector.x, prj->UnitVector.y, prj->UnitVector.z);
		__WriteLog("distTravelled %.3f,NiRefObject_muiRefCount: %u", prj->distTravelled, (prj->object114 ? prj->object114->m_uiRefCount : 0xffffffff));
		__WriteLog("byte118: %u ,pad119[0]: %u ,pad119[1]: %u ,pad119[2]: %u", prj->byte118, prj->pad119[0], prj->pad119[1], prj->pad119[2]);
		__WriteLog("NiNodeName: %s", (prj->node11C ? prj->node11C->m_blockName : "Invaild"));
		__WriteLog("unk120: %x, decalSize: %.3f", prj->unk120, prj->decalSize);
		__WriteLog("unk140: %x,", prj->unk140);
		__WriteLog("range %.3f", prj->range);
		__WriteLog("byte154 %u", prj->byte154);
		__WriteLog("pad155[0] %u,pad155[1] %u,pad155[2] %u", prj->pad155[0], prj->pad155[1], prj->pad155[2]);
		__WriteLog("lifetime %.3f,flt15C %.3f", prj->lifetime, prj->flt15C);
		__WriteLog("prj->hasPlayedPassPlayerSound %u", prj->hasPlayedPassPlayerSound); 
		__WriteLog("prj->impactRes %u", prj->eImpactResult);

		__WriteLog("=======================================");
	}



	static bool __fastcall MissileProjectileImpactVF(MissileProjectile* _this) {
		if (!_this) {
			__WriteLog("Projectile is none");
			return false;
		}
		auto list = _this->impactDataList;
		if (list.Count() == 0) {
			__WriteLog("=============== This is Fake Projectile ===============");
			PrintProj(_this);
			__WriteLog("Fake Projectile Impact Result %u", _this->eImpactResult);
			__WriteLog("Fake Projectile Has Impact Flag %u", _this->hasImpacted);
		}
		else {
			__WriteLog("=============== This is Real Projectile ===============");
			PrintProj(_this);
			__WriteLog("Real Projectile Impact Result %u", _this->eImpactResult);
			__WriteLog("Real Projectile Has Impact Flag %u", _this->hasImpacted);
			__WriteLog("Real Projectile ImpactList,count %u", list.Count());
		}
		bool ret = ThisStdCall<bool>(0x009B8B10, _this);
		if (!_this) {
			__WriteLog("Proj Get Clear After VF Calling");
			return ret;
		}
		if (list.Count() == 0) {
			__WriteLog("List Get Clear After VF Calling");
		}
		else
		{
			__WriteLog("List Not Get Clear After VF Calling,count %u", list.Count());
		}

		__WriteLog("Current Projectile Impact Result %u", _this->eImpactResult);
		__WriteLog("Current Projectile Has Impact Flag %u", _this->hasImpacted);
		return ret;
	}

	enum dealing_fakeproj {
		is_fake_proj = 0,
		not_fake_proj = 1
	};

	static __forceinline dealing_fakeproj Dealing_Fake_Impact(MissileProjectile* _fake) {
		if (const auto& fake_ret = BulletMng.IsNPProj(_fake); fake_ret.Valid()) {
			if (Projectile::ImpactData* impact_data = _fake->GetImpactDataAlt(); impact_data->refr) {
				if (TESObjectWEAP* src_weap = _fake->sourceWeap) {
					if (BGSImpactData* imp_data = GetImpactDataByMaterial(src_weap, MaterialConverter(impact_data->materialType))) {
						NiVector3 re_rot{ -_fake->rotation.x,-_fake->rotation.y,-_fake->rotation.z };
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
		if (auto pene_ret = BulletMng.IsPenetrateProj(_this); pene_ret.Valid()) {
			//if (Point3Distance(pene_ret.pene_info->LastHitPos, *_this->GetPos()) < pene_ret.pene_info->ExpectDepth) {
			//	//__WriteLog("[illegal hit] penetrated proj: %.x, 3Dis is %.2f, ExpectDepth is %.2f", _this, Point3Distance(pene_ret.pene_info->LastHitPos, *_this->GetPos()), pene_ret.pene_info->ExpectDepth);
			//	__WriteLog("[illegal hit] penetrated proj: %.x");
			//	BulletMng.EraseFromPeneMap(_this);
			//	is_illegal = true;
			//}
		}

		if (auto rico_ret = BulletMng.IsRicoProj(_this); rico_ret.Valid()) {		// Check Ricochet Impact
			float impact_dist = Point3Distance(rico_ret.rico_info->LastHitPos, *_this->GetPos());
			if (impact_dist < 16) {
				__WriteLog("[illegal hit] ricochet proj: %.x, impact dist is %.2f", _this, impact_dist);
				BulletMng.EraseFromRicoMap(_this);
				//is_illegal = true;
			}
		}
		if (is_illegal) {	// make illegal projectile will destroy directly in impact dealing
			__WriteLog("[illegal hit] mark as illegal proj: %.x", _this);
			_this->hasImpacted = 1;
			_this->eImpactResult = MissileProjectile::ImpactResult::IR_DESTROY;
			if (auto* _rootNode = _this->renderState->rootNode) {
				_rootNode->m_transformLocal.scale *= 0;
			}
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
		bool use_source, float nDamage = -1, float nSpdM = -1, float nRotX = FLT_MAX, float nRotZ = FLT_MAX) {
		bool basepj_hitscan = false;
		if ((base_pj->projFlags & BGSProjectile::kFlags_Hitscan) != 0) { // is hitscan
			basepj_hitscan = true;
			base_pj->projFlags ^= BGSProjectile::kFlags_Hitscan;
		}
		if (nDamage < 0) nDamage = _this->hitDamage; 
		if (nSpdM < 0) nSpdM = _this->speedMult;
		if (std::fabs(nRotX) > PI) nRotX = _this->rotation.x;
		if (std::fabs(nRotZ) > Double_PI) nRotZ = _this->rotation.z; 
		 
		Actor* source_ref = nullptr;
		if (use_source)
			source_ref = static_cast<Actor*>(_this->sourceRef);

		/*MuzzleFlash* muzzle_flash = nullptr;
		if (source_ref) {
			if (HighProcess* b_pc = ((HighProcess*)source_ref->baseProcess))
				if (b_pc->muzzleFlash)
					muzzle_flash = b_pc->muzzleFlash;

		}*/
		if (source_ref == PlayerCharacter::GetSingleton())
			Kill_PC_MF = true;

		MissileProjectile* newProj = (MissileProjectile*)Projectile::Spawn(base_pj, source_ref, nullptr,
			_this->sourceWeap, new_proj_pos, nRotZ, nRotX,
			0, 0, _this->parentCell, /*ignoreGravity*/ 0);

		if (source_ref == PlayerCharacter::GetSingleton())
			Kill_PC_MF = false;
		

			/*if (source_ref == PlayerCharacter::GetSingleton() && muzzle_flash) {
				if (!MuzzleFlashKiller::kill_mf) {
					MuzzleFlashKiller::cur_muzzle_duration = muzzle_flash->flashDuration * 1.2;
					MuzzleFlashKiller::time_point = std::chrono::steady_clock::now();
					MuzzleFlashKiller::kill_mf = true;
				}
			}*/

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
			unk120->PosX = _this->position.x;
			unk120->PosY = _this->position.y;
			unk120->PosZ = _this->position.z;
		}

		if (basepj_hitscan) {
			newProj->projFlags ^= Projectile::kProjFlag_Bit03Unk_HitScan;
			newProj->projFlags ^= Projectile::kProjFlag_IsHitScan;
			newProj->projFlags ^= Projectile::kProjFlag_Bit0DUnk_HitScan;
			newProj->projFlags ^= Projectile::kProjFlag_HitScanNonTracer;
		}
		return newProj;
	}

	//static bool __fastcall Do_PenePJ(MissileProjectile* _this, BGSProjectile* base_pj, TESObjectWEAP* base_weap,
	//										const Projectile::ImpactData* imp_data,
	//										const ImpactRefInfo& imp_ref_info){
	//	Pene_Info* pene_info = nullptr;
	//	if (const auto& pene_proj = BulletMng.IsPenetrateProj(_this); pene_proj.find_pene && pene_proj.Valid() ) {	// check multi penetrate
	//		if (IsFlagOn(MultiPenetrate)) {
	//			pene_info = pene_proj.pene_info;
	//		}
	//		else return false;
	//	}
	//	
	//	
	//	TESObjectREFR* imp_ref = imp_data->refr;
	//	auto new_pj_state = NewPenePJStats(_this, imp_ref, (Actor*)_this->sourceRef, base_weap, base_pj, imp_ref_info);
	//	
	//	if (!new_pj_state.Pene) 
	//	{
	//		BulletMng.EraseFromPeneMap(_this); 
	//		return false; 
	//	}
	//	NiVector3 cur_pos = *_this->GetPos();
	//	NiVector3 new_pos = NewPosBasedDepth_Pene(_this, new_pj_state.penetrate_depth); 
	//	NiVector3 DP_Pos;

	//	/*CalcMidPoint3(cur_pos, new_pos,OUT DP_Pos);
	//	BulletMng.MarkAsNewSpawnedProj(_this, DP_Pos); */

	//	float peneRotX = _this->rotation.x, peneRotZ = _this->rotation.z;
	//	CalcPenetrateRot(OUT peneRotX, OUT peneRotZ, new_pj_state.newAPScore);
	//	Projectile_SetAttribute(_this, new_pos, peneRotX, peneRotZ, new_pj_state.newDmg, new_pj_state.newSpeedMult);

	//	if (pene_info != nullptr) {										// only update when multi penetrate flag on
	//		pene_info->UpdateInfo(imp_ref, cur_pos, imp_ref_info._hit_material,
	//								new_pj_state.penetrate_depth,new_pj_state.newAPScore);
	//	}
	//	else {															// first do penetrate
	//		BulletMng.EraseFromPeneMap(_this);
	//		BulletMng.InsertPeneMap(_this,imp_ref,cur_pos,imp_ref_info._hit_material,
	//									new_pj_state.penetrate_depth, new_pj_state.newAPScore,1);
	//	}

	//	return true;
	//}


	// do penetrate proj spawn
	// if not do penetrate will return false
	static bool __fastcall Do_PenePJSpawn(MissileProjectile* _this, BGSProjectile* base_pj, TESObjectWEAP* base_weap, 
											const Projectile::ImpactData* imp_data,
											const ImpactRefInfo& imp_ref_info) {
		TESObjectREFR* imp_ref = imp_data->refr;
		
		__WriteLog("[Do_PenePJSpawn] HitMaterial is %s", GetMaterialName(imp_ref_info._hit_material));
		
		bool _isHitActor = imp_ref_info.isHitActor();
		NiVector3 opsPos{0.0, 0.0, 0.0};
		float penetrated_depth = 32.0F;

		if (!_isHitActor)
		{
			auto evalRes = EvalPenetratedDepth(_this);
			if (evalRes.isAloneVelocityDirection && evalRes.success) {
				opsPos = evalRes.oppositePosition;
				opsPos.MoveAlone(_this->UnitVector, 1.0f);
				penetrated_depth = evalRes.distance;
			}
			else {
				return false;
			}
		}
		else {
			opsPos = NewPosBasedDepth_Pene(_this, 32.0F);
		}

		if (BulletMng.NotInPeneMap(_this) && BulletMng.NotInRicoMap(_this)) {	// first hit, Not pene or rico
			auto new_pj_state = NewPenePJStats(_this, imp_ref, (Actor*)_this->sourceRef, base_weap, base_pj,imp_ref_info, penetrated_depth);// first hit
			__WriteLog("[Do_PenePJSpawn] %x pene is %u", _this, new_pj_state.Pene);
			__WriteLog("[Do_PenePJSpawn] first hit new projectile depth is %.2f", new_pj_state.penetrate_depth);
			if (!new_pj_state.Pene) 
				return false;

			
			//NiVector3 new_pos = NewPosBasedDepth_Pene(_this, new_pj_state.penetrate_depth);
			NiVector3 new_pos = opsPos;
			
			
			/*NiVector3 DP_Pos;
			CalcMidPoint3(*_this->GetPos(), new_pos, DP_Pos);
			__WriteLog("MarkAsDP_1: %x",_this);
			BulletMng.MarkAsNewSpawnedProj(_this, DP_Pos);*/

			float peneRotX = _this->rotation.x, peneRotZ = _this->rotation.z;
			CalcPenetrateRot(peneRotX, peneRotZ, new_pj_state.newAPScore);
			if (MissileProjectile* new_proj = Do_SpawnProj(_this, base_pj, new_pos, true, new_pj_state.newDmg, new_pj_state.newSpeedMult, peneRotX, peneRotZ)) {
				BulletMng.InsertPeneMap(new_proj, imp_ref, imp_data->pos, imp_ref_info._hit_material, new_pj_state.penetrate_depth, new_pj_state.newAPScore, 1);
				__WriteLog("[Do_PenePJSpawn] Spawned Pene Proj Is %x",new_proj);
				BulletMng.MarkAsNewSpawnedProj(new_proj, imp_ref_info._hit_material);
// set extraDecal infomation
				if (CheckCanAddDecal(_this, imp_data, imp_ref_info))
				{
					__WriteLog("[Do_PenePJSpawn] %x can make a extra decal", new_proj);
					bool canMakeExtraDecal = false;
					NiVector3 _exitPos;
					if (_isHitActor)
					{
						auto raycastRes = GetRayCastPos(opsPos, _this->rotation.x, _this->rotation.z + PI);
						if (raycastRes.raycastSuccess)
						{
							_exitPos = raycastRes.raycastPos;
							canMakeExtraDecal = true;
						}
						
					}
					else {
						_exitPos = opsPos;
						canMakeExtraDecal = true;
					}
					
					if (canMakeExtraDecal)
					{
						__WriteLog("[Do_PenePJSpawn] %x extra decal pos is: %.2f, %.2f, %.2f", new_proj, _exitPos.x, _exitPos.y, _exitPos.z);
						ExtraDecalMng.SetPenetratedInfoForExtraDecal(_this, _exitPos, GetImpactDataByMaterial(base_weap, imp_ref_info._hit_material), imp_data->rigidBody);
					}
					
				}
// set extraDecal infomation
			}
		}
		else if (!BulletMng.NotInRicoMap(_this)) {							// ricochet hit.Will not do penetrate
			return false;
		}
		else if (IsFlagOn(MultiPenetrate)) {								// allow penetrate projectile do penetrate
			if (auto pene_ret = BulletMng.IsPenetrateProj(_this); pene_ret.Valid()) {
				auto& _pene_info = pene_ret.pene_info;
				auto new_pj_state = NewPenePJStats(_this, imp_ref, (Actor*)_this->sourceRef, base_weap, base_pj, imp_ref_info, penetrated_depth, _pene_info->Pene_Times, _pene_info->APScore);// penetrate hit
				if (!new_pj_state.Pene) {
					BulletMng.EraseFromPeneMap(_this);
					return false;
				}

				__WriteLog("[Do_PenePJSpawn] multiply hit new projectile depth is %.2f", new_pj_state.penetrate_depth);
				//NiVector3 new_pos = NewPosBasedDepth_Pene(_this, new_pj_state.penetrate_depth);
				NiVector3 new_pos = opsPos;

				/*NiVector3 DP_Pos;
				CalcMidPoint3(*_this->GetPos(), new_pos, DP_Pos);
				__WriteLog("MarkAsDP_2: %x", _this);
				BulletMng.MarkAsNewSpawnedProj(_this, DP_Pos);*/
				float peneRotX = _this->rotation.x, peneRotZ = _this->rotation.z;
				CalcPenetrateRot(peneRotX, peneRotZ, new_pj_state.newAPScore);

				if (MissileProjectile* new_proj = Do_SpawnProj(_this, base_pj, new_pos, true, new_pj_state.newDmg, new_pj_state.newSpeedMult, peneRotX, peneRotZ)) {
					BulletMng.InsertPeneMap(new_proj, imp_ref, imp_data->pos, imp_ref_info._hit_material, new_pj_state.penetrate_depth, new_pj_state.newAPScore, _pene_info->Pene_Times + 1);
					__WriteLog("[Do_PenePJSpawn] Spawned Multi Pene Proj Is %x", new_proj);
					BulletMng.MarkAsNewSpawnedProj(new_proj, imp_ref_info._hit_material);
// set extraDecal infomation
					if (CheckCanAddDecal(_this, imp_data, imp_ref_info))
					{
						__WriteLog("[Do_PenePJSpawn] pene proj %x can make a extra decal", new_proj);
						//NiVector3 _exitPos = CalcExtraDecalPos(_this, 32.0f);
						bool canDoExtraDecal = false;
						NiVector3 _exitPos;
						if (_isHitActor)
						{
							auto raycastRes = GetRayCastPos(opsPos, _this->rotation.x, _this->rotation.z + PI);
							if (raycastRes.raycastSuccess)
							{
								_exitPos = raycastRes.raycastPos;
								canDoExtraDecal = true;
							}
						}
						else {
							_exitPos = opsPos;
							canDoExtraDecal = true;
						}

						if (canDoExtraDecal)
						{
							__WriteLog("[Do_PenePJSpawn] pene proj %x extra decal pos is: %.2f, %.2f, %.2f", new_proj, _exitPos.x, _exitPos.y, _exitPos.z);
							ExtraDecalMng.SetPenetratedInfoForExtraDecal(_this, _exitPos, GetImpactDataByMaterial(base_weap, imp_ref_info._hit_material), imp_data->rigidBody);
						}
						
					}
// set extraDecal infomation
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

	/*
	CalcAllowRicoChance
	Will always allow ricochet when impact_ref is nullptr or non-actor
	Calculate ricochet chance according DT/DR when impact_ref is actor.
	The Calculation on character and creature are available in this function 

	Note that its name is AllowRicoChance, just for calculate the chance of allowing call ricochet spawn function.
	It is not make sure to make a ricochet
	*/
	static float __fastcall CalcAllowRicoChance(TESObjectREFR* impact_ref, const ImpactRefInfo& impact_ref_info,UINT8 hit_material)
	{
		float _chance = 0;
		if (!impact_ref || !impact_ref_info._impactref_is_actor)
		{
			__WriteLog("impact ref is not actor~");
			return 100.0f;
		}

		if (IS_ACTOR(impact_ref) ) {
			Actor* impact_actor = static_cast<Actor*>(impact_ref);
			float _av_lucky = impact_actor->avOwner.GetActorValue(ActorValueCode::kAVCode_Luck); 
			const ActorDTDR& _dtdr = impact_ref_info._dtdr; 
			if (impact_ref_info._impactref_is_creature && std::holds_alternative<CreatureDTDR>(_dtdr)) {	// calculate creature ricochet chance
				HitLocation h_loc = GetHitLimbLocation(impact_ref_info._hit_location);
				if (h_loc == HitLocation::HL_None) {
					_chance = 0.0f;
				}
				else {
					CreatureDTDR cr_dtdr = std::get<CreatureDTDR>(_dtdr);
					auto Creature_Armo_Slot = h_loc == HitLocation::Head ? 
						ARMORicochet::ArmorSlot::Helmet : ARMORicochet::ArmorSlot::Armor;
					_chance =  ARMORicochet::CalcNativeRicochetChance(Creature_Armo_Slot,_av_lucky,IsMetal(hit_material),cr_dtdr.GetNativeDR(),cr_dtdr.GetNativeDT());
				}
			}
			else if (std::holds_alternative<CharacterDTDR>(_dtdr)){											// calculate character ricochet chance
				auto hit_armo_info = ARMORicochet::GetHitArmorObjInfo(impact_actor, impact_ref_info._hit_location); 
				if (!hit_armo_info.armo || hit_armo_info.slot == ARMORicochet::ArmorSlot::None) {
					_chance = 0.0f;
				}
				else {
					auto armo_class_info = GetArmorClassInfo(hit_armo_info.armo);
					CharacterDTDR ch_dtdr = std::get<CharacterDTDR>(_dtdr); 
					float _armo_dr = hit_armo_info.slot == ARMORicochet::ArmorSlot::Armor ? ch_dtdr.GetArmorDR() : ch_dtdr.GetHelmetDR(); 
					float _armo_dt = hit_armo_info.slot == ARMORicochet::ArmorSlot::Armor ? ch_dtdr.GetArmorDT() : ch_dtdr.GetHelmetDT(); 
					_chance = ARMORicochet::CalcArmorRicochetChance(hit_armo_info.slot, armo_class_info.arClass, armo_class_info.isPowerArmor,
															_av_lucky, IsMetal(hit_material), _armo_dr, _armo_dt);
				}
			}
		}
		__WriteLog("CalcAllowRicoChance %.2f", _chance);
		return _chance;
	}

	static void __forceinline PlayRicochetSound(MissileProjectile* _proj,const TESObjectREFR* _imp_ref, const ImpactRefInfo& impact_ref_info) {
		if (_imp_ref && impact_ref_info.isHitActor()){
			if (_imp_ref == PlayerCharacter::GetSingleton()){
				if (impact_ref_info.isHeadShot() ) {
					RandomlyPlayHelmetRicoSound(_proj);
				}
			}
			RandomlyPlayArmorRicoSound(_proj);
		}

		RandomlyPlayRicoSound(_proj, _proj->position);
	}

	// do rico proj spawn
	static MissileProjectile* __fastcall Do_RicoPJSpawn(MissileProjectile* _this, BGSProjectile* base_pj, TESObjectWEAP* base_weap, 
														const Projectile::ImpactData* imp_data, UINT8 hit_material, const ImpactRefInfo& impact_ref_info,
														float backward_chance = 0) {
		TESObjectREFR* imp_ref = imp_data->refr;
		NiVector3 Pos{ *_this->GetPos() };
		__WriteLog("[Do_RicoPJSpawn] Before Movealone Pos :%.2f, %.2f, %.2f", Pos.x, Pos.y, Pos.z);
		Pos.MoveAlone(_this->UnitVector, -2.0); 
		__WriteLog("[Do_RicoPJSpawn] After Movealone Pos :%.2f, %.2f, %.2f", Pos.x, Pos.y, Pos.z);
		//SetPosBasedDepth_Backward(_this, 32, Pos);
		auto new_rc_info = NewRicoPJStats(_this, base_pj, Pos, backward_chance);
		__WriteLog("[Do_RicoPJSpawn] After NewRicoPJStats Ricochet Begin Pos :%.2f, %.2f, %.2f", Pos.x, Pos.y, Pos.z);
		if (!new_rc_info.Rico) return nullptr;

		if (BulletMng.NotInRicoMap(_this) && BulletMng.NotInPeneMap(_this)) {	// first hit - not ricochet and not penetrate bullet do ricochet
			if (MissileProjectile* rico_pj = Do_SpawnProj(_this, base_pj, Pos, true, new_rc_info.newDmg, new_rc_info.newSpeedMult, new_rc_info.newRX, new_rc_info.newRZ)) {
				//RandomlyPlayRicoSound(rico_pj, *_this->GetPos());
				PlayRicochetSound(rico_pj, imp_ref, impact_ref_info);
				BulletMng.MarkAsNewSpawnedProj(rico_pj, impact_ref_info._hit_material);
				BulletMng.InsertRicoMap(rico_pj, imp_ref, *_this->GetPos(), 1);

				_this->hitDamage -= new_rc_info.newDmg;		// update projectile after ricochet projectile spawned
				return rico_pj;
			}
		}
		else if (IsFlagOn(MultiRicochet)) {	// a ricochet do ricochet
			if (auto rico_ret = BulletMng.IsRicoProj(_this); rico_ret.Valid()) {
				if (hit_material < 12) { 
					if (RandMng.GetRand(0, 100) < BulletMng.MaterialChance[hit_material]) {	// Ricochet Chance Only Apply To Ricochet
						auto& rico_info = rico_ret.rico_info;
						if (MissileProjectile* rico_pj = Do_SpawnProj(_this, base_pj, Pos, true, new_rc_info.newDmg, new_rc_info.newSpeedMult, new_rc_info.newRX, new_rc_info.newRZ)) {
							BulletMng.InsertRicoMap(rico_pj, imp_ref, *_this->GetPos(), rico_ret.rico_info->Rico_Times + 1);
							PlayRicochetSound(rico_pj, imp_ref, impact_ref_info);
							BulletMng.MarkAsNewSpawnedProj(rico_pj, impact_ref_info._hit_material);
							_this->hitDamage -= new_rc_info.newDmg;		// update projectile after ricochet projectile spawned
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
		if (isBefore)__WriteLog("Before Proj Pos { %.2f, %.2f, %.2f}", _this->posX, _this->posY, _this->posZ);
		else __WriteLog("After Proj Pos { %.2f, %.2f, %.2f}", _this->posX, _this->posY, _this->posZ);
	}
	*/

	static void __forceinline EraseProj(Projectile* _proj) {
		BulletMng.EraseFromPeneMap(_proj);
		BulletMng.EraseFromRicoMap(_proj);
		BulletMng.EraseFromNewProjMap(_proj);
		ExtraDecalMng.EraseFromMap(_proj);
	}

	// impact - call pene spawn
	static bool __fastcall PJ_Impact_Hook_9B8BD8(MissileProjectile* _this, void* edx) {
		__WriteLog("[PJ_Impact_Hook_9B8BD8] Get Projectile Impact: %x, Pos: X = %.2f, Y = %.2f, Z = %.2f, Record RX = %.2f, RZ = %.2f",
			_this, _this->position.x, _this->position.y, _this->position.z,
			_this->rotation.x * RdToAg, _this->rotation.z * RdToAg);

		if (!_this  ||
			PJIsExplosion(_this) || !_this->sourceRef || !IS_ACTOR(_this->sourceRef)) {
			return ThisStdCall<bool>(overwrite_pj_impact.GetOverwrittenAddr(), _this);
		}

		BGSProjectile* base_pj = static_cast<BGSProjectile*>(_this->baseForm);
		TESObjectWEAP* base_wp = _this->sourceWeap;
		if (!base_pj || !base_wp) {
			return ThisStdCall<bool>(overwrite_pj_impact.GetOverwrittenAddr(), _this);
		}

		if (ActorValueCode _code = GetWeaponSkill(base_wp); 
			_code != ActorValueCode::kAVCode_Guns && _code != ActorValueCode::kAVCode_BigGuns){
			return ThisStdCall<bool>(overwrite_pj_impact.GetOverwrittenAddr(), _this);
		}

		if (const auto* bs_proc = ((Actor*)_this->sourceRef)->baseProcess; bs_proc->processLevel) {
			return ThisStdCall<bool>(overwrite_pj_impact.GetOverwrittenAddr(), _this);
		}

		Actor* _actor = static_cast<Actor*>(_this->sourceRef);
		
		auto* imp_data = _this->GetImpactDataAlt();
		if (!imp_data) {
			return ThisStdCall<bool>(overwrite_pj_impact.GetOverwrittenAddr(), _this);
		}

		// unuse for now.
		float BackwardChcance = BulletMng.GetBackwardChance(_this);
		UINT8 hit_material = MaterialConverter(imp_data->materialType);
		TESObjectREFR* imp_ref = imp_data->refr;

// set decal info
		ExtraDecalMng.InsertNewDecalInfo(_this);
// set decal info
		__WriteLog("[PJ_Impact_Hook_9B8BD8] Current Impact Proj Is %x", _this);
		if (!imp_ref) {
			BulletMng.EraseFromPeneMap(_this);
			Do_RicoPJSpawn(_this, base_pj, base_wp, imp_data, hit_material, ImpactRefInfo{ false,false,255,255,std::monostate{} },BackwardChcance);
		}
		else {
			ImpactRefInfo _imp_ref_info{ false,false,255,255,std::monostate{}};
			_imp_ref_info._hit_material = hit_material;
			__WriteLog("[PJ_Impact_Hook_9B8BD8] Proj: %x, hit material is %u", _this, _imp_ref_info._hit_material);
			if (IS_ACTOR(imp_ref)) {
				_imp_ref_info._impactref_is_actor = true;
				_imp_ref_info._impactref_is_creature = imp_ref->IsCreature();
				_imp_ref_info._dtdr = GetActorDTDR((Actor*)imp_ref); 
				_imp_ref_info._hit_location = imp_data->hitLocation;
			}
			

			if (!Do_PenePJSpawn(_this, base_pj, base_wp, imp_data, _imp_ref_info)) {
				// failed to spawn penetration
				if (RandMng.GetRand(0, 100) < CalcAllowRicoChance(imp_ref, _imp_ref_info, hit_material)){
					if (!Do_RicoPJSpawn(_this, base_pj, base_wp, imp_data, hit_material, _imp_ref_info, BackwardChcance)) {
						// failed to spawn ricochet
					}
				}
				else {
					// failed to spawn ricochet
				}
			} 
			
			
		}
		__WriteLog("[PJ_Impact_Hook_9B8BD8] Current Done Impact Proj Is %x", _this);
		return ThisStdCall<bool>(overwrite_pj_impact.GetOverwrittenAddr(), (Projectile*)_this);
	}

	


	static Projectile* __fastcall MissileProjectileDestroyVF(MissileProjectile* _this, void* edx, char flag) {
		if (_this)
		{
			
			BulletMng.EraseFromPeneMap(_this);
			BulletMng.EraseFromRicoMap(_this);
			BulletMng.EraseFromNewProjMap(_this);
			ExtraDecalMng.EraseFromMap(_this);
		}
		return ThisStdCall<Projectile*>(proj_destroy_vtfun, _this, flag);
	}

	constexpr bool sound_logging = true;

	static TESSound* __fastcall GetImpactDataSound1_Hook(BGSImpactData* _this, MissileProjectile* _impact_pj, TESObjectREFR* imp_ref, UInt32 hit_material) {
		auto* tes_sound = ThisStdCall<TESSound*>(overwrite_impact_sound1.GetOverwrittenAddr(), _this);
		if (tes_sound && _impact_pj && IS_TYPE(_impact_pj, MissileProjectile)) {
			if (const auto& pene_ret = BulletMng.IsNPProj(_impact_pj); pene_ret.Valid()){
				if (const auto& pene_info = pene_ret.np_info; pene_info->last_hit_material == MaterialConverter(hit_material)) {
					return nullptr;
				}
			}
		}
		return tes_sound;
	}

	static TESSound* __fastcall GetImpactDataSound2_Hook(BGSImpactData* _this, MissileProjectile* _impact_pj, TESObjectREFR* imp_ref, UInt32 hit_material) {
		// Do Chcek
		auto* tes_sound = ThisStdCall<TESSound*>(overwrite_impact_sound2.GetOverwrittenAddr(), _this);
		if (tes_sound && _impact_pj && IS_TYPE(_impact_pj, MissileProjectile)) {
			if (const auto& pene_ret = BulletMng.IsNPProj(_impact_pj); pene_ret.Valid()){
				if (const auto& pene_info = pene_ret.np_info; pene_info->last_hit_material == MaterialConverter(hit_material)) {
						return nullptr;
				}
			}
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

	static void __fastcall Kill_Muzzle_Flash(MuzzleFlash* _mf, void* edx, Actor* _actor) {
		ThisStdCall(overwrite_load_muzzle_flash.GetOverwrittenAddr(), _mf, _actor);
	}

	// makeshift fix
	static CallDetour pj_init_update_muzzleflash{};
	// makeshift fix for multiply muzzle flash
	// just for player now
	static void __fastcall PjINITUpdateMuzzleFlash(Projectile* _this) {
		if (TESObjectREFR* src = _this->sourceRef;IS_ACTOR(src))
		{
			if (Kill_PC_MF)
			{
				return;
			}
		}
		ThisStdCall<void>(pj_init_update_muzzleflash.GetOverwrittenAddr(), _this);
	}

	static inline void InstallHook()
	{
		//INIT_Bullet_Flag();
		overwrite_pj_impact.WriteRelCall(0x9B8BD8, UINT32(PJ_Impact_Hook_9B8BD8));

		//model_clone_hook.WriteRelCall(0x92BED1, UINT32(Fucker_92BED1));
		//CreateMissileProjRefHook.WriteRelCall(0x9BCB5F, UINT32(_CreateMissileProjRef));
		// get_self_hook.WriteRelCall(0x408DBE, UINT32(GetSelf));
		pj_init_update_muzzleflash.WriteRelCall(0x9BDCA2, UINT32(PjINITUpdateMuzzleFlash));

		proj_destroy_vtfun = DetourVtable(0x108FA54, UInt32(MissileProjectileDestroyVF));

		
		overwrite_impact_sound1.WriteRelCall(0x9C2B69, UINT32(Caller_9C2B69));
		overwrite_impact_sound2.WriteRelCall(0x9C2BED, UINT32(Caller_9C2BED));
		
	}

	/*	=============================================

	==============  EXTRA DECAL  ================

	============================================= */
	static CallDetour g_cell_AddDecal_Object{};
	static CallDetour g_cell_AddTempEffect_Object{};

	static CallDetour g_cell_AddSkinedDecal_FromPC{};
	static CallDetour g_cell_AddTempEffect_Skined{};

	static CallDetour g_cell_AddSkinedDecal_FromNPC{};

	static void __fastcall cell_AddDecal_FromPC(TESObjectCELL* _this, ActorHitData* _edx, Decal* _decal, unsigned int decalType, bool a3) {
		
		if (_edx && _decal)
		{
			if (auto* proj = _edx->projectile;
				proj &&
				IS_PROJECTILE(proj) &&
				!ProjectileIsExplosion(proj) &&
				_edx->hitLocation != BGSBodyPartData::eBodyPart_Weapon &&
				!_edx->isFlagOn(ActorHitData::kFlag_ExplodeLimb))
			{
				__WriteLog("[cell_AddDecal_FromPC] _proj check passed %x", proj);
				if (auto* _src = _edx->source)
				{
					auto* _ammo = GetCurEqAmmo(_src);
					auto* _weap = _edx->weapon;
					if (_ammo && _weap && WeapIsBallisticRanged(_weap, true))
					{
						auto evalRes = GetNewSkinedDecalScale((_edx->healthDmg > _edx->weapon->attackDmg.damage ? _edx->weapon->attackDmg.damage : _edx->healthDmg),
							_edx->healthDmg,
							config_flag.IsFlagOn(CONFIG_FLAG::CFG_AP_CONSTANTS_SCALE) ? CheckIsAPPenetrateAmmo(GetCurEqAmmo(_edx->weapon, _edx->source)) : false,
							true,
							_edx->hitLocation);

						Decal exitDecal{ *_decal };
						
						
						_decal->placementRadius *= evalRes.enterScale;
						_decal->width *= evalRes.enterScale;
						_decal->height *= evalRes.enterScale;

						if (evalRes.doExit)
						{
							ExtraDecalInfo& _decalInfo = ExtraDecalMng.GetDecalInfo(proj);
							if (_decalInfo.IsValid() && _decalInfo.IsDoExit())
							{
								_decalInfo.SetScale(evalRes.enterScale, evalRes.exitScale);
								
								exitDecal.placementRadius *= evalRes.exitScale;
								exitDecal.width *= evalRes.exitScale;
								exitDecal.height *= evalRes.exitScale;
								exitDecal.kSurfaceNormal *= __m128{-1, -1, -1}; 
								exitDecal.worldPos = _decalInfo.exitPos;
								exitDecal.rotation *= __m128{-1, -1, -1};
								ThisStdCall<void>(g_cell_AddSkinedDecal_FromPC.GetOverwrittenAddr(), _this, _decal, decalType, a3);
								ThisStdCall<void>(g_cell_AddSkinedDecal_FromPC.GetOverwrittenAddr(), _this, &exitDecal, decalType, a3);
								
								if (const BGSImpactData* _impactData = _decalInfo.GetBGSImpactData())
								{
									__WriteLog("[cell_AddDecal_FromPC] Proj %x, impactdata duration is %.2f, modelpath is %s, scale is %.2f",
										proj,_impactData->effectDuration, _impactData->model.nifPath.m_data, evalRes.exitScale);
									LoadTempEffectParticle(_decal->parentCell, _impactData->effectDuration, _impactData->model.nifPath.m_data,
										exitDecal.rotation, exitDecal.worldPos, evalRes.exitScale,
										7, _decalInfo.GetOrganismNode());
								}
								ExtraDecalMng.EraseFromMap(proj);
								return;
							}
						}
					}
				}
			}
		}
		ThisStdCall<void>(g_cell_AddSkinedDecal_FromPC.GetOverwrittenAddr(), _this, _decal, decalType, a3);
	}

	static void __fastcall cell_AddDecal_FromNPC(TESObjectCELL* _this, ActorHitData* _edx, Decal* _decal, unsigned int decalType, bool a3) {
		__WriteLog("[cell_AddDecal_FromNPC] enter");
		if (_edx && _decal)
		{
			if (auto* proj = _edx->projectile;
				proj &&
				IS_PROJECTILE(proj) &&
				!ProjectileIsExplosion(proj) &&
				_edx->hitLocation != BGSBodyPartData::eBodyPart_Weapon &&
				!_edx->isFlagOn(ActorHitData::kFlag_ExplodeLimb))
			{
				__WriteLog("[cell_AddDecal_FromNPC] _proj check passed %x", proj);
				if (auto* _src = _edx->source)
				{
					auto* _ammo = GetCurEqAmmo(_src);
					auto* _weap = _edx->weapon;
					if (_ammo && _weap && WeapIsBallisticRanged(_weap, true))
					{
						auto evalRes = GetNewSkinedDecalScale((_edx->healthDmg > _edx->weapon->attackDmg.damage ? _edx->weapon->attackDmg.damage : _edx->healthDmg),
							_edx->healthDmg,
							config_flag.IsFlagOn(CONFIG_FLAG::CFG_AP_CONSTANTS_SCALE) ? CheckIsAPPenetrateAmmo(GetCurEqAmmo(_edx->weapon, _edx->source)) : false,
							true,
							_edx->hitLocation);

						Decal exitDecal{ *_decal };
						_decal->placementRadius *= evalRes.enterScale;
						_decal->width *= evalRes.enterScale;
						_decal->height *= evalRes.enterScale;

						if (evalRes.doExit)
						{
							ExtraDecalInfo& _decalInfo = ExtraDecalMng.GetDecalInfo(proj);
							if (_decalInfo.IsValid() && _decalInfo.IsDoExit())
							{
								_decalInfo.SetScale(evalRes.enterScale, evalRes.exitScale);

								exitDecal.placementRadius *= evalRes.exitScale;
								exitDecal.width *= evalRes.exitScale;
								exitDecal.height *= evalRes.exitScale;
								exitDecal.kSurfaceNormal *= __m128{-1, -1, -1};
								exitDecal.worldPos = _decalInfo.exitPos;
								exitDecal.rotation *= __m128{-1, -1, -1};
								ThisStdCall<void>(g_cell_AddSkinedDecal_FromNPC.GetOverwrittenAddr(), _this, _decal, decalType, a3);
								ThisStdCall<void>(g_cell_AddSkinedDecal_FromNPC.GetOverwrittenAddr(), _this, &exitDecal, decalType, a3);
								
								if (const BGSImpactData* _impactData = _decalInfo.GetBGSImpactData())
								{
									//__WriteLog("[cell_AddDecal_Object] impactdata duration is %.2f, modelpath is %s",
										//_impactData->effectDuration, _impactData->model.nifPath.m_data);
									LoadTempEffectParticle(_decal->parentCell, _impactData->effectDuration, _impactData->model.nifPath.m_data,
										exitDecal.rotation, exitDecal.worldPos, evalRes.exitScale,
										7, nullptr);
								}
								ExtraDecalMng.EraseFromMap(proj);
								return;
							}
						}
					}
				}
			}
		}
		ThisStdCall<void>(g_cell_AddSkinedDecal_FromNPC.GetOverwrittenAddr(), _this, _decal, decalType, a3);
	}

	constexpr bool _addDecalLog = true;

// object add decal and temp effect
	static void __fastcall cell_AddDecal_Object(TESObjectCELL* _this, Projectile* _proj, Decal* _decal, unsigned int decalType, bool a3) {
		if (_decal) {
			ExtraDecalInfo& _decalInfo = ExtraDecalMng.GetDecalInfo(_proj);
			if (_decalInfo.IsValid() && _proj && IS_PROJECTILE(_proj) && !ProjectileIsExplosion(_proj))
			{
				// __WriteLog("[cell_AddDecal_Object] Check Proj Passed: %x", _proj);
				if (auto* _weap = _proj->sourceWeap) {
					float _scale = GetNewObjcetDecalScale(_proj->hitDamage);
					_decal->placementRadius *= _scale;
					_decal->width *= _scale;
					_decal->depth *= _scale;
					_decal->height *= _scale;


					if (_decalInfo.IsDoExit())
					{
						_decalInfo.SetScale(_scale, _scale);
						Decal exitDecal{ *_decal };
						exitDecal.worldPos = _decalInfo.GetExitPos();
						exitDecal.rotation *= __m128{-1,-1,-1};
						exitDecal.kSurfaceNormal *= __m128{-1,-1,-1};
						// __WriteLog("[cell_AddDecal_Object] Projectile in ExtraDecalMng: %x", _proj);
						ThisStdCall<void>(g_cell_AddDecal_Object.GetOverwrittenAddr(), _this, _decal, decalType, a3);
						if (_decal->node) {
							__WriteLog("[cell_AddDecal_Object] _decal expect pos %.2f, %.2f, %.2f", exitDecal.worldPos.x, exitDecal.worldPos.y, exitDecal.worldPos.z);
							__WriteLog("[cell_AddDecal_Object] _decal->node pos %.2f, %.2f, %.2f", _decal->node->WorldTranslate().x, _decal->node->WorldTranslate().y, _decal->node->WorldTranslate().z);
						}
						else {
							__WriteLog("[cell_AddDecal_Object] decal dont produce node");
						}
						ThisStdCall<void>(g_cell_AddDecal_Object.GetOverwrittenAddr(), _this, &exitDecal, decalType, a3);
						
						if (const BGSImpactData* _impactData = _decalInfo.GetBGSImpactData())
						{
							__WriteLog("[cell_AddDecal_Object] proj %x impactdata duration is %.2f, modelpath is %s",
								_proj,_impactData->effectDuration, _impactData->model.nifPath.m_data);
							LoadTempEffectParticle(_decal->parentCell, _impactData->effectDuration, _impactData->model.nifPath.m_data,
								exitDecal.rotation, exitDecal.worldPos, _scale,
								7, nullptr);
						}
						ExtraDecalMng.EraseFromMap(_proj);
						return;
					}
					//object_impact_proj_recorder.emplace(_proj, Scale);
				}
			}
		}
		ThisStdCall<void>(g_cell_AddDecal_Object.GetOverwrittenAddr(), _this, _decal, decalType, a3);
	}

	static BSTempEffectParticle* __cdecl Load_SkinedTempEffectParticle_Hook(TESObjectCELL* parent_cell, float lifetime,
		const char* impact_model_path, NiVector3 impact_rot, NiVector3 ImpactPos, float scale,
		int unk_arg, NiNode* unk_node) {
		auto* ebp = GetParentBasePtr(_AddressOfReturnAddress(), false);
		auto* _proj = *reinterpret_cast<Projectile**>(ebp - 0x2B0);
		float enterScale = scale;
		if (_proj)
		{
			__WriteLog("[SkinedTempEffectParticle_Hook] _proj is %x", _proj);
			if (const ExtraDecalInfo& _extraDecalInfo = ExtraDecalMng.GetDecalInfoCst(_proj);
				_extraDecalInfo.IsValid() && _extraDecalInfo.IsDoExit() && 
				IS_PROJECTILE(_proj))
			{
				__WriteLog("[SkinedTempEffectParticle_Hook] _proj check passed %x", _proj);
				enterScale *= (_extraDecalInfo.enterScale);

				// exit effect
				NiVector3 _newRot = impact_rot;
				_newRot *= __m128{-1, -1, -1};
				BSTempEffectParticle* _extraEffect = CdeclCall<BSTempEffectParticle*>(g_cell_AddTempEffect_Skined.GetOverwrittenAddr(),
					parent_cell, lifetime, impact_model_path, _newRot, _extraDecalInfo.exitPos, _extraDecalInfo.exitScale, unk_arg, nullptr);
				// exit effect
			}
		}
		// enter effect
		return CdeclCall<BSTempEffectParticle*>(g_cell_AddTempEffect_Skined.GetOverwrittenAddr(),
			parent_cell, lifetime, impact_model_path, impact_rot, ImpactPos, enterScale, unk_arg, unk_node);
	}

	static  BSTempEffectParticle* __cdecl Load_ObjectImpactTempEffectParticle_Hook(TESObjectCELL* parent_cell, float lifetime,
		const char* impact_model_path, NiVector3 impact_rot, NiVector3 ImpactPos, float scale,
		int unk_arg, NiNode* unk_node) {
		auto* ebp = GetParentBasePtr(_AddressOfReturnAddress(), false);
		auto* _proj = *reinterpret_cast<Projectile**>(ebp - 0x2B8);
		float enterScale = scale;
		if (_proj)
		{
			__WriteLog("[ObjectImpactTempEffectParticle] _proj is %x", _proj);
			if (const auto& _extraDecalInfo = ExtraDecalMng.GetDecalInfoCst(_proj);
				_extraDecalInfo.IsValid() && _extraDecalInfo.IsDoExit() && 
				IS_PROJECTILE(_proj))
			{
				__WriteLog("[ObjectImpactTempEffectParticle] _proj check can do exit passed %x", _proj);
				enterScale *= (_extraDecalInfo.enterScale);

				// exit effect
				NiVector3 _newRot = impact_rot;
				_newRot *= __m128{-1, -1, -1};
				BSTempEffectParticle* _extraEffect = CdeclCall<BSTempEffectParticle*>(g_cell_AddTempEffect_Object.GetOverwrittenAddr(),
					parent_cell, lifetime, impact_model_path, _newRot, _extraDecalInfo.exitPos, _extraDecalInfo.exitScale, unk_arg, nullptr);
				// exit effect
			}
		}
		// enter effect
		return CdeclCall<BSTempEffectParticle*>(g_cell_AddTempEffect_Object.GetOverwrittenAddr(),
			parent_cell, lifetime, impact_model_path, impact_rot, ImpactPos, enterScale, unk_arg, unk_node);
	}
// object add decal and temp effect

	static __declspec(naked) void Fucker_88F0B4() {
		__asm {
			mov edx, [ebx + 0xC]
			jmp cell_AddDecal_FromPC
		}
	}

	static __declspec(naked) void Fucker_88F0D0() {
		__asm {
			mov edx, [ebx + 0xC]
			jmp cell_AddDecal_FromNPC
		}
	}

	static __declspec(naked) void Fucker_9C2861() {
		__asm {
			mov edx, [ebp - 0x2B8]
			jmp cell_AddDecal_Object
		}
	}

	namespace ExtraDecalAndImpactEffect {
	
		static CallDetour _dealimpact_test;
		void __fastcall DealImpact_Hook(
			Projectile* _this,
			void* _edx,
			TESObjectREFR* impact_refr,
			NiPoint3* impactref_pos,
			NiPoint3* impact_rot,
			hkCdBody* _hkCdBody,
			uint32_t hitmaterial) {
			__WriteLog("[DealImpact_Hook] Deal Impact This Proj Is %x", _this);
			ThisStdCall(_dealimpact_test.GetOverwrittenAddr(), _this, impact_refr, impactref_pos, impact_rot, _hkCdBody, hitmaterial);
		}

		

		static inline void InstallHook()
		{
			//_dealimpact_test.WriteRelCall(0x9C2058, UINT32(DealImpact_Hook));
			if (config_flag.IsFlagOn(CONFIG_FLAG::CFG_OBJECT_DECAL_SCALE)) {
				g_cell_AddDecal_Object.WriteRelCall(0x9C2861, UINT32(Fucker_9C2861));
				// g_cell_AddTempEffect_Object.WriteRelCall(0x9C2AC3, UINT32(Load_ObjectImpactTempEffectParticle_Hook)); 
			}

// add skined decal
			g_cell_AddSkinedDecal_FromPC.WriteRelCall(0x88F0B4, UINT32(Fucker_88F0B4));
			if (config_flag.IsFlagOn(CONFIG_FLAG::CFG_GENERAL_NPC_ENABLE))
				g_cell_AddSkinedDecal_FromNPC.WriteRelCall(0x88F0D0, UINT32(Fucker_88F0D0));


			// g_cell_AddTempEffect_Skined.WriteRelCall(0x88F245, UINT32(Load_SkinedTempEffectParticle_Hook));
// add skined decal
		}
	}
}


// This is a message handler for nvse events
// With this, plugins can listen to messages such as whenever the game loads
void MessageHandler(NVSEMessagingInterface::Message* msg)
{
	switch (msg->type)
	{
	case NVSEMessagingInterface::kMessage_DeferredInit:
		initSingleton();

		PenetrateAndRicochet::ReadGenericConfig();
		PenetrateAndRicochet::ReadArmorRicochetConfig();
		PenetrateAndRicochet::ExtraDecalAndImpactEffect::ReadGenericConfig();

		PenetrateAndRicochet::InstallHook();
		PenetrateAndRicochet::ExtraDecalAndImpactEffect::InstallHook();
		break;
	case NVSEMessagingInterface::kMessage_MainGameLoop:
	{
		/*if (MuzzleFlashKiller::kill_mf)
		{
			auto* _player = PlayerCharacter::GetSingleton();
			if (auto* _bp = _player->baseProcess; !_bp->processLevel) {
				if (MuzzleFlash* _p_mf = ((HighProcess*)_bp)->muzzleFlash) {
					_p_mf->enableTimer = -1;
					_p_mf->bEnabled = 0;
					_p_mf->flashDuration = 0;
				}
			}
			using _sec = std::chrono::duration<float>;
			auto c_time_point = std::chrono::steady_clock::now();
			auto d_time = c_time_point - MuzzleFlashKiller::time_point;
			auto d_sec = std::chrono::duration_cast<_sec>(d_time).count();
			if (d_sec >= MuzzleFlashKiller::cur_muzzle_duration)
				MuzzleFlashKiller::kill_mf = false;
		}*/
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