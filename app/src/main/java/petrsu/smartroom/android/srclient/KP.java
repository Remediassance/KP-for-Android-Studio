package petrsu.smartroom.android.srclient;


import android.net.Uri;
import android.os.Bundle;
import android.provider.Settings;
import android.widget.*;
import android.text.Html;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.content.Intent;
import android.content.Context;
import android.view.KeyEvent;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.content.res.Configuration;

import java.util.ArrayList;
import java.lang.ClassCastException;

import android.support.v7.app.ActionBarActivity;
import android.support.v7.app.AlertDialog;
import android.support.v7.widget.Toolbar;

import com.mikepenz.iconics.typeface.FontAwesome;
import com.mikepenz.materialdrawer.Drawer;
import com.mikepenz.materialdrawer.DrawerBuilder;
import com.mikepenz.materialdrawer.model.DividerDrawerItem;
import com.mikepenz.materialdrawer.model.PrimaryDrawerItem;
import com.mikepenz.materialdrawer.model.SecondaryDrawerItem;
import com.mikepenz.materialdrawer.model.interfaces.IDrawerItem;



/**
 * Main calss of the application. Not only it implements
 * log-in activity, it also acts as a starting point of the
 * app and holds declarations of all native methods and fields
 * that are used in other classes.
 *
 * @author pavlin
 * @author remediassance
 */
public class KP extends ActionBarActivity implements View.OnClickListener {
	
	public static boolean isChairman;		// Chairman indicator
	public static int connectionState;		// Connection indicator
	public static String ip;				// SIB IP
	public static int port;					// SIB port
	public static int personIndex = -1;		// Time slot index of a person
	public static boolean isRegistered;		// User was registered in SS
    public static String dqAddr = "http://192.168.43.33/";
	private ImageView advancedModeImg;		// Advance mode trigger
	private TextView advancedModeText;
	private Button connectBtn;	
	private Button guestBtn;
	private Button regServiceBtn;
	private final String regServiceLink = "http://smartroom.cs.petrsu.ru/content/login";
    public final static String manLink = "https://drive.google.com/file/d/0B1qQ0VknaJehaF94SmNxaTI4YlE/view?usp=sharing";
    public final static String localMan = "file:///res/raw/manual";
	private static EditText editName;
	private static EditText editPassword;
	private EditText editIP;
	private static EditText editPort;	
	private ArrayList<String> timeslotList;
	private String lastState;
	
	public static native int connectSmartSpace(String hostname, String ip,  int port);
	public static native int loadTimeslotList(Agenda obj);
	public static native void disconnectSmartSpace();
	public static native int getServicesInfo(ServicesMenu menu);
	public static native int  userRegistration(String userName, String password);
	public static native int loadPresentation(Projector projector);
	public static native int initSubscription();
	public static native int startConference();
	public static native int endConference();
	public static native void getProjectorClassObject();
	public static native int showSlide(int slideNumber);
	public static native int endPresentation();
	public static native int getCurrentTimeslotIndex();
	public static native boolean checkSpeakerState();
	public static native String getMicServiceIP();
	public static native String getMicServicePort();
	public static native String getSpeakerName();
	public static native int personTimeslotIndex();
	public static native boolean checkConnection();
	public static native String getPresentationLink(int index);
	public static native int registerGuest(String name, String phone, String email);
	public static native int startConferenceFrom(int index);
	public static native String loadProfile(Profile profile, int index);
	public static native int saveProfileChanges(String name, String phone);
	public static native String getPersonUuid();
	public static native CharSequence[] getVideoTitleList();
	public static native CharSequence[] getVideoUuidList();
	public static native String getContentUrl();
	public static native int isActiveSubscriptions();
	public static native int refreshConferenceSbcr();
	public static native int refreshPresentationSbcr();
	public static native String[] getCurrentSectionList();
	public static native boolean sectionChanged();
	public static native int startVideo(String url);
	public static native void stopVideo();
	
