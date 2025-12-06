#include "JoystickRCSDK.h"
#include "QGCLoggingCategory.h"
#include <jni.h>
#include <QtCore/QJniEnvironment>
#include <QtCore/QJniObject>
#include <QtCore/QCoreApplication>

// 定义日志分类
QGC_LOGGING_CATEGORY(JoystickRCSDKLog, "qgc.joystick.rcsdk")

// 初始化静态成员变量
QList<int> JoystickRCSDK::_latestChannels;
QList<int> JoystickRCSDK::_latestButtons;
QMutex JoystickRCSDK::_dataMutex;

// ============================================================================
// 整体调用逻辑说明 (Architecture Overview)
// ============================================================================
/*
 * 1. 启动阶段 (C++ -> Java):
 *    QGC 启动 -> JoystickManager::init() -> 调用 JoystickRCSDK::init()。
 *    JoystickRCSDK::init() 获取 Android Context，通过 JNI 调用 Java 层的:
 *      - RCSDKBridge.initSDK()
 *      - RCSDKBridge.connectDevice()
 * 
 * 2. 连接握手 (Java -> C++ -> Java):
 *    Java 层 SDK 连接成功 -> 回调 onRcConnected。
 *    Java 调用 JNI 函数 nativeOnConnectStatus(true)。
 *    C++ 收到连接成功信号 -> 主动调用 Java 的 startChannelPolling() 开启数据流。
 * 
 * 3. 数据流转 (Java -> C++):
 *    Java 层收到遥控器数据 -> 调用 nativeOnChannelsReceived(int[] channels)。
 *    JNI 函数将 int[] 转为 QList<int> -> 调用 JoystickRCSDK::updateRCChannels()。
 *    数据被存入 _latestChannels (受 Mutex 保护)。
 * 
 * 4. 数据映射与消费 (C++ 内部):
 *    QGC 主循环轮询 Joystick::_update()。
 *    调用 JoystickRCSDK::_getAxis() 和 _getButton()。
 *    
 *    通道映射关系:
 *      轴 (Axes):
 *        - 轴 0-3: 对应通道 0-3 (摇杆: Throttle, Yaw, Pitch, Roll)
 *        - 轴 4-5: 对应通道 4-5 (拨钮: Switch 1, Switch 2)
 *        - 轴 6-7: 对应通道 10-11 (辅助摇杆)
 *      按钮 (Buttons):
 *        - 按钮 0-3: 对应通道 6-9 (PWM > 1500 表示按下)
 *    
 *    _getAxis 读取对应通道的 PWM 并将其从 PWM(1000-2000) 映射为 Axis(-32767~32767)。
 *    _getButton 读取对应通道的 PWM，当 PWM > 1500 时判定为按下。
 */

// ============================================================================
// JNI Native Implementation (Java 回调 C++ 的入口)
// ============================================================================

extern "C" {

// 对应 Java: nativeOnChannelsReceived(int[] channels)
JNIEXPORT void JNICALL Java_org_skydroidsdk_RCSDKBridge_nativeOnChannelsReceived(JNIEnv *env, jclass clazz, jintArray channels) {
    Q_UNUSED(clazz);
    jsize len = env->GetArrayLength(channels);
    if (len > 0) {
        jint *body = env->GetIntArrayElements(channels, 0);
        QList<int> channelList;
        for (int i = 0; i < len; ++i) {
            channelList.append(body[i]);
        }
        env->ReleaseIntArrayElements(channels, body, 0);
        
        // 将数据传递给 C++ 类
        JoystickRCSDK::updateRCChannels(channelList);
    }
}

// 对应 Java: nativeOnConnectStatus(boolean isConnected)
JNIEXPORT void JNICALL Java_org_skydroidsdk_RCSDKBridge_nativeOnConnectStatus(JNIEnv *env, jclass clazz, jboolean isConnected) {
    Q_UNUSED(env);
    Q_UNUSED(clazz);
    qCDebug(JoystickRCSDKLog) << "RCSDK Connection Status Changed:" << (isConnected ? "Connected" : "Disconnected");
    
    if (isConnected) {
        // 关键逻辑：连接成功后，通知 Java 层开始轮询通道数据
        // 100ms 是一个经验值，既保证流畅度又不至于让 Java 层负载过高
        QJniObject::callStaticMethod<void>(
            "org/skydroidsdk/RCSDKBridge",
            "startChannelPolling",
            "(I)V",
            100 
        );
    }
}

// 对应 Java: nativeOnLog(String message)
JNIEXPORT void JNICALL Java_org_skydroidsdk_RCSDKBridge_nativeOnLog(JNIEnv *env, jclass clazz, jstring message) {
    Q_UNUSED(clazz);
    const char *nativeString = env->GetStringUTFChars(message, 0);
    qCDebug(JoystickRCSDKLog) << "[RCSDK-Java]:" << nativeString;
    env->ReleaseStringUTFChars(message, nativeString);
}

// 占位函数，防止 Java 调用报错
JNIEXPORT void JNICALL Java_org_skydroidsdk_RCSDKBridge_nativeOnSignalQualityReceived(JNIEnv *env, jclass clazz, jint level) { Q_UNUSED(env); Q_UNUSED(clazz); Q_UNUSED(level); }
JNIEXPORT void JNICALL Java_org_skydroidsdk_RCSDKBridge_nativeOnDataReceived(JNIEnv *env, jclass clazz, jbyteArray data) { Q_UNUSED(env); Q_UNUSED(clazz); Q_UNUSED(data); }
JNIEXPORT void JNICALL Java_org_skydroidsdk_RCSDKBridge_nativeOnControlModeReceived(JNIEnv *env, jclass clazz, jint mode) { Q_UNUSED(env); Q_UNUSED(clazz); Q_UNUSED(mode); }

// 对应 Java: nativeOnButtonsReceived(int[] buttons)
JNIEXPORT void JNICALL Java_org_skydroidsdk_RCSDKBridge_nativeOnButtonsReceived(JNIEnv *env, jclass clazz, jintArray buttons) {
    Q_UNUSED(clazz);
    jsize len = env->GetArrayLength(buttons);
    if (len > 0) {
        jint *body = env->GetIntArrayElements(buttons, 0);
        QList<int> buttonList;
        for (int i = 0; i < len; ++i) {
            buttonList.append(body[i]);
        }
        env->ReleaseIntArrayElements(buttons, body, 0);
        
        // 将数据传递给 C++ 类
        JoystickRCSDK::updateRCButtons(buttonList);
    }
}

} // extern "C"

