#pragma once

#include "Gathering_Utility.h"
#include <random>
#include <array>
#include "RoughINIReader.h"

template<typename T>
constexpr int sgn(const T& a, const T& b) noexcept {
	return (a > b) - (a < b);
}

template<typename T>
constexpr int sgn(const T& a) noexcept {
	return sgn(a, T(0));
}
namespace HookMisslePJ {
	using formid = UInt32;
	float constexpr DeToAg = 57.2957787;
	float constexpr PI = 3.1415926;
	float constexpr Double_PI = 6.2831852;
	float constexpr Half_PI = 1.5707963;

	static float penetrate_dmg_threshold = 12;
	static float penetrate_speed_threshold = 21760;
	static float penetrate_base_depth = 16;
	static float penetrate_furniture_depth = 4;
	static float penetrate_human_base_depth = 32;
	static float penetrate_max_depth = 48;
	static float penetrate_deviationX_mult = 1;
	static float penetrate_max_deviationX = 10;
	static float penetrate_deviationZ_mult = 1;
	static float penetrate_max_deviationZ = 30;
	static float penetrate_base_chance = 80;
	static float penetrate_ap_dmg_threshold = 36;
	static float penetrate_hollow_buffscale = 1.0;
	static float penetrate_ap_buffscale = 1.0;

	static float ricochet_max_rotX = 30;
	static float ricochet_max_rotZ = 60;
	static float ricochet_speed_threshold = 6400;
	static float ricochet_damage_threshold = 6;

	static float impact_scaler_dmg_minline = 10;
	static float impact_scaler_dmg_dmin_base = 20;
	
	static float impact_scaler_dmg_baseline = 30;

	static float impact_scaler_dmg_dmax_base = 30;
	static float impact_scaler_dmg_maxline = 60;
	
	static float impact_scaler_min = 0.5;
	static float impact_scaler_max = 2.0;

	static __forceinline NiNode* GetActorPJNode(Actor* _actor) {
		if (_actor && _actor->baseProcess && !_actor->baseProcess->processLevel)return _actor->baseProcess->GetProjectileNode();
		return nullptr;
	}

	static __forceinline NiVector3 GetActorPJNodePos(Actor* _actor) {
		if (auto* pj_node = GetActorPJNode(_actor)){
			return pj_node->m_worldTranslate;
		}
		return { 0,0,0 };
	}

	template<typename T>
	constexpr int sgn(const T& a, const T& b) noexcept {
		return (a > b) - (a < b);
	}

	template<typename T>
	constexpr int sgn(const T& a) noexcept {
		return sgn(a, T(0));
	}

	struct xorshift32 {
		uint32_t a;

		using result_type = uint32_t;

		explicit xorshift32(size_t seed = 0) : a(static_cast<uint32_t>(seed + 1)) { }

		constexpr uint32_t operator()() noexcept {
			uint32_t x = a;
			x ^= x << 13;
			x ^= x >> 17;
			x ^= x << 5;
			return a = x;
		}
#undef min
#undef max
		static constexpr uint32_t min() noexcept { return 1; }
		static constexpr uint32_t max() noexcept { return UINT32_MAX; }
	};

	

	enum InSetState
	{
		IsInSet = 0,
		NotInSet = 1
	};

	class RandomManager
	{
		RandomManager() {};
		~RandomManager() {};
	public:
		RandomManager(RandomManager&& ) = delete;
		xorshift32 Rng{ std::random_device{}() };
		float __forceinline GetRand(float min,float max) { 
			return std::uniform_real_distribution<float>{min,max}(Rng);
		}

		int __forceinline GetRandInt(int min, int max) {
			return std::uniform_int_distribution<int>{min, max}(Rng);
		}

		static __forceinline RandomManager& rand_manager_instance() {
			static RandomManager rand_manager_instance;
			return rand_manager_instance;
		}
	};

#define RandMng RandomManager::rand_manager_instance()

	enum MaterialType
	{
		kMaterial_Stone,
		kMaterial_Dirt,
		kMaterial_Grass,
		kMaterial_Glass,
		kMaterial_Metal,
		kMaterial_Wood,
		kMaterial_Organic,
		kMaterial_Cloth,
		kMaterial_Water,
		kMaterial_HollowMetal,
		kMaterial_OrganicBug,
		kMaterial_OrganicGlow
	};
#define IsMustPenetrateMaterial(type) (type == kMaterial_Grass || type == kMaterial_Cloth || type == kMaterial_Glass || type == kMaterial_Water )
#define NotMustPenetrateMaterial(type) (type != kMaterial_Grass && type != kMaterial_Cloth && type != kMaterial_Glass && type != kMaterial_Water )

	// record penetrate information
	struct Pene_Info {
		Projectile* PenePJ = nullptr;
		TESObjectREFR* impact_ref = nullptr;
		NiVector3 LastHitPos{ 0,0,0 };
		UINT8 Pene_Times = 0;
		UINT32 lastHitMaterial = 0;
		float ExpectDepth = 0;
		float APScore = 0;
		Pene_Info() {};
		Pene_Info(Projectile* _pene_pj, TESObjectREFR* _impact_ref, const NiVector3& _last_hit,UINT32 _hit_material,float expect_depth,float AP_Score,UINT8 pene_times) :
						PenePJ(_pene_pj), impact_ref(_impact_ref), LastHitPos(_last_hit),lastHitMaterial(_hit_material), ExpectDepth(expect_depth), APScore(AP_Score), Pene_Times(pene_times){}
		//Pene_Info(Projectile* _pene_pj, TESObjectREFR* _impact_ref, const NiVector3& _last_hit) :PenePJ(_pene_pj), impact_ref(_impact_ref), LastHitPos(_last_hit) {}
	};

	// record rico information
	struct Rico_Info {
		Projectile* RicoPJ = nullptr;
		TESObjectREFR* impact_ref = nullptr;
		NiVector3 LastHitPos{ 0,0,0 };
		UINT8 Rico_Times = 0;
		Rico_Info() {};
		Rico_Info(Projectile* _rico_pj,TESObjectREFR* _impact_ref,const NiVector3& _hit_pos,UINT8 _rico_times) :
			RicoPJ(_rico_pj), LastHitPos(_hit_pos), impact_ref(_impact_ref), Rico_Times(_rico_times) {}
	};

