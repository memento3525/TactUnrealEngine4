//Copyright bHaptics Inc. 2017

#include "HapticsManagerPrivatePCH.h"
#include "HapticsManagerActor.h"
#include "AllowWindowsPlatformTypes.h"
#include "SDK/hapticsManager.hpp"
#include "HapticStructures.h"
#include "HideWindowsPlatformTypes.h"
#include "BhapticsUtilities.h"

bhaptics::HapticPlayer *bhaptics::HapticPlayer::hapticManager = 0;

TArray<FHapticFeedback> AHapticsManagerActor::ChangedFeedbacks = {};
FCriticalSection AHapticsManagerActor::m_Mutex;

// Sets default values
AHapticsManagerActor::AHapticsManagerActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//bhaptics::HapticPlayer::instance()->destroy();

}

void AHapticsManagerActor::OnConstruction(const FTransform & Transform)
{
	Super::OnConstruction(Transform);

	if (!HasAnyFlags(RF_Transient))
	{
		TArray<FString> HapticFiles;
		FString FileDirectory = LoadFeedbackFiles(HapticFiles);
		for (int i = 0; i < HapticFiles.Num(); i++)
		{
			FString FileName = HapticFiles[i];
			FString FilePath = FileDirectory;
			int32 index;
			FileName.FindChar(TCHAR('.'), index);
			FilePath.Append("/");
			FilePath.Append(FileName);
			FString Key = FileName.Left(index);
			//RegisterFeeback(Key, FilePath);
			HapticFileNames.AddUnique(*Key);
		}

		HapticFileRootFolder = FileDirectory;
	}
}

// Called when the game starts or when spawned
void AHapticsManagerActor::BeginPlay()
{
	Super::BeginPlay();

	BhapticsUtilities::Initialise();
	FString temp = BhapticsUtilities::GetExecutablePath();

	UE_LOG(LogTemp, Log, TEXT("%s"),*temp);
	UE_LOG(LogTemp, Log, TEXT("Passed the executable path"));
	
	if (!BhapticsUtilities::IsPlayerRunning())
	{
		UE_LOG(LogTemp, Log, TEXT("Player is not running"));

		if (BhapticsUtilities::IsPlayerInstalled())
		{
			UE_LOG(LogTemp, Log, TEXT("Player is installed - launching"));
			BhapticsUtilities::LaunchPlayer();
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Player is not Installed"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Player is running"));
	}

	BhapticsUtilities::Free();

	bhaptics::HapticPlayer::instance()->dispatchFunctionVar = UpdateDisplayedFeedback;
	bhaptics::HapticPlayer::instance()->init();
	
	//*
	InitialiseDots(Tactal);
	InitialiseDots(TactosyLeft);
	InitialiseDots(TactosyRight);
	InitialiseDots(TactotBack);
	InitialiseDots(TactotFront);
	InitialiseDots(TactGloveLeft);
	InitialiseDots(TactGloveRight);
	InitialiseDots(TactShoeLeft);
	InitialiseDots(TactShoeRight);
	InitialiseDots(TactRacket);
	//*/

	TArray<FString> HapticFiles;
	FString FileDirectory = LoadFeedbackFiles(HapticFiles);
	for (int i = 0; i < HapticFiles.Num(); i++)
	{
		FString FileName = HapticFiles[i];
		FString FilePath = FileDirectory;
		int32 index;
		FileName.FindChar(TCHAR('.'), index);
		FilePath.Append("/");
		FilePath.Append(FileName);
		FString Key = FileName.Left(index);
		RegisterFeeback(Key, FilePath);
		HapticFileNames.AddUnique(*Key);
	}

	HapticFileRootFolder = FileDirectory;
}

void AHapticsManagerActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	bhaptics::HapticPlayer::instance()->destroy();
}

