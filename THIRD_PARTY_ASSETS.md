# 第三方与模板资源记录

本文件用于记录项目中资源的来源，方便换电脑、添加协作者和后续发布前进行许可证复核。它不是许可证文本，也不替代 Fab/Epic 账户中的购买记录与适用条款。

## 当前已纳入项目

### Unreal Engine 模板/示例内容

- 来源：Unreal Engine 5.8 模板与引擎示例内容
- 项目目录：`Content/Characters`、`Content/DemoTemplate`、`Content/FirstPerson`、`Content/Input`、`Content/LevelPrototyping`、`Content/Variant_Combat`、`Content/Variant_Shooter`、`Content/Weapons`
- 用途：角色、动画、当前武器主体步枪、输入、关卡原型与示例效果
- 注意：作为 Unreal Engine 项目组成部分使用，不单独重新分发源资源。

### Human Vocalizations

- 发布者：Gamemaster Audio
- Fab 页面：https://www.fab.com/listings/98259abf-477f-4015-8abe-2c9f62eaefdb
- 项目目录：`Content/HumanVocalizations`
- 当前仅纳入 14 个项目实际使用的 SoundWave 资产
- 用途：玩家和敌人的受击、攻击、警戒与死亡短叫声
- 注意：保留同一 Epic/Fab 账号中的获取记录；不单独分发声音源资产。

### Interface & Item Sounds Pack

- 发布者：Daydream Sound
- Fab 页面：https://www.fab.com/listings/78e31bcc-adfc-4816-8e10-609320deeeb1
- 项目目录：`Content/InterfaceAndItemSounds`
- 当前仓库保留 3 个 SoundWave 资产；拾取代码已不再引用其中的 `Pop_05`
- 当前用途：空仓与换弹反馈
- 注意：保留同一 Epic/Fab 账号中的获取记录；不单独分发声音源资产。

## 项目自产内容（非第三方采样）

### 程序化步枪开火音效

- 生成脚本：`Tools/GenerateRifleShot.ps1`
- UE 自动导入脚本：`Tools/ImportGeneratedRifleAudio.py`
- 可复现源文件：`SourceAssets/Audio/SFX_Rifle_Shot_01.wav`
- UE SoundWave：`Content/Weapons/Rifle/Audio/Generated/SFX_Rifle_Shot_01.uasset`
- UE 对象路径：`/Game/Weapons/Rifle/Audio/Generated/SFX_Rifle_Shot_01.SFX_Rifle_Shot_01`
- WAV SHA-256：`A65249175815B674809C03B1C1249CD9EEB28867D9AA23FAC89D42E29D32D611`
- 生成方式：使用固定随机种子与数学波形进行确定性离线合成，不含下载音频、实录素材或第三方音频采样。
- 运行方式：自动射击使用 8 声部音频池，让相邻枪声重叠播放，避免每发重启同一个 AudioComponent 而截断尾音。

### 程序化金属命中确认音效

- 生成脚本：`Tools/GenerateHitConfirm.ps1`
- UE 自动导入脚本：`Tools/ImportGeneratedHitConfirmAudio.py`
- 可复现源文件：`SourceAssets/Audio/SFX_HitConfirm_01.wav`
- UE SoundWave：`Content/Weapons/Rifle/Audio/Generated/SFX_HitConfirm_01.uasset`
- UE 对象路径：`/Game/Weapons/Rifle/Audio/Generated/SFX_HitConfirm_01.SFX_HitConfirm_01`
- 格式：48 kHz、单声道、16-bit PCM、0.20 秒
- WAV SHA-256：`FFCD1AB65AC3BCB2CFB58FE3E812C795D398EC55450E15248216C42E9AAB7AA6`
- 生成方式：使用固定随机种子、合成噪声和非谐波振荡器进行确定性离线合成，不含下载音频、实录素材或第三方采样。
- 运行方式：普通命中和击杀复用该 SoundWave，并通过不同音量与音高区分反馈。

### 程序化分类拾取音效

