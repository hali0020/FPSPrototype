# FPSPrototype 开发与同步指南

这份文档是配置新环境、继续开发、迁移资源、解决 Git 冲突和打包前的统一检查清单。开始开发前至少阅读“必须遵守的红线”“日常同步流程”和“UE 更新后如何刷新”。

## 一、必须遵守的红线

1. 公开仓库只能包含允许再分发的源码和资产；所有受限 Fab/Marketplace 源资源必须受 `.gitignore` 保护，并从公开 Git 历史中排除。
2. 开始工作前先执行 git pull --ff-only 和 git lfs pull；切换电脑前必须提交并推送。
3. 不要在两台电脑上同时修改同一个 .umap、.uasset、蓝图、动画或材质。这些二进制资产无法像 C++ 文本一样自动合并。
4. 不要强制提交 Binaries、DerivedDataCache、Intermediate、Saved、打包目录或 Build/*/FileOpenOrder。
5. 不要使用 git add -f 绕过忽略规则，也不要在没有备份时执行 git reset --hard、git clean -fdx 或强制推送。
6. 不要把密码、令牌、API Key、证书、Fab/Vault 缓存、绝对路径、账号信息或本地日志放进仓库。
7. 当前 Windows 渲染目标是 DX12 + Shader Model 6。Nanite 资源依赖 SM6，不要改回 SM5。
8. 正式命中特效是原生 C++ FPSImpactEffect。不要直接重新接入 /Game/Variant_Combat/VFX/NS_Damage；它曾在 Cook 后的真实运行包中触发 Niagara 序列化崩溃。

## 二、项目固定信息

- 默认分支：main
- Unreal Engine：5.8.2
- 项目文件：FPSPrototype.uproject
- C++ 模块名：FPSGame
- 默认地图：/Game/FirstPerson/Lvl_FirstPerson
- 默认 GameMode：/Script/FPSGame.FPSGameMode
- Windows 渲染：DX12、SM6

项目名与 C++ 模块名不同是正常的。不要只重命名文件夹或 .uproject 就尝试修改模块名；完整改名还涉及 Target、Build.cs、模块入口、重定向和资产引用。

`FPSPrototype.uproject` 不保存机器专属的引擎关联 GUID。新环境第一次打开时，请右键项目文件，选择“Switch Unreal Engine version/切换 Unreal Engine 版本”，指定 UE 5.8.2，然后重新生成 Visual Studio 项目文件。当前 Target 使用 UE 5.8 的构建设置，不要默认用 5.6/5.7 编译。

如果切换引擎后工具自动写入机器专属 `EngineAssociation`，提交前执行 `git diff -- FPSPrototype.uproject` 并移除该关联，避免把本地注册信息推回仓库。

## 三、新环境首次准备

### 必装软件

- Unreal Engine 5.8.2
- Visual Studio 2022
  - 使用 C++ 的游戏开发
  - Windows 10/11 SDK
  - Unreal Engine 工具
- Git
- Git LFS

### 正确克隆方式

    git lfs install
    git clone <repository-url>
    cd FPSPrototype
    git lfs pull
    git status

不要使用 GitHub 的“Download ZIP”代替克隆。ZIP 可能只包含 Git LFS 指针，导致 .uasset 看似存在但无法被 UE 读取。

克隆完成后：

1. 确认 git status 没有异常修改。
2. 关联 UE 5.8.2。
3. 右键 FPSPrototype.uproject 生成 Visual Studio 项目文件。
4. 打开解决方案，选择 Development Editor、Win64 编译；或直接打开项目并同意重建模块。
5. 首次启动会重新生成缓存和二进制文件，耗时较长属于正常现象。

## 四、每天开始和结束时的同步流程

### 开始工作前

先关闭 Unreal Editor，尤其是更新包含地图、蓝图、模型、动画或材质时。然后执行：

    git status
    git pull --ff-only
    git lfs pull
    git status

如果第一次 git status 显示未提交修改，先处理或提交自己的修改，不要直接拉取并覆盖。--ff-only 如果报告分支已经分叉，应停止并判断两台电脑各自的提交，不要盲目 rebase 或 merge 二进制资产。

### 完成工作后

在 UE 中执行 Save All，确认没有仍带星号的未保存资产，然后：

    git status
    git diff -- Source Config README.md DEVELOPMENT_GUIDE.md THIRD_PARTY_ASSETS.md
    git add .
    git status
    git commit -m "清楚描述本次修改"
    git push

git diff 主要用于检查文本；.uasset 和 .umap 只能看到文件发生变化，无法查看内部差异。推送结束后再运行一次 git status，应显示分支与 origin/main 同步且工作区干净。

### 大功能建议使用分支

    git switch -c feature/weapon-switching

功能验证后再合并回 main。不要对已经共享的 main 使用强制推送。

## 五、UE 更新后如何刷新

### 只更新了 C++ 的 .cpp

- 小型函数实现可尝试 Live Coding。
- 最稳妥方式是关闭编辑器，编译 Development Editor，再打开项目。

### 更新了 .h、UCLASS、UPROPERTY、构造函数或新增源码文件

1. 关闭 Unreal Editor。
2. 重新生成 Visual Studio 项目文件；新增或删除 .h/.cpp 时尤其重要。
3. 完整编译 Development Editor。
4. 重新打开项目。

不要依赖 Live Coding 处理反射结构、类布局或构造阶段的重大变化，否则可能出现热重载残留和对象状态不一致。

### 更新了 .uasset、.umap、动画、声音或材质

先执行：

    git lfs pull

如果编辑器在拉取时已经打开，最安全的是关闭后重开。不要让 UE 用内存中的旧资产再次保存并覆盖刚拉取的新版本。

### 更新了配置文件

关闭并重开编辑器，让 Config/*.ini 完整重新加载。渲染、默认地图、输入和 GameMode 修改尤其需要重启。

### 新增、移动或重命名资产

- 在 UE Content Browser 内移动/重命名，不要直接使用资源管理器移动 .uasset。
- 保存全部引用资产并执行 Fix Up Redirectors。
- 检查 C++ 中通过路径硬加载的资源；移动资源后必须同步修改路径。
- 提交重定向、地图、__ExternalActors__ 和 __ExternalObjects__ 的关联变化。

### 同时更新 C++ 类和声音资产时

本轮这类更新同时包含新的 `UCLASS` 源文件和新的 `.uasset`，不能只依赖 Live Coding：

1. 在 UE 中执行 Save All，然后关闭编辑器。
2. 执行 `git pull --ff-only` 和 `git lfs pull`。
3. 重新生成 Visual Studio 项目文件；新增 `FPSDeathEffect.h/.cpp` 或 `FPSEnemySpawnEffect.h/.cpp` 时必须执行这一步。
4. 编译 `Development Editor / Win64`。
5. 重新打开项目，确认命中音和三类拾取 SoundWave 均可加载，并运行 Data Validation。
6. 需要验证发布包时，重新执行 Cook + Stage，并启动这一次生成的 EXE。

仓库已经跟踪导入后的 SoundWave；正常克隆或拉取后不需要重新运行音频导入脚本。只有主动修改或重新生成源 WAV 时，才需要重新导入并保存资产。

## 六、地图与二进制资产冲突

.umap、.uasset、动画、骨骼、材质和声音是二进制文件。发生冲突时：

- 不要用文本编辑器拼接冲突内容。
- 明确选择其中一台电脑或一个分支的版本作为基础。
- 另一方的编辑通常需要在 UE 中重新制作。
- 解决前先复制冲突资产到仓库外做临时备份。

Lvl_FirstPerson 使用外部 Actor/Object 数据。提交地图变化时必须一起检查：

- Content/FirstPerson/Lvl_FirstPerson.umap
- Content/__ExternalActors__/FirstPerson/Lvl_FirstPerson/...
- Content/__ExternalObjects__/FirstPerson/Lvl_FirstPerson/...

只提交 .umap 而漏掉外部 Actor，可能造成另一台电脑缺少场景物体。

## 七、Git LFS 注意事项

仓库已经让 UE 二进制、模型、纹理、音频和视频走 Git LFS，包括 .uasset、.umap、.fbx、常见贴图、声音和视频格式。

常见异常：

- .uasset 只有一百多字节或能看到 version https://git-lfs...：执行 git lfs install、git lfs pull。
- Push 提示大文件超过 GitHub 限制：先检查 .gitattributes，不要直接强制提交。
- LFS 上传中断：再次执行 git push；已上传对象通常会复用。
- 检查本地 LFS：git lfs fsck。
- 检查当前 LFS 状态：git lfs status。
- 如果对象已下载但工作区仍是指针：git lfs checkout。

不要随意运行 git lfs migrate 改写已经发布的历史。确需迁移时，应先备份并单独制定方案。

## 八、禁止提交和可以重新生成的目录

以下内容都不属于源码资产，仓库已通过 .gitignore 排除：

- Binaries/
- DerivedDataCache/
- Intermediate/
- Saved/
- .vs/
- Build/*/FileOpenOrder/
- Content/Developers/
- 每用户编辑器配置

