/**
 * @file NetworkService.java
 * @author Pavel Y. Kovyrshin <kovyrshi@cs.karelia.ru>
 * @date 14.10.2013
 * @brief
 *
 * @section DESCRIPTION
 *
 * NetworkService.java -
 * 
 *
 * Copyright (C) SmartSlog Team (Pavel Y. Kovyrshin).
 * All rights reserved.
 * Mail-list: smartslog@cs.karelia.ru
 */
package petrsu.smartroom.android.srclient;

import android.app.Activity;
import android.app.ActivityManager;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.wifi.SupplicantState;
import android.net.wifi.WifiManager;
import android.os.IBinder;
import android.widget.Toast;

/**
 * 
 * @author pavlin
 *
 *	Service works with network by checking
 *	wifi and Smart Space connections
 */
public class NetworkService extends Service {

	public static final String BC_RECOVERING_CONNECTION = 
			"petrsu.smartroom.android.srclient.recover.connection";
	private BroadcastReceiver bcReceiver;
	public static boolean isRunning = false;
	public static final int START_RECOVER = 100;
	public static final int STOP_RECOVER = 101;
	public static final int FAIL_RECOVER = 102;
	public static final int OK_RECOVER = 103;
	private static boolean testerState = false;
	private static boolean testerIsStoped = true;
	private static boolean wifiState = false;
	
	private final int period = 7000;			// Check connection period
	private final int attemptPeriod = 3000;		// Recovery connection period
	private final int efforts = 3;				// Recovery attempts number
	private final String appPackageName = "petrsu.smartroom.android.srclient";
	private Thread testerThread;
	
	@Override
	public IBinder onBind(Intent intent) {
		return null;
	}
	
	@Override
	public void onStart(Intent intent, int startId) {
		isRunning = true;
		bcReceiver = new BroadcastReceiver() {
			public void onReceive(Context context, Intent intent) {
				String packageName = getCurrentPackageName();
				
				/* If state of establishing connection has changed */
				if(intent.getAction().equals(
						WifiManager.SUPPLICANT_STATE_CHANGED_ACTION)) {
					SupplicantState state = (SupplicantState)intent
							.getParcelableExtra(WifiManager.EXTRA_NEW_STATE);
					
					/* Recognize and handle changed state */
					if(SupplicantState.isValidState(state)) {
						if(state == SupplicantState.DISCONNECTED) {
							if(packageName.equals(appPackageName)) {
								Toast.makeText(context, R.string.wifiConnLost,
										Toast.LENGTH_LONG).show();
							}
							stopTesterThread();
							wifiState = false;
							
						} else if(state == SupplicantState.COMPLETED) {
							wifiState = true;
							resumeTesterThread();
							
							if(testerIsStoped) {
								initTester();
							}
						}
					}
				}
				
				/* If WiFi state has been changed */
				if(intent.getAction().equals(
						WifiManager.WIFI_STATE_CHANGED_ACTION)) {
					 int state = intent.getIntExtra(
							 WifiManager.EXTRA_WIFI_STATE,
							 WifiManager.WIFI_STATE_UNKNOWN);
					 
					 if(state == WifiManager.WIFI_STATE_DISABLED) {
						 stopTesterThread();
						 wifiState = false;
						 System.out.println("Wifi disabled");
					 }
				}
			}
		};
		
		if(testerIsStoped) {
			initTester();
		}
		
		IntentFilter filter = new IntentFilter(
				WifiManager.WIFI_STATE_CHANGED_ACTION);
		filter.addAction(WifiManager.SUPPLICANT_STATE_CHANGED_ACTION);
		registerReceiver(bcReceiver, filter);
	}
	
	/**
	 * Initializes network tester thread
	 */
	private void initTester() {
		testerState = true;
		testerIsStoped = false;
		
		testerThread = new Thread() {
			@Override
			public void run() {
				int isReconnected = -1;
				String pkgName;
				
				while(testerState) {
					pkgName = getCurrentPackageName();
					
					try {
						/* If wifi is active and current app is SRClient */
						if(wifiState && pkgName.equals(appPackageName)) {
							if(!KP.checkConnection()) {
								Intent intent = new Intent(
										BC_RECOVERING_CONNECTION);
								sendBroadcastMsg(intent, START_RECOVER);
								
								/* Recovering process */
								for(int i = 1; i <= efforts; i++) {
									System.out.println(
											"Recovering effort: " + i);
									isReconnected = KP.reconnect();
									
									if(isReconnected != 0) {
										Thread.sleep(attemptPeriod);
										continue;
									} else {
										break;
									}
								}
							
								sendBroadcastMsg(intent, STOP_RECOVER);
							
								if(isReconnected != 0) {
									sendBroadcastMsg(intent, FAIL_RECOVER);
								} else {
									sendBroadcastMsg(intent, OK_RECOVER);
									isReconnected = -1;
								}
							}
						}
						Thread.sleep(period);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}
				testerIsStoped = true;
			}
		};
		testerThread.start();
	}
	
	/**
	 * Sends broadcast message
	 * 
	 * @param intent - broadcast intent
	 * @param message - broadcast message
	 */
	private void sendBroadcastMsg(Intent intent, int message) {
		intent.putExtra("action", message);
		getApplicationContext().sendBroadcast(intent);
	}
	
	/**
	 * Gets current application package
	 * 
	 * @return Name of current application package
	 */
	public String getCurrentPackageName() {
		ActivityManager am = (ActivityManager) getApplicationContext().
				getSystemService(Activity.ACTIVITY_SERVICE);
		return am.getRunningTasks(1).get(0).topActivity.getPackageName();
	}
	
	/**
	 * Stops network tester thread
	 */
	private void stopTesterThread() {
		testerState = false;
	}
	
	/**
	 * Resumes network tester thread
	 */
	private void resumeTesterThread() {
		testerState = true;
	}
	
	@Override
	public void onDestroy() {
		unregisterReceiver(bcReceiver);
		isRunning = false;
		testerState = false;
	}
}
