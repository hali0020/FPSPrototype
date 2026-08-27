# Generated gameplay audio

`SFX_Rifle_Shot_01.wav` is generated locally by `Tools/GenerateRifleShot.ps1`.
It uses deterministic mathematical synthesis only and contains no downloaded,
sampled, or third-party recording. Run the script again whenever the source WAV
needs to be reproduced before importing it into Unreal Engine.

`SFX_HitConfirm_01.wav` is generated locally by
`Tools/GenerateHitConfirm.ps1` and imported by
`Tools/ImportGeneratedHitConfirmAudio.py`. It is a short, deterministic metallic
hit-confirm tick built only from synthesized noise and inharmonic oscillators. It
contains no downloaded, recorded, or third-party audio sample.

- Format: 48 kHz, mono, 16-bit PCM, 0.20 seconds
- SHA-256: `FFCD1AB65AC3BCB2CFB58FE3E812C795D398EC55450E15248216C42E9AAB7AA6`
- Unreal destination: `/Game/Weapons/Rifle/Audio/Generated/SFX_HitConfirm_01`
