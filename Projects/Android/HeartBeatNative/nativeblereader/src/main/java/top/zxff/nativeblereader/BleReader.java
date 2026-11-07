package top.zxff.nativeblereader;

import static androidx.core.content.ContextCompat.startActivity;

import android.Manifest;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanFilter;
import android.bluetooth.le.ScanResult;
import android.bluetooth.le.ScanSettings;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Build;
import android.os.Handler;
import android.os.ParcelUuid;
import android.provider.Settings;
import android.widget.Toast;

import androidx.annotation.Discouraged;
import androidx.annotation.NonNull;
import androidx.core.app.ActivityCompat;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.Set;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;

import dalvik.system.PathClassLoader;

public class BleReader {
    @SuppressLint("StaticFieldLeak")
    public static BleReader instance = null;
    @Discouraged(message = "this method should be rewritten in C++ with JNI to load this java code.")
    public static void LoadJavaLibrary(String path) throws ClassNotFoundException {
        new PathClassLoader(path,
                Class.forName("com.unity3d.player.UnityPlayer").getClassLoader())
            .loadClass("top.zxff.nativeblereader.BleReader");
    }

    public native boolean InformNativeDevice(String macAddr, byte[] deviceName);
    public native void OnDeviceData(String macAddr, int heartRate, long energy);
    public native void OnEnergyReset();

    public native void OnAutoConnectStatusChanged(boolean autoConnecting);
    public native void OnScanStatusChanged(boolean isScanning);
    public boolean IsDeviceSelected(String macAddr){
        return BleDevices.containsKey(macAddr) && BleDevices.get(macAddr).selected;
    }


    Context context;

    PermissionHint permissionHint;
    public BleReader(){
        this.context = ModHelper.GetActivity();
        if(this.context == null){
            throw new RuntimeException("The context is nullptr");
        }
        this.handler = new Handler(context.getMainLooper());
        permissionHint = getManifestStatus();

        instance = this;
    }

    class DeviceStatus{
        boolean selected = false;

        BluetoothDevice dev;
        BluetoothGattCb cb;

        ////////////////////////////////////
        boolean serviceDiscovered = false;
        public DeviceStatus(BluetoothDevice dev){
            this.dev = dev;
        }
        @SuppressLint("MissingPermission")
        public boolean Toggle(boolean selected){
            if(this.selected == selected)
                return false;
            this.selected = selected;
            if(selected){
                //turn on
                cb = new BluetoothGattCb();
                cb.gatt = this.dev.connectGatt(context, true,cb, BluetoothDevice.TRANSPORT_LE);
            }else{
                //turn off
                cb.close();
                cb = null;
            }
            return true;
        }

        //As Google documented, the Callback happens in a background thread. Good!
        class BluetoothGattCb extends BluetoothGattCallback {
            final static String HEART_UUID = "00002a37-0000-1000-8000-00805f9b34fb";
            final static String CONTROL_POINT_UUID = "00002a39-0000-1000-8000-00805f9b34fb";
            BluetoothGatt gatt;
            int retry = 0;
            @SuppressLint("MissingPermission")
            public void close(){
                serviceDiscovered = false;
                synchronized (this){
                    this.gatt.close();
                    this.gatt = null;
                }
            }
            /*
            Why we need this variable called useLatestHandleGatt:
                Some device will use the new api : onCharacteristicChanged(gatt, chara, values)
                However, this api is never called in quest 2 device.
                Quest 2 use onCharacteristicChanged(gatt, chara) instead.
                The old api is deprecated in API LEVEL 33, and I'm not sure if it will called in latest device.
                So the stupid variable here to prevent duplicate data.
             */
            boolean useLatestHandleGatt = false;
            @SuppressLint("MissingPermission")
            private void handleGatt(
                    @NonNull BluetoothGatt gatt,
                    @NonNull BluetoothGattCharacteristic characteristic,
                    byte[] value) {
                if (!HEART_UUID.equals(characteristic.getUuid().toString()))
                    return;
                if (value.length < 2)
                    return;

                int flag = value[0] & 0xFF;
                int heartRate;
                if ((flag & 0x01) != 0) {
                    if(value.length < 3)
                        return;
                    // Heart rate is UINT16
                    heartRate = ((value[2] & 0xFF) << 8) | (value[1] & 0xFF);
                } else {
                    // Heart rate is UINT8
                    heartRate = value[1] & 0xFF;
                }

                OnDeviceData(dev.getAddress(), heartRate, 0);
            }

