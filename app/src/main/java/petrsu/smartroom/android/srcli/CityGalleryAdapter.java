package petrsu.smartroom.android.srcli;

import android.app.LauncherActivity;
import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;

import java.util.ArrayList;

/**
 * Created by Remediassance on 25.08.2016.
 */
public class CityGalleryAdapter extends BaseAdapter {

    private ArrayList dataList;
    private LayoutInflater layoutInflater;

    public CityGalleryAdapter(Context context, ArrayList dataList) {
        this.dataList = dataList;
        layoutInflater = LayoutInflater.from(context);
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
    public View getView(int position, View convertView, ViewGroup parent) {

        ViewHolder holder;
        if (convertView == null) {
            convertView = layoutInflater.inflate(R.layout.list_column_layout, null);
            holder = new ViewHolder();
            holder.imageView = (ImageView) convertView.findViewById(R.id.thumbImage);
            convertView.setTag(holder);
        } else {
            holder = (ViewHolder) convertView.getTag();
        }

        if (holder.imageView != null) {
            new CityGalleryAsyncLoader(holder.imageView).execute(dataList.get(position).toString());

        }

        return convertView;
    }


    static class ViewHolder {
        /*TextView headlineView;
        TextView reporterNameView;
        TextView reportedDateView;*/
        ImageView imageView;
    }
}