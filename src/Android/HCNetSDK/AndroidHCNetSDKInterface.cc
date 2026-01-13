#include "AndroidHCNetSDKInterface.h"

#include <QJniObject>
#include <QJniEnvironment>
#include <QDebug>
#include <QtCore/QApplicationStatic>

AndroidHCNetSDKInterface::AndroidHCNetSDKInterface(QObject *parent)
    : QObject{parent}
{

}

Q_APPLICATION_STATIC(AndroidHCNetSDKInterface, _androidHCNetSDKInstance);
                                        //Qt 提供的一个宏，用于在应用范围内创建一个“懒惰初始化”的全局静态对象提供器
                                        //（不是直接裸指针单例）。它生成内部函数/对象以便在首次需要时构造实例，并在程序退出时正确析构。
                                        //宏的第一个参数是类型（这里是 AndroidHCNetSDKInterface），第二个参数是内部静态函数/名字（这里是 _androidHCNetSDKInstance）。

AndroidHCNetSDKInterface* AndroidHCNetSDKInterface::instance()  //是宏生成的函数，调用它会返回指向单例对象的指针。如果对象尚未构造
                                                                //，首次调用会构造对象并返回指针；程序结束时 Qt 会负责析构。
{                                   //函数只是把宏生成的访问器封装成更直观的静态方法，其他代码通过 AndroidHCNetSDKInterface::instance() 获取全局单例并调用其成员方法
    return _androidHCNetSDKInstance();
}

bool AndroidHCNetSDKInterface::hcNetSDKinit()   //一个程序只需要创建一次
{
            // 3️⃣ 调用 getInstance() 获得 HCNetSDK 单例对象
    hcNetSDKInstance = QJniObject::callStaticObjectMethod(  //静态对象
        "com/hikvision/netsdk/HCNetSDK",        // 类名
        "getInstance",                          // 方法
        "()Lcom/hikvision/netsdk/HCNetSDK;");   // JNI 签名
    if (!hcNetSDKInstance.isValid()) {
        qWarning() << "[Qt][Error] Failed to get HCNetSDK instance";
        return false;
    }
    qDebug() << "[Qt] ✅ HCNetSDK instance obtained";

    // 5️⃣ 调用 NET_DVR_Init 初始化 SDK
    jboolean initResult = hcNetSDKInstance.callMethod<jboolean>("NET_DVR_Init");
    qDebug() << "[Qt] NET_DVR_Init result:" << (initResult ? "true" : "false");
    return initResult;
}

bool AndroidHCNetSDKInterface::hcNetSDKcleanUp()    //一次创建对应一次销毁
{
    // 4️⃣ 调用 NET_DVR_Cleanup() 释放 SDK 资源
    jboolean cleanupResult = hcNetSDKInstance.callMethod<jboolean>(
        "NET_DVR_Cleanup",
        "()Z"
        );

    if (cleanupResult) {
        qDebug() << "[Qt] ✅ NET_DVR_Cleanup success";
    } else {
        qWarning() << "[Qt][Error] ❌ NET_DVR_Cleanup failed";
    }
    return cleanupResult;
}


int AndroidHCNetSDKInterface::hcNetSDKLoginV40()    //一次创建对应一次销毁
{

   // 13️⃣ 创建 NET_DVR_DEVICEINFO_V40 对象
    QJniObject deviceInfo("org/hcnetsdk/jna/HCNetSDKByJNA$NET_DVR_DEVICEINFO_V40");
    if (!deviceInfo.isValid()) {
        qWarning() << "[Qt][Error] Failed to create NET_DVR_DEVICEINFO_V40 object";
        return false;
    }
    qDebug() << "[Qt] ✅ Created NET_DVR_DEVICEINFO_V40 object";

    // 16️⃣ 调用 Wrapper 的静态方法 login，实现 NET_DVR_Login_V40 登录
    userID = QJniObject::callStaticMethod<jint>(
        "org/hcnetsdk/jna/HCNetSDKJNAWrapper",
        "login",
        "(Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;Lorg/hcnetsdk/jna/HCNetSDKByJNA$NET_DVR_DEVICEINFO_V40;)I",
        QJniObject::fromString("192.168.1.65").object<jstring>(),
        8000,
        QJniObject::fromString("admin").object<jstring>(),
        QJniObject::fromString("Hik12345").object<jstring>(),
        deviceInfo.object<jobject>()
        );


    // 1️⃣ 获取内部字段 struDeviceV30 （类型是另一个 Java 对象）
    QJniObject struDeviceV30 = deviceInfo.getObjectField(
        "struDeviceV30",  // 字段名
        "Lorg/hcnetsdk/jna/HCNetSDKByJNA$NET_DVR_DEVICEINFO_V30;" // Java 类型签名
        );

    // 检查是否成功
    if (!struDeviceV30.isValid()) {
        qWarning() << "[Qt][Error] Failed to get struDeviceV30";
        return false;
    }

    // ✅ 获取 Java 层 byte[] 对象
    QJniObject serialNumberArray = struDeviceV30.getObjectField(
        "sSerialNumber",
        "[B" // 表示 byte[]
        );

    if (!serialNumberArray.isValid()) {
        qWarning() << "[Qt][Error] Failed to get sSerialNumber field";
        return false;
    }

    // ✅ 使用 QJniEnvironment 操作原始 JNI 数据,结构体中的数据由于是从java转为c++需要进行转换
    QJniEnvironment env;
    jbyteArray byteArray = serialNumberArray.object<jbyteArray>();
    jsize length = env->GetArrayLength(byteArray);
    QByteArray serialBytes(length, 0);

    env->GetByteArrayRegion(byteArray, 0, length, reinterpret_cast<jbyte*>(serialBytes.data()));
    QString serialStr = QString::fromLatin1(serialBytes).trimmed();

    qDebug() << "📟 Device Serial Number =" << serialStr;
    // 获取 short 类型字段 wDevType
    jshort devType = struDeviceV30.getField<jshort>("wDevType");
    qDebug() << "📱 Device Type (wDevType) =" << devType;

            // 2️⃣ 从 struDeviceV30 中获取 byte 字段 byChanNum
    jbyte byChanNum = struDeviceV30.getField<jbyte>("byChanNum");
    jbyte byIPChanNum = struDeviceV30.getField<jbyte>("byIPChanNum");
    //jbyte byHighDChanNum =  struDeviceV30.getField<jbyte>("byHighDChanNum");
    // 输出


    if (userID >= 0) {

        qDebug() << "[Qt] ✅ Login success, userID =" << userID;
        qDebug() << "[Qt] ✅ The max number of analog channels =" << static_cast<int>(byChanNum);
        qDebug() << "[Qt] ✅ The max number of IP channels:  =" << static_cast<int>(byIPChanNum) + static_cast<int>(byIPChanNum) * 256;

    } else {
        qWarning() << "[Qt][Error] Login failed, returned ID =" << userID;
    }
    qDebug() << "==============================\n";
    return userID;
}

