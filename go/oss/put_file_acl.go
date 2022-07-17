package oss

import (
	"fmt"
	"strings"

	"github.com/aliyun/aliyun-oss-go-sdk/oss"
)

func putFileWithACL(key accessKey, acl oss.ACLType, obj string) {
	client, err := oss.New(endPoint, key.id, key.secret)
	if err != nil {
		handleErr(err)
	}

	bucket, err := client.Bucket(bucketName)
	if err != nil {
		handleErr(err)
	}

	if err := bucket.PutObject(obj, strings.NewReader("Hello I am private"), oss.ObjectACL(acl)); err != nil {
		handleErr(err)
	}
}

func getFile(key accessKey, obj, target string) {
	client, err := oss.New(endPoint, key.id, key.secret)
	if err != nil {
		handleErr(err)
	}
	bucket, err := client.Bucket(bucketName)
	if err != nil {
		handleErr(err)
	}
	signedURL, err := bucket.SignURL(obj, oss.HTTPGet, 30)
	if err != nil {
		handleErr(err)
	}
	fmt.Println(signedURL)
	body, err := bucket.GetObjectWithURL(signedURL)
	if err != nil {
		handleErr(err)
	}
	defer body.Close()

	err = bucket.GetObjectToFileWithURL(signedURL, target)
	if err != nil {
		handleErr(err)
	}
}
