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

typedef enum {	NO_ERROR 					= 0,
				ALREADY_DEFINED_STATE 		= 1,
				STATE_WITHOUT_CHANGE		= 2,
				UNDFINED_CYCLE_TIME			= 3,
				IND_STATE_OVER_MAX			= 4,
				IND_STATE_OVER_MAX_IN_TRANS = 5,
				NO_CHANGE_STATE_DEFINED		= 6,
				WRONG_IND_STATE_MAX_IN_TRANS = 7,
				MAX_NUM_ERROR 
} E_STATE_MACHINE_ERROR;

struct STR_ERR {
	const char* s;
};
STR_ERR stateMachineErrorString[MAX_NUM_ERROR] =
			{{"NO_ERROR"},
			 {"ALREADY_DEFINED_STATE"},
			 {"STATE_WITHOUT_CHANGE"},
			 {"UNDFINED_CYCLE_TIME"},
			 {"IND_STATE_OVER_MAX"},
			 {"IND_STATE_OVER_MAX_IN_TRANS"},
			 {"NO_CHANGE_STATE_DEFINED"},
			 {"WRONG_IND_STATE_MAX_IN_TRANS"},
};

class CStateMachine
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
		E_STATE_MACHINE_ERROR m_StateError;
	public:
		CStateMachine(int num_status, void* pstruct_data = nullptr, int msecCycle = 0)
		{
			m_StateError = NO_ERROR;
			m_numStatus = num_status;
			m_vfStatus.reserve(num_status);
			m_vfDropOut.reserve(num_status);
			m_vfTransition.reserve(num_status*num_status);
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
//
// AssignState
//
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
					if ((MaxMsInStatus == 0) || (NextStatusIfOverMaxMsInStatus == ind))
					{
						m_StateError = NO_CHANGE_STATE_DEFINED;
					}
				}
				if (m_vfChangeStatusFunc[ind] != nullptr)
				{
					throw invalid_argument("Status ind already defined"); 
					m_StateError = ALREADY_DEFINED_STATE;
				}
				if ((MaxMsInStatus > 0) && (m_msecCycle == 0))
				{
					m_StateError = UNDFINED_CYCLE_TIME;
				}
				if ((MaxMsInStatus > 0) && (NextStatusIfOverMaxMsInStatus == ind))
				{
					m_StateError = WRONG_IND_STATE_MAX_IN_TRANS;
				}
				if (NextStatusIfOverMaxMsInStatus >= NUM_STATES)
				{
					m_StateError = IND_STATE_OVER_MAX_IN_TRANS;
				}
				if (m_StateError != NO_ERROR)
				{
					return;
				}
				// Parameters are OK
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
				m_StateError = IND_STATE_OVER_MAX;
				throw invalid_argument("Status ind too large."); 
			}
		}
