package org.skydroidsdk;

import android.content.Context;
import android.util.Log;

import com.skydroid.rcsdk.KeyManager;
import com.skydroid.rcsdk.RCSDKManager;
import com.skydroid.rcsdk.SDKManagerCallBack;
import com.skydroid.rcsdk.PipelineManager;
import com.skydroid.rcsdk.comm.CommListener;
import com.skydroid.rcsdk.common.DeviceType;
import com.skydroid.rcsdk.common.Uart;
import com.skydroid.rcsdk.common.callback.CompletionCallback;
import com.skydroid.rcsdk.common.callback.CompletionCallbackWith;
import com.skydroid.rcsdk.common.callback.KeyListener;
import com.skydroid.rcsdk.common.error.SkyException;
import com.skydroid.rcsdk.common.pipeline.Pipeline;
import com.skydroid.rcsdk.common.remotecontroller.ControlMode;
import com.skydroid.rcsdk.key.AirLinkKey;
import com.skydroid.rcsdk.key.RemoteControllerKey;

import java.util.Arrays;
import java.util.Timer;
import java.util.TimerTask;

/**
 * RCSDKBridge - 云卓遥控器 SDK 全功能 Java 桥接类
 * 
 * 这个类封装了 Skydroid RCSDK 的核心功能，包括连接、通道、信号、数传、配对和摇杆模式设置。
 * 供 Qt C++ 代码通过 JNI 调用。
 * 
 * 主要功能模块：
 * 1. 基础管理 (Init, Connect, DeviceType)
 * 2. 通道数据 (Channels) - 自动适配 H16 (监听模式) 和 H12/H30 (轮询模式)
 * 3. 信号强度 (Signal) - 自动适配 H12 (轮询模式) 和 其他 (监听模式)
 * 4. 数传链路 (Pipeline) - 发送和接收透传数据
 * 5. 遥控器设置 (Pairing, ControlMode)
 */
public class RCSDKBridge {

    private static final String TAG = "RCSDKBridge";
    private static RCSDKBridge instance;
    private Context mContext;
    
    // 定时器
    private Timer mChannelTimer;
    private Timer mSignalTimer;
    
    // 状态
    private boolean isConnected = false;
    private Pipeline mPipeline;

    // 监听器引用 (防止被回收或用于取消监听)
    private KeyListener<int[]> h16ChannelListener;
    private KeyListener<Integer> signalListener;

    public static RCSDKBridge getInstance() {
        if (instance == null) {
            instance = new RCSDKBridge();
        }
        return instance;
    }

    // =============================================================
    // 1. 基础管理 (Initialization & Connection)
    // =============================================================

    /**
     * 初始化 SDK
     * Qt: QAndroidJniObject::callStaticMethod<void>("org/skydroidsdk/RCSDKBridge", "initSDK", "(Landroid/content/Context;)V", QtAndroid::androidActivity().object());
     */
    public static void initSDK(Context context) {
        Log.i(TAG, "initSDK: 开始初始化");
        getInstance().mContext = context;
        
        // 设置在主线程回调 (必须在 initSDK 之前调用)
        RCSDKManager.INSTANCE.setMainThreadCallBack(true);
        
        RCSDKManager.INSTANCE.initSDK(context, new SDKManagerCallBack() {
            @Override
            public void onRcConnected() {
                Log.i(TAG, "onRcConnected: 遥控器无线模块已连接");
                getInstance().isConnected = true;
                nativeOnConnectStatus(true);
                
                // 自动连接数传管道(需要飞机端接收机在线)
                connectPipeline();
            }

            @Override
            public void onRcConnectFail(SkyException e) {
                Log.w(TAG, "onRcConnectFail: " + e.getMessage());
                
                // 注意: 这个错误是正常的!
                // 原因: 遥控器未连接到飞机的接收机,无线链路未建立
                // 影响: 数传管道无法使用,但本地摇杆数据仍然可以正常读取
                Log.i(TAG, "onRcConnectFail: 无线链路未建立(正常情况),摇杆数据仍可用");
                
                // 仍然通知 C++ 层 SDK 就绪,因为摇杆是本地硬件
                getInstance().isConnected = false; // 标记为未连接飞机
                nativeOnConnectStatus(true); // 但 SDK 本身是可用的
                
                nativeOnLog("RC not paired with receiver (normal if no aircraft connected)");
            }

            @Override
            public void onRcDisconnect() {
                Log.i(TAG, "onRcDisconnect: 遥控器断开");
                getInstance().isConnected = false;
                nativeOnConnectStatus(false);
            }
        });
    }

