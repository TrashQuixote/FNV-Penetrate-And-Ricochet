#pragma once

#include "Gathering_Utility.h"
#include "Gathering_Node.h"
#include "RoughINIReader.h"

namespace PenetrateAndRicochet {
	/*
===== AngleCompensation =====
deal with the situation when the rot > 3.14
*/
#define AngleCompensation_Z(x) x -= (Double_PI * (-sgn(x))) 



#define AngleCompensation_Rico_X(x) x = (std::fabs(x) - PI) * sgn(x)
#define AngleCompensation_Pene_X(x) x = (PI - std::fabs(x)) * sgn(x)
#define RandomBackward_Z(x) x  += PI
	
	
	
	using formid = UInt32;
	// Projectiles' hit damage lower than this value will not do penetration.default (12)
	static float penetrate_dmg_threshold = 12;
	// Projectiles' speed lower( higher ) than this value will decrease( increase ) AP ability of projectile
	// Works only when SpeedCalculatedForPenetrate = 1..default (21760)
	static float penetrate_speed_threshold = 21760;
	// For every step increase in speed, the AP score will increase
	// eg. speed of projectile is 30000, then AP score buff is ( ( 30000 - SpeedThreshold ) / SpeedIncreaseStep )
	static float speed_increase_step = 1000;
	// For every step decrease in speed, the AP score will increase
	// eg. speed of projectile is 20000, then AP score buff is ( ( SpeedThreshold -  20000 ) / SpeedDecreaseStep )
	static float speed_decrease_step = 500;
	// (Unuse) Base Penetrate Depth.Not recommend set to low.default (16)
	static float penetrate_base_depth = 16;
	static float penetrate_furniture_depth = 4;
	/*
	(New in 1.4) Penetrated depth will consider as a factor to decrease AP score
	The greater the penetrated depth, the higher the AP ability reduction
	I use a power function for regulation, the DepthFactor will act as a part of power on the power function
	So, don't set too high or you will penetrate nothing

	power function: APScore Debuff = depth ^ ( 1 + penetrate_depth_factor * ( 1 - PenetrateMaterialPenalty) )
	*/
	static float penetrate_depth_factor = 0.1;
	/*
	The penetrate depth of human being.Independent of the penetrate depth settings above.
	Allows to hit the same character twice in one shot from a specific Angle. Dont set too low or too high.Recommend keep default (32)
	*/
	static float penetrate_human_base_depth = 32;
	// (Unuse) Base Penetrate Depth. Not recommend set to high.default (48)
	static float penetrate_max_depth = 48;
	// Projectile will deviate after penetrate
	// Deviation depends on the ability of DT penetration of the projectile. Unit: Angle
	// Set DeviationMult to 0 means will not do deviation in specific direction
	static float penetrate_deviationX_mult = 1;
	static float penetrate_max_deviationX = 10;
	static float penetrate_deviationZ_mult = 1;
	static float penetrate_max_deviationZ = 30;
	/*
	Projectile has DT penetrate( Ammo Effect ) will regard as AP and it must do penetration.
	In this plugin, AP ability of projectiles will decreases with each penetration
	This setting for the projectiles which have less DT penetrate ability.Will try to do ricochet when fail to penetrate.
	Less DT penetrate less chance to penetrate
	*/
	static float penetrate_base_chance = 80;
	/*
	Projectiles' hit damage greater than this value will regard as AP Projectile.default (36)
	If you would like to disable this feature, set it to a very high value which can not reach in your game
	But dont set too high or it will overflow :)
	*/
	static float penetrate_ap_dmg_threshold = 36;
	/*
	Added by 1.2 - AP(Hollow) Buff mult
	Lower than 1 will decharacterize bullets ( AP bullet will less AP ability and Hollow bullet will have more chance to penetrate )
	Higher than 1 wiill enhance bullets' feature( AP bullet will increase AP ability and Hollow bullet will less chance to penetrate )
	Scaling will occur in each iteration of the AP ability calculation so you need to handle this value properly
	*/
	static float penetrate_hollow_buffscale = 1.0;
	static float penetrate_ap_buffscale = 1.0;

// ricochet (not armor ricochet)
	/*
	The Deviate amount of ricochet bullets
	Damage of ricochet will scale according the deviation.Larger deviation means less damage
	*/
	static float ricochet_max_rotX = 30;
	static float ricochet_max_rotZ = 60;
	// Projectiles' speed lower than this value will not do ricochet.default (6400)
	static float ricochet_speed_threshold = 6400;
	// Projectiles' hit damage lower than this value will not do ricochet.default (6)
	static float ricochet_damage_threshold = 6;

// impact scaler
	static float impact_scaler_dmg_minline = 10;
	static float impact_scaler_dmg_dmin_base = 20;
	static float impact_scaler_dmg_baseline = 30;
	static float impact_scaler_dmg_dmax_base = 30;
	static float impact_scaler_dmg_maxline = 60;
	static float impact_scaler_min = 0.5;
	static float impact_scaler_max = 2.0;

	namespace ARMORicochet {
		static float ricochet_ar_hl_chance = 0;
		
		void SetArmorRicochetBaseChance(float _nVal) {
			ricochet_ar_hl_chance = _nVal;
		}
		
		float GetArmorRicochetBaseChance() {
			return ricochet_ar_hl_chance;
		}
		
		enum ArmorSlot {
			Armor,Helmet,None
		};
		
		enum BuffIndex : UINT32 {
			mult, type, lucky, light, mid, heavy, metal, power_armor, dr_buff, dt_buff, zero,all
		};
		/*
		mult : multiple of ricochet chance
		type : base ricochet chance according to type (armor or helmet)
		lucky : lucky buff on ricochet chance ( how much chance will add to the final result per lucky )
		light : light armor buff on ricochet chance
		mid : mid armor buff on ricochet chance
		heavy : heavy armor buff on ricochet chance
		metal : metal flag buff on ricochet chance
		power_armor : PA flag buff on ricochet chance
		dr_buff : DR buff on ricochet chance ( how much chance will add to the final result per DR )
		dt_buff : DT buff on ricochet chance ( how much chance will add to the final result per DT )
		*/
		static std::array<float,all> armor_ricochet_buff {0};
		static std::array<float,all> helmet_ricochet_buff {0};
		/*
		mult : multiple of ricochet chance
		type : base ricochet chance according to type (armor or helmet)
		lucky : lucky buff on ricochet chance ( how much chance will add to the final result per lucky )
		light : light armor buff on ricochet chance
		mid : mid armor buff on ricochet chance
		heavy : heavy armor buff on ricochet chance
		metal : metal flag buff on ricochet chance
		power_armor : PA flag buff on ricochet chance
		dr_buff : DR buff on ricochet chance ( how much chance will add to the final result per DR )
		dt_buff : DT buff on ricochet chance ( how much chance will add to the final result per DT )
		*/
#define RicochetBuff(_armor_type,_buff_index) _armor_type##_ricochet_buff.at(_buff_index) 

		/*
		CalcNativeRicochetChance
		As 'native' implies, this function more likely use on calculating dr/dt of creature which no armor wearing.
		ArmorSlot is use to distinguish the hit location ,getting the buff of ricochet chance in array, even through the calculation is apply to creature
		Material, attribute Luck, DR/DT are contribute to ricochet chance computing
		*/
		float CalcNativeRicochetChance(ArmorSlot _type, float _lucky,bool isMetal,float cur_DR,float cur_DT) {
			if (_type == Armor) {
				return ricochet_ar_hl_chance +
					RicochetBuff(armor, mult) * (
						(RicochetBuff(armor, lucky) * _lucky) +
						(RicochetBuff(armor, metal) * isMetal) +
						(RicochetBuff(armor, dr_buff) * cur_DR) + (RicochetBuff(armor, dt_buff) * cur_DT)
						);
			}
			else if (_type == Helmet) {
				return ricochet_ar_hl_chance +
					RicochetBuff(helmet, mult) * (
						(RicochetBuff(helmet, lucky) * _lucky) +
						(RicochetBuff(helmet, metal) * isMetal) +
						(RicochetBuff(helmet, dr_buff) * cur_DR) + (RicochetBuff(helmet, dt_buff) * cur_DT)
						);
			}
		}

		/*
		CalcArmorRicochetChance
		As 'armor' implies, this function more likely use on calculating dr/dt of character which have armor wearing.
		ArmorSlot is use to distinguish the hit location and get the buff of ricochet chance in array
		Material, attribute Luck, DR/DT are contribute to ricochet chance computing
		Power Armor Buff is independent of ArmorClass.Another word, wearing PA will only apply PA Buff
		*/
		float CalcArmorRicochetChance(ArmorSlot _type, ArmorClass _armor_class,
										bool isPA,bool isMetal,float _lucky,
										float cur_DR,float cur_DT ) {
			BuffIndex armor_buff_index = zero;
			switch (_armor_class) {
			case ArmorClass::Light:
				armor_buff_index = BuffIndex::light;
				break;
			case ArmorClass::Medium:
				armor_buff_index = BuffIndex::mid;
				break;
			case ArmorClass::Heavy:
				armor_buff_index = BuffIndex::heavy;
				break;
			}
			

			if (_type == Armor){
				return ricochet_ar_hl_chance +
					 RicochetBuff(armor, mult) * (
					(RicochetBuff(armor, lucky) * _lucky) +
					 RicochetBuff(armor,type) + 
					(RicochetBuff(armor, armor_buff_index) * (!isPA)) + 
					((RicochetBuff(armor, metal) * isMetal) * (!isPA)) +
					(RicochetBuff(armor, power_armor) * isPA) +
					(RicochetBuff(armor, dr_buff) * cur_DR) + 
					(RicochetBuff(armor, dt_buff) * cur_DT)
					);
			}
			else if (_type == Helmet) {
				return ricochet_ar_hl_chance +
					 RicochetBuff(helmet, mult) * (
					(RicochetBuff(helmet, lucky) * _lucky) +
					 RicochetBuff(helmet, type) + 
					(RicochetBuff(helmet, armor_buff_index) * (!isPA))+
					((RicochetBuff(helmet, metal) * isMetal) * (!isPA))+
					(RicochetBuff(helmet, power_armor) * isPA) +
					(RicochetBuff(helmet, dr_buff) * cur_DR) + 
					(RicochetBuff(helmet, dt_buff) * cur_DT)
					);
			}
			return 0;
		}

		/*
		GetHitArmorObjInfo
		get current equipped armor according hit loc
		*/
		static auto GetHitArmorObjInfo(Actor* hit_actor,SInt32 hit_location) {
			struct HitArmorInfo {
				ArmorSlot slot;
				TESObjectARMO* armo;
			};
			
			HitLocation h_loc = GetHitLimbLocation(hit_location);
			switch (h_loc)
			{
			case HitLocation::Head:
				return HitArmorInfo{ArmorSlot::Helmet,GetEquippedHelmetObj(hit_actor) };
			case HitLocation::Torso:
			case HitLocation::Arm:
			case HitLocation::Leg:
				return HitArmorInfo{ ArmorSlot::Armor,GetEquippedArmorObj(hit_actor) };
			case HitLocation::HL_None:
			case HitLocation::Weapon:
			default:
				return HitArmorInfo{ ArmorSlot::None,nullptr };
			}
		}

		static void ArmorRicoDebug() {
			__WriteLog("Print Armor Ricochet Buff");
			for (const auto& elem : armor_ricochet_buff){
				__WriteLog("elem: %.2f", elem);
			}
			__WriteLog("Print Helmet Ricochet Buff");
			for (const auto& elem : armor_ricochet_buff) {
				__WriteLog("elem: %.2f", elem);
			}
		}
	};
// armor ricochet
#define SetRicochetBuff(_armor_type,_buff_index,_new_value) ARMORicochet::_armor_type##_ricochet_buff.at(ARMORicochet::BuffIndex::_buff_index) = _new_value 	

