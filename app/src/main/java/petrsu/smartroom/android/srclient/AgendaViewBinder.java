/*
 * 
 */
package petrsu.smartroom.android.srclient;

import android.widget.ImageView;
import android.widget.SimpleAdapter.ViewBinder;
import android.view.View;
import android.graphics.Bitmap;

/**
 * 
 * @author pavlin
 *
 *	View binder helps to set unique images
 *	for every list view item of agenda program
 */
public class AgendaViewBinder implements ViewBinder 
{
	public AgendaViewBinder() {	}
	
	@Override
    public boolean setViewValue(View view, Object data, 
    		String textRepresentation) 
    {
    	if((view instanceof ImageView) & (data instanceof Bitmap)) 
    	{
    		ImageView iv = (ImageView) view;
    		Bitmap bitmap = (Bitmap) data;
    		iv.setImageBitmap(bitmap);
    		return true;
    	}
    	return false;
    }

}
