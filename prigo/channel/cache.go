package channel

import (
	"bytes"
	"encoding/json"
	"errors"
	"github.com/allegro/bigcache/v3"
	"net/http"
	"prigo/algo/message"
	"time"
)

var ErrNotFound = bigcache.ErrEntryNotFound

type SendDataFunc func(targetUrl string, key string, data []byte)
type RecvDataFunc func(key string, timeout int) ([]byte, error)
type GetSendUrlFunc func(targetIp string) string
type GetSendKeyFunc func(flowId, curNodeId, targetNodeId, keyName string) string
type GetRecvKeyFunc func(flowId, targetNodeId, curNodeId, keyName string) string

func GetSendUrl(targetIp string) string {
	return "http://" + targetIp + "/v1/channel/send"
}
func GetSendKey(flowId, curNodeId, targetNodeId, keyName string) string {
	return flowId + "_" + curNodeId + "_to_" + targetNodeId + "_" + keyName
}

func GetRecvKey(flowId, targetNodeId, curNodeId, keyName string) string {
	return flowId + "_" + targetNodeId + "_to_" + curNodeId + "_" + keyName
}

type BigCache struct {
	cache *bigcache.BigCache
	cli   *http.Client
}

var BigCacheGlobal *BigCache

func init() {
	BigCacheGlobal = newCache()
	message.Log.Infof("===>>init bigcache ok,BigCacheGlobal:%p", BigCacheGlobal)
	return
}
func newCache() *BigCache {
	cache, err := bigcache.NewBigCache(bigcache.DefaultConfig(time.Duration(10) * time.Second))
	if err != nil {
		panic(err)
		return nil
	}
	return &BigCache{
		cache: cache,
		cli:   &http.Client{Transport: http.DefaultTransport},
	}
}
func NewCache() (*BigCache, error) {
	cache, err := bigcache.NewBigCache(bigcache.DefaultConfig(time.Duration(10) * time.Second))
	if err != nil {
		return nil, err
	}
	return &BigCache{
		cache: cache,
		cli:   &http.Client{Transport: http.DefaultTransport},
	}, nil
}
func SetValue(key string, val []byte) error {
	return BigCacheGlobal.cache.Set(key, val)
}
func (c *BigCache) getValue(key string) ([]byte, error) {
	return c.cache.Get(key)
}
func SendData(tar_url string, key string, data []byte) {
	req := message.MsgChannelRequest{
		Key:  key,
		Data: data,
	}
	dataJson, _ := json.Marshal(&req)
	rsp, err := BigCacheGlobal.cli.Post(tar_url, "application/json", bytes.NewReader(dataJson))
	if err != nil {
		message.Log.Errorf("===>>send data error:%v", err)
		return
	}
	rsp.Body.Close()
	return
}
func RecvData(key string, timeout int) ([]byte, error) {
	dur := time.Tick(time.Duration(timeout) * time.Second)
	for {
		select {
		case <-dur:
			message.Log.Warnf("get data timeout:%v", timeout)
			return nil, errors.New("recv data timeout")
		default:
			data, err := BigCacheGlobal.getValue(key)
			if err == nil {
				BigCacheGlobal.cache.Delete(key)
				return data, nil
			} else if err != ErrNotFound {
				return nil, err
			}
		}
	}
}
