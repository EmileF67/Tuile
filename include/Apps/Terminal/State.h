#ifndef STATE_H
#define STATE_H


enum class State {
    Ground,
    Escape,
    CsiEntry,
    CsiParam,
    CsiIntermediate,
    EscapeIntermediate,
    String
};


#endif // STATE_H