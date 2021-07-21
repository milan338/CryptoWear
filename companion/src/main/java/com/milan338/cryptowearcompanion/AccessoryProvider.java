package com.milan338.cryptowearcompanion;

import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.Signature;
import android.widget.Toast;

import androidx.appcompat.app.AlertDialog;

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
    private MainActivity ui;

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
        if (peerAgent != null)
            // Begin authentication with peer agent
            authenticatePeerAgent(peerAgent);
    }

    @Override
    protected void onAuthenticationResponse(SAPeerAgent peerAgent, SAAuthenticationToken authToken, int error) {
        if (authToken.getAuthenticationType() == SAAuthenticationToken.AUTHENTICATION_TYPE_CERTIFICATE_X509) {
            mContext = getApplicationContext();
            byte[] appKey = getApplicationCertificate(mContext);
            // Ensure received token exists
            if (authToken.getKey() != null) {
                // Do token and app key match
                boolean matched = true;
                // Token and app key differ in length - do not match
                if (authToken.getKey().length != appKey.length){
                    matched = false;
                }
                else {
                    // Check all token bytes
                    for (int i = 0; i < authToken.getKey().length; i++) {
                        // Check for a differing byte - do not match if differing
                        if (authToken.getKey()[i] != appKey[i])
                            matched = false;
                    }
                }
                // Accept connection request if token and app key match
                if (matched)
                    acceptServiceConnectionRequest(peerAgent);
            }
        }
    }

    @Override
    protected void onServiceConnectionResponse(SAPeerAgent peerAgent, SASocket socket, int result) {
        if (result == CONNECTION_SUCCESS) {
            if (socket != null) {
                mConnectionHandler = (ServiceConnection) socket;
                // Get max allowed data size from peer
                maxDataSize = peerAgent.getMaxAllowedDataSize();
            }
        }
    }

    @Override
    protected void onError(SAPeerAgent peerAgent, String errorMessage, int errorCode) {
        super.onError(peerAgent, errorMessage, errorCode);
    }

    private static byte[] getApplicationCertificate(Context context) {
        if (context == null)
            return null;
        byte[] cert = null;
        String packageName = context.getPackageName();
        try {
            PackageInfo pkgInfo = context.getPackageManager().getPackageInfo(packageName, PackageManager.GET_SIGNATURES);
            if (pkgInfo == null)
                return null;
            Signature[] sigs = pkgInfo.signatures;
            if (sigs != null) {
                // Get app certificate
                CertificateFactory cf = CertificateFactory.getInstance("X.509");
                ByteArrayInputStream bis = new ByteArrayInputStream(sigs[0].toByteArray());
                X509Certificate x509cert = (X509Certificate)cf.generateCertificate(bis);
                cert = x509cert.getPublicKey().getEncoded();
            }
        } catch (PackageManager.NameNotFoundException e) {
            e.printStackTrace();
        } catch (CertificateException e) {
            e.printStackTrace();
        }
        return cert;
    }

    // Securely send data to peer
    public void secureSendData(final String data) {
        // Ensure connection is valid, and data is valid and fits in max data size
        if (data != null && mConnectionHandler != null && data.getBytes().length < maxDataSize) {
            final ServiceConnection handler = mConnectionHandler;
            new Thread(() -> {
                try {
                    byte[] bytes = data.getBytes();
                    // Securely send data to peer
                    handler.secureSend(getServiceChannelId(0), bytes);
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }).start();
        }
    }

    private void processUnsupportedException(SsdkUnsupportedException e) {
        e.printStackTrace();
        int errType = e.getType();
        switch(errType) {
            case(SsdkUnsupportedException.VENDOR_NOT_SUPPORTED):
            case(SsdkUnsupportedException.DEVICE_NOT_SUPPORTED):
                releaseAgent();
                return;
            case(SsdkUnsupportedException.LIBRARY_NOT_INSTALLED):
            case(SsdkUnsupportedException.LIBRARY_UPDATE_IS_RECOMMENDED):
            case(SsdkUnsupportedException.LIBRARY_UPDATE_IS_REQUIRED):
                return;
            default:
                break;
        }
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
            // Convert received string to JSON
            try {
                 JSONObject obj = new JSONObject(msg);
                 String requestType = obj.getString("requestType");
                 if (requestType.equals("getKeys"))
                     updateKeys();
            } catch (JSONException e) {
                e.printStackTrace();
            }
        }

        @Override
        protected void onServiceConnectionLost(int reason) {
            mConnectionHandler = null;
        }

        private void updateKeys() {
            // Get JSON of all fields
            JSONObject obj = MainActivity.getKeys();
            // Convert JSON to string
            String data = obj.toString();
            // Send data to peer
            secureSendData(data);
        }
    }
}
