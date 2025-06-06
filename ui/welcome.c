/* Copyright 2023 Dual Tachyon
 * https://github.com/DualTachyon
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
 */

#include "radio/settings.h"
#include "ui/gfx.h"
#include "ui/helper.h"

void UI_DrawWelcome(void)
{
	gColorForeground = COLOR_FOREGROUND;
	UI_DrawString(10, 85, myCALL, sizeof(myCALL)); /// "Personal ID"
	
	gColorForeground = COLOR_RED;
	UI_DrawString(40, 60, "RT-890", 6);
	///UI_DrawString(gSettings.WelcomeX, gSettings.WelcomeY, WelcomeString, sizeof(WelcomeString));
		UI_DrawString(40, 40, WelcomeString, sizeof(WelcomeString)); /// now, "Serial Number"
}

