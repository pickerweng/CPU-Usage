package com.android.cpuusage;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.Reader;
import java.util.StringTokenizer;

import android.app.Activity;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class CpuUsage extends Activity {
	final static String TAG = "CPUUSAGE";
	
	/// Message Cases
	final int UPDATE_CPU_USAGE = 0;
	
	private class CpuStat {
		int usr;
		int nice;
		int sys;
		int idle;
		int iowait;
		int irq;
		int softirq;
		int stead;
		int guest;
		
		CpuStat() {
			usr = nice = sys = idle = iowait 
				= irq = softirq = stead = guest = 0;
		}
	}
	
	private CpuStat mPreCpuStat = new CpuStat();
	private CpuStat mNextCpuStat = new CpuStat();
	private float mCpuUsage = 0f;
	private String mUsageStr;
	
	private static boolean initNative = false;
	
	private TextView mTextView;
	private Button mStartButton;
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        
        mTextView = (TextView) findViewById(R.id.usageText);
        mStartButton = (Button) findViewById(R.id.StartButton);
        
        mStartButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                // Perform action on click
            	new Thread() {
					@Override
					public void run() {
						// TODO Auto-generated method stub
						while (true) {
							//mCpuUsage = getCpuUsage();
							mCpuUsage = nativeGetCpuUsage(1000000);
							Log.v(TAG, "Usage = " + mCpuUsage);
			        		/*
							try {
								Thread.sleep(1000);
							} catch (InterruptedException e) {
								// TODO Auto-generated catch block
								e.printStackTrace();
							}
			        		*/
							
			        		Message msg = new Message();
			        		msg.what = UPDATE_CPU_USAGE;
			        		statHandler.sendMessageDelayed(msg, 1000);
						}
					}
            	}.start();
            }
        });
        
        /// Initial the resource of the native library
        nativeClassInit();
		initNative = true;
    }
    
	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();
		
        /// Destroy the resource of the native library
		nativeClassExit();
		initNative = false;
	}
	
    public void getCpuStat(long msTime) 
    		throws IOException, InterruptedException {
    	File file = new File("/proc/stat");
    	BufferedReader br = new BufferedReader(
    			(Reader) new InputStreamReader(new FileInputStream(file)));
    	StringTokenizer token = new StringTokenizer(br.readLine());
    	token.nextToken();
    	mPreCpuStat.usr = Integer.parseInt(token.nextToken());
    	mPreCpuStat.nice = Integer.parseInt(token.nextToken());
    	mPreCpuStat.sys = Integer.parseInt(token.nextToken());
    	mPreCpuStat.idle = Integer.parseInt(token.nextToken());
    	mPreCpuStat.iowait = Integer.parseInt(token.nextToken());
    	mPreCpuStat.irq = Integer.parseInt(token.nextToken());
    	mPreCpuStat.softirq = Integer.parseInt(token.nextToken());
    	mPreCpuStat.stead = Integer.parseInt(token.nextToken());
    	mPreCpuStat.guest = Integer.parseInt(token.nextToken());

    	Thread.sleep(msTime);
    	br = new BufferedReader(
    			(Reader) new InputStreamReader(new FileInputStream(file)));
    	token = new StringTokenizer(br.readLine());
    	token.nextToken();
    	mNextCpuStat.usr = Integer.parseInt(token.nextToken());
    	mNextCpuStat.nice = Integer.parseInt(token.nextToken());
    	mNextCpuStat.sys = Integer.parseInt(token.nextToken());
    	mNextCpuStat.idle = Integer.parseInt(token.nextToken());
    	mNextCpuStat.iowait = Integer.parseInt(token.nextToken());
    	mNextCpuStat.irq = Integer.parseInt(token.nextToken());
    	mNextCpuStat.softirq = Integer.parseInt(token.nextToken());
    	mNextCpuStat.stead = Integer.parseInt(token.nextToken());
    	mNextCpuStat.guest = Integer.parseInt(token.nextToken());
    }
    
    public float getCpuUsage() throws 
    		IOException, InterruptedException {
    	
    	getCpuStat(1000);
    	
    	float tot_Time1 = (float) (mPreCpuStat.usr + 
					    			mPreCpuStat.nice + 
					    			mPreCpuStat.sys + 
					    			mPreCpuStat.idle + 
					    			mPreCpuStat.iowait + 
					    			mPreCpuStat.irq + 
					    			mPreCpuStat.softirq + 
					    			mPreCpuStat.stead + 
					    			mPreCpuStat.guest);
    	float tot_Time2 = (float) (mNextCpuStat.usr + 
    								mNextCpuStat.nice + 
									mNextCpuStat.sys + 
									mNextCpuStat.idle + 
									mNextCpuStat.iowait + 
									mNextCpuStat.irq + 
									mNextCpuStat.softirq + 
									mNextCpuStat.stead + 
									mNextCpuStat.guest);
    	float tot_Time = tot_Time2 - tot_Time1;
    	float tot_Idle = mNextCpuStat.idle - mPreCpuStat.idle;
    	
    	return ((100.0f * (tot_Time - tot_Idle)) / tot_Time);
    	
    }
    
	private class StatHandler extends Handler {
		@Override
		public void handleMessage(Message msg) {
			// TODO Auto-generated method stub
			switch (msg.what) {
			case UPDATE_CPU_USAGE:
				mUsageStr = Float.toString(mCpuUsage);
				mTextView.setText(mUsageStr);
				break;
			}
			super.handleMessage(msg);
		}
		
	}
	private StatHandler statHandler = new StatHandler();
	
	/**
	 * Load Native Library 
	 */
	static {
		try {
			System.loadLibrary("cpu_usage");
		} catch (UnsatisfiedLinkError e) {
			Log.d(TAG, "CPU Usage library is not found!");
		}   
	}
	private native void nativeClassInit();
	private native void nativeClassExit();
	private native void nativeGtCpuStat(CpuStat cpuStat);
	private native float nativeGetCpuUsage(long _time);
}