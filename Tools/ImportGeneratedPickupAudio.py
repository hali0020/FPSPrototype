import os

import unreal


project_dir = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
source_directory = os.path.join(project_dir, "SourceAssets", "Audio")
destination_path = "/Game/Pickups/Audio/Generated"
asset_names = (
    "SFX_Pickup_Ammo_01",
    "SFX_Pickup_Health_01",
    "SFX_Pickup_Supply_01",
)

tasks = []
for asset_name in asset_names:
    source_wav = os.path.join(source_directory, asset_name + ".wav")
    if not os.path.isfile(source_wav):
        raise RuntimeError(f"Generated pickup WAV was not found: {source_wav}")

    task = unreal.AssetImportTask()
    task.filename = source_wav
    task.destination_path = destination_path
    task.destination_name = asset_name
    task.automated = True
    task.replace_existing = True
    task.replace_existing_settings = True
    task.save = True
    tasks.append(task)

unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)

for asset_name in asset_names:
    object_path = f"{destination_path}/{asset_name}.{asset_name}"
    sound_wave = unreal.load_asset(object_path)
    if sound_wave is None:
        raise RuntimeError("Unreal did not create the expected SoundWave: " + object_path)
    unreal.EditorAssetLibrary.save_loaded_asset(sound_wave, only_if_is_dirty=False)
    unreal.log("Imported generated pickup audio: " + object_path)