// Called every frame
void AHapticsManagerActor::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
	m_Mutex.Lock();
	
	for (int i = 0; i < ChangedFeedbacks.Num();i++)
	{
		FHapticFeedback Feedback = ChangedFeedbacks[i];

		switch (Feedback.Position)
		{
		case EPosition::Right:
			VisualiseFeedback(Feedback, TactosyRight);
			break;
		case EPosition::Left:
			VisualiseFeedback(Feedback, TactosyLeft);
			break;
		case EPosition::VestFront:
			VisualiseFeedback(Feedback, TactotFront);
			break;
		case EPosition::VestBack:
			VisualiseFeedback(Feedback, TactotBack);
			break;
		case EPosition::Head:
			VisualiseFeedback(Feedback, Tactal);
			break;
		case EPosition::HandL:
			VisualiseFeedback(Feedback, TactGloveLeft);
			break;
		case EPosition::HandR:
			VisualiseFeedback(Feedback, TactGloveRight);
			break;
		case EPosition::FootL:
			VisualiseFeedback(Feedback, TactShoeLeft);
			break;
		case EPosition::FootR:
			VisualiseFeedback(Feedback, TactShoeRight);
			break;
		case EPosition::Racket:
			VisualiseFeedback(Feedback, TactRacket);
		default:
			printf("Position not found.");
			break;
		}
	}
	ChangedFeedbacks.Empty();
	//bhaptics::HapticPlayer::instance()->checkMessage();
	m_Mutex.Unlock();
}

void AHapticsManagerActor::SubmitRegistered(const FString & Key)
{
	SubmitKey(Key);
}

void AHapticsManagerActor::SubmitKey(const FString &Key)
{
    std::string StandardKey(TCHAR_TO_UTF8(*Key));
	bhaptics::HapticPlayer::instance()->submitRegistered(StandardKey);
}

void AHapticsManagerActor::SubmitRegisteredIntesityDuration(const FString & Key, float Intensity, float Duration)
{
	SubmitKeyWithIntensityDuration(Key, Intensity, Duration);
}

void AHapticsManagerActor::SubmitKeyWithIntensityDuration(const FString &Key, float Intensity, float Duration)
{
	std::string StandardKey(TCHAR_TO_UTF8(*Key));
	bhaptics::HapticPlayer::instance()->submitRegistered(StandardKey, Intensity, Duration);
}

void AHapticsManagerActor::SubmitKeyWithTransform(const FString &Key, float DeltaX, float DeltaY, bool IsValueRotated)
{
	std::string StandardKey(TCHAR_TO_UTF8(*Key));
	bhaptics::HapticPlayer::instance()->submitRegistered(StandardKey, DeltaX, DeltaY, IsValueRotated);
}

void AHapticsManagerActor::RegisterFeeback(const FString &Key, const FString &FilePath)
{
    std::string stdKey(TCHAR_TO_UTF8(*Key));

    FString newPath = FilePath;
    std::string StandardPath(TCHAR_TO_UTF8(*newPath));

    if (!FPaths::FileExists(newPath))
    {
        UE_LOG(LogTemp, Log, TEXT("File does not exist : %s"), *newPath);
        return;
    }

	bhaptics::HapticPlayer::instance()->registerFeedback(stdKey, StandardPath);
}

FString AHapticsManagerActor::LoadFeedbackFiles(TArray<FString>& FilesOut)
{
	FString RootFolderFullPath = FPaths::GameContentDir() + ProjectFeedbackFolder;
	if (!FPaths::DirectoryExists(RootFolderFullPath) || !UseProjectFeedbackFolder)
	{
		//RootFolderFullPath = FPaths::ConvertRelativePathToFull(FPaths::EnginePluginsDir()) + "Marketplace/HapticsManager/Feedback";
		RootFolderFullPath = IPluginManager::Get().FindPlugin("HapticsManager")->GetBaseDir()+ "/Feedback";
	}
	UE_LOG(LogTemp, Log, TEXT("Looking in dir : %s"), *RootFolderFullPath);

	FPaths::NormalizeDirectoryName(RootFolderFullPath);
	IFileManager& FileManager = IFileManager::Get();

	FString Ext = "*.tactosy";
	FString Ext2 = "*.tact";

	FString FinalPath = RootFolderFullPath + "/" + Ext;
	FString FinalPath2 = RootFolderFullPath + "/" + Ext2;
	FileManager.FindFiles(FilesOut, *FinalPath, true, false);
	FileManager.FindFiles(FilesOut, *FinalPath2, true, false);
	return RootFolderFullPath;
}