	static UINT16 bullet_manager_flag = 0;

	enum MngFlag {
		Penetrate = 0x0001,
		Ricochet = 0x0002,
		SpeedCalcForPene = 0x0004,
		LocalizedDTSupport = 0x0008,
		MultiPenetrate = 0x0010,
		MultiRicochet = 0x0020,
		ImpactScaler = 0x0040,
		ExtraDecal = 0x0080
	};


	static __forceinline bool IsFlagOn(UINT16 _flag) {
		return (bullet_manager_flag & _flag) != 0;
	}

	static __forceinline void SetFlagOn(UINT16 _flag) {
		bullet_manager_flag |= _flag;
	}

	struct DP_Info {
		Projectile* dp_proj{nullptr};
		float decal_size{0};
		NiVector3 new_pos{ 0,0,0 };
		DP_Info() {}
		DP_Info(Projectile* _dp_proj,float imp_size, const NiVector3& _new_pj_pos) :dp_proj(_dp_proj), decal_size(imp_size), new_pos(_new_pj_pos) {}
	};

	// record projectiles which do penetrate
	struct DoPeneProjMap {
		std::unordered_map<formid, DP_Info> DPProjMap{};

		__forceinline void GoInDPMap(Projectile* _dp_proj,const NiVector3& _new_pos) {
			if (const auto& dp_iter = DPProjMap.find(_dp_proj->refID); dp_iter != DPProjMap.end()) {
				auto& dp_info = dp_iter->second;
				dp_info.decal_size = _dp_proj->decalSize;
				dp_info.new_pos = _new_pos;
			}
			else DPProjMap.try_emplace(_dp_proj->refID, DP_Info{ _dp_proj,_dp_proj->decalSize,_new_pos });
		}

		__forceinline auto IsInDPMap(const Projectile* _dp_proj) {
			struct ret_dp_info {
				bool find_dp;
				DP_Info* dp_info;

				bool __forceinline Valid() const { return find_dp && dp_info; }
			};
			if (const auto& dp_iter = DPProjMap.find(_dp_proj->refID); dp_iter != DPProjMap.end()) {
				DP_Info* dp_info_ptr = &dp_iter->second;
				if (!dp_info_ptr || !dp_info_ptr->dp_proj) return ret_dp_info{ false,nullptr };
				return ret_dp_info{ true,dp_info_ptr };
			}
			return ret_dp_info{ false,nullptr };
		}

		__forceinline void EraseFromMap(Projectile* _dp_proj) {
			if (const auto& dp_iter = DPProjMap.find(_dp_proj->refID); dp_iter != DPProjMap.end())
				DPProjMap.erase(dp_iter);
		}

		__forceinline bool MatchPos(const NiVector3& _impact_pos ) {
			for (const auto& iter : DPProjMap){
				if (CalcPosSquareDis(_impact_pos, *(iter.second.dp_proj->GetPos())) < 4.0f)
					return true;
			}
			return false;
		}
	};

	class BulletManager
	{
	public:
		BulletManager(BulletManager&&) = delete;
		DoPeneProjMap fake_proj_map{};
		std::unordered_map<formid, Pene_Info> PENEMap{};
		std::unordered_map<formid, Rico_Info> RICOMap{};
		std::array<float, 12> MaterialPenalty{ 0 };		// For Pene
		std::array<float, 12> MaterialChance{ 0 };		// For Rico, Ricochet Chance Only Apply To Ricochet
		
		__forceinline auto& GetDPProjMap() {
			return fake_proj_map;
		}

		__forceinline void MarkAsDP(Projectile* _dp_proj,const NiVector3& _new_pos) {
			GetDPProjMap().GoInDPMap(_dp_proj,_new_pos);
		}

		__forceinline auto IsDPProj(Projectile* _dp_proj) {
			return GetDPProjMap().IsInDPMap(_dp_proj);
		}

		bool MatchDPProj(const NiVector3& _impact_pos) {
			return GetDPProjMap().MatchPos(_impact_pos);
		}

		__forceinline auto IsRicoProj(Projectile* _Proj){
			struct rico_ret {
				bool find_rico;
				Rico_Info* rico_info;

				bool __forceinline Valid() const { return find_rico && rico_info; }
			};
			//gLog.FormattedMessage("CheckNotInMap %x", _Proj->refID);
			if (const auto& RicoIter = RICOMap.find(_Proj->refID); RicoIter != RICOMap.end()) {
				Rico_Info* rico_info_ptr = &RicoIter->second;
				if (!rico_info_ptr || !rico_info_ptr->RicoPJ) return rico_ret{ false,nullptr };
				return rico_ret{ true,rico_info_ptr };
			}
			return rico_ret{ false,nullptr };
		}

		__forceinline auto IsPenetrateProj(Projectile* _Proj){
			struct pene_ret {
				bool find_pene;
				Pene_Info* pene_info;

				bool __forceinline Valid() const { return find_pene && pene_info; }
			};
			//gLog.FormattedMessage("CheckNotInMap %x", _Proj->refID);
			if (const auto& PENEIter = PENEMap.find(_Proj->refID); PENEIter != PENEMap.end()) {
				Pene_Info* pene_info_ptr = &PENEIter->second;
				if (!pene_info_ptr || !pene_info_ptr->PenePJ) return pene_ret{ false,nullptr };
				return pene_ret{ true, pene_info_ptr };
			}
			return pene_ret{ false,nullptr };
		}

		__forceinline bool AllowMultiPene(Projectile* _Proj) {
			if (IsFlagOn(MultiPenetrate)) return true;
			if (const auto& pene_ret = IsPenetrateProj(_Proj); pene_ret.Valid()) {
				return pene_ret.pene_info->Pene_Times >= 1;
			}
			return true;
		}

		__forceinline float GetBackwardChance(Projectile* _Proj) {
			if (const auto& pene_ret = IsPenetrateProj(_Proj); pene_ret.Valid()) {
				return pene_ret.pene_info->Pene_Times;
			}
			if (const auto& rico_ret = IsRicoProj(_Proj); rico_ret.Valid()) {
				return rico_ret.rico_info->Rico_Times;
			}
			return 0;
		}

