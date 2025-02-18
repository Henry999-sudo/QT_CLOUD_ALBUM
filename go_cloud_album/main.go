package main

import (
	"fmt"
	"go_cloud_album/server"
)

func main() {
	m, _ := server.GetPicName("./server/cloud_pics")
	fmt.Printf("myMap: %v\n", m)
	server.StartServer("./server/cloud_pics", "./server/cloud_pics/")
	println("你好")
}
