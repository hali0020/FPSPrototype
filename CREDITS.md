# Credits and third-party notices

本项目由 GitHub 用户 [repository-owner](https://github.com/repository-owner) 开发。游戏专用的 C++ 玩法代码、程序化全息瞄具，以及 `SourceAssets/Audio` 中由项目脚本确定性生成的音效，均为项目自产内容。

本文件记录第三方资源的作者、原始页面和公开分发边界。它用于致谢和发布前核对，不会替代原作者或 Epic Games 的正式许可证。以下结论核对于 **2026-08-27**。

## Epic Games — Unreal Engine templates and examples

- 作者/权利方：[Epic Games](https://www.epicgames.com/)
- 项目中的主要目录：`Content/Characters`、`Content/DemoTemplate`、`Content/FirstPerson`、`Content/Input`、`Content/LevelPrototyping`、`Content/Variant_Combat`、`Content/Variant_Shooter`、`Content/Weapons`
- 用途：角色、动画、模板步枪、输入、关卡原型和示例效果。
- 许可依据：[Unreal Engine EULA](https://www.unrealengine.com/eula/unreal) 将 Samples 与 Templates 目录中的代码、美术和其他内容定义为 “Examples”；第 5(b) 节允许以源码或目标代码形式向第三方分发 Examples，包括修改后的版本。
- 边界：本仓库不包含 Unreal Engine 引擎源码。Unreal、Unreal Engine、UE 和相关标识是 Epic Games 的商标。

## Gamemaster Audio — Human Vocalizations

- 原作者/发布者：[Gamemaster Audio](https://www.gamemasteraudio.com/)
- 原始页面：[Human Vocalizations on Fab](https://www.fab.com/listings/98259abf-477f-4015-8abe-2c9f62eaefdb)
- 作者页面：[Human Vocalizations on Gamemaster Audio](https://www.gamemasteraudio.com/product/human-vocalizations/)
- 项目目录：`Content/HumanVocalizations`
- 项目实际保留：14 个 SoundWave，用于玩家和敌人的攻击、受击、警戒与死亡短叫声。
- 原作者公开说明：Fab 页面将该包列为 2018 年 11 月 Unreal Engine Sponsored Content / Permanent Collection，目前显示为 Free；Gamemaster Audio 的合集许可说明其声音可商用、免版税、用于不限数量的项目且无需署名。
- 本项目的署名政策：即使原作者不强制要求，仍在此主动致谢 Gamemaster Audio。
- **公开源码状态：不可随公开仓库分发。** 当前没有找到原作者允许第三方重新发布源音频或 `.uasset` 的条款；通过 Fab/Marketplace 获取的副本受 [Epic Content License Agreement](https://www.unrealengine.com/eula/content) 约束。该协议允许把内容作为不可分割的目标代码包含在游戏中，也允许发布渲染图片和视频，但不允许向普通第三方分发源格式 Licensed Content。

## Daydream Sound — Interface & Item Sounds Pack

- 原作者/发布者：[Daydream Sound](https://www.fab.com/seller-id/o-5ft3wwcvbe5fb9p8ejq3rxzvatqr44)
- 原始页面：[Interface & Item Sounds Pack on Fab](https://www.fab.com/listings/78e31bcc-adfc-4816-8e10-609320deeeb1)
- 作者页面：[Interface & Item Sounds Pack on itch.io](https://daydream-sound.itch.io/interface-item-sounds-pack)
- 项目目录：`Content/InterfaceAndItemSounds`
- 项目实际保留：3 个 SoundWave；`Click_03` 和 `Flick_Switch_01` 分别用于空仓与换弹反馈，`Pop_05` 当前未被玩法代码引用。
- 原作者公开说明：该包属于 Permanently Free Collection，可在项目或作品中不限次数、永久使用，无额外费用或版税。
- 本项目的署名政策：在此主动致谢 Daydream Sound 团队。
- **公开源码状态：不可随公开仓库分发。** “可用于项目”并不等于“可重新发布源文件”；作者页面没有授予重新分发原始声音或 `.uasset` 的权利，通过 Fab/Marketplace 获取的副本仍受 Epic Content License Agreement 的源格式分发限制。

## Deadghost Interactive — FPS Weapon Bundle

- 发布者：[Deadghost Interactive](https://www.fab.com/listings/8aeb9c48-b404-4dcd-9e56-1d0ecedba7f5)
- 当前状态：只完成了独立安全、许可和 UE 5.8 兼容性审计，**没有纳入当前正式项目或 Git 历史**。
- 完整记录：[FPS Weapon Bundle 审计](Docs/AssetAudits/FPS_WEAPON_BUNDLE_2026-08-27.md)

## Public repository rule

署名不会扩大许可证授予的权利。以下 Marketplace/Fab 源资产不得进入任何公开可访问的 Git 历史：

```text
Content/HumanVocalizations/
Content/InterfaceAndItemSounds/
```

这两个目录已列入 `.gitignore`。本机可以保留通过当前用户 Epic/Fab 账号合法取得的副本，但公开源码的提交、分支、标签、Release 和 Actions Artifact 都不得包含它们。安全的作品展示方式包括：

1. 发布不含上述两个目录及其历史记录的干净源码仓库；需要这些声音的开发者应使用自己的 Epic/Fab 账号获取许可副本。
2. 单独发布 Cook/Package 后的可执行游戏、截图或演示视频；不要发布可提取复用的 Marketplace 原始音频或 `.uasset`。

项目代码在找不到这些可选声音时会跳过相应音频加载，但公开版本仍应重新编译、Cook 并进行一次完整试玩验证。
