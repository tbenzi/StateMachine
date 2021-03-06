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

        void _PrintLog(const char *string, const char *format, ...)
        __attribute__((format (sprintf, 3, 4)));
        #define PrintLog(char* string, char* format, fmt, ...) _PrintLog(char* string, char* format, fmt"\n", ##__VA_ARGS__)
        void _PrintLog (const char *string, const char *format, ...)
        {
//            va_list arg;
            char txt[128];

            if (!m_bLogEnable) return;
            
//            va_start(arg, args);
            sprintf (txt, string, format, __VA_ARGS__);
            Serial.println(txt);
//            va_end(arg);
        }


typedef void    (*myStatusFunc)(void* pStructData);
typedef void    (*myDropOutFunc)(void* pStructData);
typedef void    (*myTransitionFunc)(void* pStructData);
typedef void    (*myPickUpFunc)(void* pStructData);
typedef int     (*myChangeStatusFunc)(void* pStructData);

typedef enum {  NO_ERROR                    = 0,
                ALREADY_DEFINED_STATE       = 1,
                STATE_WITHOUT_CHANGE        = 2,
                UNDFINED_CYCLE_TIME         = 3,
                IND_STATE_OVER_MAX          = 4,
                IND_STATE_OVER_MAX_IN_TRANS = 5,
                NO_CHANGE_STATE_DEFINED     = 6,
                WRONG_IND_STATE_MAX_IN_TRANS = 7,
                MALLOC_ERROR = 8,
                NUM_STATES_EQ_ZERO = 9,
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
             {"MALLOC_ERROR"},
             {"NUM_STATES_EQ_ZERO"}
};

typedef const char* myPName;

class CStateMachine
{
        myStatusFunc*       m_fStatus;                      // State functions vector
        myDropOutFunc*      m_fDropOut;                     // DropOut (from a state) functions vector
        myTransitionFunc**  m_fTransition;                  // Transition (from a specify state to another) functions vector
        myPickUpFunc*       m_fPickUp;                      // PickUp (into a state) functions vector
        myChangeStatusFunc* m_fChangeStatus;                // ChangeStatus functions vector
        myPName*            m_StatusName;                   // State names vector
        int*                m_MaxMsInStatus;                // Maximum allowed time to stack in any states vector 
        int*                m_MsInStatus;                   // Counter ms in state
        int*                m_NextStatusIfExceededMaxMsInStatus; // Next State to go if exceeded the maximum time in the state
        int m_numStates;           // states number
        int m_actualState;         // actual active state 
        int m_oldStatus;           // previous active state
        void* m_pStructData;       // pointer tu user data
        bool m_bLogEnable;         // log enable
        int m_msecCycle;           // 
        E_STATE_MACHINE_ERROR m_StateError; // error code detected
    public:
    
        CStateMachine()
        {
            m_fStatus = nullptr;        
            m_fDropOut = nullptr;       
            m_fTransition = nullptr;    
            m_fPickUp = nullptr;        
            m_fChangeStatus = nullptr;  
            m_StatusName = nullptr;     
            m_pStructData = nullptr;
            m_StateError = NO_ERROR;
            m_numStates = 0;
            m_actualState = 0;
            m_oldStatus = 0;
            m_bLogEnable = false;
        };
        virtual ~CStateMachine() 
        {
            if(m_fStatus!= nullptr) free(m_fStatus);
            if(m_fDropOut!= nullptr )free(m_fDropOut);
            if(m_fTransition!= nullptr) free(m_fTransition);
            if(m_fPickUp!= nullptr) free(m_fPickUp);
            if(m_fChangeStatus!= nullptr) free(m_fChangeStatus);
            if(m_StatusName!= nullptr) free(m_StatusName);
            if(m_MaxMsInStatus!= nullptr) free(m_MaxMsInStatus);
            if(m_MsInStatus!= nullptr) free(m_MsInStatus);
            if(m_NextStatusIfExceededMaxMsInStatus!= nullptr) free(m_NextStatusIfExceededMaxMsInStatus);
        };
        
