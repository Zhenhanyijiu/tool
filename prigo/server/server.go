package server

import (
	"fmt"
	"github.com/asim/go-micro/v3/web"
	"github.com/gin-gonic/gin"
	"prigo/algo/base"
	"prigo/algo/message"
	"prigo/channel"
)

type Server struct {
	NodeId     string
	Sa         string
	newAlgo    func(string) message.Algorithm
	maxRequest chan struct{}
}

func NewServer(NodeId, sa string) *Server {
	return &Server{
		NodeId:     NodeId,
		Sa:         sa,
		newAlgo:    base.GetAlgorithm,
		maxRequest: make(chan struct{}, 1),
	}
}

/*
 /channel/send
*/
func (s *Server) SaveDataToCache(c *gin.Context) {
	var req message.MsgChannelRequest
	err := c.BindJSON(&req)
	if err != nil {
		message.Log.Errorf("===>>bindjson error:%v", err)
		return
	}
	err = channel.SetValue(req.Key, req.Data)
	if err != nil {
		fmt.Printf("===>>set val error:%v", err)
		return
	}
	//message.Log.Infof("===>>本地缓存 set data ok")
	//todo:不需要响应返回
	//c.JSON(200, gin.H{
	//	"msg": "ok",
	//})
}

/*
/algo/start
*/
func (s *Server) Start(c *gin.Context) {
	s.maxRequest <- struct{}{}
	defer func() {
		<-s.maxRequest
	}()
	var req message.MsgRequest
	c.BindJSON(&req)
	algo := s.newAlgo(req.AlgoId)
	if algo == nil {
		message.Log.Errorf("flowId:%v,算法对象为空,启动失败...", req.FlowId)
		return
	}
	algo.Start(req)
	algo.Finish()
	//没有响应返回
	return
}

func (s *Server) Route() {
	router := gin.New()
	v1Grp := router.Group("/v1")
	v1Grp.POST("/channel/send", s.SaveDataToCache)
	v1Grp.POST("/algo/start", s.Start)
	message.Log.Info("use go-micro...\n")
	webSer := web.NewService(
		web.Name("psi_"+s.Sa),
		web.Address(s.Sa),
		web.Handler(router),
	)
	message.Log.Info("server start ...\n")
	if err := webSer.Run(); err != nil {
		panic(err)
	}
}
