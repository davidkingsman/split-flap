//Used to temporarily store data to show at a later time/date
struct ScheduledMessage {
  String Message;
  long ScheduledDateTimeUnix;
};

//Used as a work around for issues that linked list has with other libraries
//Taken from here: https://github.com/vortigont/LinkedList/commit/39c2628a7794fc6e88e024d7fc82d52bc833c332
//Credit: vortigont
namespace LL { 
#include <LinkedList.h>
}

template<typename T> using LNode = LL::ListNode<T>;
template<typename T> using LList = LL::LinkedList<T>;
