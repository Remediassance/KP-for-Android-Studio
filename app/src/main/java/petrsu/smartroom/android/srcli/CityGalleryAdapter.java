package petrsu.smartroom.android.srcli;

import android.content.Context;
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
 */
public class CityGalleryAdapter extends BaseAdapter {

    private ArrayList dataList;
    private LayoutInflater layoutInflater;

    public CityGalleryAdapter(Context context, ArrayList dataList) {
        this.dataList = dataList;
        layoutInflater = LayoutInflater.from(context);
    }

    @Override
    public int getCount() {             // для отображения неск. записей
        return 1;                       // dataList.size();
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
            holder.description = (TextView) convertView.findViewById(R.id.seemore);
            holder.foundingDate = (TextView) convertView.findViewById(R.id.fdsource);
            convertView.setTag(holder);
        } else {
            holder = (ViewHolder) convertView.getTag();
        }

        //Костыль: 0 1 и 2 - это индексы ссфлки на фото, даты основания и описания, переданных
        // в массиве из класса галереи. Если передать вместо 0 переменную position, будет плохо,
        // но если при этом не получать founding date и description, то все вообще красиво и работает.
        if (holder.imageView != null) {
            new CityGalleryAsyncLoader(holder.imageView).execute(dataList.get(0).toString());
        }

        if(holder.foundingDate != null){
            holder.foundingDate.setText(dataList.get(1).toString());
        }

        if(holder.description != null){
            holder.description.setText(dataList.get(2).toString());
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