不要为了跨环境直接运行而提交 `Saved/StagedBuilds` 或 EXE。新环境应从源码重新编译或打包。打包产物如需分发，应使用独立发布流程，而不是塞进 Git 历史。

> **旧 StagedBuild 警告：** `Saved/StagedBuilds/Windows` 可能仍是上一次打包结果。Editor 编译、Live Coding、资源导入和 Data Validation 都不会自动刷新其中的 EXE。验证最新修改时必须完成新的 Cook + Stage，核对输出时间，并启动本次生成的 EXE；不要用旧 Staged EXE 判断当前源码是否生效。

如果编译缓存损坏，通常可以在关闭 UE/VS 后删除 Binaries 和 Intermediate 再重建。删除 Saved 前要先检查其中是否有需要恢复的 Autosaves；不要把删除整个工程目录当成排错步骤。

## 九、资源与许可证安全

- 公开源码只保留允许再分发的内容；本地许可资源不进入任何公开提交、分支、标签或 Artifact。
- 每个开发环境都应使用拥有相应资源许可的 Epic/Fab 账号自行获取受限资源。
- 不要单独提取、转售或公开分发模型、声音、贴图、动画等源资产。
- 不要通过公开 Release、公开 Actions Artifact 或公开 LFS 链接泄露源资产。
- 新接入 Fab/Marketplace 内容时，在 THIRD_PARTY_ASSETS.md 记录名称、发布者、页面、获取状态、导入目录和用途。
- 保存 Fab 页面、获取记录和当时适用条款的截图；“免费”不等于可以单独重新分发源文件。