void AHapticsManagerActor::SubmitBytes(const FString &Key, EPosition PositionEnum, const TArray<uint8>& InputBytes, int32 DurationInMilliSecs)
{
	bhaptics::Position HapticPosition = bhaptics::Position::All;
	std::string StandardKey(TCHAR_TO_UTF8(*Key));

	switch (PositionEnum)
	{
	case EPosition::Left: 
		HapticPosition = bhaptics::Position::Left;
		break;
	case EPosition::Right:
		HapticPosition = bhaptics::Position::Right;
		break;
	case EPosition::Head:
		HapticPosition = bhaptics::Position::Head;
		break;
	case EPosition::VestFront:
		HapticPosition = bhaptics::Position::VestFront;
		break;
	case EPosition::VestBack:
		HapticPosition = bhaptics::Position::VestBack;
		break;
	case EPosition::HandL:
		HapticPosition = bhaptics::Position::HandL;
		break;
	case EPosition::HandR:
		HapticPosition = bhaptics::Position::HandR;
		break;
	case EPosition::FootL:
		HapticPosition = bhaptics::Position::FootL;
		break;
	case EPosition::FootR:
		HapticPosition = bhaptics::Position::FootR;
		break;
	case EPosition::Racket:
		HapticPosition = bhaptics::Position::Racket;
		break;
	default:
		//return;
		break;
	}
	
	if (InputBytes.Num() != 20)
	{
		printf("Invalid Point Array\n");
		return;
	}

	std::vector<uint8_t> SubmittedDots(20, 0);

	for (int32 i = 0; i < InputBytes.Num(); i++)
	{
		SubmittedDots[i] = InputBytes[i];
	}

	bhaptics::HapticPlayer::instance()->submit(StandardKey, HapticPosition, SubmittedDots, DurationInMilliSecs);
}

void AHapticsManagerActor::SubmitDots(const FString &Key, EPosition PositionEnum, const TArray<FDotPoint> DotPoints, int32 DurationInMilliSecs)
{
	bhaptics::Position HapticPosition = bhaptics::Position::All;
	std::string StandardKey(TCHAR_TO_UTF8(*Key));
	switch (PositionEnum)
	{
	case EPosition::Left:
		HapticPosition = bhaptics::Position::Left;
		break;
	case EPosition::Right:
		HapticPosition = bhaptics::Position::Right;
		break;
	case EPosition::Head:
		HapticPosition = bhaptics::Position::Head;
		break;
	case EPosition::VestFront:
		HapticPosition = bhaptics::Position::VestFront;
		break;
	case EPosition::VestBack:
		HapticPosition = bhaptics::Position::VestBack;
		break;
	case EPosition::HandL:
		HapticPosition = bhaptics::Position::HandL;
		break;
	case EPosition::HandR:
		HapticPosition = bhaptics::Position::HandR;
		break;
	case EPosition::FootL:
		HapticPosition = bhaptics::Position::FootL;
		break;
	case EPosition::FootR:
		HapticPosition = bhaptics::Position::FootR;
		break;
	case EPosition::Racket:
		HapticPosition = bhaptics::Position::Racket;
		break;
	default:
		//return;
		break;
	}

	std::vector<bhaptics::DotPoint> SubmittedDots;

	for (int32 i = 0; i < DotPoints.Num(); i++)
	{
		SubmittedDots.push_back(bhaptics::DotPoint(DotPoints[i].Index,DotPoints[i].Intensity));
	}

	bhaptics::HapticPlayer::instance()->submit(StandardKey, HapticPosition, SubmittedDots, DurationInMilliSecs);
}

void AHapticsManagerActor::SubmitPath(const FString &Key, EPosition PositionEnum, const TArray<FPathPoint>PathPoints, int32 DurationInMilliSecs)
{
	bhaptics::Position HapticPosition = bhaptics::Position::All;
	std::vector<bhaptics::PathPoint> PathVector;
	std::string StandardKey(TCHAR_TO_UTF8(*Key));

	for (int32 i = 0; i < PathPoints.Num(); i++)
	{
		bhaptics::PathPoint Point(PathPoints[i].X, PathPoints[i].Y, PathPoints[i].Intensity, PathPoints[i].MotorCount);
		PathVector.push_back(Point);
	}

	switch (PositionEnum)
	{
	case EPosition::Left:
		HapticPosition = bhaptics::Position::Left;
		break;
	case EPosition::Right:
		HapticPosition = bhaptics::Position::Right;
		break;
	case EPosition::Head:
		HapticPosition = bhaptics::Position::Head;
		break;
	case EPosition::VestFront:
		HapticPosition = bhaptics::Position::VestFront;
		break;
	case EPosition::VestBack:
		HapticPosition = bhaptics::Position::VestBack;
		break;
	case EPosition::HandL:
		HapticPosition = bhaptics::Position::HandL;
		break;
	case EPosition::HandR:
		HapticPosition = bhaptics::Position::HandR;
		break;
	case EPosition::FootL:
		HapticPosition = bhaptics::Position::FootL;
		break;
	case EPosition::FootR:
		HapticPosition = bhaptics::Position::FootR;
		break;
	case EPosition::Racket:
		HapticPosition = bhaptics::Position::Racket;
		break;
	default:
		//return;
		break;
	}
	
	bhaptics::HapticPlayer::instance()->submit(StandardKey, HapticPosition, PathVector, DurationInMilliSecs);
}

