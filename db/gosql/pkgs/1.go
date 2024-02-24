package a

import "os"

func LogCreate(path string) (*os.File, error) {
	return os.OpenFile(path, os.O_RDWR|os.O_CREATE, 0644)
}

func LogAppend(fp *os.File, line string) error {
	buf := []byte(line)
	buf = append(buf, '\n')
	_, err := fp.Write(buf)
	if err != nil {
		return err
	}
	return fp.Sync()
}

type BNode struct {
	data []byte
}

const (
	BNODE_NODE           = 1
	BNODE_LEAF           = 2
	HEADER               = 4
	BTREE_PAGE_SIZE      = 4096
	BTREE_MAX_KEY_SIZE   = 1000
	BTREE_MAX_VALUE_SIZE = 3000
)

type BTree struct {
	root uint64

	get   func(uint64) BNode
	new   func(BNode) uint64 // allocate a new page
	delet func(uint64)
}

func init() {
	node_max := HEADER + 4 + 2 + 4 + BTREE_MAX_KEY_SIZE + BTREE_MAX_VALUE_SIZE
	assert(node_max <= BTREE_PAGE_SIZE)
}
