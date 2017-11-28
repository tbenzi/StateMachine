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
		vector<myStatusFunc> m_vfStatus;					// State functions vector
		vector<myDropOutFunc> m_vfDropOut;					// DropOut (from a state) functions vector
		vector<vector<myTransitionFunc>> m_vfTransition;	// Transition (from a specify state to another) functions vector
		vector<myPickUpFunc> m_vfPickUp;					// PickUp (into a state) functions vector
		vector<myChangeStatusFunc> m_vfChangeStatusFunc;	// ChangeStatus functions vector
		vector<const char*> m_vStatusName;					// State names vector
		vector<int> m_vMaxMsInStatus;						// Maximum allowed time to stack in any states vector 
		vector<int> m_MsInStatus;
		vector<int> m_vNextStatusIfExceededMaxMsInStatus;	// Next State to go if exceeded the maximum time in the state
		int m_actualStatus;
		int m_oldStatus;
		int m_numStatus;
		void* m_pStructData;
		bool m_bLogEnable;
		int	m_msecCycle;
		
	public:
		StateMachine(int num_status, void* pstruct_data = nullptr, int msecCycle = 0)
		{
			if (m_pStructData == nullptr)
			{
				throw invalid_argument("Undefined data"); 
			}
			m_numStatus = num_status;
			m_vfStatus.reserve(num_status);
			m_vfDropOut.reserve(num_status);
			m_vfTransition.reserve(num_status*num_status);	 //*************************************
			m_vfPickUp.reserve(num_status);
			m_vfChangeStatusFunc.reserve(num_status);
			m_vStatusName.reserve(num_status);
			m_vMaxMsInStatus.reserve(num_status);
			m_vbOverMaxMsInStatus.reserve(num_status);
			m_vNextStatusIfExceededMaxMsInStatus.reserve(num_status);
			m_pStructData = pstruct_data;
			m_msecCycle = msecCycle;
			m_actualStatus = 0;
			m_oldStatus = 0;
			m_bLogEnable = false;
			for (int i = 0; i < NUM_STATES; i++)
			{
				m_vMaxMsInStatus[i] = 0;
			}
		}
		virtual ~StateMachine();
		void AssignState(int ind,
						  myStatusFunc 				fStatus = nullptr,
						  myDropOutFunc 			fDropOut = nullptr,
						  vector<myTransitionFunc> 	fTransition = nullptr,	 //*************************************
						  myPickUpFunc				fPickUp = nullptr,
						  myChangeStatusFunc		fChangeStatusFunc = nullptr,
						  int						MaxMsInStatus = 0,
						  int						NextStatusIfOverMaxMsInStatus = 0,
						  const char*				stausName = nullptr) 
		{
			if (ind <= num_status)
			{
				if (fChangeStatusFunc == nullptr)
				{
					throw invalid_argument("Undefined change status function"); 
				}
				if (m_vfChangeStatusFunc[ind] != nullptr)
				{
					throw invalid_argument("Status ind already defined"); 
				}
				m_vfStatus[ind] = fStatus;
				m_vfDropOut[ind] = fDropOut;
					 //*************************************
				for (int i = 0; i < num_status; i++)
				{
					m_vfTransition[ind][i] = fTransition[i];
				}
					//*************************************
				m_vfPickUp[ind] = fPickUp;
				m_vfChangeStatusFunc[ind] = fChangeStatusFunc;
				if ((MaxMsInStatus > 0) && (m_msecCycle == 0))
				{
					throw invalid_argument("Undefined cycles time"); 
				}
				m_vMaxMsInStatus[ind] = MaxMsInStatus;
				m_vNextStatusIfExceededMaxMsInStatus[ind] = NextStatusIfOverMaxMsInStatus - 1;
				m_vStatusName [ind] = stausName;
			}
			else
			{
				throw invalid_argument("Status ind too large."); 
			}
		}
		
		void Manage()
		{
			bool bMaxMsInStatus;
			
			// Things to do in actual state
			if (*m_vfStatus[m_actualStatus] != nullptr)
			{
				(*m_vfStatus[m_actualStatus])(m_pStructData);
			}
			
			// Check the stay time in the state
			// if exceeded the maximum, perform a state change
			// otherwise
			// call the chageStatus function
			if (m_vMaxMsInStatus[m_actualStatus] > 0)
			{
				m_vMsInStatus[m_actualStatus] += m_msecCycle;
				bMaxMsInStatus = m_vMsInStatus[m_actualStatus] >= m_vMaxMsInStatus[m_actualStatus];
			}
			if (bMaxMsInStatus)
			{
				m_actualStatus = m_vNextStatusIfExceededMaxMsInStatus[m_actualStatus];
			}
			else
			{
				m_actualStatus = (*m_vfChangeStatusFunc[m_actualStatus])(m_pStructData);
			}
			
			// if the state is changed call the defined functions
			if (m_actualStatus != m_oldStatus)
			{
				if (m_bLogEnable)
				{
					ostringstream stxt;
					stxt = "Change from " << m_vStatusName[m_oldStatus] << " to " << m_vStatusName[m_actualStatus];
					Serial.println(stxt.str());
				}
				if (*m_vfDropOut[m_oldStatus] != nullptr)
				{
					(*m_vfDropOut[m_oldStatus])(m_pStructData);
				}
				if (*m_vfTransition[m_oldStatus][m_actualStatus] != nullptr)	 //*************************************
				{
					(*m_vfTransition[m_oldStatus][m_actualStatus])(m_pStructData);//*************************************
				}
				m_oldStatus = m_actualStatus;
				if (*m_vfPickUp[m_oldStatus] != nullptr)
				{
					(*m_vfPickUp[m_oldStatus])(m_pStructData);
				}
			}
		};
		
		// call checkStatusConsistency after all AssingStatus calling to check consintecy of value
		bool CheckStatusConsistency ()
		{
			for (int i = 0; i < m_numStatus; i++)
			{
				if (m_vNextStatusIfExceededMaxMsInStatus[i] >= m_numStatus) return false;
			}
			return true;
		}
		
		int GetStatusInd() { return m_actualStatus + 1; };
		const char* GetStatusName () { return m_vStatusName[m_actualStatus]; };
		void EnableLog (bool b_enable = false) { m_bLogEnable = b_enable; };
		 

		
};


