package petrsu.smartroom.android.srcli;

import android.app.AlertDialog;
import android.os.Bundle;
import android.support.v7.app.ActionBarActivity;
import android.support.v7.widget.Toolbar;
import android.text.Html;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

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

        cityBtn = (Button) findViewById (R.id.searchBtn);
        cityBtn.setOnClickListener(this);

        cityText = (EditText) findViewById(R.id.cityTxt);

        displayedCity = (TextView) findViewById(R.id.displayedText);

        Navigation.getBasicDrawer(getApplicationContext(), this, toolbar);

        /*if(KP.getWelcomeServiceIP() != null)
            ipAddr = KP.getWelcomeServiceIP();
        else
            Toast.makeText(CityGallery.this, "Can't get servises' address!", Toast.LENGTH_SHORT).show();
        */


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

    /*=========================================================================
    *  IMPLEMENTATION OF ONCLICK LISTENER
    *==========================================================================
     */
    @Override
    public void onClick(View v) {
        String city = cityText.getText().toString();
        displayedCity.setText(city);
        String uuid = KP.getPersonUuid();//.substring(KP.getPersonUuid().indexOf("#")+1,KP.getPersonUuid().length());
        Toast.makeText(CityGallery.this, uuid, Toast.LENGTH_LONG).show();
        String url = KP.getPlaceInfo(city, uuid);
        if(url == null)
            displayedCity.setText("Unable to recover images for "+city);

    }
}
