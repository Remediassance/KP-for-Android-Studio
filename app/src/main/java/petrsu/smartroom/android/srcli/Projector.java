package petrsu.smartroom.android.srcli;

import android.app.ActivityManager;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.app.Service;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.support.v7.app.ActionBarActivity;
import android.support.v7.widget.Toolbar;
import android.widget.*;
import android.text.Html;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.AsyncTask;
import android.content.Context;
import java.net.HttpURLConnection;
import android.content.BroadcastReceiver;

import com.mikepenz.iconics.typeface.FontAwesome;
import com.mikepenz.materialdrawer.Drawer;
import com.mikepenz.materialdrawer.DrawerBuilder;
import com.mikepenz.materialdrawer.model.DividerDrawerItem;
import com.mikepenz.materialdrawer.model.PrimaryDrawerItem;
import com.mikepenz.materialdrawer.model.SecondaryDrawerItem;
import com.mikepenz.materialdrawer.model.SectionDrawerItem;
import com.mikepenz.materialdrawer.model.interfaces.IDrawerItem;

import java.io.InputStream;
import java.net.URL;
import java.util.List;

/**
 * 
 * @author pavlin
 * @author remediassance
 *	Controls presentation demonstration process
 */
public class Projector extends ActionBarActivity implements View.OnClickListener {
	
	public static final String BROADCAST_STATUS_SERVICE = 
			"com.example.srclient.stopMicService";
	public static final String SERVICE_STATUS = "status";
	public static int presentationCreated = 0;
	public static boolean isSpeaker = false;
	public static boolean micIsActive = false;			// Mike activity
	private static ProgressDialog progressDialog;
	
	private String contentUrl;							// Content service URL
	private BroadcastReceiver bcReceiver;
	private RelativeLayout linearLayout;				// Control panel layout
	private ImageView leftArrowBtn;
	private ImageView rightArrowBtn;
	private ImageView presentationImage;	
	private ImageButton microphoneBtn;
	private ImageView refreshBtn;
	private TextView textNumAndCount;	
	private boolean controlPanelIsActive;	
	private String slideImageLink;	
	private String speakerName;	
	private int slideNumber;	
	private int slideCount;	
	
	public Projector() {
		slideNumber = 0;
		slideCount = 0;
		slideImageLink = "";
	}
	
	@Override
	protected void onResume() {
		super.onResume();
		SharedPreferences prefs = getSharedPreferences("srclient_conf", 
				Context.MODE_PRIVATE);
		SharedPreferences.Editor editor = prefs.edit();
		
		editor.putString("last_state", "Projector");
		editor.commit();
		
		micIsActive = prefs.getBoolean("micIsActive", false);
		
		if(micIsActive) {
			Bitmap image = bitmapImageFromRes(R.drawable.start_mic);
			microphoneBtn.setImageBitmap(image);
		}
		
		//if(KP.checkSubscriptionState() != 0)
			//Log.e("Presentation check sbcr", "failed to refresh sbcr");
		
		updateProjector();
		registerReceiver();
		presentationCreated = 1;
	}
	