当前第三方声音只保留了游戏实际使用的子集。当前没有训练或克隆任何人的音色，声音是资源包内的短叫声和反馈音。

当前步枪开火声不是第三方录音，而是项目内确定性生成：

- 生成脚本：`Tools/GenerateRifleShot.ps1`
- UE 自动导入脚本：`Tools/ImportGeneratedRifleAudio.py`
- 可复现 WAV：`SourceAssets/Audio/SFX_Rifle_Shot_01.wav`
- 导入资产：`Content/Weapons/Rifle/Audio/Generated/SFX_Rifle_Shot_01.uasset`
- WAV SHA-256：`A65249175815B674809C03B1C1249CD9EEB28867D9AA23FAC89D42E29D32D611`

生成过程只使用固定随机种子和数学合成，不下载、不录制也不拼接第三方音频采样。修改生成脚本或重新生成 WAV 后，应重新计算 SHA-256、更新 `THIRD_PARTY_ASSETS.md`，并重新导入、Cook 和试听验证。

命中确认音同样是项目内确定性生成：

- 生成脚本：`Tools/GenerateHitConfirm.ps1`
- UE 自动导入脚本：`Tools/ImportGeneratedHitConfirmAudio.py`
- 可复现 WAV：`SourceAssets/Audio/SFX_HitConfirm_01.wav`
- 导入资产：`Content/Weapons/Rifle/Audio/Generated/SFX_HitConfirm_01.uasset`
- WAV SHA-256：`FFCD1AB65AC3BCB2CFB58FE3E812C795D398EC55450E15248216C42E9AAB7AA6`

该音效使用固定随机种子、合成噪声和非谐波振荡器，不含下载、录制或第三方采样。

弹药、医疗和综合补给也分别使用项目内确定性生成的音效：

