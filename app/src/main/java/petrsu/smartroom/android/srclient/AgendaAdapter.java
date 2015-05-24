package petrsu.smartroom.android.srclient;

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

		/* Highlight of current speaker */
		if(position == Agenda.currentTimeslotIndex)
			rowView.setBackgroundColor(Color.parseColor("#add8e6"));
		else
			rowView.setBackgroundColor(Color.WHITE);
		
		/* Highlight user profile */
		if(name != null)
			if(position == KP.personIndex) {
				name.setTextColor(Color.RED);
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