bool AHapticsManagerActor::IsAnythingPlaying()
{
	return bhaptics::HapticPlayer::instance()->isPlaying();
}

bool AHapticsManagerActor::IsRegisteredPlaying(const FString &Key)
{
	std::string StandardKey(TCHAR_TO_UTF8(*Key));
	return bhaptics::HapticPlayer::instance()->isPlaying(StandardKey);
}

void AHapticsManagerActor::TurnOffAllFeedback()
{
	bhaptics::HapticPlayer::instance()->turnOff();
}

void AHapticsManagerActor::TurnOffRegisteredFeedback(const FString &Key)
{
	std::string StandardKey(TCHAR_TO_UTF8(*Key));
	bhaptics::HapticPlayer::instance()->turnOff(StandardKey);
}

void AHapticsManagerActor::UpdateDisplayedFeedback(const char *ReceivedMessage)
{
	//*
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	FString JsonString(ReceivedMessage);
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	
	if (FJsonSerializer::Deserialize(Reader, JsonObject))
	{
		std::map<std::string, std::vector<int>> DeviceMotors = bhaptics::HapticPlayer::instance()->parseResponse(*JsonObject);
		
		for (auto& Device : DeviceMotors)
		{
			TArray<uint8_t> ValuesArray;
			EPosition Position = EPosition::Left;

			if (Device.first == "Left")
			{
				Position = EPosition::Left;
			}
			else if (Device.first == "Right")
			{
				Position = EPosition::Right;
			}
			else if (Device.first == "Head")
			{
				Position = EPosition::Head;
			}
			else if (Device.first == "VestFront")
			{
				Position = EPosition::VestFront;
			}
			else if (Device.first == "VestBack")
			{
				Position = EPosition::VestBack;
			}
			else if (Device.first == "Racket")
			{
				Position = EPosition::Racket;
			}
			else if (Device.first == "HandL")
			{
				Position = EPosition::HandL;
			}
			else if (Device.first == "HandR")
			{
				Position = EPosition::HandR;
			}
			else if (Device.first == "FootL")
			{
				Position = EPosition::FootL;
			}
			else if (Device.first == "FootR")
			{
				Position = EPosition::FootR;
			}
			else
			{
				continue;
			}

			for (size_t Index = 0; Index < Device.second.size(); Index++)
			{
				ValuesArray.Add(Device.second[Index]);
			}

			FHapticFeedback Feedback(Position,ValuesArray , EFeedbackMode::DOT_MODE);
			m_Mutex.Lock();
			ChangedFeedbacks.Add(Feedback);
			m_Mutex.Unlock();
		}
	}//*/

}

void AHapticsManagerActor::InitialiseDots(TArray<USceneComponent*> TactSuitItem)
{
	for (int i = 0; i < TactSuitItem.Num(); i++)
	{
		UStaticMeshComponent* DotMesh = Cast<UStaticMeshComponent>(TactSuitItem[i]);
		if (DotMesh == NULL)
		{
			continue;
		}

		UMaterialInstanceDynamic* DotMaterial = DotMesh->CreateAndSetMaterialInstanceDynamicFromMaterial(0,DotMesh->GetMaterial(0));
		//DotMesh->SetRelativeScale3D(FVector(0.2, 0.2, 0.2));
	}
}

void AHapticsManagerActor::VisualiseFeedback(FHapticFeedback Feedback, TArray<USceneComponent*> TactSuitItem)
{
	for (int i = 0; i < TactSuitItem.Num(); i++)
	{
		FString ComponentName = TactSuitItem[i]->GetName();
		ComponentName.RemoveAt(0, 2);
		float DotPosition = FCString::Atof(*ComponentName);
		float Scale = Feedback.Values[DotPosition] / 100.0;
		UStaticMeshComponent* DotMesh = Cast<UStaticMeshComponent>(TactSuitItem[i]);

		if (DotMesh == NULL)
		{
			continue;
		}
		
		UMaterialInstanceDynamic* DotMaterial = Cast<UMaterialInstanceDynamic>(DotMesh->GetMaterial(0));
		DotMaterial->SetVectorParameterValue("Base Color", FLinearColor(0.8 + Scale*0.2, 0.8 + Scale*0.01, 0.8 - Scale*0.8, 1.0));
		DotMesh->SetRelativeScale3D(FVector(0.3 + 0.15*(Scale*0.8), 0.3 + 0.15*(Scale*0.8), 0.5 + 0.15*(Scale*0.8)));
	}
}

