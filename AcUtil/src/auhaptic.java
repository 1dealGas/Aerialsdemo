package com.acutil.haptic;

import java.lang.Runnable;
import android.app.Activity;

class HapticExtension {
	private static final String TAG = "haptic";
	public static void DoHapticFeedback(final Activity A) {
		A.runOnUiThread(
			new Runnable() {
				@Override
				public void run() {
					A.getWindow().getDecorView().performHapticFeedback(0,1);
				}
			}
		);
	}
}