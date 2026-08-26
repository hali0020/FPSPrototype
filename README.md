# FPSPrototype

一个使用 Unreal Engine 5.8.2 和 C++ 编写的第一人称射击游戏原型。

## 当前内容

- 第一人称移动、跳跃、瞄准、自动射击、换弹和空仓反馈
- 第一人称手臂与步枪，以及低头可见的第三人称身体和腿部
- 敌人追击、近战攻击、受击、死亡、波次生成和掉落补给
- 生命、弹药、准星、命中/击杀提示、伤害红屏和死亡重开界面
- 开火、换弹、空仓、受击、死亡、敌人叫声和拾取音效
- 枪口火焰、表面命中特效、弹药箱、医疗箱和综合补给箱

## 操作

| 按键 | 功能 |
| --- | --- |
| `W A S D` | 移动 |
| 鼠标 | 观察/瞄准 |
| 鼠标左键 | 射击 |
| 鼠标右键 | 精确瞄准 |
| `R` | 换弹 |
| `Space` | 跳跃 |
| `Enter` | 死亡后重新开始 |

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
git pull --rebase
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

## 仓库安全与资产说明

- 仓库应保持为 **Private**。
- 不提交 `Binaries`、`DerivedDataCache`、`Intermediate`、`Saved` 或打包产物；这些都能重新生成。
- 项目包含 Unreal Engine 模板/示例内容及项目内使用的资源。不要把资源文件单独再分发，也不要在没有完成许可证审查前把仓库改为公开。
- 在另一台电脑使用 Marketplace/Fab 资源时，请使用拥有相应许可的同一 Epic 账号。
- 第三方资源记录见 `THIRD_PARTY_ASSETS.md`。