            @Override
            public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
                super.onCharacteristicChanged(gatt, characteristic);
//                if(useLatestHandleGatt)
//                    return;
                handleGatt(gatt, characteristic, characteristic.getValue());
            }


//            @Override
//            public void onCharacteristicChanged(@NonNull BluetoothGatt gatt, @NonNull BluetoothGattCharacteristic characteristic, @NonNull byte[] value) {
//                super.onCharacteristicChanged(gatt, characteristic, value);
//                useLatestHandleGatt = true;
//                handleGatt(gatt, characteristic, value);
//            }

            @SuppressLint("MissingPermission")
            @Override
            public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
                super.onConnectionStateChange(gatt, status, newState);
                if (status != BluetoothGatt.GATT_SUCCESS){
                    gatt.close();
                    this.gatt = null;
                    if(selected && this.retry < 3){
                        this.retry++;
                        handler.postDelayed(()->{
                            if(DeviceStatus.this.cb != this)
                                return;
                            if(!selected)
                                return;
                            if(this.retry < 3){
                                this.gatt = dev.connectGatt(context, true, this, BluetoothDevice.TRANSPORT_LE);
                            }

                        }, 1000);
                    }
                    return;
                }
                if (newState == BluetoothProfile.STATE_CONNECTED) {
                    retry = 0;
                    handler.postDelayed(gatt::discoverServices, 600);
                    if(autoConnectCanceler != null){
                        handler.postDelayed(()->{
                            if(autoConnectCanceler != null)
                                BleReader.this.BleScanStop();
                        }, 700);

                    }
                }else if(newState == BluetoothProfile.STATE_DISCONNECTED){
                    gatt.close();
                    this.gatt = null;
                    if(DeviceStatus.this.cb == this){
                        handler.postDelayed(()->{
                            if(DeviceStatus.this.cb != this)
                                return;
                            if(!selected)
                                return;
                            this.gatt = dev.connectGatt(context, true, this, BluetoothDevice.TRANSPORT_LE);
                        }, 1000);

                    }
                }
            }

