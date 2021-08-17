package channel

import (
	"bytes"
	"encoding/json"
	"errors"
	"fmt"
	"github.com/gin-gonic/gin"
	"net/http"
	"time"
)

type Request struct {
	Key  string `json:"key"`
	Data []byte `json:"data"`
}
type Response struct {
}

type Channel struct {
	cache  *Cache
	ipAddr string
}

func NewChannel(ipAddr string) (*Channel, error) {
	c, err := NewCache()
	if err != nil {
		return nil, err
	}
	return &Channel{cache: c, ipAddr: ipAddr}, nil
}
func (ch *Channel) start() {
	r := gin.New()
	r.POST("/cache", func(c *gin.Context) {
		var req Request
		err := c.BindJSON(&req)
		if err != nil {
			fmt.Printf("")
		}
		ch.cache.SetValue(req.Key, req.Data)
		c.JSON(200, gin.H{
			"msg": "ok",
		})
	})
	r.Run(ch.ipAddr)
}
func (ch *Channel) Start() {
	go ch.start()
}
func (c *Channel) SendData(tar_url string, key string, data []byte) {
	cli := http.Client{}
	req := Request{
		Key:  key,
		Data: data,
	}
	dataJson, _ := json.Marshal(&req)
	rd := bytes.NewReader(dataJson)
	cli.Post(tar_url, "application/json", rd)
	//fmt.Printf("post rsp err:%v\n", err)
}
func (c *Channel) RecvData(key string, timeout int) ([]byte, error) {
	dur := time.Tick(time.Duration(timeout) * time.Second)
	for {
		select {
		case <-dur:
			fmt.Printf("get data timeout")
			return nil, errors.New("recv data timeout")
		default:
			data, err := c.cache.GetValue(key)
			if err == nil {
				return data, nil
			} else if err != ErrNotFound {
				return nil, err
			}
		}
	}
}
