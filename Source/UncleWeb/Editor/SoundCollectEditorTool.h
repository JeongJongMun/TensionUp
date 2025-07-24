// SoundManagerEditorUtil.h
#pragma once

#include "CoreMinimal.h"
#include "UncleWeb/Data/SoundLibrary.h"

class FSoundCollectEditorTool
{
public:
	static void CollectSoundsToLibrary(USoundLibrary* Library, const FString& FolderPath, bool bAsBGM);
};
