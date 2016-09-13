package petrsu.smartroom.android.srcli;

import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.widget.*;

import java.util.ArrayList;

import android.text.Html;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.content.ActivityNotFoundException;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.URL;
import java.net.URLConnection;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;

import com.mikepenz.iconics.typeface.FontAwesome;
import com.mikepenz.materialdrawer.Drawer;
import com.mikepenz.materialdrawer.DrawerBuilder;
import com.mikepenz.materialdrawer.model.DividerDrawerItem;
import com.mikepenz.materialdrawer.model.PrimaryDrawerItem;
import com.mikepenz.materialdrawer.model.SecondaryDrawerItem;
import com.mikepenz.materialdrawer.model.SectionDrawerItem;
import com.mikepenz.materialdrawer.model.interfaces.IDrawerItem;

import java.lang.Thread;

/**
 *
 * @author pavlin
 * @author remediassance
 *
 * Agenda class shows activity program and
 * handles events like start conference, end
 * presentation, etc.
 *
 * TODO: обрабатывать вылет при попытке залогиниться, когда секция пустая
 */
public class Agenda extends AppCompatActivity {// implements  View.OnClickListener{

	private final int LOOK_PRESENTATION = 0;
	private final int PERSON_PROFILE = 1;
	private final int START_CONFERENCE_FROM = 2;
	private final int REMOVE_SPEECH = 3;
	private final String CUR_TIMESLOT_KEY = "curTimeslot";
	private final String absentImg;				// Absent image constant
	private final String noImage;				// No image constant
	private final String contentUrl;			// Content service URL
	public final String presentationPath;		// Presentation local directory

	private static ArrayList<Timeslot> list;	// Agenda program
	private static AgendaAdapter adapter;
	private static ListView listView;
	public static int agendaCreated;			// Agenda created indicator
	public static int currentTimeslotIndex;

	private BroadcastReceiver bc;
	private Bitmap imgDefault;
	private Bitmap imgNoImage;
	protected ProgressDialog progressDialog;
	public boolean conferenceStarted;
	public boolean conferenceEnded;

	private ImageButton addSpeechBtn;



	public Agenda() {
		contentUrl = KP.getContentUrl();
		agendaCreated = 0;
		conferenceStarted = false;
		conferenceEnded = false;
		absentImg = "absentImage";
		noImage = "noImage";
		presentationPath = "/SmartRoom/Presentations/";
        KP.dqAddr = KP.getDiscussionServiceIP();
        KP.spAddr = KP.getSocialProgramServiceIP();
	}

	@Override
	protected void onPause() {
		super.onPause();
		unregisterReceiver(bc);
		agendaCreated = 0;
	}

	@Override
	protected void onStop() {
		super.onStop();
	}

	@Override
	protected void onStart() {
		super.onStart();
		int value = KP.getCurrentTimeslotIndex() - 1;
		
		/* If section was changed */
		if(KP.sectionChanged(KP.isMeetingMode))
			updateAgenda();

		/* If time slot index was changed */
		if(value != currentTimeslotIndex)
			updateCurTimeslot();
	}

