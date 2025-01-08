// Copyright Druid Mechanics


#include "AbilitySystem/Abilities/AuraProjectileSpell.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Actor/AuraProjectile.h"
#include "Interaction/CombatInterface.h"
#include "Aura/Public/AuraGameplayTags.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"


void UAuraProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                           const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
}

void UAuraProjectileSpell::SpawnProjectile(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag)
{
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
	if (!bIsServer) return;
	
	const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(
		GetAvatarActorFromActorInfo(),
		SocketTag);
	FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();
	Rotation = UKismetMathLibrary::FindLookAtRotation(SocketLocation, ProjectileTargetLocation);
	
	//DrawDebugDirectionalArrow(GetWorld(), SocketLocation, ProjectileTargetLocation, 15.f , FColor::Red, false, 1.f);
	
	FTransform SpawnTransform;
	SpawnTransform.SetLocation(SocketLocation);
		
	AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
		ProjectileClass,
		SpawnTransform,
		GetOwningActorFromActorInfo(),
		Cast<APawn>(GetOwningActorFromActorInfo()),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	const float GravityZ = Projectile->ProjectileMovement.Get()->GetGravityZ();
	const float MaxSpeed = Projectile->ProjectileMovement.Get()->GetMaxSpeed();
	FVector OutLaunchVelocity = FVector(0, 0, 0);

	UGameplayStatics::FSuggestProjectileVelocityParameters VelocityParameters = UGameplayStatics::FSuggestProjectileVelocityParameters(this, SocketLocation, ProjectileTargetLocation, MaxSpeed);
	VelocityParameters.OverrideGravityZ = GravityZ;
	VelocityParameters.bDrawDebug = false;
	VelocityParameters.TraceOption = ESuggestProjVelocityTraceOption::DoNotTrace;
	if (GravityZ != 0 && UGameplayStatics::SuggestProjectileVelocity(VelocityParameters, OutLaunchVelocity))
	{
		Rotation = OutLaunchVelocity.GetSafeNormal().Rotation();
	}
	
	SpawnTransform.SetRotation(Rotation.Quaternion());

	
	const UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());
	FGameplayEffectContextHandle EffectContextHandle = SourceASC->MakeEffectContext();
	EffectContextHandle.SetAbility(this);
	EffectContextHandle.AddSourceObject(Projectile);
	TArray<TWeakObjectPtr<AActor>> Actors;
	Actors.Add(Projectile);
	EffectContextHandle.AddActors(Actors);
	FHitResult HitResult;
	HitResult.Location = ProjectileTargetLocation;
	EffectContextHandle.AddHitResult(HitResult);

	const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), EffectContextHandle);

	const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();

	for (auto& Pair : DamageTypes)
	{
		const float ScaledDamage = Pair.Value.GetValueAtLevel(GetAbilityLevel());
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, Pair.Key, ScaledDamage);
	}
		
	Projectile->DamageEffectSpecHandle = SpecHandle;
		
	Projectile->FinishSpawning(SpawnTransform);
}
