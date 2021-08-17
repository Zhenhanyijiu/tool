package channel

import (
	"fmt"
	"github.com/stretchr/testify/assert"
	"testing"
)

func TestNewCache(t *testing.T) {
	_, err := NewCache()
	//fmt.Printf("error:%v\n", err)
	assert.NoErrorf(t, err, "new cache")
}
func TestCache_SetValue(t *testing.T) {
	c, _ := NewCache()
	err := c.SetValue("11", []byte("rrr"))
	assert.NoErrorf(t, err, "set")
}

func TestCache_GetValue(t *testing.T) {
	c, _ := NewCache()
	err := c.SetValue("22", []byte("rrr"))
	assert.NoErrorf(t, err, "set")
	res, err := c.GetValue("22")
	assert.NoErrorf(t, err, "get")
	fmt.Printf("res:%v\n", res)
}
