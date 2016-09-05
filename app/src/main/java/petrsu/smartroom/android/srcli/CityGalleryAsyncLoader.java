package petrsu.smartroom.android.srcli;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.Drawable;
import android.os.AsyncTask;
import android.support.v7.app.AlertDialog;
import android.text.Html;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;
import android.widget.Toast;

import org.apache.http.HttpStatus;

import java.io.InputStream;
import java.lang.ref.WeakReference;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URISyntaxException;
import java.net.URL;

/**
 * Created by Remediassance on 25.08.2016.
 */
public class CityGalleryAsyncLoader extends AsyncTask<String, Void, Bitmap> {

    ImageView imageView = null;
    Bitmap bitmap;
    private WeakReference<ImageView> imageViewReference;

    public CityGalleryAsyncLoader(ImageView imageView) {
        imageViewReference = new WeakReference<ImageView>(imageView);
    }

    @Override
    protected Bitmap doInBackground(String... params) {
        return downloadBitmap(params[0]);
    }

    @Override
    protected void onPostExecute(Bitmap result) {
        if(isCancelled()){
            bitmap = null;
        }

        if(imageViewReference != null) {
            ImageView imageView = imageViewReference.get();
            if(imageView != null) {
                if (bitmap != null) {
                    imageView.setImageBitmap(bitmap);
                }
                else {
                    Drawable placeholder = imageView.getContext().getResources().getDrawable(R.drawable.not_available);
                    imageView.setImageDrawable(placeholder);
                }
            }
        }
    }

    private Bitmap downloadBitmap(String url){
        HttpURLConnection conn = null;
        try{
            URL uri = new URL(url);
            conn = (HttpURLConnection) uri.openConnection();
            conn.setInstanceFollowRedirects(true);
            HttpURLConnection.setFollowRedirects(true);

            int status = conn.getResponseCode();
            /*if (status == 301) {
                String location = conn.getHeaderField("Location");
                Log.i("Status is", String.valueOf(status));
                Log.i("Link is a redirect to ", location);
                conn = (HttpURLConnection)(new URL(location).openConnection());
            }*/


            if(status != HttpStatus.SC_OK)
                return null;

            InputStream inputStream = conn.getInputStream();

            if(inputStream != null) {
                bitmap = BitmapFactory.decodeStream(inputStream);
                return bitmap;
            }

        } catch(Exception e){
            conn.disconnect();
            Log.w("downloadBitmap()", "Error in getting bitmap "+ url);
            e.printStackTrace();
        }
        finally{
            if(conn != null)
                conn.disconnect();
        }
        return null;
    }

    protected void onProgressUpdate(Integer... progress) {
        CityGallery.progressBar.setProgress(progress[0]);
    }

}
