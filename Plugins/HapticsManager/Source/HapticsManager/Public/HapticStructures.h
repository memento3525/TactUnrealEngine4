// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Engine.h"
#include "CoreMinimal.h"
#include "HapticStructures.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EPosition : uint8
{
	All = 0, Left = 1, Right = 2,
	Vest = 3,
	Head = 4,
	VestFront = 201, VestBack = 202,
	GloveLeft = 203, GloveRight = 204,
	Custom1 = 251, Custom2 = 252, Custom3 = 253, Custom4 = 254
};

UENUM(BlueprintType)
enum class EFeeddbackMode: uint8
{
	PATH_MODE	UMETA(DisplayName = "PATHMODE"),
	DOT_MODE	UMETA(DisplayName = "DOTMODE")
};

USTRUCT(BlueprintType)
struct FDotPoint
{
	GENERATED_BODY()

	FDotPoint()
	{
		Index = 0;
		Intensity = 0;
	}

	FDotPoint(int32 _index, int32 _intensity)
	{
		Index = _index;
		Intensity = _intensity;
	}
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = Vars)
	int32 Index;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Vars)
	int32 Intensity;
};

USTRUCT(BlueprintType)
struct FPathPoint
{
	GENERATED_BODY()
	FPathPoint()
	{
		X = 0;
		Y = 0;
		Intensity = 0;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Vars)
	float X;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Vars)
	float Y;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Vars)
	int32 Intensity;

	FPathPoint(float _x, float _y, int32 _intensity)
	{
		int XRound = _x * 1000;
		int YRound = _y * 1000;
		X = XRound/1000;
		Y = YRound / 1000;
		Intensity = _intensity;
	}
};

USTRUCT(BlueprintType)
struct FHapticFeedbackFrame
{
	GENERATED_BODY()
	
	FHapticFeedbackFrame()
	{
		Position = EPosition::All;
		PathPoints.Add(FPathPoint());
		DotPoints.Add(FDotPoint());
		Texture = 0;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Vars)
	TArray<FPathPoint> PathPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Vars)
	TArray<FDotPoint> DotPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Vars)
	int32 Texture;

	UPROPERTY(BlueprintReadWrite, Category = Vars)
	EPosition Position;

	FHapticFeedbackFrame(EPosition _pos, const TArray<FPathPoint> &_pathPoints)
	{
		Position = _pos;
		PathPoints = _pathPoints;
		Texture = 0;
	}

	FHapticFeedbackFrame(EPosition _pos, const TArray<FDotPoint> &_dotPoints)
	{
		Position = _pos;
		DotPoints = _dotPoints;
		Texture = 0;
	}
};

USTRUCT(BlueprintType)
struct FHapticFeedback
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Vars)
	EPosition Position;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Vars)
	EFeeddbackMode Mode;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Vars)
	TArray<uint8> Values;

	FHapticFeedback(EPosition _pos, const int _val[], EFeeddbackMode _mod)
	{
		Position = _pos;
		Mode = _mod;
		//values.assign(20, 0);
		Values.AddZeroed(20);
		for (int i = 0; i < 20; i++)
		{
			Values[i] = _val[i];
		}
	}

	FHapticFeedback(EPosition _pos, const TArray<uint8> &_val, EFeeddbackMode _mod)
	{
		Position = _pos;
		Mode = _mod;
		Values.AddZeroed(20);
		for (int i = 0; i < 20; i++)
		{
			Values[i] = _val[i];
		}
	}

	FHapticFeedback() : Position(), Mode()
	{
		Values.AddZeroed(20);
	}
};

class HAPTICSMANAGER_API HapticStructures
{
public:
	HapticStructures();
	~HapticStructures();
};