bool AndroidHCNetSDKInterface::hcNetSDKLoginV40(AndroidHCNetSDKInterface::HCNetDevice device)    //一次创建对应一次销毁
{

    // 13️⃣ 创建 NET_DVR_DEVICEINFO_V40 对象
    QJniObject deviceInfo("org/hcnetsdk/jna/HCNetSDKByJNA$NET_DVR_DEVICEINFO_V40");
    if (!deviceInfo.isValid()) {
        qWarning() << "[Qt][Error] Failed to create NET_DVR_DEVICEINFO_V40 object";
        return false;
    }
    qDebug() << "[Qt] ✅ Created NET_DVR_DEVICEINFO_V40 object";

            // 16️⃣ 调用 Wrapper 的静态方法 login，实现 NET_DVR_Login_V40 登录
    device.userID = QJniObject::callStaticMethod<jint>(
        "org/hcnetsdk/jna/HCNetSDKJNAWrapper",
        "login",
        "(Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;Lorg/hcnetsdk/jna/HCNetSDKByJNA$NET_DVR_DEVICEINFO_V40;)I",
        QJniObject::fromString(device.DeviceAddress).object<jstring>(),
        8000,
        QJniObject::fromString("admin").object<jstring>(),
        QJniObject::fromString("Hik12345").object<jstring>(),
        deviceInfo.object<jobject>()
        );


    // 1️⃣ 获取内部字段 struDeviceV30 （类型是另一个 Java 对象）
    device.struDeviceV30 = deviceInfo.getObjectField(
        "struDeviceV30",  // 字段名
        "Lorg/hcnetsdk/jna/HCNetSDKByJNA$NET_DVR_DEVICEINFO_V30;" // Java 类型签名
        );

            // 检查是否成功
    if (!device.struDeviceV30.isValid()) {
        qWarning() << "[Qt][Error] Failed to get struDeviceV30";
        return false;
    }

    device.Channel = device.struDeviceV30.getField<jbyte>("byIPChanNum");


    if (device.userID >= 0) {

        qDebug() << "[Qt] ✅ Login success, userID =" << device.userID;
        // qDebug() << "[Qt] ✅ The max number of analog channels =" << static_cast<int>(byChanNum);
        // qDebug() << "[Qt] ✅ The max number of IP channels:  =" << static_cast<int>(byIPChanNum) + static_cast<int>(byIPChanNum) * 256;
        m_userIDCtrl = device.userID;
        m_channelCtrl = device.Channel;
    } else {
        qWarning() << "[Qt][Error] Login failed, returned ID =" << device.userID;
    }
    qDebug() << "==============================\n";
    return true;
}

