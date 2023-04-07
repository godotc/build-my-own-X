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

	bc := block.New()

	s := serverx
	e = s.Run()
	if e != nil {
		log.Fatal(e)
	}
}
