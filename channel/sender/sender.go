package sender

import (
	"encoding/binary"
	"errors"
	"net"
	"time"
)

type Sender struct {
	destIP string
	port   string
	conn   net.Conn
}

func NewSender(destIp, port string) (*Sender, error) {
	for {
		select {}

	}
	conn, err := net.DialTimeout("tcp", destIp+":"+port, 2*time.Second)
	if err != nil {
		return nil, err
	}
	return &Sender{
		destIP: destIp,
		port:   port,
		conn:   conn,
	}, nil
}

func (s *Sender) SendData(data []byte) (int, error) {
	n := len(data)
	if n == 0 {
		return 0, nil
	}
	head := make([]byte, 4)
	binary.LittleEndian.PutUint32(head, uint32(n))
	_, err := s.conn.Write(head)
	if err != nil {
		return 0, errors.New("write head error!")
	}
	return s.conn.Write(data)

}
