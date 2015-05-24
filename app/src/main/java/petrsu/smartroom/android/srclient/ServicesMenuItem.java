package petrsu.smartroom.android.srclient;

import java.util.HashMap;

/**
 * 
 * @author pavlin
 *
 *	Describes service menu item
 */
public class ServicesMenuItem extends HashMap<String, String> {
	private static final long serialVersionUID = 1L;
	public static final String SERVICE_NAME = "name";
	public static final String SERVICE_DESCR = "description";
	
	public ServicesMenuItem() {}
	
	public ServicesMenuItem(String name, String description) {
		super();
		super.put(SERVICE_NAME, name);
		super.put(SERVICE_DESCR, description);
	}
}

