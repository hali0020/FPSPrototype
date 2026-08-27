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

The three pickup sounds are generated together by
`Tools/GeneratePickupSounds.ps1` and imported by
`Tools/ImportGeneratedPickupAudio.py`. Each sound has its own fixed random seed
and synthesis design, so pickup feedback does not depend on the old `Pop_05`
asset or on downloaded samples:

- `SFX_Pickup_Ammo_01.wav`: two metallic latch impacts plus a short bolt-slide
  texture; 0.30 seconds; SHA-256
  `90569CDB2A5E90A43391A25A60556E2D24EABF11DB2CD9B4265A864214959142`
- `SFX_Pickup_Health_01.wav`: soft three-note confirmation with a gentle airy
  layer; 0.42 seconds; SHA-256
  `299AFBD87691BB686EF1192C854761D947E24F66C24BBFF65140385E15E4A5BD`
- `SFX_Pickup_Supply_01.wav`: fuller mechanical latch, low body and rising
  three-note chord; 0.50 seconds; SHA-256
  `31DB92981993579140C17E23D1D9D23501ECFDD6F9EE7DC83CC27E6D67CA966D`

All three are 48 kHz, mono, 16-bit PCM. Their Unreal destination is
`/Game/Pickups/Audio/Generated`. Re-running the generator produces the same
byte-for-byte WAV files.
