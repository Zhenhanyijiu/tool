package channel

import (
	"github.com/allegro/bigcache/v3"
	"time"
)

var ErrNotFound = bigcache.ErrEntryNotFound

type Cache struct {
	cache *bigcache.BigCache
}

func NewCache() (*Cache, error) {
	cache, err := bigcache.NewBigCache(bigcache.DefaultConfig(10 * time.Minute))
	if err != nil {
		return nil, err
	}
	return &Cache{
		cache,
	}, nil
}
func (c *Cache) SetValue(key string, val []byte) error {
	return c.cache.Set(key, val)
}
func (c *Cache) GetValue(key string) ([]byte, error) {

	return c.cache.Get(key)
}
