# Thor 上的官方 GR00T N1.7 + G1

本目录保存 Thor 的部署配置。控制程序使用 Isaac OS v0.11.0 的原始 C++ 实现，
基于上游提交 `a4ab23f83266201fd117a4cf4205fb28c936b257`。
`control.yaml` 与上游配置的唯一差异是相机设备改为 `/dev/video42`。

当前实机模型为 `nvidia/GR00T-N1.7-ApplePnP-V1`，固定版本
`90caf594a5f1ce32e114103d6debf3ffabdb802d`，搭配 WBC-AGILE v1.2。
这是苹果放盘任务；此导出没有运行时文字指令入口，尚未验证拿水瓶。

## 日常启动

Thor 已安装以下入口。G1 充电、开机并接回原来的网线后运行：

```bash
systemctl --user start isaac-gr00t-g1
journalctl --user -u isaac-gr00t-g1 -u g1-camera-relay -f
```

第一条命令先启动载板相机转发。收到五帧图像后才启动官方实机程序。
程序会执行官方抬臂、落臂和站姿初始化，在最终姿态保持 60 秒后自动启用策略。
这段初始化及等待时间来自官方配置。

停止整个运行链路：

```bash
systemctl --user stop isaac-gr00t-g1
```

停止模型服务也会停止相机转发。机器人控制不自动重启，所有服务均未设为开机启动。
`systemctl stop` 是程序退出操作，不代替机器人遥控器的停止功能。

## 无机器人检查

```bash
systemctl --user start isaac-gr00t-check
journalctl --user -u isaac-gr00t-check -n 30 --no-pager
systemctl --user show isaac-gr00t-check -p Result -p ExecMainStatus
```

这个入口运行官方 mock 的 500 条命令，使用合成图像和状态，不连接机器人控制通道。
成功结果为 `Result=success`、`ExecMainStatus=0`，运行结束后服务为 `inactive`。
已验证缓存的五个 GR00T 引擎及一个 WBC 引擎均可加载，不需要下载或重新编译。

## 相机和网络

| 项目 | 配置 |
| --- | --- |
| Thor 网口 | `enP2p1s0`，`192.168.123.160/24` |
| NetworkManager 连接 | `Wired connection 1`，静态地址，自动连接，不抢默认路由 |
| G1 载板 | `unitree@192.168.123.164`，使用已有 SSH 密钥和主机密钥记录 |
| 载板彩色相机 | `/dev/video14`，RealSense RGB |
| Thor 虚拟相机 | `/dev/video42`，YUYV，640×480，30 FPS |
| 传输 | 载板 GStreamer → SSH → Thor FFmpeg → V4L2 |

相机仍插在 G1 载板上。相机时间戳表示图像到达 Thor 的时间，非两机同步的硬件时间戳。
之前实测 60 帧耗时 2.001 秒。重新开机后的设备编号尚待实机核实；如果编号变化，
先在载板用 `v4l2-ctl --list-devices` 找到 RGB 节点，再修改相机服务中的 `device=`。

只看相机，不运行机器人控制：

```bash
systemctl --user start g1-camera-relay
ffplay -f v4l2 -input_format yuyv422 -video_size 640x480 -framerate 30 /dev/video42
systemctl --user stop g1-camera-relay
```

连接失败看 `journalctl --user -u g1-camera-relay -n 30 --no-pager`。
SSH 连接超时设为 5 秒；相机服务只有在实际捕获图像后才完成启动。
相机单独启动失败后可用上面的 stop 命令结束重试。

## 版本与安装位置

- 源码：`/home/thor/lbz_ws/g1_ws/isaac_os_ws/src/isaac-os`
- 固定本机镜像：`isaac_os_build-gr00t_n1_7_wbc_g1:thor-a4ab23f`
- 镜像 ID：`sha256:2b29ffdf10551fd4297ba5000e774627eafda8454bb48d529243de51d81840ce`
- 模型和依赖缓存：`/home/thor/.cache/isaac-os`
- 构建目录：`build/gr00t_n1_7_wbc_g1`
- 本机运行记录：`/home/thor/lbz_ws/g1_ws/isaac_os_ws/logs`

固定镜像名避免切换 Git 分支时启动器寻找一个尚未构建的镜像。
`runtime.env` 供两个模型服务共用；`UV_OFFLINE=1` 让已有启动环境直接使用本机缓存。
实时模型的 GPU 分配曾测得约 12,278 MiB，当前停止后会释放。

在这台 Thor 上重新安装服务链接：

```bash
cd /home/thor/lbz_ws/g1_ws/isaac_os_ws/src/isaac-os
ln -sfn "$PWD/deployment/thor/systemd/g1-camera-relay.service" ~/.config/systemd/user/g1-camera-relay.service
ln -sfn "$PWD/deployment/thor/systemd/isaac-gr00t-g1.service" ~/.config/systemd/user/isaac-gr00t-g1.service
ln -sfn "$PWD/deployment/thor/systemd/isaac-gr00t-check.service" ~/.config/systemd/user/isaac-gr00t-check.service
systemctl --user daemon-reload
```

`host/` 保存已安装的 v4l2loopback 配置副本；本机已经安装 DKMS 模块、FFmpeg 和
v4l-utils。模块配置位于 `/etc/modprobe.d/g1-remote-camera.conf`，模块启动配置位于
`/etc/modules-load.d/g1-remote-camera.conf`。

需要重建时，沿用同一缓存挂载和镜像名：

```bash
cd /home/thor/lbz_ws/g1_ws/isaac_os_ws/src/isaac-os
export PATH="$HOME/.local/bin:$PATH"
export ISAAC_OS_EXTRA_DOCKER_RUN_ARGS='-v /home/thor/.cache/isaac-os:/cache/isaac-os -e CPM_SOURCE_CACHE=/cache/isaac-os/cpm -e ISAAC_RESOURCE_CACHE=/cache/isaac-os/resources'
./isaac_os build gr00t_n1_7_wbc_g1 real \
  --img=isaac_os_build-gr00t_n1_7_wbc_g1:thor-a4ab23f \
  --target mock_gr00t_n1_7_wbc_g1 --target real_gr00t_n1_7_wbc_g1 \
  --target unitree_g1_wbc --configure-args=-DAPP_wbc_g1=ON \
  --configure-args=-DISAAC_RESOURCE_LOCK_TIMEOUT=1800 --parallel 8
```

## 验证边界

已完成官方 mock 500 条命令和独立 WBC 实机 4,500 条命令，均正常退出。
完整实机程序已读取载板相机、加载五个模型并报告推理就绪。
上次在上肢自动启用前停止，随后现场报告电池耗尽；尚未完成真实抓取验证。

部署整理后，已通过新的离线检查服务再次运行 500 条命令，正常退出并复用全部引擎。
相机启动检查使用本机测试图像验证：没有生产者时持续等待，开始送帧后成功，
生产者退出后不会把旧帧当作就绪。载板离线启动也已验证会失败并可停止。
这些检查没有发送机器人控制命令。

复现固定演示时，按[官方实机说明](../../docs/tutorials/gr00t_wbc/real_robot.mdx)
布置约 75 cm 高的桌面、黑色桌布、左侧红苹果和右侧白盘，使物体进入头部相机视野。
相机恢复连接后的朝向、设备编号和实际抓取结果，留待 G1 充好电后验证。
