
//
// Spin lock
//

#include <ccblkfn.h>

testset_t gFlag = 0;

adi_acquire_lock(&gFlag);

adi_release_lock(&gFlag);



//
// Protect by disabling interrupt
//

#include <builtins.h>

unsigned int _intm = cli();

sti(_intm);
