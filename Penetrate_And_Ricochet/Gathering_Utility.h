#pragma once
#include "Gathering_Code.h"
#define PC_Ref PlayerCharacter::GetSingleton()
#define Ut_Square(x) ( (x) * (x) ) 


static __forceinline bool PJIsExplosion(const BGSProjectile* basepj) {
	return (basepj->projFlags & 2) != 0 && (basepj->type & 1) != 0;
}

static __forceinline float CalcProjSpeed(const BGSProjectile* basepj, const Projectile* _pjref) {
	return (basepj->speed * _pjref->speedMult);
}

static __forceinline SceneGraph* GetSceneGraph() {
	return (*(SceneGraph**)0x11DEB7C);
}

static __forceinline float CalcPosSquareDis(const NiVector3& posA, const NiVector3& posB) {
    return (Ut_Square((posA.x - posB.x)) + Ut_Square((posA.y - posB.y)) + Ut_Square((posA.z - posB.z)));
}

static __forceinline void CalcMidPoint3(const NiVector3& posA,const NiVector3& posB,NiVector3& out_mid) {
    out_mid.x = (posA.x + posB.x) / 2;
    out_mid.y = (posA.y + posB.y) / 2;
    out_mid.z = (posA.z + posB.z) / 2;
}

static __forceinline float GetRefDistanceSquare(const TESObjectREFR* _refA, const TESObjectREFR* _refB) {
	return (Ut_Square(_refB->posX - _refA->posX) + Ut_Square(_refB->posY - _refA->posY) + Ut_Square(_refB->posZ - _refA->posZ));
}

static __forceinline float GetRefDistanceSquare2D(const TESObjectREFR* _refA, const TESObjectREFR* _refB) {
	return (Ut_Square(_refB->posX - _refA->posX) + Ut_Square(_refB->posY - _refA->posY));
}

static __forceinline bool IsPJHitscan(const BGSProjectile* _baseproj) {
	return (_baseproj->projFlags & BGSProjectile::kFlags_Hitscan) != 0;
}

static __forceinline float GetPJMuzzleFlashDuration(const BGSProjectile* _baseproj) {
	return _baseproj->flashDuration;
}

static __forceinline bool IsPJHitscan(const Projectile* _proj) {
	return (((BGSProjectile*)_proj->baseForm)->projFlags & BGSProjectile::kFlags_Hitscan) != 0;
}

static __forceinline BGSImpactDataSet* GetImpactDataSet(const TESObjectWEAP* _weap) {
	return _weap->impactDataSet;
}

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

alignas(16) const UInt8 kMaterialConvert[] =
{
    kMaterial_Stone,
    kMaterial_Cloth,
    kMaterial_Dirt,
    kMaterial_Glass,
    kMaterial_Grass,
    kMaterial_Metal,
    kMaterial_Organic,
    kMaterial_Organic,
    kMaterial_Water,
    kMaterial_Wood,
    kMaterial_Stone,
    kMaterial_Metal,
    kMaterial_Wood,
    kMaterial_Metal,
    kMaterial_Metal,
    kMaterial_Metal,
    kMaterial_HollowMetal,
    kMaterial_HollowMetal,
    kMaterial_Dirt,
    kMaterial_Stone,
    kMaterial_Metal,
    kMaterial_Metal,
    kMaterial_HollowMetal,
    kMaterial_HollowMetal,
    kMaterial_Glass,
    kMaterial_HollowMetal,
    kMaterial_Metal,
    kMaterial_Metal,
    kMaterial_HollowMetal,
    kMaterial_HollowMetal,
    kMaterial_Wood,
    kMaterial_Wood
};

// Deal with the material type from the 'raw' hit material which is recorded in impact data
static __forceinline UINT8 MaterialConverter(UINT32 hitmaterial) {
    return (hitmaterial <= 31 ? kMaterialConvert[hitmaterial] : (UINT8)255);
}

/*
 ==== material ====
0	Stone | 1	Dirt | 2	Grass | 3	Glass
4	Metal | 5	Wood | 6  Organic | 7	Cloth
8	Water | 9	Hollow Metal | 10	Organic Bug | 11	Organic Glow
*/
static __forceinline BGSImpactData* GetImpactDataByMaterial(const TESObjectWEAP* _weap,UINT8 _material) {
	if (_material < 12) return GetImpactDataSet(_weap)->impactDatas[_material];
	return nullptr;
}