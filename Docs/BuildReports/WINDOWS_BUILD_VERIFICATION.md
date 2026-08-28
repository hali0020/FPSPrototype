# FPSPrototype Windows 构建验证

引擎：Unreal Engine 5.8.2，CL `56702186`

目标：`FPSGame Win64 Development`

地图：`/Game/FirstPerson/Lvl_FirstPerson`

## 结果

- Build：通过。
- Full Cook：通过；发现 627 个包，Cook 620 个，按平台跳过 7 个，增量跳过 0 个。
- Pak：通过，Oodle Kraken 压缩。
- IoStore：通过。
- Stage：通过。
- Archive：通过。
- AutomationTool：`BUILD SUCCESSFUL`，`ExitCode=0`。
- 正式项目中 `Content/FPS_Weapon_Bundle` 不存在；归档中没有审计排除包路径。
- 本地授权音频存在时，16 个实际被代码引用的 SoundWave 进入 Cook；未使用的 `Pop_05` 仍只保留在本地忽略目录。

构建归档使用 Git 忽略的 `Saved/Packaged` 相对目录，不提交到仓库。归档共有 51 个文件，1,045,285,226 bytes。

## 公开源码基线验证

另在全 ASCII 路径的隔离工作副本中删去所有本地授权音频，只使用公开 Git 可获得内容执行同样的 BuildCookRun：

- Build：通过。
- Full Cook：通过；发现 611 个包，Cook 604 个，按平台跳过 7 个。
- Cook 摘要：`0 error(s), 0 warning(s)`。
- Pak、IoStore、Stage、Archive：全部通过。
- AutomationTool：`BUILD SUCCESSFUL`，`ExitCode=0`。
- 日志中没有 `HumanVocalizations`、`InterfaceAndItemSounds`、`Failed to find` 或 CDO 资源错误。

这证明干净克隆可以在没有受限音频的环境中独立编译、Cook、打包和启动；本地授权副本存在时则会自动加载并进入 Cook。

## 运行冒烟测试

归档外层 `FPSGame.exe` 使用以下测试边界运行：

- `-nullrhi`
- `-nosound`
- `-unattended`
- `-seconds=15`

结果：

- 进程退出码：0。
- 成功加载 `/Game/FirstPerson/Lvl_FirstPerson`。
- GameMode：`FPSGameMode`。
- 日志到达 `Main menu ready; gameplay has not started.`。
- 15 秒后由引擎计时器请求退出并完成正常清理。
- 含本地授权音频包：0 Error、0 Fatal、0 Ensure、0 Warning。
- 不含授权音频的公开源码包：0 Error、0 Fatal、0 Ensure、0 Warning。

本次自动测试证明包可启动、地图可加载、主菜单可进入且退出流程正常。射击、换弹、敌人攻击、死亡动作和暂停菜单仍需可见窗口交互回归。

## 关键文件 SHA-256

| 相对归档路径 | Bytes | SHA-256 |
| --- | ---: | --- |
| `FPSGame.exe` | 171,520 | `A31A24F494A15E9AF8F70E75675A44B46ECB7E35449692285686F08C923DBE40` |
| `FPSPrototype/Binaries/Win64/FPSGame.exe` | 332,087,808 | `8DFEC4A616F95A73155EBE6B042064D49BA555CCD5358D3D920AE2E490B33E72` |
| `FPSPrototype/Content/Paks/FPSPrototype-Windows.pak` | 11,172,200 | `2246A35F0B75CE0839C81314FA7FC3C25D56AF73972AEE12A74956677A11A678` |
| `FPSPrototype/Content/Paks/FPSPrototype-Windows.ucas` | 223,652,320 | `08AA635B7B5A3D52F9172F611744B9E6F58E7BCA33386A5477D0BB8B3427E239` |
| `FPSPrototype/Content/Paks/FPSPrototype-Windows.utoc` | 207,137 | `DED479AB042CF6AA671E593DCA1CA23138EFD4C5519F0C0B8270A0BC2E6C7CEF` |
| `FPSPrototype/Content/Paks/global.ucas` | 3,217,216 | `3B71A3E8387F5E9DDCECDECF139572946CD75A8FEA261200D83620B76EE5EFBA` |
| `FPSPrototype/Content/Paks/global.utoc` | 794 | `A3E3A4780D579306BAC792384299000F8C6928C67EA6355B686FBB79934623A7` |

## 已知提示与人工验收

- 构建环境的 MSVC `14.51.36256` 新于 UE 5.8 当前偏好的 `14.50.35717`。本次构建成功；若出现编译器相关异常，应优先切回引擎推荐工具链复核。
- 发布前应在可见窗口完成移动、ADS、自动射击、扩散、换弹、拾取、敌人生成/死亡、玩家死亡、暂停与退出测试。
- Git 只保存源码、配置和本报告；不提交 `Saved`、`Binaries`、`Intermediate`、打包目录或 Marketplace 原始资源。
