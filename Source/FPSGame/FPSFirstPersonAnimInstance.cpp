#include "FPSFirstPersonAnimInstance.h"
#include "FPSCharacter.h"

void FFPSFirstPersonAnimInstanceProxy::Initialize(UAnimInstance* AnimInstance)
{
    FAnimInstanceProxy::Initialize(AnimInstance);

    CopyPoseFromParent.bUseAttachedParent = true;
    CopyPoseFromParent.bCopyCurves = true;
    CopyPoseFromParent.bCopyCustomAttributes = true;

    SprintPosePlayer.SetLoopAnimation(true);
    AimPosePlayer.SetLoopAnimation(true);
    SprintBlend.AlphaInputType = EAnimAlphaInputType::Float;
    SprintBlend.Alpha = 0.0f;
    SprintBlend.A.SetLinkNode(&CopyPoseFromParent);
    SprintBlend.B.SetLinkNode(&SprintPosePlayer);
    AimBlend.AlphaInputType = EAnimAlphaInputType::Float;
    AimBlend.Alpha = 0.0f;
    AimBlend.A.SetLinkNode(&SprintBlend);
    AimBlend.B.SetLinkNode(&AimPosePlayer);

    ArmsSlot.SlotName = TEXT("Arms");
    ArmsSlot.bAlwaysUpdateSourcePose = true;
    ArmsSlot.Source.SetLinkNode(&AimBlend);

    FAnimationInitializeContext InitializeContext(this);
    ArmsSlot.Initialize_AnyThread(InitializeContext);
}

void FFPSFirstPersonAnimInstanceProxy::PreUpdate(UAnimInstance* AnimInstance, float DeltaSeconds)
{
    FAnimInstanceProxy::PreUpdate(AnimInstance, DeltaSeconds);
    CopyPoseFromParent.PreUpdate(AnimInstance);

    const AFPSCharacter* Character = AnimInstance
        ? Cast<AFPSCharacter>(AnimInstance->TryGetPawnOwner())
        : nullptr;
    const float SprintTarget = Character && Character->IsSprinting() ? 1.0f : 0.0f;
    const float AimTarget = Character && Character->IsAiming() ? 1.0f : 0.0f;
    if (Character)
    {
        SprintPosePlayer.SetSequence(Character->GetSprintPoseAnimation());
        SprintPosePlayer.SetPlayRate(Character->GetSprintPosePlayRate());
        AimPosePlayer.SetSequence(Character->GetAimPoseAnimation());
        AimPosePlayer.SetPlayRate(1.0f);
    }
    SprintBlend.Alpha = FMath::FInterpTo(
        SprintBlend.Alpha, SprintTarget, DeltaSeconds, 10.0f);
    AimBlend.Alpha = FMath::FInterpTo(
        AimBlend.Alpha, AimTarget, DeltaSeconds, 14.0f);
}

void FFPSFirstPersonAnimInstanceProxy::UpdateAnimationNode(const FAnimationUpdateContext& Context)
{
    UpdateCounter.Increment();
    ArmsSlot.Update_AnyThread(Context);
}

bool FFPSFirstPersonAnimInstanceProxy::Evaluate(FPoseContext& Output)
{
    ArmsSlot.Evaluate_AnyThread(Output);
    return true;
}

UFPSFirstPersonAnimInstance::UFPSFirstPersonAnimInstance(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bUseMultiThreadedAnimationUpdate = false;
}

FAnimInstanceProxy* UFPSFirstPersonAnimInstance::CreateAnimInstanceProxy()
{
    return new FFPSFirstPersonAnimInstanceProxy(this);
}
