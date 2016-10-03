package petrsu.smartroom.android.srcli;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.support.v7.app.ActionBarActivity;
import android.support.v7.widget.Toolbar;
import android.text.Html;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
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
    private TextView foundingDate;
    private TextView changeCity;
    private TextView participants;

    private ImageView imageView;

    private String ipAddr = null;
    private String picName;
    private String md5Hex;
    private String uuid;
    private String url;
    private String cityDesc;
    private String cityFD;

    private ListView lv;

    public static ProgressBar progressBar;
    public static String city;

    private static ArrayList<Timeslot> confList;
    private static ArrayList<String> uuidList;
    private static ArrayList<String> namesList;

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

        displayedCity = (TextView) findViewById(R.id.displayedText);
        imageView = (ImageView) findViewById(R.id.thumbImage);
        foundingDate = (TextView) findViewById(R.id.fdsource);
        //changeCity = (TextView) findViewById(R.id.changecity);
        participants = (TextView) findViewById(R.id.participantsTxt);

        imageView.setOnClickListener(this);
        participants.setOnClickListener(this);

        Navigation.getBasicDrawer(getApplicationContext(), this, toolbar);

        refreshActivity(KP.getPersonUuid());
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
        if (_city.matches("[A-Za-z ]+"))
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
    */
    @Override
    public void onClick(View v) {

        /*
        * Actions differ based on item selected
        */
        switch(v.getId()){
            /*
            * If changecity is called, user is opted to change >his< city
             */
            case R.id.thumbImage: {

                LayoutInflater inflater = LayoutInflater.from(this);
                final View dialogViewCity = inflater.inflate(R.layout.city_registration, null);

                android.support.v7.app.AlertDialog.Builder builder = new android.support.v7.app.AlertDialog.Builder(this);
                builder.setView(dialogViewCity);
                builder.setTitle(R.string.registrationTitle);
                builder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        EditText editCity = (EditText) dialogViewCity.findViewById(R.id.cityField);
                        String cityz = editCity.getText().toString();
                        Log.i("GalleryOnClick()", "Passing city " + cityz);

                        if (cityz.equals("") || CityGallery.isCityNameCorrect(cityz) == false) {
                            Toast.makeText(getApplicationContext(), R.string.citycheck, Toast.LENGTH_LONG).show();
                        } else {
                            if (KP.setCity(uuid, cityz) == -1) {
                                Toast.makeText(getApplicationContext(), R.string.registrationFail, Toast.LENGTH_LONG).show();
                            } else {
                                Log.i("GalleryOnClick()", "Rewriting city data...OK");
                                refreshActivity(KP.getPersonUuid());
                            }
                        }
                    }
                });
                builder.setNegativeButton(android.R.string.cancel, null);
                android.support.v7.app.AlertDialog dialog = builder.create();
                dialog.show();
            } break;

            /*
            * If participants item is selected, confList of participants of current section is displayed
             */
            case R.id.participantsTxt: {
                confList = new ArrayList<>();
                uuidList = new ArrayList<>();
                namesList = new ArrayList<>();
                Timeslot ts;

                /*
                * Fetching all participants names and uuids from ss
                 */
                if(KP.loadParticipantsList(this) == -1) {
                    Log.e("CityGallery onClick()", "Error fetching uuids of all speakers");
                    return;
                }
                else {
                    Log.i("checkingclassname", this.getLocalClassName());
                }

                for(int i = 0; i < confList.size(); i++){
                    ts = confList.get(i);
                    namesList.add(ts.getPersonName());
                    uuidList.add(ts.getPersonUuid());
                }

                CharSequence[] csNames = namesList.toArray(new CharSequence[namesList.size()]);
                LayoutInflater inflater = LayoutInflater.from(this);
                final View personListView = inflater.inflate(R.layout.personlist, null);
                AlertDialog alert;
                AlertDialog.Builder builder = new AlertDialog.Builder(this);

                builder.setTitle(R.string.choosePerson);
                builder.setView(personListView);
                builder.setItems(csNames, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        // here we have person uuid, need to go to proper city gallery page
                        KP.getCityByPersonUuid(uuidList.get(which));
                        refreshActivity(uuidList.get(which));
                    }
                });
                builder.setNegativeButton(android.R.string.cancel, null);
                alert = builder.create();
                alert.show();
            } break;
        }
    }

    public void addTimeslotItemToList2(final String uuid, final String name) throws InterruptedException {

        if(uuid != null && name != null) {
            Thread t = new Thread() {
                @Override
                public void run() {
                    confList.add(new Timeslot(uuid, name));
                }
            };
            t.start();
            t.join();
        }
    }

    //TODO Сделать отображение города для каждого из участников
    /**========================================================================
     * Rewrites fields of activity to newly changed city info
     *=========================================================================
     */
    private void refreshActivity(String _uuid){

        uuid = _uuid;//KP.getPersonUuid();

        city = null;
        city = KP.getCityByPersonUuid(uuid);

        ArrayList dataList = new ArrayList(3);
        try {
            url = KP.getPlacePhoto(city, uuid);
            cityDesc = KP.getPlaceDescription(city);
            cityFD = KP.getPlaceFoundingDate(city);

            //Если у города нет даты основания, то, возможно, инфа введена неверно
            if (cityFD != null) {
                foundingDate.setText(cityFD);

                Log.i("CityGallery():", "City title is " + city);
                displayedCity.setText(city);

                if (url != null) {
                    picName = url.substring(url.lastIndexOf("/") + 1, url.indexOf("?"));
                    md5Hex = new String(Hex.encodeHex(DigestUtils.md5(picName)));
                    url = "https://upload.wikimedia.org/wikipedia/commons/thumb/"
                            + md5Hex.substring(0, 1) + "/"
                            + md5Hex.substring(0, 2) + "/"
                            + picName + "/700px-"
                            + picName;
                    Log.i("city link is", url);
                    new CityGalleryAsyncLoader(imageView).execute(url);
                } else {
                    new CityGalleryAsyncLoader(imageView).execute("http://0.tqn.com/d/webclipart/1/0/5/l/4/floral-icon-5.jpg");
                }

                if (cityDesc != null) {
                    dataList.clear();
                    dataList.add(cityDesc);
                } else {
                    dataList.clear();
                    dataList.add("Oops! Description was lost :(");
                }

                lv = (ListView) findViewById(R.id.listView);
                lv.setAdapter(new CityGalleryAdapter(this, dataList, CityGallery.this));

            } else {
                Toast.makeText(CityGallery.this, R.string.cityerror, Toast.LENGTH_SHORT).show();
                registerCity(uuid);
            }

        } catch (NullPointerException e) {
            e.printStackTrace();
        }
    }


    /**
     * Register city property after the user by his/her uuid
     * @param  uuid - uuid of a user without a city property
     */
    public void registerCity(final String uuid) {
        LayoutInflater inflater = LayoutInflater.from(this);
        final View dialogViewCity = inflater.inflate(R.layout.city_registration, null);

        android.support.v7.app.AlertDialog.Builder builder = new android.support.v7.app.AlertDialog.Builder(this);
        builder.setView(dialogViewCity);
        builder.setTitle(R.string.registrationTitle);
        builder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                EditText editCity = (EditText) dialogViewCity.findViewById(R.id.cityField);
                String cityz = editCity.getText().toString();

                if(cityz.equals("") || CityGallery.isCityNameCorrect(cityz) == false) {
                    Toast.makeText(getApplicationContext(),R.string.citycheck,Toast.LENGTH_LONG).show();
                } else {
                    if(KP.setCity(uuid,cityz) == -1) {
                        Toast.makeText(getApplicationContext(), R.string.registrationFail, Toast.LENGTH_LONG).show();
                        return;
                    }
                    else {
                        Log.i("registerCity()", "Going to the Gallery!");
                        refreshActivity(uuid);
                        //startActivity(Navigation.getGalleryIntent(getApplicationContext()));
                    }
                }
            }
        });
        builder.setNegativeButton(android.R.string.cancel, null);
        android.support.v7.app.AlertDialog dialog = builder.create();
        dialog.show();
    }

}