void AHapticsManagerActor::SetTactoSuit(USceneComponent * SleeveLeft, USceneComponent * SleeveRight, USceneComponent * Head, USceneComponent * VestFront,
	USceneComponent * VestBack, USceneComponent * GloveLeft, USceneComponent * GloveRight, USceneComponent * ShoeLeft, USceneComponent * ShoeRight, USceneComponent * Racket)
{
	if (SleeveLeft != NULL)
	{
		SleeveLeft->GetChildrenComponents(false, TactosyLeft);
	}

	if (SleeveRight != NULL)
	{
		SleeveRight->GetChildrenComponents(false, TactosyRight);
	}

	if(Head != NULL)
	{
		Head->GetChildrenComponents(false, Tactal);
	}
	if(VestFront != NULL)
	{
		VestFront->GetChildrenComponents(false, TactotFront);
	}
	if (VestBack != NULL)
	{
		VestBack->GetChildrenComponents(false, TactotBack);
	}

	if (GloveLeft!=NULL)
	{
		GloveLeft->GetChildrenComponents(false, TactGloveLeft);
	}	

	if (GloveRight!= NULL)
	{
		GloveRight->GetChildrenComponents(false, TactGloveRight);
	}

	if (ShoeLeft != NULL)
	{
		ShoeLeft->GetChildrenComponents(false, TactShoeLeft);
	}

	if (ShoeRight != NULL)
	{
		ShoeRight->GetChildrenComponents(false, TactShoeRight);
	}
	
	if(Racket!=NULL)
	{
		Racket->GetChildrenComponents(false, TactRacket);
	}
}

void AHapticsManagerActor::Reset()
{
	FHapticFeedback BlankFeedback = FHapticFeedback();

	VisualiseFeedback(BlankFeedback, Tactal);
	VisualiseFeedback(BlankFeedback, TactosyLeft);
	VisualiseFeedback(BlankFeedback, TactosyRight);
	VisualiseFeedback(BlankFeedback, TactotFront);
	VisualiseFeedback(BlankFeedback, TactotBack);
	VisualiseFeedback(BlankFeedback, TactGloveLeft);
	VisualiseFeedback(BlankFeedback, TactGloveRight);
	VisualiseFeedback(BlankFeedback, TactShoeLeft);
	VisualiseFeedback(BlankFeedback, TactShoeRight);
	VisualiseFeedback(BlankFeedback, TactRacket);

	bhaptics::HapticPlayer::instance()->destroy();
	bhaptics::HapticPlayer::instance()->init();
}

void AHapticsManagerActor::EnableFeedback()
{
	bhaptics::HapticPlayer::instance()->enableFeedback();
}

void AHapticsManagerActor::DisableFeedback()
{
	bhaptics::HapticPlayer::instance()->disableFeedback();
}

void AHapticsManagerActor::ToggleFeedback()
{
	bhaptics::HapticPlayer::instance()->toggleFeedback();
}

bool AHapticsManagerActor::IsDeviceConnected(EPosition device)
{
	bhaptics::Position pos = bhaptics::Position::All;

	switch (device)
	{
	case EPosition::Left:
		pos = bhaptics::Position::Left;
		break;
	case EPosition::Right:
		pos = bhaptics::Position::Right;
		break;
	case EPosition::Head:
		pos = bhaptics::Position::Head;
		break;
	case EPosition::Racket:
		pos = bhaptics::Position::Racket;
		break;
	case EPosition::HandL:
		pos = bhaptics::Position::HandL;
		break;
	case EPosition::HandR:
		pos = bhaptics::Position::HandR;
		break;
	case EPosition::FootL:
		pos = bhaptics::Position::FootL;
		break;
	case EPosition::FootR:
		pos = bhaptics::Position::FootR;
		break;
	case EPosition::VestFront:
		pos = bhaptics::Position::Vest;
		break;
	case EPosition::VestBack:
		pos = bhaptics::Position::Vest;
		break;
	case EPosition::Vest:
		pos = bhaptics::Position::Vest;
		break;
	default:
		return false;
		break;
	}

	return bhaptics::HapticPlayer::instance()->isDevicePlaying(pos);
}