// ============================================================================
// JoystickRCSDK 类实现
// ============================================================================

JoystickRCSDK::JoystickRCSDK(const QString& name, int axisCount, int buttonCount, int hatCount, QObject* parent)
    : Joystick(name, axisCount, buttonCount, hatCount, parent)
{
    qCDebug(JoystickRCSDKLog) << "JoystickRCSDK instance created";
    
    // 初始化默认通道值 (1500 为中位)
    // 需要至少12个通道来支持所有映射 (0-11)
    QMutexLocker locker(&_dataMutex);
    if (_latestChannels.isEmpty()) {
        for(int i = 0; i < 12; i++) {
            _latestChannels.append(1500);
        }
    }
}

JoystickRCSDK::~JoystickRCSDK()
{
}

bool JoystickRCSDK::init()
{
    // 1. 获取 Android Context
    // QNativeInterface::QAndroidApplication::context() 是 Qt 6 的标准写法
    QJniObject context = QNativeInterface::QAndroidApplication::context();
    if (!context.isValid()) {
        qCWarning(JoystickRCSDKLog) << "Failed to get Android Context! RCSDK cannot init.";
        return false;
    }

    qCDebug(JoystickRCSDKLog) << "Initializing RCSDKBridge (Java)...";

    // 2. 调用 Java: RCSDKBridge.initSDK(context)
    QJniObject::callStaticMethod<void>(
        "org/skydroidsdk/RCSDKBridge",
        "initSDK",
        "(Landroid/content/Context;)V",
        context.object<jobject>()
    );

    // 3. 调用 Java: RCSDKBridge.connectDevice()
    // 注意：这里只负责发起连接，连接成功后会在 nativeOnConnectStatus 中启动数据轮询
    QJniObject::callStaticMethod<void>(
        "org/skydroidsdk/RCSDKBridge",
        "connectDevice",
        "()V"
    );
    
    return true;
}

QMap<QString, Joystick*> JoystickRCSDK::discover()
{
    static QMap<QString, Joystick*> ret;
    QString name = "Skydroid RCSDK";
    
    if (!ret.contains(name)) {
        // 通道映射:
        // 轴 (0-5):
        //   0-3: 摇杆 (Throttle, Yaw, Pitch, Roll)
        //   4-5: 拨钮 (Switch 1, Switch 2)
        // 按钮 (0-3):
        //   6-9 -> 按钮 0-3
        // 轴 (6-7):
        //   10-11: 摇杆 (Aux channels)
        int axisCount = 8;      // 0-5为主要轴，6-7为辅助轴
        int buttonCount = 4;    // 4个按钮对应通道6-9
        ret[name] = new JoystickRCSDK(name, axisCount, buttonCount, 0);
        qCDebug(JoystickRCSDKLog) << "Skydroid RCSDK Joystick registered.";
    }
    
    return ret;
}

