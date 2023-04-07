package main

import (
	"chain/internal/serverx"
	"log"

	"github.com/joho/godotenv"
)

func main() {
	e := godotenv.Load()
	if e != nil {
		log.Fatal(e)
	}

	s := serverx.NewServer()
	e = s.Run()
	if e != nil {
		log.Fatal(e)
	}
}
