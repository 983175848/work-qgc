package org.hcnetsdk.jna;

public class HCNetSDKJNAWrapper {

    /**
     * 登录设备的静态方法包装
     * 对HCNetSDKJNAInstance做了再包装，初始化在程序周期内只需要做一次，后续返回的都是同一个实例，
     */
    public static int login(String ip, int port, String user, String pwd,
                            HCNetSDKByJNA.NET_DVR_DEVICEINFO_V40 deviceInfo) {

        // 构造登录信息
        HCNetSDKByJNA.NET_DVR_USER_LOGIN_INFO loginInfo =
                new HCNetSDKByJNA.NET_DVR_USER_LOGIN_INFO();
        System.arraycopy(ip.getBytes(), 0, loginInfo.sDeviceAddress, 0, ip.length());
        System.arraycopy(user.getBytes(), 0, loginInfo.sUserName, 0, user.length());
        System.arraycopy(pwd.getBytes(), 0, loginInfo.sPassword, 0, pwd.length());
        loginInfo.wPort = (short) port;

        loginInfo.write(); // 写入内存

        // ⚠️ 1️⃣ 调用 SDK 登录（native 层会修改 deviceInfo 指针指向的结构体内容）
        int userId = HCNetSDKJNAInstance.getInstance().NET_DVR_Login_V40(
                loginInfo.getPointer(),
                deviceInfo.getPointer()
        );

        // 登录成功后，刷新结构体内容
        deviceInfo.read();

        // 输出关键字段
        //System.out.println("设备通道数: " + deviceInfo.struDeviceV30.byChanNum);
        //System.out.println("设备序列号: " + new String(deviceInfo.struDeviceV30.sSerialNumber).trim());


        // 调用 JNA 实例方法
        return userId;

    }
}
