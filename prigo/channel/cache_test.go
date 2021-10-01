package channel

import (
	"github.com/stretchr/testify/assert"
	"testing"
)

//go test -v -run TestNewCache
func TestNewCache(t *testing.T) {
	_, err := NewCache()
	assert.NoErrorf(t, err, "new cache")
}
func TestSetValue(t *testing.T) {
	_, err := NewCache()
	assert.NoError(t, err)
	err = SetValue("11", []byte("rrr"))
	assert.NoErrorf(t, err, "SetValue")
	ret, err := BigCacheGlobal.getValue("11")
	assert.NoErrorf(t, err, "getValue")
	t.Logf("ret:%v\n", string(ret))
}

func TestRecvData(t *testing.T) {
	//c, _ := NewCache()
	//err := c.SetValue("22", []byte("rrr"))
	//assert.NoErrorf(t, err, "set")
	//res, err := c.GetValue("22")
	//assert.NoErrorf(t, err, "get")
	//fmt.Printf("res:%v\n", res)
}
func TestSendData(t *testing.T) {

}
