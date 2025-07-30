#include "SoundCollectEditorTool.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UncleWeb/Util/TUDefines.h"

void FSoundCollectEditorTool::CollectSoundsToLibrary(USoundLibrary* Library, const FString& FolderPath, bool bAsBGM)
{
	if (!Library) return;

	if (bAsBGM)
	{
		Library->BGMMap.Reset();
	}
	else
	{
		Library->SFXMap.Reset();
	}

	FString NormalizedPath = FolderPath;
	if (!NormalizedPath.StartsWith("/Game"))
	{
		NormalizedPath = "/Game/" + NormalizedPath;
	}

	FARFilter Filter;
	Filter.PackagePaths.Add(*NormalizedPath);
	Filter.ClassPaths.Add(USoundWave::StaticClass()->GetClassPathName());
	Filter.bRecursivePaths = true;

	TArray<FAssetData> AssetList;
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	AssetRegistryModule.Get().GetAssets(Filter, AssetList);

	int32 Count = 0;
	for (const FAssetData& Asset : AssetList)
	{
		TObjectPtr<USoundWave> Sound = Cast<USoundWave>(Asset.GetAsset());
		if (!Sound) continue;

		const FName Key = Asset.AssetName;

		if (bAsBGM)
		{
			Library->BGMMap.FindOrAdd(Key) = Sound;
		}
		else
		{
			Library->SFXMap.FindOrAdd(Key) = Sound;
		}

		Count++;
	}

	Library->MarkPackageDirty();

	UE_LOG(LogTemp, Log, TEXT("[%s] Collected %d sounds from '%s' into %s"), CURRENT_CONTEXT, Count, *NormalizedPath, bAsBGM ? TEXT("BGMMap") : TEXT("SFXMap"));
}