	struct ImpactRefInfo {
		bool _impactref_is_actor;
		bool _impactref_is_creature;
		// the hit material here is converted 
		UINT8 _hit_material;
		SInt32 _hit_location;
		ActorDTDR _dtdr;

		__forceinline bool isHitActor() const {
			return _impactref_is_actor || _impactref_is_creature;
		}

		__forceinline bool isHeadShot() const {
			switch (_hit_location)
			{
			case 1:
			case 2:
			case 13:
				return true;
			default:
				return false;
			}
		}
	};

	static __forceinline auto GetActorDTDR(Actor* _actor) {
		if (_actor->IsCreature()) {			// character
			return GetCreatureDTDR(_actor);
		}
		else return GetCharacterDTDR(_actor);
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
		// get random number, return float
		float __forceinline GetRand(float min,float max) { 
			return std::uniform_real_distribution<float>{min,max}(Rng);
		}

		// get random number, return int
		int __forceinline GetRandInt(int min, int max) {
			return std::uniform_int_distribution<int>{min, max}(Rng);
		}

		static __forceinline RandomManager& rand_manager_instance() {
			static RandomManager rand_manager_instance;
			return rand_manager_instance;
		}
	};

#define RandMng RandomManager::rand_manager_instance()


	static __forceinline int RayCastEval(const Projectile* _proj,float rot_x_1,float rot_x_2,float rot_z_1,float rot_z_2) {
		float _dis_0 = GetRayCastRange(_proj, rot_x_1, rot_z_1);
		float _dis_1 = GetRayCastRange(_proj, rot_x_2, rot_z_1);
		float _dis_2 = GetRayCastRange(_proj, rot_x_1, rot_z_2);
		float _dis_3 = GetRayCastRange(_proj, rot_x_2, rot_z_2);
		__m128 distance = _mm_set_ps(_dis_3, _dis_2, _dis_1, _dis_0);
		__m128 ricoMinDis = _mm_set_ps(768.0f, 768.0f, 768.0f, 768.0f);
		__m128 compRes = _mm_cmplt_ps(ricoMinDis, distance);
		int mask_bits = _mm_movemask_ps(compRes);
	}

	// new in 1.4
	static __forceinline auto EvalRicoTrajectory(const Projectile* _proj,
													const NiVector3& _beginPos,
													float rot_X, float rot_Z,
													float deviate_X, float deviate_Z,
													float backward_Chance) {
		/*__WriteLog("RotX: %.5f, RotZ: %.5f, DeviateX: %.5f, DeviateZ: %.5f",
								rot_X * DeToAg , rot_Z * DeToAg, deviate_X * DeToAg, deviate_Z * DeToAg);*/

		struct {
			bool hasTrajectory = false;
			float newRotX = 0.0f;
			float newRotZ = 0.0f;
			NiVector3 ricochetBeginPos;
		} _ret;

		struct normalEval{
			int sgnRX;
			int sgnRZDva;

			normalEval(int _sX, int _sZ) : sgnRX(_sX), sgnRZDva(_sZ) {}
		};


		static std::array<normalEval,4> _normalMode = { normalEval{1,1}, normalEval{-1,1},normalEval{1,-1},normalEval{-1,-1} };
		std::array<NiVector3*, 4> _l_raycastPos = {nullptr,nullptr,nullptr,nullptr};

		__WriteLog("PJ %x RX: %.5f, RZ: %.5f", _proj, rot_X* RdToAg, rot_Z* RdToAg);
		float newRotX = rot_X + deviate_X;
		float newRotZ_d1 = rot_Z + deviate_Z;
		float newRotZ_d2 = rot_Z - deviate_Z;
		__WriteLog("before newRX: %.5f, newRotZ_d1: %.5f, newRotZ_d2: %.5f",
			newRotX* RdToAg, newRotZ_d1* RdToAg, newRotZ_d2* RdToAg);
		if (std::fabs(newRotX) > Half_PI) AngleCompensation_Rico_X(newRotX);
		Radiu_Z_User_to_Game(newRotZ_d1);
		Radiu_Z_User_to_Game(newRotZ_d2);
		//if (std::fabs(newRotZ_d1) > PI) AngleCompensation_Z(newRotZ_d1);
		//if (std::fabs(newRotZ_d2) > PI) AngleCompensation_Z(newRotZ_d2);
		__WriteLog("after newRX: %.5f, newRotZ_d1: %.5f, newRotZ_d2: %.5f",
			newRotX * RdToAg, newRotZ_d1 * RdToAg, newRotZ_d2 * RdToAg);

		//NiVector3 _raycastPos_0 = GetRayCastPos(_beginPos, newRotX, newRotZ_d1);
		auto rayCastRes_0 = GetRayCastPos(_beginPos, newRotX, newRotZ_d1);
		_l_raycastPos[0] = &rayCastRes_0.raycastPos;
		float _dis_0 = rayCastRes_0.raycastSuccess ? CalcPosSquareDis(_proj->position, rayCastRes_0.raycastPos) : 0.0f;
		__WriteLog(" RX,RZd1 (1,1) : %.5f", _dis_0);

		//NiVector3 _raycastPos_1 = GetRayCastPos(_beginPos, -newRotX, newRotZ_d1);
		auto rayCastRes_1 = GetRayCastPos(_beginPos, -newRotX, newRotZ_d1);
		_l_raycastPos[1] = &rayCastRes_1.raycastPos;
		float _dis_1 = rayCastRes_1.raycastSuccess ? CalcPosSquareDis(_proj->position, rayCastRes_1.raycastPos) : 0.0f;
		__WriteLog("-RX,RZd1 (-1,1) : %.5f", _dis_1);

		//NiVector3 _raycastPos_2 = GetRayCastPos(_beginPos, newRotX, newRotZ_d2);
		auto rayCastRes_2 = GetRayCastPos(_beginPos, newRotX, newRotZ_d2);
		_l_raycastPos[2] = &rayCastRes_2.raycastPos;
		float _dis_2 = rayCastRes_2.raycastSuccess ? CalcPosSquareDis(_proj->position, rayCastRes_2.raycastPos) : 0.0f;
		__WriteLog(" RX,RZd2 (1,-1) : %.5f", _dis_2);
		
		//NiVector3 _raycastPos_3 = GetRayCastPos(_beginPos, -newRotX, newRotZ_d2);
		auto rayCastRes_3 = GetRayCastPos(_beginPos, -newRotX, newRotZ_d2);
		_l_raycastPos[3] = &rayCastRes_3.raycastPos;
		float _dis_3 = rayCastRes_2.raycastSuccess ? CalcPosSquareDis(_proj->position, rayCastRes_3.raycastPos) : 0.0f;
		__WriteLog("-RX,RZd2 (-1,-1) : %.5f", _dis_3);

		__m128 distance = _mm_set_ps(_dis_3, _dis_2, _dis_1, _dis_0);
		__m128 ricoMinDis = _mm_set1_ps(1024.0f);
		__m128 compRes = _mm_cmplt_ps(ricoMinDis, distance);
		int mask_bits = _mm_movemask_ps(compRes);
		if (mask_bits > 0)
		{
			std::vector<int> indexs;
			for (int i = 0; i < 4; ++i) {
				if (mask_bits & (1 << i)) {
					indexs.push_back(i);
				}
			}
			int idx = indexs.at(RandMng.GetRandInt(0, indexs.size() - 1));
			const NiVector3* _raycastPos = _l_raycastPos.at(idx);
			float zDif = _proj->position.z - _raycastPos->z;
			if (std::fabs(zDif) < 2.0f)
			{
				return _ret;
			}

			const normalEval& _nE = _normalMode.at(idx);
			_ret.hasTrajectory = true;
			_ret.newRotZ = rot_Z + deviate_Z * _nE.sgnRZDva;

// Sgn of rotation X
			float _sgnX = 1.0f;
			_sgnX -= (2.0f * ((zDif) < 0));
			_ret.newRotX = newRotX * _sgnX;
// Sgn of rotation X

// Get Direction Vector
			
			NiVector3 _direction = { *_raycastPos };
			_direction -= _beginPos.PS();
			_direction.Normalize();
// Get Direction Vector

// Set Ricochet Begin Position
			_ret.ricochetBeginPos = _beginPos;
			_ret.ricochetBeginPos.MoveAlone(_direction,32);
			__WriteLog("ricochets' position is %.5f, %.5f, %.5f",
				_ret.ricochetBeginPos.x, _ret.ricochetBeginPos.y, _ret.ricochetBeginPos.z);
// Set Ricochet Begin Position

			__WriteLog("bits %d, newRX %.5f, newRZ %.5f, picked nE is %d, %d",mask_bits, _ret.newRotX, _ret.newRotZ, _nE.sgnRX, _nE.sgnRZDva);
			return _ret;
		}
		else {	// == 0, special deal£¬ backward
			__WriteLog("special deal");
			if (RandMng.GetRand(0, Half_PI) < std::abs(rot_X) ){

				float invRotZ = (rot_Z + PI) + (deviate_Z * ((RandMng.GetRandInt(0, 100) < 50) ? 1 : -1));
				//if (std::fabs(invRotZ) > PI) AngleCompensation_Z(invRotZ);
				Radiu_Z_User_to_Game(invRotZ);
				auto _raycastReS_special = GetRayCastPos(_beginPos, newRotX, invRotZ);
				if (_raycastReS_special.raycastSuccess)
				{
					NiVector3 _raycastPos_special{ _raycastReS_special.raycastPos };
					float _dis_special = CalcPosSquareDis(_proj->position, _raycastPos_special);
					if (_dis_special > 1024.0f) {
						_ret.hasTrajectory = true;
						_ret.newRotX = newRotX;
						_ret.newRotZ = invRotZ;
						// Get Direction Vector
						NiVector3 _direction = { _raycastPos_special };
						_direction -= _beginPos.PS();
						_direction.Normalize();
						// Get Direction Vector

						// Set Ricochet Begin Position
						_ret.ricochetBeginPos = _beginPos;
						_ret.ricochetBeginPos.MoveAlone(_direction, 32);
						__WriteLog("special deal, raycast pos is %.2f, %.2f, %.2f, newRX %.5f, newRZ %.5f",
							_raycastPos_special.x, _raycastPos_special.y, _raycastPos_special.z, _ret.newRotX, _ret.newRotZ);
						return _ret;
					}
				
				}
				else {
				//float invRotX = -rot_X + (deviate_Z * ((RandMng.GetRandInt(0, 100) < 50) ? 1 : -1));
				}
			}
		}
		//__WriteLog("no ricochet");
		return _ret;
	}


	// new in 1.4

	/*static __forceinline void Projectile_ClearImpact(MissileProjectile* _proj) {
		if (_proj->hasImpacted && _proj->eImpactResult == MissileProjectile::ImpactResult::IR_DESTROY) {
			_proj->impactDataList.RemoveAll();
			_proj->byte118 = 0;
			_proj->hasImpacted = 0;
			_proj->eImpactResult = MissileProjectile::ImpactResult::IR_NONE;
			_proj->distTravelled = 0;
			ResetRefCollision(_proj);
			ResetProjectileCharCtrlFlag(_proj);
		}
	}*/

