package com.linyou.xserver;


import android.content.Context;

import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.ArrayList;
import java.util.Enumeration;

public class XServer {
    // Used to load the 'speaker' library on application startup.
    static {
        System.loadLibrary("XServer");
    }
    public static String GetLocalIPs() {
        ArrayList<String> list = new ArrayList<String>();
        try {
            Enumeration<NetworkInterface> en = NetworkInterface.getNetworkInterfaces();
            for (; en.hasMoreElements();) {
                NetworkInterface intf = en.nextElement();
                Enumeration<InetAddress> enumIpAddr = intf.getInetAddresses();
                for (; enumIpAddr.hasMoreElements();)
                {
                    InetAddress inetAddress = enumIpAddr.nextElement();
                    list.add(inetAddress.getHostAddress().toString());
                }
            }
        }
        catch (SocketException ex){
            ex.printStackTrace();
        }
        StringBuilder builder = new StringBuilder();
        for (int i = 0; i < list.size(); i++) {
            builder.append(list.get(i));
            builder.append("\n");
        }
        String result = builder.toString();
        return result;
    }

    static public native void MainJNI();
    static public native String StringFromJNI();
    public static native void NativeSetContext(final Context pContext);
}
