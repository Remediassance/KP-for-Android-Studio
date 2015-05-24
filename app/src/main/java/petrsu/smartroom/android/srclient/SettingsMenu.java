package petrsu.smartroom.android.srclient;

import android.os.Bundle;
import android.preference.PreferenceManager;
import android.provider.Settings;
import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;

/**
 * 
 * @author pavlin
 *
 *	Sets user preferences
 */
public class SettingsMenu extends Activity
	implements SharedPreferences.OnSharedPreferenceChangeListener {

	public static final String TIMEOUT_SCREEN_PREF = "screen_timeout";
	public static final String SYSTEM_TIMEOUT_SCREEN_PREF = 
			"system_screen_timeout";
	public static final String SHOW_SPEAKER_NAME = "show_speakerName";
	public static final int defaultTimeout = 300000;
	
	@Override
	public void onResume() {
		super.onResume();
		
		SharedPreferences prefs = PreferenceManager
				.getDefaultSharedPreferences(this);
		prefs.registerOnSharedPreferenceChangeListener(this);
	}
	
	@Override
	public void onPause() {
		super.onPause();
		
		SharedPreferences prefs = PreferenceManager
				.getDefaultSharedPreferences(this);
		prefs.unregisterOnSharedPreferenceChangeListener(this);
	}
	
	@Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        getFragmentManager().beginTransaction()
        	.replace(android.R.id.content, new SettingsFragment())
        	.commit();
    }
	
	@Override
	public void onSharedPreferenceChanged(SharedPreferences preference, 
			String key) {
		
		/* Timeout preference changed */
		if(key.equals(TIMEOUT_SCREEN_PREF)) {
			
			int timeout = Integer.parseInt(
					preference.getString(key, String.valueOf(defaultTimeout)));
			
			android.provider.Settings.System.putInt(getContentResolver(), 
					Settings.System.SCREEN_OFF_TIMEOUT , timeout);
			
			SharedPreferences prefs = getSharedPreferences("srclient_conf", 
					Context.MODE_PRIVATE);
			SharedPreferences.Editor editor = prefs.edit();
			
			editor.putInt(TIMEOUT_SCREEN_PREF, timeout);
			editor.commit();
		
		/* Show speaker name preference changed
		 * NOTE: preference disabled */
		} else if(key.equals(SHOW_SPEAKER_NAME)) {
			
			boolean showSpeakerName = preference.getBoolean("show_speakerName", 
					true);
			SharedPreferences prefs = getSharedPreferences("srclient_conf", 
					Context.MODE_PRIVATE);
			SharedPreferences.Editor editor = prefs.edit();
			
			editor.putBoolean(SHOW_SPEAKER_NAME, showSpeakerName);
			editor.commit();
		}
	}
}
