package transport

import (
	"encoding/binary"
	"errors"
	"fmt"
	"net"
	"time"
)

const (
	SENDER = iota
	RECEIVER
)

type Transport struct {
	partyType int
	address   string
	port      string
	conn      net.Conn
}

func NewTransport(partyType int, ip, port string) (*Transport, error) {
	dur := time.Tick(time.Second * 3)
	if partyType == SENDER {
		for {
			select {
			case <-dur:
				return nil, errors.New("dial timeout")
			default:
				conn, err := net.Dial("tcp", ip+":"+port)
				if err != nil {
					continue
				}
				return &Transport{
					partyType: partyType,
					address:   ip,
					port:      port,
					conn:      conn}, nil
			}
		}
	}
	if partyType == RECEIVER {
		listener, err := net.Listen("tcp", ip+":"+port)
		if err != nil {
			return nil, err
		}
		conn, err := listener.Accept()
		if err != nil {
			return nil, err
		}
		fmt.Printf("remoteaddr:%v\n", conn.RemoteAddr())
		return &Transport{
			partyType: partyType,
			address:   ip,
			port:      port,
			conn:      conn}, nil
	}
	return nil, errors.New("party type error")
}

func (t *Transport) SendData(data []byte) (int, error) {
	n := len(data)
	if n == 0 {
		return 0, nil
	}
	head := make([]byte, 4)
	binary.LittleEndian.PutUint32(head, uint32(n))
	_, err := t.conn.Write(head)
	if err != nil {
		return 0, errors.New("write head error!")
	}
	return t.conn.Write(data)
}
func (t *Transport) RecvData() ([]byte, error) {
	head := make([]byte, 4)
	n, err := t.conn.Read(head)
	if err != nil || n != 4 {
		return nil, errors.New("read head error")
	}
	nLen := binary.LittleEndian.Uint32(head)
	data := make([]byte, nLen)
	_, err = t.conn.Read(data)
	if err != nil {
		return nil, errors.New("read buffer error!")
	}
	return data, nil
}