bool AndroidHCNetSDKInterface::hcNetSDKLoginV40Device1(QString ipaddress)    //一次创建对应一次销毁
{

    // 13️⃣ 创建 NET_DVR_DEVICEINFO_V40 对象
    QJniObject deviceInfo("org/hcnetsdk/jna/HCNetSDKByJNA$NET_DVR_DEVICEINFO_V40");
    if (!deviceInfo.isValid()) {
        qWarning() << "[Qt][Error] Failed to create NET_DVR_DEVICEINFO_V40 object";
        return false;
    }
    hcNetDevice1.DeviceAddress = ipaddress;
    qDebug() << "[Qt] ✅ Created NET_DVR_DEVICEINFO_V40 object";
    qDebug() << "[Qt] ✅ Created NET_DVR_DEVICEINFO_V40 object:" << hcNetDevice1.DeviceAddress;


            // 16️⃣ 调用 Wrapper 的静态方法 login，实现 NET_DVR_Login_V40 登录
    hcNetDevice1.userID = QJniObject::callStaticMethod<jint>(
        "org/hcnetsdk/jna/HCNetSDKJNAWrapper",
        "login",
        "(Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;Lorg/hcnetsdk/jna/HCNetSDKByJNA$NET_DVR_DEVICEINFO_V40;)I",
        QJniObject::fromString(QString("192.168.10.")+hcNetDevice1.DeviceAddress).object<jstring>(),
        8000,
        QJniObject::fromString("admin").object<jstring>(),
        QJniObject::fromString("Hik12345").object<jstring>(),
        deviceInfo.object<jobject>()
        );


            // 1️⃣ 获取内部字段 struDeviceV30 （类型是另一个 Java 对象）
    hcNetDevice1.struDeviceV30 = deviceInfo.getObjectField(
        "struDeviceV30",  // 字段名
        "Lorg/hcnetsdk/jna/HCNetSDKByJNA$NET_DVR_DEVICEINFO_V30;" // Java 类型签名
        );

            // 检查是否成功
    if (!hcNetDevice1.struDeviceV30.isValid()) {
        qWarning() << "[Qt][Error] Failed to get struDeviceV30";
        return false;
    }

    hcNetDevice1.Channel = hcNetDevice1.struDeviceV30.getField<jbyte>("byChanNum");


    if (hcNetDevice1.userID >= 0) {

        qDebug() << "[Qt] ✅ Login success, userID =" << hcNetDevice1.userID;
        // qDebug() << "[Qt] ✅ The max number of analog channels =" << static_cast<int>(byChanNum);
        // qDebug() << "[Qt] ✅ The max number of IP channels:  =" << static_cast<int>(byIPChanNum) + static_cast<int>(byIPChanNum) * 256;
        m_userIDCtrl = hcNetDevice1.userID;
        m_channelCtrl = hcNetDevice1.Channel;
        qDebug() << "[Qt] userIDCtrl =" << m_userIDCtrl;
        qDebug() << "[Qt] userIDCtrl =" << m_channelCtrl;
    } else {
        qWarning() << "[Qt][Error] Login failed, returned ID =" << hcNetDevice1.userID;
    }
    qDebug() << "==============================\n";
    return true;
}