void JoystickRCSDK::updateRCChannels(const QList<int>& channels)
{
    QMutexLocker locker(&_dataMutex);
    _latestChannels = channels;
    
    // // 调试输出：显示所有通道的原始PWM值
    // static int debugCounter = 0;
    // if (++debugCounter % 20 == 0) {  // 每20次更新打印一次，避免日志过多
    //     QString debugMsg = "【云卓遥控器】通道值(PWM): ";
    //     for (int i = 0; i < qMin(channels.count(), 12); ++i) {
    //         debugMsg += QString("Ch%1=%2 ").arg(i).arg(channels[i]);
    //     }
    //     qCDebug(JoystickRCSDKLog) << debugMsg;
    // }
}

void JoystickRCSDK::updateRCButtons(const QList<int>& buttons)
{
    QMutexLocker locker(&_dataMutex);
    _latestButtons = buttons;
}

bool JoystickRCSDK::_getButton(int i) const
{
    QMutexLocker locker(&_dataMutex);
    if (i < 0 || i >= 4) {
        return false;
    }
    
    // 按钮映射: 通道 6-9 -> 按钮 0-3
    // 这里假设PWM值大于1500表示按下，小于1500表示释放
    if (6 + i < _latestChannels.count()) {
        int pwm = _latestChannels[6 + i];
        bool pressed = pwm > 1500;
        
        // // 调试输出：当按钮状态改变时打印
        // static bool lastButtonStates[4] = {false, false, false, false};
        // if (pressed != lastButtonStates[i]) {
        //     qCDebug(JoystickRCSDKLog) << QString("【云卓遥控器】按钮%1 (通道%2): PWM=%3  →  %4")
        //         .arg(i).arg(6 + i).arg(pwm).arg(pressed ? "●按下" : "○释放");
        //     lastButtonStates[i] = pressed;
        // }
        
        return pressed;
    }
    return false;
}

int JoystickRCSDK::_getAxis(int i) const
{
    QMutexLocker locker(&_dataMutex);
    if (i < 0) {
        return 0;
    }

    int channelIndex;
    
    // 轴索引映射到通道索引
    if (i < 6) {
        // 轴 0-5 对应通道 0-5
        channelIndex = i;
    } else if (i < 8) {
        // 轴 6-7 对应通道 10-11
        channelIndex = 10 + (i - 6);
    } else {
        return 0;
    }
    
    if (channelIndex >= _latestChannels.count()) {
        return 0;
    }

    int pwm = _latestChannels[channelIndex];
    
    // 数据映射算法：
    // RCSDK: 1000 (Min) <-> 1500 (Center) <-> 2000 (Max)
    // QGC: -32767 (Min) <-> 0 (Center) <-> 32767 (Max)
    
    float diff = pwm - 1500;
    float percent = diff / 500.0f;
    int value = static_cast<int>(percent * 32767);
    
    // 安全限幅
    if (value > 32767) value = 32767;
    if (value < -32767) value = -32767;
    
    // // 调试输出：定期显示轴映射和转换结果
    // static int axisDebugCounter = 0;
    // if (i == 0 && ++axisDebugCounter % 50 == 0) {  // 每50次轴0查询时打印所有轴
    //     qCDebug(JoystickRCSDKLog) << "==================== 云卓遥控器轴映射详情 ====================";
    //     for (int axis = 0; axis < 8; ++axis) {
    //         int chIdx = (axis < 6) ? axis : (10 + axis - 6);
    //         if (chIdx < _latestChannels.count()) {
    //             int axisPwm = _latestChannels[chIdx];
    //             float axisDiff = axisPwm - 1500;
    //             float axisPercent = axisDiff / 500.0f;
    //             int axisValue = static_cast<int>(axisPercent * 32767);
    //             if (axisValue > 32767) axisValue = 32767;
    //             if (axisValue < -32767) axisValue = -32767;
                
    //             QString axisName;
    //             if (axis == 0) axisName = "油门(Throttle)";
    //             else if (axis == 1) axisName = "偏航(Yaw)    ";
    //             else if (axis == 2) axisName = "俯仰(Pitch)  ";
    //             else if (axis == 3) axisName = "横滚(Roll)   ";
    //             else if (axis == 4) axisName = "开关1(Sw1)   ";
    //             else if (axis == 5) axisName = "开关2(Sw2)   ";
    //             else if (axis == 6) axisName = "辅助1(Aux1)  ";
    //             else if (axis == 7) axisName = "辅助2(Aux2)  ";
                
    //             qCDebug(JoystickRCSDKLog) << QString("  轴%1 %2: 通道%3  PWM=%4  →  QGC值=%5")
    //                 .arg(axis).arg(axisName).arg(chIdx, 2).arg(axisPwm, 4).arg(axisValue, 6);
    //         }
    //     }
    //     qCDebug(JoystickRCSDKLog) << "================================================================";
    // }
    
    return value;
}
