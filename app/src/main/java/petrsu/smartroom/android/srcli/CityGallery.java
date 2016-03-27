package petrsu.smartroom.android.srcli;

import android.app.AlertDialog;
import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.ActionBarActivity;
import android.support.v7.widget.Toolbar;
import android.text.Html;
import android.view.View;
import android.widget.AdapterView;
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
public class CityGallery extends ActionBarActivity  {

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.city_gallery);

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
                        new PrimaryDrawerItem().withName(R.string.help_agenda).withIcon(FontAwesome.Icon.faw_info),
                        new PrimaryDrawerItem().withName(R.string.manual).withIcon(FontAwesome.Icon.faw_download),

                        new DividerDrawerItem(),
                        new SecondaryDrawerItem().withName(R.string.exitClientTitle).withIcon(FontAwesome.Icon.faw_close)
                ).withOnDrawerItemClickListener(new Drawer.OnDrawerItemClickListener() {
                    @Override
                    public boolean onItemClick(AdapterView<?> parent, View view, int position, long id, IDrawerItem drawerItem) {
                        //Toast.makeText(Agenda.this, String.valueOf(id), Toast.LENGTH_SHORT).show();
                        switch ((int) id) {
                            case 1:
                                break;
                            case 2:
                                gotoPresentation();
                                break;
                            case 3:
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
                                openHelp();
                                break;
                            case 11:
                                gotoManual();
                                break;
                            case 13:
                                exitApp();
                                break;
                            default:
                                break;
                        }
                        return true;
                    }
                }).build();
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
        intent.putExtra("url",KP.dqAddr+"/listCurrentThreads/?user_uuid" + KP.getPersonUuid());

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
     * GO TO  SOCIAL PROGRAM
     *==========================================================================
     */
    private void gotoSocialProgram(){

        Intent intent = new Intent(getApplicationContext(), WebViewer.class);
        String uuid = KP.getPersonUuid();

        intent.putExtra("url", KP.spAddr + "?person_uuid="+ uuid.substring(uuid.indexOf("#")+1));

        startActivity(intent);
    }


    /**=========================================================================
     * QUITS TO THE DESKTOP
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
        intent.putExtra("url",KP.manLink);
        intent.putExtra("reading", true);

        startActivity(intent);
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
}
