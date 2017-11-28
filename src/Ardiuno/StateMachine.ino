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
// ******************************
// CODE EMBRYO UNDER DEVELOPMENT
// ******************************

typedef void    (*myStatusFunc)(void* pStructData);
typedef void    (*myDropOutFunc)(void* pStructData);
typedef void    (*myTransitionFunc)(void* pStructData);
typedef void    (*myPickUpFunc)(void* pStructData);
typedef int     (*myChangeStatusFunc)(void* pStructData);

class StateMachine
{
		myStatusFunc m_fStatus[NUM_STATES];					// State functions vector
		myDropOutFunc m_fDropOut[NUM_STATES];					// DropOut (from a state) functions vector
		myTransitionFunc m_fTransition[NUM_STATES][NUM_STATES];	// Transition (from a specify state to another) functions vector
		myPickUpFunc m_fPickUp[NUM_STATES];					// PickUp (into a state) functions vector
		myChangeStatusFunc m_fChangeStatusFunc[NUM_STATES];	// ChangeStatus functions vector
		const char* m_StatusName[NUM_STATES];					// State names vector
		int m_MaxMsInStatus[NUM_STATES];						// Maximum allowed time to stack in any states vector 
		int m_MsInStatus[NUM_STATES];
		int m_NextStatusIfExceededMaxMsInStatus[NUM_STATES];	// Next State to go if exceeded the maximum time in the state
		int m_actualStatus;
		int m_oldStatus;
		void* m_pStructData;
		bool m_bLogEnable;
		int	m_msecCycle;
		
	public:
		StateMachine(ivoid* pstruct_data = nullptr, int msecCycle = 0)
		{
			if (m_pStructData == nullptr)
			{
				//throw invalid_argument("Undefined data"); 
			}
			m_pStructData = pstruct_data;
			m_msecCycle = msecCycle;
			m_actualStatus = 0;
			m_oldStatus = 0;
			m_bLogEnable = false;
			for (int i = 0; i++; i < NUM_STATES)
			{
				m_MaxMsInStatus[i] = 0;
			}
		}
		virtual ~StateMachine();
		void AssingStatus(int ind,
						  myStatusFunc 				fStatus = nullptr,
						  myDropOutFunc 			fDropOut = nullptr,
						  myTransitionFunc* 		fTransition = nullptr,	 //*************************************
						  myPickUpFunc				fPickUp = nullptr,
						  myChangeStatusFunc		fChangeStatusFunc = nullptr,
						  int						MaxMsInStatus = 0,
						  int						NextStatusIfOverMaxMsInStatus = 0,
						  const char*				stausName = nullptr) 
		{
			if (ind < NUM_STATES)
			{
				if (fChangeStatusFunc == nullptr)
				{
					//throw invalid_argument("Undefined change status function"); 
				}
				ind;
				if (m_fChangeStatusFunc[ind] != nullptr)
				{
					//throw invalid_argument("Status ind already defined"); 
				}
				m_fStatus[ind] = fStatus;
				m_fDropOut[ind] = fDropOut;
					 //*************************************
				for (int i = 0; i < NUM_STATES; i++)
				{
					m_fTransition[ind][i] = fTransition[i];
				}
					//*************************************
				m_fPickUp[ind] = fPickUp;
				m_fChangeStatusFunc[ind] = fChangeStatusFunc;
				if ((MaxMsInStatus > 0) && (m_msecCycle == 0))
				{
					//throw invalid_argument("Undefined cycles time"); 
				}
				m_MaxMsInStatus[ind] = MaxMsInStatus;
				m_NextStatusIfExceededMaxMsInStatus[ind] = NextStatusIfOverMaxMsInStatus - 1;
				m_StatusName [ind] = stausName;
			}
			else
			{
				//throw invalid_argument("Status ind too large."); 
			}
		}
		
		void manage()
		{
			bool bMaxMsInStatus;
			
			// Things to do in actual state
			if (*m_fStatus[m_actualStatus] != nullptr)
			{
				(*m_fStatus[m_actualStatus])(m_pStructData);
			}
			
			// Check the stay time in the state
			// if exceeded the maximum, perform a state change
			// otherwise
			// call the chageStatus function
			if (m_MaxMsInStatus[m_actualStatus] > 0)
			{
				m_MsInStatus[m_actualStatus] += m_msecCycle;
				bMaxMsInStatus = m_MsInStatus[m_actualStatus] >= m_MaxMsInStatus[m_actualStatus];
			}
			if (bMaxMsInStatus)
			{
				m_actualStatus = m_NextStatusIfExceededMaxMsInStatus[m_actualStatus];
			}
			else
			{
				m_actualStatus = (*m_fChangeStatusFunc[m_actualStatus])(m_pStructData);
			}
			
			// if the state is changed call the defined functions
			if (m_actualStatus != m_oldStatus)
			{
				if (m_bLogEnable)
				{
					ostringstream stxt;
					stxt = "Change from " << m_StatusName[m_oldStatus] << " to " << m_StatusName[m_actualStatus];
					Serial.println(stxt.str());
				}
				if (*m_fDropOut[m_oldStatus] != nullptr)
				{
					(*m_fDropOut[m_oldStatus])(m_pStructData);
				}
				if (*m_fTransition[m_oldStatus][m_actualStatus] != nullptr)	 //*************************************
				{
					(*m_fTransition[m_oldStatus][m_actualStatus])(m_pStructData);//*************************************
				}
				m_oldStatus = m_actualStatus;
				if (*m_fPickUp[m_oldStatus] != nullptr)
				{
					(*m_fPickUp[m_oldStatus])(m_pStructData);
				}
			}
		};
		
		// call checkStatusConsistency after all AssingStatus calling to check consintecy of value
		bool CheckStatusConsistency ()
		{
			for (int i = 0; i < NUM_STATES; i++)
			{
				if (m_NextStatusIfExceededMaxMsInStatus[i] >= NUM_STATES) return false;
			}
			return true;
		}
		
		int GetStatusInd() { return m_actualStatus + 1; };
		const char* GetStatusName () { return m_StatusName[m_actualStatus]; };
		void EnableLog (bool b_enable = false) { m_bLogEnable = b_enable; };
		 

		
};


