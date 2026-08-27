import os
import unreal


project_dir = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
source_wav = os.path.join(
    project_dir, "SourceAssets", "Audio", "SFX_HitConfirm_01.wav"
)
destination_path = "/Game/Weapons/Rifle/Audio/Generated"
expected_object_path = (
    destination_path + "/SFX_HitConfirm_01.SFX_HitConfirm_01"
)

if not os.path.isfile(source_wav):
    raise RuntimeError(f"Generated hit-confirm WAV was not found: {source_wav}")

task = unreal.AssetImportTask()
task.filename = source_wav
task.destination_path = destination_path
task.destination_name = "SFX_HitConfirm_01"
task.automated = True
task.replace_existing = True
task.replace_existing_settings = True
task.save = True

unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
sound_wave = unreal.load_asset(expected_object_path)
if sound_wave is None:
    raise RuntimeError(
        "Unreal did not create the expected SoundWave: " + expected_object_path
    )

unreal.EditorAssetLibrary.save_loaded_asset(sound_wave, only_if_is_dirty=False)
unreal.log(
    "Imported generated hit-confirm report: "
    + source_wav
    + " -> "
    + expected_object_path
)
