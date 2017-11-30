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

#define MS_CYCLE 1000

// global data for Statemachine ----------------
typedef struct { bool bEnable;} MY_DATA;

MY_DATA myData;
//----------------------------------------------

CStateMachine StateMachine;

// ---------------------------------------------
// To print the value of bool bEnable
void PrintEnable (MY_DATA* pmyData)
{
	char c = pmyData->bEnable ? '1' : '0';
	Serial.print("bEnable:");
	Serial.println(c);
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

int ChangeStateZero (void* pData)
{
	MY_DATA* pmyData = (MY_DATA*)pData;
	return pmyData->bEnable ? 1 : 0;
}

// 
myTransitionFunc pTransitionFuncZero[NUM_STATES] =
{
	nullptr,
	TransitionFuncZeroToOne
};

void setup ()
{
	// Start the serial interface
	Serial.begin(115200);

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
							nullptr,                // transition fucntion array
							PickUpFuncOne,          // pick up function
							nullptr,                // change state function
							2000,                   // maximum allowed time to stack in states (0 = no max)
							0,                      // next State to go if exceeded the maximum time in the state
							"One");                 // name of state
							
	// Enable buitin log
	StateMachine.EnableLog(true);
	
	// check the consitency of statemachine build data
	Serial.println(StateMachine.GetInitError ());
	
	// 
	Serial.println(StateMachine.GetInitErrorString());

}

void loop ()
{
	if (Serial.available())
	{
		char c = Serial.read();
		myData.bEnable = c == 'E' ? true : false;
	}
	StateMachine.Manage();
	Serial.println(StateMachine.GetStatusName());
	Serial.println(StateMachine.GetStatusInd());
	delay(MS_CYCLE);
}