- 生成脚本：`Tools/GeneratePickupSounds.ps1`
- UE 自动导入脚本：`Tools/ImportGeneratedPickupAudio.py`
- 可复现 WAV：`SourceAssets/Audio/SFX_Pickup_Ammo_01.wav`、`SFX_Pickup_Health_01.wav`、`SFX_Pickup_Supply_01.wav`
- UE 目录：`Content/Pickups/Audio/Generated`
- SHA-256：弹药 `90569CDB2A5E90A43391A25A60556E2D24EABF11DB2CD9B4265A864214959142`；医疗 `299AFBD87691BB686EF1192C854761D947E24F66C24BBFF65140385E15E4A5BD`；综合补给 `31DB92981993579140C17E23D1D9D23501ECFDD6F9EE7DC83CC27E6D67CA966D`

`FPSPickup` 必须根据 `EFPSPickupType` 选择对应 SoundWave：弹药使用金属/机械反馈，医疗使用柔和确认音，综合补给使用更饱满的机械与和弦组合。不要再将三类拾取物统一回退到 `Pop_05`。

## 十、迁移新模型、武器和音效

1. 先确认来源、评分、许可和 UE 5.8 兼容性。
2. 优先使用 Epic/UE 官方模板内容或已在自己 Fab 库中合法获取的资源。
3. 先在测试项目或功能分支中迁移，不要直接污染 main。
4. 只迁移实际需要的资产及其依赖，不要无选择地复制整个 Vault Cache。
5. 检查材质、骨骼、动画、碰撞、LOD/Nanite、声音衰减和 Cook 结果。
6. 接入前提交一个干净的 Git 节点，失败时才能安全回退。
7. 接入后更新 THIRD_PARTY_ASSETS.md，并进行 Editor 编译、资源验证和打包运行测试。

Deadghost Interactive 的 FPS Weapon Bundle 已于 2026-08-27 通过隔离审计。官方下载内容中只允许使用 `FPS_Weapon_Bundle/Weapons` 下的 218 个白名单资产；已在 UE 5.8.2 中隔离重存并完成第二次 Cook，结果为 `218/218`、`0 errors`、`0 warnings`、外部 `/Game` 依赖 0。必须排除 568 字节空包 `SM_KA_Val_Mag_Loaded_X.uasset`、展示地图、`IndustryPropsPack6` 和 `InterfaceAndItemSounds`。登录后的 Fab 页面显示 `License terms: UE Marketplace`，项目内使用和打包游戏发布通过；仓库政策禁止上传原始或重存后的 Marketplace `.uasset`。其他环境应使用拥有许可的 Epic 账号重新下载并按审计报告恢复白名单。当前正式项目尚未迁入该包，完整记录见 `Docs/AssetAudits/FPS_WEAPON_BUNDLE_2026-08-27.md`。

## 十一、当前代码职责

- FPSCharacter：玩家输入、移动、1.5 倍 ADS、射击扩散、自动射击减速、8 声部枪声音频池、换弹、第一/第三人称身体和玩家死亡。
- FPSEnemy：Quinn 快速型/Manny 重装型变体、追击、攻击、受击反馈、语音和多套死亡动作。
- FPSGameMode：波次、敌人生成、计分和掉落逻辑。
- FPSHUD：生命、弹药、按当前弹道扩散动态变化的准星、命中/击杀、波次、拾取和死亡界面。
- HealthComponent：生命、治疗、伤害和死亡状态。
- FPSPickup：弹药、医疗和综合补给。
- FPSImpactEffect：原生表面/角色命中特效。
- FPSEnemySpawnEffect：敌人生成时的青蓝光柱和扩张环提示；只使用 7 个固定基础图元组件与 2 个共享动态材质，不创建动态点光源。
- FPSDeathEffect：敌人死亡时的原生核心光爆、扩张冲击环和飞散碎片效果；固定使用基础图元组件、共享动态材质和一个点光源。
- FPSFirstPersonAnimInstance：第一人称动画状态。

很多动画、声音和模型当前通过 C++ 资源路径加载。重命名或移动 Content 资产前，先搜索 Source/FPSGame 中的 /Game/... 路径。

## 十二、项目特有风险