bool AndroidHCNetSDKInterface::hcNetSDKLoginV40Device2(QString ipaddress)    //一次创建对应一次销毁
{

    // 13️⃣ 创建 NET_DVR_DEVICEINFO_V40 对象
    QJniObject deviceInfo("org/hcnetsdk/jna/HCNetSDKByJNA$NET_DVR_DEVICEINFO_V40");
    if (!deviceInfo.isValid()) {
        qWarning() << "[Qt][Error] Failed to create NET_DVR_DEVICEINFO_V40 object";
        return false;
    }
    hcNetDevice2.DeviceAddress = ipaddress;
    qDebug() << "[Qt] ✅ Created NET_DVR_DEVICEINFO_V40 object";
    qDebug() << "[Qt] ✅ Created NET_DVR_DEVICEINFO_V40 object:" << hcNetDevice2.DeviceAddress;
            // 16️⃣ 调用 Wrapper 的静态方法 login，实现 NET_DVR_Login_V40 登录
    hcNetDevice2.userID = QJniObject::callStaticMethod<jint>(
        "org/hcnetsdk/jna/HCNetSDKJNAWrapper",
        "login",
        "(Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;Lorg/hcnetsdk/jna/HCNetSDKByJNA$NET_DVR_DEVICEINFO_V40;)I",
        QJniObject::fromString(QString("192.168.10.")+hcNetDevice2.DeviceAddress).object<jstring>(),
        8000,
        QJniObject::fromString("admin").object<jstring>(),
        QJniObject::fromString("Hik12345").object<jstring>(),
        deviceInfo.object<jobject>()
        );


            // 1️⃣ 获取内部字段 struDeviceV30 （类型是另一个 Java 对象）
    hcNetDevice2.struDeviceV30 = deviceInfo.getObjectField(
        "struDeviceV30",  // 字段名
        "Lorg/hcnetsdk/jna/HCNetSDKByJNA$NET_DVR_DEVICEINFO_V30;" // Java 类型签名
        );

            // 检查是否成功
    if (!hcNetDevice2.struDeviceV30.isValid()) {
        qWarning() << "[Qt][Error] Failed to get struDeviceV30";
        return false;
    }

    hcNetDevice2.Channel = hcNetDevice2.struDeviceV30.getField<jbyte>("byChanNum");


    if (hcNetDevice2.userID >= 0) {

        qDebug() << "[Qt] ✅ Login success, userID =" << hcNetDevice2.userID;
        // qDebug() << "[Qt] ✅ The max number of analog channels =" << static_cast<int>(byChanNum);
        // qDebug() << "[Qt] ✅ The max number of IP channels:  =" << static_cast<int>(byIPChanNum) + static_cast<int>(byIPChanNum) * 256;
        m_userIDCtrl = hcNetDevice2.userID;
        m_channelCtrl = hcNetDevice2.Channel;
        qDebug() << "[Qt] userIDCtrl =" << m_userIDCtrl;
        //qDebug() << "[Qt] ✅ The max number of analog channels =" << static_cast<int>(byChanNum);
        //qDebug() << "[Qt] ✅ The max number of IP channels:  =" << static_cast<int>(byIPChanNum) + static_cast<int>(byIPChanNum) * 256;
        qDebug() << "[Qt] m_channelCtrl =" << m_channelCtrl;
    } else {
        qWarning() << "[Qt][Error] Login failed, returned ID =" << hcNetDevice2.userID;
    }
    qDebug() << "==============================\n";
    return true;
}
bool AndroidHCNetSDKInterface::hcNetSDKLoginV40Device3(QString ipaddress)    //一次创建对应一次销毁
{

    // 13️⃣ 创建 NET_DVR_DEVICEINFO_V40 对象
    QJniObject deviceInfo("org/hcnetsdk/jna/HCNetSDKByJNA$NET_DVR_DEVICEINFO_V40");
    if (!deviceInfo.isValid()) {
        qWarning() << "[Qt][Error] Failed to create NET_DVR_DEVICEINFO_V40 object";
        return false;
    }
    hcNetDevice3.DeviceAddress = ipaddress;
    qDebug() << "[Qt] ✅ Created NET_DVR_DEVICEINFO_V40 object";

            // 16️⃣ 调用 Wrapper 的静态方法 login，实现 NET_DVR_Login_V40 登录
    hcNetDevice3.userID = QJniObject::callStaticMethod<jint>(
        "org/hcnetsdk/jna/HCNetSDKJNAWrapper",
        "login",
        "(Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;Lorg/hcnetsdk/jna/HCNetSDKByJNA$NET_DVR_DEVICEINFO_V40;)I",
        QJniObject::fromString(QString("192.168.10.")+hcNetDevice3.DeviceAddress).object<jstring>(),
        8000,
        QJniObject::fromString("admin").object<jstring>(),
        QJniObject::fromString("Hik12345").object<jstring>(),
        deviceInfo.object<jobject>()
        );


            // 1️⃣ 获取内部字段 struDeviceV30 （类型是另一个 Java 对象）
    hcNetDevice3.struDeviceV30 = deviceInfo.getObjectField(
        "struDeviceV30",  // 字段名
        "Lorg/hcnetsdk/jna/HCNetSDKByJNA$NET_DVR_DEVICEINFO_V30;" // Java 类型签名
        );

            // 检查是否成功
    if (!hcNetDevice3.struDeviceV30.isValid()) {
        qWarning() << "[Qt][Error] Failed to get struDeviceV30";
        return false;
    }

    hcNetDevice3.Channel = hcNetDevice3.struDeviceV30.getField<jbyte>("byChanNum");


    if (hcNetDevice3.userID >= 0) {

        qDebug() << "[Qt] ✅ Login success, userID =" << hcNetDevice3.userID;
        // qDebug() << "[Qt] ✅ The max number of analog channels =" << static_cast<int>(byChanNum);
        // qDebug() << "[Qt] ✅ The max number of IP channels:  =" << static_cast<int>(byIPChanNum) + static_cast<int>(byIPChanNum) * 256;
        m_userIDCtrl = hcNetDevice3.userID;
        m_channelCtrl = hcNetDevice3.Channel;
    } else {
        qWarning() << "[Qt][Error] Login failed, returned ID =" << hcNetDevice3.userID;
    }
    qDebug() << "==============================\n";
    return true;
}
bool AndroidHCNetSDKInterface::hcNetSDKLoginV40Device4(QString ipaddress)    //一次创建对应一次销毁
{

    // 13️⃣ 创建 NET_DVR_DEVICEINFO_V40 对象
    QJniObject deviceInfo("org/hcnetsdk/jna/HCNetSDKByJNA$NET_DVR_DEVICEINFO_V40");
    if (!deviceInfo.isValid()) {
        qWarning() << "[Qt][Error] Failed to create NET_DVR_DEVICEINFO_V40 object";
        return false;
    }
    hcNetDevice4.DeviceAddress = ipaddress;
    qDebug() << "[Qt] ✅ Created NET_DVR_DEVICEINFO_V40 object";

            // 16️⃣ 调用 Wrapper 的静态方法 login，实现 NET_DVR_Login_V40 登录
    hcNetDevice4.userID = QJniObject::callStaticMethod<jint>(
        "org/hcnetsdk/jna/HCNetSDKJNAWrapper",
        "login",
        "(Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;Lorg/hcnetsdk/jna/HCNetSDKByJNA$NET_DVR_DEVICEINFO_V40;)I",
        QJniObject::fromString(QString("192.168.10.")+hcNetDevice4.DeviceAddress).object<jstring>(),
        8000,
        QJniObject::fromString("admin").object<jstring>(),
        QJniObject::fromString("Hik12345").object<jstring>(),
        deviceInfo.object<jobject>()
        );


            // 1️⃣ 获取内部字段 struDeviceV30 （类型是另一个 Java 对象）
    hcNetDevice4.struDeviceV30 = deviceInfo.getObjectField(
        "struDeviceV30",  // 字段名
        "Lorg/hcnetsdk/jna/HCNetSDKByJNA$NET_DVR_DEVICEINFO_V30;" // Java 类型签名
        );

            // 检查是否成功
    if (!hcNetDevice4.struDeviceV30.isValid()) {
        qWarning() << "[Qt][Error] Failed to get struDeviceV30";
        return false;
    }

    hcNetDevice4.Channel = hcNetDevice4.struDeviceV30.getField<jbyte>("byChanNum");


    if (hcNetDevice4.userID >= 0) {

        qDebug() << "[Qt] ✅ Login success, userID =" << hcNetDevice4.userID;
        // qDebug() << "[Qt] ✅ The max number of analog channels =" << static_cast<int>(byChanNum);
        // qDebug() << "[Qt] ✅ The max number of IP channels:  =" << static_cast<int>(byIPChanNum) + static_cast<int>(byIPChanNum) * 256;
        m_userIDCtrl = hcNetDevice4.userID;
        m_channelCtrl = hcNetDevice4.Channel;
    } else {
        qWarning() << "[Qt][Error] Login failed, returned ID =" << hcNetDevice4.userID;
    }
    qDebug() << "==============================\n";
    return true;
}
bool AndroidHCNetSDKInterface::hcNetSDKLoginV40Device5(QString ipaddress)    //一次创建对应一次销毁
{

    // 13️⃣ 创建 NET_DVR_DEVICEINFO_V40 对象
    QJniObject deviceInfo("org/hcnetsdk/jna/HCNetSDKByJNA$NET_DVR_DEVICEINFO_V40");
    if (!deviceInfo.isValid()) {
        qWarning() << "[Qt][Error] Failed to create NET_DVR_DEVICEINFO_V40 object";
        return false;
    }
    hcNetDevice5.DeviceAddress = ipaddress;
    qDebug() << "[Qt] ✅ Created NET_DVR_DEVICEINFO_V40 object";

            // 16️⃣ 调用 Wrapper 的静态方法 login，实现 NET_DVR_Login_V40 登录
    hcNetDevice5.userID = QJniObject::callStaticMethod<jint>(
        "org/hcnetsdk/jna/HCNetSDKJNAWrapper",
        "login",
        "(Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;Lorg/hcnetsdk/jna/HCNetSDKByJNA$NET_DVR_DEVICEINFO_V40;)I",
        QJniObject::fromString(QString("192.168.10.")+hcNetDevice5.DeviceAddress).object<jstring>(),
        8000,
        QJniObject::fromString("admin").object<jstring>(),
        QJniObject::fromString("Hik12345").object<jstring>(),
        deviceInfo.object<jobject>()
        );


            // 1️⃣ 获取内部字段 struDeviceV30 （类型是另一个 Java 对象）
    hcNetDevice5.struDeviceV30 = deviceInfo.getObjectField(
        "struDeviceV30",  // 字段名
        "Lorg/hcnetsdk/jna/HCNetSDKByJNA$NET_DVR_DEVICEINFO_V30;" // Java 类型签名
        );

            // 检查是否成功
    if (!hcNetDevice5.struDeviceV30.isValid()) {
        qWarning() << "[Qt][Error] Failed to get struDeviceV30";
        return false;
    }

    hcNetDevice5.Channel = hcNetDevice5.struDeviceV30.getField<jbyte>("byChanNum");


    if (hcNetDevice5.userID >= 0) {

        qDebug() << "[Qt] ✅ Login success, userID =" << hcNetDevice5.userID;
        // qDebug() << "[Qt] ✅ The max number of analog channels =" << static_cast<int>(byChanNum);
        // qDebug() << "[Qt] ✅ The max number of IP channels:  =" << static_cast<int>(byIPChanNum) + static_cast<int>(byIPChanNum) * 256;
        m_userIDCtrl = hcNetDevice5.userID;
        m_channelCtrl = hcNetDevice5.Channel;
    } else {
        qWarning() << "[Qt][Error] Login failed, returned ID =" << hcNetDevice5.userID;
    }
    qDebug() << "==============================\n";
    return true;
}