	/* Loading of shared library */
	static {
		System.loadLibrary("sslog");
	}
	
	@Override
	protected void onResume() {
		super.onResume();
		
		try {
			applyPreferences();
		} catch(ClassCastException e) {
			e.printStackTrace();
		}
	}
	
	@Override
	protected void onPause() {
		super.onPause();
		savePreferences();
	}
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
        setContentView(R.layout.kp_interface);
        setScreenTimeoutSpec();
        
        connectBtn = (Button) findViewById (R.id.connectBtn);
        connectBtn.setOnClickListener(this);
        guestBtn = (Button) findViewById (R.id.guestBtn);
        guestBtn.setOnClickListener(this);
        
        editName = (EditText) findViewById (R.id.editName);
        editPassword = (EditText) findViewById (R.id.editPassword);
        editIP = (EditText) findViewById (R.id.editIP);
        editPort = (EditText) findViewById (R.id.editPort);
        advancedModeImg = (ImageView) findViewById (R.id.advModeImg);
        advancedModeImg.setOnClickListener(this);
        advancedModeText = (TextView) findViewById (R.id.advModeText);
        advancedModeText.setOnClickListener(this);

        // Handle Toolbar
        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);

        Drawer result = new DrawerBuilder()
                .withActivity(this)
                .withToolbar(toolbar)
                .withActionBarDrawerToggle(true)
                .withHeader(R.layout.drawer_header)
				.withDrawerWidthDp(320)
                .addDrawerItems(
                        new PrimaryDrawerItem().withName(R.string.signup).withIcon(FontAwesome.Icon.faw_barcode),
                        new PrimaryDrawerItem().withName(R.string.manual).withIcon(FontAwesome.Icon.faw_download),
                        new PrimaryDrawerItem().withName(R.string.drawer_item_help).withIcon(FontAwesome.Icon.faw_info),
                        new DividerDrawerItem(),
                        new PrimaryDrawerItem().withName(R.string.discussionCur).withIcon(FontAwesome.Icon.faw_comment),
                        new PrimaryDrawerItem().withName(R.string.discussionList).withIcon(FontAwesome.Icon.faw_comments),
                        new DividerDrawerItem(),
                        new SecondaryDrawerItem().withName(R.string.exitClientTitle).withIcon(FontAwesome
								.Icon.faw_close)
                ).withOnDrawerItemClickListener(new Drawer.OnDrawerItemClickListener() {
            @Override
            public boolean onItemClick(AdapterView<?> parent, View view, int position, long id, IDrawerItem drawerItem) {
                switch ((int) id) {
                    case 0:
                        scanQrCode();
                        break;
                    case 1:
                        gotoManual();
                        break;
                    case 2:
                        openHelp();
                        break;
                    case 4:
                        gotoCurDisq();
                        break;
                    case 5:
                        gotoDisqList();
                        break;
                    case 7:
                        exitApp();
                        break;
                    default:
                        break;
                }
                return true;
            }
        }).build();

            isChairman=false;
            isRegistered=false;
        }



    /**=========================================================================
     * GO TO CURRENT DISCUSSION
     *==========================================================================
     */
    private void gotoCurDisq(){
        String contentUrl = KP.getContentUrl();
        //String addr = contentUrl.substring(0,contentUrl.lastIndexOf("files")); //smartroom.cs.petrsu.ru
        //Toast.makeText(getApplicationContext(), addr, Toast.LENGTH_LONG).show();

        Intent intent = new Intent(getApplicationContext(), WebViewer.class);
        intent.putExtra("url", KP.dqAddr+"chat");
        intent.putExtra("flag", true);

        startActivity(intent);
    }


    /**=========================================================================
     * GO TO  DISCUSSION LIST
     *==========================================================================
     */
    private void gotoDisqList(){
        String contentUrl = KP.getContentUrl();
        //String addr = contentUrl.substring(0, contentUrl.lastIndexOf("files")); //smartroom.cs.petrsu.ru

        Intent intent = new Intent(getApplicationContext(), WebViewer.class);
        intent.putExtra("url",KP.dqAddr+"chat/listCurrentThreads");
        intent.putExtra("flag", true);

        startActivity(intent);
    }


    /*=========================================================================
    * OPENS BROWSER ON THE DOWNLOAD MANUAL PAGE
    * =========================================================================
     */
    private void gotoManual() {
        Intent intent = new Intent(getApplicationContext(), WebViewer.class);
        intent.putExtra("url",manLink);
        intent.putExtra("flag", true);

        startActivity(intent);

    }

    /*=========================================================================
    * SHOWS HELP WINDOW
    *==========================================================================
     */
    private void openHelp() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(R.string.joiningSR);
        builder.setMessage(Html.fromHtml(getResources().getString(R.string.kp_help_content)));
        builder.create();
        builder.show();
    }

    /*=========================================================================
    * QITS TO THE DESKTOP
    *==========================================================================
    */
    private void exitApp() {
        Intent intent = new Intent(Intent.ACTION_MAIN);
        intent.addCategory(Intent.CATEGORY_HOME);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(intent);
    }

  	@Override
	public void onClick(View view) {
		final String name = editName.getText().toString();
		final String password = editPassword.getText().toString();
		ip = editIP.getText().toString();

		switch(view.getId()) {
			case R.id.advModeImg:
			case R.id.advModeText:
				if(editIP.getVisibility() == EditText.VISIBLE) {
					advancedModeImg.setImageResource(R.drawable.ic_add);
					advancedModeText.setText(R.string.advancedModeOn);
					editIP.setVisibility(EditText.INVISIBLE);
					editPort.setVisibility(EditText.INVISIBLE);
				} else {
					advancedModeImg.setImageResource(R.drawable.ic_close);
					advancedModeText.setText(R.string.advancedModeOff);
					editIP.setVisibility(EditText.VISIBLE);
					editPort.setVisibility(EditText.VISIBLE);
				}
				break;

			case R.id.connectBtn:
                try {
                    port = Integer.parseInt(editPort.getText().toString());
                    joinSmartSpace(name, password);
                } catch(NumberFormatException e) {
                    Toast.makeText(this, R.string.portFormatErr,
                            Toast.LENGTH_SHORT).show();
                }
                break;

            case R.id.guestBtn:
                try {
                    port = Integer.parseInt(editPort.getText().toString());
                    joinAsGuest(port);
                } catch(NumberFormatException e) {
                    Toast.makeText(this, R.string.portFormatErr,
                            Toast.LENGTH_SHORT).show();
                }
                break;
		}
	}

	/**
	 * Starts Agenda activity
	 */
	public void loadAgenda() {
		Intent intent = new Intent();
		intent.setClass(getApplicationContext(), Agenda.class);
		startActivity(intent);
	}
	
	/**
	 * Starts Projector activity
	 */
	public void loadProjector() {
		Intent intent = new Intent();
		intent.setClass(getApplicationContext(), Projector.class);
		startActivity(intent);
	}
	
	/**
	 * Starts Services menu activity
	 */
	public void loadServicesMenu() {
		Intent intent = new Intent();
		intent.setClass(getApplicationContext(), ServicesMenu.class);
		startActivity(intent);
	}
	
	/**
	 * Starts reconnection process
	 * 
	 * @return 0 if reconnection was successful and -1 otherwise
	 */
	public static int reconnect() {
		String name = KP.editName.getText().toString();
		String password = KP.editPassword.getText().toString();
		int port = Integer.parseInt(KP.editPort.getText().toString());
		
		disconnectSmartSpace();
		
		if(connectSmartSpace("X", KP.ip, port) != 0) {
			System.out.println("Connection failed");
			return -1;
		} else		
			connectionState = 1;
		
		if(initSubscription() != 0) {
			System.out.println("Sbcr init failed");
			KP.disconnectSmartSpace();
			KP.connectionState = -1;
			return -1;
		}
		
		/* If user joined as a guest */
		if(name.equals("") && password.equals("")) {
			/* TODO: it is might be that after loosing connection
			 * SIB was cleaned and such reconnection for guest
			 * will not create profile but user will have access to SS
			 */
			KP.isRegistered = true;
			return 0;
		}
		
		/* TODO: password is open! */
		if(password.equals("chairman")) {
			KP.isChairman = true;
		} else {
			
			/* Start user registration */
			if(userRegistration(name, password) == 0) {
				Log.i("Java KP", "Registration successful");
			} else
				return -1;
		}
		KP.isRegistered = true;
		return 0;
	}
	
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event)  {
		if (keyCode == KeyEvent.KEYCODE_BACK) {
			setScreenTimeoutDefault();
			stopService(new Intent(this, NetworkService.class));
		}
		
		return super.onKeyDown(keyCode, event);
	}
	
	/**
	 * Establishes connection with Smart Space
	 * 
	 * @param name - User name
	 * @param password - User password
	 * @param port - SIB port
	 * @return 0 in success and -1 otherwise
	 */
	public int establishConnection(String name, String password, int port) {
		
		/* If not connected to SS */
		if(connectionState != 1) {
			if(connectSmartSpace("X", ip, port) != 0) {
				Log.e("Connection", "Connection failed");
				Toast.makeText(this, R.string.connectionFail, 
						Toast.LENGTH_SHORT)
						.show();
				return -1;
			} else
				connectionState = 1;
			
			if(initSubscription() != 0) {
				Log.e("Java KP", "Init subscription failed");
				KP.disconnectSmartSpace();
				connectionState = -1;
				return -1;
			}
		}

		if(!isRegistered) {
			/* TODO: password is open! */
			if(password.equals("chairman")) {
				isChairman = true;
			} else {
				if(userRegistration(name, password) == 0) {
					Log.i("Java KP", "Registration successful");
				} else {
					Log.e("Registration", "registration failed");
					Toast.makeText(this, R.string.registrationFail, 
							Toast.LENGTH_SHORT).show();
					KP.disconnectSmartSpace();
					connectionState = -1;
					return -1;
				}
			}
			isRegistered = true;
		}
		return 0;
	}
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		MenuInflater inflater = getMenuInflater();
		inflater.inflate(R.menu.kp_bar, menu);		
		return super.onCreateOptionsMenu(menu);
	}
	
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch(item.getItemId()) {
			case R.id.kp_help:
				AlertDialog.Builder builder = new AlertDialog.Builder(this);
				builder.setTitle(R.string.joiningSR);
				builder.setMessage(Html.fromHtml(getResources().getString(
						R.string.kp_help_content)));
				builder.create();
				builder.show();
				break;
		}
		return super.onOptionsItemSelected(item);
	}
	
	@Override
	protected void onDestroy() {
		super.onDestroy();
		stopService(new Intent(getApplicationContext(), NetworkService.class));
	}
	
	/**
	 * Saves preferences
	 */
	public void savePreferences() {
		SharedPreferences prefs = getSharedPreferences("srclient_conf", 
				Context.MODE_PRIVATE);
		SharedPreferences.Editor editor = prefs.edit();
		
		editor.putString("ip", editIP.getText().toString());
		editor.putString("port", editPort.getText().toString());
		editor.putString("username", editName.getText().toString());
		editor.putString("password", editPassword.getText().toString());
		editor.putInt("advancedMode", editIP.getVisibility());
		editor.commit();
	}
	
	/**
	 * Load user preferences
	 */
	public void applyPreferences() {
		SharedPreferences prefs = getSharedPreferences("srclient_conf", 
				Context.MODE_PRIVATE);
		String ip = prefs.getString("ip", "");
		String port = prefs.getString("port", "");
		String username = prefs.getString("username", "");
		String password = prefs.getString("password", "");
		int advancedMode = prefs.getInt("advancedMode", EditText.INVISIBLE);
		
		if(getResources().getConfiguration().orientation == 
        		Configuration.ORIENTATION_LANDSCAPE) {
        	advancedModeImg.setVisibility(ImageView.VISIBLE);
        	advancedModeText.setVisibility(TextView.VISIBLE);
        	advancedMode = EditText.VISIBLE;
        }
		
		editIP.setText(ip);
		editPort.setText(port);
		editName.setText(username);
        editPassword.setText(password);
        editIP.setVisibility(View.VISIBLE);
        editPort.setVisibility(View.VISIBLE);
        
        if(advancedMode == EditText.VISIBLE) {
        	advancedModeImg.setImageResource(R.drawable.ic_close);
			advancedModeText.setText(R.string.advancedModeOff);
        } else {
        	advancedModeImg.setImageResource(R.drawable.ic_add);
			advancedModeText.setText(R.string.advancedModeOn);
        }

        int timeout = prefs.getInt(SettingsMenu.TIMEOUT_SCREEN_PREF, 
				SettingsMenu.defaultTimeout);
		
		android.provider.Settings.System.putInt(getContentResolver(), 
				Settings.System.SCREEN_OFF_TIMEOUT, timeout);
		
		lastState = prefs.getString("last_state", "Agenda");
	}
	
	/**
	 * Joins to Smart Space and starts background services
	 * 
	 * @param name - User name
	 * @param password - User password
	 */
	public void joinSmartSpace(final String name, final String password) {
		if(name.equals("") || password.equals("")) {
			showAnonimousDialog();
			return;
		}
		
		if(establishConnection(name, password, port) != 0) {
			return;
		}
		
		personIndex = KP.personTimeslotIndex();
		
		stopService(new Intent(this, NetworkService.class));
		startService(new Intent(this, NetworkService.class));
		
		loadAgenda();
		/*if(lastState.equals("Agenda"))
			loadAgenda();
		else if(lastState.equals("Projector"))
			loadProjector();
		else
			loadServicesMenu();
		*/
	}
	
	/**
	 * Joins to Smart Space as a guest
	 * 
	 * @param port - SIB port
	 */
	public void joinAsGuest(final int port) {
		LayoutInflater inflater = LayoutInflater.from(this);
		final View dialogView = inflater.inflate(
				R.layout.guest_registration, null);
		
		AlertDialog.Builder builder = new AlertDialog.Builder(this);
		builder.setView(dialogView);
		builder.setTitle(R.string.registrationTitle);
		builder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
			@Override
			public void onClick(DialogInterface dialog, int which) {
				EditText editName = (EditText) dialogView
						.findViewById(R.id.guestName);
				EditText editPhone = (EditText) dialogView
						.findViewById(R.id.guestPhone);
				EditText editEmail = (EditText) dialogView
						.findViewById(R.id.guestEmail);
				
				String name = editName.getText().toString();
				String phone = editPhone.getText().toString();
				String email = editEmail.getText().toString();
				int ret_value = 0;
				
				if(name.equals("")) {
					Toast.makeText(getApplicationContext(), 
							R.string.guestNameEmpty, 
							Toast.LENGTH_SHORT)
							.show();
				} else {
					if(connectSmartSpace("X", ip, port) != 0) {
						Toast.makeText(getApplicationContext(), 
								R.string.connectionFail, 
								Toast.LENGTH_SHORT)
								.show();
						return;
					} else					
						connectionState = 1;
					
					if(initSubscription() != 0) {
						System.out.println("Sbcr failed");
						KP.disconnectSmartSpace();
						KP.connectionState = -1;
						return;
					}
					
					ret_value = registerGuest(name, phone, email);
					
					if(ret_value == -1) {
						Toast.makeText(getApplicationContext(), 
								R.string.registrationFail, 
								Toast.LENGTH_SHORT)
								.show();
						return;
					} else if(ret_value == 1) {
						Toast.makeText(getApplicationContext(), 
								R.string.nameIsUsed, 
								Toast.LENGTH_SHORT)
								.show();
						return;
					}
					
					stopService(new Intent(getApplicationContext(), 
							NetworkService.class));
					startService(new Intent(getApplicationContext(), 
							NetworkService.class));
					loadAgenda();
				}
			}
		});
		builder.setNegativeButton(android.R.string.cancel, null);
		AlertDialog dialog = builder.create();
		dialog.show();
	}
	
	/**
	 * Sets screen timeout used by application
	 */
	private void setScreenTimeoutSpec() {
		SharedPreferences prefs = getSharedPreferences(
				"srclient_conf", Context.MODE_PRIVATE);
		SharedPreferences.Editor editor = prefs.edit();
		
		/* Save system screen timeout value */
		int screenTimeout = android.provider.Settings.System.getInt(
				getContentResolver(), Settings.System.SCREEN_OFF_TIMEOUT , 
				SettingsMenu.defaultTimeout);
		
		editor.putInt(SettingsMenu.SYSTEM_TIMEOUT_SCREEN_PREF, screenTimeout);
		editor.commit();
	}
	
	/**
	 * Sets default (saved) screen timeout
	 */
	private void setScreenTimeoutDefault() {
		SharedPreferences prefs = getSharedPreferences(
				"srclient_conf", Context.MODE_PRIVATE);
		int timeout = prefs.getInt(SettingsMenu.SYSTEM_TIMEOUT_SCREEN_PREF, 
				SettingsMenu.defaultTimeout);
		
		android.provider.Settings.System.putInt(getContentResolver(), 
				Settings.System.SCREEN_OFF_TIMEOUT, timeout);
	}
	
	/**
	 * Checks whether subscriptions are active
	 * 
	 * @return 0 if subscriptions are active and -1 otherwise
	 */
	public static int checkSubscriptionState() {
		final int sbcrState = KP.isActiveSubscriptions();
		final int conferenceSbcrNotActive = -1;
		final int presentationSbcrNotActive = -2;
		
		switch(sbcrState) {
			case conferenceSbcrNotActive:
				if(KP.refreshConferenceSbcr() != 0)
					return -1;
				break;
				
			case presentationSbcrNotActive:
				if(KP.refreshPresentationSbcr() != 0)
					return -1;
				break;
		}
		
		return 0;
	}
	
	/**
	 * Shows dialog if join button was clicked
	 * with empty login and password parameters
	 */
	public void showAnonimousDialog() {
		AlertDialog.Builder builder = new AlertDialog.Builder(this);
		builder.setTitle(R.string.joiningSR);
		builder.setMessage(getResources().getString(R.string.anonimJoinMsg));
		builder.setPositiveButton(android.R.string.ok, 
				new DialogInterface.OnClickListener() {
			@Override
			public void onClick(DialogInterface dialog, int which) {
				isRegistered = true;
				if(establishConnection("", "", port) == 0)
					loadAgenda();
			}
		});
		builder.setNegativeButton(android.R.string.cancel, null);
		builder.show(); 
	}
	
	/**========================================================================
	 * Starts QR code scanner application or
	 * offers to install one
     *=========================================================================
	 */
	public void scanQrCode() {
		try {
			Intent intent = new Intent("com.google.zxing.client.android.SCAN");
		    intent.putExtra("SCAN_MODE", "QR_CODE_MODE");
		    startActivityForResult(intent, 0);
		} catch (Exception e) {
			Uri marketUri = Uri.parse(
					"market://details?id=com.google.zxing.client.android");
			Intent marketIntent = new Intent(Intent.ACTION_VIEW,marketUri);
			startActivity(marketIntent);
		}
	}
	
	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {           
	    super.onActivityResult(requestCode, resultCode, data);
	    if (requestCode == 0) {
	        if (resultCode == RESULT_OK) {
	            String contents = data.getStringExtra("SCAN_RESULT");
	            Uri marketUri = Uri.parse(contents);
			    Intent marketIntent = new Intent(Intent.ACTION_VIEW, marketUri);
			    startActivity(marketIntent);
	        }
	    }
	}
}
