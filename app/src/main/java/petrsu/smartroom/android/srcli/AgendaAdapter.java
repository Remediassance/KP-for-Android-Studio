package petrsu.smartroom.android.srcli;

import java.util.List;
import java.util.Map;

import android.content.Context;
import android.graphics.Color;
import android.view.View;
import android.view.ViewGroup;
import android.widget.SimpleAdapter;
import android.widget.TextView;

/**
 * 
 * @author pavlin
 *
 *	Agenda adapter is using for highlighting
 *	separate list items in agenda program 
 */
public class AgendaAdapter extends SimpleAdapter {
	
	public AgendaAdapter(Context context, List<? extends Map<String, ?>> data,
			int resource, String[] from, int[] to) {
		super(context, data, resource, from, to);
	}
	
	@Override
	public View getView(int position, View convertView, ViewGroup parent) {
		View rowView = super.getView(position, convertView, parent);
		TextView name = (TextView) rowView.findViewById(R.id.speakerName);

		/*Button downloadSlides = (Button) rowView.findViewById(R.id.downloadSlidesBtn);
		Button seeProfile = (Button) rowView.findViewById(R.id.seeProfileBtn);

        downloadSlides.setFocusable(false);
        seeProfile.setFocusable(false);*/


		/* Highlight of current speaker */
		if(position == Agenda.currentTimeslotIndex)
			rowView.setBackgroundColor(Color.parseColor("#B9F6CA"));
		else
			rowView.setBackgroundColor(Color.WHITE);
		
		/* Highlight user profile */
		if(name != null)
			if(position == KP.personIndex) {
				name.setTextColor(Color.parseColor("#64DD17"));
			} else {
				name.setTextColor(Color.BLACK);
			}
		
		return rowView;
	}

	/**
	 * Notifies list view that data was changed
	 */
	public void updateHighlight() {
		notifyDataSetChanged();
	}
}