	static __forceinline void Projectile_SetAttribute(MissileProjectile* _proj,
		NiVector3& new_proj_pos,
		float nRotX, float nRotZ,
		float nDamage = -1, float nSpdM = -1) 
	{
		if (nDamage > 0)_proj->hitDamage = nDamage;
		if (nSpdM > 0)_proj->speedMult = nSpdM;
		_proj->position = new_proj_pos;
		SetStartingPos(_proj, new_proj_pos);
		_proj->rotation.x = nRotX;
		_proj->rotation.z = nRotZ;
	}

// material that must penetrate like grass, cloth, glass, water. They will be penetrated no matter what bullet is
#define IsMustPenetrateMaterial(type) (type == kMaterial_Grass || type == kMaterial_Cloth || type == kMaterial_Glass || type == kMaterial_Water )
// material that may not be penetrated, will depend on bullet
#define NotMustPenetrateMaterial(type) (type != kMaterial_Grass && type != kMaterial_Cloth && type != kMaterial_Glass && type != kMaterial_Water )

	// record penetrate information
	struct Pene_Info {
		Projectile* PenePJ = nullptr;
		TESObjectREFR* impact_ref = nullptr;
		NiVector3 LastHitPos{ 0,0,0 };
		INT32 Pene_Times = 0; 
		UINT32 lastHitMaterial = 0;
		float ExpectDepth = 0;
		float APScore = 0;
		Pene_Info() {};
		Pene_Info(Projectile* _pene_pj, TESObjectREFR* _impact_ref, const NiVector3& _last_hit,UINT32 _hit_material,float expect_depth,float AP_Score, INT32 pene_times) : 
						PenePJ(_pene_pj), impact_ref(_impact_ref), LastHitPos(_last_hit),lastHitMaterial(_hit_material), ExpectDepth(expect_depth), APScore(AP_Score), Pene_Times(pene_times){}
		
		void UpdateInfo(TESObjectREFR* _impact_ref, const NiVector3& _last_hit, UINT32 _hit_material, float expect_depth, float AP_Score, INT32 pene_times = -1) { 
			impact_ref = _impact_ref;
			LastHitPos = _last_hit;
			lastHitMaterial = _hit_material;
			ExpectDepth = expect_depth;
			APScore = AP_Score;
			Pene_Times = pene_times < 0 ? (Pene_Times + 1) : pene_times; 
		}
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

		void UpdateInfo(TESObjectREFR* _impact_ref, const NiVector3& _hit_pos, UINT8 _rico_times) {
			impact_ref = _impact_ref;
			LastHitPos = _hit_pos;
			Rico_Times = _rico_times;
		}
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
		ExtraDecal = 0x0080,
		HelmetRicochet = 0x0100,
		ArmorRicochet = 0x0200,
		DepthConsider = 0x0400,		// consider depth of an non-actor object
	};


	static __forceinline bool IsFlagOn(UINT16 _flag) {
		return (bullet_manager_flag & _flag) != 0;
	}

	static __forceinline void SetFlagOn(UINT16 _flag) {
		bullet_manager_flag |= _flag;
	}

	struct NewProj_Info {
		UINT8 last_hit_material = 0;
		NewProj_Info() {}
		NewProj_Info( UINT8 _last_hit_material) : last_hit_material(_last_hit_material) {}
	};

	// record projectiles which spawned due to penetration or ricochet
	// use to impact optimization
	struct NewSpawnedProjMap {
		std::unordered_map<Projectile*, NewProj_Info> newSpawnedProjs{};

		__forceinline void GoInNPMap(Projectile* _dp_proj,UINT8 _lastHitMaterial) {
			if (const auto& dp_iter = newSpawnedProjs.find(_dp_proj); dp_iter != newSpawnedProjs.end()) {
				auto& np_info = dp_iter->second;
			}
			else newSpawnedProjs.try_emplace(_dp_proj, NewProj_Info{ _lastHitMaterial });
		}

		__forceinline auto IsInNPMap(Projectile* _dp_proj) {
			struct ret_nProj_info {
				bool find_nProj;
				NewProj_Info* np_info;

				bool __forceinline Valid() const { return find_nProj && np_info; }
			};
			if (const auto& np_iter = newSpawnedProjs.find(_dp_proj); np_iter != newSpawnedProjs.end()) {
				NewProj_Info* np_info_ptr = &np_iter->second;
				return ret_nProj_info{ true,np_info_ptr };
			}
			return ret_nProj_info{ false,nullptr };
		}

		__forceinline void EraseFromMap(Projectile* _np_proj) {
			if (const auto& np_iter = newSpawnedProjs.find(_np_proj); np_iter != newSpawnedProjs.end())
				newSpawnedProjs.erase(np_iter);
		}

	};

	class BulletManager
	{
		using new_projectile_map = NewSpawnedProjMap;
		using penetrate_map = std::unordered_map<formid, Pene_Info>;
		using ricochet_map = std::unordered_map<formid, Rico_Info>;
	public:
		BulletManager(BulletManager&&) = delete;
		// all actor, who producing new projectiles will be recorded.
		std::unordered_map<Actor*,float> is_producing_proj{};
		new_projectile_map new_proj_map{};
		penetrate_map PENEMap{};
		ricochet_map RICOMap{};
		std::array<float, 12> MaterialPenalty{ 0 };		// For Pene
		std::array<float, 12> MaterialChance{ 0 };		// For Rico, Ricochet Chance Only Apply To Ricochet
		
	
		__forceinline void EraseFromProducingProj(Actor* _actor) {
			is_producing_proj.erase(_actor);
		}
		__forceinline bool IsActorProducingProj(Actor* _actor) {
			return (is_producing_proj.find(_actor) != is_producing_proj.end());
		}
		__forceinline auto UpdateProducingMapTimer(Actor* _actor, float timePassed) {
			struct {
				bool updateResult = false;
				float newTimer = 0.0f;
			} ret;
			
			if (const auto& itor = is_producing_proj.find(_actor); itor != is_producing_proj.end())
			{
				if (float _tMult = GetGlobalTimeMult(); _tMult > 0.0F)
				{
					timePassed = timePassed / GetGlobalTimeMult();
				}
				itor->second = itor->second + timePassed;
				ret.updateResult = true;
				ret.newTimer = itor->second;
			}
			return ret;
		}
		__forceinline float GetProducingMapTimer(Actor* _actor) {
			if (const auto& itor = is_producing_proj.find(_actor); itor != is_producing_proj.end())
			{
				return itor->second;
			}
			return FLT_MAX;
		}
		__forceinline void MarkAsProducingProj(Actor* _actor) {
			if (const auto& itor = is_producing_proj.find(_actor); itor != is_producing_proj.end())
			{
				itor->second = 0.0F;
			}
			else
				is_producing_proj.emplace(_actor,0.0f);
		}
		__forceinline new_projectile_map& GetNPProjMap() {
			return new_proj_map;
		}

		// mark new projectile which spawned due to penetration
		__forceinline void MarkAsNewSpawnedProj(Projectile* _dp_proj,UINT8 _lastHitPos) {
			GetNPProjMap().GoInNPMap(_dp_proj, _lastHitPos);
		}

		// check is the projectile which spawned due to last penetration or ricochet
		__forceinline auto IsNPProj(Projectile* _dp_proj) {
			return GetNPProjMap().IsInNPMap(_dp_proj);
		}

		__forceinline auto IsRicoProj(Projectile* _Proj){
			struct rico_ret {
				bool find_rico;
				Rico_Info* rico_info;

				bool __forceinline Valid() const { return find_rico && rico_info; }
			};
			//__WriteLog("CheckNotInMap %x", _Proj->refID);
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
			//__WriteLog("CheckNotInMap %x", _Proj->refID);
			if (const auto& PENEIter = PENEMap.find(_Proj->refID); PENEIter != PENEMap.end()) {
				Pene_Info* pene_info_ptr = &PENEIter->second;
				if (!pene_info_ptr || !pene_info_ptr->PenePJ) return pene_ret{ false,nullptr };
				return pene_ret{ true, pene_info_ptr };
			}
			return pene_ret{ false,nullptr };
		}

