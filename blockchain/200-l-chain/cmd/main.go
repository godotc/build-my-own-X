package main

import (
	"chain/pkg/block"
	"log"
	"time"

	"github.com/davecgh/go-spew/spew"
	"github.com/joho/godotenv"
)

func main() {
	e := godotenv.Load()
	if e != nil {
		log.Fatal(e)
	}

	go func() {
		t := time.Now()
		genesisBlock := block.Block{
			Index:     0,
			Timestamp: t.String(),
			BPM:       0,
			Hash:      "",
			PrevHash:  "",
		}

		spew.Dump(genesisBlock)
		block.Blockchain = append(block.Blockchain, genesisBlock)
	}()

	s := block.NewServer()
	e = s.Run()
	if e != nil {
		log.Fatal(e)
	}
}