bool AndroidHCNetSDKInterface::hcNetSDKLogoutV30(int userID)    //一次创建对应一次销毁
{
    qDebug() << "\n==============================";
    qDebug() << "[Qt] logoutDevice() start";

            // 1️⃣ 检查 hcNetSDKInstance 是否有效
    if (!hcNetSDKInstance.isValid()) {
        qWarning() << "[Qt][Error] HCNetSDK instance invalid, cannot logout.";
        return false;
    }

            // 2️⃣ 检查 userID 是否有效
    if (userID < 0) {
        qWarning() << "[Qt][Warning] Invalid userID, maybe already logged out.";
    } else {
        // 3️⃣ 调用 NET_DVR_Logout_V30(userID)
        jboolean logoutResult = hcNetSDKInstance.callMethod<jboolean>(
            "NET_DVR_Logout_V30",
            "(I)Z",
            userID
            );

        if (logoutResult) {
            qDebug() << "[Qt] ✅ NET_DVR_Logout_V30 success";
        } else {
            qWarning() << "[Qt][Error] ❌ NET_DVR_Logout_V30 failed";
        }

                // 清空 userID
        userID = -1;
    }
    return true;
}

bool AndroidHCNetSDKInterface::hcNetSDKLogoutV30(AndroidHCNetSDKInterface::HCNetDevice device)    //一次创建对应一次销毁
{
    qDebug() << "\n==============================";
    qDebug() << "[Qt] logoutDevice() start";

            // 1️⃣ 检查 hcNetSDKInstance 是否有效
    if (!hcNetSDKInstance.isValid()) {
        qWarning() << "[Qt][Error] HCNetSDK instance invalid, cannot logout.";
        return false;
    }

            // 2️⃣ 检查 userID 是否有效
    if (device.userID < 0) {
        qWarning() << "[Qt][Warning] Invalid userID, maybe already logged out.";
    } else {
        // 3️⃣ 调用 NET_DVR_Logout_V30(userID)
        jboolean logoutResult = hcNetSDKInstance.callMethod<jboolean>(
            "NET_DVR_Logout_V30",
            "(I)Z",
            device.userID
            );

        if (logoutResult) {
            qDebug() << "[Qt] ✅ NET_DVR_Logout_V30 success";
        } else {
            qWarning() << "[Qt][Error] ❌ NET_DVR_Logout_V30 failed";
        }

                // 清空 userID
        device.userID = -1;
    }
    return true;
}