- 生成脚本：`Tools/GeneratePickupSounds.ps1`
- UE 自动导入脚本：`Tools/ImportGeneratedPickupAudio.py`
- 可复现源文件：`SourceAssets/Audio/SFX_Pickup_Ammo_01.wav`、`SFX_Pickup_Health_01.wav`、`SFX_Pickup_Supply_01.wav`
- UE 目录：`Content/Pickups/Audio/Generated`
- UE 对象路径：`/Game/Pickups/Audio/Generated/SFX_Pickup_Ammo_01.SFX_Pickup_Ammo_01`、`SFX_Pickup_Health_01.SFX_Pickup_Health_01`、`SFX_Pickup_Supply_01.SFX_Pickup_Supply_01`
- 格式：48 kHz、单声道、16-bit PCM；时长分别为 0.30、0.42 和 0.50 秒。
- WAV SHA-256：弹药 `90569CDB2A5E90A43391A25A60556E2D24EABF11DB2CD9B4265A864214959142`；医疗 `299AFBD87691BB686EF1192C854761D947E24F66C24BBFF65140385E15E4A5BD`；综合补给 `31DB92981993579140C17E23D1D9D23501ECFDD6F9EE7DC83CC27E6D67CA966D`。
- 生成方式：三类声音使用独立固定随机种子、合成噪声和数学振荡器离线生成，不含下载音频、实录素材或第三方采样。
- 运行方式：`FPSPickup` 按 `EFPSPickupType` 分别选择弹药金属/机械音、医疗柔和确认音和更饱满的综合补给音；不再使用 `Pop_05`。

### 程序化全息瞄具外框

- 用途：约 1.5 倍 ADS 的枪上瞄具外观与瞄准参照。
- 来源：由项目代码和引擎基础图元程序化构成，没有导入第三方瞄准镜模型。
- 边界：全息外框是项目自产部分，外框下方的枪械主体仍是 Epic/UE 模板步枪。

## 已获取并通过隔离审计、尚未纳入当前仓库

### FPS Weapon Bundle

- 发布者：Deadghost Interactive
- Fab 页面：https://www.fab.com/listings/8aeb9c48-b404-4dcd-9e56-1d0ecedba7f5
- 获取与许可：登录后的官方 Fab 页面显示 `Saved in My Library` 和 `License terms: UE Marketplace`，并链接 Epic Content License Agreement。
- 2026-08-27 技术审计：只保留武器目录的 218 个资产，在 UE 5.8.2 隔离重存后完成第二次 Cook，结果为 `218/218`、`0 errors`、`0 warnings`；外部 `/Game` 依赖和缺失依赖均为 0。
- 必须排除：568 字节空包 `SM_KA_Val_Mag_Loaded_X.uasset`、展示地图、无关的 `IndustryPropsPack6` 和 `InterfaceAndItemSounds`。
- 内容边界：包内提供武器/附件/弹药模型、材质、纹理、Skeleton 和 PhysicsAsset；不包含开火、换弹或持枪动画，也不包含枪声、拾取声、粒子特效或玩法逻辑。
- 当前使用情况：正式项目尚未迁入该包；现有枪械主体仍来自 Epic/UE 模板，程序化全息外框也不属于该包。
- 分发政策：许可允许项目内使用、修改并作为游戏不可分割的一部分打包发布；不得单独分发源资源。为降低风险，即使本仓库是 Private，也禁止通过 Git 或 Git LFS 上传原始或重存后的 Marketplace `.uasset`。另一台电脑必须使用拥有许可的同一 Epic 账号重新下载。
- 完整审计：`Docs/AssetAudits/FPS_WEAPON_BUNDLE_2026-08-27.md`。审计没有发现风险迹象，但不宣称绝对“零风险”。

## 发布前检查

- 仓库保持 Private，只邀请实际开发协作者。
- 不通过公开 Release、Actions Artifact 或公开 LFS 链接分发源资源。
- 若准备公开源码或发布可下载工程，逐项复核当时有效的 Epic/Fab 条款。
- 发布游戏打包版本前保留资产页面、获取日期与订单/库记录截图。
- 对项目自产枪声和命中确认音分别保留生成脚本、固定参数、源 WAV 与 SHA-256；重新生成或导入后复核哈希、SoundWave 引用和实际 Cook 结果。
