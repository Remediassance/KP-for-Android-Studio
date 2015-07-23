package petrsu.smartroom.android.srclient;

import android.app.AlertDialog;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.support.v7.app.ActionBarActivity;
import android.support.v7.widget.Toolbar;
import android.text.Html;
import android.view.KeyEvent;
import android.view.View;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.AdapterView;

import com.mikepenz.iconics.typeface.FontAwesome;
import com.mikepenz.materialdrawer.Drawer;
import com.mikepenz.materialdrawer.DrawerBuilder;
import com.mikepenz.materialdrawer.model.DividerDrawerItem;
import com.mikepenz.materialdrawer.model.PrimaryDrawerItem;
import com.mikepenz.materialdrawer.model.SecondaryDrawerItem;
import com.mikepenz.materialdrawer.model.SectionDrawerItem;
import com.mikepenz.materialdrawer.model.interfaces.IDrawerItem;

/**
 * Created by Андрей on 22.07.2015.
 *
 * Creates browser activity inside application,
 * so user doesn't need to launch browser to
 * access discussions, thus, saving navigation
 * options that are provided by SRClient side menu.
 *
 */
public class WebViewer extends ActionBarActivity {

    WebView webview;

    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.web_view);

        Intent intent = new Intent();
        String url = intent.getStringExtra("url");

        webview = (WebView) findViewById(R.id.webView);
        webview.setWebViewClient(new Callback());
        webview.loadUrl("http://m.vk.com/");

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

                        new SectionDrawerItem().withName(R.string.discussion),
                        new PrimaryDrawerItem().withName(R.string.discussionCur).withIcon(FontAwesome.Icon.faw_comment_o),
                        new PrimaryDrawerItem().withName(R.string.discussionList).withIcon(FontAwesome.Icon.faw_comments_o),

                        new SectionDrawerItem().withName(R.string.action_settings),
                        new PrimaryDrawerItem().withName(R.string.action_settings).withIcon(FontAwesome.Icon.faw_cog),

                        new SectionDrawerItem().withName(R.string.help),
                        new PrimaryDrawerItem().withName(R.string.manual).withIcon(FontAwesome.Icon.faw_download),

                        new DividerDrawerItem(),
                        new SecondaryDrawerItem().withName(R.string.exitClientTitle).withIcon(FontAwesome.Icon.faw_close)
                ).withOnDrawerItemClickListener(new Drawer.OnDrawerItemClickListener() {
                    @Override
                    public boolean onItemClick(AdapterView<?> parent, View view, int position, long id, IDrawerItem drawerItem) {
                        //Toast.makeText(Agenda.this, String.valueOf(id), Toast.LENGTH_SHORT).show();
                        switch ((int) id) {
                            case 1:     gotoAgenda(); break;
                            case 2:     gotoPresentation(); break;
                            case 4:     gotoCurDisq();      break;
                            case 5:     gotoDisqList();     break;
                            case 7:     gotoSettings();     break;
                            case 9:     gotoManual();       break;
                            case 11:    exitApp();          break;
                            default:  break;
                        }
                        return true;
                    }
                }).build();
    }


    /**========================================================================
     *  ENABLES FOLLOWING LINKS INSIDE APPLICATION
     *=========================================================================
     */
    private class Callback extends WebViewClient {
        @Override
        public boolean shouldOverrideUrlLoading
                (WebView view, String url) {
            return(false);
        }
    }


    /**========================================================================
     * TRACKS EVENTS OF PRESSING HARDWARE BUTTONS OF THE DEVICE
     * @param keyCode  code of pressed button
     * @param event signal of event being launched
     * @return true, if success, false otherwise
     *=========================================================================
     */
    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if ((keyCode == KeyEvent.KEYCODE_BACK) && webview.canGoBack()) {
            webview.goBack();
            return true;
        }
        return super.onKeyDown(keyCode, event);
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


    /**=========================================================================
     * GO TO CURRENT DISCUSSION
     *==========================================================================
     */
    private void gotoCurDisq(){
        String contentUrl = KP.getContentUrl();
        String addr = contentUrl.substring(0,contentUrl.lastIndexOf("files")); //smartroom.cs.petrsu.ru
        //Toast.makeText(getApplicationContext(), addr, Toast.LENGTH_LONG).show();

        Intent intent = new Intent(getApplicationContext(), WebViewer.class);
        intent.putExtra("url", contentUrl+"chat");

        startActivity(intent);
    }


    /**=========================================================================
     * GO TO  DISCUSSION LIST
     *==========================================================================
     */
    private void gotoDisqList(){
        String contentUrl = KP.getContentUrl();
        String addr = contentUrl.substring(0,contentUrl.lastIndexOf("files")); //smartroom.cs.petrsu.ru

        Intent intent = new Intent(getApplicationContext(), WebViewer.class);
        intent.putExtra("url",contentUrl+"chat/listCurrentThreads");

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
}


