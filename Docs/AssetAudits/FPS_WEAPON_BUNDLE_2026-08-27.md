# FPS Weapon Bundle 隔离安全、许可与 UE 5.8 兼容性审计

审计日期：2026-08-27

Fab 商品：[FPS Weapon Bundle](https://www.fab.com/listings/8aeb9c48-b404-4dcd-9e56-1d0ecedba7f5)

发布者：Deadghost Interactive

Fab listing ID：`8aeb9c48-b404-4dcd-9e56-1d0ecedba7f5`

## 决策

- `TECHNICAL_GATE = PASS`
- `UE58_RESAVE_AND_COOK = PASS`
- `LICENSE_GATE_PROJECT_USE = PASS`
- `LICENSE_GATE_PACKAGED_GAME = PASS`
- `PUBLIC_RAW_ASSET_DISTRIBUTION = BLOCKED`
- `GITHUB_RAW_ASSETS = BLOCKED_BY_PROJECT_POLICY`
- `FORMAL_PROJECT_MIGRATION = NOT_STARTED`

技术检查未发现可执行代码、脚本、插件、蓝图逻辑、外部项目依赖或明显恶意结构。白名单中的 218 个武器资产已在 UE 5.8.2 中隔离重存，并以 `218/218`、`0 errors`、`0 warnings` 完成第二次 Windows Cook。

这表示当前检查范围内没有发现阻止项目使用的技术问题，不表示任何二进制内容在理论上绝对“零风险”。正式项目目前仍未迁入该资源包。

## 官方页面与许可证据

审计时，已登录的官方 Fab 商品页同时显示：

- `Saved in My Library`
- `License terms: UE Marketplace`
- 指向 [Epic Content License Agreement](https://www.unrealengine.com/eula/content) 的许可证链接

与本项目直接相关的条款摘要：

- 可在项目内使用、复制和修改 Licensed Content。
- 可把资源以不可分割的 object-code 形式包含在打包游戏中向最终用户发布。
- 不得把 Marketplace 源格式资产作为独立内容向普通第三方分发。
- 源格式共享仅限为该项目善意工作的员工、关联方或承包商，并受条款中的删除和使用限制约束。

项目采用更保守的仓库政策：不上传该包原始或 UE5 重存后的 `.uasset/.umap`。其他开发环境使用拥有该许可的 Epic 账号重新下载，再按本报告的白名单和转换步骤恢复。该摘要不替代 Epic 的正式条款。

建议在项目合规档案中另行保存商品名、发布者、`Saved in My Library`、许可类型和 Epic 获取记录的截图；截图、账户信息、令牌和签名下载地址不得提交到仓库。

## 下载内容与隔离边界

官方 `Add to Project` 下载共有 548 个文件，约 416 MiB，包含三个顶层内容根：

| 内容根 | 文件数 | 字节数 | 结论 |
|---|---:|---:|---|
| `FPS_Weapon_Bundle` | 220 | 206,511,464 | 只审计并保留武器白名单 |
| `IndustryPropsPack6` | 94 | 192,958,008 | 无关附赠包，排除 |
| `InterfaceAndItemSounds` | 234 | 36,707,048 | 无关音效包，排除 |

隔离环境仅用于读取、Asset Registry 分析、Data Validation、UE5.8 格式转换和 Cook。资源没有直接下载到正式项目，也没有在审计期间提交到 Git。

## 静态结构检查

`FPS_Weapon_Bundle` 原始范围包含 219 个 `.uasset` 和 1 个 `.umap`。包头均具有合法 Unreal package magic，并带有 `++UE4+Release-4.12` 标记。

未发现：

- EXE、DLL、安装程序、批处理、PowerShell、JavaScript、Python、Lua 或 shell 脚本
- C/C++、C#、头文件、`uplugin`、Config、ThirdParty 二进制目录
- Blueprint、K2Node、EventGraph、声音、动画、Niagara、行为树、Custom HLSL
- ObjectRedirector、符号链接、目录联接、其他 reparse point 或 ADS
- 零字节、隐藏/系统文件、大小写冲突、Windows 非法文件名或超长路径

原始静态清单摘要 SHA-256：

`bd840a9b88867aa161235959f696e94c379b401d7a0260354e0eadd801d041b7`

## 白名单资产

永久排除问题包和展示地图后，白名单包含 218 个资产：

| 资产类型 | 数量 |
|---|---:|
| Material | 4 |
| MaterialInstanceConstant | 34 |
| PhysicsAsset | 16 |
| SkeletalMesh | 16 |
| Skeleton | 16 |
| StaticMesh | 54 |
| Texture2D | 78 |

内容覆盖 AR4、G67、KA Val、KA47、KA74U、M9 Knife 和 SMG11，以及瞄具、消音器、握把、弹匣和多种口径的 loaded/empty 弹药模型。

该资源包只提供模型侧资产，不包含开火、换弹、持枪或死亡动画，不包含枪声、拾取音效、粒子特效或游戏逻辑。接入时仍需由本项目的 C++、动画和音频系统完成玩法。

## 永久排除项

1. `FPS_Weapon_Bundle/Weapons/Meshes/KA_Val/SM_KA_Val_Mag_Loaded_X.uasset`
   - 仅 568 bytes。
   - 只有 PackageMetaData，没有 StaticMesh export。
   - 不是 redirector，且没有入站依赖。
2. `FPS_Weapon_Bundle/Maps/Weapons_Showcase.umap`
   - UE4.12 展示地图，包含旧 WorldSettings/BSP 警告，游戏不需要。
3. `IndustryPropsPack6/**`
   - 与武器目录没有 Asset Registry 依赖，不属于本次武器接入范围。
4. `InterfaceAndItemSounds/**`
   - 与武器目录没有 Asset Registry 依赖，且不作为枪声、命中声或拾取声使用。

## Asset Registry 与依赖结果

原始武器范围：

- 219 个有效 registry 资产；第 220 个文件是 568 字节问题包。
- 327 条依赖边。
- 外部 `/Game` 依赖：0。
- 缺失 `/Game` 依赖：0。
- Blueprint-like：0。
- Sound/MetaSound：0。

UE5.8 重存后的隔离项目：

- 218 个武器白名单资产；展示地图保持隔离且未重存。
- 外部 `/Game` 依赖：0。
- 缺失 `/Game` 依赖：0。
- Blueprint-like：0。
- Sound/MetaSound：0。

## UE 5.8 验证记录

### 首次 Data Validation

- 验证范围：218 个武器资产和 1 个隔离展示地图。
- `0 errors`、`149 warnings`。
- 警告来自展示地图旧数据、3 个 StaticMesh 物理数据重算，以及旧 Skeleton/PhysicsAsset 的迁移数据。

### 首次 Weapons-only Cook

- 只 Cook `Content/FPS_Weapon_Bundle/Weapons`。
- `218/218`、`0 errors`、`50 warnings`。
- 44 条警告来自旧骨骼/物理包在 Cook 时创建迁移子对象。
- 6 条警告来自 3 个 StaticMesh 的物理数据重算。

### 隔离重存

- 只重存 `Content/FPS_Weapon_Bundle/Weapons`。
- `218/218 packages were considered for resaving`。
- `218/218 packages were resaved`。
- `0/218 packages were deleted`。
- 展示地图的 SHA-256 在重存前后保持不变，证明未越过目录边界。

重存会把资产升级为 UE 5.8 当前序列化格式；这些副本不应再用于 UE4.12。

### 重存后第二次 Cook

- `218/218`、`0 errors`、`0 warnings`。
- `recomputing physics on load`：0。
- `created at cook time`：0。
- Cooked Asset Registry：218 个资产。
- 输出只包含 `/Game/FPS_Weapon_Bundle/Weapons`，没有地图或其他下载内容根。

## 正式项目接入规则

1. 只允许从通过 UE5.8 重存的 `FPS_Weapon_Bundle/Weapons` 白名单取用资源。
2. 永久排除本报告列出的四项内容范围。
3. 第一次只选择一把武器，并只迁入它的 SkeletalMesh、Skeleton、PhysicsAsset、材质、纹理和实际附件依赖。
4. 不执行 Reimport；原作者的导入源路径不是运行依赖，也不是本项目可用的源文件。
5. 逐项复核骨骼、碰撞、材质、枪口 socket、弹匣骨骼、ADS 基准、换弹和开火表现。
6. 每次接入后执行 Asset Validation、Editor 编译、Windows Cook/Stage 和真实 EXE 冒烟测试。
7. 保持 `.gitignore` 中的 `/Content/FPS_Weapon_Bundle/` 规则，不得用 `git add -f` 或 Git LFS 绕过。
8. Git 只提交代码、配置、审计记录和安装说明；不提交 Marketplace 原始/重存包、Fab 缓存、令牌、签名 URL、账户记录或收据截图。

## 剩余风险与发布前复核

- 官方商品页只声明 UE 4.12–4.27；UE 5.8 兼容性是本项目通过重存和 Cook 实测得到，不是发布者的官方声明。
- 静态审计和 Cook 无法证明不存在未知引擎解析漏洞、驱动问题、商标/外观争议或未来条款变化。
- 正式接入后仍必须人工检查第一人称遮挡、ADS 对齐、枪口方向、动画骨骼和实际帧率。
- 发布游戏前重新核对当时有效的 Fab 页面与 Epic Content License Agreement，并保留账户侧获取记录。