		__forceinline bool AllowMultiRico(Projectile* _Proj) {
			if (IsFlagOn(MultiRicochet)) return true;
			if (const auto& rico_ret = IsRicoProj(_Proj);rico_ret.Valid()) {
				return rico_ret.rico_info->Rico_Times >= 1;
			}
			return true;
		}

		__forceinline bool NotInRicoMap(Projectile* _Proj) {
			if (const auto& RicoIter = RICOMap.find(_Proj->refID); RicoIter != RICOMap.end()) {
				if (!RicoIter->second.RicoPJ) return true;
				return false;
			}
			return true;
		}

		__forceinline bool NotInPeneMap(Projectile* _Proj) {
			if (const auto& PENEIter = PENEMap.find(_Proj->refID); PENEIter != PENEMap.end()) {
				if (!PENEIter->second.PenePJ) return true;
				return false;
			}
			return true;
		}

		__forceinline void GoInPeneMap(Projectile* _Proj, TESObjectREFR* _imp_ref,const NiVector3& _pos,UINT32 material,float expect_depth, float ap_score, UINT8 pene_times){
			if (const auto& PENEIter = PENEMap.find(_Proj->refID); PENEIter != PENEMap.end()) {
				if (auto& pene_info = PENEIter->second; !pene_info.PenePJ) {
					pene_info = Pene_Info{ _Proj,_imp_ref,_pos,material,expect_depth,ap_score,pene_times};
				}
			}
			else PENEMap.try_emplace(_Proj->refID, Pene_Info{ _Proj,_imp_ref,_pos,material,expect_depth,ap_score,pene_times});
		}

		__forceinline void GoInRicoMap(Projectile* _Proj,TESObjectREFR* _imp_ref, const NiVector3& begin_pos, UINT8 pene_times) {
			if (const auto& RicoIter = RICOMap.find(_Proj->refID); RicoIter != RICOMap.end()) {
				if (auto& rico_info = RicoIter->second; !rico_info.RicoPJ) {
					rico_info = Rico_Info{ _Proj,_imp_ref,begin_pos,pene_times };
				}
			}
			else RICOMap.try_emplace(_Proj->refID, Rico_Info{ _Proj,_imp_ref,begin_pos,pene_times });
		}

		__forceinline void EraseFromPeneMap(Projectile* _Proj){
			if (const auto& PENEIter = PENEMap.find(_Proj->refID); PENEIter != PENEMap.end()) 
				PENEMap.erase(PENEIter);
		}

		__forceinline void EraseFromRicoMap(Projectile* _Proj) {
			if (const auto& RicoIter = RICOMap.find(_Proj->refID); RicoIter != RICOMap.end()) 
				RICOMap.erase(RicoIter);
		}

		__forceinline void EraseFromDPMap(Projectile* _do_pene_proj) {
			GetDPProjMap().EraseFromMap(_do_pene_proj);
		}

		static __forceinline BulletManager& bullet_manager_instance() {
			static BulletManager bullet_manager_instance;
			return bullet_manager_instance;
		}


	private:
		__forceinline void InitPeneMaterialPenalty() {
			MaterialPenalty[kMaterial_Stone] = 0.33;
			MaterialPenalty[kMaterial_Dirt] = 0.33;
			MaterialPenalty[kMaterial_Grass] = 1;
			MaterialPenalty[kMaterial_Glass] = 0.9;
			MaterialPenalty[kMaterial_Metal] = 0.5;
			MaterialPenalty[kMaterial_Wood] = 0.75;
			MaterialPenalty[kMaterial_Organic] = 0.85;
			MaterialPenalty[kMaterial_Cloth] = 1;
			MaterialPenalty[kMaterial_Water] = 1;
			MaterialPenalty[kMaterial_HollowMetal] = 0.8;
			MaterialPenalty[kMaterial_OrganicBug] = 0.85;
			MaterialPenalty[kMaterial_OrganicGlow] = 0.85;

		}
		__forceinline void InitRicoMaterialChance() {
			MaterialChance[kMaterial_Stone] = 70;
			MaterialChance[kMaterial_Dirt] = 33;
			MaterialChance[kMaterial_Grass] = 33;
			MaterialChance[kMaterial_Glass] = 0;
			MaterialChance[kMaterial_Metal] = 85;
			MaterialChance[kMaterial_Wood] = 20;
			MaterialChance[kMaterial_Organic] = 0;
			MaterialChance[kMaterial_Cloth] = 0;
			MaterialChance[kMaterial_Water] = 0;
			MaterialChance[kMaterial_HollowMetal] = 70;
			MaterialChance[kMaterial_OrganicBug] = 0;
			MaterialChance[kMaterial_OrganicGlow] = 0;
		}
		BulletManager() { 
			InitPeneMaterialPenalty();
			InitRicoMaterialChance();
		};
		~BulletManager() {};
	};

	static __forceinline bool RandomlyPlayRicoSound(TESObjectREFR* _rico_pj) {
		TESSound* rico_sound = GetClonedTESSoundForRico(0x21f);
		if (!rico_sound) return false;
		_Sound::PlayTESSound(rico_sound, 0x4102u ,_rico_pj);
	}

	static __forceinline bool RandomlyPlayRicoSound(TESObjectREFR* _rico_pj,NiVector3& _pos) {
		TESSound* rico_sound = GetClonedTESSoundForRico(0x21f);
		if (!rico_sound) return false;
		_Sound::PlayTESSound(rico_sound, 0x4102u, _rico_pj, _pos);
		return true;
	}

	static __forceinline bool PJIsExplosion(Projectile* _pj) {
		if (_pj) {
			if (const BGSProjectile* basepj = (BGSProjectile*)_pj->baseForm; (basepj->projFlags & 2) != 0 && (basepj->type & 1) != 0 ) {// Check type and explosion flag
				return true;
			}
		}
		return false;
	}
	 
	static __forceinline TESAmmo* GetCurEqAmmo(const TESObjectWEAP* _Weap,const Actor* _Actor) {
		return ThisStdCall<TESAmmo*>(0x525980,_Weap,_Actor);
	}

	enum BulletType : UINT8{
		None,AP,Normal,Hollow
	};

	struct BulletInfo {
		BulletType type = BulletType::None;
		float AP_Score = 0.0;
	};

