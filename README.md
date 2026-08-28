# FPSPrototype

基于 Unreal Engine 5.8.2 与 C++ 构建的单人第一人称射击原型。项目围绕移动、瞄准、射击反馈、敌人行为和完整游戏流程展开，核心运行逻辑位于原生 C++ 模块中。

## 项目概览

| 系统 | 实现内容 |
| --- | --- |
| 移动与视角 | 行走、跳跃、冲刺、最终渲染视角对齐、约 1.5 倍 ADS |
| 武器 | Hitscan 自动步枪、弹匣与备弹、换弹、空仓、枪口反馈、连续射击扩散 |
| 命中反馈 | 动态准星、命中与击杀标记、伤害红屏、表面命中特效 |
| 敌人 | 快速型与重装型两种配置、追击、近战、受击、死亡、波次生成 |
| 游戏流程 | 启动菜单、暂停、死亡重开、返回主菜单、退出 |
| 补给 | 弹药、医疗和综合补给三种拾取类型 |
| 视听表现 | C++ 生成/驱动的出生、死亡和命中特效；确定性合成音效 |

## 关键设计

### 统一的瞄准与射击基准

腰射准星、ADS 标记和实际射线都以最终相机视角为基准。枪械与第一人称手臂可以保留轻微运动表现，但不会改变真正的命中方向，从而避免视觉瞄点与射击结果分离。

### 同源扩散状态

移动、滞空和持续连射共同影响扩散。准星张合与射线偏移读取同一份扩散状态，玩家看到的精度变化与实际弹道保持一致。自动射击期间移动速度会降至正常步速的 65%，停止或中断射击后恢复。

### 可控的动画与生命周期

敌人死亡时先播放完整死亡动作并保留尸体，随后隐藏模型并生成独立的死亡效果 Actor。效果生命周期不依赖尸体 Actor，避免清理尸体时提前截断视觉表现。

### 原生 C++ 特效路径

命中、敌人出生和死亡效果由基础组件、动态材质与灯光组合完成，不依赖运行时 Niagara 资产。该实现减少了打包后的资源序列化差异，并让效果时序直接受玩法代码控制。

### 可复现音频

步枪开火、命中确认和三类拾取音效由 PowerShell 脚本使用固定随机种子与数学波形生成，不包含下载采样或第三方录音。源 WAV、生成脚本和 Unreal 导入脚本均保留在仓库中。

## 代码结构

| 文件/模块 | 职责 |
| --- | --- |
| `FPSCharacter` | 输入、移动、视角、ADS、武器状态、射击、换弹和玩家反馈 |
| `FPSEnemy` | 敌人配置、追击、攻击、动画、受击和死亡流程 |
| `FPSGameMode` | 波次、生成点、计分、补给掉落和重开流程 |
| `FPSHUD` | 生命、弹药、准星、命中提示、伤害与死亡界面 |
| `HealthComponent` | 通用生命值、伤害与死亡事件 |
| `FPSImpactEffect` | 表面命中的原生 C++ 视觉反馈 |
| `FPSEnemySpawnEffect` | 敌人出生光柱与扩张环 |
| `FPSDeathEffect` | 核心光爆、冲击环与飞散碎片 |
| `FPSMenuWidget` / `FPSPlayerController` | 主菜单、暂停和输入模式切换 |

核心源码位于 [`Source/FPSGame`](Source/FPSGame)。

## 操作

| 输入 | 功能 |
| --- | --- |
| `W A S D` | 移动 |
| `Left Shift` + `W` | 冲刺；冲刺期间锁定开火 |
| 鼠标 | 观察 |
| 鼠标左键 | 自动射击 |
| 鼠标右键 | ADS |
| `R` | 换弹 |
| `Space` | 跳跃 |
| `Enter` | 死亡后重新开始 |
| `Esc` | 暂停或返回菜单流程 |

## 技术环境

- Unreal Engine 5.8.2（已验证 CL `56702186`）
- Unreal C++ Runtime Module
- Windows / DirectX 12 / Shader Model 6
- Visual Studio C++ 工具链（按仓库内 `.vsconfig` 安装）
- UE 5.8 首选 MSVC 14.50；当前也已用 MSVC 14.51 验证
- Git LFS
- PowerShell 音频生成脚本
- Unreal Python 资源导入脚本

## 获取与运行

