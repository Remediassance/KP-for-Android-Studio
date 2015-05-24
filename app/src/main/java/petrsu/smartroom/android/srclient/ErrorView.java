/**
 * @file ErrorView.java
 * @author Pavel Y. Kovyrshin <kovyrshi@cs.karelia.ru>
 * @date 30.09.2013
 * @brief
 *
 * @section DESCRIPTION
 *
 * ErrorView.java -
 * 
 *
 * Copyright (C) SmartSlog Team (Pavel Y. Kovyrshin).
 * All rights reserved.
 * Mail-list: smartslog@cs.karelia.ru
 */
package petrsu.smartroom.android.srclient;

import java.util.HashMap;

/**
 * 
 * @author pavlin
 *
 *	Class is using for showing error
 *	window if error was occur
 */
public class ErrorView extends HashMap<String, String> {
	private static final long serialVersionUID = 1L;
	public static final String MSG = "message";
	
	public ErrorView(String msg) {
		super();
		super.put(MSG, msg);
	}
}
