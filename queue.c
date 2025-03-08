#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head =
        (struct list_head *) malloc(sizeof(struct list_head));
    if (!head) {
        return NULL;
    }
    INIT_LIST_HEAD(head);
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head)
        return;

    element_t *safe, *it;
    list_for_each_entry_safe (it, safe, head, list)
        q_release_element(it);

    free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *element = malloc(sizeof(element_t));
    if (!element)
        return false;
    element->value = strdup(s);
    if (!element->value) {
        free(element);
        return false;
    }
    list_add(&element->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;
    return q_insert_head(head->prev, s);
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *elem = list_first_entry(head, element_t, list);
    list_del(&elem->list);

    if (sp && bufsize) {
        strncpy(sp, elem->value, bufsize);
        sp[bufsize - 1] = '\0';
    }

    return elem;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    return q_remove_head(head->prev->prev, sp, bufsize);
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;

    int cnt = 0;
    struct list_head *it;

    list_for_each (it, head)
        ++cnt;

    return cnt;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head))
        return false;
    struct list_head *slow, *fast;
    slow = fast = head->next;
    for (; fast != head && (fast = fast->next) != head; fast = fast->next)
        slow = slow->next;
    element_t *element = list_entry(slow, element_t, list);
    list_del(&element->list);
    q_release_element(element);
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head)
        return false;
    bool dup = false;
    element_t *prev = NULL, *curr = NULL;
    list_for_each_entry (curr, head, list) {
        bool equal = prev && !strcmp(prev->value, curr->value);
        if (equal || dup) {
            list_del(&prev->list);
            q_release_element(prev);
            dup = equal;
        }
        prev = curr;
    }
    // Check the last element
    if (dup) {
        list_del(&prev->list);
        q_release_element(prev);
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head)
        return;
    struct list_head *node;
    list_for_each (node, head) {
        if (node->next == head)
            break;
        list_move(node, node->next);
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head)
        return;
    struct list_head *node, *safe;
    list_for_each_safe (node, safe, head)
        list_move(node, head);
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (!head || list_empty(head))
        return;
    struct list_head *node = head->next;
    while (true) {
        struct list_head *safe = node, *start = node->prev;
        // Check if there are k nodes from this node.
        for (int i = 0; i < k; ++i) {
            if (node == head)
                return;
            node = node->next;
        }
        // Reset node and safe pointer.
        node = safe;
        safe = node->next;
        // Reverse k nodes by insert them to the next of the start node.
        for (int i = 0; i < k; ++i) {
            list_move(node, start);
            node = safe;
            safe = safe->next;
        }
    }
}

void q_merge_two(struct list_head *left, struct list_head *right, bool descend)
{
    if (!left || !right)
        return;

    // Create a temporary list
    struct list_head tmp_head;
    INIT_LIST_HEAD(&tmp_head);

    // Move list node to temporary list
    while (!list_empty(left) && !list_empty(right)) {
        element_t *left_front = list_first_entry(left, element_t, list);
        element_t *right_front = list_first_entry(right, element_t, list);
        char *first_str = left_front->value, *second_str = right_front->value;
        bool condition;
        if (descend)
            condition = strcmp(first_str, second_str) > 0;
        else
            condition = strcmp(first_str, second_str) < 0;
        element_t *minimum = condition ? left_front : right_front;
        list_move_tail(&minimum->list, &tmp_head);
    }

    // Move the remaining nodes
    list_splice_tail_init(left, &tmp_head);
    list_splice_tail_init(right, &tmp_head);

    // Move the temporary list back to the left
    list_splice(&tmp_head, left);
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    // Create fast and slow pointer to get the middle node
    struct list_head *slow = head, *fast = head->next;
    for (; fast != head && (fast = fast->next) != head; fast = fast->next)
        slow = slow->next;

    // Create a new list
    struct list_head left;

    // Move the nodes from the beginning to the middle node in `head` list to
    // `left` list
    list_cut_position(&left, head, slow);
    q_sort(&left, descend);
    q_sort(head, descend);
    q_merge_two(head, &left, descend);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    element_t *entry = NULL;
    int size = 0;
    list_for_each_entry (entry, head, list) {
        struct list_head *prev = entry->list.prev, *safe = prev->prev;
        for (; prev != head; prev = safe, safe = safe->prev) {
            element_t *prev_entry = list_entry(prev, element_t, list);
            if (strcmp(prev_entry->value, entry->value) <= 0)
                break;
            --size;
            list_del(prev);
            q_release_element(prev_entry);
        }
        ++size;
    }
    return size;
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    element_t *entry = NULL;
    int size = 0;
    list_for_each_entry (entry, head, list) {
        struct list_head *prev = entry->list.prev, *safe = prev->prev;
        for (; prev != head; prev = safe, safe = safe->prev) {
            element_t *prev_entry = list_entry(prev, element_t, list);
            if (strcmp(prev_entry->value, entry->value) >= 0)
                break;
            --size;
            list_del(prev);
            q_release_element(prev_entry);
        }
        ++size;
    }
    return size;
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    if (!head || list_empty(head))
        return 0;

    // We need to locate the first queue
    queue_contex_t *first = NULL, *entry, *safe;
    bool first_time = true;
    int size = 0;
    list_for_each_entry_safe(entry, safe, head, chain) {
        // Locate the first queue
        if (first_time) {
            first_time = false;
            first = entry;
            size += q_size(first->q);
            continue;
        }
        size += q_size(entry->q);

        // Merge the following queues into the first queue
        q_merge_two(first->q, entry->q, descend);

        // Move the current queue to the beginning of the `queue_chain_t`
        list_move(&entry->chain, head);
    }
    // After the procedure, the first queue will be at the tail of the `queue_chain_t`.
    // Therefore, we move the first queue to the beginning of the `queue_chain_t`.
    list_move(&first->chain, head);
    return size;
}
