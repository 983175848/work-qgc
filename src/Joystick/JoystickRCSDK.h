#pragma once

#include <QObject>
#include <QMutex>
#include <QLoggingCategory>
#include "Joystick.h"

Q_DECLARE_LOGGING_CATEGORY(JoystickRCSDKLog)

/**
 * @brief Skydroid RCSDK 操纵杆实现类
 * 
 * 该类负责：
 * 1. 初始化 Java 层的 RCSDK。
 * 2. 接收来自 Java 层 (JNI) 的通道数据。
 * 3. 将 RCSDK 的 PWM 数据 (1000-2000) 转换为 QGC 的轴数据 (-32767~32767)。
 * 4. 伪装成一个标准的 QGC Joystick 设备供 JoystickManager 调度。
 */
class JoystickRCSDK : public Joystick
{
    Q_OBJECT
public:
    JoystickRCSDK(const QString& name, int axisCount, int buttonCount, int hatCount, QObject* parent = nullptr);
    ~JoystickRCSDK();

    // -------------------------------------------------------------------------
    // 静态接口 (供外部调用)
    // -------------------------------------------------------------------------

    /**
     * @brief 初始化 RCSDK 桥接
     * @return 成功返回 true
     * 
     * 逻辑：获取 Android Context -> 调用 Java initSDK -> 调用 Java connectDevice
     */
    static bool init();

    /**
     * @brief 设备发现函数
     * @return 包含本设备的 Map，供 JoystickManager 注册使用
     */
    static QMap<QString, Joystick*> discover();

    /**
     * @brief 更新通道数据 (供 JNI 回调使用)
     * @param channels 包含 16 个通道 PWM 值的列表
     */
    static void updateRCChannels(const QList<int>& channels);

    /**
     * @brief 更新按钮状态 (供 JNI 回调使用)
     * @param buttons 按钮状态数组，1表示按下，0表示释放
     */
    static void updateRCButtons(const QList<int>& buttons);

private:
    // -------------------------------------------------------------------------
    // Joystick 基类虚函数实现 (供 QGC 内部轮询)
    // -------------------------------------------------------------------------
    bool _open() override { return true; }
    void _close() override {}
    bool _update() override { return true; }

    bool _getButton(int i) const override;
    int _getAxis(int i) const override;
    bool _getHat(int hat, int i) const override { Q_UNUSED(hat); Q_UNUSED(i); return false; }

    // -------------------------------------------------------------------------
    // 内部数据存储
    // -------------------------------------------------------------------------
    static QList<int> _latestChannels; // 存储最新的通道数据
    static QList<int> _latestButtons;  // 存储最新的按钮状态
    static QMutex _dataMutex;          // 保证 JNI 线程与 QGC 线程的数据安全
};