	/*
	===== CheckBulletTypeByAmmoEffect =====
	Check Ammo Effect , Only Check EffectType_DTMod
	'AP_Score' represent the AP Effect of the projectile
	'Baseline' is projecile hitdamage,and the final 'AP_Score' will minus 'Baseline'
	So 'AP_Score' can be positive or negative. positive is AP,negative is Hollow, and 'AP_Score' use to calc the depth and damage after penetrate
	*/
	// calc the AP_Score
	static BulletInfo CheckBulletTypeByAmmoEffect(const TESAmmo* _ammo,const Projectile* _proj) {
		if (!_ammo) return BulletInfo{ BulletType::Normal ,0 };
		BulletInfo blt_info;
		const auto& effect_list = _ammo->effectList;

		auto iter = effect_list.Head();
		blt_info.AP_Score = 0;		// greater than baseline is AP,lower than baseline is Hollow
		do
		{
			if (!iter) break;
			if (const auto* effect = iter->data){
				if (effect->type == TESAmmoEffect::kEffectType_DTMod){
					switch (effect->operation)
					{
					case TESAmmoEffect::kOperation_Add:
						blt_info.AP_Score -= effect->value;	// Hollow
						break;
					case TESAmmoEffect::kOperation_Multiply:
						blt_info.AP_Score += (_proj->hitDamage * (1 - effect->value));
						break;
					case TESAmmoEffect::kOperation_Subtract:
						blt_info.AP_Score += effect->value;	// AP
						break;
					default:
						break;
					}
				}
			}
		} while (iter = iter->next);
		blt_info.type = BulletType::Normal;
		if (blt_info.AP_Score > 0) blt_info.type = BulletType::AP;
		else if (blt_info.AP_Score < 0) blt_info.type = BulletType::Hollow;
		return blt_info;
	}

	static __forceinline float GetProjRefSpeed(Projectile* PJRef)
	{
		if (const BGSProjectile* bs_proj = (BGSProjectile*)PJRef->baseForm) {
			return (bs_proj->speed * PJRef->speedMult);
		}
		return 0;
	}

	static __forceinline NiVector3 NewPosBasedDepth_Pene(Projectile* PJRef, float Depth) {
		NiVector3 Direction = PJRef->UnitVector;
		const NiVector3* OriginPos = PJRef->GetPos();
		return NiVector3{ (OriginPos->x + (Direction.x * Depth)) ,(OriginPos->y + (Direction.y * Depth)) ,(OriginPos->z + (Direction.z * Depth)) };
	}

	static __forceinline void SetPosBasedDepth_Backward(Projectile* PJRef, float Depth, NiVector3& new_pos) {
		NiVector3 ReverseDirection = ScaleVector(PJRef->UnitVector, NiVector3{-1,-1,-1});
		const NiVector3* OriginPos = PJRef->GetPos();
		new_pos.x = (OriginPos->x + (ReverseDirection.x * Depth));
		new_pos.y = (OriginPos->y + (ReverseDirection.y * Depth));
		new_pos.z = (OriginPos->z + (ReverseDirection.z * Depth));
	}

	/*
	===== AngleCompensation =====
	deal with the situation when the rot > 3.14
	*/
#define AngleCompensation_Z(x) x -= (Double_PI * (-sgn(x))) 

#define AngleCompensation_Rico_X(x) x = (std::fabs(x) - PI) * sgn(x)
#define AngleCompensation_Pene_X(x) x = (PI - std::fabs(x)) * sgn(x)
#define RandomBackward_Z(x) x  += PI
	

	/*
	===== CalcPenetrateRot =====
	outRX, outRZ should be "ThisProj" current rot.They will be the new penetrate projectile rot
	only when AP_Score less than 0 will do penetrate rotation
	*/
	static __forceinline void CalcPenetrateRot(float &outRX,float &outRZ,float AP_Score) {
		if (AP_Score < 0) {
			float RotMaxX = std::fabs(AP_Score);
			float RotMazZ = RotMaxX;
			if (RotMaxX > penetrate_max_deviationX) RotMaxX = penetrate_max_deviationX;
			if (RotMazZ > penetrate_max_deviationZ) RotMazZ = penetrate_max_deviationZ;
			RotMaxX *= penetrate_deviationX_mult;
			RotMazZ *= penetrate_deviationZ_mult;
			outRX += (RandMng.GetRand(-RotMaxX,RotMaxX)/DeToAg);
			outRZ += (RandMng.GetRand(-RotMazZ,RotMazZ)/ DeToAg);
			if (std::fabs(outRX) > Half_PI) AngleCompensation_Pene_X(outRX);
			if (std::fabs(outRZ) > PI) AngleCompensation_Z(outRZ);
		}
	}

	/*
	===== SetRicoAttribute =====
	Set attributes for the new penetrate ricochet
	Call after RX,RZ Offset have generated
	_RX_Offset,_RZ_Offset -> can use to calc new damage,no fraction
	Set damage and speedmult for ricochet projectile according the rotation offset.
	*/
	static __forceinline void SetRicoAttribute(const float& _RX_Offset,const float& _RZ_Offset,
												float& _nDmg, float& _nSpedMult) {
		float de_dmg_ratio = std::fabs(_RX_Offset) / 90 + std::fabs(_RZ_Offset) / 180;
		if (de_dmg_ratio > 0.7) de_dmg_ratio = 0.7;
		float new_damage = _nDmg * (1 - de_dmg_ratio);
		float new_SpdM = _nSpedMult * std::sqrtf(new_damage / _nDmg);

		_nDmg = new_damage;
		_nSpedMult = new_SpdM;
	}


