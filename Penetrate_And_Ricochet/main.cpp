#pragma once
#include "nvse/PluginAPI.h"
#include "nvse/GameObjects.h"
#include "nvse/GameData.h"
#include "nvse/SafeWrite.h"
#include "nvse/NiObjects.h"
#include "Gathering_Code.h"
#include "internal/class_vtbls.h"
#include <set>
#include <random>
#include "BulletManager.h"
#include <unordered_set>
//#include "utilities/IConsole.h"
//NoGore is unsupported in xNVSE


IDebugLog		gLog("Pene.log");
PluginHandle	g_pluginHandle = kPluginHandle_Invalid;
NVSEMessagingInterface* g_messagingInterface{};
NVSEInterface* g_nvseInterface{};
NVSEEventManagerInterface* g_eventInterface{};
_InventoryRefGetForID InventoryRefGetForID;
_InventoryRefCreate InventoryRefCreate;

static CallDetour overwrite_pj_impact{};
static UINT32 proj_destroy_vtfun;
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
		gLog.FormattedMessage("prjid %x,ref_flag %x", prj->refID,prj->flags);
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
		gLog.FormattedMessage("lifetime %.3f,flt15C %.3f", prj->lifetime,prj->flt15C);
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
			gLog.FormattedMessage("Fake Projectile Impact Result %u",_this->eImpactResult);
			gLog.FormattedMessage("Fake Projectile Has Impact Flag %u",_this->hasImpacted);
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
			gLog.FormattedMessage("List Not Get Clear After VF Calling,count %u",list.Count());
		}

		gLog.FormattedMessage("Current Projectile Impact Result %u", _this->eImpactResult);
		gLog.FormattedMessage("Current Projectile Has Impact Flag %u", _this->hasImpacted);
		return ret;
	}

	/* ===== Is_Illegal_Impact =====
	use to check penetrate and ricochet is "illegal" or not
	impact distance too short will regard as "illegal"
	*/
	static __forceinline bool Is_Illegal_Impact(MissileProjectile* _this) {
		bool is_illegal = false;
		if (auto pene_ret = BulletMng.IsPenetrateProj(_this); pene_ret.find_pene) {
			NiVector3 cur_hit_pos = *_this->GetPos();
			if (Point3Distance(pene_ret.pene_info.LastHitPos, cur_hit_pos) < pene_ret.pene_info.ExpectDepth) {
				BulletMng.EraseFromPeneMap(_this);
				is_illegal = true;
			}
		}

		if (auto rico_ret = BulletMng.IsRicoProj(_this); rico_ret.find_rico) {		// Check Ricochet Impact
			float impact_dist = Point3Distance(rico_ret.rico_info.LastHitPos, *_this->GetPos());
			if (impact_dist < 16) {
				BulletMng.EraseFromRicoMap(_this);
				is_illegal = true;
			}
		}

		if (is_illegal){	// make illegal projectile will destroy directly in impact dealing
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
	static __forceinline MissileProjectile* Do_SpawnDealDmgProj(MissileProjectile* _this, BGSProjectile* base_pj, NiPoint3& new_proj_pos,
																float nDamage = -1,float nSpdM = -1,float nRotX = -1,float nRotZ = -1) {
		bool basepj_hitscan = false;
		if ((base_pj->projFlags & BGSProjectile::kFlags_Hitscan) != 0) { // is hitscan
			basepj_hitscan = true;
			base_pj->projFlags ^= BGSProjectile::kFlags_Hitscan;
		}
		if (nDamage < 0) nDamage = _this->hitDamage;
		if (nSpdM < 0) nSpdM = _this->speedMult;
		if (nRotX < 0) nRotX = _this->rotX;
		if (nRotZ < 0) nRotZ = _this->rotZ;


		MissileProjectile* newProj = (MissileProjectile*)Projectile::Spawn(base_pj, (static_cast<Actor*>(_this->sourceRef)), nullptr,
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
	static bool __fastcall Do_PenePJSpawn(MissileProjectile* _this,BGSProjectile* base_pj, TESObjectWEAP* base_weap,const Projectile::ImpactData* imp_data ) {
		TESObjectREFR* imp_ref = imp_data->refr;
		if (!imp_ref) { 
			BulletMng.EraseFromPeneMap(_this);
			return false; 
		}
		if (BulletMng.NotInPeneMap(_this) && BulletMng.NotInRicoMap(_this) ) {	// first hit, Not pene or rico
			auto new_pj_state = NewPenePJStats(_this, imp_ref, (Actor*)_this->sourceRef, base_weap, base_pj, imp_data->hitLocation, imp_data->materialType);// first hit
			if (!new_pj_state.Pene) return false;
			NiPoint3 new_pos = NewPosBasedDepth_Pene(_this, new_pj_state.penetrate_depth);

			float peneRotX = _this->rotX, peneRotZ = _this->rotZ;
			CalcPenetrateRot(peneRotX, peneRotZ, new_pj_state.newAPScore);
			if (MissileProjectile* new_proj = Do_SpawnDealDmgProj(_this, base_pj, new_pos, new_pj_state.newDmg, new_pj_state.newSpeedMult, peneRotX, peneRotZ)) {
				BulletMng.GoInPeneMap(new_proj, imp_ref, imp_data->pos, imp_data->materialType, new_pj_state.penetrate_depth, new_pj_state.newAPScore, 1);
			}
		}
		else if (!BulletMng.NotInRicoMap(_this)){							// ricochet hit.
			return false;
		}
		else if (IsFlagOn(MultiPenetrate) ) {								// allow penetrate projectile do penetrate
			if (auto pene_ret = BulletMng.IsPenetrateProj(_this); pene_ret.find_pene){
				auto& _pene_info = pene_ret.pene_info;
				auto new_pj_state = NewPenePJStats(_this, imp_ref, (Actor*)_this->sourceRef, base_weap, base_pj, imp_data->hitLocation, imp_data->materialType, _pene_info.Pene_Times, _pene_info.APScore);// penetrate hit
				if (!new_pj_state.Pene) { 
					BulletMng.EraseFromPeneMap(_this);
					return false; 
				}
				NiPoint3 new_pos = NewPosBasedDepth_Pene(_this, new_pj_state.penetrate_depth);

				float peneRotX = _this->rotX, peneRotZ = _this->rotZ;
				CalcPenetrateRot(peneRotX, peneRotZ, new_pj_state.newAPScore);
				if (MissileProjectile* new_proj = Do_SpawnDealDmgProj(_this, base_pj, new_pos, new_pj_state.newDmg, new_pj_state.newSpeedMult, peneRotX, peneRotZ)) {
					BulletMng.GoInPeneMap(new_proj, imp_ref, imp_data->pos, imp_data->materialType, new_pj_state.penetrate_depth, new_pj_state.newAPScore, _pene_info.Pene_Times + 1);
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
	static MissileProjectile* __fastcall Do_RicoPJSpawn(MissileProjectile* _this, BGSProjectile* base_pj, TESObjectWEAP* base_weap, const Projectile::ImpactData* imp_data,float backward_chance = 0) {
		TESObjectREFR* imp_ref = (!imp_data->refr) ? nullptr : imp_data->refr;
		NiPoint3 Pos;
		SetPosBasedDepth_Backward(_this,8, Pos);
		auto new_rc_info = NewRicoPJStats(_this, base_pj, backward_chance);
		if (!new_rc_info.Rico) return nullptr;
		
		if (BulletMng.NotInRicoMap(_this) && BulletMng.NotInPeneMap(_this) ) {	// first hit - not ricochet and not penetrate bullet do ricochet
			if (MissileProjectile* rico_pj = Do_SpawnDealDmgProj(_this, base_pj, Pos, new_rc_info.newDmg, new_rc_info.newSpeedMult, new_rc_info.newRX, new_rc_info.newRZ)) {
				RandomlyPlayRicoSound(rico_pj,*_this->GetPos());
				BulletMng.GoInRicoMap(rico_pj, imp_ref, *_this->GetPos(), 1);
				return rico_pj;
			}
		}
		else if (IsFlagOn(MultiRicochet)) {	// a ricochet do ricochet
			if (auto rico_ret = BulletMng.IsRicoProj(_this); rico_ret.find_rico) {
				if (imp_data->materialType < 12) {
					if (RandMng.GetRand(0, 100) < BulletMng.MaterialChance[imp_data->materialType]) {	// Ricochet Chance Only Apply To Ricochet
						auto& rico_info = rico_ret.rico_info;
						if (MissileProjectile* rico_pj = Do_SpawnDealDmgProj(_this, base_pj, Pos, new_rc_info.newDmg, new_rc_info.newSpeedMult, new_rc_info.newRX, new_rc_info.newRZ)) {
							BulletMng.GoInRicoMap(rico_pj, imp_ref, *_this->GetPos(), rico_ret.rico_info.Rico_Times + 1);
							return rico_pj;
						}
					}
				}
			}
			BulletMng.EraseFromRicoMap(_this);
		}
		return nullptr;
	}

	// impact - call pene spawn
	static bool __fastcall PJ_Impact_Hook_9B8BD8(MissileProjectile* _this, void* edx) {
		if (!_this) return ThisStdCall(overwrite_pj_impact.GetOverwrittenAddr(), _this);
		if (Is_Illegal_Impact(_this)) return ThisStdCall(overwrite_pj_impact.GetOverwrittenAddr(), _this);

		bool ret = ThisStdCall(overwrite_pj_impact.GetOverwrittenAddr(), (Projectile*)_this);
		if (!ret) return ret;
		if (PJIsExplosion(_this)) return ret;
		if (!_this->sourceRef || !IS_ACTOR(_this->sourceRef)) return ret;
		if (const auto* bs_proc = ((Actor*)_this->sourceRef)->baseProcess; bs_proc->processLevel) return ret;
		auto* imp_data = _this->GetImpactData();
		if (!imp_data) return ret;
		auto* base_pj = static_cast<BGSProjectile*>(_this->baseForm);
		auto* base_wp = static_cast<TESObjectWEAP*>(_this->sourceWeap);
		if (!base_pj || !base_wp) return ret;
		
		float BackwardChcance = BulletMng.GetBackwardChance(_this);
		if (!Do_PenePJSpawn(_this, base_pj, base_wp, imp_data)){
			if (IsFlagOn(Ricochet)) Do_RicoPJSpawn(_this, base_pj, base_wp, imp_data, BackwardChcance);
		}
		return ret;
	}

	static Projectile* __fastcall MissileProjectileDestroyVF(MissileProjectile* _this, void* edx, char flag) {
		BulletMng.EraseFromPeneMap(_this);
		BulletMng.EraseFromRicoMap(_this);
		return ThisStdCall<Projectile*>(proj_destroy_vtfun, _this, flag);
	}
	
	

	static inline void InstallHook()
	{
		//INIT_Bullet_Flag();
		overwrite_pj_impact.WriteRelCall(0x9B8BD8, UINT32(PJ_Impact_Hook_9B8BD8));
		//overwrite_pj_impact.WriteRelCall(0x9B8BD8, UINT32(PJ_Impact_Hook_Rico));
		proj_destroy_vtfun = DetourVtable(0x108FA54, UInt32(MissileProjectileDestroyVF));
		
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
