package block

import "github.com/davecgh/go-spew/spew"

func NewBlock(BPM int) (*Block, error) {
	newBlock, err := generateBlock(Blockchain[len(Blockchain)-1], BPM)
	if err != nil {
		return nil, err
	}

	if isBlockValid(newBlock, Blockchain[len(Blockchain)-1]) {
		newBlockchain := append(Blockchain, newBlock)
		replaceChain(newBlockchain, Blockchain)
		spew.Dump(Blockchain)
	}

	return &newBlock, nil
}

func GetBlockchain() []Block {
	return Blockchain
}
