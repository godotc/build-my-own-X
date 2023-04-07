package serverx

import (
	"chain/pkg/block"
	"encoding/json"
	"net/http"

	"github.com/gin-gonic/gin"
)

type Message struct {
	BPM int
}

func handleGetBlockchain(ctx *gin.Context) {
	println("get")
	bytes, err := json.MarshalIndent(block.GetBlockchain(), "", "  ")
	if err != nil {
		ctx.JSON(400, gin.H{"msg": err})
		return
	}

	ctx.JSON(200, gin.H{"data": bytes})
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

	newBlock, err := block.NewBlock(m.BPM)
	if err != nil {
		ctx.JSON(400, gin.H{
			"msg": err,
		})
	}

	ctx.JSON(http.StatusCreated, *newBlock)

}
