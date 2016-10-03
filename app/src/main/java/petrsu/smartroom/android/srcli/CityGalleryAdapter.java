package petrsu.smartroom.android.srcli;

import android.app.Activity;
import android.content.Context;
import android.support.v7.app.AlertDialog;
import android.text.Html;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import java.util.ArrayList;

/**
 * Created by Remediassance on 25.08.2016.
 * Adapter for entries in the City Gallery activity.
 * Maintains the content of activity.
 *
 * As of 16.09.16 I changed it so it only displays description of a city.
 * To mke it display all data again, uncomment everything here and add
 * proper changes to CityGallery class.
 */
public class CityGalleryAdapter extends BaseAdapter {

    private ArrayList dataList;
    private LayoutInflater layoutInflater;
    private Activity parentActivity;

    public CityGalleryAdapter(Context context, ArrayList dataList, Activity activity) {
        this.dataList = dataList;
        this.layoutInflater = LayoutInflater.from(context);
        this.parentActivity = activity;
    }

    @Override
    public int getCount() {
        return dataList.size();
    }

    @Override
    public Object getItem(int position) {
        return dataList.get(position);
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @Override
    public View getView(int position, View convertView, final ViewGroup parent) {

        ViewHolder holder;
        if (convertView == null) {
            convertView = layoutInflater.inflate(R.layout.list_column_layout, null);
            holder = new ViewHolder();
            //holder.imageView = (ImageView) convertView.findViewById(R.id.thumbImage);
            holder.description = (TextView) convertView.findViewById(R.id.seemore);

           /* holder.description.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    AlertDialog.Builder builder = new AlertDialog.Builder(parentActivity);
                    builder.setTitle(CityGallery.city);
                    builder.setMessage(dataList.get(2).toString());
                    builder.create();
                    builder.show();
                }
            });*/
            //holder.foundingDate = (TextView) convertView.findViewById(R.id.fdsource);
            convertView.setTag(holder);
        } else {
            holder = (ViewHolder) convertView.getTag();
        }


        /*if (holder.imageView != null) {
            new CityGalleryAsyncLoader(holder.imageView).execute(dataList.get(position).toString());
        }

        if(holder.foundingDate != null){
            holder.foundingDate.setText(dataList.get(1).toString());
        }*/

        if(holder.description != null){
            holder.description.setText(dataList.get(position).toString());
        }

        return convertView;
    }


    /*
    * Все поля для отображения внутри одной записи объявлять тут.
    */
    static class ViewHolder {
        TextView description;
        TextView foundingDate;
        ImageView imageView;
    }
}