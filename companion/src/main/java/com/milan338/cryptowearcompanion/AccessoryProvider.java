package com.milan338.cryptowearcompanion;

import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.Signature;

import com.samsung.android.sdk.SsdkUnsupportedException;
import com.samsung.android.sdk.accessory.*;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;

public class AccessoryProvider extends SAAgentV2 {
    private static final String AGENT_NAME = "CryptoWearAccessoryProv";
    private static final Class<ServiceConnection> SOCKET_CLASS = ServiceConnection.class;
    private int maxDataSize = 0;
    private ServiceConnection mConnectionHandler = null;
    private Context mContext;

    public AccessoryProvider(Context context) {
        super(AGENT_NAME, context, SOCKET_CLASS);
        mContext = context;
        SA mAccessory = new SA();
        try {
            mAccessory.initialize(mContext);
        } catch (SsdkUnsupportedException e) {
            processUnsupportedException(e);
        } catch (Exception e) {
            e.printStackTrace();
            releaseAgent();
        }
    }

    @Override
    protected void onServiceConnectionRequested(SAPeerAgent peerAgent) {
        if (peerAgent != null) authenticatePeerAgent(peerAgent);
    }

    @Override
    protected void onAuthenticationResponse(SAPeerAgent peerAgent, SAAuthenticationToken authToken, int error) {
        if (authToken.getAuthenticationType() == SAAuthenticationToken.AUTHENTICATION_TYPE_CERTIFICATE_X509) {
            mContext = getApplicationContext();
            byte[] appKey = getApplicationCertificate(mContext);

            if (authToken.getKey() == null || authToken.getKey().length != appKey.length) return;

            for (int i = 0; i < authToken.getKey().length; i++) {
                if (authToken.getKey()[i] != appKey[i]) return;
            }

            // Token lengths and bytes match, so accept the connection request
            acceptServiceConnectionRequest(peerAgent);
        }
    }

    @Override
    protected void onServiceConnectionResponse(SAPeerAgent peerAgent, SASocket socket, int result) {
        if (socket == null || result != CONNECTION_SUCCESS) return;
        mConnectionHandler = (ServiceConnection) socket;
        maxDataSize = peerAgent.getMaxAllowedDataSize();
    }

    @Override
    protected void onError(SAPeerAgent peerAgent, String errorMessage, int errorCode) {
        super.onError(peerAgent, errorMessage, errorCode);
    }

    private static byte[] getApplicationCertificate(Context context) {
        if (context == null)  return null;

        try {
            PackageInfo pkgInfo = context
                .getPackageManager()
                .getPackageInfo(context.getPackageName(), PackageManager.GET_SIGNATURES);
            if (pkgInfo == null)  return null;

            Signature[] signatures = pkgInfo.signatures;
            if (signatures == null) return null;

            CertificateFactory certFactory = CertificateFactory.getInstance("X.509");
            ByteArrayInputStream bis = new ByteArrayInputStream(signatures[0].toByteArray());
            X509Certificate x509cert = (X509Certificate)certFactory.generateCertificate(bis);
            return x509cert.getPublicKey().getEncoded();
        } catch (PackageManager.NameNotFoundException e) {
            e.printStackTrace();
        } catch (CertificateException e) {
            e.printStackTrace();
        }

        return null;
    }

    // Securely send data to peer
    public void secureSendData(final String data) {
        if (data == null || mConnectionHandler == null || data.getBytes().length >= maxDataSize)
            return;

        final ServiceConnection handler = mConnectionHandler;
        new Thread(() -> {
            try {
                handler.secureSend(getServiceChannelId(0), data.getBytes());
            } catch (IOException e) {
                e.printStackTrace();
            }
        }).start();
    }

    private void processUnsupportedException(SsdkUnsupportedException e) {
        e.printStackTrace();

        String message = "";
        switch(e.getType()) {
            case(SsdkUnsupportedException.VENDOR_NOT_SUPPORTED):
            case(SsdkUnsupportedException.DEVICE_NOT_SUPPORTED):
                releaseAgent();
                message = "Device not supported";
                break;
            case(SsdkUnsupportedException.LIBRARY_NOT_INSTALLED):
                message = "Samsung SAP library is not installed";
                break;
            case(SsdkUnsupportedException.LIBRARY_UPDATE_IS_RECOMMENDED):
            case(SsdkUnsupportedException.LIBRARY_UPDATE_IS_REQUIRED):
                message = "Samsung SAP library is out-of-date";
                break;
        }

        Intent errorIntent = new Intent(mContext, ErrorDialogActivity.class);
        errorIntent.putExtra("title", "Error");
        errorIntent.putExtra("message", message);
        mContext.startActivity(errorIntent);
    }

    public class ServiceConnection extends SASocket {
        public ServiceConnection() {
            super(ServiceConnection.class.getName());
        }

        @Override
        public void onError(int channelId, String errorMessage, int errorCode) { }

        @Override
        public void onReceive(int channelId, byte[] data) {
            if (mConnectionHandler == null)
                return;
            String msg = new String(data);
            try {
                 JSONObject obj = new JSONObject(msg);
                 String requestType = obj.getString("requestType");
                 if (requestType.equals("getKeys")) updateKeys();
            } catch (JSONException e) {
                e.printStackTrace();
            }
        }

        @Override
        protected void onServiceConnectionLost(int reason) {
            mConnectionHandler = null;
        }

        private void updateKeys() {
            JSONObject keys = MainActivity.getKeys();
            secureSendData(keys.toString());
        }
    }
}
