#pragma once

#include <cstdint>
#include <cstdio>
#include <stdexcept>
enum ENodeType : uint8_t {
  NODE_ROOT,
  NODE_INTERNAL,
  NODE_LEAF,
};

const uint32_t NODE_TYPE_SIZE = sizeof(uint8_t);
const uint32_t NODE_TYPE_OFFSET = 0;

const uint32_t IS_ROOT_SIZE = sizeof(uint8_t);
const uint32_t IS_ROOT_OFFSET = NODE_TYPE_SIZE;

const uint32_t PARENT_POINTER_SIZE = sizeof(uint32_t);
const uint32_t PARENT_POINTER_OFFSET = IS_ROOT_OFFSET + IS_ROOT_SIZE;

const uint8_t COMMON_NODE_HEADER_SIZE =
    NODE_TYPE_SIZE + IS_ROOT_SIZE + PARENT_POINTER_SIZE;

/*
 * Leaf Node Header Layout
 */
const uint32_t LEAF_NODE_NUM_CELLS_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_NUM_CELLS_OFFSET = COMMON_NODE_HEADER_SIZE;

const uint32_t LEAF_NODE_HEADER_SIZE =
    COMMON_NODE_HEADER_SIZE + LEAF_NODE_NUM_CELLS_SIZE;

/*
 * Leaf Node Body Layout
 */
const uint32_t LEAF_NODE_KEY_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_KEY_OFFSET = 0;
// body see implement in table....

inline ENodeType get_node_type(char *node) {
  return *(ENodeType *)(node + NODE_TYPE_OFFSET);
}

inline void set_node_type(char *node, ENodeType type) {
  *(node + NODE_TYPE_OFFSET) = type;
}

inline uint32_t *leaf_node_num_cells(char *node) {
  return (uint32_t *)(node + LEAF_NODE_NUM_CELLS_OFFSET);
}

inline void initialize_leaf_node(char *node) {
  set_node_type(node, NODE_LEAF);
  *leaf_node_num_cells(node) = 0;
}

// TODO: WHY NOT use the nodetype bits to reprsent the root?
inline bool is_node_root(char **node) {
  uint8_t value = *((uint8_t *)(node + IS_ROOT_OFFSET));
  return (bool)value;
}

inline void set_node_root(char *node, bool is_root) {
  uint8_t value = is_root;
  *((uint8_t *)(node + IS_ROOT_OFFSET)) = value;
}

/*
 * Internal Node Header Layout
 */
const uint32_t INTERNAL_NODE_NUM_KEYS_SIZE = sizeof(uint32_t);
const uint32_t INTERNAL_NODE_NUM_KEYS_OFFSET = COMMON_NODE_HEADER_SIZE;

const uint32_t INTERNAL_NODE_RIGHT_CHILD_SIZE = sizeof(uint32_t);
const uint32_t INTERNAL_NODE_RIGHT_CHILD_OFFSET =
    INTERNAL_NODE_NUM_KEYS_OFFSET + INTERNAL_NODE_NUM_KEYS_SIZE;

const uint32_t INTERNAL_NODE_HEADER_SIZE = COMMON_NODE_HEADER_SIZE +
                                           INTERNAL_NODE_NUM_KEYS_SIZE +
                                           INTERNAL_NODE_RIGHT_CHILD_SIZE;

/*
+ * Internal Node Body Layout
+ */
const uint32_t INTERNAL_NODE_KEY_SIZE = sizeof(uint32_t);
const uint32_t INTERNAL_NODE_CHILD_SIZE = sizeof(uint32_t);
const uint32_t INTERNAL_NODE_CELL_SIZE =
    INTERNAL_NODE_CHILD_SIZE + INTERNAL_NODE_KEY_SIZE;

inline uint32_t *internal_node_num_keys(char *node) {
  return (uint32_t *)(node + INTERNAL_NODE_NUM_KEYS_OFFSET);
}

inline uint32_t *internal_node_right_child(char *node) {
  return (uint32_t *)(node + INTERNAL_NODE_RIGHT_CHILD_OFFSET);
}

inline uint32_t *internal_node_cell(char *node, uint32_t cell_index) {
  return (uint32_t *)(node + INTERNAL_NODE_HEADER_SIZE +
                      cell_index * INTERNAL_NODE_CELL_SIZE);
}

inline uint32_t *internal_node_child(char *node, uint32_t child_index) {
  uint32_t num_keys = *internal_node_num_keys(node);
  if (child_index > num_keys) {
    char buf[512];
    sprintf(buf, "Tried to access child_index %d > num_keys %d\n", child_index,
            num_keys);
    throw std::runtime_error(buf);
  } else if (child_index == num_keys) {
    return internal_node_right_child(node);
  } else {
    return internal_node_cell(node, child_index);
  }
}

inline uint32_t *internal_node_key(char *node, uint32_t key_index) {
  return internal_node_cell(node, key_index) + INTERNAL_NODE_CHILD_SIZE;
}