	@Override
	protected void onResume() {
		super.onResume();
		SharedPreferences prefs = getSharedPreferences("srclient_conf",Context.MODE_PRIVATE);
		SharedPreferences.Editor editor = prefs.edit();
		
		/* Save the last application state */
		editor.putString("last_state", "Agenda");
		editor.apply(); // was commit()

		registerReceiver();
		agendaCreated = 1;
	}

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.agenda_interface);

		/*
		* Button that sends new speech request to meeting service
		 */
		addSpeechBtn = (ImageButton) findViewById(R.id.addSpeechBtn);
		addSpeechBtn.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				Toast.makeText(getApplicationContext(),"Sent new speech notification",Toast.LENGTH_LONG);
			}
		});

        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
		try {
			setSupportActionBar(toolbar);
            if(getSupportActionBar() != null)
			    getSupportActionBar().setDisplayHomeAsUpEnabled(true);
		} catch(NullPointerException e){
			e.printStackTrace();
		}

		new DrawerBuilder()
				.withActivity(this)
				.withToolbar(toolbar)
				.withActionBarDrawerToggle(true)
				.withHeader(R.layout.drawer_header)
				.withDrawerWidthDp(320)
				.addDrawerItems(
						new SectionDrawerItem().withName(R.string.services),
						new PrimaryDrawerItem().withName(R.string.agenda).withIcon(FontAwesome.Icon.faw_server),
						new PrimaryDrawerItem().withName(R.string.presentation).withIcon(FontAwesome.Icon.faw_image),
                        new PrimaryDrawerItem().withName(R.string.socialservice).withIcon(FontAwesome.Icon.faw_globe),
						new PrimaryDrawerItem().withName(R.string.welcome).withIcon(FontAwesome.Icon.faw_group),

                        new SectionDrawerItem().withName(R.string.discussion),

                        new PrimaryDrawerItem().withName(R.string.discussionCur).withIcon(FontAwesome.Icon.faw_comment_o),
                        new PrimaryDrawerItem().withName(R.string.discussionList).withIcon(FontAwesome.Icon.faw_comments_o),

						new SectionDrawerItem().withName(R.string.action_settings),
						new PrimaryDrawerItem().withName(R.string.action_settings).withIcon(FontAwesome.Icon.faw_cog),
						new PrimaryDrawerItem().withName(R.string.reconnectText).withIcon(FontAwesome.Icon.faw_refresh),

						new SectionDrawerItem().withName(R.string.help),
						new PrimaryDrawerItem().withName(R.string.help_agenda).withIcon(FontAwesome.Icon.faw_info),
						new PrimaryDrawerItem().withName(R.string.manual).withIcon(FontAwesome.Icon.faw_download),

						new DividerDrawerItem(),//TODO ДОПИШИ ВХОД В ВЕЛКОМ ОТОВСЮДУ
						//new PrimaryDrawerItem().withName("City gallery *WIP*").withIcon(FontAwesome.Icon.faw_globe),
						new SecondaryDrawerItem().withName(R.string.exitClientTitle).withIcon(FontAwesome.Icon.faw_close)
                ).withOnDrawerItemClickListener(new Drawer.OnDrawerItemClickListener() {
            @Override
            public boolean onItemClick(AdapterView<?> parent, View view, int position, long id, IDrawerItem drawerItem) {
                //Toast.makeText(Agenda.this, String.valueOf(id), Toast.LENGTH_SHORT).show();
                switch ((int) id) {
					case 1:
						break;
					case 2:
						startActivity(Navigation.getPresentationIntent(getApplicationContext()));
						break;
					case 3:
						startActivity(Navigation.getSocialProgramIntent(getApplicationContext()));
						break;
					case 4:
						startActivity(Navigation.getSocialProgramIntent(getApplicationContext()));
						break;
					case 6:
						startActivity(Navigation.getCurDisqIntent(getApplicationContext()));
						break;
					case 7:
						startActivity(Navigation.getDisqListIntent(getApplicationContext()));
						break;
					case 9:
						startActivity(Navigation.getSettingsIntent(getApplicationContext()));
						break;
                    case 10:
                        updateCurTimeslot();
                        break;
                    case 12:
                        openHelp();
                        break;
                    case 13:
						startActivity(Navigation.getManIntent(getApplicationContext()));
                        break;
                    case 15:
						startActivity(Navigation.exitApp());
                        break;
                    default:
                        break;
                }
                return true;
            }
        }).build();

		if(KP.isMeetingMode)
			addSpeechBtn.setVisibility(RelativeLayout.VISIBLE);
		else
			addSpeechBtn.setVisibility(RelativeLayout.INVISIBLE);
		
		/* Initialize context menu */
		initListView();

		BitmapFactory.Options options = new BitmapFactory.Options();
		options.inSampleSize = 1;
		imgDefault = BitmapFactory.decodeResource(getResources(),R.drawable.crop, options);
		imgNoImage = BitmapFactory.decodeResource(getResources(),R.drawable.crop, options);
		
		/* Initialize progress dialog for showing download 
		 * process */
		progressDialog = createProgressDialog();
		
		/* If started the first time */
		if(list == null) {
			if(prepareAgendaData() != 0) {
				setContentView(R.layout.agenda_interface_ext);

				ImageView refreshBtn = (ImageView) findViewById (R.id.agendaRefresh);
				refreshBtn.setImageDrawable(getResources().getDrawable(R.drawable.refresh));
				refreshBtn.setOnClickListener(new View.OnClickListener() {
					@Override
					public void onClick(View v) {
						//ImageButton image = (ImageButton) v.findViewById(R.id.agendaRefresh);
						switch(v.getId()) {
							case R.id.agendaRefresh:
								updateAgenda();
								break;
						}
					}
				});
				return;
			}
		}
		
		/* If agenda is empty show error window */
		if(list.isEmpty()) {
			ArrayList<ErrorView> list = new ArrayList<>();
			list.add(new ErrorView(getResources()
					.getString(R.string.agendaNotAvailable)));

			adapter = new AgendaAdapter(
					this, list, R.layout.agenda_interface_ext,
					new String[] {ErrorView.MSG},
					new int[] {R.id.noAgendaView});
		} else {
			adapter = new AgendaAdapter(
					this, list, R.layout.agenda_item,
					new String[] {Timeslot.NAME, Timeslot.TITLE,
							Timeslot.IMG, Timeslot.STATUS},
					new int[] {R.id.speakerName, R.id.presentationTitle,
							R.id.avatar, R.id.speakerStatus});
		}

        // was cast to SimpleAdapter
		(adapter).setViewBinder(new AgendaViewBinder());
		listView.setAdapter(adapter);
	}



	/**=========================================================================
	 * SHOWS HELP WINDOW
	 *==========================================================================
	 */
	private void openHelp() {
		AlertDialog.Builder builder = new AlertDialog.Builder(this);
		builder.setTitle(R.string.joiningSR);
		builder.setMessage(Html.fromHtml(getResources().getString(R.string.agenda_help_content)));
		builder.create();
		builder.show();
	}



	/**========================================================================
	 * REFRESH AGENDA PAGE
	 *=========================================================================
	 */
	public void updateAgenda() {
		agendaCreated = 0;
		list = null;
		KP.personIndex = KP.personTimeslotIndex(KP.isMeetingMode);
		finish();
		Intent restartIntent = new Intent(this, Agenda.class);
		startActivity(restartIntent);
	}



	/**
	 * Updates agenda time slot
	 */
	public void updateCurTimeslot() {
		int index = KP.getCurrentTimeslotIndex() - 1;
		setCurrentTimeslot(index);

		/* Update agenda view */
		try {
			new updateAgendaAsync(index).execute();
		} catch(ExceptionInInitializerError e) {
			e.printStackTrace();
			updateAgenda();
		}

		/* Triggers to Projector if user is speaker */
		if(KP.checkSpeakerState(KP.isMeetingMode)) {
			Intent intent = new Intent();
			intent.setClass(this, Projector.class);
			startActivity(intent);
		}
	}



	@Override
	protected void onSaveInstanceState(Bundle outState) {
		super.onSaveInstanceState(outState);

		/* Save current time slot index */
		outState.putInt(CUR_TIMESLOT_KEY, currentTimeslotIndex);
	}


	@Override
	protected void onRestoreInstanceState(Bundle savedInstanceState) {
		super.onRestoreInstanceState(savedInstanceState);
	    
	    /* Load saved time slot index */
		currentTimeslotIndex = savedInstanceState.getInt(CUR_TIMESLOT_KEY, -1);
	}


	/**=============================================================================================
	 * Adds time slot item to agenda program list.
	 *
	 * @param name Name of participant
	 * @param title Presentation title
	 * @param img Participant's avatar
	 * @param status Participant status (online, offline)
	 * @throws InterruptedException
     * =============================================================================================
	 */
	public void addTimeslotItemToList(final String name, final String title,
									  final String img, final String status) throws InterruptedException {
		if(!img.equals(absentImg) && !img.equals(noImage)) {
			Thread t = new Thread() {
				@Override
				public void run() {
					String imageLink = prepareLink(img);
					Bitmap imgAvatar = loadImage(imageLink);
					if(imgAvatar == null) {
						list.add(new Timeslot(name, title, imgNoImage,
								status));
					} else {
						list.add(new Timeslot(name, title, imgAvatar,
								status));
					}
				}
			};
			t.start();
			t.join();

		} else if(img.equals(noImage)) {
			list.add(new Timeslot(name, title, imgNoImage, status));
		} else {
			list.add(new Timeslot(name, title, imgDefault, status));
		}
	}


	/**
	 * Loads image by URL.
	 *
	 * @param link Image link
	 * @return Image Bitmap if success and null otherwise
	 */
	synchronized public Bitmap loadImage(String link) {
		BitmapFactory.Options options = new BitmapFactory.Options();
		options.inSampleSize = 1;
		Bitmap image;

		try {
			URLConnection url = new URL(link).openConnection();
			url.setConnectTimeout(2000);

			InputStream is = (InputStream) url.getContent();
			image = BitmapFactory.decodeStream(is, null, options);
		} catch (OutOfMemoryError e) {
			e.printStackTrace();
			return null;
		} catch (Exception e) {
			e.printStackTrace();
			return null;
		}

		return image;
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		MenuInflater inflater = getMenuInflater();
		inflater.inflate(R.menu.agenda_menu, menu);

		if(!KP.isChairman) {
			menu.findItem(R.id.conferenceStart).setVisible(false);
			menu.findItem(R.id.conferenceEnd).setVisible(false);
		}

		return super.onCreateOptionsMenu(menu);
	}

	@Override
	public boolean onPrepareOptionsMenu(Menu menu) {
		menu.findItem(R.id.conferenceStart).setEnabled(true);
		menu.findItem(R.id.conferenceEnd).setEnabled(true);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch(item.getItemId()) {
			case R.id.services:
				Intent intentServices = new Intent();
				intentServices.setClass(this, ServicesMenu.class);
				startActivity(intentServices);
				break;

			case R.id.settings:
				Intent intentSettings = new Intent();
				intentSettings.setClass(this, SettingsMenu.class);
				startActivity(intentSettings);
				break;

			case R.id.conferenceStart:
				if(!checkConnection())
					return false;

				if(startConference() != 0)
					Toast.makeText(this, "Start conference failed",
							Toast.LENGTH_SHORT).show();
				conferenceStarted = true;
				break;

			case R.id.conferenceEnd:
				if(!checkConnection())
					return false;

				if(endConference() != 0)
					Toast.makeText(this, "End conference failed",
							Toast.LENGTH_SHORT).show();
				conferenceEnded = true;
				conferenceStarted = false;
				break;

			case R.id.reconnect:
				if(KP.reconnect() == 0)
					updateAgenda();
				break;

			case R.id.agenda_help:
				/* Show help dialog */
				AlertDialog.Builder builder = new AlertDialog.Builder(this);
				builder.setTitle(R.string.help_agenda);
				builder.setMessage(Html.fromHtml(getResources()
						.getString(R.string.agenda_help_content)));
				builder.create();
				builder.show();
				break;
		}

		return super.onOptionsItemSelected(item);
	}



	/**
	 * Tells content service to start conference
	 *
	 * @return 0 if success and -1 otherwise
	 */
	public int startConference() {
		return KP.startConference();
	}

	/**
	 * Tells content service to stop conference
	 *
	 * @return 0 if success and -1 otherwise
	 */
	public int endConference() {
		return KP.endConference();
	}



	/**
	 * Loads agenda program
	 *
	 * @return 0 in success and -1 otherwise
	 */
	public int prepareAgendaData() {

		list = new ArrayList<>();
		
		/* If loading program failed */
		if(KP.loadTimeslotList(this, KP.isMeetingMode) == -1) {
			Log.i("Agenda GUI", "Fill agenda fail");
			return -1;
		}

		return 0;
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event)  {
		if (keyCode == KeyEvent.KEYCODE_BACK) {
			AlertDialog.Builder dialog = new AlertDialog.Builder(this);
			dialog.setTitle(R.string.exitClientTitle);
			dialog.setMessage(R.string.exitClientQuestion);
			dialog.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
				@Override
				public void onClick(DialogInterface dialog, int which) {
					stopService(new Intent(getApplicationContext(),
							NetworkService.class));
					KP.disconnectSmartSpace();
					KP.connectionState = -1;
					KP.isRegistered = false;
					KP.isChairman = false;
					KP.personIndex = -1;
					list = null;
					currentTimeslotIndex = -1;
					/*
					 * This thing below is always false. Don't quite know why this was done this way
					 */
					/*if(list != null)
						list.clear();*/
					finish();
				}
			});

			dialog.setNegativeButton(android.R.string.cancel,
					new DialogInterface.OnClickListener() {

						@Override
						public void onClick(DialogInterface dialog, int which) {
							//return;
						}
					});
			dialog.show();
		}

		return super.onKeyDown(keyCode, event);
	}

	/**
	 * Sets current time slot index value
	 *
	 * @param index Time slot index value
	 */
	public void setCurrentTimeslot(int index) {
		currentTimeslotIndex = index;
	}

	/**
	 * Checks whether connection is active
	 *
	 * @return True if connection established and false otherwise
	 */
	public boolean checkConnection() {
		boolean state = KP.checkConnection();

		if(!state)
			Toast.makeText(this, R.string.connectionLost,
					Toast.LENGTH_SHORT).show();
		return state;
	}

	/**
	 * Initializes agenda context menu
	 */
	public void initListView() {
		listView = (ListView) findViewById (R.id.agendaListView);
		listView.setOnItemLongClickListener(new AdapterView.OnItemLongClickListener() {
			public boolean onItemLongClick(AdapterView<?> parent, View view, int position, long id) {
				final int pos = position;
				int agendaContextMenu = R.array.agenda_action_user;

				if (KP.isChairman)
					agendaContextMenu = R.array.agenda_action_chairman;

				AlertDialog.Builder builder = new AlertDialog.Builder(Agenda.this);
				builder.setTitle(R.string.chooseAction);
				builder.setItems(agendaContextMenu, new DialogInterface.OnClickListener() {
					@Override
					public void onClick(DialogInterface dialog, int which) {
						switch (which) {
							case LOOK_PRESENTATION:
								String link = KP.getPresentationLink(pos, KP.isMeetingMode);
								if (link != null) {
									Uri uri = Uri.parse(prepareLink(link));

									if (openRemotePresentation(uri) != 0) {
										showDownloadDialog(uri);
									}
								} else {
									Toast.makeText(getApplicationContext(),
											R.string.presUnreach,
											Toast.LENGTH_SHORT).show();
								}
								break;

							case PERSON_PROFILE:
								Intent intent = new Intent(getApplicationContext(), Profile.class);
								intent.putExtra("index", pos);
								startActivity(intent);
								break;

							case REMOVE_SPEECH:
								Toast.makeText(getApplicationContext(),"Speech was removed",Toast.LENGTH_LONG);
								break;

							case START_CONFERENCE_FROM:
								KP.startConferenceFrom(pos, KP.isMeetingMode);
								break;
						}
					}
				});

				AlertDialog dialog = builder.create();
				dialog.setCanceledOnTouchOutside(true);
				dialog.show();
				return true;
			}
		});
	}

	/**
	 * Registers broadcast receiver for showing
	 * recovering connection process
	 */
	public void registerReceiver() {
		bc = new BroadcastReceiver() {
			@Override
			public void onReceive(Context context, Intent intent) {
				switch(intent.getIntExtra("action", 0)) {
					
					/* Show dialog if recovering started */
					case NetworkService.START_RECOVER:
						agendaCreated = 0;
						progressDialog = ProgressDialog.show(context,
								getResources()
                                        .getString(R.string.connectionRecoverTitle),
								getResources()
                                        .getString(R.string.connectionRecoverMsg),
								false, false);
						break;
						
					/* Close dialog if recovering stopped */
					case NetworkService.STOP_RECOVER:
						if(progressDialog != null)
							if(progressDialog.isShowing())
								progressDialog.dismiss();
						break;
					
					/* If recovering failed */
					case NetworkService.FAIL_RECOVER:
                        Toast.makeText(context, "Failed to recover",
                                Toast.LENGTH_LONG).show();
						break;
						
					/* If recovering successful */
					case NetworkService.OK_RECOVER:
						Toast.makeText(context, R.string.connectionRecoverOk,
								Toast.LENGTH_LONG).show();
						break;
				}
			}
		};
		IntentFilter filter = new IntentFilter(
				NetworkService.BC_RECOVERING_CONNECTION);
		this.registerReceiver(bc, filter);
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
	 * Shows dialog of presentation downloading options 
	 *
	 * @param uri - presentation resource identifier
	 */
	public void showDownloadDialog(final Uri uri) {
		String fileName = uri.toString();
		fileName = fileName.substring(fileName.lastIndexOf("/") + 1);

		if(fileExists(fileName, presentationPath)) {
			showRewriteDialog(uri);
			return;
		}

		AlertDialog.Builder builder = new AlertDialog.Builder(this);
		builder.setTitle(R.string.presDownloadTitle);
		builder.setMessage(R.string.presDownload);
		builder.setPositiveButton(android.R.string.ok,
				new DialogInterface.OnClickListener() {
					@Override
					public void onClick(DialogInterface dialog, int which) {
				
				/* Show download progress dialog */
						new DownloadFileProgress().execute(uri);
					}
				});
		builder.setNegativeButton(android.R.string.cancel, null);

		AlertDialog dialog = builder.create();
		dialog.show();
	}

	/**
	 * Shows the dialog for rewriting downloaded presentation
	 *
	 * @param uri - presentation identifier
	 */
	public void showRewriteDialog(final Uri uri) {
		AlertDialog.Builder builder = new AlertDialog.Builder(this);
		builder.setTitle(R.string.presDownloadTitle);
		builder.setMessage(R.string.presRewriteDownload);
		builder.setPositiveButton(android.R.string.yes,
				new DialogInterface.OnClickListener() {
					@Override
					public void onClick(DialogInterface dialog, int which) {
						new DownloadFileProgress().execute(uri);
					}
				});
		builder.setNegativeButton(android.R.string.no,
				new DialogInterface.OnClickListener() {
					@Override
					public void onClick(DialogInterface dialog, int which) {
						openLocalPresentation(uri);
					}
				});

		AlertDialog dialog = builder.create();
		dialog.show();
	}

	/**
	 * Creates progress dialog
	 *
	 * @return Progress dialog object
	 */
	public ProgressDialog createProgressDialog() {
		progressDialog = new ProgressDialog(this);
		progressDialog.setMessage(getResources()
				.getString(R.string.presDownloadTitle));
		progressDialog.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
		progressDialog.setIndeterminate(false);
		progressDialog.setCancelable(true);

		return progressDialog;
	}

	/**
	 * Checks whether file exists in local storage
	 *
	 * @param fileName - name of a file
	 * @param subPath - local path to presentation
	 * @return True if file exists and false otherwise
	 */
	public static boolean fileExists(String fileName, String subPath) {
		File file = new File(Environment.getExternalStorageDirectory()
				.getPath() + subPath + fileName);
		return file.exists();
	}

	/**
	 * Opens presentation by URI if appropriate
	 * application installed
	 *
	 * @param uri - presentation identifier
	 * @return 0 in success and -1 otherwise
	 */
	public int openRemotePresentation(Uri uri) {
		Intent intent = new Intent(Intent.ACTION_VIEW);
		intent.setDataAndType(uri, "application/pdf");

		try {
			startActivity(intent);
		} catch (ActivityNotFoundException e) {
			e.printStackTrace();
			return -1;
		}
		return 0;
	}

	/**
	 * Opens local presentation which was
	 * downloaded earlier
	 *
	 * @param uri - presentation identifier
	 * @return 0 in success and -1 otherwise
	 */
	public int openLocalPresentation(Uri uri) {
		String fileName = uri.toString().substring(uri.toString().
				lastIndexOf('/') + 1);
		try {
			File file = new File(Environment.getExternalStorageDirectory()
                    .getPath() + presentationPath + fileName);
			Intent intent = new Intent(Intent.ACTION_VIEW);
			intent.setDataAndType(Uri.fromFile(file), "application/pdf");
			startActivity(intent);
		} catch (ActivityNotFoundException e) {
			e.printStackTrace();
			return -1;
		}

		return 0;
	}


	/**
	 *
	 * @author pavlin
	 *
	 *	Shows downloading progress dialog.
	 */
	class DownloadFileProgress extends AsyncTask<Uri, Integer, Long> {
		private String fileName;	// Presentation file name

		protected void onPreExecute() {
			super.onPreExecute();
			progressDialog.show();
			makeDir();
		}

		protected Long doInBackground(Uri...uris) {
			byte data[] = new byte[1024];
			long total = 0;
			int count;

			try {
				URL url = new URL(uris[0].toString());
				fileName = url.toString().substring(url.toString().
						lastIndexOf('/') + 1);

				if(Agenda.fileExists(fileName, presentationPath))
					return total;

				URLConnection connection = url.openConnection();
				connection.connect();
				progressDialog.setMax(connection.getContentLength());

				InputStream input = new BufferedInputStream(
						url.openStream(), 8192);

				OutputStream output = new FileOutputStream(Environment.
						getExternalStorageDirectory().getPath() +
						presentationPath + fileName);
		        
		        /* Downloading and updating progress */
				while ((count = input.read(data)) != -1) {
					total += count;
					publishProgress((int)total);
					output.write(data, 0, count);
				}

				output.flush();
				output.close();
				input.close();
			} catch (IOException e) {
				e.printStackTrace();
			}

			return total;
		}

		protected void onProgressUpdate(Integer...progress) {
			progressDialog.setProgress(progress[0]);
            //prog 0 was cast to int
		}

		protected void onPostExecute(Long result) {
			progressDialog.dismiss();
			try {
				File file = new File(Environment.getExternalStorageDirectory()
                        .getPath() + presentationPath + fileName);
				Intent intent = new Intent(Intent.ACTION_VIEW);
				intent.setDataAndType(Uri.fromFile(file), "application/pdf");
				startActivity(intent);
			} catch (ActivityNotFoundException e) {
				e.printStackTrace();
			}
		}

		/**
		 * Creates local directory for storing
		 * downloaded presentations
		 */
		private void makeDir() {
			File dir = new File(Environment.getExternalStorageDirectory().getPath() + presentationPath);
            boolean isDirCreated = dir.exists();
			if(!isDirCreated)
                isDirCreated = dir.mkdirs();
            if(isDirCreated)
                System.out.println("Dirs are already created");
		}
	}

	/**
	 *
	 * @author pavlin
	 *
	 *	Updates agenda current time slot view
	 */
	class updateAgendaAsync extends AsyncTask<Void, Void, Void> {
		private int index;	// time slot index value

		public updateAgendaAsync(int index) {
			super();
			this.index = index;
		}

		@Override
		protected Void doInBackground(Void...empty) {
			return null;
		}

		@Override
		protected void onPostExecute(Void empty) {
			if(adapter != null) {
				adapter.updateHighlight();
				listView.smoothScrollToPosition(index);
				listView.invalidate();
			}
			else {
                startActivity(Navigation.getSocialProgramIntent(getApplicationContext()));
                Toast.makeText(getApplicationContext(), "No conference detected. Redirected to social program.", Toast.LENGTH_LONG).show();

            }

		}
	}
}