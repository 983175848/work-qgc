#pragma once

#include <QObject>
#include <QJniObject>
#include <QTimer>
#include <QtQml/qqml.h>

class AndroidHCNetSDKInterface : public QObject
{
    Q_OBJECT
    QML_ELEMENT               // ✅ 允许 QML 直接 import 使用
    QML_SINGLETON             // ✅ 注册为单例

    Q_PROPERTY(int userIDCtrl READ userIDCtrl WRITE setUserIDCtrl NOTIFY userIDCtrlChanged)
    Q_PROPERTY(int channelCtrl READ channelCtrl WRITE setChannelCtrl NOTIFY channelCtrlChanged)

public:
    explicit AndroidHCNetSDKInterface(QObject *parent = nullptr);

    // Application-wide singleton accessor (跟 VideoManager 的风格一致)
    static AndroidHCNetSDKInterface* instance();

    struct HCNetDevice{                 // 单设备结构体
        int userID = -1;                    // 登录句柄
        QString DeviceAddress;          // 设备地址 端口、用户名和密码都所有设备都相同，其他不输入值为默认值
        int Channel = -1;
        QJniObject struDeviceV30;       // 设备信息结构体,c++使用时需要转换

    }hcNetDevice1,hcNetDevice2,hcNetDevice3,hcNetDevice4,hcNetDevice5;

    Q_INVOKABLE bool hcNetSDKinit();        //在main.cc中管理
    Q_INVOKABLE bool hcNetSDKcleanUp();     //在main.cc中管理

    // Q_INVOKABLE bool hcNetSDKGetLastError();
    // Q_INVOKABLE bool hcNetSDKGetErrorMsg();

    Q_INVOKABLE int hcNetSDKLoginV40();
    Q_INVOKABLE bool hcNetSDKLoginV40(HCNetDevice device);
    Q_INVOKABLE bool hcNetSDKLoginV40Device1(QString ipaddress);
    Q_INVOKABLE bool hcNetSDKLoginV40Device2(QString ipaddress);
    Q_INVOKABLE bool hcNetSDKLoginV40Device3(QString ipaddress);
    Q_INVOKABLE bool hcNetSDKLoginV40Device4(QString ipaddress);
    Q_INVOKABLE bool hcNetSDKLoginV40Device5(QString ipaddress);

    Q_INVOKABLE bool hcNetSDKLogoutV30(int userID);
    Q_INVOKABLE bool hcNetSDKLogoutV30(HCNetDevice device);
    Q_INVOKABLE bool hcNetSDKLogoutV30_all();
    Q_INVOKABLE bool hcNetSDKPTZControlOther(jint userID,int dwPTZCommand,int dwSTOP);
    Q_INVOKABLE bool hcNetSDKPTZControlOther(int userID,int channel,int dwPTZCommand,int dwSTOP);
    Q_INVOKABLE bool hcNetSDKPTZControlOther(struct HCNetDevice,int dwPTZCommand,int dwSTOP);
    //Q_INVOKABLE bool hcNetSDKPTZControlOther(int userID,int dwPTZCommand,int dwSTOP);

    Q_INVOKABLE void loginToDevice();
    Q_INVOKABLE void loginOutDevice();
    Q_INVOKABLE void dvrPTZControlDevice(int dwPTZCommand,int dwSTOP);

//   signals:

//private:                             //目前为全局共享单例 不用私有变量


    int userID;                        // 保存登录句柄1  该用户ID具有唯一性，后续对设备的操作都需要通过此ID实现。
    int Channel;                        // 保存登录句柄1  该用户ID具有唯一性，后续对设备的操作都需要通过此ID实现。
    // int userIDCtrl;                        // 控制通道号  该用户ID具有唯一性，后续对设备的操作都需要通过此ID实现。
    // int channelCtrl;                        // 控制通道号  该用户ID具有唯一性，后续对设备的操作都需要通过此ID实现。
    int userID1;                       // 保存登录句柄1  该用户ID具有唯一性，后续对设备的操作都需要通过此ID实现。
    int userID2;                       // 保存登录句柄2  该用户ID具有唯一性，后续对设备的操作都需要通过此ID实现。
    int userID3;                       // 保存登录句柄3  该用户ID具有唯一性，后续对设备的操作都需要通过此ID实现。
    int userID4;                       // 保存登录句柄4  该用户ID具有唯一性，后续对设备的操作都需要通过此ID实现。
    int userID5;                       // 保存登录句柄5  该用户ID具有唯一性，后续对设备的操作都需要通过此ID实现。





    QJniObject hcNetSDKInstance;        // 保存 HCNetSDK 实例，即使多个摄像头也只需要一个实例,全局仅保留一个


    // getter / setter
    int userIDCtrl() const { return m_userIDCtrl; }
    void setUserIDCtrl(int id) {
        if (m_userIDCtrl != id) {
            m_userIDCtrl = id;
            emit userIDCtrlChanged(m_userIDCtrl); // 触发信号
        }
    }

    int channelCtrl() const { return m_channelCtrl; }
    void setChannelCtrl(int ch) {
        if (m_channelCtrl != ch) {
            m_channelCtrl = ch;
            emit channelCtrlChanged(m_channelCtrl); // 触发信号
        }
    }

signals:
    void userIDCtrlChanged(int newUserID);
    void channelCtrlChanged(int newChannel);

private:
    int m_userIDCtrl{-1};
    int m_channelCtrl{-1};

    //AndroidHCNetSDKInterface.userID = 5

};

