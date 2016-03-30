package petrsu.smartroom.android.srcli;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.view.View;
import android.widget.AdapterView;
import android.widget.Toolbar;

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
 * This class is created solely to keep functions
 * that serve to navigate between activities in one place.
 *
 * For future reference: practice shows that you will highly possible be editing this class.
 *                       Add everything connected to switching between intents / activities here
 */
public abstract class Navigation {

    /*public Drawer getDrawer(String classname){
        Drawer drawer = null;
        switch (classname) {
            case "profile":
                drawer = getBasicDrawer();
                break;
        }
        return drawer;
    }*/

    public static  void getBasicDrawer(final Context context, final Activity activity, android.support.v7.widget.Toolbar toolbar){
        Drawer result = new DrawerBuilder()
                .withActivity(activity)
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
                        new PrimaryDrawerItem().withName(R.string.manual).withIcon(FontAwesome.Icon.faw_download),

                        new DividerDrawerItem(),
                        new SecondaryDrawerItem().withName(R.string.exitClientTitle).withIcon(FontAwesome.Icon.faw_close)
                ).withOnDrawerItemClickListener(new Drawer.OnDrawerItemClickListener() {
                    @Override
                    public boolean onItemClick(AdapterView<?> parent, View view, int position, long id, IDrawerItem drawerItem) {
                        //Toast.makeText(Agenda.this, String.valueOf(id), Toast.LENGTH_SHORT).show();
                        switch ((int) id) {

                            case 1:
                                activity.startActivity(getAgendaIntent(context));
                                break;
                            case 2:
                                activity.startActivity(getPresentationIntent(context));
                                break;
                            case 4:
                                activity.startActivity(getSocialProgramIntent(context));
                                break;
                            case 5:
                                activity.startActivity(getCurDisqIntent(context));
                                break;
                            case 6:
                                activity.startActivity(getDisqListIntent(context));
                                break;
                            case 8:
                                activity.startActivity(getSettingsIntent(context));
                                break;
                            case 10:
                                activity.startActivity(getManIntent(context));
                                break;
                            case 12:
                                activity.startActivity(exitApp(context));
                                break;
                            default:
                                break;
                        }
                        return true;
                    }
                }).build();
    }

    /**========================================================================
     * GET PRESENTATION SERVICE INTENT
     *=========================================================================
     */
    public static Intent getPresentationIntent(Context context){
        Intent intent = new Intent(context, Projector.class);
        return intent;
    }



    /**========================================================================
     * GET AGENDA SERVICE'S INTENT
     *=========================================================================
     */
    public static Intent getAgendaIntent(Context context){
        Intent intent = new Intent(context, Agenda.class);
        return intent;
    }



    /**=========================================================================
     * GET CURRENT DISCUSSION'S INTENT
     *==========================================================================
     */
    public static Intent getCurDisqIntent(Context context){
        Intent intent = new Intent(context, WebViewer.class);
        intent.putExtra("url", KP.dqAddr);
        return intent;
    }


    /**=========================================================================
     * GET DISCUSSION LIST'S INTENT
     *==========================================================================
     */
    public static Intent getDisqListIntent(Context context){
        Intent intent = new Intent(context, WebViewer.class);
        intent.putExtra("url", KP.dqAddr + "chat/");
        return intent;
    }



    /**=========================================================================
     * GETS SOCIAL PROGRAM INTENT
     *==========================================================================
     */
    public static Intent getSocialProgramIntent(Context context){
        Intent intent = new Intent(context, WebViewer.class);
        intent.putExtra("url",KP.spAddr);
        return intent;
    }



    /**=========================================================================
     * GETS SETTINGS INTENT
     *==========================================================================
     */
    public static Intent getSettingsIntent(Context context){
        Intent intent = new Intent(context, SettingsMenu.class);
        return intent;
    }



    /*=========================================================================
    * OPENS BROWSER ON THE DOWNLOAD MANUAL PAGE
    * =========================================================================
     */
    private static Intent getManIntent(Context context) {
        Intent intent = new Intent(context, WebViewer.class);
        intent.putExtra("url", KP.manLink);
        intent.putExtra("reading", true);

        return intent;
    }



    /**=========================================================================
     * QITS TO THE DESKTOP
     *==========================================================================
     */
    private static Intent exitApp(Context context) {
        Intent intent = new Intent(Intent.ACTION_MAIN);
        intent.addCategory(Intent.CATEGORY_HOME);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        return intent;
    }
}
