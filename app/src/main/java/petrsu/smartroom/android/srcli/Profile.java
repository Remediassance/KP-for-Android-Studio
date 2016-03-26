package petrsu.smartroom.android.srcli;

import java.io.InputStream;
import java.net.URL;
import java.net.URLConnection;

import android.app.ActionBar;
import android.app.AlertDialog;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.support.v7.app.ActionBarActivity;
import android.support.v7.widget.Toolbar;
import android.text.Html;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.Toast;
import com.mikepenz.iconics.typeface.FontAwesome;
import com.mikepenz.materialdrawer.Drawer;
import com.mikepenz.materialdrawer.DrawerBuilder;
import com.mikepenz.materialdrawer.model.DividerDrawerItem;
import com.mikepenz.materialdrawer.model.PrimaryDrawerItem;
import com.mikepenz.materialdrawer.model.SecondaryDrawerItem;
import com.mikepenz.materialdrawer.model.SectionDrawerItem;
import com.mikepenz.materialdrawer.model.interfaces.IDrawerItem;

/**
 * 
 * @author pavlin
 * @author remediassance
 *
 *	Class holds participant's information
 */
public class Profile extends ActionBarActivity {
	
	private ActionBar actionBar;
	private ImageView imageView;
	private EditText nameEditor;
	private EditText phoneEditor;
	private String contentUrl;		// Content service URL
	private String name;
	private String image;
	private String phone;
	private String personUuid;
	
	/* TODO: fields are not used
	private String status;
	private String age;
	private String mbox;
	private String language;
	private String interests;
	private String organization;*/
	
	public Profile() {}
	
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);		
		setContentView(R.layout.profile);

		
		Intent intent = getIntent();
		int index = intent.getIntExtra("index", -1);
		
		personUuid = KP.loadProfile(this, index);
		System.out.println("Person UUID: " + personUuid);
		
		/* Checks whether user is online */
		if(personUuid == null) {
			Toast.makeText(getApplicationContext(), R.string.personIsOffline, 
					Toast.LENGTH_SHORT).show();
			finish();
			return;
		}
		
		contentUrl = KP.getContentUrl();
		nameEditor = (EditText) findViewById (R.id.nameValue);
		phoneEditor = (EditText) findViewById (R.id.phoneValue);
		imageView = (ImageView) findViewById (R.id.profileAvatar);
		
		nameEditor.setText(name);
		nameEditor.setEnabled(false);
		phoneEditor.setText(phone);
		phoneEditor.setEnabled(false);
		
		/* Loading user avatar */
		try {
			Thread thread = new Thread() {
				@Override
				public void run(){
					if(image != null) {
						if(!image.contains("http://"))
							image = contentUrl + image;
						imageView.setImageBitmap(loadImage(image));
					} else
						imageView.setImageResource(R.drawable.no_image);
				}
			};
		
			thread.start();
			thread.join();
		} catch (InterruptedException e) {
			e.printStackTrace();
		}

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

                        new SectionDrawerItem().withName(R.string.help),
                        new PrimaryDrawerItem().withName(R.string.manual).withIcon(FontAwesome.Icon.faw_download),

                        new DividerDrawerItem(),
                        new SecondaryDrawerItem().withName(R.string.exitClientTitle).withIcon(FontAwesome.Icon.faw_close)
                ).withOnDrawerItemClickListener(new Drawer.OnDrawerItemClickListener() {
            @Override
            public boolean onItemClick(AdapterView<?> parent, View view, int position, long id, IDrawerItem drawerItem) {
                //Toast.makeText(Agenda.this, String.valueOf(id), Toast.LENGTH_SHORT).show();
                switch ((int) id) {

                    case 1:
                        gotoAgenda();
                        break;
                    case 2:
                        gotoPresentation();
                        break;
                    case 4:
                        gotoSocialProgram();
                        break;
                    case 5:
                        gotoCurDisq();
                        break;
                    case 6:
                        gotoDisqList();
                        break;
                    case 8:
                        gotoSettings();
                        break;
                    case 10:
                        gotoManual();
                        break;
                    case 12:
                        exitApp();
                        break;
                    default:
                        break;

                }
                return true;
            }
        }).build();
		
		//actionBar = getActionBar();
		
		/* Show action bar if user is profile owner
		if(personUuid.equals(KP.getPersonUuid()))
			actionBar.show();
		else
			actionBar.hide();*/
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


    /**========================================================================
     * GO TO PRESENTATION SERVICE
     *=========================================================================
     */
    private void gotoPresentation(){
        Intent intent = new Intent();
        intent.setClass(this, Projector.class);
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
        intent.putExtra("url", KP.manLink);
        intent.putExtra("reading", true);

        startActivity(intent);
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


    /**=========================================================================
     * GO TO CURRENT DISCUSSION
     *==========================================================================
     */
    private void gotoCurDisq(){

        Intent intent = new Intent(getApplicationContext(), WebViewer.class);
        intent.putExtra("url", KP.dqAddr+"/?user_uuid="+KP.getPersonUuid());

        startActivity(intent);
    }


    /**=========================================================================
     * GO TO  DISCUSSION LIST
     *==========================================================================
     */
    private void gotoDisqList(){

        Intent intent = new Intent(getApplicationContext(), WebViewer.class);
        intent.putExtra("url",KP.dqAddr+"/listCurrentThreads/?user_uuid="+KP.getPersonUuid());

        startActivity(intent);
    }



    /**=========================================================================
     * GO TO  SOCIAL PROGRAM
     *==========================================================================
     */
    private void gotoSocialProgram(){

        Intent intent = new Intent(getApplicationContext(), WebViewer.class);
        intent.putExtra("url",KP.spAddr);

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



	public void setName(String name) {
		this.name = name;
	}
	
	public void setPhone(String phone) {
		this.phone = phone;
	}
	
	public void setImage(String image) {
		this.image = image;
	}
	
	/**
	 * Loads an image by link
	 * 
	 * @param link - image URL
	 * @return Bitmap image if success and null otherwise
	 */
	public Bitmap loadImage(String link) {
		BitmapFactory.Options options = new BitmapFactory.Options();
		options.inSampleSize = 2;
		
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
		inflater.inflate(R.menu.profile, menu);		
		return super.onCreateOptionsMenu(menu);
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch(item.getItemId()) {
			case R.id.action_edit:
				item.setCheckable(true);
				
				if(!item.isChecked()) {
					item.setChecked(true);
					item.setIcon(R.drawable.ic_edit_checked);
					nameEditor.setEnabled(true);
					phoneEditor.setEnabled(true);
				} else {
					item.setChecked(false);
					item.setIcon(R.drawable.ic_edit);
					nameEditor.setEnabled(false);
					phoneEditor.setEnabled(false);
				}
				break;
				
			case R.id.action_save:
				String newName = nameEditor.getText().toString();
				String newPhone = phoneEditor.getText().toString();
				
				nameEditor.setEnabled(false);
				phoneEditor.setEnabled(false);
				KP.saveProfileChanges(newName, newPhone);
				break;
		}
		return super.onOptionsItemSelected(item);
	}
}
