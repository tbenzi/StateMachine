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

//#include "T:\Arduino\StateMachine\src\Arduino\StateMachine.h"
#include "C:\arduino\Develop\StateMachine\src\Arduino\StateMachine.h"
//#include "..\..\..\src\Arduino\StateMachine.h"

#define MS_CYCLE 1000

bool bEnableStateMachine;

// global data for Statemachine ----------------
typedef struct {bool bChangeZero;
				bool bChangeOne;
				} MY_DATA;

MY_DATA myData;
//----------------------------------------------

CStateMachine StateMachine;

// ---------------------------------------------
// To print the value of bool bEnable
void PrintEnable (MY_DATA* pmyData)
{
	char txt[64];
	sprintf(txt, "bChangeZero:%d bChangeOne:%d",pmyData->bChangeZero,pmyData->bChangeOne);
	Serial.println(txt);
}

// function for ZERO state
void StatusFuncZero(void* pData)
{
	Serial.println("StatusFuncZero");
	PrintEnable ((MY_DATA*)pData);
}

void DropOutFuncZero(void* pData)
{
	Serial.println("DropOutFuncZero");
	PrintEnable ((MY_DATA*)pData);
}

void TransitionFuncZeroToOne(void* pData)
{
	Serial.println("TransitionFuncZeroToOne");
	PrintEnable ((MY_DATA*)pData);
}

void PickUpFuncZero(void* pData)
{
	Serial.println("PickUpFuncZero");
	PrintEnable ((MY_DATA*)pData);
}

int ChangeStateZero (void* pData)
{
	Serial.println("ChangeStateZero");
	PrintEnable ((MY_DATA*)pData);
	MY_DATA* pmyData = (MY_DATA*)pData;
	if (pmyData->bChangeZero)
	{
		pmyData->bChangeZero = false;
		return true;
	}
	return false;
}

// Function to ONE state
void StatusFuncOne(void* pData)
{
	Serial.println("StatusFuncOne");
	PrintEnable ((MY_DATA*)pData);
}

void DropOutFuncOne(void* pData)
{
	Serial.println("DropOutFuncOne");
	PrintEnable ((MY_DATA*)pData);
}

void TransitionFuncOneToZero(void* pData)
{
	Serial.println("TransitionFuncOneToZero");
	PrintEnable ((MY_DATA*)pData);
}

void PickUpFuncOne(void* pData)
{
	Serial.println("PickUpFuncOne");
	PrintEnable ((MY_DATA*)pData);
}

int ChangeStateOne (void* pData)
{
	Serial.println("ChangeStateOne");
	PrintEnable ((MY_DATA*)pData);
	MY_DATA* pmyData = (MY_DATA*)pData;
	if (pmyData->bChangeOne)
	{
		pmyData->bChangeOne = false;
		return true;
	}
	return false;
}

// 
myTransitionFunc pTransitionFuncZero[NUM_STATES] =
{
	nullptr,
	TransitionFuncZeroToOne
};

myTransitionFunc pTransitionFuncOne[NUM_STATES] =
{
	nullptr,
	nullptr
};

void setup ()
{
	// Start the serial interface
	Serial.begin(115200);
	
	bEnableStateMachine = false;
	myData.bChangeZero  = false;
	myData.bChangeOne   = false;
	
	// assign address of global data and cycle to Statemachine
	StateMachine.AssignData(&myData, MS_CYCLE);
	
	// assing data for ZERO state
	StateMachine.AssignState(0,						// state index (zero based)
							StatusFuncZero,			// state function
							DropOutFuncZero,		// drop out function
							pTransitionFuncZero,	// transition fucntion array
							PickUpFuncZero,			// pick up function
							ChangeStateZero,		// change state function
							0,						// maximum allowed time to stack in states (0 = no max)
							0,						// next State to go if exceeded the maximum time in the state
							"Zero"); 				// name of state
							
	// assing data for ONE state
	StateMachine.AssignState(1,						// state index (zero based)
							StatusFuncOne,          // state function
							DropOutFuncOne,         // drop out function
							pTransitionFuncOne,     // transition fucntion array
							PickUpFuncOne,          // pick up function
							ChangeStateOne,         // change state function
							10000,                  // maximum allowed time to stack in states (0 = no max)
							0,                      // next State to go if exceeded the maximum time in the state
							"One");                 // name of state
							
	// Enable buitin log
	StateMachine.EnableLog(true);
	
	delay(500);
	// check the consitency of statemachine build data
	Serial.println(StateMachine.GetInitError ());
	
	// 
	Serial.println(StateMachine.GetInitErrorString());
}

void loop ()
{
	char c;
	if (Serial.available())
	{
		c = Serial.read();
		myData.bChangeZero  = c == '0' ? true : myData.bChangeZero;
		myData.bChangeOne   = c == '1' ? true : myData.bChangeOne;
		bEnableStateMachine = c == 'S' ? true : bEnableStateMachine;
		bEnableStateMachine = c == 'T' ? false : bEnableStateMachine;
		if (c == '?')
		{
			StateMachine.ShowStateData();
		}
	}
	if (bEnableStateMachine)
	{
		StateMachine.Manage();
		Serial.println(StateMachine.GetStatusName());
		Serial.println(StateMachine.GetStatusInd());
		bEnableStateMachine = false;
	}
	delay(MS_CYCLE);
}


