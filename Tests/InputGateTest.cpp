#include "../Source/InputGate.h"
#include <iostream>
int main(){int failures=0;const auto check=[&](bool b){if(!b)++failures;};InputGate gate;
 check(!gate.accept(false,false,1,1000));gate.record(0,1,1000);check(!gate.accept(false,false,-1,1010));check(gate.accept(false,false,1,1020));check(!gate.accept(false,false,1,1030));
 gate.record(2,-1,2000);check(!gate.accept(false,false,-1,2010));check(gate.accept(true,false,-1,2020));
 gate.record(1,1,3000);check(!gate.accept(true,false,1,3010));check(gate.accept(false,true,1,3020));
 gate.record(0,1,4000);check(!gate.accept(false,false,1,4501));gate.record(0,1,5000);gate.consume();check(!gate.accept(false,false,1,5010));
 std::cout<<(failures?"FAIL":"PASS")<<": native input required, direction/axis matching, expiry and replay rejection\n";return failures?1:0;}
