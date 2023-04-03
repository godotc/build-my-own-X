package block

import (
	"log"
	"net/http"
	"os"
	"time"

	"github.com/gin-gonic/gin"
)

type Server struct {
	gin *gin.Engine
}

func NewServer() *Server {
	return &Server{gin: gin.Default()}
}

func (s *Server) Run() error {
	println("server starting...")

	HOST := os.Getenv("HOST")
	PORT := os.Getenv("PORT")

	s.initRouter()

	server := &http.Server{
		Addr:           HOST + ":" + PORT,
		Handler:        s.gin,
		ReadTimeout:    10 * time.Second,
		WriteTimeout:   10 * time.Second,
		MaxHeaderBytes: 1 << 20,
	}

	log.Println("listen on ", "127.0.0.1:"+PORT)

	e := server.ListenAndServe()
	if e != nil {
		return e
	}

	return nil

}

func (s *Server) initRouter() {
	s.gin.GET("/", handleGetBlockchain)
	s.gin.POST("/", handleWriteBlockchain)
}
