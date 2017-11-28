// MIT License
// 
// Copyright (c) 2017 tbenzi Tullio Benzi
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define NUM_STATES 2

#include "StateMachine.h"

#define MS_CYCLE 1000

typedef struct { bool bEnable;} MY_DATA;

MY_DATA myData;

CStateMachine StateMachine;

int ChangeStateZero (void* pData)
{
	MY_DATA* pmyData = (MY_DATA*)pData;
	return pmyData->bEnable ? 1 : 0;
}

void setup ()
{
	// Start the serial interface
	Serial.begin(115200);

	StateMachine.AssignData(&myData, MS_CYCLE);
	StateMachine.AssignState(0,
							nullptr,
							nullptr,
							nullptr,
							nullptr,
							ChangeStateZero,
							0,
							0,
							"zero"); 
	StateMachine.AssignState(1,
							nullptr,
							nullptr,
							nullptr,
							nullptr,
							nullptr,
							2000,
							0,
							"one"); 
	StateMachine.EnableLog(true);
}

void loop ()
{
	if (Serial.available())
	{
		char c = Serial.read();
		myData.bEnable = c == 'E' ? true : false;
	}
	StateMachine.Manage();
	delay(MS_CYCLE);
}


