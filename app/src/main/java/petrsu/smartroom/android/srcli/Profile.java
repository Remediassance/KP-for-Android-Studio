package petrsu.smartroom.android.srcli;

import java.io.InputStream;
import java.net.URL;
import java.net.URLConnection;

import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;

import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;

import android.widget.EditText;
import android.widget.ImageView;
import android.widget.Toast;


/**
 * 
 * @author pavlin
 * @author remediassance
 *
 *	Class holds participant's information
 */
public class Profile extends AppCompatActivity {
	
	//private ActionBar actionBar;
	private ImageView imageView;
	private EditText nameEditor;
	private EditText phoneEditor;
	private String contentUrl;		// Content service URL
	private String name;
	private String image;
	private String phone;

	
	/* fields are not used
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

        String personUuid;
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
        try{
            if(getSupportActionBar()!=null)
            getSupportActionBar().setDisplayHomeAsUpEnabled(true);
        } catch (NullPointerException e){
            e.printStackTrace();
        }


		Navigation.getBasicDrawer(getApplicationContext(),this,toolbar);

		
		/* Show action bar if user is profile owner
		if(personUuid.equals(KP.getPersonUuid()))
			actionBar.show();
		else
			actionBar.hide();*/
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
