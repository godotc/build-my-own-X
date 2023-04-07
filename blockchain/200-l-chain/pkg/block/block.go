package block

import (
	"crypto/sha256"
	"encoding/hex"
	"time"

	"github.com/davecgh/go-spew/spew"
)

var Blockchain []Block

type Block struct {
	Index     int
	Timestamp string
	BPM       int
	Hash      string
	PrevHash  string
}

func init() {
	t := time.Now()
	genesisBlock := Block{
		Index:     0,
		Timestamp: t.String(),
		BPM:       0,
		Hash:      "",
		PrevHash:  "",
	}

	spew.Dump(genesisBlock)
	Blockchain = append(Blockchain, genesisBlock)
}

func calculateHash(block Block) string {
	record := string(rune(block.Index)) + block.Timestamp + string(rune(block.BPM)) + block.PrevHash
	h := sha256.New()
	h.Write([]byte(record))

	hash := h.Sum(nil)
	return hex.EncodeToString(hash)
}

func generateBlock(oldBlock Block, BPM int) (Block, error) {
	t := time.Now()

	newBlock := Block{
		Index:     oldBlock.Index + 1,
		Timestamp: t.String(),
		BPM:       BPM,
		PrevHash:  oldBlock.Hash,
	}
	newBlock.Hash = calculateHash(newBlock)

	return newBlock, nil
}

func isBlockValid(newBlock, oldBlock Block) bool {
	if oldBlock.Index+1 != newBlock.Index ||
		oldBlock.Hash != newBlock.PrevHash ||
		calculateHash(newBlock) != newBlock.Hash {
		return false
	}

	return true
}

func replaceChain(oldBlocks, newBLocks []Block) {
	if len(newBLocks) > len(oldBlocks) {
		oldBlocks = newBLocks
	}
}
