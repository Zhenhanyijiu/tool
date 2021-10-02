package cache

import (
	"bytes"
	"encoding/json"
	"errors"
	"fmt"
	"github.com/allegro/bigcache/v3"
	"github.com/asim/go-micro/v3/web"
	"github.com/gin-gonic/gin"
	"net/http"
	"time"
)

var ErrNotFound = bigcache.ErrEntryNotFound

type Cache struct {
	cache *bigcache.BigCache
	sa    string
	cli   *http.Client
}

func NewCache(sa string) *Cache {
	cache, err := bigcache.NewBigCache(bigcache.DefaultConfig(time.Duration(10) * time.Second))
	if err != nil {
		panic(err)
	}
	return &Cache{
		cache: cache,
		sa:    sa,
		cli:   &http.Client{Transport: http.DefaultTransport},
	}
}

func (c *Cache) SetValue(key string, value []byte) error {
	return c.cache.Set(key, value)
}

func (c *Cache) GetValue(key string, timeout int) (value []byte, err error) {
	dur := time.Tick(time.Duration(timeout) * time.Second)
	for {
		select {
		case <-dur:
			fmt.Printf("get data timeout:%v", timeout)
			return nil, errors.New("recv data timeout")
		default:
			data, err := c.cache.Get(key)
			if err == nil {
				c.cache.Delete(key)
				return data, nil
			} else if err != ErrNotFound {
				return nil, err
			}
		}
	}
}

func (c *Cache) Save(ctx *gin.Context) {
	var req MsgRequest
	err := ctx.BindJSON(&req)
	if err != nil {
		fmt.Printf("===>>bindjson error:%v", err)
		return
	}
	err = c.SetValue(req.Key, req.Data)
	if err != nil {
		fmt.Printf("===>>set val error:%v", err)
		return
	}
	//ctx.JSON(200, gin.H{"msg": "ok"})
}
func (c *Cache) CacheHandle() {
	router := gin.New()
	v1Grp := router.Group("/v1")
	v1Grp.POST("/cache/save", c.Save)
	webSer := web.NewService(
		web.Name("psi_"+c.sa),
		web.Address(c.sa),
		web.Handler(router),
	)
	fmt.Printf("server start ...\n")
	if err := webSer.Run(); err != nil {
		panic(err)
	}
}

type MsgRequest struct {
	Key  string `json:"key"`
	Data []byte `json:"data"`
}

func (c *Cache) SendData(tar_url string, key string, data []byte) {
	req := MsgRequest{
		Key:  key,
		Data: data,
	}
	dataJson, _ := json.Marshal(&req)
	rsp, err := c.cli.Post(tar_url, "application/json", bytes.NewReader(dataJson))
	if err != nil {
		fmt.Printf("===>>send data error:%v", err)
		return
	}
	rsp.Body.Close()
	return
}
