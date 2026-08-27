#pragma once

#include "Animation/AnimInstance.h"
#include "Animation/AnimInstanceProxy.h"
#include "Animation/AnimNode_SequencePlayer.h"
#include "AnimNodes/AnimNode_CopyPoseFromMesh.h"
#include "AnimNodes/AnimNode_Slot.h"
#include "AnimNodes/AnimNode_TwoWayBlend.h"
#include "FPSFirstPersonAnimInstance.generated.h"

USTRUCT()
struct FPSGAME_API FFPSFirstPersonAnimInstanceProxy : public FAnimInstanceProxy
{
    GENERATED_BODY()

    FFPSFirstPersonAnimInstanceProxy() = default;
    explicit FFPSFirstPersonAnimInstanceProxy(UAnimInstance* AnimInstance)
        : FAnimInstanceProxy(AnimInstance)
    {
    }

    virtual void Initialize(UAnimInstance* AnimInstance) override;
    virtual void PreUpdate(UAnimInstance* AnimInstance, float DeltaSeconds) override;
    virtual void UpdateAnimationNode(const FAnimationUpdateContext& Context) override;
    virtual bool Evaluate(FPoseContext& Output) override;

private:
    FAnimNode_CopyPoseFromMesh CopyPoseFromParent;
    FAnimNode_SequencePlayer_Standalone SprintPosePlayer;
    FAnimNode_SequencePlayer_Standalone AimPosePlayer;
    FAnimNode_TwoWayBlend SprintBlend;
    FAnimNode_TwoWayBlend AimBlend;
    FAnimNode_Slot ArmsSlot;
};

UCLASS(Transient, NotBlueprintable)
class FPSGAME_API UFPSFirstPersonAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UFPSFirstPersonAnimInstance(const FObjectInitializer& ObjectInitializer);

protected:
    virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;
};