    /**
     * 连接遥控器
     * 注意: 这个方法尝试连接遥控器的无线模块,如果遥控器未与飞机配对会失败
     * 但失败不影响本地摇杆数据的读取
     * Qt: QAndroidJniObject::callStaticMethod<void>("org/skydroidsdk/RCSDKBridge", "connectDevice", "()V");
     */
    public static void connectDevice() {
        Log.i(TAG, "connectDevice: 尝试连接遥控器无线模块");
        
        DeviceType deviceType = RCSDKManager.INSTANCE.getDeviceType();
        Log.i(TAG, "connectDevice: 设备类型 = " + deviceType);
        
        if (deviceType == DeviceType.UNKNOWN) {
            Log.e(TAG, "connectDevice: 设备类型未知 - SDK 可能未在云卓遥控器上运行!");
            nativeOnLog("ERROR: Not running on Skydroid RC hardware!");
            return;
        }
        
        try {
            Log.i(TAG, "connectDevice: 调用 connectToRC()");
            RCSDKManager.INSTANCE.connectToRC();
            // 注意: 连接结果会在 SDKManagerCallBack 中异步返回
            // 如果返回 "Not yet implemented",说明遥控器未与飞机配对,但这是正常的
        } catch (Exception e) {
            Log.e(TAG, "connectDevice: 异常 - " + e.getMessage());
            nativeOnLog("connectToRC exception: " + e.getMessage());
        }
    }

    /**
     * 获取设备类型 ID
     * 返回值对应 DeviceType 枚举的 ordinal
     * Qt: int type = QAndroidJniObject::callStaticMethod<jint>("com/skydroid/rcsdkdemo/RCSDKBridge", "getDeviceType", "()I");
     */
    public static int getDeviceType() {
        DeviceType type = RCSDKManager.INSTANCE.getDeviceType();
        Log.i(TAG, "getDeviceType: " + type.name());
        return type.ordinal();
    }

    // =============================================================
    // 2. 通道数据 (Channels)
    // =============================================================

    /**
     * 开始获取通道数据
     * 自动根据设备类型选择 监听模式(H16) 或 轮询模式(H12/H30)
     * @param intervalMs 轮询间隔 (仅对非 H16 设备有效)
     * Qt: QAndroidJniObject::callStaticMethod<void>("com/skydroid/rcsdkdemo/RCSDKBridge", "startChannelPolling", "(I)V", 50);
     */
    public static void startChannelPolling(int intervalMs) {
        DeviceType type = RCSDKManager.INSTANCE.getDeviceType();
        Log.i(TAG, "startChannelPolling: Device=" + type + ", Interval=" + intervalMs);

        if (type == DeviceType.H16) {
            // H16 使用监听模式
            if (getInstance().h16ChannelListener == null) {
                getInstance().h16ChannelListener = new KeyListener<int[]>() {
                    @Override
                    public void onValueChange(int[] oldValue, int[] newValue) {
                        nativeOnChannelsReceived(newValue);
                    }
                };
            }
            // 先取消可能存在的监听
            KeyManager.INSTANCE.cancelListen(getInstance().h16ChannelListener);
            KeyManager.INSTANCE.listen(RemoteControllerKey.INSTANCE.getKeyH16Channels(), getInstance().h16ChannelListener);
        } else {
            // 其他设备使用轮询模式
            stopChannelPolling(); // 先停止旧的
            getInstance().mChannelTimer = new Timer();
            getInstance().mChannelTimer.schedule(new TimerTask() {
                @Override
                public void run() {
                    KeyManager.INSTANCE.get(RemoteControllerKey.INSTANCE.getKeyChannels(), new CompletionCallbackWith<int[]>() {
                        @Override
                        public void onSuccess(int[] channels) {
                            //Log.i(TAG, "channels len=" + (channels == null ? -1 : channels.length) +
                            //    ", data=" + Arrays.toString(channels));
                            nativeOnChannelsReceived(channels);
                        }
                        @Override
                        public void onFailure(SkyException e) {
                            // Log.e(TAG, "Get Channels Fail: " + e);
                        }
                    });
                }
            }, 0, intervalMs);
        }
    }