//
// Manage 
//
		void Manage()
		{
			char txt[124];
			if (m_bLogEnable)
			{
				sprintf(txt,"StateMachine::Manage - State: %s [%d] -------- ", m_StatusName[m_actualStatus],m_actualStatus);
//??				Serial.println(txt);
			}
			if (m_StateError != NO_ERROR)
			{
				if (m_bLogEnable)
				{
					sprintf(txt,"StateMachine::Manage - Init Data Error:%d Exit!",m_StateError);
//??					Serial.println(txt);
				}
				return;
			}
			bool bMaxMsInStatus = false;
			
			// Things to do in actual state
			if (*m_vfStatus[m_actualStatus] != nullptr)
			{
				if (m_bLogEnable)
				{
					sprintf(txt,"StateMachine::Manage - Call State function");
//??					Serial.println(txt);
				}
				(*m_vfStatus[m_actualStatus])(m_pStructData);
			}
			
			// Check the stay time in the state
			// if exceeded the maximum, perform a state change
			// otherwise
			// call the chageStatus function
			if (m_vMaxMsInStatus[m_actualStatus] > 0)
			{
				m_vMsInStatus[m_actualStatus] += m_msecCycle;
				if (m_bLogEnable)
				{
					sprintf(txt,"StateMachine::Manage - MaxMsInStatus:%d Check the stay time in the state, now is:%d",
								m_MaxMsInStatus[m_actualStatus],
								m_MsInStatus[m_actualStatus]);
//??					Serial.println(txt);
				}
				bMaxMsInStatus = m_vMsInStatus[m_actualStatus] >= m_vMaxMsInStatus[m_actualStatus];
			}
			if (bMaxMsInStatus)
			{
				m_MsInStatus[m_actualStatus] = 0;
				m_actualStatus = m_vNextStatusIfExceededMaxMsInStatus[m_actualStatus];
 				if (m_bLogEnable)
				{
					sprintf(txt,"StateMachine::Manage - Exceeded the maximum, perform a state change to:, m_actualStatus");
//??					Serial.println(txt);
				}
			}
			else
			{
				if (m_fChangeStatusFunc[m_actualStatus] != nullptr)
				{
				m_actualStatus = (*m_vfChangeStatusFunc[m_actualStatus])(m_pStructData);
					sprintf(txt,"StateMachine::Manage - Called ChangeState function go in:%d",m_actualStatus);
//??				Serial.println(txt);
				}
			}
			
			// if the state is changed call the defined functions
			if (m_actualStatus != m_oldStatus)
			{
				if (m_bLogEnable)
				{
					ostringstream stxt;
					stxt = "Change from " << m_vStatusName[m_oldStatus] << " to " << m_vStatusName[m_actualStatus];
//??					Serial.println(stxt.str());
				}
				if (*m_vfDropOut[m_oldStatus] != nullptr)
				{
					if (m_bLogEnable)
					{
						sprintf(txt,"StateMachine::Manage - Call DropOut function");
//??						Serial.println(txt);
					}
					(*m_vfDropOut[m_oldStatus])(m_pStructData);
				}
				if (*m_vfTransition[m_oldStatus][m_actualStatus] != nullptr)	 //*************************************
				{
					if (m_bLogEnable)
					{
						sprintf(txt,"StateMachine::Manage - Call Transition function");
//??						Serial.println(txt);
					}
					(*m_vfTransition[m_oldStatus][m_actualStatus])(m_pStructData);//*************************************
				}
				m_oldStatus = m_actualStatus;
				if (*m_vfPickUp[m_oldStatus] != nullptr)
				{
					if (m_bLogEnable)
					{
						sprintf(txt,"StateMachine::Manage - Call PickUp function");
//??						Serial.println(txt);
					}
					(*m_vfPickUp[m_oldStatus])(m_pStructData);
				}
			}
		};
		
		// call GetInitError after all AssingState calling to check consintecy of init value
		E_STATE_MACHINE_ERROR GetInitError ()	{ return m_StateError; };
		
		const char* GetInitErrorString() { return stateMachineErrorString[m_StateError].s; };

		int GetStatusInd() { return m_actualStatus; };
		const char* GetStatusName () { return m_StatusName[m_actualStatus]; };
		void EnableLog (bool b_enable = false) { m_bLogEnable = b_enable; };
//
// ShowStateData 
//
		void ShowStateData ()
		{
			char txt[64];
//??			Serial.println(" ----- StateMachine::ShowStateData ------");
			sprintf(txt,"pStructData: %s",m_pStructData!=nullptr?"defined":"-");
//??			Serial.println(txt);
			for (int i = 0; i < NUM_STATES; i++)
			{
				sprintf(txt,"----- ind:%d - %s -----", i, m_StatusName[i]);
//??				Serial.println(txt);
				sprintf(txt,"fStatus:  %s",m_fStatus[i]!=nullptr?"defined":"-");
//??				Serial.println(txt);
				sprintf(txt,"fDropOut: %s",m_fDropOut[i]!=nullptr?"defined":"-");
//??				Serial.println(txt);
				for (int j = 0; j < NUM_STATES; j++)
				{
					sprintf(txt,"fTransition[%d]: %s",j,m_fTransition[i][j]!=nullptr?"defined":"-");
//??					Serial.println(txt);
				}
				sprintf(txt,"fPickUp: %s",m_fPickUp[i]!=nullptr?"defined":"-");
//??				Serial.println(txt);
				sprintf(txt,"fChangeStatusFunc: %s",m_fChangeStatus[i]!=nullptr?"defined":"-");
//??				Serial.println(txt);
				sprintf(txt,"MaxMsInStatus: %d",m_MaxMsInStatus[i]);
//??				Serial.println(txt);
				sprintf(txt,"NextStatusIfExceededMaxMsInStatus: %d",m_NextStatusIfExceededMaxMsInStatus[i]);
//??				Serial.println(txt);
			}
		}
};


