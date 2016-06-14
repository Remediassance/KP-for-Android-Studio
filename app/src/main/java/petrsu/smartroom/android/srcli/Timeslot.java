package petrsu.smartroom.android.srcli;

import java.util.HashMap;

import android.graphics.Bitmap;

/**
 * 
 * @author pavlin
 *
 *	Describes time slot item
 */
public class Timeslot extends HashMap<String, Object> {
	
	private static final long serialVersionUID = 1L;
	public static final String NAME = "name";
	public static final String TITLE = "title";
	public static final String IMG = "img";
	public static final String STATUS = "status";
	public String personUuid;
	
	public Timeslot() {}
	
	/**
	 * 
	 * @param name - participant name
	 * @param title	- presentation title
	 * @param img - participant avatar
	 * @param status - participant status (online/offline)
	 */
	public Timeslot(String name, String title, String img, String status) {
		super();
		super.put(NAME, name);
		super.put(TITLE, title);
		super.put(IMG, img);
		super.put(STATUS, status);
	}
	
	public Timeslot(String name, String title, Bitmap img, String status) {
		super();
		super.put(NAME, name);
		super.put(TITLE, title);
		super.put(IMG, img);
		super.put(STATUS, status);
	}
	
	public Timeslot(Timeslot item) {
		super();
		super.put(NAME, item.get(Timeslot.NAME));
		super.put(TITLE, item.get(Timeslot.TITLE));
		super.put(IMG, item.get(Timeslot.IMG));
		super.put(STATUS, item.get(Timeslot.STATUS));
	}
	
	/**
	 * Sets person uuid
	 *
	 * @param uuid - person uuid
	 */
	public void setPersonUuid(String uuid) {
		personUuid = uuid;
	}
}
