# FPSPrototype

一个使用 Unreal Engine 5.8.2 和 C++ 编写的第一人称射击游戏原型。

## 重要文档

- 开始开发或换电脑前，请先阅读：[开发与同步注意事项](DEVELOPMENT_GUIDE.md)
- 模型、声音与模板来源记录：[第三方与模板资源记录](THIRD_PARTY_ASSETS.md)

## 当前内容

- 第一人称移动、跳跃、瞄准、冲刺、自动射击、换弹和空仓反馈
- 冲刺时切换专用持枪跑动动作，并锁定开火与准星
- 腰射准星和 ADS 标记始终固定在最终渲染视角中心；枪械与手臂的小幅摆动只影响第一人称表现
- 实际射击以同一最终视角方向为基准，再叠加移动、空中和持续连射扩散；准星按同一扩散状态动态张合
- 自动射击期间移动速度降为正常步行速度的 65%，停止开火后恢复
- 右键进入约 1.5 倍 ADS；枪上使用项目内程序化生成的全息瞄具外框
- 第一人称手臂与 Epic/UE 模板步枪，以及低头可见的第三人称身体和腿部
- 敌人追击、近战攻击、受击、死亡、波次生成和掉落补给
- 敌人死亡时生成原生 C++ 光爆与碎片效果；使用固定数量的引擎基础图元和点光源，不依赖 Niagara
- 生命、弹药、准星、命中/击杀提示、伤害红屏和死亡重开界面
- 项目内确定性合成的步枪开火声和金属命中确认音；开火声使用 8 声部音频池保留连射尾音
- 换弹、空仓、受击、死亡、敌人叫声和拾取音效
- 枪口火焰、表面命中特效、弹药箱、医疗箱和综合补给箱
- 启动主菜单、游戏内暂停菜单，以及死亡后的重开、返回主菜单和退出选项

## 项目自生成音频

### 步枪开火声

- 生成脚本：`Tools/GenerateRifleShot.ps1`
- UE 导入脚本：`Tools/ImportGeneratedRifleAudio.py`
- 可复现 WAV：`SourceAssets/Audio/SFX_Rifle_Shot_01.wav`
- 导入后的 SoundWave：`Content/Weapons/Rifle/Audio/Generated/SFX_Rifle_Shot_01.uasset`
- 格式：48 kHz、单声道、16-bit PCM、0.82 秒
- WAV SHA-256：`A65249175815B674809C03B1C1249CD9EEB28867D9AA23FAC89D42E29D32D611`

### 金属命中确认音

- 生成脚本：`Tools/GenerateHitConfirm.ps1`
- UE 导入脚本：`Tools/ImportGeneratedHitConfirmAudio.py`
- 可复现 WAV：`SourceAssets/Audio/SFX_HitConfirm_01.wav`
- 导入后的 SoundWave：`Content/Weapons/Rifle/Audio/Generated/SFX_HitConfirm_01.uasset`
- UE 对象路径：`/Game/Weapons/Rifle/Audio/Generated/SFX_HitConfirm_01.SFX_HitConfirm_01`
- 格式：48 kHz、单声道、16-bit PCM、0.20 秒
- WAV SHA-256：`FFCD1AB65AC3BCB2CFB58FE3E812C795D398EC55450E15248216C42E9AAB7AA6`

两种声音都只使用固定随机种子和数学合成生成，不包含下载、录制或第三方音频采样。枪上的全息瞄具外框同样由项目程序化生成；武器主体仍是 Epic/UE 模板步枪，并不是尚未审计的 Deadghost 武器包。

## 操作

| 按键 | 功能 |
| --- | --- |
| `W A S D` | 移动 |
| `Left Shift` + `W` | 冲刺（冲刺中不能开火） |
| 鼠标 | 观察/瞄准 |
| 鼠标左键 | 射击；连射会增加扩散并暂时降低移动速度 |
| 鼠标右键 | 约 1.5 倍全息瞄具 ADS |
| `R` | 换弹 |
| `Space` | 跳跃 |
| `Enter` | 死亡后重新开始 |
| `Esc` | 游戏中暂停/继续，可在菜单中重开、返回主界面或退出游戏 |

## 家中电脑首次安装

1. 安装 Unreal Engine **5.8.2**。
2. 安装 Visual Studio 2022，并勾选“使用 C++ 的游戏开发”、Windows SDK 和 Unreal Engine 工具。
3. 安装 Git 与 Git LFS，然后执行 `git lfs install`。
4. 从 GitHub 克隆此私有仓库。不要只下载源码 ZIP，否则 Git LFS 资产可能不完整。
5. 如果提示找不到关联引擎，右键 `FPSPrototype.uproject`，先选择“Switch Unreal Engine version/切换 Unreal Engine 版本”并指定 5.8.2；然后生成 Visual Studio 项目文件。
6. 打开项目；首次启动会重新生成 `Binaries`、`Intermediate`、`Saved` 和缓存，时间较长属于正常现象。

如果引擎提示重建模块，请选择“是”。也可以先打开生成的解决方案，编译 `FPSPrototypeEditor` / `Development Editor`。

## 日常同步

开始工作前：

```powershell
git pull --ff-only
git lfs pull
```

完成工作后：

```powershell
git status
git add .
git commit -m "描述本次修改"
git push
```

UE 的 `.uasset` 和 `.umap` 无法像文本代码一样自动合并。两台电脑不要同时修改同一张地图或同一个蓝图/资产；切换电脑前先提交并推送。

> `Saved/StagedBuilds` 可能保留旧包。编译 Editor、Live Coding、导入资产和 Data Validation 都不会刷新旧 EXE；验证最新声音、模型或 C++ 时必须重新 Cook + Stage，并启动这一次新生成的可执行文件。

## 仓库安全与资产说明

- 仓库应保持为 **Private**。
- 不提交 `Binaries`、`DerivedDataCache`、`Intermediate`、`Saved` 或打包产物；这些都能重新生成。
- 项目包含 Unreal Engine 模板/示例内容及项目内使用的资源。不要把资源文件单独再分发，也不要在没有完成许可证审查前把仓库改为公开。
- Deadghost Interactive 的 FPS Weapon Bundle 目前仅在 Fab 账号中领取，尚未下载、导入或完成许可证与内容审计；当前游戏没有使用该包，不能将其描述为“零风险”。
- 在另一台电脑使用 Marketplace/Fab 资源时，请使用拥有相应许可的同一 Epic 账号。
- 完整注意事项见 `DEVELOPMENT_GUIDE.md`，第三方资源记录见 `THIRD_PARTY_ASSETS.md`。
