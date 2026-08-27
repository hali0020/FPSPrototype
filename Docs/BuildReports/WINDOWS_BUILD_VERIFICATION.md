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

构建归档使用 Git 忽略的 `Saved/Packaged` 相对目录，不提交到仓库。归档共有 48 个文件，1,045,304,311 bytes。

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
- 运行日志：0 Error、0 Fatal、0 Ensure、0 Warning。

本次自动测试证明包可启动、地图可加载、主菜单可进入且退出流程正常。射击、换弹、敌人攻击、死亡动作和暂停菜单仍需可见窗口交互回归。

## 关键文件 SHA-256

| 相对归档路径 | Bytes | SHA-256 |
| --- | ---: | --- |
| `FPSGame.exe` | 171,520 | `A31A24F494A15E9AF8F70E75675A44B46ECB7E35449692285686F08C923DBE40` |
| `FPSPrototype/Binaries/Win64/FPSGame.exe` | 332,088,320 | `515D212C158B8C95B823CF199AA28C734E68A36D6936B11A2B4AEB0DA32533AA` |
| `FPSPrototype/Content/Paks/FPSPrototype-Windows.pak` | 11,172,829 | `49F630224664696A6A336FE4BE88F9A9B6062D0E8A99121635EABA5754E38F76` |
| `FPSPrototype/Content/Paks/FPSPrototype-Windows.ucas` | 223,652,320 | `08AA635B7B5A3D52F9172F611744B9E6F58E7BCA33386A5477D0BB8B3427E239` |
| `FPSPrototype/Content/Paks/FPSPrototype-Windows.utoc` | 207,137 | `DED479AB042CF6AA671E593DCA1CA23138EFD4C5519F0C0B8270A0BC2E6C7CEF` |
| `FPSPrototype/Content/Paks/global.ucas` | 3,228,336 | `D6DE7CC0AC463E7E12A82935C94F9E38AF6DC57D13C01B2A663A70E4772F206D` |
| `FPSPrototype/Content/Paks/global.utoc` | 794 | `2F527BFBE82AC4A3B168F1B727AFDDDC938BA1EB399B62B23FD4712C63693ACC` |

## 已知提示与人工验收

- 构建环境的 MSVC `14.51.36256` 新于 UE 5.8 当前偏好的 `14.50.35717`。本次构建成功；若出现编译器相关异常，应优先切回引擎推荐工具链复核。
- 发布前应在可见窗口完成移动、ADS、自动射击、扩散、换弹、拾取、敌人生成/死亡、玩家死亡、暂停与退出测试。
- Git 只保存源码、配置和本报告；不提交 `Saved`、`Binaries`、`Intermediate`、打包目录或 Marketplace 原始资源。
