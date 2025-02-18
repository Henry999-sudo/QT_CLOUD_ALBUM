package server

import (
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"os"
	"path/filepath"
	"strings"
)

const ImagePathPrefix = "/images/"

func isImageFile(path string) bool {
	extensions := []string{".jpg", ".jpeg", ".png", ".gif"} // 支持的图片文件扩展名
	ext := strings.ToLower(filepath.Ext(path))
	for _, e := range extensions {
		if ext == e {
			return true
		}
	}
	return false
}

func GetPicName(root string) (map[string]string, error) {
	var imageNamesPath map[string]string = make(map[string]string)

	err := filepath.Walk(root, func(path string, info os.FileInfo, err error) error {
		if err != nil {
			return err
		}

		if info.IsDir() {
			return nil
		}

		if isImageFile(path) {
			imageNamesPath[info.Name()] = ImagePathPrefix + info.Name()
		}

		return nil
	})

	if err != nil {
		return nil, err
	}

	return imageNamesPath, nil

}

func StartServer(root string, uploadPath string) {
	fmt.Println("Start Server...")
	mx := http.NewServeMux()

	mx.HandleFunc("/hello", func(w http.ResponseWriter, r *http.Request) {
		fmt.Fprintf(w, "你好！")
	})

	mx.HandleFunc("/get_pic_path_dict", func(w http.ResponseWriter, r *http.Request) {
		fmt.Print("someone get /get_pic_path_dict")
		m, _ := GetPicName(root)
		jsonData, err := json.Marshal(m)
		if err != nil {
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}

		// 设置Content-Type为application/json
		w.Header().Set("Content-Type", "application/json")

		// 将JSON数据写入ResponseWriter
		w.Write(jsonData)

	})

	mx.HandleFunc("/upload", func(w http.ResponseWriter, r *http.Request) {
		if r.Method == "GET" {
			// 返回一个简单的上传文件的 HTML 表单
			html := `
			<html>
			<body>
			<form method="POST" action="/upload" enctype="multipart/form-data">
				<input type="file" name="file">
				<input type="submit" value="Upload">
			</form>
			</body>
			</html>
			`
			fmt.Fprintf(w, html)
			return
		}
		fmt.Println("some one upload file!")
		// 解析上传的文件
		r.ParseMultipartForm(10 << 20) // 限制上传文件的大小

		file, handler, err := r.FormFile("file")
		if err != nil {
			fmt.Println("Error retrieving the file", err)
			return
		}
		defer file.Close()

		// 创建一个新的文件来保存上传的文件
		f, err := os.Create(uploadPath + handler.Filename)
		fmt.Println(uploadPath + handler.Filename)
		if err != nil {
			fmt.Println("Error creating the file")
			return
		}
		defer f.Close()

		// 将上传的文件内容复制到新创建的文件中
		_, err = io.Copy(f, file)
		if err != nil {
			fmt.Println("Error copying the file")
			return
		}

		fmt.Fprintf(w, "File uploaded successfully: %s\n", handler.Filename)
	})

	mx.Handle(ImagePathPrefix, http.StripPrefix(ImagePathPrefix, http.FileServer(http.Dir(root))))

	server := &http.Server{
		Addr:    ":8080",
		Handler: mx,
	}

	fmt.Println("Server is running on http://localhost:8080/hello")
	server.ListenAndServe()

}
