package petrsu.smartroom.android.srcli;

import android.app.AlertDialog;
import android.app.Application;
import android.os.Bundle;
import android.support.v7.app.ActionBarActivity;
import android.support.v7.widget.Toolbar;
import android.text.Html;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import org.apache.commons.codec.binary.Hex;
import org.apache.commons.codec.digest.DigestUtils;
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
    public static String city;
    private String url;
    private String cityDesc;
    private String cityFD;
    private ListView lv;
    public static ProgressBar progressBar;

    /*=========================================================================
   *  IMPLEMENTATION OF ONCREATE
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
        uuid = KP.getPersonUuid();//TODO В агенде неверный линк в велком

        for (int i = 0; i < 5; i++)
            if (city == null) {
                city = KP.getCityByPersonUuid(uuid);
            }

        Log.i("CityGallery():","City title is " + city);


        ArrayList dataList = new ArrayList(3);
        try {
            url = KP.getPlacePhoto(city, uuid);
            cityDesc = KP.getPlaceDescription(city);
            cityFD = KP.getPlaceFoundingDate(city);

            if (url != null) {
                dataList.clear();
                picName = url.substring(url.lastIndexOf("/") + 1, url.indexOf("?"));
                md5Hex = new String(Hex.encodeHex(DigestUtils.md5(picName)));
                url = "https://upload.wikimedia.org/wikipedia/commons/thumb/"
                        + md5Hex.substring(0, 1) + "/"
                        + md5Hex.substring(0, 2) + "/"
                        + picName + "/300px-"
                        + picName;
                dataList.add(url);
            } else {
                dataList.clear();
                dataList.add("http://0.tqn.com/d/webclipart/1/0/5/l/4/floral-icon-5.jpg");
            }

            //TODO аналогичные проверки для описания и даты основания
            if(cityDesc != null){
                dataList.add(cityDesc);
            }
            else dataList.add("NaN");

            if(cityFD != null){
                dataList.add(cityFD);
            }
            else dataList.add("Oops! Description was lost :(");

        } catch (NullPointerException e) {
            e.printStackTrace();
        }

        lv = (ListView) findViewById(R.id.listView);
        lv.setAdapter(new CityGalleryAdapter(this, dataList, CityGallery.this));

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


    public static boolean isCityNameCorrect(String city) {
        String _city = city.toLowerCase();
        if (_city.matches("[A-Za-z]+"))
            return true;
        else return false;
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
    * TODO: Получение описания города и даты основания.
    * TODO: Перепилить интерфейс сервиса с учетом последних правок
     */
    @Override
    public void onClick(View v) {
        ArrayList arrayList = null;

        city = cityText.getText().toString();
        if (isCityNameCorrect(city)) {
            displayedCity.setText(R.string.cityis + " " + city);

            uuid = KP.getPersonUuid();
            KP.setCity(uuid, city);
            url = KP.getPlacePhoto(city, uuid);
            cityDesc = KP.getPlaceDescription(city);
            cityFD = KP.getPlaceFoundingDate(city);

            if (url != null) {
                picName = url.substring(url.lastIndexOf("/") + 1, url.indexOf("?"));
                md5Hex = new String(Hex.encodeHex(DigestUtils.md5(picName)));
                url = "https://upload.wikimedia.org/wikipedia/commons/thumb/"
                        + md5Hex.substring(0, 1) + "/"
                        + md5Hex.substring(0, 2) + "/"
                        + picName + "/300px-"
                        + picName;

                Log.i("Full url is", url);


                arrayList = new ArrayList(50);

                if (url == null) {
                    displayedCity.setText("Unable to recover images for " + city);
                } else {
                    if (arrayList != null)
                        arrayList.clear();
                    arrayList.add(url);
                    lv.setAdapter(new CityGalleryAdapter(this, arrayList, CityGallery.this));

                }
            } else Log.i("CityGallery-OnClick()", "Url is NULL, check getPlacePhoto()!");
        }
        else Toast.makeText(CityGallery.this, "City name should be in Latin!", Toast.LENGTH_LONG).show();
    }

    /*public void showDescriptionDialog(String description){
        android.support.v7.app.AlertDialog.Builder builder = new android.support.v7.app.AlertDialog.Builder(getApplicationContext());
        builder.setTitle(R.string.joiningSR);
        builder.setMessage(description);
        builder.create();
        builder.show();
    }*/
}

