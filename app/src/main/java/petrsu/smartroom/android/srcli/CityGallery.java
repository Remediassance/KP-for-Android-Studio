package petrsu.smartroom.android.srcli;

import android.app.AlertDialog;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.support.v7.app.ActionBarActivity;
import android.support.v7.widget.Toolbar;
import android.text.Html;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.HorizontalScrollView;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import org.apache.commons.codec.binary.Hex;
import org.apache.commons.codec.digest.DigestUtils;
import org.apache.http.HttpEntity;
import org.apache.http.HttpResponse;
import org.apache.http.client.HttpClient;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.entity.BufferedHttpEntity;
import org.apache.http.impl.client.DefaultHttpClient;

import java.io.InputStream;
import java.net.URL;
import java.security.MessageDigest;
import java.util.ArrayList;

/**
 * Created by Remediassance on 26.03.2016.
 * Class for fruct demo.
 * Asks for city and returns its pictures
 */
public class CityGallery extends ActionBarActivity implements View.OnClickListener {

    private Button cityBtn;
    private EditText cityText;
    private TextView displayedCity;
    private String ipAddr = null;
    private String picName;
    private String md5Hex;
    private String uuid;
    private String city;
    private String url;
    private ListView lv;
    public static ProgressBar progressBar;

    /*=========================================================================
   *  IMPLEMENTATION OF ONCREATE LISTENER
   *==========================================================================
    */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.city_gallery);

        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);

        cityBtn = (Button) findViewById(R.id.searchBtn);
        cityBtn.setOnClickListener(this);

        cityText = (EditText) findViewById(R.id.cityTxt);

        displayedCity = (TextView) findViewById(R.id.displayedText);

        Navigation.getBasicDrawer(getApplicationContext(), this, toolbar);
        uuid = KP.getPersonUuid();
        city = "Petrozavodsk";



        ArrayList dataList = new ArrayList(20);
        try {
            if(KP.getPlaceInfo(city, uuid) != null) {
                dataList.clear();
                url = KP.getPlaceInfo(city, uuid);

                picName = url.substring(url.lastIndexOf("/")+1,url.indexOf("?"));
                md5Hex = new String(Hex.encodeHex(DigestUtils.md5(picName)));
                url = "https://upload.wikimedia.org/wikipedia/commons/thumb/"
                + md5Hex.substring(0,1) + "/"
                + md5Hex.substring(0,2) + "/"
                + picName + "/300px-"
                +picName;
                dataList.add(url);
            }
            else {
                dataList.clear();
                dataList.add("http://0.tqn.com/d/webclipart/1/0/5/l/4/floral-icon-5.jpg");
            }
        } catch(NullPointerException e){
            e.printStackTrace();
        }

        lv = (ListView) findViewById(R.id.listView);
        lv.setAdapter(new CityGalleryAdapter(this, dataList));

    }



    @Override
    protected void onPause() {
        super.onPause();
    }

    @Override
    protected void onStop() {
        super.onStop();
    }

    @Override
    protected void onStart() {
        super.onStart();
    }

    @Override
    protected void onResume() {
        super.onResume();
    }


    /**
     * =========================================================================
     * SHOWS HELP WINDOW
     * ==========================================================================
     */
    private void openHelp() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(R.string.joiningSR);
        builder.setMessage(Html.fromHtml(getResources().getString(R.string.agenda_help_content)));
        builder.create();
        builder.show();
    }

    /*=========================================================================
    *  IMPLEMENTATION OF ONCLICK LISTENER
    *==========================================================================
     */
    @Override
    public void onClick(View v) {
        ArrayList arrayList = null;

        city = cityText.getText().toString();
        displayedCity.setText(city);

        uuid = KP.getPersonUuid();
        url = KP.getPlaceInfo(city, uuid);

        picName = url.substring(url.lastIndexOf("/")+1,url.indexOf("?"));
        md5Hex = new String(Hex.encodeHex(DigestUtils.md5(picName)));
        url = "https://upload.wikimedia.org/wikipedia/commons/thumb/"
                + md5Hex.substring(0,1) + "/"
                + md5Hex.substring(0,2) + "/"
                + picName + "/300px-"
                +picName;

        Log.i("Full url is", url);


        arrayList = new ArrayList(50);

        if (url == null) {
            displayedCity.setText("Unable to recover images for " + city);
        } else {
            if(arrayList != null)
                arrayList.clear();
            arrayList.add(url);
            lv.setAdapter(new CityGalleryAdapter(this, arrayList));

        }
    }
}

