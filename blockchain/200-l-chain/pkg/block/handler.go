package block

import (
	"encoding/json"
	"net/http"

	"github.com/davecgh/go-spew/spew"
	"github.com/gin-gonic/gin"
)

func handleGetBlockchain(ctx *gin.Context) {
	println("get")
	bytes, err := json.MarshalIndent(Blockchain, "", "  ")
	if err != nil {
		ctx.JSON(400, gin.H{"msg": err})
		return
	}

	ctx.JSON(200, gin.H{"data": bytes})
}

type Message struct {
	BPM int
}

func handleWriteBlockchain(ctx *gin.Context) {
	println("post")
	var m Message

	decoder := json.NewDecoder(ctx.Request.Body)
	if err := decoder.Decode(&m); err != nil {
		ctx.JSON(400, gin.H{
			"body": ctx.Request.Body,
		})
		return
	}

	newBlock, err := generateBlock(Blockchain[len(Blockchain)-1], m.BPM)
	if err != nil {
		ctx.JSON(400, gin.H{
			"msg": m,
		})
		return
	}

	if isBlockValid(newBlock, Blockchain[len(Blockchain)-1]) {
		newBlockChian := append(Blockchain, newBlock)
		replaceChain(newBlockChian)
		spew.Dump(Blockchain)
	}

	ctx.JSON(http.StatusCreated, newBlock)

}
