package oss

import (
	"fmt"

	"github.com/aliyun/aliyun-oss-go-sdk/oss"
)

func setObjectACL() {
	client, err := oss.New(endPoint, dayMini2AccessKeyID, dayMini2AccessKeySecret)
	if err != nil {
		handleErr(err)
	}

	bucket, err := client.Bucket(bucketName)
	if err != nil {
		handleErr(err)
	}

	acl, err := bucket.GetObjectACL("private/*")
	if err != nil {
		handleErr(err)
	}
	fmt.Println(acl)

	// if err := bucket.SetObjectACL("private/IMG_1892.JPG", oss.ACLPrivate); err != nil {
	// 	handleErr(err)
	// }

	acl, err = bucket.GetObjectACL("private/IMG_1892.JPG")
	if err != nil {
		handleErr(err)
	}
	fmt.Println(acl)
}
