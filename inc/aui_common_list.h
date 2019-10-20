/**
<!--
Notice:             This file mainly contains Doxygen-style comments to generate
                    the HTML help file (.chm) provided by ALi Corporation to
                    give ALi Customers a quick support for using ALi AUI API
                    Interface.\n
                    Furthermore, the comments have been wrapped at about 80 bytes
                    in order to avoid the horizontal scrolling when splitting
                    the display in two part for coding, testing, debugging
                    convenience
Current ALi Author: Davy.Wu
Last update:        2016.04.25
-->

@file     aui_common_list.h

@brief    Common Function List Module

          This Module is used to define
          - One common two-way linked list prototype
          - Some basic operations

          The defined common two-way linked list can be used to construct a
          customized two-way linked list structure.\n
          The defined basic operations can be used to operate the customized
          two-way linked list, e.g. add/delete/iterate/get the structure
          variable's start pointer.

          When user wants to customize one two-way linked list structure, one
          member which type is the same as the struct #aui_list_head needs to
          be defined in own structure.

          The two-way linked list here defined is used as a circular two-way
          linked list.

@copyright  Copyright &copy; 2015 ALi &reg; Corporation. All Rights Reserved

@htmlonly <td colspan="2"><h2 class="groupheader">
List of Included Files
</h2></td> @endhtmlonly

No other files are included in this header file
*/

#ifndef __AUI_COMMON_LIST_H__

#define __AUI_COMMON_LIST_H__

#ifdef __cplusplus

