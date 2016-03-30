package petrsu.smartroom.android.srcli;

import android.app.AlertDialog;
import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.ActionBarActivity;
import android.support.v7.widget.Toolbar;
import android.text.Html;
import android.view.View;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
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
 * Created by Remediassance on 26.03.2016.
 * Class for fruct demo.
 * Asks for city and returns its pictures
 */
public class CityGallery extends ActionBarActivity implements View.OnClickListener {

    private Button cityBtn;
    private EditText cityText;
    private TextView displayedCity;

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

        Navigation.getBasicDrawer(getApplicationContext(),this,toolbar);
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

    public void spiptest() {
        Toast.makeText(this.getApplicationContext(), KP.dqAddr + '\n' + KP.spAddr, Toast.LENGTH_LONG).show();
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

    @Override
    public void onClick(View v) {
        String city = cityText.getText().toString();
        displayedCity.setText(city);
    }
}