	/*
	====== Calculate New Rico Proj Stats According "This Proj" Stats ======
	"This Proj" can be a "first hit proj", "penetrate projectile" and "ricochet projectile" 
	ricochet projectile has chance to go backward
	Will calculate rico projectile "new damage","new speedmult","new RX,RZ"
	*/
	[[nodiscard]] static auto NewRicoPJStats(const Projectile* ThisProj,const BGSProjectile* base_pj,float Backward_Chance = 0) {
		struct NewRicoPJInfo {
			bool Rico;
			float newDmg;
			float newSpeedMult;
			float newRX;
			float newRZ;
		};
		float new_spdM = ThisProj->speedMult;
		if (base_pj->speed * new_spdM < ricochet_speed_threshold) return NewRicoPJInfo{false,0,0,0,0};
		float new_pj_dmg = ThisProj->hitDamage;
		float outRX = ThisProj->rotX, outRZ = ThisProj->rotZ;

		if (RandMng.GetRand(0, 100) < 50) outRX *= -1;
		float DeviateAngle_X = RandMng.GetRand(-ricochet_max_rotX, ricochet_max_rotX);
		float DeviateAngle_Z = RandMng.GetRand(-ricochet_max_rotZ, ricochet_max_rotZ);
		SetRicoAttribute(DeviateAngle_X, DeviateAngle_Z, new_pj_dmg, new_spdM);	// set projectile newdmg and newspedM
		
		if (base_pj->speed * new_spdM < ricochet_speed_threshold || 
			new_pj_dmg < ricochet_damage_threshold ) return NewRicoPJInfo{ false,0,0,0,0 };

		outRX += (DeviateAngle_X / DeToAg);
		outRZ += (DeviateAngle_Z / DeToAg);
		if (std::fabs(outRX) > Half_PI) AngleCompensation_Rico_X(outRX);
		if (std::fabs(outRZ) > PI) AngleCompensation_Z(outRZ);
		if (RandMng.GetRand(0, 100) < (5 * Backward_Chance)) { 
			RandomBackward_Z(outRZ); 
			if (std::fabs(outRZ) > PI) AngleCompensation_Z(outRZ);
		}

		return NewRicoPJInfo{ true,new_pj_dmg,new_spdM,outRX,outRZ };
	}
	/*
	===== SetPeneAttribute =====
	Set attributes for the new penetrate projectile
	_nPene_depth - new penetrate depth, init as penetrate_base_depth
	_nAPScore - new AP Score, init as 0
	_nDmg - new penetrate projectile damage
	_dDmg - the d-value
	_oldAPScore - old AP Score
	*/
	static __forceinline void SetPeneAttribute(TESObjectREFR* imp_ref,SInt32 hit_location,
												float& _nPene_depth,float& _nAPScore,float& _nDmg,float& _dDmg,const float& _oldAPScore) {
		_nPene_depth += _oldAPScore;	// Old AP_Score decide depth. In fact it doesnt matter
		_nAPScore += _oldAPScore;		// let New AP_Score = Old AP_Score
		
		if ( IS_ACTOR(imp_ref) ) {
			Actor* actor_ref = (Actor*)imp_ref;
			float Actor_DT = 0;
			if (!((Actor*)imp_ref)->IsCreature()) {			// character
				if (IsFlagOn(LocalizedDTSupport)){
					if (hit_location == 1) Actor_DT = GetHelmatDT(actor_ref);
					else if (hit_location == 14 && actor_ref->IsWeaponOut()) {}
					else Actor_DT = GetArmorDT(actor_ref);
				}
				else Actor_DT = actor_ref->GetTotalArmorDT();
			}
			else Actor_DT = actor_ref->avOwner.GetActorValue(76);	// Creature DT
			
			
			_nAPScore -= Actor_DT;		// Decrease the new AP Score due to Actor_DT
			_nDmg += _nAPScore;			// nDmg will Plus when AP,Minus when Hollow 
			_nDmg *= BulletManager::bullet_manager_instance().MaterialPenalty[kMaterial_Organic];	// mult penalty due to hit an actor
			_nAPScore -= _dDmg;			// AP Score will minus D-value of damage to get New AP Score
			return;
		}
		//kFormType_TESFurniture
		
		_nDmg += _nAPScore;				// nDmg will Plus when AP,Minus when Hollow 
		_nAPScore -= _dDmg;				// AP Score will minus D-value of damage to get New AP Score
		return;
	}

	static void __forceinline SetPeneDepthAccordingImpactRefType(const TESObjectREFR* imp_ref,float& nPene_depth) {
		if (nPene_depth < penetrate_base_depth) nPene_depth = penetrate_base_depth;
		else if (nPene_depth > penetrate_max_depth) nPene_depth = penetrate_max_depth;

		if (!imp_ref) return;
		else if (IS_ACTOR(imp_ref)) nPene_depth = penetrate_human_base_depth;
		//else if (IS_TYPE(imp_ref, TESFurniture)) nPene_depth = penetrate_furniture_depth;
	}

	/*
	====== Calculate New Proj Stats According "This Proj" Stats ======
	"This Proj" can be a "first hit proj" or a "penetrate projectile"
	Return a struct which has information about "penetrate or not" , "new damage" and "new speedmult" of new spawn projectile
	Damage will scale by the global setting (dmg_decrease_percent) at first,And then scale by the MaterialPenalty
	Projectile speed will be considered as a factor in "penetrate or not" when SpeedCalcForPene flag on

	Needs to create pene_info first.This function will fill the pene_info
	*/
	[[nodiscard]] static auto NewPenePJStats(const Projectile* ThisProj,TESObjectREFR* imp_ref,const Actor* PJSrc,const TESObjectWEAP* bs_weap,
							const BGSProjectile* bs_proj,SInt32 hit_location,UINT32 impact_material,UINT32 pene_times = 0,float OldAPScore = 0) {
		struct NewPenePJInfo {
			bool Pene;
			float newDmg;
			float newSpeedMult;
			float penetrate_depth;
			float newAPScore;
		};
		float nDmg = ThisProj->hitDamage;
		// actor will not calculate as material 
		if (!IS_ACTOR(imp_ref)) nDmg *= (impact_material < 12 ? BulletManager::bullet_manager_instance().MaterialPenalty[impact_material] : 1.0f);
		float nPene_depth = penetrate_base_depth;
		float dDmg = ThisProj->hitDamage - nDmg;
		float nAPScore = 0;
		//gLog.FormattedMessage("impact_material %u,pene_times %u, OldAPScore %.2f, PjHitdmg %.2f", impact_material, pene_times, OldAPScore, ThisProj->hitDamage);

		if (pene_times == 0){	// When Pene_Times is 0 means it is the first hit ,it needs to calc the ap_score
			BulletInfo bullet_info = CheckBulletTypeByAmmoEffect(GetCurEqAmmo(bs_weap, PJSrc), ThisProj);
			SetPeneAttribute(imp_ref, hit_location,nPene_depth, nAPScore, nDmg, dDmg, bullet_info.AP_Score);
		}
		else {
			SetPeneAttribute(imp_ref, hit_location,nPene_depth, nAPScore, nDmg, dDmg, OldAPScore);
		}

		if (nDmg < penetrate_dmg_threshold) return NewPenePJInfo{ false,0,0,0,0 };
		if (nDmg > ThisProj->hitDamage) nDmg = ThisProj->hitDamage;
		if (nDmg > penetrate_ap_dmg_threshold) {
			float extra_ap_ability = nDmg - penetrate_ap_dmg_threshold;
			nAPScore += extra_ap_ability;
			nPene_depth += extra_ap_ability;
		};

		SetPeneDepthAccordingImpactRefType(imp_ref, nPene_depth);
		if (nAPScore > 0) nAPScore *= penetrate_ap_buffscale;
		else if (nAPScore < 0) nAPScore *= penetrate_hollow_buffscale;

		float SpeedScale = nDmg / ThisProj->hitDamage;
		float nSpdM = ThisProj->speedMult * std::sqrtf(SpeedScale);
		
		bool DoPenetrate = true;
		if (nAPScore <= 0 && NotMustPenetrateMaterial(impact_material) ){
			if (RandMng.GetRand(0, 100) > (penetrate_base_chance + nAPScore))DoPenetrate = false;	// Less AP_Score, Less Chance to do penetration
		}
		if (IsFlagOn(MngFlag::SpeedCalcForPene))
			return NewPenePJInfo{ (DoPenetrate && nDmg > penetrate_dmg_threshold && (bs_proj->speed * nSpdM) > penetrate_speed_threshold),nDmg,nSpdM,nPene_depth,nAPScore };
		else
			return NewPenePJInfo{(DoPenetrate && nDmg > penetrate_dmg_threshold),nDmg,nSpdM,nPene_depth,nAPScore };
	}
	
