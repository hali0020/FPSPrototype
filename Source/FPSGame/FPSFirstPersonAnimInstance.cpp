#include "FPSFirstPersonAnimInstance.h"

void FFPSFirstPersonAnimInstanceProxy::Initialize(UAnimInstance* AnimInstance)
{
    FAnimInstanceProxy::Initialize(AnimInstance);

    CopyPoseFromParent.bUseAttachedParent = true;
    CopyPoseFromParent.bCopyCurves = true;
    CopyPoseFromParent.bCopyCustomAttributes = true;
    ArmsSlot.SlotName = TEXT("Arms");
    ArmsSlot.bAlwaysUpdateSourcePose = true;
    ArmsSlot.Source.SetLinkNode(&CopyPoseFromParent);

    FAnimationInitializeContext InitializeContext(this);
    ArmsSlot.Initialize_AnyThread(InitializeContext);
}

void FFPSFirstPersonAnimInstanceProxy::PreUpdate(UAnimInstance* AnimInstance, float DeltaSeconds)
{
    FAnimInstanceProxy::PreUpdate(AnimInstance, DeltaSeconds);
    CopyPoseFromParent.PreUpdate(AnimInstance);
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
