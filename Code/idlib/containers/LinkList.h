/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition
Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or
modify it under the terms of the GNU General Public License as published by the
Free Software Foundation, either version 3 of the License, or (at your option)
any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see
<http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain
additional terms. You should have received a copy of these additional terms
immediately following the terms and conditions of the GNU General Public License
which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a
copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional
terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite
120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#ifndef __LINKLIST_H__
#define __LINKLIST_H__

/*
==============================================================================

idLinkList

Circular linked list template

==============================================================================
*/

template <class type>
class idLinkList {
 public:
  idLinkList();
  virtual ~idLinkList();
  idLinkList(const idLinkList&) = default;
  idLinkList& operator=(const idLinkList&) = default;
  idLinkList(idLinkList&&) = default;
  idLinkList& operator=(idLinkList&&) = default;

  bool IsListEmpty() const noexcept;
  bool InList() const noexcept;
  int Num() const;
  void Clear() noexcept;

  void InsertBefore(idLinkList& node) noexcept;
  void InsertAfter(idLinkList& node);
  void AddToEnd(idLinkList& node) noexcept;
  void AddToFront(idLinkList& node);

  void Remove() noexcept;

  std::shared_ptr<type> Next() const noexcept;
  std::shared_ptr<type> Prev() const noexcept;

  std::shared_ptr<type> Owner() const;
  void SetOwner(std::shared_ptr<type> object) noexcept;

  idLinkList* ListHead() const;
  idLinkList* NextNode() const;
  idLinkList* PrevNode() const;

 private:
  idLinkList* head;
  idLinkList* next;
  idLinkList* prev;
  std::weak_ptr<type> owner;
};

/*
================
idLinkList<type>::idLinkList

Node is initialized to be the head of an empty list
================
*/
template <class type>
idLinkList<type>::idLinkList() {
  owner.reset();
  head = this;
  next = this;
  prev = this;
}

/*
================
idLinkList<type>::~idLinkList

Removes the node from the list, or if it's the head of a list, removes
all the nodes from the list.
================
*/
template <class type>
idLinkList<type>::~idLinkList() {
  Clear();
}

/*
================
idLinkList<type>::IsListEmpty

Returns true if the list is empty.
================
*/
template <class type>
bool idLinkList<type>::IsListEmpty() const noexcept {
  return head->next == head;
}

/*
================
idLinkList<type>::InList

Returns true if the node is in a list.  If called on the head of a list, will
always return false.
================
*/
template <class type>
bool idLinkList<type>::InList() const noexcept {
  return head != this;
}

/*
================
idLinkList<type>::Num

Returns the number of nodes in the list.
================
*/
template <class type>
int idLinkList<type>::Num() const {
  idLinkList<type>* node;
  int num;

  num = 0;
  for (node = head->next; node != head; node = node->next) {
    num++;
  }

  return num;
}

/*
================
idLinkList<type>::Clear

If node is the head of the list, clears the list.  Otherwise it just removes the
node from the list.
================
*/
template <class type>
void idLinkList<type>::Clear() noexcept {
  if (head == this) {
    while (next != this) {
      next->Remove();
    }
  } else {
    Remove();
  }
}

/*
================
idLinkList<type>::Remove

Removes node from list
================
*/
template <class type>
void idLinkList<type>::Remove() noexcept {
  prev->next = next;
  next->prev = prev;

  next = this;
  prev = this;
  head = this;
}

/*
================
idLinkList<type>::InsertBefore

Places the node before the existing node in the list.  If the existing node is
the head, then the new node is placed at the end of the list.
================
*/
template <class type>
void idLinkList<type>::InsertBefore(idLinkList& node) noexcept {
  Remove();

  next = &node;
  prev = node.prev;
  node.prev = this;
  prev->next = this;
  head = node.head;
}

/*
================
idLinkList<type>::InsertAfter

Places the node after the existing node in the list.  If the existing node is
the head, then the new node is placed at the beginning of the list.
================
*/
template <class type>
void idLinkList<type>::InsertAfter(idLinkList& node) {
  Remove();

  prev = &node;
  next = node.next;
  node.next = this;
  next->prev = this;
  head = node.head;
}

/*
================
idLinkList<type>::AddToEnd

Adds node at the end of the list
================
*/
template <class type>
void idLinkList<type>::AddToEnd(idLinkList& node) noexcept {
  InsertBefore(*node.head);
}

/*
================
idLinkList<type>::AddToFront

Adds node at the beginning of the list
================
*/
template <class type>
void idLinkList<type>::AddToFront(idLinkList& node) {
  InsertAfter(*node.head);
}

/*
================
idLinkList<type>::ListHead

Returns the head of the list.  If the node isn't in a list, it returns
a pointer to itself.
================
*/
template <class type>
idLinkList<type>* idLinkList<type>::ListHead() const {
  return head;
}

/*
================
idLinkList<type>::Next

Returns the next object in the list, or NULL if at the end.
================
*/
template <class type>
std::shared_ptr<type> idLinkList<type>::Next() const noexcept {
  if (!next || (next == head)) {
    return nullptr;
  }
  return next->owner.lock();
}

/*
================
idLinkList<type>::Prev

Returns the previous object in the list, or NULL if at the beginning.
================
*/
template <class type>
std::shared_ptr<type> idLinkList<type>::Prev() const noexcept {
  if (!prev || (prev == head)) {
    return nullptr;
  }
  return prev->owner.lock();
}

/*
================
idLinkList<type>::NextNode

Returns the next node in the list, or NULL if at the end.
================
*/
template <class type>
idLinkList<type>* idLinkList<type>::NextNode() const {
  if (next == head) {
    return nullptr;
  }
  return next;
}

/*
================
idLinkList<type>::PrevNode

Returns the previous node in the list, or NULL if at the beginning.
================
*/
template <class type>
idLinkList<type>* idLinkList<type>::PrevNode() const {
  if (prev == head) {
    return nullptr;
  }
  return prev;
}

/*
================
idLinkList<type>::Owner

Gets the object that is associated with this node.
================
*/
template <class type>
std::shared_ptr<type> idLinkList<type>::Owner() const {
  return owner.lock();
}

/*
================
idLinkList<type>::SetOwner

Sets the object that this node is associated with.
================
*/
template <class type>
void idLinkList<type>::SetOwner(std::shared_ptr<type> object) noexcept {
  owner = object;
}

#endif /* !__LINKLIST_H__ */