1. 安装 Unreal Engine 5.8.2。
2. 使用 `.vsconfig` 安装 Visual Studio C++、Windows SDK 和 Unreal Engine 工具组件。
3. 安装 Git 与 Git LFS。
4. 将仓库克隆到全 ASCII 路径（例如 `D:\UEProjects\FPSPrototype`），并准备资源：

```powershell
git lfs install
git clone <repository-url>
cd FPSPrototype
git lfs pull
```

   当前 UE/MSVC 组合在含中文等非 ASCII 字符的工程路径下可能因 PCH 路径而编译失败，所以不要把正式工作副本放在这类路径。

5. 初始化只属于当前电脑的隐私文件和通用提交身份：

```powershell
Copy-Item .local/private.example.json .local/private.json
git config --local user.name "FPSPrototype Contributors"
git config --local user.email "contributors@users.noreply.github.com"
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\Tools\CheckPrivacy.ps1
```

6. 右键 `FPSPrototype.uproject`，选择 UE 5.8.2 并生成 Visual Studio 项目文件。
7. 编译 `FPSGameEditor` / `Development Editor`，然后打开项目。
8. 默认地图为 `/Game/FirstPerson/Lvl_FirstPerson`。

首次启动会生成 `Binaries`、`Intermediate`、`Saved` 和缓存目录，这些内容均不会提交到仓库。

## 资源边界

公开源码不包含 `Human Vocalizations` 与 `Interface & Item Sounds Pack` 的源格式 SoundWave。缺少这些可选资源时，代码会静默跳过相应角色语音、空仓和换弹声音，核心玩法仍可编译、Cook 和运行。需要完整声音的开发环境应通过自己的 Epic/Fab 账号取得许可副本。本地资源保留在已忽略的 `Content/HumanVocalizations`、`Content/InterfaceAndItemSounds` 和 `.local-licensed-assets` 中，不会进入 Git。

Unreal Engine 模板内容、第三方资源来源和分发边界记录在 [`CREDITS.md`](CREDITS.md) 与 [`THIRD_PARTY_ASSETS.md`](THIRD_PARTY_ASSETS.md)。受限 Marketplace/Fab 源资源不会进入提交、分支、标签、Release 或公开构建附件。

## 本地隐私与提交检查

项目需要记住的本机用户名、计算机名、账号别名和绝对路径统一写在 `.local/private.json`；该文件已被 Git 精确忽略，仓库只跟踪空值模板 `.local/private.example.json`。`Tools/CheckPrivacy.ps1` 会扫描已跟踪工作树和暂存区中的受支持文本文件，拦截本地值、用户目录、非通用邮箱、凭据 URL、令牌和私钥；它还会拒绝本地授权资源目录被 `git add -f` 强制加入索引，并且不会在报告中回显命中内容。

`.gitignore` 不是加密。密码、token、API Key、私钥和证书仍必须使用 Windows Credential Manager、环境变量或其他专用秘密存储，不能写入这个 JSON。Git 远程地址保留在本地 `.git/config`，登录凭据由凭据管理器保管，两者都不是跟踪文件。检查脚本不会解析 `.uasset`、`.umap` 等二进制资产的内部元数据；新导入或来源不明的二进制资源仍须按资产审计流程单独检查。

## 验证状态

- Windows Development 编译通过
- Full Cook、Pak、IoStore、Stage 与 Archive 通过
- 打包版本完成无渲染、无声音启动冒烟测试
- 关键构建步骤结果为 `0 errors / 0 warnings`

详细记录见 [`Docs/BuildReports/WINDOWS_BUILD_VERIFICATION.md`](Docs/BuildReports/WINDOWS_BUILD_VERIFICATION.md)。打包产物不存放在 Git 历史中。

## 当前边界

- 当前玩法为本地单人模式，尚未实现网络复制与服务端权威逻辑。
- 主要验证平台为 Windows DX12 / SM6。
- 自动化冒烟测试覆盖启动、地图加载、主菜单和正常退出；完整战斗流程仍需可见窗口测试。
- UE 二进制资产无法像文本源码一样自动合并，协作时应避免并行修改同一 `.uasset` 或 `.umap`。

## 延伸文档

- [开发、同步与打包指南](DEVELOPMENT_GUIDE.md)
- [第三方资源与许可证记录](THIRD_PARTY_ASSETS.md)
- [作者署名与公开分发边界](CREDITS.md)
- [FPS Weapon Bundle 隔离审计](Docs/AssetAudits/FPS_WEAPON_BUNDLE_2026-08-27.md)
