package petrsu.smartroom.android.srcli;

import android.app.Service;
import android.os.IBinder;
import android.content.Intent;
import android.media.MediaRecorder.AudioSource;
import android.media.AudioRecord;
import android.media.AudioFormat;
import android.widget.Toast;

import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.SocketException;
import java.lang.Thread;
import java.net.InetSocketAddress;

import java.io.IOException;
import org.xiph.speex.SpeexEncoder;
import android.util.Log;

/**
 * 
 * @author pavlin
 *
 *	Service allows to interact with microphone
 *	service and use smartphone as a mike
 */
public class MicService extends Service {
	
	private static String ip;					// Mike service IP
	private static String port;					// Mike service port
	private AudioRecord mInputData;
	private int mMinBufferSize;
	private static boolean mActiveMic = false;	// Service activity indicator
	private final int mSampleRate = 11025;
	private final int channels = 1;
	private final int mode = 0;
	private final int samples = 320;
	private final int quality = 10;
	private SpeexEncoder encoder;

	@Override
	public IBinder onBind(Intent intent) {
		return null;
	}
	
	@Override
	public void onStart(Intent intent, int startId) {
		
		ip = KP.getMicServiceIP();
		port = KP.getMicServicePort();
		
		/* If service is not available */
		if((ip == null) || (port == null)) {
			Toast.makeText(this, R.string.mic_service_not_avail, 
					Toast.LENGTH_SHORT)
					.show();
			
			/* Broadcast `stop` mike service status */
			Intent recvIntent = new Intent(Projector.BROADCAST_STATUS_SERVICE);
			recvIntent.putExtra(Projector.SERVICE_STATUS, false);
			sendBroadcast(recvIntent);
			
			stopSelf();
			return;
		}
		
		startRecording();
	}
	
	/**
	 * Initialize recorder and start sending
	 * audio data
	 */
	public void startRecording() {		
		new Thread() {
			@Override
			public void run() {
				initEncoder();
				initRecorder();
				
				mInputData.startRecording();
				mActiveMic = true;
				
				/* While mike is active */
				while(mActiveMic) {
					byte[] audioData = new byte[samples];
					
					/* Read data from smartphone mike */
					mInputData.read(audioData, 0, samples);
					
					/* Encode data with Speex */
					if(!encoder.processData(audioData, 0, samples))
						Log.i("encoding", "failed");
					
					int encDataLen = encoder.getProcessedDataByteSize();
					byte[] encData = new byte[encDataLen];
					
					/* Load processed data to byte array */
					encoder.getProcessedData(encData, 0);
					
					/*Log.i("encoded data size", String.valueOf(
						encoder.getProcessedData(encData, 0)));*/
					
					sendEncodedBytes(encData);
				}
				mActiveMic = false;
				stopRecording();
			}
		}.start();
	}
	
	@Override
	public void onDestroy() {
		mActiveMic = false;
	}
	
	/**
	 * Stops recording and closes
	 * input streams
	 */
	public void stopRecording() {
		try {
			mInputData.stop();
			mInputData.release();
		} catch(IllegalStateException e) {
			e.printStackTrace();
		}
	}
	
	/**
	 * Initializes Speex encoder
	 */
	public void initEncoder() {
		encoder = new SpeexEncoder();
		encoder.init(mode, quality, mSampleRate, channels);
	}

	/**
	 * Initializes audio recorder
	 */
	public void initRecorder() {
		mMinBufferSize = AudioRecord.getMinBufferSize(mSampleRate, 
				AudioFormat.CHANNEL_IN_MONO, AudioFormat.ENCODING_PCM_16BIT);
		
		mInputData = new AudioRecord(AudioSource.MIC, mSampleRate, 
				AudioFormat.CHANNEL_IN_MONO, AudioFormat.ENCODING_PCM_16BIT, 
				mMinBufferSize);
	}
	
	/**
	 * Sends processed data to microphone service
	 * 
	 * @param data - encoded audio data
	 */
	public void sendEncodedBytes(final byte[] data) {
		new Thread() {
			@Override
			public void run() {
				
				try {
					DatagramSocket socket = new DatagramSocket();
					DatagramPacket packet = new DatagramPacket(data, 
							data.length, 
							new InetSocketAddress(ip, Integer.parseInt(port)));
					socket.send(packet);
					socket.close();
				} catch (SocketException e) {
					e.printStackTrace();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}.start();
	}
}
