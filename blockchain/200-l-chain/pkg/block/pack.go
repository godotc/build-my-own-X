package block

import (
	"errors"

	"github.com/davecgh/go-spew/spew"
)

func NewBlock(BPM int) (*Block, error) {
	newBlock, err := generateBlock(Blockchain[len(Blockchain)-1], BPM)
	if err != nil {
		return nil, err
	}

	if isBlockValid(newBlock, Blockchain[len(Blockchain)-1]) {
		newBlockchain := append(Blockchain, newBlock)
		replaceChain(newBlockchain)
		//replaceChain_pass_by_value(Blockchain, newBlockchain) // failed to replace
		//replaceChain_pass_by_ptr(&Blockchain, &newBlockchain)
		spew.Dump(Blockchain)
		return &newBlock, nil
	}

	return nil, errors.New("Block not valid")

}

func GetBlockchain() []Block {
	return Blockchain
}