    /**
     * 停止获取通道数据
     * Qt: QAndroidJniObject::callStaticMethod<void>("com/skydroid/rcsdkdemo/RCSDKBridge", "stopChannelPolling", "()V");
     */
    public static void stopChannelPolling() {
        // 停止定时器
        if (getInstance().mChannelTimer != null) {
            getInstance().mChannelTimer.cancel();
            getInstance().mChannelTimer = null;
        }
        // 停止监听
        if (getInstance().h16ChannelListener != null) {
            KeyManager.INSTANCE.cancelListen(getInstance().h16ChannelListener);
        }
    }

    // =============================================================
    // 3. 信号强度 (Signal Quality)
    // =============================================================

    /**
     * 开始获取信号强度
     * 自动适配 H12 (轮询) 和 其他 (监听)
     * Qt: QAndroidJniObject::callStaticMethod<void>("com/skydroid/rcsdkdemo/RCSDKBridge", "startSignalPolling", "(I)V", 1000);
     */
    public static void startSignalPolling(int intervalMs) {
        DeviceType type = RCSDKManager.INSTANCE.getDeviceType();
        
        if (type == DeviceType.H12) {
            // H12 需要轮询
            if (getInstance().mSignalTimer != null) getInstance().mSignalTimer.cancel();
            getInstance().mSignalTimer = new Timer();
            getInstance().mSignalTimer.schedule(new TimerTask() {
                @Override
                public void run() {
                    KeyManager.INSTANCE.get(AirLinkKey.INSTANCE.getKeyH12SignalQuality(), new CompletionCallbackWith<Integer>() {
                        @Override
                        public void onSuccess(Integer val) {
                            nativeOnSignalQualityReceived(val);
                        }
                        @Override
                        public void onFailure(SkyException e) {}
                    });
                }
            }, 0, intervalMs);
        } else {
            // 其他设备使用监听
            if (getInstance().signalListener == null) {
                getInstance().signalListener = new KeyListener<Integer>() {
                    @Override
                    public void onValueChange(Integer oldVal, Integer newVal) {
                        nativeOnSignalQualityReceived(newVal);
                    }
                };
            }
            KeyManager.INSTANCE.cancelListen(getInstance().signalListener);
            KeyManager.INSTANCE.listen(AirLinkKey.INSTANCE.getKeySignalQuality(), getInstance().signalListener);
        }
    }

    public static void stopSignalPolling() {
        if (getInstance().mSignalTimer != null) {
            getInstance().mSignalTimer.cancel();
            getInstance().mSignalTimer = null;
        }
        if (getInstance().signalListener != null) {
            KeyManager.INSTANCE.cancelListen(getInstance().signalListener);
        }
    }

    // =============================================================
    // 4. 数传链路 (Data Pipeline)
    // =============================================================

