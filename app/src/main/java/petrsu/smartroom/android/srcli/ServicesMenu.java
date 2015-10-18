package petrsu.smartroom.android.srcli;

import java.util.ArrayList;

import android.app.ListActivity;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.widget.ListAdapter;
import android.widget.SimpleAdapter;
import android.widget.ListView;
import android.widget.Toast;
import android.view.View;

/**
 * 
 * @author pavlin
 *
 *	Displays list of available SmartRoom services
 */
public class ServicesMenu extends ListActivity {
	
	private ArrayList<ServicesMenuItem> list;		// Services list
	private static ProgressDialog progressDialog;
	private BroadcastReceiver bc;
	
	public int agendaServiceState;
	public int presentationServiceState;
	
	public ServicesMenu() {}
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		/* Checks available services 
		 * TODO: better to return array of services or indicators */
		KP.getServicesInfo(this);
		
		list = new ArrayList<ServicesMenuItem>();
		
		/* If agenda service is available */
		if(agendaServiceState == 1) {
			list.add(new ServicesMenuItem(getResources()
					.getString(R.string.agenda), 
					getResources().getString(R.string.agenda_descr)));
		}

		/* If presentation service is available */
		if(presentationServiceState == 1) {
			list.add(new ServicesMenuItem(getResources()
					.getString(R.string.presentation), 
					getResources().getString(R.string.presentation_descr)));
		}

		/* If no services available */
		if(list.isEmpty()) {
			ArrayList<ErrorView> list = new ArrayList<ErrorView>();
			list.add(new ErrorView(getResources()
					.getString(R.string.noServicesAvailable)));
			ListAdapter adapter = new SimpleAdapter(
	        		this, list, R.layout.services_interface_ext,
	        		new String[] {ErrorView.MSG}, 
	        		new int[] {R.id.noServicesView});
	        
	        setListAdapter(adapter);
	        
	        return;
		}

		ListAdapter adapter = new SimpleAdapter(
        		this, list, R.layout.services_interface, 
        		new String[] {ServicesMenuItem.SERVICE_NAME, 
        				ServicesMenuItem.SERVICE_DESCR}, 
        		new int[] {R.id.service_name, R.id.service_descr});
        
        setListAdapter(adapter);
	}
	
	@Override
	protected void onListItemClick(ListView l, View v, int position, long id) {
		Intent intent = new Intent();
		String serviceName = l.getItemAtPosition(position).toString();
		
		if(serviceName.contains(getResources().getString(R.string.agenda))) {
			intent.setClass(this, Agenda.class);
			startActivity(intent);
		} else if(serviceName.contains(getResources()
				.getString(R.string.presentation))) {
			intent.setClass(this, Projector.class);
			startActivity(intent);
		}
	}
	
	@Override
	protected void onResume() {
		super.onResume();
		registerReceiver();
	}
	
	@Override
	protected void onPause() {
		super.onPause();
		unregisterReceiver(bc);
	}
	
	public void registerReceiver() {
		/* Displays connection recovery process */
		bc = new BroadcastReceiver() {
			@Override
			public void onReceive(Context context, Intent intent) {
				switch(intent.getIntExtra("action", 0)) {
					case NetworkService.START_RECOVER:
						progressDialog = ProgressDialog.show(context,
								getResources().getString(
										R.string.connectionRecoverTitle), 
								getResources().getString(
										R.string.connectionRecoverMsg), 
										false, true);
						break;
						
					case NetworkService.STOP_RECOVER:
						if(progressDialog != null)
							if(progressDialog.isShowing())
								progressDialog.dismiss();
						break;
						
					case NetworkService.FAIL_RECOVER:
						/*Toast.makeText(context, 
						 		R.string.connectionRecoverFail, 
								Toast.LENGTH_LONG).show(); */
						break;
						
					case NetworkService.OK_RECOVER:
						Toast.makeText(context, R.string.connectionRecoverOk, 
								Toast.LENGTH_LONG).show();
						break;
				}
			}
		};
		IntentFilter filter = new IntentFilter(
				NetworkService.BC_RECOVERING_CONNECTION);
		this.registerReceiver(bc, filter);
	}
}
