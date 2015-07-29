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
 * Created by ������ on 22.07.2015.
 *
 * This class can be used to display any web related
 * information or act as a built-in web-browser.
 * ATM it is used to display Discussion service pages.
 */
public class WebViewer extends ActionBarActivity {

    WebView webview;
    static Boolean isFromLogin = false;
    static Boolean isReadingman = false;


    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.web_view);
        Drawer drawer;

        Intent intent = getIntent();
        String link = intent.getStringExtra("url");
        isFromLogin = intent.getBooleanExtra("flag", false);
        isReadingman = intent.getBooleanExtra("reading",false);

        if (isReadingman == true)
            this.setTitle(R.string.manual);
        else
            this.setTitle(R.string.dqBrowser);

        webview = (WebView) findViewById(R.id.webView);
        webview.getSettings().setJavaScriptEnabled(true);
        webview.getSettings().setSupportZoom(true);
        webview.getSettings().setBuiltInZoomControls(true);
        webview.setWebViewClient(new Callback());
        webview.loadUrl(link);


        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);

        // ������ ���� ��� ������ �  ������ ��������
        if(isFromLogin==false) {
            drawer = new DrawerBuilder()
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

                            switch ((int) id) {
                                case 1:
                                    gotoAgenda();
                                    break;
                                case 2:
                                    gotoPresentation();
                                    break;
                                case 4:
                                    gotoCurDisq();
                                    break;
                                case 5:
                                    gotoDisqList();
                                    break;
                                case 7:
                                    gotoSettings();
                                    break;
                                case 9:
                                    gotoManual();
                                    break;
                                case 11:
                                    exitApp();
                                    break;
                                default:
                                    break;
                            }
                            return true;
                        }
                    }).build();
        }
        else {
            drawer = new DrawerBuilder()
                    .withActivity(this)
                    .withToolbar(toolbar)
                    .withActionBarDrawerToggle(true)
                    .withHeader(R.layout.drawer_header)
                    .withDrawerWidthDp(320)
                    .addDrawerItems(
                            new PrimaryDrawerItem().withName(R.string.loginMenu).withIcon(FontAwesome.Icon.faw_desktop),
                            new PrimaryDrawerItem().withName(R.string.manual).withIcon(FontAwesome.Icon.faw_download),
                            new DividerDrawerItem(),
                            new PrimaryDrawerItem().withName(R.string.discussionCur).withIcon(FontAwesome.Icon.faw_comment),
                            new PrimaryDrawerItem().withName(R.string.discussionList).withIcon(FontAwesome.Icon.faw_comments),
                            new DividerDrawerItem(),
                            new SecondaryDrawerItem().withName(R.string.exitClientTitle).withIcon(FontAwesome
                                    .Icon.faw_close)
                    ).withOnDrawerItemClickListener(new Drawer.OnDrawerItemClickListener() {
                        @Override
                        public boolean onItemClick(AdapterView<?> parent, View view, int position, long id, IDrawerItem drawerItem) {
                            switch ((int) id) {
                                case 0:
                                    gotoLogin();
                                    break;
                                case 1:
                                    gotoManual();
                                    break;
                                case 3:
                                    gotoCurDisq();
                                    break;
                                case 4:
                                    gotoDisqList();
                                    break;
                                case 6:
                                    exitApp();
                                    break;
                                default:
                                    break;
                            }
                            return true;
                        }
                    }).build();
        }
    }

    private void gotoLogin() {
        Intent intent = new Intent();
        intent.setClass(this, KP.class);
        startActivity(intent);
    }



    /**========================================================================
     *  ENABLES USING ACTIVITY AS IF IT WAS A WEB BROWSER
     *=========================================================================
     */
    private class Callback extends WebViewClient {
        @Override
        public boolean shouldOverrideUrlLoading(WebView view, String url) {
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


    /*=========================================================================
    * OPENS BROWSER ON THE DOWNLOAD MANUAL PAGE
    * =========================================================================
     */
    private void gotoManual() {
        Intent intent = new Intent(getApplicationContext(), WebViewer.class);
        intent.putExtra("url", KP.manLink);
        intent.putExtra("reading", true);

        if (KP.isRegistered == true)
            intent.putExtra("flag", false);
        else intent.putExtra("flag", true);

        startActivity(intent);

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
        //String addr = contentUrl.substring(0,contentUrl.lastIndexOf("files")); //smartroom.cs.petrsu.ru
        //Toast.makeText(getApplicationContext(), addr, Toast.LENGTH_LONG).show();

        Intent intent = new Intent(getApplicationContext(), WebViewer.class);
        intent.putExtra("url", KP.dqAddr+"chat");

        if (KP.isRegistered == true)
            intent.putExtra("flag", false);
        else intent.putExtra("flag", true);

        startActivity(intent);
    }


    /**=========================================================================
     * GO TO  DISCUSSION LIST
     *==========================================================================
     */
    private void gotoDisqList(){
        String contentUrl = KP.getContentUrl();
        //String addr = contentUrl.substring(0,contentUrl.lastIndexOf("files"));

        Intent intent = new Intent(getApplicationContext(), WebViewer.class);
        intent.putExtra("url",KP.dqAddr+"chat/listCurrentThreads");

        if (KP.isRegistered == true)
            intent.putExtra("flag", false);
        else intent.putExtra("flag", true);

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


