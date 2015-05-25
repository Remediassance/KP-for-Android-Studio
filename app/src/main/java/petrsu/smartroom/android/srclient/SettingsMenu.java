package petrsu.smartroom.android.srclient;

import android.app.AlertDialog;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.provider.Settings;
import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.support.v7.app.ActionBarActivity;
import android.support.v7.widget.Toolbar;
import android.text.Html;
import android.view.View;
import android.widget.AdapterView;
import android.widget.Toast;
import com.mikepenz.iconics.typeface.FontAwesome;
import com.mikepenz.materialdrawer.Drawer;
import com.mikepenz.materialdrawer.model.DividerDrawerItem;
import com.mikepenz.materialdrawer.model.PrimaryDrawerItem;
import com.mikepenz.materialdrawer.model.SecondaryDrawerItem;
import com.mikepenz.materialdrawer.model.SectionDrawerItem;
import com.mikepenz.materialdrawer.model.interfaces.IDrawerItem;

/**
 * 
 * @author pavlin
 *
 *	Sets user preferences
 */
public class SettingsMenu extends ActionBarActivity
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


    /**=========================================================================
     * OPENS BROWSER ON THE DOWNLOAD MANUAL PAGE
     * =========================================================================
     */
    private void gotoManual() {
        Intent browserIntent = new Intent(Intent.ACTION_VIEW,
                Uri.parse("http://sourceforge.net/projects/smartroom/files/clients/android/manual.pdf/download"));
        startActivity(browserIntent);
    }


    /**=========================================================================
     * GO TO CURRENT DISCUSSION
     *==========================================================================
     */
    private void gotoCurDisq(){
        Intent browserIntent = new Intent(Intent.ACTION_VIEW,
                Uri.parse("http://194.85.173.9:10011/listCategories"));
        startActivity(browserIntent);
    }


    /**=========================================================================
     * GO TO  DISCUSSION LIST
     *==========================================================================
     */
    private void gotoDisqList(){
        Intent browserIntent = new Intent(Intent.ACTION_VIEW,
                Uri.parse("http://194.85.173.9:8080/listCategories"));
        startActivity(browserIntent);
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