        bool MemAlloc (void* pv, size_t size)
        {
            pv = malloc(size);
            if (pv == nullptr)
            {
                m_StateError = IND_STATE_OVER_MAX_IN_TRANS;
                return false;
            }
            return true;
        }
        
//
// AssignData
//
        bool AssignData(int num_states = 0, void* pstruct_data = nullptr, int msecCycle = 0)
        {
            m_numStates     = num_states;
            if (num_states == 0)
            {
                m_StateError = NUM_STATES_EQ_ZERO;
                return false;
            }
            m_pStructData   = pstruct_data;
            m_msecCycle     = msecCycle;
            for (int i = 0; i < num_states; i++)
            {
                m_MaxMsInStatus[i] = 0;
            }
            if (!MemAlloc (m_fStatus,       sizeof(myStatusFunc*)*num_states))   return false;
            if (!MemAlloc (m_fDropOut,      sizeof(myDropOutFunc*)*num_states))  return false;
            if (!MemAlloc (m_fTransition,   sizeof(myTransitionFunc*)*num_states)) return false;
            for (int i = 0; i < num_states; i++)
            {
                if (!MemAlloc (m_fTransition[i], sizeof(myTransitionFunc*)*num_states)) return false;
            }
            if (!MemAlloc (m_fPickUp,       sizeof(myPickUpFunc*)*num_states))       return false;
            if (!MemAlloc (m_fChangeStatus, sizeof(myChangeStatusFunc*)*num_states)) return false;
            if (!MemAlloc (m_StatusName,    sizeof(myPName*)*num_states))            return false;
            if (!MemAlloc (m_MaxMsInStatus, sizeof(int*)*num_states))                return false;
            if (!MemAlloc (m_MsInStatus,    sizeof(int*)*num_states))                return false;
            if (!MemAlloc (m_NextStatusIfExceededMaxMsInStatus, sizeof(int*)*num_states)) return false;
            return true;
        };
//
// AssignState
//
        void AssignState(int ind,
                        myStatusFunc       fStatus = nullptr,
                        myDropOutFunc      fDropOut = nullptr,
                        myTransitionFunc*  fTransition = nullptr,
                        myPickUpFunc       fPickUp = nullptr,
                        myChangeStatusFunc fChangeStatusFunc = nullptr,
                        int                MaxMsInStatus = 0,
                        int                NextStatusIfOverMaxMsInStatus = 0,
                        const char*        stausName = nullptr) 
        {
            if (ind < m_numStates)
            {   
                // Check Parameters
                if (fChangeStatusFunc == nullptr)
                {
                    if ((MaxMsInStatus == 0) || (NextStatusIfOverMaxMsInStatus == ind))
                    {
                        m_StateError = NO_CHANGE_STATE_DEFINED;
                    }
                }
                if (m_fChangeStatus[ind] != nullptr)
                {
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
                if (NextStatusIfOverMaxMsInStatus >= m_numStates)
                {
                    m_StateError = IND_STATE_OVER_MAX_IN_TRANS;
                }
                if (m_StateError != NO_ERROR)
                {
                    return;
                }
                // Parameters are OK
                m_fStatus[ind] = fStatus;
                m_fDropOut[ind] = fDropOut;
                for (int i = 0; i < m_numStates; i++)
                {
                    m_fTransition[ind][i] = fTransition[i];
                }
                m_fPickUp[ind] = fPickUp;
                m_fChangeStatus[ind] = fChangeStatusFunc;
                m_MaxMsInStatus[ind] = MaxMsInStatus;
                m_NextStatusIfExceededMaxMsInStatus[ind] = NextStatusIfOverMaxMsInStatus;
                m_StatusName [ind] = stausName;
            }
            else
            {
                m_StateError = IND_STATE_OVER_MAX;
            }
        };
//
// Manage 
//
        void Manage()
        {
            char txt[124];
            PrintLog ("StateMachine::Manage - State: %s [%d] -------- ", m_StatusName[m_actualState],m_actualState);
//            if (m_bLogEnable)
//            {
//                sprintf(txt,"StateMachine::Manage - State: %s [%d] -------- ", m_StatusName[m_actualState],m_actualState);
//                Serial.println(txt);
//            }
            if (m_StateError != NO_ERROR)
            {
                if (m_bLogEnable)
                {
                    sprintf(txt,"StateMachine::Manage - Init Data Error:%d Exit!",m_StateError);
                    Serial.println(txt);
                }
                return;
            }
            bool bMaxMsInStatus = false;
            
            // Things to do in actual state
            if (*m_fStatus[m_actualState] != nullptr)
            {
                if (m_bLogEnable)
                {
                    sprintf(txt,"StateMachine::Manage - Call State function");
                    Serial.println(txt);
                }
                (*m_fStatus[m_actualState])(m_pStructData);
            }
            
            // Check the stay time in the state
            // if exceeded the maximum, perform a state change
            // otherwise
            // call the chageStatus function
            if (m_MaxMsInStatus[m_actualState] > 0)
            {
                m_MsInStatus[m_actualState] += m_msecCycle;
                if (m_bLogEnable)
                {
                    sprintf(txt,"StateMachine::Manage - MaxMsInStatus:%d Check the stay time in the state, now is:%d",
                                m_MaxMsInStatus[m_actualState],
                                m_MsInStatus[m_actualState]);
                    Serial.println(txt);
                }
                bMaxMsInStatus = m_MsInStatus[m_actualState] >= m_MaxMsInStatus[m_actualState];
            }
            if (bMaxMsInStatus)
            {
                m_MsInStatus[m_actualState] = 0;
                m_actualState = m_NextStatusIfExceededMaxMsInStatus[m_actualState];
                if (m_bLogEnable)
                {
                    sprintf(txt,"StateMachine::Manage - Exceeded the maximum, perform a state change to:, m_actualState");
                    Serial.println(txt);
                }
            }
            else
            {
                if (m_fChangeStatus[m_actualState] != nullptr)
                {
                    m_actualState = (*m_fChangeStatus[m_actualState])(m_pStructData);
                    sprintf(txt,"StateMachine::Manage - Called ChangeState function go in:%d",m_actualState);
                    Serial.println(txt);
                }
            }
            
            // if the state is changed call the defined functions
            if (m_actualState != m_oldStatus)
            {
                if (m_bLogEnable)
                {
                    char txt[64];
                    sprintf(txt, "StateMachine::Manage - Change from %s to %s", m_StatusName[m_oldStatus], m_StatusName[m_actualState]);
                    Serial.println(txt);
                }
                if (*m_fDropOut[m_oldStatus] != nullptr)
                {
                    if (m_bLogEnable)
                    {
                        sprintf(txt,"StateMachine::Manage - Call DropOut function");
                        Serial.println(txt);
                    }
                    (*m_fDropOut[m_oldStatus])(m_pStructData);
                }
                if (*m_fTransition[m_oldStatus][m_actualState] != nullptr)
                {
                    if (m_bLogEnable)
                    {
                        sprintf(txt,"StateMachine::Manage - Call Transition function");
                        Serial.println(txt);
                    }
                    (*m_fTransition[m_oldStatus][m_actualState])(m_pStructData);
                }
                m_oldStatus = m_actualState;
                if (*m_fPickUp[m_oldStatus] != nullptr)
                {
                    if (m_bLogEnable)
                    {
                        sprintf(txt,"StateMachine::Manage - Call PickUp function");
                        Serial.println(txt);
                    }
                    (*m_fPickUp[m_oldStatus])(m_pStructData);
                }
            }
        };
        
        // call GetInitError after all AssingState calling to check consintecy of init value
        E_STATE_MACHINE_ERROR GetInitError ()   { return m_StateError; };
        
        const char* GetInitErrorString() { return stateMachineErrorString[m_StateError].s; };

        int GetStatusInd() { return m_actualState; };
        const char* GetStatusName () { return m_StatusName[m_actualState]; };
        void EnableLog (bool b_enable = false) { m_bLogEnable = b_enable; };
//
// ShowStateData 
//
        void ShowStateData ()
        {
            char txt[64];
            Serial.println(" ----- StateMachine::ShowStateData ------");
            sprintf(txt,"pStructData: %s",m_pStructData!=nullptr?"defined":"-");
            Serial.println(txt);
            for (int i = 0; i < m_numStates; i++)
            {
                sprintf(txt,"----- ind:%d - %s -----", i, m_StatusName[i]);
                Serial.println(txt);
                sprintf(txt,"fStatus:  %s",m_fStatus[i]!=nullptr?"defined":"-");
                Serial.println(txt);
                sprintf(txt,"fDropOut: %s",m_fDropOut[i]!=nullptr?"defined":"-");
                Serial.println(txt);
                for (int j = 0; j < m_numStates; j++)
                {
                    sprintf(txt,"fTransition[%d]: %s",j,m_fTransition[i][j]!=nullptr?"defined":"-");
                    Serial.println(txt);
                }
                sprintf(txt,"fPickUp: %s",m_fPickUp[i]!=nullptr?"defined":"-");
                Serial.println(txt);
                sprintf(txt,"fChangeStatusFunc: %s",m_fChangeStatus[i]!=nullptr?"defined":"-");
                Serial.println(txt);
                sprintf(txt,"MaxMsInStatus: %d",m_MaxMsInStatus[i]);
                Serial.println(txt);
                sprintf(txt,"NextStatusIfExceededMaxMsInStatus: %d",m_NextStatusIfExceededMaxMsInStatus[i]);
                Serial.println(txt);
            }
        }
};