extern "C" {

#endif

// @coding

/*******************************Global Type List*******************************/

/**
@brief @htmlonly <div class="details">
       Struct of the <b> Common Function List Module </b> as one basic two-way
       linked list structure
       </div> @endhtmlonly

       Definition of one basic two-way linked list structure which contains two
       pointers where
       - One points to the previous element node
       - The other one points to the next element node

       This struct is used to contruct one two-way linked element node list.
*/
struct aui_list_head {

  /**
  Member as
  - Either pointer to the @b next element node
  - Or pointer to the @b previous element node

  which type is the same as the struct #aui_list_head
  */
  struct aui_list_head *next, *prev;

};

/******************************Global Macro List*******************************/

/**
Macro used to initialize one variable which type is the same as the struct
#aui_list_head. In particular, variable's members point to the variable itself.
*/
#define AUI_LIST_HEAD_INIT(name) { &(name), &(name) }

/**
Macro used to define and initialize one variable which type is the same as the
struct #aui_list_head

Please note that the only parameter of this macro is the variable name. After the
initialization, all members of the variable point to the variable itself.
*/
#define AUI_LIST_HEAD(name) \
    struct aui_list_head name = AUI_LIST_HEAD_INIT(name)

/**
Macro used to initialize one pointer which type is the same as the struct
#aui_list_head. In particular, all members of the pointer will be initialized and
the value will be the same as pointer's value
*/
#define AUI_INIT_LIST_HEAD(ptr) \
    do { \
      (ptr)->next = (ptr); \
          (ptr)->prev = (ptr); \
              } while (0)

/**
Macro used to get the structure variable's pointer by its member's pointer and
member name, where:
- @b ptr    = The pointer of the structure variable's member
- @b type   = The structure type name
- @b member = The structure member name of the parameter @b ptr

@note  @b Example
          Structure with name @b example_st containing one member @b list_head
          as below

                struct example_st {
                       int a;
                       int b;
                       aui_list_head list_head;
                       int c;
                };

@note     Known the member @b list_head 's pointer, to retrieve the
          structure variable's pointer user can do as following:

                aui_list_head *plist = (value from some where);
                struct example_st *p_st;
                p_st = aui_list_entry(plist, struct example_st, list_head);

*/
#define aui_list_entry(ptr, type, member) \
    ((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

/**
Macro used to iterate all element nodes in the list by the head parameter from
the head to tail, where:
- @b pos  = The pointer which type is the same as the struct #aui_list_head and
            used to point to the current element node during the iteration
- @b head = The head of the circular two-way linked list which type is the same
            as the struct #aui_list_head
*/
#define aui_list_for_each(pos, head) \
    for (pos = (head)->next /*, prefetch(pos->next)*/; \
        pos != (head); \
            pos = pos->next /*, prefetch(pos->next)*/)

/**
Macro used to iterate all element nodes in the list by the head parameter from
the tail to head, where:
- @b pos  = The pointer which type is the same as the struct #aui_list_head and
            used to point to the current element node during the iteration
- @b head = The head of the circular two-way linked list which type is the same
            as the struct #aui_list_head
*/
#define aui_list_for_each_prev(pos, head) \
    for (pos = (head)->prev; \
        /*prefetch(pos->prev), */pos != (head); \
            pos = pos->prev)

/**
Macro used to iterate all element nodes in the list by the head parameter from
the head to tail, where:
- @b pos  = The pointer which type is the same as the struct #aui_list_head and
            used to point to the current element node during the iteration
- @b n    = The pointer which type is the same as the struct #aui_list_head and
            used to point to the next element node during the iteration
- @b head = The head of the circular two-way linked list which type is the same
            as the struct #aui_list_head

@note     Compared with the macro #aui_list_for_each, two pointers can be gotten
          during the iteration where:
          - One points to the current element node
          - The other one points to the next element node
*/
#define aui_list_for_each_safe(pos, n, head) \
    for (pos = (head)->next, n = pos->next; \
        pos != (head); \
            pos = n, n = pos->next)

/**
Macro used to iterate all element nodes in the list by the head parameter from
the tail to head, where:
- @b pos  = The pointer which type is the same as the struct #aui_list_head and
            used to point to the current element node during the iteration
- @b n    = The pointer which type is the same as the struct #aui_list_head and
            used to point to the previous element node during the iteraton
- @b head = The head of the circular two-way linked list which type is the same
            as the struct #aui_list_head

@note     Compared with the macro #aui_list_for_each_prev, two pointers can be
          gotten during the iteration where:
          - One points to the current element node
          - The other one points to the previous element node
*/
#define aui_list_for_each_prev_safe(pos, n, head) \
    for (pos = (head)->prev, n = pos->prev; \
        pos != (head); \
            pos = n, n = pos->prev)

/**
Macro used to iterate the structure variable's pointer which element nodes belong to.

When using #aui_list_head element nodes to construct one two-way linked list, all
these element nodes should belong to the structure variables themselves which are
the same struture data type. All pointers of these structure variables can be gotten
by using this macro where:
- @b pos    = The pointer which type is the same as the structure data type and
              used to point to the structure variable which current element node
              belongs to
- @b head   = The head of the circular two-way linked list which type is the same
              as the struct #aui_list_head
- @b member = The member name which is the element node name in the user structure
*/
#define aui_list_for_each_entry(pos, head, member) \
    for (pos = aui_list_entry((head)->next, typeof(*pos), member); \
        &pos->member != (head); \
            pos = aui_list_entry(pos->member.next, typeof(*pos), member))

/**
Macro used to iterate the structure variable's pointer which element nodes belong to.

When using #aui_list_head element nodes to construct one two-way linked list, all
these element nodes should belong to the structure variables themselves which are
the same struture data type. All pointers of these structure variables can be gotten
by using this macro, where:
- @b pos    = The pointer which type is the same as the user structure data type
              and used to point to the structure variable which current element
              node belongs to
- @b n      = The pointer which type is the same as the user structure data type
              and used to point to the structure variable which next element node
              belongs to
- @b head   = The head of the circular two-way linked list which type is the same
              as the struct #aui_list_head.
- @b member = The member name which is the element node name in the user structure

@note       Compared with the macro #aui_list_for_each_entry, the current and next
            two structure variables can be gotten during the iteration
*/
#define aui_list_for_each_entry_safe(pos, n, head, member) \
    for (pos = aui_list_entry((head)->next, typeof(*pos), member), \
        n = aui_list_entry(pos->member.next, typeof(*pos), member); \
            &pos->member != (head); \
                pos = n, n = aui_list_entry(n->member.next, typeof(*n), member))

/*****************************Global Function List*****************************/

/**
@brief       Inline function used to insert a new element node into the
             existing circular two-way linked list

@warning     It is suggested don't use this function which is mainly used by
             the inline function #aui_list_add for its internal purpose.

@param[in]   newp = The pointer to the new element node to be added between two
                    consecutive element nodes, which type is the same as the
                    struct #aui_list_head
@param[in]   prev  = The pointer to the first consecutive element node intended
                    to become the previous of the one to be added
@param[in]   next = The pointer to the second consecutive element node intended
                    to become the next of the one to be added

@note  @b 1. The general case expects to insert the new element node between
             two exixiting consecutive element nodes.\n

@note  @b 2. As one special case, at the beginning there is only one element
             node in the list then the parameter @b prev and @b next point to
             the element node itself.
*/
static __inline__ void __aui_list_add (

  struct aui_list_head * newp,

  struct aui_list_head * prev,

  struct aui_list_head * next)

{
  next->prev = newp;
  newp->next = next;
  newp->prev = prev;
  prev->next = newp;
}

/**
@brief       Inline function used to delete one element node from the existing
             circular two-way linked list

@warning     It is suggested don't use this function which is mainly used by
             the inline function #aui_list_del for its internal purpose.

@param[in]   prev = The pointer to the element node which is the previous of the
                    one to be deleted
@param[in]   next = The pointer to the element node which is the next of the one
                    to be deleted

@note  @b 1. The general case expects that there are more than three (3) element
             nodes in the existing circular two-way linked list.\n

@note  @b 2. As first special case, there are only two (2) element nodes in the list
             then the parameter @b prev and @b next point to the element node which
             need to be kept in the list.\n

@note  @b 3. As second special case, there is only one element node in the list then
             the parameter @b prev and @b next point to it but no element will be
             deleted.
*/
static __inline__ void __aui_list_del(

  struct aui_list_head * prev,

  struct aui_list_head * next

  )
{
  next->prev = prev;
  prev->next = next;
}

/**
@brief           Inline function used to insert a new element node into the existing
                 circular two-way linked list, in particular at the head of the list

@param[in]       newp   = The pointer to the new element node to be added

@param[in,out]   head   = The pointer to the head of the list
*/
static __inline__ void aui_list_add(

  struct aui_list_head *newp,

  struct aui_list_head *head

  )

{
  __aui_list_add(newp, head, head->next);
}

/**
@brief           Inline function used to insert a new element node into the existing
                 circular two-way linked list, in particular at the tail of the list

@param[in]       newp   = The pointer to the new element node to be added

@param[in,out]   head   = The pointer to the head of the list
*/
static __inline__ void aui_list_add_tail(

  struct aui_list_head *newp,

  struct aui_list_head *head

  )

{
  __aui_list_add(newp, head->prev, head);
}

/**
@brief           Inline function used to delete one element node from the existing
                 circular two-way linked list

@param[in]       entry  = The pointer to the element node to be deleted
*/
static __inline__ void aui_list_del(

  const struct aui_list_head *entry

  )

{
  __aui_list_del(entry->prev, entry->next);
}

/**
@brief           Inline function used to delete one element node from the existing
                 circular two-way linked list, and initialize again the element
                 node just deleted from the list

@param[in]       entry  = The pointer to the element node to be deleted from the
                          list
*/
static __inline__ void aui_list_del_init(

  struct aui_list_head *entry

  )

{
  __aui_list_del(entry->prev, entry->next);
  AUI_INIT_LIST_HEAD(entry);
}

/**
@brief           Inline function used to check whether the existing circular
                 two-way linked list is empty or not

@param[in]       head   = Pointer to the list to test

@return          @b 1   = The list is empty
@return          @b 0   = The list is not empty
*/
static __inline__ int aui_list_empty(

  const struct aui_list_head *head

  )

{
  return head->next == head;
}

/**
@brief           Inline function used to merge two (2) the existing circular
                 two-way linked list into one

@param[in]       list   = Pointer to the new list to be merged

@param[in,out]   head   = Pointer to the list to be merged with the new one

@note            When performing the splice, the new list will be inserted into
                 the head of the final list
 */
static __inline__ void aui_list_splice(

  const struct aui_list_head *list,

  struct aui_list_head *head

  )

{
  struct aui_list_head *first = list->next;
  if (first != list) {
    struct aui_list_head *last = list->prev;
    struct aui_list_head *at = head->next;
    first->prev = head;
    head->next = first;
    last->next = at;
    at->prev = last;
  }
}

/**
@brief           Inline function used to check whether the existing circular
                 two-way linked list is valid or not.

@param[in]       head   = Pointer to the list to be checked

@return          @b -1   = The list is invalid
@return          @b  0   = The list is empty
@return          @b  1   = The list is valid
*/
static __inline__ int aui_list_valid_check(

  const struct aui_list_head *head

  )

{
  struct aui_list_head *next = head->next;
  if(next == head)
    return 0;
  while(next!=head)
  {
    next = next->next;
    if(next == next->prev)
    {
      return -1;
    }
  }
  return 1;
}

// @endcoding

#ifdef __cplusplus

}

#endif

#endif /* __AUI_COMMON_LIST_H__ */

/* END Of FILE */