### 地图与重开路径硬编码

默认地图和 GameMode 写在 Config/DefaultEngine.ini 中，死亡后重开路径还写在 FPSGameMode.cpp 中。重命名 Lvl_FirstPerson 时必须同时更新 Config 和 C++。Config/DefaultEditor.ini 中仍有模板遗留的 FirstPersonBP/Maps/FirstPersonExampleMap，它不是当前正式地图。

### 输入系统是混合状态

当前角色使用 BindAxis/BindAction 的传统绑定，映射来自 Config/DefaultInput.ini；项目同时使用 EnhancedInput 的默认输入类，但 FPSGame.Build.cs 尚未加入 EnhancedInput 模块。只新建 IA/IMC 资产不会自动生效：要么继续维护传统映射，要么一次性完成 Enhanced Input 迁移、模块依赖和 Mapping Context 注入。

### 角色、武器、动画契约

当前人物和枪依赖 head、HandGrip_R、Muzzle Socket，以及第一人称 Arms 动画 Slot。替换角色、骨骼或武器时必须保留或重建这些 Socket/Slot，并正确 Retarget 动画；否则会出现身体不可见、枪错位或开火/换弹不播放。

### 当前枪械、ADS 与扩散契约

- 当前枪械主体仍是 Epic/UE 模板步枪，不是 Deadghost FPS Weapon Bundle 中的模型。
- 枪上的全息瞄具外框由项目程序化生成，没有引入第三方瞄准镜模型；当前 ADS 约为 1.5 倍。
- 腰射准星和 ADS 标记以 Canvas 中心，也就是最终渲染视角中心为稳定瞄准中心。
- `FPSCharacter::FireShot` 使用 `GetPlayerViewPoint` 得到同一视角方向，再通过当前扩散角生成实际射线。
- `WeaponAimRoot` 只平移第一人称武器，使镜窗中心保持在相机瞄准轴；它不移动玩家相机，枪械仍可保留小幅旋转摆动。
- `GetCurrentWeaponSpreadDegrees` 同时驱动实际弹道和 HUD 张合反馈；修改移动、空中、ADS 或连射扩散时必须同步验证两者。
- 自动射击期间移动速度为正常步行速度的 65%。停止射击、空仓、换弹、死亡、暂停或其他中断路径都必须恢复或重新计算正确速度，避免残留减速。

### 敌人生成与死亡特效契约

- `AFPSEnemySpawnEffect` 在敌人脚下显示约 0.72 秒的青蓝光柱和 6 段扩张环。`FPSEnemy::BeginPlay` 只生成一次该 Actor，不改变敌人的碰撞、生命或移动逻辑。生成环在设为可见前已初始化变换，避免首帧出现重叠的大方块。
- `AFPSDeathEffect` 使用 14 个飞散基础图元、核心闪光和 12 段扩张冲击环模拟约 1.05 秒的死亡光爆。`FPSEnemy::HandleDeath` 先按死亡动画与语音长度计算 `CorpseLifetime`，完整保留尸体；寿命结束前默认 0.45 秒由原生 Timer 调用一次 `TriggerDeathEffect`，先隐藏 SkeletalMesh，再生成独立的特效 Actor。特效不会随尸体 Actor 销毁而提前结束。
- `DeathEffectLeadTime` 是 `EditDefaultsOnly` 的提前量，默认值为 0.45 秒，运行时限制为 0.05–2.0 秒。死亡动画决定尸体寿命时，`CorpseLifetime` 至少包含完整动画长度和该提前量，避免模型在动作结束前隐藏。一次性守卫必须继续保留；调整时序时不得移动 `OnEnemyDied`，也不得改变得分、掉落、碰撞、波次或死亡语音逻辑。
- 两种效果的组件都在构造阶段固定创建，动态材质与运动数组只在 `BeginPlay` 初始化；`Tick` 只更新已有组件的位置、旋转、缩放，以及死亡效果的点光强度，不逐帧创建组件、材质或粒子。生成效果刻意不使用点光源，以降低整波敌人同帧生成时的峰值。它们不依赖 Niagara，也不要重新接入已知会使打包版崩溃的 `/Game/Variant_Combat/VFX/NS_Damage`。修改后必须完成 Editor 编译、Cook + Stage，并运行新生成的真实 EXE 检查显示与退出日志。

