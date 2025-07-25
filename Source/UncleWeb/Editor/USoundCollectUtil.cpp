#include "USoundCollectUtil.h"
#include "SoundCollectEditorTool.h"
#include "UncleWeb/Util/TUDefines.h"

void USoundCollectUtil::CollectAllSounds()
{
	USoundLibrary* Library = LoadObject<USoundLibrary>(nullptr, TEXT("/Script/UncleWeb.SoundLibrary'/Game/Data/DA_SoundLibrary.DA_SoundLibrary'"));
	if (!Library)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] SoundLibrary not found!"), CURRENT_CONTEXT);
		return;
	}

	FSoundCollectEditorTool::CollectSoundsToLibrary(Library, "/Game/Sounds/BGM", true);
	FSoundCollectEditorTool::CollectSoundsToLibrary(Library, "/Game/Sounds/SFX", false);

	UE_LOG(LogTemp, Log, TEXT("[%s] Collected sounds into SoundLibrary!"), CURRENT_CONTEXT);
}
