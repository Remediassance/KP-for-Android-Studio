package petrsu.smartroom.android.srcli;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
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
 * Created by Borodulin Andrey on 26.03.2016.
 * This class is created solely to keep functions
 * that serve to navigate between activities in one place.
 *
 * For future reference: practice shows that you will highly possible be editing this class.
 *                       Add everything connected to switching between intents / activities here
 */
public class Navigation { // was abstract lol


    /**
     * Возвращает боковое меню. Из-за трудностей переноса, аналогичные меню Agenda и Presentation
     * оставлены в соответствующих классах
     */
    public static  void getBasicDrawer(final Context context, final Activity activity, android.support.v7.widget.Toolbar toolbar){
        new DrawerBuilder()
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
                                activity.startActivity(exitApp());
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
        return new Intent(context, Projector.class);
    }



    /**========================================================================
     * GET AGENDA SERVICE'S INTENT
     *=========================================================================
     */
    public static Intent getAgendaIntent(Context context){
        return new Intent(context, Agenda.class);
    }


    /**=========================================================================
     * GET CURRENT DISCUSSION'S INTENT
     *==========================================================================
     */
    public static Intent getCurDisqIntent(Context context){
        Intent intent = new Intent(context, WebViewer.class);
        String uuid = KP.getPersonUuid().substring(KP.getPersonUuid().indexOf("#")+1,KP.getPersonUuid().length());
        intent.putExtra("url", KP.dqAddr + "?user_uuid=" + uuid);
        intent.putExtra("service", R.string.discussion);
        Toast.makeText(context,  KP.dqAddr+"?user_uuid="+uuid, Toast.LENGTH_LONG).show();

        return intent;
    }


    /**=========================================================================
     * GET DISCUSSION LIST'S INTENT
     *==========================================================================
     */
    public static Intent getDisqListIntent(Context context){
        Intent intent = new Intent(context, WebViewer.class);
        String uuid = KP.getPersonUuid().substring(KP.getPersonUuid().indexOf("#")+1,KP.getPersonUuid().length());
        intent.putExtra("url", KP.dqAddr + "/listCurrentThreads?user_uuid=" + uuid); //"chat/?user_uuid=" + uuid
        intent.putExtra("service", R.string.discussion);
        Toast.makeText(context,  KP.dqAddr+"/listCurrentThreads?user_uuid="+uuid, Toast.LENGTH_LONG).show();
        return intent;
    }



    /**=========================================================================
     * GETS SOCIAL PROGRAM INTENT
     *==========================================================================
     */
    public static Intent getSocialProgramIntent(Context context){
        Intent intent = new Intent(context, WebViewer.class);
        String uuid = KP.getPersonUuid().substring(KP.getPersonUuid().indexOf("#")+1,KP.getPersonUuid().length());
        intent.putExtra("url",KP.spAddr+"?person_uuid="+uuid);
        intent.putExtra("service", "SocialProgram");
        return intent;
    }



    /**=========================================================================
     * GETS SETTINGS INTENT
     *==========================================================================
     */
    public static Intent getSettingsIntent(Context context){
        return new Intent(context, SettingsMenu.class);
    }



    /*=========================================================================
    * OPENS BROWSER ON THE DOWNLOAD MANUAL PAGE
    * =========================================================================
     */
    public static Intent getManIntent(Context context) {
        Intent intent = new Intent(context, WebViewer.class);
        intent.putExtra("url", KP.manLink);
        intent.putExtra("service", R.string.manual);

        return intent;
    }



    /**=========================================================================
     * QUITS TO THE DESKTOP
     *==========================================================================
     */
    public static Intent exitApp() {
        Intent intent = new Intent(Intent.ACTION_MAIN);
        intent.addCategory(Intent.CATEGORY_HOME);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        return intent;
    }



    /**========================================================================
     * GO TO GALLERY SERVICE
     *=========================================================================
     */
    public static Intent getGalleryIntent(Context context){
        Intent intent = new Intent();
        intent.setClass(context, CityGallery.class);
        return intent;
    }
}

/*
* Если все пойдет плохо, заменить вызовы getBasicDrawer вот этой простыней
*/

/*Drawer result = new DrawerBuilder()
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
        }).build();*/