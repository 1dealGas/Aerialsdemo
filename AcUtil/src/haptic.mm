#ifdef DM_PLATFORM_IOS   /* Aerials Haptic Feedback iOS Implementation */
#include <dmsdk/sdk.h>
#include "UIKit/UIKit.h"

void AcUtilDoHapticFeedback() {
	UIImpactFeedbackGenerator *H = [[UIImpactFeedbackGenerator alloc] initWithStyle:UIImpactFeedbackStyleMedium];
	[H prepare];	[H impactOccurred];
	 H = NULL;   // ARC
}
#endif