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
 * This class can be used to display any web related
 * information or act as a built-in web-browser.
 * ATM it is used to display Discussion service pages.
 */
public class WebViewer extends ActionBarActivity {

    WebView webview;
    Boolean isFromLogin = false;


    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.web_view);
        Drawer drawer;

        Intent intent = new Intent();
        //String url = intent.getStringExtra("url");
        isFromLogin = intent.getBooleanExtra("flag", false);

        webview = (WebView) findViewById(R.id.webView);
        webview.setWebViewClient(new Callback());
        webview.loadUrl("http://9gag.com/");

        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);

        // Разные меню для входов с  разных активити
        if(!isFromLogin) {
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
                            //Toast.makeText(Agenda.this, String.valueOf(id), Toast.LENGTH_SHORT).show();
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
                            new PrimaryDrawerItem().withName(R.string.signup).withIcon(FontAwesome.Icon.faw_barcode),
                            new PrimaryDrawerItem().withName(R.string.manual).withIcon(FontAwesome.Icon.faw_download),
                            new PrimaryDrawerItem().withName(R.string.drawer_item_help).withIcon(FontAwesome.Icon.faw_info),
                            new DividerDrawerItem(),
                            new SecondaryDrawerItem().withName(R.string.exitClientTitle).withIcon(FontAwesome
                                    .Icon.faw_close),
                            new SecondaryDrawerItem().withName("About Developers").withIcon(FontAwesome
                                    .Icon.faw_connectdevelop)
                    ).withOnDrawerItemClickListener(new Drawer.OnDrawerItemClickListener() {
                        @Override
                        public boolean onItemClick(AdapterView<?> parent, View view, int position, long id, IDrawerItem drawerItem) {
                            switch ((int) id) {
                                case 0:
                                    scanQrCode();
                                    break;
                                case 1:
                                    gotoManual();
                                    break;
                                case 2:
                                    openHelp();
                                    break;
                                case 4:
                                    exitApp();
                                    break;
                                case 5:
                                    gotoDisqList();
                                default:
                                    break;
                            }
                            return true;
                        }
                    }).build();
        }
    }

    /**========================================================================
     * STARTS QR CODE SCANNER APPLICATION OR OFFERS TO INSTALL ONE
     *=========================================================================
     */
    public void scanQrCode() {
        try {
            Intent intent = new Intent("com.google.zxing.client.android.SCAN");
            intent.putExtra("SCAN_MODE", "QR_CODE_MODE");
            startActivityForResult(intent, 0);
        } catch (Exception e) {
            Uri marketUri = Uri.parse(
                    "market://details?id=com.google.zxing.client.android");
            Intent marketIntent = new Intent(Intent.ACTION_VIEW,marketUri);
            startActivity(marketIntent);
        }
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