bool AndroidHCNetSDKInterface::hcNetSDKLogoutV30_all()    //一次创建对应一次销毁
{
    qDebug() << "\n==============================";
    qDebug() << "[Qt] logoutDevice() start";

            // 1️⃣ 检查 hcNetSDKInstance 是否有效
    if (!hcNetSDKInstance.isValid()) {
        qWarning() << "[Qt][Error] HCNetSDK instance invalid, cannot logout.";
        return false;
    }

            // 2️⃣ 检查 userID 是否有效
    if (hcNetDevice1.userID < 0) {
        qWarning() << "[Qt][Warning] Invalid userID, maybe already logged out.";
    } else {
        // 3️⃣ 调用 NET_DVR_Logout_V30(userID)
        jboolean logoutResult = hcNetSDKInstance.callMethod<jboolean>(
            "NET_DVR_Logout_V30",
            "(I)Z",
            hcNetDevice1.userID
            );

        if (logoutResult) {
            qDebug() << "[Qt] ✅ NET_DVR_Logout_V30 success";
        } else {
            qWarning() << "[Qt][Error] ❌ NET_DVR_Logout_V30 failed";
        }

                // 清空 userID
        hcNetDevice1.userID = -1;
        hcNetDevice1.Channel = -1;
    }
    if (hcNetDevice2.userID < 0) {
        qWarning() << "[Qt][Warning] Invalid userID, maybe already logged out.";
    } else {
        // 3️⃣ 调用 NET_DVR_Logout_V30(userID)
        jboolean logoutResult = hcNetSDKInstance.callMethod<jboolean>(
            "NET_DVR_Logout_V30",
            "(I)Z",
            hcNetDevice2.userID
            );

        if (logoutResult) {
            qDebug() << "[Qt] ✅ NET_DVR_Logout_V30 success";
        } else {
            qWarning() << "[Qt][Error] ❌ NET_DVR_Logout_V30 failed";
        }

                // 清空 userID
        hcNetDevice2.userID = -1;
        hcNetDevice2.Channel = -1;
    }
    if (hcNetDevice3.userID < 0) {
        qWarning() << "[Qt][Warning] Invalid userID, maybe already logged out.";
    } else {
        // 3️⃣ 调用 NET_DVR_Logout_V30(userID)
        jboolean logoutResult = hcNetSDKInstance.callMethod<jboolean>(
            "NET_DVR_Logout_V30",
            "(I)Z",
            hcNetDevice3.userID
            );

        if (logoutResult) {
            qDebug() << "[Qt] ✅ NET_DVR_Logout_V30 success";
        } else {
            qWarning() << "[Qt][Error] ❌ NET_DVR_Logout_V30 failed";
        }

                // 清空 userID
        hcNetDevice3.userID = -1;
        hcNetDevice3.Channel = -1;
    }
    if (hcNetDevice4.userID < 0) {
        qWarning() << "[Qt][Warning] Invalid userID, maybe already logged out.";
    } else {
        // 3️⃣ 调用 NET_DVR_Logout_V30(userID)
        jboolean logoutResult = hcNetSDKInstance.callMethod<jboolean>(
            "NET_DVR_Logout_V30",
            "(I)Z",
            hcNetDevice4.userID
            );

        if (logoutResult) {
            qDebug() << "[Qt] ✅ NET_DVR_Logout_V30 success";
        } else {
            qWarning() << "[Qt][Error] ❌ NET_DVR_Logout_V30 failed";
        }

                // 清空 userID
        hcNetDevice4.userID = -1;
        hcNetDevice4.Channel = -1;
    }
    if (hcNetDevice5.userID < 0) {
        qWarning() << "[Qt][Warning] Invalid userID, maybe already logged out.";
    } else {
        // 3️⃣ 调用 NET_DVR_Logout_V30(userID)
        jboolean logoutResult = hcNetSDKInstance.callMethod<jboolean>(
            "NET_DVR_Logout_V30",
            "(I)Z",
            hcNetDevice5.userID
            );

        if (logoutResult) {
            qDebug() << "[Qt] ✅ NET_DVR_Logout_V30 success";
        } else {
            qWarning() << "[Qt][Error] ❌ NET_DVR_Logout_V30 failed";
        }

                // 清空 userID
        hcNetDevice5.userID = -1;
        hcNetDevice5.Channel = -1;
    }
    return true;
}

