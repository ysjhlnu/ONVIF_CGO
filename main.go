package main

import (
	"encoding/json"
	"net"
	"time"

	"github.com/wonderivan/logger"
)

var tcpConn *net.TCPConn

func tcpClient() bool {
	server := "127.0.0.1:1111"
	tcpAddr, err := net.ResolveTCPAddr("tcp4", server)
	if err != nil {
		logger.Warn(err)
		return false
	}
	tcpConn, err = net.DialTCP("tcp", nil, tcpAddr)
	if err != nil {
		logger.Warn(err)
		return false
	}

	logger.Debug("connection tcp success")
	return true
}

func tcpSender(conn net.Conn, words string) {
	conn.Write([]byte(words))

	//接收服务端反馈
	buffer := make([]byte, 2048)

	conn.SetReadDeadline(time.Now().Add(3 * time.Second))
	n, err := conn.Read(buffer)
	if err != nil {
		logger.Warn(err)
		return
	}
	logger.Debug(string(buffer[:n]))
	return
}

func main() {
	if tcpClient() {
		type message struct {
			Action      int
			Username    string
			Password    string
			ServiceAddr string
		}
		m := message{
			Action:      4,
			Username:    "ts",
			Password:    "ts",
			ServiceAddr: "http://192.168.2.19:80/onvif/device_service",
		}
		b, err := json.Marshal(m)
		if err != nil {
			logger.Error(err)
			return
		}
		tcpSender(tcpConn, string(b))
	}
}