    /**
     * 连接数传管道 (UART0)
     * 通常在 initSDK 成功后自动调用，也可以手动调用
     * Qt: QAndroidJniObject::callStaticMethod<void>("com/skydroid/rcsdkdemo/RCSDKBridge", "connectPipeline", "()V");
     */
    public static void connectPipeline() {
        Log.i(TAG, "connectPipeline: 创建并连接管道");
        Pipeline pipeline = PipelineManager.INSTANCE.createPipeline(Uart.UART0);
        pipeline.setOnCommListener(new CommListener() {
            @Override
            public void onConnectSuccess() {
                Log.i(TAG, "Pipeline Connected");
                nativeOnLog("Pipeline Connected");
            }
            @Override
            public void onConnectFail(SkyException e) {
                Log.e(TAG, "Pipeline Connect Fail: " + e);
            }
            @Override
            public void onDisconnect() {
                Log.i(TAG, "Pipeline Disconnected");
            }
            @Override
            public void onReadData(byte[] bytes) {
                // 收到数据，传给 Qt
                nativeOnDataReceived(bytes);
            }
        });
        PipelineManager.INSTANCE.connectPipeline(pipeline);
        getInstance().mPipeline = pipeline;
    }

    /**
     * 发送数据到遥控器串口
     * Qt: 
     * QByteArray data = ...;
     * jbyteArray jData = env->NewByteArray(data.size());
     * env->SetByteArrayRegion(jData, 0, data.size(), (jbyte*)data.data());
     * QAndroidJniObject::callStaticMethod<void>("com/skydroid/rcsdkdemo/RCSDKBridge", "sendData", "([B)V", jData);
     */
    public static void sendData(byte[] data) {
        if (getInstance().mPipeline != null) {
            getInstance().mPipeline.writeData(data);
        } else {
            Log.e(TAG, "sendData: Pipeline is null");
        }
    }

    public static void disconnectPipeline() {
        if (getInstance().mPipeline != null) {
            PipelineManager.INSTANCE.disconnectPipeline(getInstance().mPipeline);
            getInstance().mPipeline = null;
        }
    }

    // =============================================================
    // 5. 遥控器设置 (Settings)
    // =============================================================

    /**
     * 请求对频
     * Qt: QAndroidJniObject::callStaticMethod<void>("com/skydroid/rcsdkdemo/RCSDKBridge", "requestPairing", "()V");
     */
    public static void requestPairing() {
        KeyManager.INSTANCE.action(RemoteControllerKey.INSTANCE.getKeyRequestPairing(), new CompletionCallback() {
            @Override
            public void onResult(SkyException e) {
                if (e == null) nativeOnLog("Pairing Request Success");
                else nativeOnLog("Pairing Request Fail: " + e);
            }
        });
    }

    /**
     * 设置摇杆模式 (1=USA, 2=JAPAN, etc.)
     * Qt: QAndroidJniObject::callStaticMethod<void>("com/skydroid/rcsdkdemo/RCSDKBridge", "setControlMode", "(I)V", 1);
     */
    public static void setControlMode(int modeOrdinal) {
        ControlMode mode = ControlMode.values()[modeOrdinal]; // 简单映射，需注意越界
        KeyManager.INSTANCE.set(RemoteControllerKey.INSTANCE.getKeyControlMode(), mode, new CompletionCallback() {
            @Override
            public void onResult(SkyException e) {
                if (e == null) nativeOnLog("Set Control Mode Success");
                else nativeOnLog("Set Control Mode Fail: " + e);
            }
        });
    }

    /**
     * 获取摇杆模式
     * Qt: QAndroidJniObject::callStaticMethod<void>("com/skydroid/rcsdkdemo/RCSDKBridge", "getControlMode", "()V");
     */
    public static void getControlMode() {
        KeyManager.INSTANCE.get(RemoteControllerKey.INSTANCE.getKeyControlMode(), new CompletionCallbackWith<ControlMode>() {
            @Override
            public void onSuccess(ControlMode mode) {
                nativeOnControlModeReceived(mode.ordinal());
            }
            @Override
            public void onFailure(SkyException e) {
                nativeOnLog("Get Control Mode Fail: " + e);
            }
        });
    }

    // =============================================================
    // Native Callbacks (JNI)
    // =============================================================

    public static native void nativeOnConnectStatus(boolean isConnected);
    public static native void nativeOnChannelsReceived(int[] channels);
    public static native void nativeOnSignalQualityReceived(int level);
    public static native void nativeOnDataReceived(byte[] data);
    public static native void nativeOnControlModeReceived(int mode);
    public static native void nativeOnLog(String message);
}
