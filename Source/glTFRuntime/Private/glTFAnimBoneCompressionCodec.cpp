// Copyright 2020, Roberto De Ioris.


#include "glTFAnimBoneCompressionCodec.h"

void UglTFAnimBoneCompressionCodec::DecompressBone(FAnimSequenceDecompressionContext& DecompContext, int32 TrackIndex, FTransform& OutAtom) const
{
	if (TrackIndex >= Tracks.Num())
	{
		return;
	}

	int32 FrameA = 0;
	int32 FrameB = 0;

	if (Tracks[TrackIndex].PosKeys.Num() > 0)
	{
		float Alpha = TimeToIndex(DecompContext.SequenceLength, DecompContext.RelativePos, Tracks[TrackIndex].PosKeys.Num(), DecompContext.Interpolation, FrameA, FrameB);
		FVector Location = FMath::Lerp(Tracks[TrackIndex].PosKeys[FrameA], Tracks[TrackIndex].PosKeys[FrameB], Alpha);
		OutAtom.SetLocation(Location);
	}

	if (Tracks[TrackIndex].RotKeys.Num() > 0)
	{
		float Alpha = TimeToIndex(DecompContext.SequenceLength, DecompContext.RelativePos, Tracks[TrackIndex].RotKeys.Num(), DecompContext.Interpolation, FrameA, FrameB);
		FQuat Rotation = FQuat::Slerp(Tracks[TrackIndex].RotKeys[FrameA], Tracks[TrackIndex].RotKeys[FrameB], Alpha);
		OutAtom.SetRotation(Rotation);
	}

	if (Tracks[TrackIndex].ScaleKeys.Num() > 0)
	{
		float Alpha = TimeToIndex(DecompContext.SequenceLength, DecompContext.RelativePos, Tracks[TrackIndex].ScaleKeys.Num(), DecompContext.Interpolation, FrameA, FrameB);
		FVector Scale = FMath::Lerp(Tracks[TrackIndex].ScaleKeys[FrameA], Tracks[TrackIndex].ScaleKeys[FrameB], Alpha);
		OutAtom.SetScale3D(Scale);
	}
}

void UglTFAnimBoneCompressionCodec::DecompressPose(FAnimSequenceDecompressionContext& DecompContext, const BoneTrackArray& RotationPairs, const BoneTrackArray& TranslationPairs, const BoneTrackArray& ScalePairs, TArrayView<FTransform>& OutAtoms) const
{
	for (int32 TrackIndex = 0; TrackIndex < OutAtoms.Num(); TrackIndex++)
	{
		DecompressBone(DecompContext, TrackIndex, OutAtoms[TrackIndex]);
	}
}

// Taken from official Unreal Engine code base.
float UglTFAnimBoneCompressionCodec::TimeToIndex(
	float SequenceLength,
	float RelativePos,
	int32 NumKeys,
	EAnimInterpolationType Interpolation,
	int32& PosIndex0Out,
	int32& PosIndex1Out) const
{
	float Alpha;

	if (NumKeys < 2)
	{
		checkSlow(NumKeys == 1); // check if data is empty for some reason.
		PosIndex0Out = 0;
		PosIndex1Out = 0;
		return 0.0f;
	}
	// Check for before-first-frame case.
	if (RelativePos <= 0.f)
	{
		PosIndex0Out = 0;
		PosIndex1Out = 0;
		Alpha = 0.0f;
	}
	else
	{
		NumKeys -= 1; // never used without the minus one in this case
		// Check for after-last-frame case.
		if (RelativePos >= 1.0f)
		{
			// If we're not looping, key n-1 is the final key.
			PosIndex0Out = NumKeys;
			PosIndex1Out = NumKeys;
			Alpha = 0.0f;
		}
		else
		{
			// For non-looping animation, the last frame is the ending frame, and has no duration.
			const float KeyPos = RelativePos * float(NumKeys);
			checkSlow(KeyPos >= 0.0f);
			const float KeyPosFloor = floorf(KeyPos);
			PosIndex0Out = FMath::Min(FMath::TruncToInt(KeyPosFloor), NumKeys);
			Alpha = (Interpolation == EAnimInterpolationType::Step) ? 0.0f : KeyPos - KeyPosFloor;
			PosIndex1Out = FMath::Min(PosIndex0Out + 1, NumKeys);
		}
	}
	return Alpha;
}