bool AndroidHCNetSDKInterface::hcNetSDKPTZControlOther(jint lUserID,int dwPTZCommand,int dwSTOP){
    if (lUserID < 0 || !hcNetSDKInstance.isValid()) {
        qWarning() << "[Qt][Error] SDK not initialized or not logged in!";
        return false;
    }

    jboolean result = hcNetSDKInstance.callMethod<jboolean>(
        "NET_DVR_PTZControl_Other",
        "(IIII)Z",
        lUserID,
        1,              // 通道号，一般为 1
        dwPTZCommand,   // 云台命令
        dwSTOP          // 0 = 开始, 1 = 停止
        );

    qDebug() << QString("[Qt] PTZ command=%1, stop=%2, result=%3")
                    .arg(dwPTZCommand)
                    .arg(dwSTOP)
                    .arg(result ? "true" : "false");
    return true;
}

bool AndroidHCNetSDKInterface::hcNetSDKPTZControlOther(int userID, int channel,int dwPTZCommand,int dwSTOP){
    if (userID < 0 || !hcNetSDKInstance.isValid() || channel < 0) {
        qWarning() << "[Qt][Error] SDK not initialized or not logged in!";
        return false;
    }
    qDebug() << QString("[Qt] Input parameters -> userID: %1, channel: %2, command: %3, stop: %4")
                    .arg(userID)
                    .arg(channel)
                    .arg(dwPTZCommand)
                    .arg(dwSTOP);
    // ✅ 显式转换为 JNI 类型
    jint jUserID = static_cast<jint>(userID);
    jint jChannel = static_cast<jint>(channel);
    // 打印转换后的 JNI 参数
    qDebug() << QString("[Qt] JNI parameters -> jUserID: %1, jChannel: %2")
                    .arg(jUserID)
                    .arg(jChannel)
                    ;

    jboolean result = hcNetSDKInstance.callMethod<jboolean>(
        "NET_DVR_PTZControl_Other",
        "(IIII)Z",
        jUserID,
        jChannel,              // 通道号，一般为 1
        dwPTZCommand,   // 云台命令
        dwSTOP          // 0 = 开始, 1 = 停止
        );

    qDebug() << QString("[Qt] PTZ command=%1, stop=%2, result=%3")
                    .arg(dwPTZCommand)
                    .arg(dwSTOP)
                    .arg(result ? "true" : "false");
    return true;
}


bool AndroidHCNetSDKInterface::hcNetSDKPTZControlOther(AndroidHCNetSDKInterface::HCNetDevice device,int dwPTZCommand,int dwSTOP){
    if (device.userID < 0 || !hcNetSDKInstance.isValid() || device.Channel < 0) {
        qWarning() << "[Qt][Error] SDK not initialized or not logged in!";
        return false;
    }

    jboolean result = hcNetSDKInstance.callMethod<jboolean>(
        "NET_DVR_PTZControl_Other",
        "(IIII)Z",
        device.userID,
        device.Channel,              // 通道号，一般为 1
        dwPTZCommand,   // 云台命令
        dwSTOP          // 0 = 开始, 1 = 停止
        );

    qDebug() << QString("[Qt] PTZ command=%1, stop=%2, result=%3")
                    .arg(dwPTZCommand)
                    .arg(dwSTOP)
                    .arg(result ? "true" : "false");
    return true;
}






