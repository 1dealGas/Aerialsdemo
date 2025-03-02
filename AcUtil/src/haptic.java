package ideal.acutil;

import java.lang.Runnable;
import android.app.Activity;

class Haptic {
	public static void DoHapticFeedback(final Activity A) {
		A.runOnUiThread(
			new Runnable() {
				@Override public void run() {
					A.getWindow().getDecorView().performHapticFeedback(0,1);
				}
			}
		);
	}
}