### 敌人变体与动画契约

- `FPSGameMode` 通过延迟生成明确把每组第 3 个敌人设为 Manny 重装型，其余为 Quinn 快速型；单独生成敌人且未指定时，`FPSEnemy::ConfigureVariant` 才以实例 ID 作稳定回退。两者都使用 Epic/UE Mannequin 共同骨架，不需要新增第三方模型。
- 快速型为 85 生命、330 移速、9 点近战伤害；重装型为 150 生命、245 移速、16 点近战伤害。`HealthComponent::SetMaxHealth` 会同步最大生命和当前生命，修改变体时不能只改其中一个值。
- 每个敌人从 3 套攻击、3 套受击和 5 套含前/后/左/右动作的死亡动画资源中稳定轮换，避免所有单位播放完全相同的动作。当前只是外观轮换，不会根据实际来弹方向选择动画；替换候选时必须继续使用兼容 Mannequin 骨架和 `DefaultSlot` 的资源。
- Quinn 使用现有女性警戒、攻击、受伤和死亡语音；Manny 只使用项目现有的男性受伤与死亡语音。没有来源匹配的男性警戒/攻击声音时保持静音，不用不相符的声音凑数。
- 非 Shipping 测试可显式加入 `-FPSVerifyEnemyVariants`；每个生成敌人会记录变体、网格、生命、速度与伤害，用来确认首波组成。默认运行不输出这组日志。

### 枪声音频契约

步枪声使用 `SFX_Rifle_Shot_01` 和 8 声部 AudioComponent 池。当前 WAV 约 0.82 秒，射击间隔约 0.12 秒，8 个声部可以覆盖正常连射重叠。不要恢复为“每发先 Stop 同一个 AudioComponent 再 Play”的实现，否则尾音会被截断，声音会重新变薄和失真。若以后延长音频或提高射速，应按 `ceil(音频时长 / 射击间隔) + 1` 重新核算声部数量，并进行实际连射试听。

### 伤害和碰撞契约

射击与敌人近战直接查找 HealthComponent，不是通用的 UE Damage 流程。新增可受伤敌人或机关时必须挂 HealthComponent。玩家和敌人 Capsule 需要阻挡 Visibility；随意修改碰撞 Profile 可能同时破坏子弹命中、敌人探测、近战和出生点检查。

### 当前是单机架构

波次、分数、HUD、敌人目标和死亡流程都按本地 Player 0 编写，尚未做 Authority、Replication 或客户端状态同步。增加多人功能前需要单独重构，不能只勾选 Replicates。

### 当前敌人不使用 NavMesh

敌人通过 Tick 移动和 Visibility Sweep 做简单绕障；仅摆放 NavMeshBoundsVolume 不会自动改善 AI。关卡的 WorldStatic、Visibility 和 Pawn 碰撞必须正确。若改用 AIController/NavMesh，需要重写移动与寻路逻辑。

### HUD 不是 UMG

战斗 HUD 仍使用 `AFPSHUD::DrawHUD` 的 Canvas 绘制；主菜单、暂停菜单和死亡菜单则由原生 C++ `FPSMenuWidget` 使用 UMG/Slate 构建，因此 `Build.cs` 已依赖 `UMG`、`Slate` 和 `SlateCore`。若以后改用 Widget Blueprint 或 CommonUI，需要保留现有依赖，并按所选方案补充 Widget 创建流程或 `CommonUI` 模块。

### 引擎插件

项目启用了 ModelingToolsEditorMode、Landmass 和 InEditorDocumentation。配置新环境时应优先安装一致的 UE 5.8.2 构建；不要在未检查资产依赖前随意禁用 Landmass。

### Shader Model 6

Config/DefaultEngine.ini 已启用 PCD3D_SM6。如果出现 Nanite“缺失 Shader Model 6”提示，先确认：

- UE 使用 DX12；
- 项目仍包含 +D3D12TargetedShaderFormats=PCD3D_SM6；
- 显卡驱动支持相应功能；
- 修改后重启编辑器并重新编译 Shader。