		__forceinline bool IsInPeneMap(Projectile* _Proj) {
			if (const auto& PENEIter = PENEMap.find(_Proj->refID); PENEIter != PENEMap.end()) {
				Pene_Info* pene_info_ptr = &PENEIter->second;
				if (!pene_info_ptr || !pene_info_ptr->PenePJ) return false;
				return true;
			}
			return true;
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

		__forceinline bool NotInRicoMap(Projectile* _Proj) const {
			if (const auto& RicoIter = RICOMap.find(_Proj->refID); RicoIter != RICOMap.end()) {
				if (!RicoIter->second.RicoPJ) return true;
				return false;
			}
			return true;
		}

		__forceinline bool NotInPeneMap(Projectile* _Proj) const {
			if (const auto& PENEIter = PENEMap.find(_Proj->refID); PENEIter != PENEMap.end()) {
				if (!PENEIter->second.PenePJ) return true;
				return false;
			}
			return true;
		}

		__forceinline void InsertPeneMap(Projectile* _Proj, TESObjectREFR* _imp_ref,const NiVector3& last_hit_pos,UINT32 material,float expect_depth, float ap_score, UINT8 pene_times){
			if (const auto& PENEIter = PENEMap.find(_Proj->refID); PENEIter != PENEMap.end()) {
				if (auto& pene_info = PENEIter->second; !pene_info.PenePJ) {
					pene_info = Pene_Info{ _Proj,_imp_ref,last_hit_pos,material,expect_depth,ap_score,pene_times};
				}
			}
			else PENEMap.try_emplace(_Proj->refID, Pene_Info{ _Proj,_imp_ref,last_hit_pos,material,expect_depth,ap_score,pene_times});
		}


		__forceinline void InsertRicoMap(Projectile* _Proj,TESObjectREFR* _imp_ref, const NiVector3& begin_pos, UINT8 pene_times) {
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

		__forceinline void EraseFromNewProjMap(Projectile* _do_pene_proj) {
			__WriteLog("EraseFromDPMap");
			GetNPProjMap().EraseFromMap(_do_pene_proj);
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

	static TESSound* pc_helmet_rico = nullptr;
	static TESSound* armor_rico = nullptr;

	// Only work for player
	static __forceinline bool RandomlyPlayHelmetRicoSound(TESObjectREFR* _rico_pj) {
		TESSound* rico_sound = GetNewTESSound(OUT pc_helmet_rico,R"(fx\ArmorRicochetSound\HelmetRicochet1p)", 0x21f);
		if (!rico_sound) return false;
		_Sound::PlayTESSound(rico_sound, 0x4102u, _rico_pj);
	}

	// For armor ricochet
	static __forceinline bool RandomlyPlayArmorRicoSound(TESObjectREFR* _rico_pj) {
		TESSound* rico_sound = GetNewTESSound(OUT armor_rico, R"(fx\ArmorRicochetSound\ArmorRicochet3p)", 0x21f);
		if (!rico_sound) return false;
		_Sound::PlayTESSound(rico_sound, 0x4102u, _rico_pj);
	}

	static __forceinline bool RandomlyPlayRicoSound(TESObjectREFR* _rico_pj) {
		TESSound* rico_sound = GetClonedTESSoundForRico(0x21f);
		if (!rico_sound) return false;
		_Sound::PlayTESSound(rico_sound, 0x4102u ,_rico_pj);
	}

	static __forceinline bool RandomlyPlayRicoSound(TESObjectREFR* _rico_pj, const NiVector3& _pos) {
		TESSound* rico_sound = GetClonedTESSoundForRico(0x21f);
		if (!rico_sound) return false;
		_Sound::PlayTESSound(rico_sound, 0x4102u, _rico_pj, _pos);
		return true;
	}

	static __forceinline bool PJIsExplosion(Projectile* _pj) {
		if (_pj) {
			if (const BGSProjectile* basepj = (BGSProjectile*)_pj->baseForm; (basepj->projFlags & 2) != 0 && (basepj->type & 1) != 0) {// Check type and explosion flag
				return true;
			}
		}
		return false;
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

	/*
	===== NewPosBasedDepth_Pene =====
	Check whether two coordinates are on the same direction
	Note that this function only check 2 dimension ( x and y )
	*/
	static __forceinline bool EvalPosDirection(const __m128& _begin, const __m128& _end, const __m128& _direction) {
		__m128 _directionVec = _mm_sub_ps(_end, _begin);
		__m128 _mulRes = _mm_mul_ps(_directionVec, _direction);
		int bitmask = _mm_movemask_ps(_mulRes);
		return ( bitmask & (0b11) ) == 0;
	}

	/*
	===== EvalPenetratedOppositePos =====
	Use raycast eval penetrated depth of object.
	Use it when only the projectile hit a not-actor object
	The return is position of object's opposite surface
	This function works, due to fxxking tes engine
	*/
	//static __forceinline auto EvalPenetratedOppositePos(Projectile* PJRef, TESObjectREFR* imp_ref, float beginPosOffsetDepth,float endPosOffsetDepth) {
	//	struct {
	//		NiVector3 forwardPos;
	//		NiVector3 backwardPos;
	//	} ret;
	//	
	//	NiVector3 Raycast_OriPos { *PJRef->GetPos() };

	//	__WriteLog("[EvalPenetratedOppositePos] before movealone %.2f, %.2f, %.2f", Raycast_OriPos.x, Raycast_OriPos.y, Raycast_OriPos.z);
	//	__WriteLog("[EvalPenetratedOppositePos] proj speed vec %.2f, %.2f, %.2f", PJRef->UnitVector.x, PJRef->UnitVector.y, PJRef->UnitVector.z);
	//	Raycast_OriPos.MoveAlone(PJRef->UnitVector, 2.0);
	//	__WriteLog("[EvalPenetratedOppositePos] after movealone %.2f, %.2f, %.2f", Raycast_OriPos.x, Raycast_OriPos.y, Raycast_OriPos.z);

	//	NiVector3 depth_probe = GetRayCastPos(Raycast_OriPos,PJRef->rotation.x, PJRef->rotation.z,49000.0F);
	//	depth_probe.MoveAlone(PJRef->UnitVector, 2.0);
	//	float object_thickness = Point3Distance(Raycast_OriPos, depth_probe);
	//	if (imp_ref)
	//		imp_ref->ToggleCollision(false);

	//	NiVector3 backward_begin = GetRayCastPos(Raycast_OriPos, PJRef->rotation.x, PJRef->rotation.z, object_thickness);
	//	/*NiVector3 forwardPos_2 = GetRayCastPos(forwardPos_1,PJRef->rotation.x, PJRef->rotation.z,1.0F);
	//	ret.forwardPos = forwardPos_1;

	//	forwardPos_2.MoveAlone(PJRef->UnitVector, -endPosOffsetDepth);*/
	//	if (imp_ref)
	//		imp_ref->ToggleCollision(true);

	//	
	//	NiVector3 backwardPos = GetRayCastPos(backward_begin, PJRef->rotation.x, PJRef->rotation.z + PI, 50000.0F);
	//	ret.backwardPos = backwardPos;
	//	
	//	__WriteLog("[EvalPenetratedOppositePos] forward_1 pos is %.2f, %.2f, %.2f", depth_probe.x, depth_probe.y, depth_probe.z);
	//	//__WriteLog("[EvalPenetratedOppositePos] forward_2 pos is %.2f, %.2f, %.2f", forwardPos_2.x, forwardPos_2.y, forwardPos_2.z);
	//	__WriteLog("[EvalPenetratedOppositePos] backward pos is %.2f, %.2f, %.2f", backwardPos.x, backwardPos.y, backwardPos.z);
	//	
	//	return ret;
	//}

	/*
	===== EvalPenetratedOppositePos =====
	Use raycast eval penetrated depth of object.
	Use it when only the projectile hit a not-actor object
	The return is position of object's opposite surface
	This function works, due to fxxking tes engine
	*/
	static __forceinline auto EvalPenetratedOppositePosAlt(Projectile* PJRef, float beginPosOffsetDepth, float endPosOffsetDepth) {
		struct {
			bool success = false;
			NiVector3 forwardPos;
			NiVector3 backwardPos;
		} ret;

		NiVector3 Raycast_OriPos{ *PJRef->GetPos() };

		__WriteLog("[EvalPenetratedOppositePos] before movealone %.2f, %.2f, %.2f", Raycast_OriPos.x, Raycast_OriPos.y, Raycast_OriPos.z);
		__WriteLog("[EvalPenetratedOppositePos] proj speed vec %.2f, %.2f, %.2f", PJRef->UnitVector.x, PJRef->UnitVector.y, PJRef->UnitVector.z);
		Raycast_OriPos.MoveAlone(PJRef->UnitVector, 4.0);
		__WriteLog("[EvalPenetratedOppositePos] after movealone %.2f, %.2f, %.2f", Raycast_OriPos.x, Raycast_OriPos.y, Raycast_OriPos.z);

		auto depthProbeRes = GetRayCastPos(Raycast_OriPos, PJRef->rotation.x, PJRef->rotation.z, 49000.0F);
		if (depthProbeRes.raycastSuccess)
		{
			NiVector3 depth_probe{ depthProbeRes.raycastPos };
			ret.forwardPos = depth_probe;
			depth_probe.MoveAlone(PJRef->UnitVector, 4.0);

			auto backwardRes = GetRayCastPos(depth_probe, PJRef->rotation.x, PJRef->rotation.z + PI, 5.0F);
			if (backwardRes.raycastSuccess)
			{
				NiVector3 backwardPos{ backwardRes.raycastPos};
				ret.backwardPos = backwardPos;
				ret.success = true;
				__WriteLog("[EvalPenetratedOppositePos] depth_probe pos is %.2f, %.2f, %.2f", depth_probe.x, depth_probe.y, depth_probe.z);
				//__WriteLog("[EvalPenetratedOppositePos] forward_2 pos is %.2f, %.2f, %.2f", forwardPos_2.x, forwardPos_2.y, forwardPos_2.z);
				__WriteLog("[EvalPenetratedOppositePos] backward pos is %.2f, %.2f, %.2f", backwardPos.x, backwardPos.y, backwardPos.z);
			}
		}
		else {
			__WriteLog("[EvalPenetratedOppositePos] depth_probe raycast failed");
		}

		return ret;
	}

	[[nodiscard]] static __forceinline auto EvalPenetratedDepth(Projectile* PJRef) {
		struct {
			bool success = false;
			// the opposite position of hitting position
			NiVector3 oppositePosition;
			// the distance between hitting position and opposite position
			float distance;
			// check whether the hitting postion and opposite position is alone with projectile velocity direction
			bool isAloneVelocityDirection;
		} evalRes;
		
		
		auto raycastInfo = EvalPenetratedOppositePosAlt(PJRef,2.0, 2.0);

		if (raycastInfo.success)
		{
			evalRes.success = true;
			evalRes.oppositePosition = raycastInfo.backwardPos;

			evalRes.isAloneVelocityDirection =
				EvalPosDirection(PJRef->GetPos()->PS(), evalRes.oppositePosition.PS(), PJRef->UnitVector.PS());

			evalRes.distance =
				Point3Distance(*PJRef->GetPos(), raycastInfo.backwardPos);
			__WriteLog("[EvalPenetratedDepth] distance is %.5f", evalRes.distance);
			__WriteLog("[EvalPenetratedOppositePos] isAloneTheDirection %d", evalRes.isAloneVelocityDirection);

			return evalRes;
		}
		
// when the backward pos is almost the same as projectile pos
		/*float dist = CalcPosSquareDisSIMD(*PJRef->GetPos(), evalRes.oppositePosition);
		evalRes.distance = std::sqrtf(dist);*/
		/*if (dist < 1.0f){
			evalRes.distance =
				Point3Distance(raycastInfo.forwardPos, raycastInfo.backwardPos);
		}
		else {
			evalRes.distance = std::sqrtf(dist);
		}*/

		
		return evalRes;
	}

	static __forceinline void SetPosBasedDepth_Backward(Projectile* PJRef, float Depth, NiVector3& new_pos) {
		NiVector3 ReverseDirection = ScaleVector(PJRef->UnitVector, NiVector3{-1,-1,-1});
		const NiVector3* OriginPos = PJRef->GetPos();
		new_pos.x = (OriginPos->x + (ReverseDirection.x * Depth));
		new_pos.y = (OriginPos->y + (ReverseDirection.y * Depth));
		new_pos.z = (OriginPos->z + (ReverseDirection.z * Depth));
	}


	

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
			outRX += (RandMng.GetRand(-RotMaxX,RotMaxX)/RdToAg);
			outRZ += (RandMng.GetRand(-RotMazZ,RotMazZ)/ RdToAg);
			if (std::fabs(outRX) > Half_PI) AngleCompensation_Pene_X(outRX);
			Radiu_Z_User_to_Game(outRZ);
		}
	}



	/*
	===== SetRicoAttribute =====
	Set attributes for the new penetrate ricochet
	Call after RX,RZ Offset have generated
	_RX_Offset,_RZ_Offset -> it is deviation of ricochet projectile, they are radius,
		can use to calc new damage, higher deviation means lower damage of ricochet projectile.
	Set damage and speedmult for ricochet projectile according the rotation offset.
	*/
	static __forceinline void SetRicoAttribute(const float& _RX_Offset,const float& _RZ_Offset,
												float& _nDmg, float& _nSpedMult) {
		
		float de_dmg_ratio = std::fabs(_RX_Offset) / Half_PI + std::fabs(_RZ_Offset) / PI;
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
	The "new begin postion" of ricochet will be set in this function
	*/
	[[nodiscard]] static auto NewRicoPJStats(const Projectile* ThisProj,const BGSProjectile* base_pj, NiVector3& beginPos, float Backward_Chance = 0) {
		
		/*
		NewRicoPJInfo
		This struct contains information for spawned ricochet.
		*/
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
		float outRX = ThisProj->rotation.x, outRZ = ThisProj->rotation.z;

		
		float DeviateAngle_X = RandMng.GetRand(0, ricochet_max_rotX);

		float DeviateAngle_Z = RandMng.GetRand(0, ricochet_max_rotZ);

		/*DeviateAngle_X /= 3.1415926F;
		DeviateAngle_Z /= 3.1415926F;*/

		DeviateAngle_X /= RdToAg;
		DeviateAngle_Z /= RdToAg;
		// Eval 
		__WriteLog("[NewRicoPJStats] The Hit Proj Rotation: x = %.2f,y = %.2f", ThisProj->rotation.x, ThisProj->rotation.z);

// let the deviation connected with rotation X
		float _pjRX = std::abs(outRX);
		float _ratioX = _pjRX / 1.5707963F;
		
		float _DeviateX_Factor = RoughSlowDown(RoughClamp<float>(_ratioX,0.0f,1.0f));

		//DeviateAngle_X *= _ratioX;
		//DeviateAngle_Z *= (1 - _ratioX);
		DeviateAngle_X *= _DeviateX_Factor;
		DeviateAngle_Z *= (1 - _DeviateX_Factor);
// let the deviation connected with rotation X

// Eval 
		auto trajectoryEvalRes = EvalRicoTrajectory(ThisProj, beginPos, outRX, outRZ, DeviateAngle_X , DeviateAngle_Z, Backward_Chance);
		if (!trajectoryEvalRes.hasTrajectory)
		{
			__WriteLog("Eval Trajectory Failed");
			return NewRicoPJInfo{ false,0,0,0,0 };
		}

// set ricochet begin position
		beginPos = trajectoryEvalRes.ricochetBeginPos;


		SetRicoAttribute(DeviateAngle_X, DeviateAngle_Z, new_pj_dmg, new_spdM);	// set projectile newdmg and newspedM
		if (base_pj->speed * new_spdM < ricochet_speed_threshold || 
			new_pj_dmg < ricochet_damage_threshold ) return NewRicoPJInfo{ false,0,0,0,0 };

		return NewRicoPJInfo{ true,new_pj_dmg,new_spdM,trajectoryEvalRes.newRotX,trajectoryEvalRes.newRotZ}; 
	}
	/*
	===== SetPeneAttribute =====
	Set attributes for the new penetrate projectile
	_nPene_depth - new penetrate depth, init as penetrate_base_depth
	_nAPScore - new AP Score, init as ( 0 - depth penalty + damage ap buff)
	_nDmg - new penetrate projectile damage
	_dDmg - the d-value
	_armor_dtdr - armor dt & dr
	_helmet_dtdr - helmet dt & dr
	_oldAPScore - old AP Score

	*/
	static __forceinline void SetPeneAttribute(TESObjectREFR* imp_ref,SInt32 hit_location,
												float& _nPene_depth,float& _nAPScore,float& _nDmg,float& _dDmg,
												const ImpactRefInfo& imp_ref_info,
												const float& _oldAPScore) {
		_nPene_depth += _oldAPScore;	// Old AP_Score decide depth. In fact it do nothing until v1.4
		_nAPScore += _oldAPScore;		// let New AP_Score = Old AP_Score
		
		// Will check actor dt when hit an actor
		if (imp_ref_info._impactref_is_actor) {
			Actor* actor_ref = (Actor*)imp_ref;
			float Actor_DT = 0;
			const ActorDTDR& actor_dtdr = imp_ref_info._dtdr;
			if ( !imp_ref_info._impactref_is_creature ) {			// character
				if (std::holds_alternative<CharacterDTDR>(actor_dtdr)) {
					const CharacterDTDR& character_dtdr = std::get<CharacterDTDR>(actor_dtdr);
					if (IsFlagOn(LocalizedDTSupport)) {
						if (hit_location == 1) Actor_DT = character_dtdr.GetHelmetDT();				// helmet dt
						else if (hit_location == 14 && actor_ref->IsWeaponOut()) {}					// hit weapon,Do nothing
						else Actor_DT = character_dtdr.GetArmorDT();								// armor dt
					}
					else Actor_DT = character_dtdr.GetTotalDT();
				}
			}
			else {
				if (std::holds_alternative<CreatureDTDR>(actor_dtdr)) {
					const CreatureDTDR& creature_dtdr = std::get<CreatureDTDR>(actor_dtdr);
					Actor_DT = creature_dtdr.GetNativeDT();											// Creature DT
				}
			}
			
			
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
		//if (nPene_depth < penetrate_base_depth) nPene_depth = penetrate_base_depth;
		//else if (nPene_depth > penetrate_max_depth) nPene_depth = penetrate_max_depth;

		if (!imp_ref) return;
		else if (IS_ACTOR(imp_ref)) nPene_depth = penetrate_human_base_depth;
		//else if (IS_TYPE(imp_ref, TESFurniture)) nPene_depth = penetrate_furniture_depth;
	}

	/*
	====== Calculate New Proj Stats According "This Proj" Stats ======
	"This Proj" can be a "first hit proj" or a "penetrated projectile"
	Return a struct which has information about "penetrate or not" , "new damage" and "new speedmult" of new spawn projectile
	Damage will scale by the global setting (dmg_decrease_percent) at first,And then scale by the MaterialPenalty
	Projectile speed will be considered as a factor in "penetrate or not" when SpeedCalcForPene flag on

	Needs to create pene_info first.This function will fill the pene_info

	V1.4 - Move applying "extra_ap_ability" buff to "first hit", mean that it will not contribute to the calculation angin when multiply penetrated.
	*/
	[[nodiscard]] static auto NewPenePJStats(const Projectile* ThisProj,TESObjectREFR* imp_ref,const Actor* PJSrc,const TESObjectWEAP* bs_weap,
							const BGSProjectile* bs_proj, const ImpactRefInfo& imp_ref_info, float penetrated_depth,
							UINT32 pene_times = 0,float OldAPScore = 0) {
		struct NewPenePJInfo {
			bool  Pene;					// decide do penetrate or not
			float newDmg;				// new damage of penetrate bullet
			float newSpeedMult;			// new speed mult of penetrate bullet
			float penetrate_depth;		// new penetrate_depth.unuse for now
			float newAPScore;			// new AP score of penetrate bullet

		};
		float nDmg = ThisProj->hitDamage;
		UINT8 impact_material = imp_ref_info._hit_material;
		SInt32 hit_location = imp_ref_info._hit_location;
		float material_penalty = (impact_material < 12 ? BulletManager::bullet_manager_instance().MaterialPenalty[impact_material] : 1.0f);

		float nAPScore = 0;
		bool _isHitActor = imp_ref_info.isHitActor();
		// actor will not calculate as material here, they will consider DT/DR
		if (!_isHitActor) {
			nDmg *= material_penalty;
			float depthPenalty = std::powf(penetrated_depth, 1.0f + penetrate_depth_factor * (1.0f - material_penalty));
			nAPScore -= depthPenalty;
			__WriteLog2(false,"[NewPenePJStats] Calc Depth penalty, material penalty is %.2f, depth is %.2f,depth penalty is %.2f", 
								material_penalty, penetrated_depth, depthPenalty);
		}
			
		
		float nPene_depth = penetrate_base_depth;
		float dDmg = ThisProj->hitDamage - nDmg;
		//__WriteLog("impact_material %u,pene_times %u, OldAPScore %.2f, PjHitdmg %.2f", impact_material, pene_times, OldAPScore, ThisProj->hitDamage);
		
		if (pene_times == 0){	// When Pene_Times is 0 means it is the first hit ,it needs to calc the ap_score
			BulletInfo bullet_info = CheckBulletTypeByAmmoEffect(GetCurEqAmmo(bs_weap, PJSrc), ThisProj);
			__WriteLog("[NewPenePJStats] before SetPeneAttribute, APScore is %.2f, dDmg is %.2f",bullet_info.AP_Score, dDmg);
			if (nDmg > penetrate_ap_dmg_threshold) {
				float extra_ap_ability = nDmg - penetrate_ap_dmg_threshold;
				__WriteLog("[NewPenePJStats] NewDmg is %.2f, apDMG_Threshold is %.2f, extra_ap_ability is %.2f", nDmg, penetrate_ap_dmg_threshold, extra_ap_ability);
				nAPScore += extra_ap_ability;
				nPene_depth += extra_ap_ability;
			};
			//__WriteLog2(true, "[NewPenePJStats] NewAPScore Before Calc Speed : %.2f", nAPScore);
			
			SetPeneAttribute(imp_ref, hit_location, OUT nPene_depth, OUT nAPScore, OUT nDmg, dDmg, imp_ref_info,bullet_info.AP_Score);
			__WriteLog2(false, "[NewPenePJStats] NewAPScore Before Calc DT : %.2f", nAPScore);
			if (IsFlagOn(MngFlag::SpeedCalcForPene))
			{
				bool _allowSpeedCalc = true;
				if (_isHitActor && nAPScore < 0.0f) {
					_allowSpeedCalc = false;
				}
				if (_allowSpeedCalc)
				{
					float ap_buff = 0.0f;
					float proj_speed = bs_proj->speed * ThisProj->speedMult;
					float speed_dif = proj_speed - penetrate_speed_threshold;

					ap_buff = speed_dif / (speed_dif < 0 ? speed_decrease_step : speed_increase_step);
					nAPScore += ap_buff;
					__WriteLog("[NewPenePJStats] Before SetPeneAttribute nAPScore is %.2f, APBuff form speed is %.2f", nAPScore, ap_buff);
				}
			}
		}
		else {
			__WriteLog("[NewPenePJStats] before SetPeneAttribute, Old APScore is %.2f, dDmg is %.2f", OldAPScore, dDmg);
			SetPeneAttribute(imp_ref, hit_location, OUT nPene_depth, OUT nAPScore, OUT nDmg, dDmg, imp_ref_info,OldAPScore);
		}
		__WriteLog("[NewPenePJStats] after SetPeneAttribute, new APScore is %.2f, new DMG is %.2f", nAPScore, nDmg);
		if (nDmg < penetrate_dmg_threshold) return NewPenePJInfo{ false,0,0,0,0 };
		if (nDmg > ThisProj->hitDamage) nDmg = ThisProj->hitDamage;
		

		SetPeneDepthAccordingImpactRefType(imp_ref, nPene_depth);
		if (nAPScore > 0) nAPScore *= penetrate_ap_buffscale;
		else if (nAPScore < 0) nAPScore *= penetrate_hollow_buffscale;

		float SpeedScale = nDmg / ThisProj->hitDamage;
		float nSpdM = ThisProj->speedMult * std::sqrtf(SpeedScale);
		
		bool DoPenetrate = true;
		__WriteLog("[NewPenePJStats] %x PeneTimes is %d, APScore is %.2f,nDmg is %.2f,dDmg is %.2f, nSpeed is %.2f", 
			ThisProj, pene_times, nAPScore, nDmg, dDmg, (bs_proj->speed* nSpdM)); 
		__WriteLog("[NewPenePJStats] Penetrate_Dmg_Threshold is %.2f,Penetrate_Speed_Threshold is %.2f", penetrate_dmg_threshold, penetrate_speed_threshold);
		if (nAPScore <= 0 && NotMustPenetrateMaterial(impact_material) ){
			if (_isHitActor)
				DoPenetrate = false;
			else if (RandMng.GetRand(0, 100) > (penetrate_base_chance + nAPScore)) 
				DoPenetrate = false;	// Less AP_Score, Less Chance to do penetration
		}
		return NewPenePJInfo{(DoPenetrate && (nDmg > penetrate_dmg_threshold)),nDmg,nSpdM,nPene_depth,nAPScore };
	}
	
	namespace fs = std::filesystem;
	static inline bool ReadGenericConfig() {
		__WriteLog("ReadGenericConfig");
		fs::path config_root_path = fs::current_path();
		config_root_path += R"(\Data\NVSE\Plugins\PeneAndRicoConfig\)";
		if (!fs::exists(config_root_path)) {
			__WriteLog("ReadGenericConfig path not exist");
			return false;
		}

		roughinireader::INIReader _ini{ config_root_path };

		auto ret = _ini.SetCurrentINIFileName("PeneAndRico.ini");
		if (!ret.has_value()) {
			__WriteLog("Failed to set generic config filename : %s", ret.error().message());
			return false;
		}
		ret = _ini.ConstructSectionMap();
		if (!ret.has_value()) {
			__WriteLog("Failed to construct section map : %s", ret.error().message());
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

		/*raw_type_val = _ini.GetRawTypeVal("ImpactScaler", "Enable");
		temp_flag = raw_type_val.empty() ? 0 : static_cast<UINT16>(std::stoi(raw_type_val));
		if (temp_flag > 0) SetFlagOn(ImpactScaler);

		raw_type_val = _ini.GetRawTypeVal("ExtraDecal", "ExtraDecal");
		temp_flag = raw_type_val.empty() ? 0 : static_cast<UINT16>(std::stoi(raw_type_val));
		if (temp_flag > 0) SetFlagOn(ExtraDecal);*/

		__WriteLog("flag %u", bullet_manager_flag);

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

		raw_type_val = _ini.GetRawTypeVal("PenetrateGeneral", "SpeedIncreaseStep");
		speed_increase_step = raw_type_val.empty() ? 1000 : (std::stof(raw_type_val));

		raw_type_val = _ini.GetRawTypeVal("PenetrateGeneral", "SpeedDecreaseStep");
		speed_decrease_step = raw_type_val.empty() ? 500 : (std::stof(raw_type_val));

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

		__WriteLog("penetrate_dmg_threshold %f", penetrate_dmg_threshold);
		__WriteLog("penetrate_base_depth %f", penetrate_base_depth);
		__WriteLog("penetrate_max_depth %f", penetrate_max_depth);
		__WriteLog("penetrate_human_base_depth %f", penetrate_human_base_depth);
		__WriteLog("penetrate_speed_threshold %f", penetrate_speed_threshold);
		__WriteLog("penetrate_deviationX_mult %f", penetrate_deviationX_mult);
		__WriteLog("penetrate_deviationZ_mult %f", penetrate_deviationZ_mult);
		__WriteLog("penetrate_max_deviationX %f", penetrate_max_deviationX);
		__WriteLog("penetrate_max_deviationZ %f", penetrate_max_deviationZ);
		__WriteLog("ricochet_speed_threshold %f", ricochet_speed_threshold);
		__WriteLog("ricochet_damage_threshold %f", ricochet_damage_threshold);
		__WriteLog("ricochet_max_rotX %f", ricochet_max_rotX);
		__WriteLog("ricochet_max_rotZ %f", ricochet_max_rotZ);
		__WriteLog("penetrate_ap_buffscale %f", penetrate_ap_buffscale);
		__WriteLog("impact_scaler_dmg_baseline %f", impact_scaler_dmg_baseline);
		__WriteLog("impact_scaler_dmg_minline %f", impact_scaler_dmg_minline);
		__WriteLog("impact_scaler_dmg_maxline %f", impact_scaler_dmg_maxline);
		__WriteLog("impact_scaler_min %f", impact_scaler_min);
		__WriteLog("impact_scaler_max %f", impact_scaler_max);
		__WriteLog("impact_scaler_dmg_dmin_base %f", impact_scaler_dmg_dmin_base);
		__WriteLog("impact_scaler_dmg_dmax_base %f", impact_scaler_dmg_dmax_base);

		//for (auto iter = BulletManager::bullet_manager_instance().MaterialPenalty.begin(); iter != BulletManager::bullet_manager_instance().MaterialPenalty.end(); iter++)
		//{
		//	__WriteLog("MP %f",*iter);
		//} 
		return true;
	}

	static inline bool ReadArmorRicochetConfig() {
		__WriteLog("ReadArmorRicochetConfig");
		fs::path config_root_path = fs::current_path();
		config_root_path += R"(\Data\NVSE\Plugins\PeneAndRicoConfig\)";
		if (!fs::exists(config_root_path)) {
			__WriteLog("ReadArmorRicochetConfig path not exist");
			return false;
		}

		roughinireader::INIReader _ini{ config_root_path };

		auto ret = _ini.SetCurrentINIFileName("ArmorRicochet.ini");
		if (!ret.has_value()) {
			__WriteLog("Failed to set generic config filename : %s", ret.error().message());
			return false;
		}
		ret = _ini.ConstructSectionMap();
		if (!ret.has_value()) {
			__WriteLog("Failed to construct section map : %s", ret.error().message());
			return false;
		}

		// init the generic config
		std::string raw_type_val = "";
		UINT16 temp_flag = 0;
		raw_type_val = _ini.GetRawTypeVal("General", "HelmetRicochet");
		temp_flag = raw_type_val.empty() ? 0 : static_cast<UINT16>(std::stoi(raw_type_val));
		if (temp_flag > 0) SetFlagOn(HelmetRicochet);

		raw_type_val = _ini.GetRawTypeVal("General", "ArmorRicochet");
		temp_flag = raw_type_val.empty() ? 0 : static_cast<UINT16>(std::stoi(raw_type_val));
		if (temp_flag > 0) SetFlagOn(ArmorRicochet);

		raw_type_val = _ini.GetRawTypeVal("General", "BaseChance");
		ARMORicochet::SetArmorRicochetBaseChance(raw_type_val.empty() ? 0 : (std::stof(raw_type_val))); 

		raw_type_val = _ini.GetRawTypeVal("General", "HelmetChanceBuff");
		raw_type_val.empty() ? SetRicochetBuff(helmet,type,5) : SetRicochetBuff(helmet, type, (std::stof(raw_type_val)) );

		raw_type_val = _ini.GetRawTypeVal("General", "ArmorChanceBuff");
		raw_type_val.empty() ? SetRicochetBuff(armor, type, 5) : SetRicochetBuff(armor, type, (std::stof(raw_type_val))); 

		raw_type_val = _ini.GetRawTypeVal("General", "HelmetChanceMult");
		raw_type_val.empty() ? SetRicochetBuff(helmet, mult, 1) : SetRicochetBuff(helmet, mult, (std::stof(raw_type_val))); 

		raw_type_val = _ini.GetRawTypeVal("General", "ArmorChanceMult");
		raw_type_val.empty() ? SetRicochetBuff(armor, mult, 1) : SetRicochetBuff(armor, mult, (std::stof(raw_type_val))); 

		raw_type_val = _ini.GetRawTypeVal("LuckyBuff", "HelmetLuckyBuff");
		raw_type_val.empty() ? SetRicochetBuff(helmet, lucky, 0.5) : SetRicochetBuff(helmet, lucky, (std::stof(raw_type_val))); 
		raw_type_val = _ini.GetRawTypeVal("LuckyBuff", "ArmorLuckyBuff");
		raw_type_val.empty() ? SetRicochetBuff(armor, lucky, 0.5) : SetRicochetBuff(armor, lucky, (std::stof(raw_type_val))); 

		raw_type_val = _ini.GetRawTypeVal("Light", "HelmetBuff");
		raw_type_val.empty() ? SetRicochetBuff(helmet, light, 0) : SetRicochetBuff(helmet, light, (std::stof(raw_type_val))); 
		raw_type_val = _ini.GetRawTypeVal("Light", "ArmorBuff");
		raw_type_val.empty() ? SetRicochetBuff(armor, light, 0) : SetRicochetBuff(armor, light, (std::stof(raw_type_val)));

		raw_type_val = _ini.GetRawTypeVal("Mid", "HelmetBuff");
		raw_type_val.empty() ? SetRicochetBuff(helmet, mid, 0) : SetRicochetBuff(helmet, mid, (std::stof(raw_type_val)));
		raw_type_val = _ini.GetRawTypeVal("Mid", "ArmorBuff");
		raw_type_val.empty() ? SetRicochetBuff(armor, mid, 0) : SetRicochetBuff(armor, mid, (std::stof(raw_type_val)));

		raw_type_val = _ini.GetRawTypeVal("Heavy", "HelmetBuff");
		raw_type_val.empty() ? SetRicochetBuff(helmet, heavy, 0) : SetRicochetBuff(helmet, heavy, (std::stof(raw_type_val)));
		raw_type_val = _ini.GetRawTypeVal("Heavy", "ArmorBuff");
		raw_type_val.empty() ? SetRicochetBuff(armor, heavy, 0) : SetRicochetBuff(armor, heavy, (std::stof(raw_type_val)));

		raw_type_val = _ini.GetRawTypeVal("Metal", "HelmetBuff");
		raw_type_val.empty() ? SetRicochetBuff(helmet, metal, 5) : SetRicochetBuff(helmet, metal, (std::stof(raw_type_val)));
		raw_type_val = _ini.GetRawTypeVal("Metal", "ArmorBuff");
		raw_type_val.empty() ? SetRicochetBuff(armor, metal, 5) : SetRicochetBuff(armor, metal, (std::stof(raw_type_val)));

		raw_type_val = _ini.GetRawTypeVal("PowerArmor", "HelmetBuff");
		raw_type_val.empty() ? SetRicochetBuff(helmet, power_armor, 7) : SetRicochetBuff(helmet, power_armor, (std::stof(raw_type_val)));
		raw_type_val = _ini.GetRawTypeVal("PowerArmor", "ArmorBuff");
		raw_type_val.empty() ? SetRicochetBuff(armor, power_armor, 7) : SetRicochetBuff(armor, power_armor, (std::stof(raw_type_val)));

		raw_type_val = _ini.GetRawTypeVal("DTBuff", "HelmetBuff");
		raw_type_val.empty() ? SetRicochetBuff(helmet, dt_buff, 0) : SetRicochetBuff(helmet, dt_buff, (std::stof(raw_type_val)));
		raw_type_val = _ini.GetRawTypeVal("DTBuff", "ArmorBuff");
		raw_type_val.empty() ? SetRicochetBuff(armor, dt_buff, 0) : SetRicochetBuff(armor, dt_buff, (std::stof(raw_type_val)));

		raw_type_val = _ini.GetRawTypeVal("DRBuff", "HelmetBuff");
		raw_type_val.empty() ? SetRicochetBuff(helmet, dr_buff, 0) : SetRicochetBuff(helmet, dr_buff, (std::stof(raw_type_val)));
		raw_type_val = _ini.GetRawTypeVal("DRBuff", "ArmorBuff");
		raw_type_val.empty() ? SetRicochetBuff(armor, dr_buff, 0) : SetRicochetBuff(armor, dr_buff, (std::stof(raw_type_val)));
	
		ARMORicochet::ArmorRicoDebug();
	}

/* ================================================

	========== ExtraDecalAndImpactEffect ==========

   ================================================*/

	namespace ExtraDecalAndImpactEffect {
		enum class ACTOR_IMPACT_CONFIG : UINT32 {
			Min_Enter_Scale,
			Max_Enter_Scale,
			Min_Exit_Dmg,
			Min_Exit_Scale,
			Max_Exit_Scale,
			Extra_Decal_Display_Distance_Square,
			Extra_Decal_Display_Heading_Angle,
			COUNT
		};

		enum class OBJECT_IMPACT_CONFIG : UINT32 {
			Benchmark_Damage,
			Min_Impact_Scale,
			Max_Impact_Scale,
			Extra_Decal_Display_Distance_Square,
			Extra_Decal_Display_Heading_Angle,
			COUNT
		};

		enum class CONFIG_FLAG : UINT32 {
			CFG_GENERAL_NPC_ENABLE = 0x0001,
			CFG_AP_CONSTANTS_SCALE = 0x0002,
			CFG_FACEGEN_DECAL = 0x0004,
			CFG_OBJECT_DECAL_SCALE = 0x0008,
		};
		
		static inline OptFlag<CONFIG_FLAG> config_flag{};

		struct ScaleEvalResult {
			bool doExit;
			float enterScale;
			float exitScale;
		};

		// calculate new decal scale
		

		struct ExtraDecalInfo {
			// exit pos, should be set when projectile make penetration
			NiVector3 exitPos = NiVector3{0.0,0.0,0.0};
			// only set to true when a projectile make penetration
			
			// set true when the projectile recorded.
			bool valid = false;
			
			// set true when the projectile will do penetrated.
			bool doExit = false;

			float enterScale = 0.0f;
			
			float exitScale = 0.0f;

			BGSImpactData* tempEffectInfo = nullptr;

			NiNode* organismNode = nullptr;

			void __forceinline SetScale(float _enterScale, float _exitScale) {
				enterScale = _enterScale;
				exitScale = _exitScale;
			}

			// check if projectile is in recorder
			bool __forceinline IsValid() const {
				return valid;
			}

			bool __forceinline IsDoExit() const {
				return doExit;
			}

			const __forceinline NiVector3& GetExitPos() const {
				return exitPos;
			}

			const __forceinline BGSImpactData* GetBGSImpactData() const{
				return tempEffectInfo;
			}

			const __forceinline NiNode* GetOrganismNode() const {
				return organismNode;
			}

			void SetOrganismNode(hkpRigidBody* _rigidBody) {
				if (_rigidBody){
					if (hkCdBody* cdBody = &(_rigidBody->cdBody)){
						if (auto* unkNode = CdBodyUnkGetNode(cdBody)) {
							organismNode = unkNode;
						}
					}
				}
			}
		};
		
		struct ExtraDecal {
			/*
				record the projectiles and mark them as do penetration or not
				That is, whether or not the projectile penetrates, it will be record.
			*/
			std::unordered_map<Projectile*, ExtraDecalInfo> projectile_recorder{};
			std::unordered_set<Decal*> _decal_recorder{};
			
			std::array<float, (UINT32)ACTOR_IMPACT_CONFIG::COUNT> penetrate_impact_conf{};
			std::array<float, (UINT32)OBJECT_IMPACT_CONFIG::COUNT> obj_impact_conf{};

			ExtraDecalInfo matchFailed { NiVector3{0.0F,0.0F,0.0F},false, false,-1.0f,-1.0f,nullptr,nullptr};
			ExtraDecal(ExtraDecal&&) = delete;
			ExtraDecal() {};
			~ExtraDecal() {};

			static __forceinline ExtraDecal& extra_decal_instance() {
				static ExtraDecal extra_decal_instance;
				return extra_decal_instance;
			}

			/*
			InsertNewDecalInfo
			This function is to insert extra decal infomation to record map.
			This plugin call it in projectile impact hook, before respawning penetrate proj and ricochet proj, 
				and it inserts an 'empty' ExtraDecalInfo
			'doExit' member in ExtraDecalInfo will set later if projectile make penetration.
			'valid' member will set true only when this function get called
			*/
			__forceinline void InsertNewDecalInfo(Projectile* _dp_proj, float _enterScale = 1.0f, float _exitScale = 1.0f) {
				if (const auto& dp_iter = projectile_recorder.find(_dp_proj); dp_iter != projectile_recorder.end()) {
					auto& decal_info = dp_iter->second;
					decal_info = ExtraDecalInfo{ NiVector3{0.0f,0.0f,0.0f} ,true ,false ,_enterScale ,_exitScale,nullptr, nullptr };
				}
				else projectile_recorder.try_emplace(_dp_proj, ExtraDecalInfo{ NiVector3{0.0f,0.0f,0.0f} ,true ,false ,_enterScale ,_exitScale, nullptr, nullptr });
			}

			/*
			InsertNewDecalInfo
			This function is to insert extra decal infomation to record map.
			This plugin call it when eval a projectile make penetrate or not (in penetrated projectile spawned), 
			*/
			/*__forceinline void InsertNewDecalInfo(const Projectile* _dp_proj, const NiVector3& _exit_pos,bool _doExit,float _enterScale = 1.0f,float _exitScale = 1.0f) {
				if (const auto& dp_iter = projectile_recorder.find(_dp_proj->refID); dp_iter != projectile_recorder.end()) {
					auto& decal_info = dp_iter->second;
					decal_info = ExtraDecalInfo{ _exit_pos ,_doExit ,_enterScale ,_exitScale };
				}
				else projectile_recorder.try_emplace(_dp_proj->refID, ExtraDecalInfo{ _exit_pos ,_doExit ,_enterScale ,_exitScale });
			}*/

			/*
			SetPenetratedInfoForExtraDecal
			This function is to set the exit pos of extra decal.So we call it after penetrate projectile spawned
			If projectile not found in map, return false.
			Meanwhile, it will set 'doExit' to true
			*/
			
			__forceinline bool SetPenetratedInfoForExtraDecal(Projectile* _dp_proj, const NiVector3& _exit_pos, BGSImpactData* _impact_data, hkpRigidBody* _rigidBody) {
				if (const auto& dp_iter = projectile_recorder.find(_dp_proj); dp_iter != projectile_recorder.end()) {
					auto& decal_info = dp_iter->second;
					decal_info.exitPos = _exit_pos;
					decal_info.doExit = true;
					decal_info.tempEffectInfo = _impact_data;
					decal_info.SetOrganismNode(_rigidBody);
					return true;
				}
				return false;
			}
			/*
			SetScaleForDecalInfo
			This function is to set decal scale.
			If projectile not found in map, return false.
				If in map, check 'doExit' member.

			Note that The scale calculation occur in 'TESObjectCELL::AddDecal' hook,
				So store the scale is for the temp effect scaling,
					it can be use by skined or object but seem the temp effect scaling is not much effect...
			*/
			__forceinline bool SetScaleForDecalInfo(Projectile* _dp_proj, float _enterScal , float _exitScale ) {
				if (const auto& dp_iter = projectile_recorder.find(_dp_proj); dp_iter != projectile_recorder.end()) {
					auto& decal_info = dp_iter->second;
					decal_info.enterScale = _enterScal;
					decal_info.exitScale = _exitScale;
					return decal_info.doExit;
				}
				return false;
			}

			__forceinline void EraseFromMap(Projectile* _dp_proj) {
				//__WriteLog("[Extra Decal] Erase Projectile: %x", _dp_proj);
				if (const auto& dp_iter = projectile_recorder.find(_dp_proj); dp_iter != projectile_recorder.end())
					projectile_recorder.erase(dp_iter);

				/*if (const auto& dp_iter = object_impact_proj_recorder.find(_dp_proj->refID); dp_iter != object_impact_proj_recorder.end())
					object_impact_proj_recorder.erase(dp_iter);*/
			}

			/*
			GetDecalInfo
			This function is to get decal info from record map.
			If projectile not found, return 'matchFailed' .
			The 'matchFailed' is to eval add extra decal or not,
				so the 'doExit' member in 'matchFailed' is set as false by default
			*/
			__forceinline  const ExtraDecalInfo& GetDecalInfoCst(Projectile* _proj) {
				if (const auto& dp_iter = projectile_recorder.find(_proj); dp_iter != projectile_recorder.end())
					return dp_iter->second;

				return matchFailed;
			}

			__forceinline ExtraDecalInfo& GetDecalInfo(Projectile* _proj) {
				if (const auto& dp_iter = projectile_recorder.find(_proj); dp_iter != projectile_recorder.end())
					return dp_iter->second;

				return matchFailed;
			}
		};
		
#define ExtraDecalMng PenetrateAndRicochet::ExtraDecalAndImpactEffect::ExtraDecal::extra_decal_instance()
#define ActorImpactConfig(index) PenetrateAndRicochet::ExtraDecalAndImpactEffect::ExtraDecal::extra_decal_instance().penetrate_impact_conf[static_cast<UINT32>(ACTOR_IMPACT_CONFIG::index)]
#define ObjectImpactConfig(index) PenetrateAndRicochet::ExtraDecalAndImpactEffect::ExtraDecal::extra_decal_instance().obj_impact_conf[static_cast<UINT32>(OBJECT_IMPACT_CONFIG::index)]

		static auto __forceinline CalcExtraDecalPos(const Projectile* _proj, float _depth ) {
			struct {
				bool success = false;
				NiVector3 extraDecalPos{};
			} ret;
			
			__m128 _dPos = _proj->UnitVector * _depth;
			__WriteLog("[CalcExtraDecalPos] Before Raycast");
			auto _GetNextRayBegin = GetRayCastPos((NiVector3{ _proj->position } += _dPos), _proj->rotation.x, _proj->rotation.z );
			if (_GetNextRayBegin.raycastSuccess)
			{
				NiVector3 _NextRayBeginPos{ _GetNextRayBegin.raycastPos };
				_dPos = _proj->UnitVector * -_depth;
				auto _DirectionalRes = GetRayCastPos((NiVector3{ _NextRayBeginPos } += _dPos), _proj->rotation.x, _proj->rotation.z + PI);
				if (_DirectionalRes.raycastSuccess)
				{
					ret.extraDecalPos = _DirectionalRes.raycastPos;
					ret.success = true;
				}
			}
			
			return ret;
		}

// CheckCanAddDecal
		static bool __forceinline CheckCanAddDecal(const NiVector3& _srcPos,float _zAngle, const NiVector3& _hitPos,
													float benchmarkDis,float benchmarkZAngle) {
			
			__m128 _p12_diff = _mm_sub_ps(_srcPos.PS(), _hitPos.PS());
			__m128 _mul = _mm_mul_ps(_p12_diff, _p12_diff);
			__m128 hadd1 = _mm_hadd_ps(_mul, _mul);
			__m128 hadd2 = _mm_hadd_ps(hadd1, hadd1);
			float _dis_square = _mm_cvtss_f32(hadd2);
			__WriteLog("[CheckCanAddDecal] Actor to hit Dis: %.2f", _dis_square);
			if (_dis_square > benchmarkDis)
				return false;

			float _h_ZAng = GetHeadingAngleZ_PosBased(_srcPos, _zAngle, _hitPos);
			__WriteLog("[CheckCanAddDecal] Actor to hit hAng: %.2f", _h_ZAng);
			if (std::abs(_h_ZAng ) > benchmarkZAngle)
				return false;

			__WriteLog("[CheckCanAddDecal] Can Add Decal");
			return true;
		}

		static bool __forceinline CheckCanAddDecal(const MissileProjectile* _proj,
			const Projectile::ImpactData* imp_data,
			const ImpactRefInfo& imp_ref_info) {

			return CheckCanAddDecal
			(
				_proj->sourceRef->position,
				_proj->sourceRef->rotation.z, 
				imp_data->pos,
				imp_ref_info.isHitActor() ?
					ActorImpactConfig(Extra_Decal_Display_Distance_Square) :
					ObjectImpactConfig(Extra_Decal_Display_Distance_Square),
				imp_ref_info.isHitActor() ?
					ActorImpactConfig(Extra_Decal_Display_Heading_Angle) :
					ObjectImpactConfig(Extra_Decal_Display_Heading_Angle)
			);
		}
// CheckCanAddDecal

		/*
		GetNewSkinedDecalScale
		This function is to calculate decal scale when game add skined decal.
		This plugin call it in TesObjectCell::AddDecal hook.
		Calculate the scale of decal and apply.
		the scale will use by temp effect, so it need to store in TesObjectCell::AddDecal hook
		*/
		static auto __forceinline GetNewSkinedDecalScale( float _baseDmg, float _healthDmg, bool _isAPAmmo, bool _isPenetrated, SInt32 hitLoc) {
			if (!_isPenetrated)
				return ScaleEvalResult{ false,ActorImpactConfig(Min_Enter_Scale), ActorImpactConfig(Min_Exit_Scale) };
			
			float enter_overFlowDmg = _baseDmg - ActorImpactConfig(Min_Exit_Dmg);
			float exit_overFlowDmg = _healthDmg - ActorImpactConfig(Min_Exit_Dmg);

// Enter Scale
			float newEnterScale = enter_overFlowDmg <= 0.0f ?
				ActorImpactConfig(Min_Enter_Scale) :
				(ActorImpactConfig(Min_Enter_Scale) + (enter_overFlowDmg / 100));

			if (newEnterScale > ActorImpactConfig(Max_Enter_Scale))
				newEnterScale = ActorImpactConfig(Max_Enter_Scale);
// Enter Scale

			if (_isAPAmmo)
				return ScaleEvalResult{ true,newEnterScale, newEnterScale };

// Exit Scale
			float newExitScale = exit_overFlowDmg <= 0.0f ?
								ActorImpactConfig(Min_Exit_Scale) :
								ActorImpactConfig(Min_Exit_Scale) + (exit_overFlowDmg / 100);/*
			__WriteLog("newEnterScale %.2f, newExitScale %.2f", newEnterScale, newExitScale);*/

			if (newExitScale > ActorImpactConfig(Max_Exit_Scale))
				newExitScale = ActorImpactConfig(Max_Exit_Scale);
// Exit Scale
			return ScaleEvalResult{
					true,
					newEnterScale,
					newExitScale
			};
		}

		/*
		GetNewObjcetDecalScale
		This function is to calculate decal scale when game add object decal.
		this plugin call it in TesoObjectCell::AddDecal hook.
		Calculate the scale of decal and apply.
		the scale will use by temp effect, so it need to store in TesObjectCell::AddDecal hook
		*/
		static float __forceinline GetNewObjcetDecalScale( float _healthDmg) {
			float overflowDmgRatio = (_healthDmg - ObjectImpactConfig(Benchmark_Damage)) / 100.0f;
			float Scale = (1.0f + overflowDmgRatio);
			if (Scale < ObjectImpactConfig(Min_Impact_Scale))
				Scale = ObjectImpactConfig(Min_Impact_Scale);
			else if (Scale > ObjectImpactConfig(Max_Impact_Scale))
				Scale = ObjectImpactConfig(Max_Impact_Scale);
			return Scale ;
		}

		// read ini
		static inline bool ReadGenericConfig() {
			__WriteLog("ReadGenericConfig");
			fs::path config_root_path = fs::current_path();
			config_root_path += R"(\Data\NVSE\plugins\ExtraDecalConfig\)";
			if (!fs::exists(config_root_path)) {
				__WriteLog("ReadGenericConfig path not exist");
				return false;
			}

			roughinireader::INIReader _ini{ config_root_path };
			auto ret = _ini.SetCurrentINIFileName("ExtraDecalConfig.ini");
			if (!ret.has_value()) {
				__WriteLog("Failed to set generic config filename : %s", ret.error().message());
				return false;
			}
			ret = _ini.ConstructSectionMap();
			if (!ret.has_value()) {
				__WriteLog("Failed to construct section map : %s", ret.error().message());
				return false;
			}


			std::string raw_type_val = "";
			UINT32 temp_flag = 0;
			raw_type_val = _ini.GetRawTypeVal("General", "NPCEnable");
			temp_flag = raw_type_val.empty() ? 0 : static_cast<UINT32>(std::stoi(raw_type_val));
			__WriteLog(raw_type_val.c_str());
			if (temp_flag != 0)
				config_flag.SetFlagOn(CONFIG_FLAG::CFG_GENERAL_NPC_ENABLE);

			raw_type_val = _ini.GetRawTypeVal("General", "APConstantScale");
			temp_flag = raw_type_val.empty() ? 0 : static_cast<UINT32>(std::stoi(raw_type_val));
			__WriteLog(raw_type_val.c_str());
			if (temp_flag != 0)
				config_flag.SetFlagOn(CONFIG_FLAG::CFG_AP_CONSTANTS_SCALE);

			raw_type_val = _ini.GetRawTypeVal("General", "FaceGenDecal");
			temp_flag = raw_type_val.empty() ? 0 : static_cast<UINT32>(std::stoi(raw_type_val));
			__WriteLog(raw_type_val.c_str());
			if (temp_flag != 0)
				config_flag.SetFlagOn(CONFIG_FLAG::CFG_FACEGEN_DECAL);

			raw_type_val = _ini.GetRawTypeVal("General", "ObjectDecalScale");
			temp_flag = raw_type_val.empty() ? 0 : static_cast<UINT32>(std::stoi(raw_type_val));
			__WriteLog(raw_type_val.c_str());
			if (temp_flag != 0)
				config_flag.SetFlagOn(CONFIG_FLAG::CFG_OBJECT_DECAL_SCALE);

			raw_type_val = _ini.GetRawTypeVal("General", "MaxSkinDecalsPerFrame");
			SInt32 gameINI_MaxSkinDecalsPerFrame = raw_type_val.empty() ? 6 : (std::stoi(raw_type_val));

			raw_type_val = _ini.GetRawTypeVal("General", "MaxSkinDecalPerActor");
			UINT32 gameINI_MaxSkinDecalPerActor = raw_type_val.empty() ? 18 : (UINT32)(std::stoi(raw_type_val));

			raw_type_val = _ini.GetRawTypeVal("General", "MaxSkinDecals");
			UINT32 gameINI_MaxSkinDecals = raw_type_val.empty() ? 72 : (UINT32)(std::stoi(raw_type_val));

			Setting* iMaxSkinDecalsPerFrame_Display = (Setting*)0x11C5858;
			iMaxSkinDecalsPerFrame_Display->data.i = gameINI_MaxSkinDecalsPerFrame;
			__WriteLog("set Display::iMaxSkinDecalsPerFrame to %d", iMaxSkinDecalsPerFrame_Display->data.i);

			Setting* uMaxSkinDecalPerActor_Decals = (Setting*)0x11C589C;
			uMaxSkinDecalPerActor_Decals->data.uint = gameINI_MaxSkinDecalPerActor;
			__WriteLog("set Decals::uMaxSkinDecalPerActor to %u", uMaxSkinDecalPerActor_Decals->data.uint);

			Setting* uMaxSkinDecals_Decals = (Setting*)0x11C58E4;
			uMaxSkinDecals_Decals->data.uint = gameINI_MaxSkinDecals;
			__WriteLog("set Decal::uMaxSkinDecals to %u", uMaxSkinDecals_Decals->data.uint);



			// ACTOR_IMPACT_CONFIG
			raw_type_val = _ini.GetRawTypeVal("ActorImpactDecalScaleConfig", "MinEnterScale");
			ActorImpactConfig(Min_Enter_Scale) = raw_type_val.empty() ? 0.15 : (std::stof(raw_type_val));

			raw_type_val = _ini.GetRawTypeVal("ActorImpactDecalScaleConfig", "MaxEnterScale");
			ActorImpactConfig(Max_Enter_Scale) = raw_type_val.empty() ? 0.75 : (std::stof(raw_type_val));

			raw_type_val = _ini.GetRawTypeVal("ActorImpactDecalScaleConfig", "MinExitDamage");
			ActorImpactConfig(Min_Exit_Dmg) = raw_type_val.empty() ? 15 : (std::stof(raw_type_val));

			raw_type_val = _ini.GetRawTypeVal("ActorImpactDecalScaleConfig", "MinExitScale");
			ActorImpactConfig(Min_Exit_Scale) = raw_type_val.empty() ? 0.75 : (std::stof(raw_type_val));

			raw_type_val = _ini.GetRawTypeVal("ActorImpactDecalScaleConfig", "MaxExitScale");
			ActorImpactConfig(Max_Exit_Scale) = raw_type_val.empty() ? 1.25 : (std::stof(raw_type_val));

			raw_type_val = _ini.GetRawTypeVal("ActorImpactDecalScaleConfig", "DisappearDistance");
			ActorImpactConfig(Extra_Decal_Display_Distance_Square) = raw_type_val.empty() ? (8192.0)*(8192.0) : (std::stof(raw_type_val) * std::stof(raw_type_val));

			raw_type_val = _ini.GetRawTypeVal("ActorImpactDecalScaleConfig", "DisappearHeadingAngle");
			ActorImpactConfig(Extra_Decal_Display_Heading_Angle) = raw_type_val.empty() ? 90 : (std::stof(raw_type_val) );

			__WriteLog("[ActorImpactConfig] Min_Enter_Scale: %.2f", ActorImpactConfig(Min_Enter_Scale));
			__WriteLog("[ActorImpactConfig] Max_Enter_Scale: %.2f", ActorImpactConfig(Max_Enter_Scale));
			__WriteLog("[ActorImpactConfig] Min_Exit_Dmg: %.2f", ActorImpactConfig(Min_Exit_Dmg));
			__WriteLog("[ActorImpactConfig] Min_Exit_Scale: %.2f", ActorImpactConfig(Min_Exit_Scale));
			__WriteLog("[ActorImpactConfig] Max_Exit_Scale: %.2f", ActorImpactConfig(Max_Exit_Scale));
			__WriteLog("[ActorImpactConfig] Extra_Decal_Display_Distance_Square: %.2f", ActorImpactConfig(Extra_Decal_Display_Distance_Square));
			__WriteLog("[ActorImpactConfig] Extra_Decal_Display_Heading_Angle: %.2f", ActorImpactConfig(Extra_Decal_Display_Heading_Angle));

			// OBJECT_IMPACT_CONFIG
			raw_type_val = _ini.GetRawTypeVal("ObjectImpactDecalScaleConfig", "BenchmarkDamage");
			ObjectImpactConfig(Benchmark_Damage) = raw_type_val.empty() ? 35 : (std::stof(raw_type_val));

			raw_type_val = _ini.GetRawTypeVal("ObjectImpactDecalScaleConfig", "MinImpactScale");
			ObjectImpactConfig(Min_Impact_Scale) = raw_type_val.empty() ? 0.33 : (std::stof(raw_type_val));

			raw_type_val = _ini.GetRawTypeVal("ObjectImpactDecalScaleConfig", "MaxImpactScale");
			ObjectImpactConfig(Max_Impact_Scale) = raw_type_val.empty() ? 1.75 : (std::stof(raw_type_val));

			raw_type_val = _ini.GetRawTypeVal("ObjectImpactDecalScaleConfig", "DisappearDistance");
			ObjectImpactConfig(Extra_Decal_Display_Distance_Square) = raw_type_val.empty() ? (8092.0) * (8092.0) : (std::stof(raw_type_val) * std::stof(raw_type_val));

			raw_type_val = _ini.GetRawTypeVal("ObjectImpactDecalScaleConfig", "DisappearHeadingAngle");
			ObjectImpactConfig(Extra_Decal_Display_Heading_Angle) = raw_type_val.empty() ? 90 : (std::stof(raw_type_val));
			
			__WriteLog("[ObjectImpactConfig] Benchmark_Damage: %.2f", ObjectImpactConfig(Benchmark_Damage));
			__WriteLog("[ObjectImpactConfig] Min_Impact_Scale: %.2f", ObjectImpactConfig(Min_Impact_Scale));
			__WriteLog("[ObjectImpactConfig] Max_Impact_Scale: %.2f", ObjectImpactConfig(Max_Impact_Scale));
			__WriteLog("[ObjectImpactConfig] Extra_Decal_Display_Distance_Square: %.2f", ObjectImpactConfig(Extra_Decal_Display_Distance_Square));
			__WriteLog("[ObjectImpactConfig] Extra_Decal_Display_Heading_Angle: %.2f", ObjectImpactConfig(Extra_Decal_Display_Heading_Angle));
			__WriteLog("extra decal config %d", config_flag.flag);
		}
	}
	// Extra Decal Namespace
};

/*
struct lastDecalInfo {
	NiVector3 exitPos;
	bool doExit;
	float enterScale;
	float exitScale;
};

static std::unordered_map<Projectile*, lastDecalInfo> actor_impact_proj_recorder{};
static std::unordered_map<Projectile*, float> object_impact_proj_recorder{};
static std::unordered_set<Decal*> _decal_recorder{};
*/