void AndroidHCNetSDKInterface::loginToDevice()
{
    qDebug() << "\n==============================";
    qDebug() << "[Qt] loginToDevice() start";

            // 3️⃣ 调用 getInstance() 获得 HCNetSDK 单例对象
    hcNetSDKInstance = QJniObject::callStaticObjectMethod(  //静态对象
        "com/hikvision/netsdk/HCNetSDK",        // 类名
        "getInstance",                          // 方法
        "()Lcom/hikvision/netsdk/HCNetSDK;");   // JNI 签名
    if (!hcNetSDKInstance.isValid()) {
        qWarning() << "[Qt][Error] Failed to get HCNetSDK instance";
        return;
    }
    qDebug() << "[Qt] ✅ HCNetSDK instance obtained";

    // 5️⃣ 调用 NET_DVR_Init 初始化 SDK
    jboolean initResult = hcNetSDKInstance.callMethod<jboolean>("NET_DVR_Init");
    qDebug() << "[Qt] NET_DVR_Init result:" << (initResult ? "true" : "false");


            // 13️⃣ 创建 NET_DVR_DEVICEINFO_V40 对象
    QJniObject deviceInfo("org/hcnetsdk/jna/HCNetSDKByJNA$NET_DVR_DEVICEINFO_V40");
    if (!deviceInfo.isValid()) {
        qWarning() << "[Qt][Error] Failed to create NET_DVR_DEVICEINFO_V40 object";
        return;
    }
    qDebug() << "[Qt] ✅ Created NET_DVR_DEVICEINFO_V40 object";


            // 16️⃣ 调用 Wrapper 的静态方法 login，实现 NET_DVR_Login_V40 登录
    userID = QJniObject::callStaticMethod<jint>(
        "org/hcnetsdk/jna/HCNetSDKJNAWrapper",
        "login",
        "(Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;Lorg/hcnetsdk/jna/HCNetSDKByJNA$NET_DVR_DEVICEINFO_V40;)I",
        QJniObject::fromString("192.168.1.64").object<jstring>(),
        8000,
        QJniObject::fromString("admin").object<jstring>(),
        QJniObject::fromString("Hik12345").object<jstring>(),
        deviceInfo.object<jobject>()
        );


            // 1️⃣ 获取内部字段 struDeviceV30 （类型是另一个 Java 对象）
    QJniObject struDeviceV30 = deviceInfo.getObjectField(
        "struDeviceV30",  // 字段名
        "Lorg/hcnetsdk/jna/HCNetSDKByJNA$NET_DVR_DEVICEINFO_V30;" // Java 类型签名
        );

            // 检查是否成功
    if (!struDeviceV30.isValid()) {
        qWarning() << "[Qt][Error] Failed to get struDeviceV30";
        return;
    }

            // ✅ 获取 Java 层 byte[] 对象
    QJniObject serialNumberArray = struDeviceV30.getObjectField(
        "sSerialNumber",
        "[B" // 表示 byte[]
        );

    if (!serialNumberArray.isValid()) {
        qWarning() << "[Qt][Error] Failed to get sSerialNumber field";
        return;
    }

            // ✅ 使用 QJniEnvironment 操作原始 JNI 数据
    QJniEnvironment env;
    jbyteArray byteArray = serialNumberArray.object<jbyteArray>();
    jsize length = env->GetArrayLength(byteArray);
    QByteArray serialBytes(length, 0);

    env->GetByteArrayRegion(byteArray, 0, length, reinterpret_cast<jbyte*>(serialBytes.data()));
    QString serialStr = QString::fromLatin1(serialBytes).trimmed();

    qDebug() << "📟 Device Serial Number =" << serialStr;
    // 获取 short 类型字段 wDevType
    jshort devType = struDeviceV30.getField<jshort>("wDevType");
    qDebug() << "📱 Device Type (wDevType) =" << devType;

            // 2️⃣ 从 struDeviceV30 中获取 byte 字段 byChanNum
    jbyte byChanNum = struDeviceV30.getField<jbyte>("byChanNum");
    jbyte byIPChanNum = struDeviceV30.getField<jbyte>("byIPChanNum");
    //jbyte byHighDChanNum =  struDeviceV30.getField<jbyte>("byHighDChanNum");
    // 输出


    if (userID >= 0) {



        qDebug() << "[Qt] ✅ Login success, userID =" << userID;
        qDebug() << "[Qt] ✅ The max number of analog channels =" << static_cast<int>(byChanNum);
        qDebug() << "[Qt] ✅ The max number of IP channels:  =" << static_cast<int>(byIPChanNum) + static_cast<int>(byIPChanNum) * 256;

    } else {
        qWarning() << "[Qt][Error] Login failed, returned ID =" << userID;
    }

    qDebug() << "==============================\n";

}

void AndroidHCNetSDKInterface::dvrPTZControlDevice(int dwPTZCommand, int dwSTOP)
{
    if (userID < 0 || !hcNetSDKInstance.isValid()) {
        qWarning() << "[Qt][Error] SDK not initialized or not logged in!";
        return;
    }

    jboolean result = hcNetSDKInstance.callMethod<jboolean>(
        "NET_DVR_PTZControl_Other",
        "(IIII)Z",
        userID,
        1,              // 通道号，一般为 1
        dwPTZCommand,   // 云台命令
        dwSTOP          // 0 = 开始, 1 = 停止
        );

    qDebug() << QString("[Qt] PTZ command=%1, stop=%2, result=%3")
                    .arg(dwPTZCommand)
                    .arg(dwSTOP)
                    .arg(result ? "true" : "false");
}

void AndroidHCNetSDKInterface::loginOutDevice()
{
    qDebug() << "\n==============================";
    qDebug() << "[Qt] logoutDevice() start";

            // 1️⃣ 检查 hcNetSDKInstance 是否有效
    if (!hcNetSDKInstance.isValid()) {
        qWarning() << "[Qt][Error] HCNetSDK instance invalid, cannot logout.";
        return;
    }

            // 2️⃣ 检查 userID 是否有效
    if (userID < 0) {
        qWarning() << "[Qt][Warning] Invalid userID, maybe already logged out.";
    } else {
        // 3️⃣ 调用 NET_DVR_Logout_V30(userID)
        jboolean logoutResult = hcNetSDKInstance.callMethod<jboolean>(
            "NET_DVR_Logout_V30",
            "(I)Z",
            userID
            );

        if (logoutResult) {
            qDebug() << "[Qt] ✅ NET_DVR_Logout_V30 success";
        } else {
            qWarning() << "[Qt][Error] ❌ NET_DVR_Logout_V30 failed";
        }

                // 清空 userID
        userID = -1;
    }

            // 4️⃣ 调用 NET_DVR_Cleanup() 释放 SDK 资源
    jboolean cleanupResult = hcNetSDKInstance.callMethod<jboolean>(
        "NET_DVR_Cleanup",
        "()Z"
        );

    if (cleanupResult) {
        qDebug() << "[Qt] ✅ NET_DVR_Cleanup success";
    } else {
        qWarning() << "[Qt][Error] ❌ NET_DVR_Cleanup failed";
    }

    qDebug() << "[Qt] logoutDevice() done";
    qDebug() << "==============================\n";
}