            @Override
            public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
                super.onCharacteristicWrite(gatt, characteristic, status);
                if(CONTROL_POINT_UUID.equals(characteristic.getUuid().toString())){
                    if(status == 0){
                        OnEnergyReset();
                    }
                }
            }

            @SuppressLint("MissingPermission")
            @Override
            public void onServicesDiscovered(BluetoothGatt gatt, int status) {
                super.onServicesDiscovered(gatt, status);
                if (gatt != this.gatt)
                    return;
                if(status != BluetoothGatt.GATT_SUCCESS)
                    return;
                for (BluetoothGattService serv : gatt.getServices()) {
                    for (BluetoothGattCharacteristic ch : serv.getCharacteristics()) {
                        if (HEART_UUID.equals(ch.getUuid().toString())) {
                            gatt.setCharacteristicNotification(ch, true);
                            BluetoothGattDescriptor descriptor = ch.getDescriptor(UUID.fromString("00002902-0000-1000-8000-00805f9b34fb"));
                            if(descriptor != null){
                                descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
                                gatt.writeDescriptor(descriptor);
                                serviceDiscovered = true;
                            }
                        }
                    }
                }
            }
        }

    }

    enum PermissionHint{
        UNKNOWN,
        GOOD_LOCATION_REQUIRED,             // for both old game version and new game version
        GOOD_NONEED_LOCATION,               // for manifest that have never_for_location flags
        BAD_BLUETOOTH_OR_LOCATION_MISSED,
        PAIRED_ONLY
    }

    public int getPermisionStatus(){
        switch (permissionHint){
            case UNKNOWN:
                return 0;
            case GOOD_LOCATION_REQUIRED:
                return 1;
            case GOOD_NONEED_LOCATION:
                return 2;
            case BAD_BLUETOOTH_OR_LOCATION_MISSED:
                return 3;
            case PAIRED_ONLY:
                return 4;
        }
        return 0;
    }
    PermissionHint getManifestStatus(){
        try{
            PackageInfo packageInfo = context.getPackageManager().getPackageInfo(context.getPackageName(), PackageManager.GET_PERMISSIONS);

            boolean perm1 = false, perm2 = false, location_perm = false, perm_flag = false;
            if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.S){
                boolean perm_bluetooth = false;
                for(int i=0;i<packageInfo.requestedPermissions.length;i++){
                    String perm = packageInfo.requestedPermissions[i];
                    if(Manifest.permission.BLUETOOTH.equals(perm))
                        perm_bluetooth = true;
                    if(Manifest.permission.BLUETOOTH_CONNECT.equals(perm)) {
                        perm1 = true;
                    }
                    if(Manifest.permission.BLUETOOTH_SCAN.equals(perm)){
                        perm2 = true;
                        if(0 != (packageInfo.requestedPermissionsFlags[i] & PackageInfo.REQUESTED_PERMISSION_NEVER_FOR_LOCATION)){
                            perm_flag = true;
                        }
                    }
                    if(Manifest.permission.ACCESS_FINE_LOCATION.equals(perm))
                        location_perm = true;
                }
                if(!perm1 || !perm2){
                    if(perm_bluetooth && perm1)
                        return PermissionHint.PAIRED_ONLY;
                    return PermissionHint.BAD_BLUETOOTH_OR_LOCATION_MISSED;
                }
                if(perm_flag){
                    return PermissionHint.GOOD_NONEED_LOCATION;
                }
                if(location_perm){
                    return PermissionHint.GOOD_LOCATION_REQUIRED;
                }
                return PermissionHint.BAD_BLUETOOTH_OR_LOCATION_MISSED;
            }else{
                for(int i=0;i<packageInfo.requestedPermissions.length;i++){
                    String perm = packageInfo.requestedPermissions[i];
                    if(Manifest.permission.BLUETOOTH.equals(perm)) {
                        perm1 = true;
                    }
                    if(Manifest.permission.BLUETOOTH_ADMIN.equals(perm))
                        perm2 = true;
                    if(Manifest.permission.ACCESS_FINE_LOCATION.equals(perm))
                        location_perm = true;
                }
                if(!perm1 || !perm2 || !location_perm){
                    return PermissionHint.BAD_BLUETOOTH_OR_LOCATION_MISSED;
                }
                return PermissionHint.GOOD_LOCATION_REQUIRED;
            }
        }catch (Exception e){
            return PermissionHint.UNKNOWN;
        }

    }

    /* mac -> DeviceStatus */
    ConcurrentHashMap<String, DeviceStatus> BleDevices = new ConcurrentHashMap<>();
    @SuppressLint("InlinedApi")
    private boolean testIfHavePermissions(boolean requirePermissions){
        LinkedList<String> permissions = new LinkedList<>();

        if(permissionHint == PermissionHint.BAD_BLUETOOTH_OR_LOCATION_MISSED)
            return false;

        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.S){
            if(context.checkSelfPermission(Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED)
                permissions.add(Manifest.permission.BLUETOOTH_CONNECT);

            if(permissionHint == PermissionHint.PAIRED_ONLY){
                if(context.checkSelfPermission(Manifest.permission.BLUETOOTH) != PackageManager.PERMISSION_GRANTED)
                    permissions.add(Manifest.permission.BLUETOOTH);
            }else{
                if(context.checkSelfPermission(Manifest.permission.BLUETOOTH_SCAN) != PackageManager.PERMISSION_GRANTED)
                    permissions.add(Manifest.permission.BLUETOOTH_SCAN);
            }
        }else{
            if(context.checkSelfPermission(Manifest.permission.BLUETOOTH) != PackageManager.PERMISSION_GRANTED)
                permissions.add(Manifest.permission.BLUETOOTH);

            if(context.checkSelfPermission(Manifest.permission.BLUETOOTH_ADMIN) != PackageManager.PERMISSION_GRANTED)
                permissions.add(Manifest.permission.BLUETOOTH_ADMIN);
        }

        boolean ret = permissions.isEmpty();

        if(permissionHint == PermissionHint.GOOD_LOCATION_REQUIRED){
            // this is optional if player only wants connect bounded devices, but we will add it.
            if(context.checkSelfPermission(Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED)
                permissions.add(Manifest.permission.ACCESS_FINE_LOCATION);
        }

        if(requirePermissions && !permissions.isEmpty()){
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                ((Activity)context).requestPermissions(
                    permissions.toArray(new String[0]),
                    1
                );
            }
        }

        return ret;
    }


    private BluetoothLeScanner leScanner;

    private boolean isScanning = false;

    private Runnable autoConnectCanceler;
    static class AutoConnectPattern {
        public String name, mac;
        public AutoConnectPattern(String name, String mac){
            this.name = name;
            this.mac = mac;
        }
    };
    AutoConnectPattern autoConnectPattern;
    private Handler handler;

    private ScanCallback leScanCallback = new ScanCallback() {
        @SuppressLint("MissingPermission")
        @Override
        public void onScanResult(int callbackType, ScanResult result) {
            super.onScanResult(callbackType, result);
            if(!isScanning)
                return;
            BluetoothDevice device = result.getDevice();
            if(!BleDevices.containsKey(device.getAddress())){
                BleDevices.put(device.getAddress(),new DeviceStatus(device));
            }
            String devName = device.getName();
            if(InformNativeDevice(device.getAddress(), (devName == null ? "Unknown" : devName).getBytes(StandardCharsets.UTF_8))) {
                BleDevices.get(device.getAddress()).Toggle(true);
            }
        }
    };

    @SuppressLint("MissingPermission")
    public void BleScanStart(){
        if(!testIfHavePermissions(true)){
            System.out.println("No enough permission for bluetooth scan");
            return;
        }
        BluetoothManager mgr = (BluetoothManager) context.getSystemService(Context.BLUETOOTH_SERVICE);
        if(permissionHint != PermissionHint.PAIRED_ONLY){
            if(leScanner == null){
                leScanner = mgr.getAdapter().getBluetoothLeScanner();
            }
            LinkedList<ScanFilter> filters = new LinkedList<>();
            filters.add(new ScanFilter.Builder()
                    .setServiceUuid(new ParcelUuid(UUID.fromString("0000180d-0000-1000-8000-00805f9b34fb")))
                    .build());
            ScanSettings settings = new ScanSettings.Builder()
                    .setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY)
                    .build();
            isScanning = true;
            OnScanStatusChanged(true);
            System.out.println("Start bluetooth scan");
            leScanner.startScan(filters, settings, leScanCallback);
        }else{
            System.out.println("No bluetooth scan, paired only mode");
        }

        Set<BluetoothDevice> deviceSet = mgr.getAdapter().getBondedDevices();
        for (BluetoothDevice bluetoothDevice : deviceSet) {
            if(!BleDevices.containsKey(bluetoothDevice.getAddress())){
                BleDevices.put(bluetoothDevice.getAddress(),
                        new DeviceStatus(bluetoothDevice));
            }
            String devName = bluetoothDevice.getName();
            if(devName == null)
                devName = "Unknown";
            if(InformNativeDevice(bluetoothDevice.getAddress(), devName.getBytes(StandardCharsets.UTF_8))){
                BleDevices.get((bluetoothDevice.getAddress())).Toggle(true);
            }
        }

    }

    @SuppressLint("MissingPermission")
    public void BleScanStop(){
        if(autoConnectCanceler!=null){
            handler.removeCallbacks(autoConnectCanceler);
            autoConnectCanceler=null;
            OnAutoConnectStatusChanged(false);
        }
        if(leScanner != null) {
            leScanner.stopScan(leScanCallback);
            isScanning = false;
            OnScanStatusChanged(false);
        }
    }

    @SuppressLint("MissingPermission")
    public void AutoConnectStart(){
        if(!testIfHavePermissions(false))
            return;
        if(autoConnectCanceler != null){
            handler.removeCallbacks(autoConnectCanceler);
        }
        autoConnectCanceler = ()->{
            autoConnectCanceler = null;
            if(leScanner != null){
                leScanner.stopScan(leScanCallback);
                isScanning = false;
            }
            OnAutoConnectStatusChanged(false);
            OnScanStatusChanged(false);
        };
        // we only search the device in 1 mins.
        handler.postDelayed(autoConnectCanceler,1000 * 20);
        BleScanStart();
        if(permissionHint != PermissionHint.PAIRED_ONLY){
            OnAutoConnectStatusChanged(true);
        }
    }
    public void AutoConnectSetPattern(String macAddress, String deviceName){
        autoConnectPattern = new AutoConnectPattern(deviceName, macAddress);
    }

    void AutoConnectStop(){
        if(autoConnectCanceler != null){
            handler.removeCallbacks(autoConnectCanceler);
            autoConnectCanceler = null;
            OnAutoConnectStatusChanged(false);
        }
        if(isScanning)
            BleScanStop();
    }
    public boolean BleToggle(String macAddress, boolean selected){
        return BleDevices.containsKey(macAddress) && BleDevices.get(macAddress).Toggle(selected);
    }

    @SuppressLint("QueryPermissionsNeeded")
    public void OpenSystemLocationSetthing(){
        Intent intent = new Intent(Intent.ACTION_VIEW, Uri.parse("https://www.meta.com/help/quest/1202271140482151/"));
        // It is good practice to check if an activity can handle the intent
        if (intent.resolveActivity(this.context.getPackageManager()) != null) {
            this.context.startActivity(intent);
        }

    }
}
