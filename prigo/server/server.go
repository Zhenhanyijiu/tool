package server

import (
	"fmt"
	"github.com/gin-gonic/gin"
	"prigo/algo/base"
	"prigo/algo/message"
	"prigo/channel"
)

type Server struct {
	NodeId  string
	Sa      string
	newAlgo func(string) message.Algorithm
}

func NewServer(NodeId, sa string) *Server {
	return &Server{
		NodeId:  NodeId,
		Sa:      sa,
		newAlgo: base.GetAlgorithm,
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
	message.Log.Infof("===>>本地缓存 set data ok")
	//todo:不需要响应返回
	//c.JSON(200, gin.H{
	//	"msg": "ok",
	//})
}

/*
/algo/start
*/
func (s *Server) Start(c *gin.Context) {
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
	g := gin.New()
	gChannel := g.Group("/channel")
	gChannel.POST("/send", s.SaveDataToCache)
	gAlgo := g.Group("/algo")
	gAlgo.POST("/start", s.Start)
	g.Run(s.Sa)
}