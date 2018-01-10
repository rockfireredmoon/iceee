#ifndef COMMONTYPES_H
#define COMMONTYPES_H

#include <vector>
#include <string>
#include <iostream>
#include <mutex>

//These were being defined in multiple places so decided to link them here.

typedef std::vector<std::string> STRINGLIST;  //One dimensional array (ex: [0], [1], [2] ...);
typedef std::vector<STRINGLIST> MULTISTRING;  //Two dimensional array (ex: [0][0], [0][1], [1][0], etc);

#define COUNT_ARRAY_ELEMENTS(arrayObject)   (sizeof(arrayObject) / sizeof(arrayObject[0]))

#define SYNCHRONIZED(m) \
    for(std::unique_lock<std::recursive_mutex> lk(m); lk; lk.unlock())

#endif  //#ifndef COMMONTYPES_H
