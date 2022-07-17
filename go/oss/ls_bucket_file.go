package oss

import (
	"fmt"

	"github.com/aliyun/aliyun-oss-go-sdk/oss"
)

func lsFile() {
	client, err := oss.New(endPoint, dayMini1AccessKeyID, dayMini1AccessKeySecret)
	if err != nil {
		handleErr(err)
	}
	bucket, err := client.Bucket(bucketName)
	if err != nil {
		handleErr(err)
	}
	marker := ""
	for {
		lsRes, err := bucket.ListObjects(oss.Marker(marker))
		if err != nil {
			handleErr(err)
		}
		for _, obj := range lsRes.Objects {
			fmt.Println("bucket: ", obj.Key)
		}
		if !lsRes.IsTruncated {
			break
		}
		marker = lsRes.NextMarker
	}
}
