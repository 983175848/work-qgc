# QGC USB Manager NullPointerException 修复总结

## 问题描述

启动 QGC 应用时出现以下错误：

```
W/QGroundControl_Daily: java.lang.NullPointerException: 
Attempt to invoke virtual method 'java.util.HashMap android.hardware.usb.UsbManager.getDeviceList()' 
on a null object reference
at org.mavlink.qgroundcontrol.QGCUsbSerialManager.availableDevicesInfo(QGCUsbSerialManager.java:416)
```

## 根本原因

`UsbManager` 静态成员变量为 `null`，导致在调用其方法时产生 `NullPointerException`。

可能的原因：+
1. **时序问题**：Qt 层在 Java 初始化完成前就调用了 USB 相关方法
2. **初始化失败**：Context 服务获取失败
3. **条件竞争**：多线程访问未初始化的 `usbManager`

## 解决方案

### 1. 在 `availableDevicesInfo()` 中添加空值检查（第407-435行）
```java
public static String[] availableDevicesInfo() {
    // 添加空值检查
    if (usbManager == null) {
        Log.w(TAG, "UsbManager not initialized. Call initialize() first.");
        return null;
    }
    
    if (usbManager.getDeviceList().size() < 1) {
        return null;
    }
    // ... 其他代码
}
```

### 2. 在 `addOrUpdateDevice()` 中添加空值检查（第210-224行）
```java
private static void addOrUpdateDevice(UsbDevice device) {
    if (usbManager == null) {
        QGCLogger.w(TAG, "UsbManager not initialized in addOrUpdateDevice");
        return;
    }
    // ... 其他代码
}
```

### 3. 在添加驱动时添加空值检查（第335-348行）
```java
if (usbManager == null) {
    QGCLogger.w(TAG, "UsbManager not initialized when trying to request permission");
    return;
}
```

### 4. 在 `openDriver()` 中添加空值检查（第509行）
```java
private static boolean openDriver(...) {
    if (usbManager == null) {
        QGCLogger.e(TAG, "UsbManager not initialized when trying to open device");
        nativeDeviceException(classPtr, "UsbManager not initialized");
        return false;
    }
    // ... 其他代码
}
```

### 5. 在 `QGCActivity.onCreate()` 中添加异常处理（第50行）
```java
@Override
public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    
    // ... 其他初始化代码
    
    // 确保 USB 管理器在应用初始化时被设置
    try {
        QGCUsbSerialManager.initialize(this);
    } catch (Exception e) {
        Log.e(TAG, "Failed to initialize QGCUsbSerialManager", e);
    }
}
```

## 关于 tagSocket 日志

```
D/TrafficStats: tagSocket(83) with statsTag=0xffffffff, statsUid=-1
```

这是 Android 系统网络流量统计功能的调试日志，用于标记套接字的流量用途。这是**完全无害的**，不需要修复。

## 修改文件列表

1. `android/src/org/mavlink/qgroundcontrol/QGCUsbSerialManager.java` - 添加多处空值检查
2. `android/src/org/mavlink/qgroundcontrol/QGCActivity.java` - 添加异常处理

## 测试建议

1. 启动应用并确认不再出现 `NullPointerException`
2. 连接/断开 USB 设备，观察 USB 管理器是否正常工作
3. 检查日志中是否有新的警告信息，表示 `UsbManager` 未初始化

## 预期效果

修复后，如果 `UsbManager` 未初始化，应用将：
- 记录警告日志而不是崩溃
- 安全地返回 `null` 或执行降级操作
- 不影响应用的其他功能
