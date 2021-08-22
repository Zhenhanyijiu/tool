package main

import (
	"bytes"
	"crypto/md5"
	"encoding/hex"
	"encoding/json"
	"flag"
	"fmt"
	"github.com/allegro/bigcache/v3"
	"net/http"
	"prigo/algo/message"
	"strconv"
	"sync"
	"time"
)

func main() {
	url1 := "http://127.0.0.1:8800/algo/start"
	url2 := "http://127.0.0.1:8801/algo/start"
	num := flag.Int("num", 100, "cycle number")
	prefix := flag.String("prefix", "", "prefix to deffer")
	flag.Parse()
	if *prefix == "" {
		flag.Usage()
		return
	}
	req1 := message.MsgRequest{
		//FlowID: uu.String(),
		AlgoId: "PsiAlgo",
		CurrNode: message.Node{
			Role:   message.GUEST,
			Owner:  message.OWNER,
			NodeID: "8800",
			Ip:     "127.0.0.1:8800",
		},
		PartnerNodes: []message.Node{
			message.Node{
				Role:   message.HOST,
				Owner:  message.PARTICIPATION,
				NodeID: "8801",
				Ip:     "127.0.0.1:8801",
			},
		},
	}
	req2 := message.MsgRequest{
		//FlowID: uu.String(),
		AlgoId: "PsiAlgo",
		CurrNode: message.Node{
			Role:   message.HOST,
			Owner:  message.PARTICIPATION,
			NodeID: "8801",
			Ip:     "127.0.0.1:8801",
		},
		PartnerNodes: []message.Node{
			message.Node{
				Role:   message.GUEST,
				Owner:  message.OWNER,
				NodeID: "8800",
				Ip:     "127.0.0.1:8800",
			},
		},
	}
	for i := 0; i < *num; i++ {
		hashmd := md5.New()
		hashmd.Write([]byte(strconv.Itoa(i)))
		//key:=hashmd.Sum(nil)
		key := hex.EncodeToString(hashmd.Sum(nil))
		//key, err := uuid.NewUUID()
		//if err != nil {
		//	log.Fatal(err)
		//	return
		//}
		//id := hex.EncodeToString(uu)
		req1.FlowId = key + "_" + *prefix
		req2.FlowId = key + "_" + *prefix
		go startPost(req1, url1)
		startPost(req2, url2)
		fmt.Printf("===>i:%v,uid:%v\n", i, key)
	}

	//ecdsa.PublicKey{}
}
func startPost(data interface{}, urlData string) {
	c1 := http.Client{}
	da, _ := json.Marshal(data)
	rsp, err := c1.Post(urlData, "application/json", bytes.NewReader(da))
	if err != nil {
		fmt.Printf("error: %v\n", err)
		return
	}
	rsp.Body.Close()
}
func main11() {
	cache, err := bigcache.NewBigCache(bigcache.DefaultConfig(time.Duration(10) * time.Second))
	if err != nil {
		panic(err)
		return
	}
	wg := sync.WaitGroup{}
	cycNum := 500000
	wg.Add(1)
	go func() {
		defer wg.Done()
		hashmd := md5.New()
		for i := 0; i < cycNum; i++ {
			fmt.Printf("===>>set,i:%v\n", i)
			hashmd.Write([]byte(strconv.Itoa(i)))
			//key:=hashmd.Sum(nil)
			key := hex.EncodeToString(hashmd.Sum(nil))
			err := cache.Set(key, []byte(key))
			if err != nil {
				panic(err)
				return
			}

			hashmd.Reset()
		}
	}()
	wg.Add(1)
	go func() {
		defer wg.Done()
		hashmd := md5.New()
		for i := 0; i < cycNum; i++ {
			//fmt.Printf("===>>get,i:%v\n", i)
			hashmd.Write([]byte(strconv.Itoa(i)))
			//key:=hashmd.Sum(nil)
			key := hex.EncodeToString(hashmd.Sum(nil))
			dur := time.Tick(time.Second * time.Duration(5))
		cyc:
			for {
				select {
				case <-dur:
					fmt.Printf("timeout....\n")
					break cyc
				default:
					da, err := cache.Get(key)
					if err == nil {
						fmt.Printf("i:%v, ok, data:%v\n", i, string(da))
						break cyc
					} else if err != bigcache.ErrEntryNotFound {
						fmt.Printf("get error....\n")
						panic(err)
						break cyc
					}

				}

			}
			hashmd.Reset()
		}
	}()
	wg.Wait()
}

//func main1() {
//	// create a new service
//	service := micro.NewService(
//		micro.Name("helloworld"),
//	)
//
//	// initialise flags
//	service.Init()
//
//	// start the service
//	service.Run()
//	registry.Register()
//}