### Niagara 命中特效

旧的 NS_Damage 在编辑器验证和 Cook 阶段没有暴露问题，但在真实 Staged EXE 中曾崩溃。因此任何重新引入 Niagara 的修改都必须启动实际打包 EXE 验证，不能只依赖 PIE 或 Data Validation。

### Git safe.directory 提示

如果 Git 报 `dubious ownership`，只对当前仓库的精确路径添加 `safe.directory`。不要设置 `safe.directory=*`，也不要把本地绝对路径写入项目文件或文档。

## 十三、编译、验证与打包等级

### 日常小修改

- 编译 Development Editor。
- 进入 PIE 测试静止、移动、空中和持续连射时的扩散与准星一致性。
- 移动和开火时确认枪械可以轻微摆动，但腰射准星与 ADS 标记保持在最终视角中心，镜窗继续包住 ADS 标记。
- 检查自动射击期间速度降为正常步速的 65%，停止开火及所有中断路径后正确恢复。
- 测试 1.5 倍 ADS、程序化全息外框和 8 声部枪声连续播放，确认瞄准参照可见且尾音不被截断。
- 继续测试换弹、受伤和死亡重开。
- 分别触发普通命中和击杀，确认不再播放旧泡泡式命中反馈；确认敌人会先完整播放死亡动作并保留尸体，临近销毁时模型才隐藏、死亡光爆只生成一次且能独立播放到结束。
- 检查 Output Log 没有新增 Error。

### 资源或玩法里程碑

- 编译 Editor Target。
- 编译独立 Game Target。
- 运行 Data Validation。
- Cook + Stage Windows Development 包。
- 启动真实 FPSGame.exe，至少完成一次出生、射击、命中、击杀、拾取和死亡重开测试。

### 必须做真实打包测试的修改

- Niagara/VFX、材质或 Shader；
- ConstructorHelpers 或硬资源路径；
- GameMode、默认地图或 Cook 配置；
- 插件和 Build.cs 依赖；
- 新模型、动画、声音资源；
- 只在 Shipping/Standalone 路径触发的逻辑。

打包产物通常位于 `Saved/StagedBuilds/Windows`，该相对目录被 Git 忽略。

## 十四、常见问题速查

### 项目提示模块缺失或版本不兼容

1. 确认使用 UE 5.8.2。
2. 切换项目关联引擎。
3. 重新生成 Visual Studio 项目文件。
4. 编译 Development Editor。
5. 仍失败时，关闭 UE/VS，删除可生成的 Binaries 和 Intermediate 后重建。

### 模型、声音或动画在新环境缺失

    git lfs install
    git lfs pull
    git lfs fsck

随后重启 UE。还要确认登录的是拥有相应 Fab 许可的 Epic 账号。

### 拉取后看不到更新

- C++：关闭 UE，编译后重开。
- Content 资产：执行 git lfs pull 后重开 UE。
- Config：重启 UE。
- 新源码文件：重新生成解决方案。
- 地图物体缺失：检查外部 Actor/Object 文件是否被提交。

### 二进制资产冲突

不要手工合并。备份双方文件，选择一个版本进入仓库，再在 UE 中重做另一方修改。

### 打包版本与编辑器表现不同

查看打包日志和 Staged EXE 日志；检查资源是否被硬引用、是否被 Cook、插件是否为 Runtime 依赖，并实际启动新生成的包，避免误测旧 EXE。

## 十五、推荐的下一阶段顺序

1. 在独立功能分支从已重存的白名单中选择一把 Deadghost 武器，只迁入该武器及其 Skeleton、PhysicsAsset、材质和纹理依赖；不得迁入排除项。
2. 保持 `Content/FPS_Weapon_Bundle` 受 `.gitignore` 保护；另一台电脑使用同一获许可 Epic 账号重新下载并按审计报告重存，不通过 Git/Git LFS 同步 Marketplace 源资源。
3. 实现多武器数据、拾取、切换和独立弹药。
4. 增加敌人外观、属性类型与更丰富行为。
5. 将原型补给箱替换为正式模型，并丰富关卡、机关、工具箱和战斗节奏。
6. 每个里程碑完成后编译、验证、打包、运行并推送。
