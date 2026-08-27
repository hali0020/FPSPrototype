#include "FPSFirstPersonAnimInstance.h"
#include "FPSCharacter.h"

void FFPSFirstPersonAnimInstanceProxy::Initialize(UAnimInstance* AnimInstance)
{
    FAnimInstanceProxy::Initialize(AnimInstance);

    CopyPoseFromParent.bUseAttachedParent = true;
    CopyPoseFromParent.bCopyCurves = true;
    CopyPoseFromParent.bCopyCustomAttributes = true;

    SprintPosePlayer.SetLoopAnimation(true);
    SprintBlend.AlphaInputType = EAnimAlphaInputType::Float;
    SprintBlend.Alpha = 0.0f;
    SprintBlend.A.SetLinkNode(&CopyPoseFromParent);
    SprintBlend.B.SetLinkNode(&SprintPosePlayer);

    ArmsSlot.SlotName = TEXT("Arms");
    ArmsSlot.bAlwaysUpdateSourcePose = true;
    ArmsSlot.Source.SetLinkNode(&SprintBlend);

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
    if (Character)
    {
        SprintPosePlayer.SetSequence(Character->GetSprintPoseAnimation());
        SprintPosePlayer.SetPlayRate(Character->GetSprintPosePlayRate());
    }
    SprintBlend.Alpha = FMath::FInterpTo(
        SprintBlend.Alpha, SprintTarget, DeltaSeconds, 10.0f);
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