	namespace fs = std::filesystem;
	static inline bool ReadGenericConfig() {
		gLog.Message("ReadGenericConfig");
		fs::path config_root_path = fs::current_path();
		config_root_path += R"(\Data\NVSE\Plugins\PeneAndRicoConfig\)";
		if (!fs::exists(config_root_path)) {
			gLog.Message("ReadGenericConfig path not exist");
			return false;
		}

		roughinireader::INIReader _ini{ config_root_path };

		auto ret = _ini.SetCurrentINIFileName("PeneAndRico.ini");
		if (!ret.has_value()) {
			gLog.FormattedMessage("Failed to set generic config filename : %s", ret.error().message());
			return false;
		}
		ret = _ini.ConstructSectionMap();
		if (!ret.has_value()) {
			gLog.FormattedMessage("Failed to construct section map : %s", ret.error().message());
			return false;
		}

		// init the generic config
		std::string raw_type_val = "";
		UINT16 temp_flag = 0;
		raw_type_val = _ini.GetRawTypeVal("PenetrateGeneral", "MutiPenetrate");
		temp_flag = raw_type_val.empty() ? 0 : static_cast<UINT16>(std::stoi(raw_type_val));
		if (temp_flag > 0) SetFlagOn(MultiPenetrate);

		raw_type_val = _ini.GetRawTypeVal("PenetrateGeneral", "SpeedCalculatedForPenetrate");
		temp_flag = raw_type_val.empty() ? 0 : static_cast<UINT16>(std::stoi(raw_type_val));
		if (temp_flag > 0) SetFlagOn(SpeedCalcForPene);

		raw_type_val = _ini.GetRawTypeVal("PenetrateGeneral", "LocalizedDTSupport");
		temp_flag = raw_type_val.empty() ? 0 : static_cast<UINT16>(std::stoi(raw_type_val));
		if (temp_flag > 0) SetFlagOn(LocalizedDTSupport);

		raw_type_val = _ini.GetRawTypeVal("RicochetGeneral", "Ricochet");
		temp_flag = raw_type_val.empty() ? 0 : static_cast<UINT16>(std::stoi(raw_type_val));
		if (temp_flag > 0) SetFlagOn(Ricochet);

		raw_type_val = _ini.GetRawTypeVal("RicochetGeneral", "MutiRicochet");
		temp_flag = raw_type_val.empty() ? 0 : static_cast<UINT16>(std::stoi(raw_type_val));
		if (temp_flag > 0) SetFlagOn(MultiRicochet);

		raw_type_val = _ini.GetRawTypeVal("ImpactScaler", "Enable");
		temp_flag = raw_type_val.empty() ? 0 : static_cast<UINT16>(std::stoi(raw_type_val));
		if (temp_flag > 0) SetFlagOn(ImpactScaler);

		raw_type_val = _ini.GetRawTypeVal("ImpactScaler", "ExtraDecal");
		temp_flag = raw_type_val.empty() ? 0 : static_cast<UINT16>(std::stoi(raw_type_val));
		if (temp_flag > 0) SetFlagOn(ExtraDecal);

		gLog.FormattedMessage("flag %u", bullet_manager_flag);

		raw_type_val = _ini.GetRawTypeVal("PenetrateGeneral", "DamageThreshold");
		penetrate_dmg_threshold = raw_type_val.empty() ? 12 : (std::stof(raw_type_val));

		raw_type_val = _ini.GetRawTypeVal("PenetrateGeneral", "BasePenetrateChance");
		penetrate_base_chance = raw_type_val.empty() ? 80 : (std::stof(raw_type_val));

		raw_type_val = _ini.GetRawTypeVal("PenetrateGeneral", "BasePenetrateDepth");
		penetrate_base_depth = raw_type_val.empty() ? 16 : (std::stof(raw_type_val));

		raw_type_val = _ini.GetRawTypeVal("PenetrateGeneral", "MaxPenetrateDepth");
		penetrate_max_depth = raw_type_val.empty() ? 48 : (std::stof(raw_type_val));

		raw_type_val = _ini.GetRawTypeVal("PenetrateGeneral", "HumanBeingPenetrateDepth");
		penetrate_human_base_depth = raw_type_val.empty() ? 32 : (std::stof(raw_type_val));

		raw_type_val = _ini.GetRawTypeVal("PenetrateGeneral", "SpeedThreshold");
		penetrate_speed_threshold = raw_type_val.empty() ? 21760 : (std::stof(raw_type_val));

		raw_type_val = _ini.GetRawTypeVal("PenetrateGeneral", "VerticalDeviationMult");
		penetrate_deviationX_mult = raw_type_val.empty() ? 1.0 : (std::stof(raw_type_val));

		raw_type_val = _ini.GetRawTypeVal("PenetrateGeneral", "HorizontalDeviationMult");
		penetrate_deviationZ_mult = raw_type_val.empty() ? 1.0 : (std::stof(raw_type_val));

		raw_type_val = _ini.GetRawTypeVal("PenetrateGeneral", "MaxVerticalDeviation");
		penetrate_max_deviationX = raw_type_val.empty() ? 10 : (std::stof(raw_type_val));
		
		raw_type_val = _ini.GetRawTypeVal("PenetrateGeneral", "MaxHorizontalDeviation");
		penetrate_max_deviationZ = raw_type_val.empty() ? 30 : (std::stof(raw_type_val));

		raw_type_val = _ini.GetRawTypeVal("PenetrateGeneral", "APDamageThreshold");
		penetrate_ap_dmg_threshold = raw_type_val.empty() ? 36 : (std::stof(raw_type_val));

		raw_type_val = _ini.GetRawTypeVal("PenetrateGeneral", "APBuffscale");
		penetrate_ap_buffscale = raw_type_val.empty() ? 1.0 : (std::stof(raw_type_val));

		raw_type_val = _ini.GetRawTypeVal("PenetrateGeneral", "HollowBuffScale");
		penetrate_hollow_buffscale = raw_type_val.empty() ? 1.0 : (std::stof(raw_type_val));
// ricochet config
		raw_type_val = _ini.GetRawTypeVal("RicochetGeneral", "SpeedThreshold");
		ricochet_speed_threshold = raw_type_val.empty() ? 6400 : (std::stof(raw_type_val));
		
		raw_type_val = _ini.GetRawTypeVal("RicochetGeneral", "DamageThreshold");
		ricochet_damage_threshold = raw_type_val.empty() ? 6 : (std::stof(raw_type_val));

		raw_type_val = _ini.GetRawTypeVal("RicochetGeneral", "MaxVerticalDeviation");
		ricochet_max_rotX = raw_type_val.empty() ? 30 : (std::stof(raw_type_val));

		raw_type_val = _ini.GetRawTypeVal("RicochetGeneral", "MaxHorizontalDeviation");
		ricochet_max_rotZ = raw_type_val.empty() ? 60 : (std::stof(raw_type_val));

		raw_type_val = _ini.GetRawTypeVal("ImpactScaler", "BaseDamage");
		impact_scaler_dmg_baseline = raw_type_val.empty() ? 30 : (std::stof(raw_type_val));

		raw_type_val = _ini.GetRawTypeVal("ImpactScaler", "MinDamage");
		impact_scaler_dmg_minline = raw_type_val.empty() ? 10 : (std::stof(raw_type_val));

		raw_type_val = _ini.GetRawTypeVal("ImpactScaler", "MaxDamage");
		impact_scaler_dmg_maxline = raw_type_val.empty() ? 60 : (std::stof(raw_type_val));

		raw_type_val = _ini.GetRawTypeVal("ImpactScaler", "MinScaler");
		impact_scaler_min = raw_type_val.empty() ? 0.5 : (std::stof(raw_type_val));

		raw_type_val = _ini.GetRawTypeVal("ImpactScaler", "MaxScaler");
		impact_scaler_max = raw_type_val.empty() ? 2.0 : (std::stof(raw_type_val));

		impact_scaler_dmg_dmin_base = std::fabs(impact_scaler_dmg_baseline - impact_scaler_dmg_minline);
		impact_scaler_dmg_dmax_base = std::fabs(impact_scaler_dmg_maxline - impact_scaler_dmg_baseline);

		raw_type_val = _ini.GetRawTypeVal("PenetrateMaterialPenalty", "Stone");
		BulletManager::bullet_manager_instance().MaterialPenalty[kMaterial_Stone] = raw_type_val.empty() ? 0.33 : (std::stof(raw_type_val));
		raw_type_val = _ini.GetRawTypeVal("PenetrateMaterialPenalty", "Dirt");
		BulletManager::bullet_manager_instance().MaterialPenalty[kMaterial_Dirt] = raw_type_val.empty() ? 0.33 : (std::stof(raw_type_val));
		raw_type_val = _ini.GetRawTypeVal("PenetrateMaterialPenalty", "Grass");
		BulletManager::bullet_manager_instance().MaterialPenalty[kMaterial_Grass] = raw_type_val.empty() ? 1 : (std::stof(raw_type_val));
		raw_type_val = _ini.GetRawTypeVal("PenetrateMaterialPenalty", "Glass");
		BulletManager::bullet_manager_instance().MaterialPenalty[kMaterial_Glass] = raw_type_val.empty() ? 0.9 : (std::stof(raw_type_val));
		raw_type_val = _ini.GetRawTypeVal("PenetrateMaterialPenalty", "Metal");
		BulletManager::bullet_manager_instance().MaterialPenalty[kMaterial_Metal] = raw_type_val.empty() ? 0.5 : (std::stof(raw_type_val));
		raw_type_val = _ini.GetRawTypeVal("PenetrateMaterialPenalty", "Wood");
		BulletManager::bullet_manager_instance().MaterialPenalty[kMaterial_Wood] = raw_type_val.empty() ? 0.75 : (std::stof(raw_type_val));
		raw_type_val = _ini.GetRawTypeVal("PenetrateMaterialPenalty", "Organic");
		BulletManager::bullet_manager_instance().MaterialPenalty[kMaterial_Organic] = raw_type_val.empty() ? 0.85 : (std::stof(raw_type_val));
		raw_type_val = _ini.GetRawTypeVal("PenetrateMaterialPenalty", "Cloth");
		BulletManager::bullet_manager_instance().MaterialPenalty[kMaterial_Cloth] = raw_type_val.empty() ? 1 : (std::stof(raw_type_val));
		raw_type_val = _ini.GetRawTypeVal("PenetrateMaterialPenalty", "Water");
		BulletManager::bullet_manager_instance().MaterialPenalty[kMaterial_Water] = raw_type_val.empty() ? 1 : (std::stof(raw_type_val));
		raw_type_val = _ini.GetRawTypeVal("PenetrateMaterialPenalty", "HollowMetal");
		BulletManager::bullet_manager_instance().MaterialPenalty[kMaterial_HollowMetal] = raw_type_val.empty() ? 0.8 : (std::stof(raw_type_val));
		raw_type_val = _ini.GetRawTypeVal("PenetrateMaterialPenalty", "OrganicBug");
		BulletManager::bullet_manager_instance().MaterialPenalty[kMaterial_OrganicBug] = raw_type_val.empty() ? 0.85 : (std::stof(raw_type_val));
		raw_type_val = _ini.GetRawTypeVal("PenetrateMaterialPenalty", "OrganicGlow");
		BulletManager::bullet_manager_instance().MaterialPenalty[kMaterial_OrganicGlow] = raw_type_val.empty() ? 0.85 : (std::stof(raw_type_val));

		raw_type_val = _ini.GetRawTypeVal("RicochetMaterialChance", "Stone");
		BulletManager::bullet_manager_instance().MaterialChance[kMaterial_Stone] = raw_type_val.empty() ? 70 : (std::stof(raw_type_val));
		raw_type_val = _ini.GetRawTypeVal("RicochetMaterialChance", "Dirt");
		BulletManager::bullet_manager_instance().MaterialChance[kMaterial_Dirt] = raw_type_val.empty() ? 33 : (std::stof(raw_type_val));
		raw_type_val = _ini.GetRawTypeVal("RicochetMaterialChance", "Grass");
		BulletManager::bullet_manager_instance().MaterialChance[kMaterial_Grass] = raw_type_val.empty() ? 33 : (std::stof(raw_type_val));
		raw_type_val = _ini.GetRawTypeVal("RicochetMaterialChance", "Glass");
		BulletManager::bullet_manager_instance().MaterialChance[kMaterial_Glass] = raw_type_val.empty() ? 0 : (std::stof(raw_type_val));
		raw_type_val = _ini.GetRawTypeVal("RicochetMaterialChance", "Metal");
		BulletManager::bullet_manager_instance().MaterialChance[kMaterial_Metal] = raw_type_val.empty() ? 85 : (std::stof(raw_type_val));
		raw_type_val = _ini.GetRawTypeVal("RicochetMaterialChance", "Wood");
		BulletManager::bullet_manager_instance().MaterialChance[kMaterial_Wood] = raw_type_val.empty() ? 20 : (std::stof(raw_type_val));
		raw_type_val = _ini.GetRawTypeVal("RicochetMaterialChance", "Organic");
		BulletManager::bullet_manager_instance().MaterialChance[kMaterial_Organic] = raw_type_val.empty() ? 0 : (std::stof(raw_type_val));
		raw_type_val = _ini.GetRawTypeVal("RicochetMaterialChance", "Cloth");
		BulletManager::bullet_manager_instance().MaterialChance[kMaterial_Cloth] = raw_type_val.empty() ? 0 : (std::stof(raw_type_val));
		raw_type_val = _ini.GetRawTypeVal("RicochetMaterialChance", "Water");
		BulletManager::bullet_manager_instance().MaterialChance[kMaterial_Water] = raw_type_val.empty() ? 0 : (std::stof(raw_type_val));
		raw_type_val = _ini.GetRawTypeVal("RicochetMaterialChance", "HollowMetal");
		BulletManager::bullet_manager_instance().MaterialChance[kMaterial_HollowMetal] = raw_type_val.empty() ? 70 : (std::stof(raw_type_val));
		raw_type_val = _ini.GetRawTypeVal("RicochetMaterialChance", "OrganicBug");
		BulletManager::bullet_manager_instance().MaterialChance[kMaterial_OrganicBug] = raw_type_val.empty() ? 0 : (std::stof(raw_type_val));
		raw_type_val = _ini.GetRawTypeVal("RicochetMaterialChance", "OrganicGlow");
		BulletManager::bullet_manager_instance().MaterialChance[kMaterial_OrganicGlow] = raw_type_val.empty() ? 0 : (std::stof(raw_type_val));

		gLog.FormattedMessage("penetrate_dmg_threshold %f", penetrate_dmg_threshold);
		gLog.FormattedMessage("penetrate_base_depth %f", penetrate_base_depth);
		gLog.FormattedMessage("penetrate_max_depth %f", penetrate_max_depth);
		gLog.FormattedMessage("penetrate_human_base_depth %f", penetrate_human_base_depth);
		gLog.FormattedMessage("penetrate_speed_threshold %f", penetrate_speed_threshold);
		gLog.FormattedMessage("penetrate_deviationX_mult %f", penetrate_deviationX_mult);
		gLog.FormattedMessage("penetrate_deviationZ_mult %f", penetrate_deviationZ_mult);
		gLog.FormattedMessage("penetrate_max_deviationX %f", penetrate_max_deviationX);
		gLog.FormattedMessage("penetrate_max_deviationZ %f", penetrate_max_deviationZ);
		gLog.FormattedMessage("ricochet_speed_threshold %f", ricochet_speed_threshold);
		gLog.FormattedMessage("ricochet_damage_threshold %f", ricochet_damage_threshold);
		gLog.FormattedMessage("ricochet_max_rotX %f", ricochet_max_rotX);
		gLog.FormattedMessage("ricochet_max_rotZ %f", ricochet_max_rotZ);
		gLog.FormattedMessage("penetrate_ap_buffscale %f", penetrate_ap_buffscale);
		gLog.FormattedMessage("impact_scaler_dmg_baseline %f", impact_scaler_dmg_baseline);
		gLog.FormattedMessage("impact_scaler_dmg_minline %f", impact_scaler_dmg_minline);
		gLog.FormattedMessage("impact_scaler_dmg_maxline %f", impact_scaler_dmg_maxline);
		gLog.FormattedMessage("impact_scaler_min %f", impact_scaler_min);
		gLog.FormattedMessage("impact_scaler_max %f", impact_scaler_max);
		gLog.FormattedMessage("impact_scaler_dmg_dmin_base %f", impact_scaler_dmg_dmin_base);
		gLog.FormattedMessage("impact_scaler_dmg_dmax_base %f", impact_scaler_dmg_dmax_base);

		//for (auto iter = BulletManager::bullet_manager_instance().MaterialPenalty.begin(); iter != BulletManager::bullet_manager_instance().MaterialPenalty.end(); iter++)
		//{
		//	gLog.FormattedMessage("MP %f",*iter);
		//} 
		return true;
	}

};
