# StateMachine

Present library implements an object **``StateMachine``**, representing an advanced finite state machine useful to build a complex automation process.

Present **``StateMachine``** object is conceived to be as more general as possible: it allowe building Moore finite state machines (where output depends only by states) or Mealy finite state machines (where output depends by inputs and state), and it embeds some time-based counters that allows to implement an advanced error handling behavior.

## Structure

Every state of present automata consist of:
1. Status function, or actions to execute when system is in present state;
2. ChangeState function, to determine if there are conditions to change state, and to what state you are going to go (next state)
3. Dropout function, or actions to execute when exiting present state;
4. Transition function, or actions to execute when exiting from present state an enter in the next state; 
5. Pickup function, or actions to execute when system enters present state;
6. Counter to store the time elapsed in present state;
7. Maximum allowed time to stack in present state;
8. String storing state information

## Working flow