	@Override
	protected void onPause() {
		super.onPause();
		SharedPreferences prefs = getSharedPreferences("srclient_conf", 
				Context.MODE_PRIVATE);
		SharedPreferences.Editor editor = prefs.edit();
		
		editor.putBoolean("micIsActive", micIsActive);
		editor.commit();
		
		stopService(new Intent(this, NetworkService.class));
		
		unregisterReceiver(bcReceiver);
		presentationCreated = 0;
	}

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.projector_interface);
		
		linearLayout = (RelativeLayout) findViewById (R.id.presMenu);
		leftArrowBtn = (ImageView) findViewById (R.id.btnBack);
		leftArrowBtn.setOnClickListener(this);
		rightArrowBtn = (ImageView) findViewById (R.id.btnForward);
		rightArrowBtn.setOnClickListener(this);
		microphoneBtn = (ImageButton) findViewById (R.id.mic);
		microphoneBtn.setOnClickListener(this);
		presentationImage = (ImageView) findViewById (R.id.presImage);
		Bitmap image = bitmapImageFromRes(R.drawable.title);
		presentationImage.setImageBitmap(image);
		presentationImage.setOnClickListener(this);
		textNumAndCount = (TextView) findViewById (R.id.slideNumber);
		textNumAndCount.setText("0/0");

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
						new SectionDrawerItem().withName(R.string.services),
						new PrimaryDrawerItem().withName(R.string.agenda).withIcon(FontAwesome.Icon.faw_server),
						new PrimaryDrawerItem().withName(R.string.presentation).withIcon(FontAwesome.Icon.faw_image),
                        new PrimaryDrawerItem().withName("SocialProgram").withIcon(FontAwesome.Icon.faw_globe),

						new SectionDrawerItem().withName(R.string.discussion),
						new PrimaryDrawerItem().withName(R.string.discussionCur).withIcon(FontAwesome.Icon.faw_comment_o),
						new PrimaryDrawerItem().withName(R.string.discussionList).withIcon(FontAwesome.Icon.faw_comments_o),

						new SectionDrawerItem().withName(R.string.action_settings),
						new PrimaryDrawerItem().withName(R.string.action_settings).withIcon(FontAwesome.Icon.faw_cog),
						new PrimaryDrawerItem().withName(R.string.reconnectText).withIcon(FontAwesome.Icon.faw_refresh),

						new SectionDrawerItem().withName(R.string.help),
						new PrimaryDrawerItem().withName(R.string.help_presentation).withIcon(FontAwesome.Icon.faw_info),
						new PrimaryDrawerItem().withName(R.string.manual).withIcon(FontAwesome.Icon.faw_download),

						new DividerDrawerItem(),
						new SecondaryDrawerItem().withName(R.string.exitClientTitle).withIcon(FontAwesome.Icon.faw_close)
                ).withOnDrawerItemClickListener(new Drawer.OnDrawerItemClickListener() {
            @Override
            public boolean onItemClick(AdapterView<?> parent, View view, int position, long id, IDrawerItem drawerItem) {
                //Toast.makeText(Projector.this, String.valueOf(id), Toast.LENGTH_SHORT).show();
                switch ((int) id) {
                    case 1:     gotoAgenda();       break;
                    case 2:     break;
                    case 3:     gotoSocialProgram();break;
                    case 4:     gotoCurDisq();      break;
                    case 5:     gotoDisqList();     break;
                    case 7:     gotoSettings();     break;
                    case 8:     updateProjector();  break;
                    case 10:    openHelp();     	break;
                    case 11:	gotoManual();       break;
                    case 13:    exitApp();          break;
                    default:    break;

                }
				return true;
            }
        }).build();
		
		/* If connection established */
		if(checkConnection()) {
			contentUrl = KP.getContentUrl();
			KP.loadPresentation(this);
		} 
		
		if(isSpeaker || KP.isChairman) {
			linearLayout.setVisibility(RelativeLayout.VISIBLE);
			leftArrowBtn.setVisibility(RelativeLayout.VISIBLE);
			rightArrowBtn.setVisibility(RelativeLayout.VISIBLE);
			microphoneBtn.setVisibility(RelativeLayout.VISIBLE);
			presentationImage.setClickable(true);
			
		} else {
			linearLayout.setVisibility(RelativeLayout.INVISIBLE);
			leftArrowBtn.setVisibility(RelativeLayout.INVISIBLE);
			rightArrowBtn.setVisibility(RelativeLayout.INVISIBLE);
			microphoneBtn.setVisibility(RelativeLayout.INVISIBLE);
			presentationImage.setClickable(false);
		}

		controlPanelIsActive = false;
		presentationCreated = 1;
	}




    /**=========================================================================
     * GO TO  SOCIAL PROGRAM
     *==========================================================================
     */
    private void gotoSocialProgram(){

        Intent intent = new Intent(getApplicationContext(), WebViewer.class);
        intent.putExtra("url",KP.spAddr.toString());

        startActivity(intent);
    }



    /**========================================================================
     * GO TO AGENDA SERVICE
     *=========================================================================
     */
    private void gotoAgenda(){
        Intent intent = new Intent();
        intent.setClass(this, Agenda.class);
        startActivity(intent);
    }

    /**=========================================================================
     * QITS TO THE DESKTOP
     *==========================================================================
     */
    private void exitApp() {
        Intent intent = new Intent(Intent.ACTION_MAIN);
        intent.addCategory(Intent.CATEGORY_HOME);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(intent);
    }


    /*=========================================================================
    * OPENS BROWSER ON THE DOWNLOAD MANUAL PAGE
    * =========================================================================
     */
    private void gotoManual() {
        Intent intent = new Intent(getApplicationContext(), WebViewer.class);
        intent.putExtra("url",KP.manLink);
        intent.putExtra("reading", true);

        startActivity(intent);
    }

    /**=========================================================================
     * SHOWS HELP WINDOW
     *==========================================================================
     */
    private void openHelp() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(R.string.presentation);
        builder.setMessage(Html.fromHtml(getResources().getString(R.string.presentation_help_content)));
        builder.create();
        builder.show();
    }


    /**========================================================================
     * REFRESH AGENDA PAGE
     *=========================================================================
     */
    private void updateProjector() {
        speakerName = KP.getSpeakerName();
        isSpeaker = KP.checkSpeakerState();

        if(!isSpeaker && !KP.isChairman && micIsActive) {
            micIsActive = false;
            stopService(new Intent(this, MicService.class));
        }
        new updatePresentationAsync().execute();
    }


	/**=========================================================================
	 * GO TO CURRENT DISCUSSION
	 *==========================================================================
	 */
	private void gotoCurDisq(){

		Intent intent = new Intent(getApplicationContext(), WebViewer.class);
        intent.putExtra("url", KP.dqAddr.toString());

        startActivity(intent);
	}


	/**=========================================================================
	 * GO TO  DISCUSSION LIST
	 *==========================================================================
	 */
	private void gotoDisqList(){

		Intent intent = new Intent(getApplicationContext(), WebViewer.class);
        intent.putExtra("url",KP.dqAddr.toString()+"listCurrentThreads");

		startActivity(intent);
	}


    /**========================================================================
     * GO TO SETTINGS ACTIVITY
     *=========================================================================
     */
    private void gotoSettings() {
        Intent intentSettings = new Intent();
        intentSettings.setClass(this, SettingsMenu.class);
        startActivity(intentSettings);
    }
	
	
	public void onClick(View v) {
		
		switch(v.getId()) {
		
			/* Presentation image */
			case R.id.presImage:
				if(controlPanelIsActive) {
					linearLayout.setVisibility(RelativeLayout.INVISIBLE);
					leftArrowBtn.setVisibility(RelativeLayout.INVISIBLE);
					rightArrowBtn.setVisibility(RelativeLayout.INVISIBLE);
					controlPanelIsActive = false;
				} else {
					linearLayout.setVisibility(RelativeLayout.VISIBLE);
					leftArrowBtn.setVisibility(RelativeLayout.VISIBLE);
					rightArrowBtn.setVisibility(RelativeLayout.VISIBLE);
					controlPanelIsActive = true;
				}
				break;
				
			/* Mike button */
			case R.id.mic:
				
				if(!micIsActive) {
					Bitmap image = bitmapImageFromRes(R.drawable.start_mic);
					microphoneBtn.setImageBitmap(image);
					
					startService(new Intent(this, MicService.class));
					micIsActive = true;
				} else {
					Bitmap image = bitmapImageFromRes(R.drawable.inactive_mic);
					microphoneBtn.setImageBitmap(image);
					
					stopService(new Intent(this, MicService.class));
					micIsActive = false;
				}
				break;
            case R.id.btnForward: nextSlide(); break;
            case R.id.btnBack: previousSlide(); break;

		}
	}
	


	/**
	 * Loads presentation slide image
	 *
	 * @param link - image link
	 * @return Image Bitmap if success and null otherwise
	 */
	public Bitmap loadImage(String link) {
		Bitmap imgBitmap = null;
		HttpURLConnection connection = null;

		try {
			URL url = new URL(link);
			connection = (HttpURLConnection) url.openConnection();
			connection.setDoInput(true);
			connection.connect();
			
			InputStream in = connection.getInputStream();
			imgBitmap = BitmapFactory.decodeStream(in);
			in.close();
			connection.disconnect();
			
		} catch (OutOfMemoryError e) {
			e.printStackTrace();
			return null;
		} catch (Exception e) {
			e.printStackTrace();
			if(connection != null)
				connection.disconnect();
			return null;
		} 
		
		return imgBitmap;
	}
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		MenuInflater inflater = getMenuInflater();
		inflater.inflate(R.menu.projector_menu, menu);
		boolean visibility = false;

		if(isSpeaker || KP.isChairman) {
			visibility = true;
		} else {
			visibility = false;
		}
			
		menu.findItem(R.id.showVideo).setVisible(visibility);
		menu.findItem(R.id.stopVideo).setVisible(visibility);
		menu.findItem(R.id.endPresentation).setVisible(visibility);
		
		return super.onCreateOptionsMenu(menu);
	}
	
	@Override
	public boolean onPrepareOptionsMenu(Menu menu) {
		boolean visibility = false;

		if(isSpeaker || KP.isChairman) {
			visibility = true;
		} else {
			visibility = false;
		}
			
		menu.findItem(R.id.showVideo).setVisible(visibility);
		menu.findItem(R.id.stopVideo).setVisible(visibility);
		menu.findItem(R.id.endPresentation).setVisible(visibility);
		
		return super.onPrepareOptionsMenu(menu);
	}
	
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		
		/* Menu items handling */
		switch(item.getItemId()) {
			case R.id.services:
				Intent intent = new Intent();
				intent.setClass(this, ServicesMenu.class);
				startActivity(intent);
				break;
			
			case R.id.settings:
				Intent intentSettings = new Intent();
				intentSettings.setClass(this, SettingsMenu.class);
				startActivity(intentSettings);
				break;
				
			case R.id.endPresentation:
				if(!checkConnection()) 
					return false;
				
				if(isSpeaker) {
                    linearLayout.setVisibility(RelativeLayout.INVISIBLE);
					leftArrowBtn.setVisibility(RelativeLayout.INVISIBLE);
					rightArrowBtn.setVisibility(RelativeLayout.INVISIBLE);
					presentationImage.setClickable(false);
				}
				isSpeaker = false;
				endPresentation();
				break;
			
			case R.id.reconnect:
				if(KP.reconnect() == 0)
					updateProjector();
				break;
				
			case R.id.projector_help:
				AlertDialog.Builder help_builder = 
						new AlertDialog.Builder(this);
				help_builder.setTitle(R.string.help_presentation);
				help_builder.setMessage(Html.fromHtml(getResources().
						getString(R.string.presentation_help_content)));
				help_builder.create();
				help_builder.show();
				break;
				
			case R.id.showVideo:
				try {
					final CharSequence[] titlesArray = KP.getVideoTitleList();
					final CharSequence[] urlsArray = KP.getVideoUuidList();

					if (titlesArray == null || urlsArray == null)
						break;
				
				/* Show dialog with video titles list */
					AlertDialog.Builder video_builder =
							new AlertDialog.Builder(this);
					video_builder.setTitle(R.string.showVideoTitle)
							.setItems(titlesArray, new DialogInterface.OnClickListener() {
										public void onClick(
												DialogInterface dialog, int which) {

											if (which < urlsArray.length && which >= 0) {

												if (urlsArray[which] != null) {
													String videoLink = prepareLink(
															urlsArray[which].toString());

													if (KP.startVideo(videoLink) != 0)
														Toast.makeText(getApplicationContext(),
                                                                "No video was found!",
                                                                Toast.LENGTH_SHORT).show();
												}
											}
										}
									});
					video_builder.create();
					video_builder.show();
				}
				catch(Exception e){e.printStackTrace();}
				break;
				
			case R.id.stopVideo:
				KP.stopVideo();
				break;
		}
		
		return super.onOptionsItemSelected(item);
	}
	
	/**
	 * Formatting URL as an absolute link
	 * 
	 * @param link to format
	 * @return Absolute link
	 */
	public String prepareLink(String link) {
		if(!link.contains("http://"))
			if(contentUrl != null)
				link = contentUrl + link;
		return link;
	}

	
	/**
	 * Switches to the next slide
	 * 
	 * @return 0 if success and -1 otherwise
	 */
	public int nextSlide() {
		if(slideNumber < slideCount)
			return KP.showSlide(slideNumber + 1);
		return -1;
	}
	
	/**
	 * Switches to the previous slide
	 * 
	 * @return 0 if success and -1 otherwise
	 */
	public int previousSlide() {
		if(slideNumber > 1)
			return KP.showSlide(slideNumber - 1);
		return -1;
	}
	
	/**
	 * Ends current presentation
	 * 
	 * @return 0 if success and -1 otherwise
	 */
	public int endPresentation() {
		return KP.endPresentation();
	}
	
	public void setSlideNumber(String number) {
		try {
			slideNumber = Integer.parseInt(number);
		} catch (NumberFormatException e) {
			e.printStackTrace();
		}
	}
	
	public void setSlideCount(String count) {
		try {
			slideCount = Integer.parseInt(count);
		} catch (NumberFormatException e) {
			e.printStackTrace();
		}
	}
	
	public void setSlideImage(String link) throws InterruptedException {
		if(!link.contains("http://"))
			slideImageLink = contentUrl + link;
		else
			slideImageLink = link;
	}
	
	/**
	 * Checks whether connection has been established
	 * @return
	 */
	public boolean checkConnection() {
		boolean state = KP.checkConnection();
		
		if(!state)
			Toast.makeText(this, R.string.connectionLost, 
					Toast.LENGTH_SHORT).show();
		return state;
	}
	
	/**
	 * Makes Bitmap object from resources
	 * 
	 * @param resId - resource identifier
	 * @return Bitmap object if success and null otherwise
	 */
	public Bitmap bitmapImageFromRes(int resId) {
		Bitmap image = null;
		BitmapFactory.Options options = new BitmapFactory.Options();
		options.inSampleSize = 1;
		image = BitmapFactory.decodeResource(getResources(), resId, options);
		
		return image;
	}
	
	/**
	 * Registers broadcast receiver which
	 * handles recovery process if connection 
	 * was lost; handles mike state.
	 */
	public void registerReceiver() {

		bcReceiver = new BroadcastReceiver() {
			public void onReceive(Context context, Intent intent) {
				boolean status = intent.getBooleanExtra(SERVICE_STATUS, true);
				
				/* If microphone service is not available */
				if(!status) {
					Bitmap image = bitmapImageFromRes(R.drawable.inactive_mic);
					microphoneBtn.setImageBitmap(image);
					micIsActive = false;
				}
				
				/* Handle recovery process */
				switch(intent.getIntExtra("action", 0)) {
					case NetworkService.START_RECOVER:
						presentationCreated = 0;
						progressDialog = ProgressDialog.show(context,
								getResources()
								.getString(R.string.connectionRecoverTitle), 
								getResources()
								.getString(R.string.connectionRecoverMsg), 
								false, false);
						break;
					
					case NetworkService.STOP_RECOVER:
						if(progressDialog != null)
							if(progressDialog.isShowing())
								progressDialog.dismiss();
						presentationCreated = 1;
						break;
					
					case NetworkService.FAIL_RECOVER:
						/*Toast.makeText(context, 
						 		R.string.connectionRecoverFail, 
								Toast.LENGTH_LONG).show();*/
						break;
						
					case NetworkService.OK_RECOVER:
						Toast.makeText(context, R.string.connectionRecoverOk, 
								Toast.LENGTH_LONG).show();
						break;
						
					default:
						break;
				}
			}
		};
				
		IntentFilter filter = new IntentFilter(BROADCAST_STATUS_SERVICE);
		filter.addAction(NetworkService.BC_RECOVERING_CONNECTION);
		registerReceiver(bcReceiver, filter);
	}
	
	/* TODO: incorrect transition if last state is using */
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event)  {
		if (keyCode == KeyEvent.KEYCODE_BACK) {
			final ActivityManager am = (ActivityManager) 
					getSystemService(Service.ACTIVITY_SERVICE);
		    
			final List<ActivityManager.RunningTaskInfo> tasks = 
		    		am.getRunningTasks(Integer.MAX_VALUE);
		    ActivityManager.RunningTaskInfo prevTask = new ActivityManager
		    		.RunningTaskInfo();
		    
		    for (final ActivityManager.RunningTaskInfo task : tasks) {
		        if (!task.topActivity.getPackageName().equals(
		        		"petrsu.smartroom.android.srclient.Projector")) {
		            prevTask = task;
		            
		        }
		    }

		    if(prevTask.getClass().equals(KP.class)) {
		    	Log.i("Previous task", "KP");
		    	AlertDialog.Builder dialog = new AlertDialog.Builder(this);
				dialog.setTitle(R.string.exitClientTitle);
				dialog.setMessage(R.string.exitClientQuestion);
				dialog.setPositiveButton(android.R.string.ok, 
						new DialogInterface.OnClickListener() {
					
					@Override
					public void onClick(DialogInterface dialog, int which) {
						presentationCreated = 0;
						KP.disconnectSmartSpace();
						KP.connectionState = -1;
						KP.isRegistered = false;
						KP.personIndex = -1;
						stopService(new Intent(getApplicationContext(), 
								NetworkService.class));
						finish();
					}
				});
				dialog.setNegativeButton(android.R.string.cancel, 
						new DialogInterface.OnClickListener() {
					
					@Override
					public void onClick(DialogInterface dialog, int which) {
						return;
					}
				});
				dialog.show();
		    }
		}
		
		return super.onKeyDown(keyCode, event);
	}
	
	/**
	 * 
	 * @author pavlin
	 *
	 *	Updates service view in background mode
	 */
	class updatePresentationAsync extends AsyncTask<Void, Void, Bitmap> {
		@Override
		protected Bitmap doInBackground(Void...empty) {
			return loadImage(slideImageLink);
		}
		
		@Override
		protected void onPostExecute(Bitmap slideImage) {
			String slideDigits = "0/0";
			int visibility = RelativeLayout.INVISIBLE;
			boolean clickable = false;
			
			controlPanelIsActive = false;
			
			if(speakerName != null) {
				setTitle("Speaker: "+speakerName );
				slideDigits = String.valueOf(slideNumber) + "/" + 
						String.valueOf(slideCount);
				
				if(isSpeaker || KP.isChairman) {
					controlPanelIsActive = true;
					visibility = RelativeLayout.VISIBLE;
					clickable = true;
				}
				
			} else {
				setTitle("No speakers");
				slideImage = bitmapImageFromRes(R.drawable.title);
				visibility = RelativeLayout.INVISIBLE;
				
				if(KP.isChairman)
					clickable = true;
			}
			
			textNumAndCount.setText(slideDigits);
			
			if(slideImage != null)
				presentationImage.setImageBitmap(slideImage);

			presentationImage.invalidate();
            linearLayout.setVisibility(visibility);
            leftArrowBtn.setVisibility(visibility);
			rightArrowBtn.setVisibility(visibility);
			microphoneBtn.setVisibility(visibility);
			presentationImage.setClickable(clickable);
			
			invalidateOptionsMenu();
		}
	}
}
