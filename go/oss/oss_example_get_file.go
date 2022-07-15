package oss

import (
	"fmt"
	"io/ioutil"
	"os"

	"github.com/aliyun/aliyun-oss-go-sdk/oss"
)

func handleErr(err error) {
	fmt.Println("Error:", err)
	os.Exit(-1)
}

// 使用签名URL下载文件
// 以下代码用于下载目标存储空间 examplebucket 中 exampledir 目录下的 exampleobject.txt 到本地
func getFileViaSecurityToken() {
	client, err := oss.New("", "", "")
	if err != nil {
		handleErr(err)
	}
	bucketName := ""
	objectName := ""
	localDownloadedFilename := "/Users/daydaylw3/Downloads/exampleobject.JPG"
	bucket, err := client.Bucket(bucketName)
	if err != nil {
		handleErr(err)
	}
	signedURL, err := bucket.SignURL(objectName, oss.HTTPGet, 60)
	if err != nil {
		handleErr(err)
	}
	fmt.Println(signedURL)
	body, err := bucket.GetObjectWithURL(signedURL)
	if err != nil {
		handleErr(err)
	}
	defer body.Close()
	// 使用数据
	_, err = ioutil.ReadAll(body)
	if err != nil {
		handleErr(err)
	}

	// 下载文件到本地
	err = bucket.GetObjectToFileWithURL(signedURL, localDownloadedFilename)
	if err != nil {
		handleErr(err)
	}
}
