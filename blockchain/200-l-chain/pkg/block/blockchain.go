package block

import (
	"time"

	"github.com/davecgh/go-spew/spew"
)

type Blockchain struct {
	blocks []Block
}

func New() *Blockchain {
	t := time.Now()
	genesisBlock := Block{
		Index:     0,
		Timestamp: t.String(),
		BPM:       0,
		Hash:      "",
		PrevHash:  "",
	}
	spew.Dump(genesisBlock)

	bc := &Blockchain{}

	bc.blocks = append(bc.blocks, genesisBlock)

	return bc
}

func (bc *Blockchain) NewBlock(oldBlock Block, BPM int) error {
	newBlock, err := generateBlock(oldBlock, BPM)
	if err != nil {
		return err
	}

	if isBlockValid(newBlock, bc.blocks[len(bc.blocks)-1]) {
		newBlockChain := append(bc.blocks, newBlock)
		replaceChain(bc.blocks, newBlockChain)
		spew.Dump(bc.blocks)
	}

	return nil
}
