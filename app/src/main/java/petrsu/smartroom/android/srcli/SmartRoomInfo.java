package petrsu.smartroom.android.srcli;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlPullParserFactory;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Bundle;

/**
 * 
 * @author pavlin
 *
 *	Extracts SmartRoom info from *.sr file
 *	and sets to corresponding GUI fields
 */
public class SmartRoomInfo extends Activity {
	private final String ipTag = "ip";
	private final String portTag = "port";
	private String ip;		// SIB IP
	private String port;	// SIB port
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		Uri uri = getIntent().getData();

		try {
			parseData(uri);
			savePreferences();
		} catch (IOException e) {
			e.printStackTrace();
		} catch (XmlPullParserException e) {
			e.printStackTrace();
		}
		
		Intent intent = new Intent(this, KP.class);
		startActivity(intent);
		finish();
	}
	
	/**
	 * Parsing *.sr file data
	 * 
	 * @param link - path to *.sr file
	 * @throws XmlPullParserException
	 * @throws IOException
	 */
	public void parseData(Uri link) throws XmlPullParserException, IOException {
		InputStream inStream;
		
		/* Resolves file path */
		if(link.toString().contains("content:")) {
			inStream = getContentResolver().openInputStream(link);
		} else {
			String path = link.toString().replace("file://", "");
			File file = new File(path);
			inStream = new FileInputStream(file);
		}
		
		XmlPullParserFactory factory = XmlPullParserFactory.newInstance();
		XmlPullParser parser = factory.newPullParser();
		parser.setInput(inStream, org.apache.http.protocol.HTTP.UTF_8);
		
		int eventType = parser.getEventType();
		
		while(eventType != XmlPullParser.END_DOCUMENT) {
			if(eventType == XmlPullParser.START_TAG) {
				String name = parser.getName();
				eventType = parser.next();
				
				if(ipTag.equals(name))
					ip = parser.getText();
				else if(portTag.equals(name))
					port = parser.getText();
			}

			eventType = parser.next();
		}
	}
	
	/**
	 * Saves parsed data to app settings file
	 */
	public void savePreferences() {
		SharedPreferences prefs = getSharedPreferences("srclient_conf", 
				Context.MODE_PRIVATE);
		SharedPreferences.Editor editor = prefs.edit();
		
		editor.putString("ip", ip);
		editor.putString("port", port);
		editor.commit();
	}
}
